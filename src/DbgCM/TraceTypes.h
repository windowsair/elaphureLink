/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.0
 * @date     $Date: 2015-05-26 14:55:12 +0200 (Tue, 26 May 2015) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     Internal Trace Decoder Data Types
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

#ifndef __TRACETYPES_H__
#define __TRACETYPES_H__

#include "..\agdi_types.h"
#include "..\TraceDat.h"

#pragma pack(1)

#define TRC_SIZE (1<<20)        // 1M samples
#define TRC_MASK (TRC_SIZE - 1)


typedef struct trc_dataS {      // Trace Data Sample
  UINT32   nSampTyp : 8;        // type of Trace-Sample, at this point value 0 is TR_TYPE_FLOW
  UINT32     bFlag1 : 1;        // internal: used for 'highlight search hit'
  UINT32     bFlag2 : 1;        // internal: reserved for BookMark
  UINT32     bFlag3 : 1;        // internal: reserved for indicating a fake timestamp
  UINT32     bFlag4 : 1;        // internal: reserved  //jenrei02: temporarily using to forward bAddr
  UINT32     bExec  : 1;        // ETM: Execution Flag, ITM - Data Access: nPC holds exact data access address.
  UINT32     bOvf   : 1;        // Overflow Flag (ITM)
  UINT32     bPC    : 1;        // PC exists (ITM Data R/W)
  UINT32     bFlag  : 1;        // Reserved Flag
  UINT32     nDataS : 2;        // ITM: Data Size (0:None, 1:8-bit, 2:16-bit, 3:32-bit)
  UINT32     bTS    : 1;        // Timestamp exists (Cycles & Time)
  UINT32     bTD    : 1;        // Timestamp delayed Flag
  UINT32          nAddr;        // Execution Address (PC)
  UINT32       nMemAddr;        // Memory Address
  UINT32          nData;        // Data (Memory, Exception, ITM), Flow packet type
  I64           nCycles;        // Cycles
  double           Time;        // Time
  UINT32        nOffset;        /* Offset into a "cycle package", i.e. the offset starting from 0 within a
                                   set of records associated with a cycle number; ITM records and ETM records
                                   having separate timestamps with the same cycle number are treated as one
                                   package. */

  trc_dataS() :	nSampTyp(0),
	  bFlag1(0),
	  bFlag2(0),
	  bFlag3(0),
	  bFlag4(0),
	  bExec(0),
	  bOvf(0),
	  bPC(0),
	  bFlag(0),
	  nDataS(0),
	  bTS(0),
	  bTD(0),
	  nAddr(0),
	  nMemAddr(0),
	  nData(0),
	  nCycles(-1),
	  Time(0.0),
    nOffset(0) {};

  void operator=(const trc_dataS& val) {
    this->bExec = val.bExec;
    this->bFlag = val.bFlag;
    this->bFlag1 = val.bFlag1;
    this->bFlag2 = val.bFlag2;
    this->bFlag3 = val.bFlag3;
    this->bFlag4 = val.bFlag4;
    this->bOvf = val.bOvf;
    this->bPC = val.bPC;
    this->bTD = val.bTD;
    this->bTS = val.bTS;
    this->nAddr = val.nAddr;
    this->nCycles = val.nCycles;
    this->nData = val.nData;
    this->nDataS = val.nDataS;
    this->nMemAddr = val.nMemAddr;
    this->nSampTyp = val.nSampTyp;
    this->Time = val.Time;
    this->nOffset = val.nOffset;
  };
} TRC_DATAS;

#pragma pack()

#pragma pack(1)
typedef struct trcEntry_t
{
  UINT8   nType;

  UINT8   bTS    : 1;        // Timestamp exists (Cycles & Time)
  UINT8   bPC    : 1;        // PC exists (ITM Data R/W), Is In Function Call (ETM)
  UINT8   bAddr  : 1;        // nAddr field exists
  UINT8   nDataS : 2;        /* Data Size (ITM) (0:None, 1:8-bit, 2:16-bit, 3:32-bit)
                                Function Info (ETM) (0: not in function, 1:Function Entry, 2:In Function, 3:Function Break)*/
  UINT8   bExec  : 1;        /* ETM: Execution Flag
                                ITM - Data Access: nPC holds exact data access address. */
  UINT8   bTD    : 1;        // Timestamp delayed Flag
  UINT8   bOvf   : 1;        // Overflow Flag (ITM)

  UINT32  nPC;
  UINT32  nAddr;
  UINT32  nData;
  UINT64  nCycle;
  UINT64  nEntryNum;

  trcEntry_t() :
  nType(0),
  bExec(0),
  bOvf(0),
  bPC(0),
  nDataS(0),
  bTS(0),
  bTD(0),
  nPC(0),
  nAddr(0),
  nData(0),
  nCycle(0),
  nEntryNum(-1) {};

  void operator=(const trcEntry_t& value) {
    nType     = value.nType;
    bExec     = value.bExec;
    bOvf      = value.bOvf;
    bPC       = value.bPC;
    nDataS    = value.nDataS;
    bTS       = value.bTS;
    bTD       = value.bTD;
    nPC       = value.nPC;
    nAddr     = value.nAddr;
    nData     = value.nData;
    nCycle    = value.nCycle;
    nEntryNum = value.nEntryNum;
  };

} TRC_ENTRY;
#pragma pack()

#endif //__TRACETYPES_H__
