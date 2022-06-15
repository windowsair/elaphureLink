/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.4
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


#ifndef __SWD_H__
#define __SWD_H__

#include "COLLECT.H"

extern DWORD SWD_IDCode;           // SWD ID Code


// SWJ Reset
//   return value: error status
extern int SWJ_Reset (void);


// SWJ Switch: SWD <-> JTAG
//   val    : Switch Code (16-bit)
//   return value: error status
extern int SWJ_Switch (WORD val);


// SWD Read ID Code
//   return value: error status
extern int SWD_ReadID (void);


// SWD Data/Access Port Abort
//   return value: error status
extern int SWD_DAPAbort (void);


// SWD Read DP Register
//   adr    : Address
//   val    : Pointer to Value
//   return value: error status
extern int SWD_ReadDP (BYTE adr, DWORD *val);

// SWD Write DP Register
//   adr    : Address
//   val    : Value
//   return value: error status
extern int SWD_WriteDP (BYTE adr, DWORD val);

// SWD Read AP Register
//   adr    : Address
//   val    : Pointer to Value
//   return value: error status
extern int SWD_ReadAP (BYTE adr, DWORD *val);

// SWD Write AP Register
//   adr    : Address
//   val    : Value
//   return value: error status
extern int SWD_WriteAP (BYTE adr, DWORD val);


// SWD Read 32-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_ReadD32 (DWORD adr, DWORD *val, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_ReadD32 (DWORD adr, DWORD *val);
#endif // DBGCM_V8M

// SWD Write 32-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_WriteD32 (DWORD adr, DWORD val, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_WriteD32 (DWORD adr, DWORD val);
#endif // DBGCM_V8M

// SWD Read 16-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_ReadD16 (DWORD adr, WORD *val, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_ReadD16 (DWORD adr, WORD *val);
#endif // DBGCM_V8M

// SWD Write 16-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_WriteD16 (DWORD adr, WORD val, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_WriteD16 (DWORD adr, WORD val);
#endif // DBGCM_V8M

// SWD Read 8-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_ReadD8 (DWORD adr, BYTE *val, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_ReadD8 (DWORD adr, BYTE *val);
#endif // DBGCM_V8M

// SWD Write 8-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_WriteD8 (DWORD adr, BYTE val, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_WriteD8 (DWORD adr, BYTE val);
#endif // DBGCM_V8M

// SWD Read Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
extern int SWD_ReadBlock (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// SWD Write Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
extern int SWD_WriteBlock (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// SWD Verify Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_VerifyBlock (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_VerifyBlock (DWORD adr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M


// SWD Read ARM Memory
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_ReadARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_ReadARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M

// SWD Write ARM Memory
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_WriteARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_WriteARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M

// SWD Verify ARM Memory
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Verify
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
extern int SWD_VerifyARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);
#else // DBGCM_V8M
extern int SWD_VerifyARMMem (DWORD *nAdr, BYTE *pB, DWORD nMany);
#endif // DBGCM_V8M


// SWD Get ARM Registers
//   regs   : Pointer to ARM Registers
//   rfpu   : Pointer to FPU Registers
//   rsec   : Pointer to v8-M Security Extension Registers
//   mask   : Register Mask
//   return value: error status
#if DBGCM_V8M
extern int SWD_GetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask);
#else // DBGCM_V8M
extern int SWD_GetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, U64 mask);
#endif // DBGCM_V8M

// SWD Set ARM Registers
//   regs   : Pointer to ARM Registers
//   rfpu   : Pointer to FPU Registers
//   rsec   : Pointer to v8-M Security Extension Registers
//   mask   : Register Mask
//   return value: error status
#if DBGCM_V8M
extern int SWD_SetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask);
#else // DBGCM_V8M
extern int SWD_SetARMRegs (RgARMCM *regs, RgARMFPU *rfpu, U64 mask);
#endif // DBGCM_V8M


// SWD Execute System Call
//   regs   : Pointer to ARM Registers
//   return value: error status
extern int SWD_SysCallExec (RgARMCM *regs);

// SWD Read System Call Result
//   rval   : Pointer to Result Value
//   return value: error status
extern int SWD_SysCallRes (DWORD *rval);


// SWD Init Debugger
//   return value: error status
extern int SWD_DebugInit (void);


// SWD Test Sizes Supported in AP CSW
//   return value: error status
extern int SWD_TestSizesAP (void);


// SWD Data/Access Port Abort with value to write
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_DAPAbortVal (DWORD val);


// SWD Read Data Block (8-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_ReadBlockD8 (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);

// SWD Read Data Block (16-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U16s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_ReadBlockD16 (DWORD adr, U16 *pB, DWORD nMany, BYTE attrib);

// SWD Read Data Block (32-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U32s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_ReadBlockD32 (DWORD adr, U32 *pB, DWORD nMany, BYTE attrib);


// SWD Write Data Block (8-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_WriteBlockD8 (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);


// SWD Write Data Block (16-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U16s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_WriteBlockD16 (DWORD adr, U16 *pB, DWORD nMany, BYTE attrib);


// SWD Write Data Block (32-bit Elements inside 4kB Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of U32s
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_WriteBlockD32 (DWORD adr, U32 *pB, DWORD nMany, BYTE attrib);


// SWD Read ARM Memory (8-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_ReadARMMemD8 (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);


// SWD Read ARM Memory (16-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_ReadARMMemD16 (DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib);


// SWD Read ARM Memory (32-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_ReadARMMemD32 (DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib);


// SWD Write ARM Memory (8-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_WriteARMMemD8 (DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);


// SWD Write ARM Memory (16-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_WriteARMMemD16 (DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib);


// SWD Write ARM Memory (32-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_WriteARMMemD32 (DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib);


// SWD Configure SWJ Debug Protocol
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int SWJ_ConfigureProtocol(int retry);


// SWD Read Device List Target
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int SWD_GetDeviceList(JDEVS *DevList, unsigned int maxdevs, bool merge);


// Get Device Names of ICs connected to the SW-DP
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int SWD_GetDeviceNames (JDEVS *DevList, unsigned int maxdevs, bool merge);


// SWD Power Up Debug Port
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int SWD_DebugPortStart(void);


// SWD Switch DP
//   pdscId : DP ID to switch to
//   force  : Force a DP switch even if it seems to be the same DP
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_SwitchDP (DWORD id, bool force);


// SWD Power Up Debug Port
//   return  value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
extern int SWD_DebugPortStart();


// SWD Execute SWJ Sequence
//   cnt    : Length of sequence in bits (0 < cnt <= 64)
//   val    : SWDIO values, LSB first
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
//  - DBGCM_RECOVERY Feature
extern int SWD_SWJ_Sequence (int cnt, U64 val);


// SWD Set Debugger Clock
//   cid    : Debugger Specific Clock ID
//   rtck   : Use Return Clock (FALSE for Cortex-M)
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
extern int SWD_SWJ_Clock(BYTE cid, BOOL rtck);

#endif

