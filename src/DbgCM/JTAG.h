/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.3
 * @date     $Date: 2016-10-18 16:42:09 +0200 (Tue, 18 Oct 2016) $
 *
 * @note
 * Copyright (C) 2009-2016 ARM Limited. All rights reserved.
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



#ifndef __JTAG_H__
#define __JTAG_H__

#include "COLLECT.H"

// Public JTAG Instructions
#define JTAG_EXTEST  0x00
#define JTAG_SCAN_N  0x02
#define JTAG_RESTART 0x04
#define JTAG_ABORT   0x08
#define JTAG_DPACC   0x0A
#define JTAG_APACC   0x0B
#define JTAG_INTEST  0x0C
#define JTAG_IDCODE  0x0E
#define JTAG_BYPASS  0x0F


#define NJDEVS 8

typedef struct {               // JTAG single device
  unsigned long          id;   // device ID
  unsigned char      ir_len;   // length of IR
  unsigned char      res[3];   // reserved
} J_IC;

typedef struct {               // JTAG Device List
  unsigned int          cnt;   // number of devices
  unsigned int       com_no;   // selected device for communication
  J_IC           ic[NJDEVS];   // device list
  char   icname[NJDEVS][30];   // device names (used for dialog)
  int        icinfo[NJDEVS];   // additional information about the device (used for dialog)
  char        icacc[NJDEVS];   // Accessed Devices List
} JDEVS;

extern JDEVS JTAG_devs;        // JTAG Device List


extern DWORD JTAG_IDCode;      // JTAG ID Code


// JTAG Reset
//   return value: error status
extern int JTAG_Reset (void);


// JTAG Detection of chained Devices
//   return value: error status
extern int JTAG_DetectDevices (void);


// JTAG Read ID Code
//   return value: error status
extern int JTAG_ReadID (void);


// JTAG Data/Access Port Abort
//   return value: error status
extern int JTAG_DAPAbort (void);


// JTAG Read DP Register
//   adr    : Address
//   val    : Pointer to Value
//   return value: error status
extern int JTAG_ReadDP (BYTE adr, DWORD *val);

// JTAG Write DP Register
//   adr    : Address
//   val    : Value
//   return value: error status
extern int JTAG_WriteDP (BYTE adr, DWORD val);

// JTAG Read AP Register
//   adr    : Address
//   val    : Pointer to Value
//   return value: error status
extern int JTAG_ReadAP (BYTE adr, DWORD *val);

// JTAG Write AP Register
//   adr    : Address
//   val    : Value
//   return value: error status
extern int JTAG_WriteAP (BYTE adr, DWORD val);


// JTAG Read 32-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int JTAG_ReadD32 (DWORD adr, DWORD *val, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_ReadD32 (DWORD adr, DWORD *val);
#endif // DBGCM_V8M

// JTAG Write 32-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int JTAG_WriteD32 (DWORD adr, DWORD val, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_WriteD32 (DWORD adr, DWORD val);
#endif // DBGCM_V8M

// JTAG Read 16-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int JTAG_ReadD16 (DWORD adr, WORD *val, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_ReadD16 (DWORD adr, WORD *val);
#endif // DBGCM_V8M

// JTAG Write 16-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int JTAG_WriteD16 (DWORD adr, WORD val, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_WriteD16 (DWORD adr, WORD val);
#endif // DBGCM_V8M

// JTAG Read 8-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int JTAG_ReadD8 (DWORD adr, BYTE *val, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_ReadD8 (DWORD adr, BYTE *val);
#endif // DBGCM_V8M

// JTAG Write 8-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int JTAG_WriteD8 (DWORD adr, BYTE val, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_WriteD8 (DWORD adr, BYTE val);
#endif // DBGCM_V8M

// JTAG Read Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute))
//   return value: error status
extern int JTAG_ReadBlock (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// JTAG Write Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute))
//   return value: error status
extern int JTAG_WriteBlock (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// JTAG Verify Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute))
//   return value: error status
#if DBGCM_V8M
extern int JTAG_VerifyBlock (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_VerifyBlock (DWORD adr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M


// JTAG Read ARM Memory
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute))
//   return value: error status
#if DBGCM_V8M
extern int JTAG_ReadARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_ReadARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M

// JTAG Write ARM Memory
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute))
//   return value: error status
#if DBGCM_V8M
extern int JTAG_WriteARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_WriteARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M

// JTAG Verify ARM Memory
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Verify
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute))
//   return value: error status
#if DBGCM_V8M
extern int JTAG_VerifyARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int JTAG_VerifyARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M


