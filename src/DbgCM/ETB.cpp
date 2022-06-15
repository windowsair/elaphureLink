/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.2
 * @date     $Date: 2020-07-30 14:15:04 +0200 (Thu, 30 Jul 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM Embedded Trace Buffer Module
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision
 * and Cortex-M processor based microcontrollers.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include "stdafx.h"
#define _IN_TARG_
#include "..\BOM.h"
#include "Collect.h"
#include "Debug.h"
#include "ETB.h"
#include "CSTF.h"

#include "TraceWinConnect.h"
#include "TraceTypes.h"
#include "..\TraceView\TraceDataTypes.h"
#include "Trace.h"
#include "PDSCDebug.h"


#define MaxLinkData   0x1000   // Max Debug probe Data for R/W


BOOL  ETB_TMC        = FALSE;    // ETB is TMC in ETB mode

// CS Location (Potentially Shared in Multi-Core System)
CS_LOCATION ETB_Location   = { 0, 0, 0 }; // ETB Register Base Location, addr 0 if ETB not available
BOOL        ETB_Configured = FALSE;    // ETB is available and set up
BOOL        ETB_Active     = FALSE;    // ETB Active Flag (CTRL.CAPTEN set)

BOOL        ETB_Updated    = FALSE;    // Indicates whether the ETB buffer has been read since last activation
BOOL        ETB_Wraparound = FALSE;    // Indicates whether the ETB buffer was wrapped around

// ETB properties
DWORD ETB_Width     = 0;            // ETB word size in bytes
DWORD ETB_WMask     = 0;            // ETB word mask

DWORD ETB_TrigDelay = 0;

// ETB Formatter Mode, required if combining ETM and ITM
DWORD ETB_FmtMode = 0;

// Trace sources connected to ETB
BOOL  ETB_ETMConnected = FALSE;
BOOL  ETB_ITMConnected = FALSE;

// ETB Main Registers
RgETB RegETB;                   // ETB Registers


// ETB's ITM Decoder
BOOL ETB_ITMSynchronized;   // ITM Stream Synchronized
BYTE ETB_ITMFlags;          // Flags of currently decoded ITM record
TRC_DATAS ETB_ITMRecord;    // ITM Record which is currently filled


#define ETB_BUFFER_SIZE     0x10000  // 64 KByte per buffer (General, ETM, ITM)
#define ETB_FMT_FRAME_SIZE  0x10     // 16 Bytes per formatter packet

// ETB Formatter Frame Packet Buffer
BYTE  ETB_PacketID;                          // Current ATB ID
BYTE  ETB_FormatPacket[ETB_FMT_FRAME_SIZE];  // Buffer to store the current FormatterPacket
DWORD ETB_FormatPacketIdx;                   // Current Index in ETB_FormatPacket

BYTE  ETB_Buffer   [ETB_BUFFER_SIZE];  // ETB Formatter output if enabled
BYTE  ETB_ITMBuffer[ETB_BUFFER_SIZE];  // ITM part of the ETB contents (NOT DECODED YET!)

DWORD ETB_Num;           // Current number of bytes in ETB_Buffer
DWORD ETB_ETMNum;        // Current number of bytes in ETB_ETMBuffer
DWORD ETB_ITMNum;        // Current number of bytes in ETB_ITMBuffer
DWORD ETB_Idx;           // Next slot to fill in ETB_Buffer
DWORD ETB_ETMIdx;        // Next slot to fill in ETB_ETMBuffer
DWORD ETB_ITMIdx;        // Next slot to fill in ETB_ITMBuffer

// External functions to send decoded instruction trace to trace window
extern void  ITM_Record (TRC_DATAS *pItem);
extern void  DeleteLastRecord();
extern void  Flow_Record (BYTE nFlags, BOOL cyc);

// External functions to initialize the trace view
extern void TraceWinInit (void);
extern void InitTraceInterface(void);
extern void TraceWinClr (void);


// External memory attributes (reused from streaming trace)
MAMAP      ETB_MAdr;         // Current Memory Address (ETB)
WORD       ETB_MAtr;         // Current Memory Attributes
struct EMM ETB_MSeg;         // Current Memory Segment

extern struct EMM (*slots[256])[256];


// Forward declarations
static DWORD ETB_ProcessITMData();



// ETB_ClearBuffer
//   Clears ETB input buffer for formatted ETB data
static void ETB_ClearBuffer() {
  ETB_Num             = 0;
  ETB_Idx             = 0;
  ETB_FormatPacketIdx = 0;
}


// ETB Trace Status Update
//   return : 0 - OK,  else error code
int ETB_TraceStatus () {

  if (T_Err & T_ERR_DATA_ERR) {
    T_Err &=~ T_ERR_DATA_ERR;
    if (T_Msg >= T_MSG_DATA_ERR) {
      if (T_Msg != T_MSG_DATA_ERR) {
        T_Msg = T_MSG_DATA_ERR;
        OutTraceMsg(T_Msg);
      }
    }
  }
  if (T_Err & T_ERR_DATA_OVF) {
    T_Err &=~ T_ERR_DATA_OVF;
    if (T_Msg >= T_MSG_DATA_OVF) {
      if (T_Msg != T_MSG_DATA_OVF) {
        T_Msg = T_MSG_DATA_OVF;
        OutTraceMsg(T_Msg);
      }
    }
  }

  if (T_Err) {
    ;
  } else {
    OutMsg("");
  }

  return (0);
}


// Decode incoming formatted ETB data, output goes into ETB's ETM and ITM buffers
// Reused parts from TPIU_Process(), Trace.cpp
static DWORD ETB_ProcessFormattedData () {
  int   i = 0;
  BYTE  bit, id;

  // ETB Formatter (similar to TPIU Formatter)
  while (ETB_Num) {
    ETB_FormatPacket[ETB_FormatPacketIdx] = ETB_Buffer[(ETB_Idx - ETB_Num) & (ETB_BUFFER_SIZE - 1)];
    ETB_Num--;

    if (++ETB_FormatPacketIdx == ETB_FMT_FRAME_SIZE) {
      ETB_FormatPacketIdx = 0;
      for (i = 0; i < ETB_FMT_FRAME_SIZE; i += 2) {
        bit = (ETB_FormatPacket[15] >> (i >> 1)) & 1;
        if (ETB_FormatPacket[i] & 1) {
          // Even Byte is ID
          id = ETB_PacketID;
          ETB_PacketID = ETB_FormatPacket[i] >> 1;
          if (i == 14) break;
          if (bit == 0) {
            if (id != ETB_PacketID) {
              // Packet ID change which immediately applies
              // Decode data of previous source first (synchronization)
              switch(id) {
              case 0:
                // Do nothing, NULL source
                break;
              case ITM_ATBID:
                ETB_ProcessITMData();
                break;
              case ETM_ATBID:
                break;
              // else unknown source, cannot decode
              }
            }
            id = ETB_PacketID;
          }
          switch (id) {
            case 0:     // Idle
              break;
            case ITM_ATBID:
              ETB_ITMBuffer[ETB_ITMIdx] = ETB_FormatPacket[i+1];
              ETB_ITMIdx = (ETB_ITMIdx + 1) & (ETB_BUFFER_SIZE - 1);
              ETB_ITMNum++;
              break;
            case ETM_ATBID:
              break;
            case 0x7D:  // Trigger (Next Data Byte must be zero)
              TRACE("Received TPIU Trigger\n");
              break;
            case 0x7E:  // Reserved
              break;
            case 0x7F:  // Not allowed (conflicts with Sync Packets)
              T_Err |= T_ERR_DATA_ERR;
              break;
          }
          if (id != ETB_PacketID) {
            // Packet ID change applies after second byte
            // Decode data of previous source first (synchronization)
            switch(id) {
            case 0:
              // Do nothing, NULL source
              break;
            case ITM_ATBID:
              ETB_ProcessITMData();
              break;
            case ETM_ATBID:
              break;
            // else unknown source, cannot decode
            }
          }
        } else {
          // Even Byte is Data
          switch (ETB_PacketID) {
            case ITM_ATBID:
              ETB_ITMBuffer[ETB_ITMIdx] = (ETB_FormatPacket[i] & 0xFE) | bit;
              ETB_ITMIdx = (ETB_ITMIdx + 1) & (ETB_BUFFER_SIZE - 1);
              ETB_ITMNum++;
              if (i == 14) break;
              ETB_ITMBuffer[ETB_ITMIdx] = ETB_FormatPacket[i+1];
              ETB_ITMIdx = (ETB_ITMIdx + 1) & (ETB_BUFFER_SIZE - 1);
              ETB_ITMNum++;
              break;
            case ETM_ATBID:
              break;
          }
        }
      }
    }
  }

  ETB_ProcessITMData();

  return ETB_Num;
}



// ETB_ClearITMBuffer
//   Clears ITM input buffer
static void ETB_ClearITMBuffer() {
  ETB_ITMNum = 0;
  ETB_ITMIdx = 0;
}


// ETB_ClearITMRecord()
//   Resets ETB_ITMRecord and associated state variables
static __inline void ETB_ClearITMRecord() {
  ETB_ITMFlags  = 0;
  memset(&ETB_ITMRecord, 0, sizeof(ETB_ITMRecord));
}

// ETB_InitITMDecoder
//   Initialize global ITM Decoder variables
static void ETB_InitITMDecoder() {
  ETB_ITMSynchronized = FALSE;
  ETB_ClearITMRecord();
}


// ITM Process Trace Data
// Reused parts from ITM_ProcessData(), ITM.cpp
// ETB_ITMNum, ETB_ITMIdx and ETB_ITMBuffer filled by ETB_ProcessFormattedData
//   return : Remaining number of bytes (incomplete packet)
static DWORD ETB_ProcessITMData () {
  BYTE     ctrl;
  DWORD    data;
  BYTE     size;
  DWORD    cnt;
  DWORD    n;
  BYTE     tb;
  BYTE     dwtnum;
  DWORD    nCurIdx = (ETB_ITMIdx - ETB_ITMNum) & (ETB_BUFFER_SIZE - 1);
  BYTE     *td = &ETB_ITMBuffer[nCurIdx];

  while (ETB_ITMNum) {

    tb  = *td++; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
    cnt =  ETB_ITMNum - 1;

    if (tb == 0x00) {
      // Idle or Start of Sync
      if (cnt < 5) return (ETB_ITMNum);
      for (n = 0; n < cnt; n++) {
        tb = *td++; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
        if (tb != 0x00) break;
      }
      if (tb == 0x80) {
        if (n >= (5 - 1)) {
          ETB_ITMSynchronized = TRUE;
        }
        ETB_ITMNum -= n + 1 + 1;
        continue;
      }
      if (tb == 0x00) {
        if (n > (5 - 1)) return (5);
      }
      ETB_ITMNum -= n + 1;
      cnt         = ETB_ITMNum - 1;
    }

    if (tb == 0x70) {
      // Overflow
      if (cnt == 0) return (ETB_ITMNum);
      ETB_ITMRecord.bOvf = 1;
      T_Err             |= T_ERR_DATA_OVF;  // leave it for now
      ETB_ITMNum--;
      continue;
    }

    size = tb & 0x03;

    if (size == 0x00) {
      switch ((tb >> 2) & 0x03) {
        case 0x00:  // Time Stamp
          ctrl = (tb >> 4) & 0x07;
          data = 0;
          for (n = 0; n < 4; n++) {
            if (!(tb & 0x80)) break;
            if (cnt == 0) return (ETB_ITMNum);
            tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
            data |= (tb & 0x7F) << (7*n);
          }
          if ((n == 4) && (tb & 0x80)) {
            T_Err |= T_ERR_DATA_ERR;
            break;
          }
          break;
        case 0x01:  // Reserved
          for (n = 0; n < 4; n++) {
            if (!(tb & 0x80)) break;
            if (cnt == 0) return (ETB_ITMNum);
            tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
          }
          break;
        case 0x02:  // SW Extension
          data = (tb >> 4) & 0x07;
          if (tb & 0x80) {
            if (cnt == 0) return (ETB_ITMNum);
            tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
            data |= (tb & 0x7F) << 3;
            if (tb & 0x80) {
              if (cnt == 0) return (ETB_ITMNum);
              tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
              data |= (tb & 0x7F) << 10;
              if (tb & 0x80) {
                if (cnt == 0) return (ETB_ITMNum);
                tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
                data |= (tb & 0x7F) << 17;
                if (tb & 0x80) {
                  if (cnt == 0) return (ETB_ITMNum);
                  tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
                  data |= tb << 24;
                }
              }
            }
          }
          // SW Extension Packet (parameter: data) not used
          break;
        case 0x03:  // HW Extension
          data = (tb >> 4) & 0x07;
          if (tb & 0x80) {
            if (cnt == 0) return (ETB_ITMNum);
            tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
            data |= (tb & 0x7F) << 3;
            if (tb & 0x80) {
              if (cnt == 0) return (ETB_ITMNum);
              tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
              data |= (tb & 0x7F) << 10;
              if (tb & 0x80) {
                if (cnt == 0) return (ETB_ITMNum);
                tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
                data |= (tb & 0x7F) << 17;
                if (tb & 0x80) {
                  if (cnt == 0) return (ETB_ITMNum);
                  tb = *td++; cnt--; if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
                  data |= tb << 24;
                }
              }
            }
          }
          // HW Extension Packet (parameter: data) not used
          break;
      }
      ETB_ITMNum = cnt;
      continue;
    }

    size = 1 << (size - 1);
    if (cnt < size) return (ETB_ITMNum);
    data = 0;
    for (n = 0; n < size; n++) {
      data |= *td++ << (8*n); if (td >= ETB_ITMBuffer + ETB_BUFFER_SIZE) td = &ETB_ITMBuffer[0];
    }

    if (tb & 0x04) {
      // HWIT
      switch ((tb >> 6) & 0x03) {
        case 0x00:      // Events
          switch ((tb >> 3) & 0x07) {
            case 0x00:  // Event (8-bit)
              ETB_ITMRecord.nSampTyp = TR_TYPE_EVT;
              ETB_ITMRecord.nData    = data & ~TR_EVT_CYC;  /* Mask out CYCEVT */
              ETB_ITMRecord.nDataS   = (size < 3) ? size : 3;
              ITM_Record(&ETB_ITMRecord);
              ETB_ClearITMRecord();
              break;
            case 0x01:  // EXCTRC Event (16-bit)
              ETB_ITMRecord.nSampTyp = TR_TYPE_EXCTRC;
              ETB_ITMRecord.nData    = data;
              ETB_ITMRecord.nDataS   = 0;
              ITM_Record(&ETB_ITMRecord);
              ETB_ClearITMRecord();
              break;
            case 0x02:  // PC Sample Event (32-bit)
              ETB_ITMRecord.nSampTyp = TR_TYPE_PC_SAMPLE;
              ETB_ITMRecord.bPC      = 1;
              ETB_ITMRecord.nAddr    = data;
              ITM_Record(&ETB_ITMRecord);
              ETB_ClearITMRecord();
              break;
            default:    // Reserved
              break;
          }
          break;
        case 0x01:      // PC or AddrOfs
          if (tb & 0x08) {
            // AddrOfs (16bit)
            dwtnum = (tb >> 4) & 0x03;
            // set exact address to nAddr and make it valid as exact address by bExec
            if (RegDWT.CMP[dwtnum].FUNC & DWT_DATAVMATCH) {
              ETB_ITMRecord.nAddr = (RegDWT.CMP[((RegDWT.CMP[dwtnum].FUNC >> DWT_DATAVADDR0) & 0xF)].COMP & 0xFFFF0000) | data;
            } else {
              ETB_ITMRecord.nAddr = (RegDWT.CMP[dwtnum].COMP & 0xFFFF0000) | data;
            }
            ETB_ITMRecord.bExec = 1;
          } else {
            // PC
            ETB_ITMRecord.nAddr = data;
            ETB_ITMRecord.bPC   = 1;
          }
          break;
        case 0x02:      // Data R/W
          ETB_ITMRecord.nSampTyp = (tb & 0x08) ? TR_TYPE_DATA_WRITE : TR_TYPE_DATA_READ;
          dwtnum = (tb >> 4) & 0x03;
          if (RegDWT.CMP[dwtnum].FUNC & DWT_DATAVMATCH) {
            ETB_ITMRecord.nMemAddr = RegDWT.CMP[((RegDWT.CMP[dwtnum].FUNC >> DWT_DATAVADDR0) & 0xF)].COMP;
          } else {
            ETB_ITMRecord.nMemAddr = RegDWT.CMP[dwtnum].COMP;
          }


          if (RegDWT.CMP[dwtnum].FUNC & DWT_DATAVMATCH) {
            if (RegDWT.CMP[dwtnum].FUNC & DWT_DATAVSIZEH) {
              ETB_ITMRecord.nData = RegDWT.CMP[dwtnum].COMP & 0xFFFF;
            } else if (RegDWT.CMP[dwtnum].FUNC & DWT_DATAVSIZEW) {
              ETB_ITMRecord.nData = RegDWT.CMP[dwtnum].COMP;
            } else {
              ETB_ITMRecord.nData = RegDWT.CMP[dwtnum].COMP & 0xFF;
            }
          } else {
            ETB_ITMRecord.nData = data;
          }

          ETB_ITMRecord.nDataS = (size < 3) ? size : 3;
          ITM_Record(&ETB_ITMRecord);
          ETB_ClearITMRecord();
          break;
        case 0x03:      // Reserved
          break;
      }
    } else {
      // SWIT
      ETB_ITMRecord.nSampTyp = TR_TYPE_ITM;
      ETB_ITMRecord.nMemAddr = (tb >> 3) & 0x1F;
      ETB_ITMRecord.nData    = data;
      ETB_ITMRecord.nDataS   = (size < 3) ? size : 3;
      ITM_Record(&ETB_ITMRecord);
      ETB_ClearITMRecord();
    }

    ETB_ITMNum -= n + 1;

  }

  return (ETB_ITMNum);
}


// ETB_ReadData()
//  Read data from ETB and store to ETB_Buffer,
//  destination depends on used ETB formatter mode.
// nMany : Number of ETB words to read from target
// Returns 0 on success, otherwise error.
static int ETB_ReadData(DWORD nMany) {
  int status = 0;
  DWORD nIdx, n, rb, bi;
  DWORD nNum;
  DWORD bufferedWord;
  DWORD bytesPerAccess = (ETB_TMC ? 4 : ETB_Width);
  DWORD nAccesses      = (ETB_TMC ? (nMany*ETB_Width)/4 : nMany); // TMC Mem width always a multiple of 32, hence of 4
  DWORD nBytes         = nAccesses*bytesPerAccess;
  BOOL  widthIsMul4    = ((ETB_Width/4)*4 == ETB_Width);          // ETB Width is multiple of 4
  BYTE* pBuf;
  DWORD APSel;

  // Set data destination
  if (ETB_FmtMode) {
    nIdx = ETB_Idx;
    nNum = ETB_Num;
    pBuf = ETB_Buffer;
  } else {
    return 0;
  }

  if (widthIsMul4) {
    // Optimized version for ETB/TMC widths which are a multiple of 4
    while (nBytes > 0) {
      bi = (nBytes > MaxLinkData) ? MaxLinkData : nBytes;  // bytes in iteration

      status = LinkCom(1);                    // Apply lock to protect AP_Sel
      if (status) return (status);

      APSel  = AP_Sel;
      AP_Sel = (ETB_Location.AP << APSEL_P);  // Select ETB AP

      if (nIdx + bi > ETB_BUFFER_SIZE) {
        // ETB_BUFFER_SIZE multiple of 4 bytes, buffer entries will be 4-byte aligned when reaching buffer boundary.
        //  => Can simply split up the data read into two block reads

        // First read to buffer end
        rb = (ETB_BUFFER_SIZE - nIdx);
        status = ReadBlock(ETB_RD_DATA, &pBuf[nIdx], rb, BLOCK_NADDRINC);
        if (status) goto end_err;
        nNum += rb; bi -= rb; nIdx = 0;

        // Second read from buffer start
        rb = bi - rb;
        status = ReadBlock(ETB_RD_DATA, &pBuf[nIdx], rb, BLOCK_NADDRINC);
        if (status) goto end_err;
        nNum += rb; nIdx  = (nIdx + rb) & (ETB_BUFFER_SIZE - 1);
      } else {
        // Single read
        status = ReadBlock(ETB_RD_DATA, &pBuf[nIdx], bi, BLOCK_NADDRINC);
        if (status) goto end_err;
        nNum += bi;
        nIdx = (nIdx + bi) & (ETB_BUFFER_SIZE - 1);
      }

      AP_Sel = APSel;          // Restore previous AP setting
      status = LinkCom(0);
      if (status) return (status);

      nBytes -= bi;
    }
  } else {
    // ETB width is not multiple of 4 (ETB in 24bit mode), slow read (should be a rare case)
    while (nAccesses) {

      status = LinkCom(1);                   // 09.11.2018: Apply lock to protect AP_Sel, do inside loop for fairness (other threads may need some access time)
      if (status) return (status);

      APSel  = AP_Sel;
      AP_Sel = (ETB_Location.AP << APSEL_P);  // Select ETB AP

      if (nIdx + sizeof(DWORD) > ETB_BUFFER_SIZE) {
        // Read to temp buffer and write byte by byte to buffer (wraparound)
#if DBGCM_V8M
        status = ReadD32(ETB_RD_DATA, &bufferedWord, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
        status = ReadD32(ETB_RD_DATA, &bufferedWord);
#endif // DBGCM_V8M
        if (status) goto end_err;
        for (n = 0; n < bytesPerAccess; n++) {
          pBuf[nIdx] = (BYTE)(bufferedWord & (0xFF << n)) >> n;
          nIdx       = (nIdx + 1) & (ETB_BUFFER_SIZE - 1);
        }
      } else {
        // Directly store data in destination buffer
#if DBGCM_V8M
        status = ReadD32(ETB_RD_DATA, (DWORD*)&pBuf[nIdx], BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
        status = ReadD32(ETB_RD_DATA, (DWORD*)&pBuf[nIdx]);
#endif // DBGCM_V8M
        if (status) goto end_err;
        nIdx  = (nIdx + bytesPerAccess) & (ETB_BUFFER_SIZE - 1);
      }

      AP_Sel = APSel;          // Restore previous AP setting
      status = LinkCom(0);
      if (status) return (status);

      nNum += bytesPerAccess;
      nAccesses--;
    }
  }

  // Update destination number and index
  if (ETB_FmtMode) {
    ETB_Idx = nIdx;
    ETB_Num = nNum;
  } else {
    return 0;
  }

  return (0);

end_err:
  AP_Sel = APSel;          // Restore previous AP setting
  LinkCom(0);

  return (status);
}


// Stop ETB Capturing and wait for all required bits to be set
static int ETB_StopCapture() {
  DWORD val, tick;
  int status = 0;
  DWORD APSel;

  status = ETB_Flush();        /* Flush the ETB and connected components (data may be still
                                  latched somewhere in the path, e.g. ATB upsizer).  */
  if (status) return status;

  status = LinkCom(1);                   // 16.11.2018: Apply lock to protect AP_Sel
  if (status) goto end;

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

  // Disable trace capture
  RegETB.CTRL = 0;
#if DBGCM_V8M
  status = WriteD32(ETB_CTRL, RegETB.CTRL, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ETB_CTRL, RegETB.CTRL);
#endif // DBGCM_V8M
  if (status) goto end;
  ETB_Active  = FALSE;

  // Wait for AcqComplete flag
  tick = GetTickCount();
  do {
#if DBGCM_V8M
    status = ReadD32(ETB_STATUS, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = ReadD32(ETB_STATUS, &val);
#endif // DBGCM_V8M
    if ((val & ETB_STATUS_ACQCOMP) != 0) break;
  } while ((GetTickCount() - tick) < 10);
  if ((val & ETB_STATUS_ACQCOMP) == 0) {
    status = EU53; //capture error
    goto end;
  }

  // Wait for DFEEmpty
  if ((val & ETB_STATUS_DFEMPTY) == 0) {
    tick = GetTickCount();
    do {
#if DBGCM_V8M
      status = ReadD32(ETB_STATUS, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
      status = ReadD32(ETB_STATUS, &val);
#endif // DBGCM_V8M
      if ((val & ETB_STATUS_DFEMPTY) != 0) break;
    } while ((GetTickCount() - tick) < 10);
  }
  if ((val & ETB_STATUS_DFEMPTY) == 0) {
    status = EU53; //capture error
    goto end;
  }


  /* Clear ETB FFCR, some targets seem to need this to recover
     from a met stop condition. */
#if DBGCM_V8M
  status = WriteD32(ETB_FFCR, 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ETB_FFCR, 0);
#endif // DBGCM_V8M
  if (status) goto end;

end:
  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  return (status);
}




// ETB Process Trace
//   bAppend: TRUE  - Append data to buffer if the ETB is not full.
//            FALSE - Always clear the buffer regardless of the ETB status
int ETB_Process () {
  //BYTE  buf[MaxULINKData];
  DWORD  adr;
  DWORD  nWordsToRead, nWordsInETB, nWordsRead, nBytesInETB;
  DWORD  nPossWords;
  DWORD  mask = 0;
  DWORD  dataWords = 0;
  DWORD  val = 0, nTries = 0;//, nIdx = 0;
  int    status = 0;
  DWORD *pNum = NULL;
  DWORD  APSel;


  //if (!ETB_Active) return (0);
  if (!ETB_Configured) return (0);

  // Abort if buffer is already up to date
  if (ETB_Updated) {
    // is up to date
    return (0);
  }

  ETB_ClearBuffer();
  ETB_ClearITMBuffer();
  Flow_Record(TR_TYPE_FLOW_GAP, 0);

  // Stop ETB Capturing
  status = ETB_StopCapture();             // also flushes the ETB and sets ETB_Active to FALSE
  if (status) return status;

  status = LinkCom(1);                   // 16.11.2018: Apply lock to protect AP_Sel
  if (status) return status;

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

  // Check if ETB is full (wrapped around)
#if DBGCM_V8M
  status = ReadD32(ETB_STATUS, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(ETB_STATUS, &val);
#endif // DBGCM_V8M
  if (status) goto init_end;
  ETB_Wraparound = (val & ETB_STATUS_FULL);

  if (ETB_Updated) {
    TraceWinClr();  // This function skips the test for "trace is processing"
  }

  // Get read and write pointers
  adr = ETB_RD_PTR;
#if DBGCM_V8M
  status = ReadARMMem(&adr, (BYTE*)&RegETB.RD_PTR, 2*4, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadARMMem(&adr, (BYTE*)&RegETB.RD_PTR, 2*4);
#endif // DBGCM_V8M
  if (status) goto init_end;

  ETB_InitITMDecoder();

  // Calc amount of data to read and invalidate PC on ETB wraparound
  if (ETB_Wraparound) {
    // overflow or full, cannot distinguish, but if not overflown a sync packet will come first
    nWordsInETB   = RegETB.DEPTH;
    RegETB.RD_PTR = RegETB.WR_PTR;
    if (ETB_TMC) {
      nBytesInETB   = nWordsInETB*4;  // TMC RAM Size in 32bit words
    } else {
      nBytesInETB   = nWordsInETB*ETB_Width;
    }
  } else {
    RegETB.RD_PTR = 0;
    if (ETB_TMC) {
      // TMC: Read and Write Pointers are in Bytes
      nBytesInETB   = RegETB.WR_PTR;
      nWordsInETB   = nBytesInETB/ETB_Width;
    } else {
      // Standard ETB: Read and Write Pointers are data words
      nWordsInETB   = RegETB.WR_PTR;
      nBytesInETB   = nWordsInETB*ETB_Width;
    }
  }

  if (ETB_Wraparound && nBytesInETB > 0) {
    //Indicate a wraparound as gap in the stream
    //Flow_Record(TR_TYPE_FLOW_GAP, FALSE);
  }

  // Set read pointer to oldest buffer entry
#if DBGCM_V8M
  status = WriteD32(ETB_RD_PTR, RegETB.RD_PTR, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ETB_RD_PTR, RegETB.RD_PTR);
#endif // DBGCM_V8M
  if (status) goto init_end;

  // Make sure that the ETB is accessible through JTAG
  RegETB.CTRL &= ~ETB_CTRL_SWCTRL;
#if DBGCM_V8M
  status = WriteD32(ETB_CTRL, RegETB.CTRL, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ETB_CTRL, RegETB.CTRL);
#endif // DBGCM_V8M
  if (status) goto init_end;

init_end:
  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }
  if (status) return (status);

  // Set the flag to avoid any additional updates by other threads
  ETB_Updated = TRUE;

  // Outer loop to initiate read from ETB and decoding
  nWordsRead = 0;
  pNum       = ETB_FmtMode ? &ETB_Num : &ETB_ETMNum;
  while (nWordsRead < nWordsInETB) {
    // Make sure that next read to buffer fits in
    nPossWords = (ETB_BUFFER_SIZE - *pNum) / ETB_Width;
    if (nPossWords > nWordsInETB - nWordsRead) {
      nWordsToRead = nWordsInETB - nWordsRead;
    } else {
      nWordsToRead = nPossWords;
    }

    // Do the reading
    status = ETB_ReadData(nWordsToRead);
    if (status) return status;
    nWordsRead += nWordsToRead;

    // Do the decoding
    if (ETB_FmtMode) {
      // Decode ETB formatter
      ETB_ProcessFormattedData();
    } else {
      // Decode ETM data
    }

    // Update trace status
    ETB_TraceStatus();
  }

  return 0;
}


// ETB_Flush()
//   Manually flush the ETB and wait for the flush to finish
int ETB_Flush() {
  DWORD val, ticks;
  int status = 0;
  DWORD APSel;

  status = LinkCom(1);                   // 16.11.2018: Apply lock to protect AP_Sel
  if (status) return (status);

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

#if DBGCM_V8M
  status = ReadD32(ETB_FFSR, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(ETB_FFSR, &val);
#endif // DBGCM_V8M
  if (status) goto end;

  if ((val & ETB_FFSR_FTSTOPPED) == 0) {
#if DBGCM_V8M
    status = ReadD32(ETB_FFCR, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = ReadD32(ETB_FFCR, &val);
#endif // DBGCM_V8M
    if (status) goto end;

    // 21.11.2018: Enable StopOnFlush as requird for ETB/TMC to really stop
    val |= ETB_FFCR_STOPFL;
#if DBGCM_V8M
    status = WriteD32(ETB_FFCR, val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(ETB_FFCR, val);
#endif // DBGCM_V8M
    if (status) goto end;

    // Trigger manual flash
    val |= ETB_FFCR_FONMAN;
#if DBGCM_V8M
    status = WriteD32(ETB_FFCR, val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(ETB_FFCR, val);
#endif // DBGCM_V8M
    if (status) goto end;

    ticks = GetTickCount();
    do {
#if DBGCM_V8M
      status = ReadD32(ETB_FFCR, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
      status = ReadD32(ETB_FFCR, &val);
#endif // DBGCM_V8M
      if ((val & ETB_FFCR_FONMAN) == 0) break;
    } while ((GetTickCount() - ticks) < 10);
    if ((val & ETB_FFCR_FONMAN) != 0) {
      status = EU53;
      goto end;
    }
  } // else stopped by a previous flush

end:
  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  return (status);
}



// ETB Activate Trace
int ETB_Activate (void) {
  int status = 0;
  DWORD addr, val, ticks;
  DWORD APSel;

  if (ETB_Location.Addr == 0) {
    return (EU32);  // Trace HW not present
  }

  status = LinkCom(1);                   // 16.11.2018: Apply lock to protect AP_Sel
  if (status) return (status);

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

  // Clear the stop bits if set
#if DBGCM_V8M
  status = ReadD32(ETB_FFCR, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(ETB_FFCR, &val);
#endif // DBGCM_V8M
  if (status) goto end;
  if (val & (ETB_FFCR_STOPFL | ETB_FFCR_STOPTRIG)) {
    val &= ~(ETB_FFCR_STOPFL | ETB_FFCR_STOPTRIG);
#if DBGCM_V8M
    status = WriteD32(ETB_FFCR, val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(ETB_FFCR, val);
#endif // DBGCM_V8M
    if (status) goto end;
  }

  // 10.07.2012: Changed from ETB_FFCR_STOPTRIG, only. Some targets stop
  //             the ETB too early => required leftovers in not-flushable ETM
  // Set the formatter mode, make the formatter flush on trigger and stop after flush
  val = ETB_FFCR_STOPFL | ETB_FFCR_FONTRIG | ETB_FmtMode;
#if DBGCM_V8M
  status = WriteD32(ETB_FFCR, val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ETB_FFCR, val);
#endif // DBGCM_V8M
  if (status) goto end;

  if (ETB_TMC) {
    // TODO: Review this once we got a properly working target!!

    AP_Sel = APSel;                      // Restore previous AP setting

    status = LinkCom(0);                // 16.11.2018: Release temporarily. A potentially long timeout is ahead
    if (status) return (status);

    status = LinkCom(1);                // 16.11.2018: Reapply lock to protect AP_Sel
    if (status) return (status);

    APSel  = AP_Sel;
    AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

    // Wait for TMC to stop
    ticks = GetTickCount();
    do {
#if DBGCM_V8M
      status = ReadD32(ETB_STATUS, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
      status = ReadD32(ETB_STATUS, &val);
#endif // DBGCM_V8M
      if (status) goto end;
      if (val & ETB_STATUS_ACQCOMP) break;
    } while (GetTickCount() - ticks < 1000);
    if ((val & ETB_STATUS_ACQCOMP) == 0) {
      status = EU54;                    // Trace Flush Failed
      goto end;
    }

    AP_Sel = APSel;                      // Restore previous AP setting

    status = LinkCom(0);                // 16.11.2018: Release temporarily. Coming back from a potentially long timeout
    if (status) return (status);


    status = LinkCom(1);                // 16.11.2018: Reapply lock to protect AP_Sel
    if (status) return (status);

    APSel  = AP_Sel;
    AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

    // Disable TMC if required
#if DBGCM_V8M
    status = ReadD32(ETB_CTRL, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = ReadD32(ETB_CTRL, &val);
#endif // DBGCM_V8M
    if (status) goto end;

    if (val & ETB_CTRL_CAPTEN) {
#if DBGCM_V8M
      status = WriteD32(ETB_CTRL, 0x0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
      status = WriteD32(ETB_CTRL, 0x0);
#endif // DBGCM_V8M
      if (status) goto end;
    }

    // Set Operating Mode
#if DBGCM_V8M
    status = WriteD32(TMC_MODE, TMC_MODE_CIRCULAR, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(TMC_MODE, TMC_MODE_CIRCULAR);
#endif // DBGCM_V8M
    if (status) goto end;
  }

  // Set pointers, delay counter and enable capturing
  RegETB.RD_PTR  = 0;
  RegETB.WR_PTR  = 0;
  RegETB.TRG_CNT = ETB_TrigDelay;
  RegETB.CTRL    = ETB_CTRL_CAPTEN;  //also disables the AHB access
  addr           = ETB_RD_PTR;
#if DBGCM_V8M
  status         = WriteARMMem(&addr, (BYTE*)&RegETB.RD_PTR, 4*4, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status         = WriteARMMem(&addr, (BYTE*)&RegETB.RD_PTR, 4*4);
#endif // DBGCM_V8M
  if (status) goto end;

  // ETB is now active
  ETB_Active     = TRUE;
  ETB_Updated    = FALSE;
  ETB_Wraparound = FALSE;

end:
  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  return (status);
}


// ETB Setup Trace
int ETB_Setup (void) {
  int status = 0;
  DWORD adr, val;
  DWORD APSel;

  // Check for Trace HW
  if (ETB_Location.Addr == 0) {
    return 0;// DEBUGGING (EU32);  // Trace HW not present
  }

  // Latch Active Trace Options
  TraceOpt = TraceConf.Opt;

  // Remove Unlimited Trace Flag, should be left in TraceConf.Opt
  // though if switching between TPIU and ETB.
  TraceOpt &= ~UNLIMITED_TRACE;

  if (!CSTF_Single) {
    // More than one trace funnel, manual configuration of trace funnel;
    // enabling formatter since assuming multiple trace sources (not always true if multiple funnels).
    ETB_FmtMode = ETB_FFCR_ENFTC | ETB_FFCR_ENFCONT;
  } else if (CSTF_Addr != 0) {

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled() || CSTF_GetSlavesNum() > 0) {
#else // DBGCM_DBG_DESCRIPTION
    if (CSTF_GetSlavesNum() > 0) {
#endif // DBGCM_DBG_DESCRIPTION

      // Single trace funnel, multiple slaves connected or debug description, use formatter
      ETB_FmtMode = ETB_FFCR_ENFTC | ETB_FFCR_ENFCONT;
    }
  }

  status = LinkCom(1);                    // Apply lock to protect AP_Sel
  if (status) return (status);

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

  // Get initital information
  adr = ETB_Location.Addr;
#if DBGCM_V8M
  status = ReadARMMem(&adr, (BYTE*)&RegETB, sizeof(RegETB), BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadARMMem(&adr, (BYTE*)&RegETB, sizeof(RegETB));
#endif // DBGCM_V8M
  if (status) goto etb_end;

  if (ETB_TMC) {
#if DBGCM_V8M
    status = ReadD32(TMC_DEVID, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = ReadD32(TMC_DEVID, &val);
#endif // DBGCM_V8M
    if (status) goto etb_end;

    ETB_Width = 1 << ((val & TMC_DEVID_MEMWIDTH_MASK) >> TMC_DEVID_MEMWIDTH_POS);  // 21.11.2018: ETB_Width calculation for TMC fixed
    ETB_WMask = 0xFFFFFFFF;
  } else {
    // Read ETB Properties
    switch (RegETB.WIDTH) {
    case ETB_WIDTH_24BIT:
      ETB_Width = 3;
      ETB_WMask = 0x00FFFFFF;
      break;
    case ETB_WIDTH_32BIT:
    default:
      // assume default word size to be 32 bit
      ETB_Width = 4;
      ETB_WMask = 0xFFFFFFFF;
      break;
    }
  }

  // Get information on connected Trace Sources
#if DBGCM_DBG_DESCRIPTION
  if (!CSTF_Single || PDSCDebug_IsEnabled()) {
#else // DBGCM_DBG_DESCRIPTION
  if (!CSTF_Single) {
#endif // DBGCM_DBG_DESCRIPTION
    // Assume all trace features connected to ETB (TMC)
    ETB_ETMConnected = FALSE;			// TODO: SET to TRUE once ETM is supported
    ETB_ITMConnected = TRUE;
  } else {
    ETB_ETMConnected = (CSTF_ETMConnected() == 0) ? FALSE : TRUE;
    ETB_ITMConnected = (CSTF_ITMConnected() == 0) ? FALSE : TRUE;
  }

etb_end:
  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }
  if (status) return (status);

  // Special handling
  //status = Trace_Setup_...;
  //if (status) return (status);

  ETB_Configured = TRUE;

  return (status);
}

// ETB Init Trace
int ETB_Init (void) {

  if (ETB_Location.Addr == 0) {
    // ETB requested but not available
    return EU32;
  }

  // Initialize the trace window part
  pio->bTrcDisp = 1;
  // init the new window first, otherwise old one will be set up!!
  InitTraceInterface();
#if 0
  ConfigureTraceWinETB();
#endif
  TraceWinInit();

  return (0);
}

// ETB_CanAccess
//   Checks if the ETB is stopped and the buffer can be safely accessed
//   Returns TRUE if accessible, FALSE if not.
BOOL ETB_CanAccess() {
  DWORD status = 0;
  DWORD val;
  DWORD APSel;

  if (ETB_Configured == FALSE || ETB_Location.Addr == 0) {
    return FALSE;
  }

  status = LinkCom(1);                    // Apply lock to protect AP_Sel
  if (status) return FALSE;

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

#if DBGCM_V8M
  status = ReadD32(ETB_FFSR, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(ETB_FFSR, &val);
#endif // DBGCM_V8M

  AP_Sel = APSel;          // Restore previous AP setting

  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  if (status) return FALSE;

  // ETB can be accessed as soon as its formatter is stopped
  return (val & ETB_FFSR_FTSTOPPED);
}

int ETB_Clear(void) {
  // Noticing the clear, enables another update from the ETB
  ETB_Updated    = FALSE;
  ETB_Wraparound = FALSE;
  return 0;
}

// ETB Recovery, e.g. after low-power mode
int ETB_Recovery(void) {
  int status;
  DWORD val;
  DWORD APSel;

  status = LinkCom(1);                    // Apply lock to protect AP_Sel
  if (status) return (status);

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

  // Check ETB state
#if DBGCM_V8M
  status = ReadD32(ETB_CTRL, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(ETB_CTRL, &val);
#endif // DBGCM_V8M

  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  if (status) return (status);

  if (val & ETB_CTRL_CAPTEN) {
    // ETB is capturing, nothing to do
    return (0);
  }

  status = LinkCom(1);                    // Apply lock to protect AP_Sel
  if (status) return (status);

  APSel  = AP_Sel;
  AP_Sel = (ETB_Location.AP << APSEL_P);  // Set ETB AP

  // Clear the stop bits if set
#if DBGCM_V8M
  status = ReadD32(ETB_FFCR, &val, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#else // DBGCM_V8M
  status = ReadD32(ETB_FFCR, &val);
  if (status) goto end;
#endif // DBGCM_V8M

  if (val & (ETB_FFCR_STOPFL | ETB_FFCR_STOPTRIG)) {
    val &= ~(ETB_FFCR_STOPFL | ETB_FFCR_STOPTRIG);

#if DBGCM_V8M
    status = WriteD32(ETB_FFCR, val, BLOCK_SECTYPE_ANY);
    if (status) goto end;
#else // DBGCM_V8M
    status = WriteD32(ETB_FFCR, val);
    if (status) goto end;
#endif // DBGCM_V8M

  }

  // Changed from ETB_FFCR_STOPTRIG, only. Some targets stop
  // the ETB too early => required leftovers in not-flushable ETM
  // Set the formatter mode, make the formatter flush on trigger and stop after flush
  val = ETB_FFCR_STOPFL | ETB_FFCR_FONTRIG | ETB_FmtMode;

#if DBGCM_V8M
  status = WriteD32(ETB_FFCR, val, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#else // DBGCM_V8M
  status = WriteD32(ETB_FFCR, val);
  if (status) goto end;
#endif // DBGCM_V8M

  // Set delay counter and enable capturing, use values as set during activation
  status  = WriteBlock(ETB_TRG_CNT, (BYTE*)&RegETB.TRG_CNT, 2*4, 0 /*attrib*/);
  if (status) goto end;

end:
  AP_Sel = APSel;          // Restore previous AP setting
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  return (status);
}

void InitETB() {
  ETB_TMC        = FALSE;

  // CS Location (Potentially Shared in Multi-Core System)
  memset(&ETB_Location, 0, sizeof(ETB_Location));
  ETB_Configured = FALSE;
  ETB_Active     = FALSE;

  ETB_Updated    = FALSE;
  ETB_Wraparound = FALSE;

  // ETB properties
  ETB_Width     = 0;
  ETB_WMask     = 0;

  ETB_TrigDelay = 0;

  // ETB Formatter Mode, required if combining ETM and ITM
  ETB_FmtMode = 0;

  // Trace sources connected to ETB
  ETB_ETMConnected = FALSE;
  ETB_ITMConnected = FALSE;

  // ETB Main Registers
  memset(&RegETB, 0, sizeof(RegETB));

  // ETB's ITM Decoder
  ETB_ITMSynchronized = 0;
  ETB_ITMFlags = 0;

  memset(&ETB_ITMRecord, 0, sizeof(ETB_ITMRecord));
	ETB_ITMRecord.nCycles = -1;
	ETB_ITMRecord.Time = 0.0;

  // ETB Formatter Frame Packet Buffer
  ETB_PacketID = 0;
  memset(&ETB_FormatPacket, 0, sizeof(ETB_FormatPacket));
  ETB_FormatPacketIdx = 0;

  memset(&ETB_Buffer, 0, sizeof(ETB_Buffer));
  memset(&ETB_ITMBuffer, 0, sizeof(ETB_ITMBuffer));

  ETB_Num = 0;
  ETB_ETMNum = 0;
  ETB_ITMNum = 0;
  ETB_Idx = 0;
  ETB_ETMIdx = 0;
  ETB_ITMIdx = 0;

  // External memory attributes (reused from streaming trace)
  memset(&ETB_MAdr, 0, sizeof(ETB_MAdr));
  ETB_MAtr = 0;
  memset(&ETB_MSeg, 0, sizeof(ETB_MSeg));
}