// JTAG Get ARM Registers
//   regs   : Pointer to ARM Registers
//   rfpu   : Pointer to FPU Registers
//   rsec   : Pointer to v8-M Security Extension Registers
//   mask   : Register Mask
//   return value: error status
#if DBGCM_V8M
extern int JTAG_GetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask);
#else // DBGCM_V8M
extern int JTAG_GetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, U64 mask);
#endif // DBGCM_V8M

// JTAG Set ARM Registers
//   regs   : Pointer to ARM Registers
//   rfpu   : Pointer to FPU Registers
//   rsec   : Pointer to v8-M Security Extension Registers
//   mask   : Register Mask
//   return value: error status
#if DBGCM_V8M
extern int JTAG_SetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask);
#else // DBGCM_V8M
extern int JTAG_SetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, U64 mask);
#endif // DBGCM_V8M


// JTAG Execute System Call
//   regs   : Pointer to ARM Registers
//   return value: error status
extern int JTAG_SysCallExec (RgARMCM *regs);

// JTAG Read System Call Result
//   rval   : Pointer to Result Value
//   return value: error status
extern int JTAG_SysCallRes (DWORD *rval);


// JTAG Init Debugger
//   return value: error status
extern int JTAG_DebugInit (void);


// JTAG Test Sizes Supported in AP CSW
//   return value: error status
extern int JTAG_TestSizesAP (void);


// JTAG Data/Access Port Abort with value to write
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_DAPAbortVal (DWORD val);


// JTAG Read Data Block (8-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_ReadBlockD8 (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// JTAG Read Data Block (16-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U16s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_ReadBlockD16 (DWORD adr, U16 *pB, DWORD nMany, BYTE attrib);

// JTAG Read Data Block (32-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U32s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_ReadBlockD32 (DWORD adr, U32 *pB, DWORD nMany, BYTE attrib);


// JTAG Write Data Block (8-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_WriteBlockD8 (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// JTAG Write Data Block (16-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U16s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_WriteBlockD16 (DWORD adr, U16 *pB, DWORD nMany, BYTE attrib);

// JTAG Write Data Block (32-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U32s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_WriteBlockD32 (DWORD adr, U32 *pB, DWORD nMany, BYTE attrib);


// JTAG Read ARM Memory (8-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_ReadARMMemD8 (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);

// JTAG Read ARM Memory (16-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_ReadARMMemD16 (DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib);

// JTAG Read ARM Memory (32-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_ReadARMMemD32 (DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib);

// JTAG Write ARM Memory (8-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_WriteARMMemD8 (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);

// JTAG Write ARM Memory (16-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_WriteARMMemD16 (DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib);

// JTAG Write ARM Memory (32-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_WriteARMMemD32 (DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib);


// JTAG Init Debugger
//   return value: error status
extern int JTAG_DebugInit (void);


// JTAG Configure SWJ Debug Protocol
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int JSW_ConfigureProtocol(int retry);

// JTAG Read Device List Target
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int JTAG_GetDeviceList(JDEVS *DevList, unsigned int maxdevs, bool merge);

// Get Device Names of ICs connected to JTAG chain
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int JTAG_GetDeviceNames(JDEVS *DevList, unsigned int maxdevs, bool merge);


// JTAG Power Up Debug Port
//   return  value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_DebugPortStart(void);

// JTAG Switch DP
//   id     : DP ID to switch to
//   force  : Force a DP switch even if it seems to be the same DP
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int JTAG_SwitchDP (DWORD id, bool force);


// JTAG Execute SWJ Sequence
//   cnt    : Length of sequence in bits (0 < cnt <= 64)
//   val    : TMS values, LSB first
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
//  - DBGCM_RECOVERY Feature
extern int JTAG_SWJ_Sequence (int cnt, U64 val);


// JTAG Execute TDI Sequence with fixed TMS
//   cnt    : Length of sequence in bits (0 < cnt <= 64)
//   tms    : TMS value for this sequence
//   tdi    : TDI values, LSB first
//   tdo    : Captured TDO values, LSB first
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int JTAG_Sequence (int cnt, int tms, U64 tdi, U64 *tdo);

// JTAG Set Debugger Clock
//   cid    : Debugger Specific Clock ID
//   rtck   : Use Return Clock (FALSE for Cortex-M)
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int JTAG_SWJ_Clock(BYTE cid, BOOL rtck);


// Clear descriptions of ICs connected to JTAG chain
extern int ClearDeviceList (JDEVS *DevList);


typedef enum {
  NOCPU = 0,      // No valid CPU
  ARMCSDP,        // ARM CoreSight Debug Port (ARM Cortex)
  //--TODO: Extend as required
} CPUINFO;



struct KNOWNDEVICES {
  DWORD   id;            // JTAG ID
  DWORD   idmask;        // mask for JTAG ID to support different chip revisions
  CPUINFO CpuType;       // number for CPU type
  char  * name;
};

extern struct KNOWNDEVICES KnownDevices[];
#endif
