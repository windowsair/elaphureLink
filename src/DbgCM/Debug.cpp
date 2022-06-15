/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.19
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
 *
 * @brief     Initializes the Debug Session, reads CPU Basics and sets up Function Pointers for JTAG / SWD
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
#include "..\AGDI.h"
#include "Collect.h"
#include "Debug.h"
#include "Trace.h"
#include "ETB.h"
#include "CSTF.h"   // CoreSight Trace Funnel
#include "CTI.h"    // CoreSight Cross Trigger Interface
#include "JTAG.h"
#include "SWD.h"
#include "..\BOM.H"


// Debug Variables
BOOL  DbgInit = FALSE;             // Debug Initialization

DWORD DP_Type = 0;                 // Debug Port Type
BYTE  DP_Ver  = 0;                 // Debug Port Version (V0, V1, V2)
BOOL  DP_Min  = FALSE;             // Minimal DP (without Pushed Verify/Compare,
                                   //                     Transaction Counter)
DWORD AP_Sel  = 0x00000000;        // Current AP
BYTE  AP_Bank = 0x00;              // Current AP Bank
BOOL  AP_PT       = TRUE;          // AP Packed Transfer Support in HW
BYTE  AP_AccSizes = AP_ACCSZ_WORD; // AP Supported Access Sizes
// 02.07.2019: Deprecated CSW_Val - Use AP_CONTEXT::CSW_Val_Base mechanism instead (see "Usage of AP_Context" in JTAG.CPP/SWD.CPP)
DWORD CSW_Val = CSW_RESERVED   |   // Current CSW Value, use to store latest AP CSW value of currently selected AP
                CSW_MSTRDBG    |
                CSW_HPROT_PRIV |
                CSW_DBGSTAT    |
                CSW_SADDRINC   |
                CSW_SIZE32;

BYTE  NumBP;                       // Number of HW Breakpoints
BYTE  NumWP;                       // Number of HW Watchpoints
BYTE  NTrWP;                       // Number of Trace Watchpoints
BYTE  MTrWP;                       // Mask of Trace Watchpoints used
BYTE  MDtWP;                       // Mask of Data Watchpoints used
BYTE  AM_WP = 0x0F;                // Address Mask Max Bit Count for Watchpoints

// 27.01.2020: Tracepoint support
BYTE  NTrRP;                       // Number of Run  TracePoints (input to ETM)
BYTE  NTrSP;                       // Number of Stop TracePoints (input to ETM)
BYTE  NTrHP;                       // Number of Halt TracePoints (input to ETM)
BYTE  NTrDP;                       // Number of Data TracePoints (input to ETM)  // Separate from LA Points

BYTE  MTrRP;                       // Mask of Run  TracePoints (input to ETM)
BYTE  MTrSP;                       // Mask of Stop TracePoints (input to ETM)
BYTE  MTrHP;                       // Mask of Halt TracePoints (input to ETM)
BYTE  MTrDP;                       // Mask of Data TracePoints (input to ETM)    // Separate from LA Points

BYTE  FPB_Ver       = 0;           // FPB Version (0-Cortex-M0/1/3/4, 1-Cortex-M7)
DWORD FPB_CompMask  = FPB_COMP_M;  // Mask for comparator address value
BOOL  DWT_ITM_F_R_W = FALSE;       // Separate DWT ITM Functions for Data R/W

int   NumIRQ;                      // Number of External IRQ

DWORD RWPage = 0x1000;             // R/W Page (Auto Increment)

DWORD SWBKPT32 = 0xE1200070;       // SW Breakpoint: ARM Instruction BKPT 0
WORD  SWBKPT16 = 0xBE00;           // SW Breakpoint: Thumb Instruction BKPT 0

DWORD DHCSR_MaskIntsStop = 0;      // Mask Interrupts on stop (0 - don't mask, C_MASKINTS - do mask)
WORD  SWJ_SwitchSeq;               // Succeeding Switch Sequence (for later recovery)
RgFPB RegFPB;                      // FPB Registers
RgDWT RegDWT;                      // DWT Registers


// Debug Block Addresses
DWORD DBG_Addr  = 0;               // Core Debug Base Address
DWORD DWT_Addr  = 0;               // DWT Base Address
DWORD FPB_Addr  = 0;               // FPB Base Address
DWORD ITM_Addr  = 0;               // ITM Base Address
//DWORD TPIU_Addr = 0;               // TPIU Base Address
DWORD ETM_Addr  = 0;               // ETM Base Address
DWORD NVIC_Addr = 0;               // NVIC Base Address
DWORD MTB_Addr  = 0;               // MTB Base Address
// CS Location (Potentially Shared in Multi-Core System)
CS_LOCATION TPIU_Location = { 0, 0, 0 }; // TPIU Base Location

// System Block Addresses
DWORD Cache_Addr = 0;              // Cache Base Address (i.e. Cortex-M35P I-Cache present in ROM Table)

// Debug Block Versions
BOOL  MTB_CM0P    = FALSE;         // MTB Cortex-M0+
DWORD TPIU_Type   = TPIU_TYPE_CM;  // Trace Interface Type (TPIU_TYPE_CM, TPIU_TYPE_CS, TPIU_TYPE_SWO)
DWORD ETM_Version = 3;             // ETM Version (can be 3 or 4)

// 02.07.2019: Deprecated CSW_Val - Use AP_CONTEXT::CSW_Val_Base mechanism instead (see "Usage of AP_Context" in JTAG.CPP/SWD.CPP)
DWORD CSW_Val_Base = CSW_RESERVED   | // Basic CSW Value, bits to use for MEM-AP accesses except from access size and address increment setting
                     CSW_MSTRDBG    |
                     CSW_HPROT_PRIV |
                     CSW_DBGSTAT;

BOOL  DSCSR_Has_CDSKEY = FALSE;    // DSCSR has CDSKEY Bit (Security Extensions only)

AP_CONTEXT AP_Context[DP_NUM][AP_NUM];
BOOL  MemAccX_AP = FALSE;          // Extended Memory Access with explicit AP in progress

// Debug Functions
int (*DAPAbort)    (void);                                    // DAP Abort
int (*ReadDP)      (BYTE    adr, DWORD *val);                 // Read DP Register
int (*WriteDP)     (BYTE    adr, DWORD  val);                 // Write DP Register
int (*ReadAP)      (BYTE    adr, DWORD *val);                 // Read AP Register
int (*WriteAP)     (BYTE    adr, DWORD  val);                 // Write AP Register
#if DBGCM_V8M
int (*ReadD32)     (DWORD   adr, DWORD *val, BYTE attrib);    // Read 32-bit Data
int (*WriteD32)    (DWORD   adr, DWORD  val, BYTE attrib);    // Write 32-bit Data
int (*ReadD16)     (DWORD   adr, WORD  *val, BYTE attrib);    // Read 16-bit Data
int (*WriteD16)    (DWORD   adr, WORD   val, BYTE attrib);    // Write 16-bit Data
int (*ReadD8)      (DWORD   adr, BYTE  *val, BYTE attrib);    // Read 8-bit Data
int (*WriteD8)     (DWORD   adr, BYTE   val, BYTE attrib);    // Write 8-bit Data
int (*ReadBlock)   (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Read Data Block
int (*WriteBlock)  (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Write Data Block
int (*VerifyBlock) (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Verify Data Block
int (*ReadARMMem)  (DWORD *nAdr, BYTE  *pB, DWORD nMany, BYTE attrib); // Read ARM Memory
int (*WriteARMMem) (DWORD *nAdr, BYTE  *pB, DWORD nMany, BYTE attrib); // Write ARM Memory
int (*VerifyARMMem)(DWORD *nAdr, BYTE  *pB, DWORD nMany, BYTE attrib); // Verify ARM Memory
int (*GetARMRegs)  (RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask); // Get ARM Registers
int (*SetARMRegs)  (RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask); // Set ARM Registers
#else // DBGCM_V8M
int (*ReadD32)     (DWORD   adr, DWORD *val);                 // Read 32-bit Data
int (*WriteD32)    (DWORD   adr, DWORD  val);                 // Write 32-bit Data
int (*ReadD16)     (DWORD   adr, WORD  *val);                 // Read 16-bit Data
int (*WriteD16)    (DWORD   adr, WORD   val);                 // Write 16-bit Data
int (*ReadD8)      (DWORD   adr, BYTE  *val);                 // Read 8-bit Data
int (*WriteD8)     (DWORD   adr, BYTE   val);                 // Write 8-bit Data
int (*ReadBlock)   (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Read Data Block
int (*WriteBlock)  (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Write Data Block
int (*VerifyBlock) (DWORD   adr, BYTE  *pB, DWORD nMany);     // Verify Data Block
int (*ReadARMMem)  (DWORD *nAdr, BYTE  *pB, DWORD nMany);     // Read ARM Memory
int (*WriteARMMem) (DWORD *nAdr, BYTE  *pB, DWORD nMany);     // Write ARM Memory
int (*VerifyARMMem)(DWORD *nAdr, BYTE  *pB, DWORD nMany);     // Verify ARM Memory
int (*GetARMRegs)  (RgARMCM *regs, RgARMFPU *rfpu, U64 mask); // Get ARM Registers
int (*SetARMRegs)  (RgARMCM *regs, RgARMFPU *rfpu, U64 mask); // Set ARM Registers
#endif // DBGCM_V8M
int (*SysCallExec) (RgARMCM *regs);                           // System Call Execute
int (*SysCallRes)  (DWORD *rval);                             // System Call Result
int (*TestSizesAP)   (void);                                             // Test Sizes Supported in AP CSW
int (*DAPAbortVal)   (DWORD val);                                        // DAP Abort with value to write
int (*ReadBlockD8)   (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Read Data Block of nMany 8-bit accesses
int (*ReadBlockD16)  (DWORD   adr, U16   *pB, DWORD nMany, BYTE attrib); // Read Data Block of nMany 16-bit accesses
int (*ReadBlockD32)  (DWORD   adr, U32   *pB, DWORD nMany, BYTE attrib); // Read Data Block of nMany 32-bit accesses
int (*WriteBlockD8)  (DWORD   adr, BYTE  *pB, DWORD nMany, BYTE attrib); // Write Data Block of nMany 8-bit accesses
int (*WriteBlockD16) (DWORD   adr, U16   *pB, DWORD nMany, BYTE attrib); // Write Data Block of nMany 16-bit accesses
int (*WriteBlockD32) (DWORD   adr, U32   *pB, DWORD nMany, BYTE attrib); // Write Data Block of nMany 32-bit accesses
int (*ReadARMMemD8)  (DWORD *nAdr, BYTE  *pB, DWORD nMany, BYTE attrib); // Read ARM Memory (nMany 8-bit accesses)
int (*ReadARMMemD16) (DWORD *nAdr, U16   *pB, DWORD nMany, BYTE attrib); // Read ARM Memory (nMany 16-bit accesses)
int (*ReadARMMemD32) (DWORD *nAdr, U32   *pB, DWORD nMany, BYTE attrib); // Read ARM Memory (nMany 32-bit accesses)
int (*WriteARMMemD8) (DWORD *nAdr, BYTE  *pB, DWORD nMany, BYTE attrib); // Write ARM Memory (nMany 8-bit accesses)
int (*WriteARMMemD16)(DWORD *nAdr, U16   *pB, DWORD nMany, BYTE attrib); // Write ARM Memory (nMany 16-bit accesses)
int (*WriteARMMemD32)(DWORD *nAdr, U32   *pB, DWORD nMany, BYTE attrib); // Write ARM Memory (nMany 32-bit accesses)
int (*SwitchDP)      (DWORD   id, bool force);                           // Switch DP
int (*SWJ_Sequence)  (int cnt, U64 val);                                 // Execute SWJ (SWDIO_TMS) Sequence
int (*SWJ_Clock)     (BYTE cid, BOOL rtck);


// Big Endian 16-bit Swap
//   D0..D7,D8..D15 <-> D8..D15,D0..D7
WORD Swap16 (WORD v) {
  __asm   {
    mov    ax,v
    xchg   ah,al
    mov    v,ax
  }
  return (v);
}

// 32-bit Big Endian Swap
//   D0..D7,D8..D15,D16..D23,D24..D31 <-> D24..D31,D16..D23,D8..D15,D0..D7
DWORD Swap32 (DWORD v) {
  __asm   {
    mov    eax,v
    bswap  eax
    mov    v,eax
  }
  return (v);
}


// v8-M
// Ranges exempt from checking security violation:
//  - 0xE0000000 - 0xE0002FFF : ITM, DWT, FPB.
//  - 0xE000E000 - 0xE000EFFF : SCS range.           (<-- Partly Banked)
//  - 0xE002E000 - 0xE002EFFF : SCS NS alias range.  (<-- RAZ/WI for debugger)
//  - 0xE0040000 - 0xE0041FFF : TPIU, ETM.
//  - 0xE00FF000 - 0xE00FFFFF : ROM table.
//  - 0xE0000000 - 0xEFFFFFFF : for instruction fetch only.
//  - Additional address ranges specified by the IDAU.


// Address in v8-M SCS Register Range
//    4K-Block starting at SCS Base Address (NVIC_Addr variable)
BYTE OverlapSCSv8M(DWORD adr, DWORD many) {
  if ((adr >= NVIC_Addr) && (adr < NVIC_Addr + 0x1000)) {
    return (1);
  }
  if ((adr + many - 1 >= NVIC_Addr) && (adr + many - 1 < NVIC_Addr + 0x1000)) {
    return (1);
  }
  if (adr < NVIC_Addr && adr + many - 1 > NVIC_Addr + 0x1000) {
    return (1);
  }
  return (0);
}


// Process ROM Table
//   Search through ROM table hierarchy for CoreSight Cortex-M Debug/Trace
//   components like SCS (with NVIC), DWT/DWU, FPB/BPU, ITM, TPIU, ETM.
//   Check CID (Component ID) and PID (Peripheral ID) to identify Cortex-M
//   components based on ARM JEP106 Identity Code and Part Number (in PID).
//   CID is checked only to identify ROM tables.

// Component ID (CID)
#define CID_ROMTAB  0xB105100D              // CID: ROM Table

// Peripheral ID (PID)
#define PID_MASKPN  0x0000000000000FFFULL   // PID Mask: PN
#define PID_MASK4KB 0x000000F000000000ULL   // PID Mask: 4KB Count
#define PID_MASKJEP 0x0000000F000FF000ULL   // PID Mask: JEP106
#define PID_ARMJEP  0x00000004000BB000ULL   // PID ARM JEP106 Code

// ARM Part Numbers (PN)
#define PN_SCS_7M   0x000                   // PN: SCS ARMv7-M
#define PN_ITM_7M   0x001                   // PN: ITM ARMv7-M
#define PN_DWT_7M   0x002                   // PN: DWT ARMv7-M
#define PN_FPB_7M   0x003                   // PN: FPB ARMv7-M
#define PN_SCS_7MS  0x004                   // PN: SCS ARMv7-M Secure
#define PN_ITM_7MS  0x005                   // PN: ITM ARMv7-M Secure
#define PN_DWT_7MS  0x006                   // PN: DWT ARMv7-M Secure
#define PN_FPB_7MS  0x007                   // PN: FPB ARMv7-M Secure
#define PN_SCS_6M   0x008                   // PN: SCS ARMv6-M
#define PN_DWT_6M   0x00A                   // PN: DWT ARMv6-M
#define PN_BPU_6M   0x00B                   // PN: BPU ARMv6-M
#define PN_SCS_7MF  0x00C                   // PN: SCS ARMv7-M with FP
#define PN_SCS_6MS  0x00D                   // PN: SCS ARMv6-M Secure
#define PN_BPU_7M   0x00E                   // PN: BPU ARMv7-M (New Programmers Model)
#define PN_TS_GEN   0x101                   // PN: Prime Cell Time Stamp Generator
#define PN_ROM_CM1  0x470                   // PN: ROM Table Cortex-M1
#define PN_ROM_CM0  0x471                   // PN: ROM Table Cortex-M0
#define PN_ROM_CM0P 0x4C0                   // PN: ROM Table Cortex-M0+
#define PN_ROM_CM3  0x4C3                   // PN: ROM Table Cortex-M3 post r2p0
#define PN_ROM_CM4  0x4C4                   // PN: ROM Table Cortex-M4
#define PN_ROM_CM7  0x4C7                   // PN: ROM Table Cortex-M7
#define PN_ROM_CM33 0x4C9                   // PN: ROM Table Cortex-M33
#define PN_ROM_CM23 0x4CB                   // PN: ROM Table Cortex-M23
#define PN_ROM_CM35P 0x4D0                  // PN: ROM Table Cortex-M35P
#define PN_MPS      0x740                   // PN: Microcontroller Prototyping System (MPS)
#define PN_ICACHE_CM35P 0x880               // PN: Cortex-M35P I-Cache
#define PN_CTI      0x906                   // PN: Cross Trigger Interface CoreSight
#define PN_ETB      0x907                   // PN: ETB CoreSight
#define PN_TF       0x908                   // PN: Trace Funnel CoreSight
#define PN_TPIU     0x912                   // PN: TPIU CoreSight
#define PN_ITM      0x913                   // PN: ITM CoreSight
#define PN_SWO      0x914                   // PN: CoreSight SWO
#define PN_TPIU_CM3 0x923                   // PN: TPIU Cortex-M3
#define PN_ETM_CM3  0x924                   // PN: ETM Cortex-M3
#define PN_ETM_CM4  0x925                   // PN: ETM Cortex-M4
#define PN_ETM_SC3  0x926                   // PN: ETM SC300
#define PN_TPIU_SC3 0x927                   // PN: TPIU SC300
#define PN_MTB_CM0P 0x932                   // PN: MTB Cortex-M0+
#define PN_TPIU_L   0x941                   // PN: TPIU-Lite CoreSight
#define PN_TMC      0x961                   // PN: CoreSight Trace Memory Controller (TMC)
#define PN_ETM_CM7  0x975                   // PN: ETM Cortex-M7
#define PN_TPIU_CM4 0x9A1                   // PN: TPIU Cortex-M4/Cortex-M7 (early RTL revisions)
#define PN_MTB_CM0  0x9A3                   // PN: MTB Cortex-M0
#define PN_CTI_CM0P 0x9A6                   // PN: Cortex-M0+ CTI
#define PN_TPIU_CM7 0x9A9                   // PN: TPIU Cortex-M4/Cortex-M7 (since r0p0-03bet0 RTL)
#define PN_CPU_CM23 0xD20                   // PN: CPU Cortex-M23
#define PN_CPU_CM33 0xD21                   // PN: CPU Cortex-M33
#define PN_CPU_CM35P 0xD31                  // PN: CPU Cortex-M35P

// PN Designer (AGDI Internal 4 Bit ID)
#define PN_DESIGNER_ARM     0x0


// v8-M requires one or both of CoreSight Device Architecture and
// Device Type registers to be non-zero. Updated identification
// scheme for CoreSight components is:
//   - PN: CPU specific PN.
//   - DEVARCH/DEVTYPE: CoreSight component identification.

// Device Architecture (DEVARCH)
#define DEVARCH_ARCHITECT_M   0xFFE00000    // Architect ID (Bits[31..28]: JEP106 Continuation Code,
                                            //               Bits[27..21]: LSBs of JEP106 ID Code,
                                            //               Bit 20      : PRESENT Bit)
#define DEVARCH_PRESENT_M     0x00100000    // DEVARCH Register Present
#define DEVARCH_REVISION_M    0x000F0000    // Architecture Revision
#define DEVARCH_ARCHID_M      0x0000FFFF    // Component Architecture ID Mask
#define DEVARCH_ARCHVER_M     0x0000F000    // Component Architecture Version Mask
#define DEVARCH_ARCHVER_P     12            // Component Architecture Version Position
#define DEVARCH_ARCHPART_M    0x00000FFF    // Component Architecture Part Number Mask
#define DEVARCH_ID_ARM        0x47600000    // ARM Architect ID

// Device Type (DEVTYPE)
#define DEVTYPE_MASKSUB       0x000000F0    // Major Device Type
#define DEVTYPE_MASKMAJOR     0x0000000F    // Sub Device Type
#define DEVTYPE_TYPE          0x000000FF    // Full Type

// Architecture Part Numbers
#define ARCHPART_ITM          0xA01         // ARCHID: ITM (v8-M)
#define ARCHPART_DWT          0xA02         // ARCHID: DWT (v8-M)
#define ARCHPART_FPB          0xA03         // ARCHID: FPB (v8-M)
#define ARCHPART_SCS          0xA04         // ARCHID: SCS (v8-M)
#define ARCHPART_ETM          0xA13         // ARCHID: ETM (v8-M)
#define ARCHPART_CTI          0xA14         // ARCHID: CTI (v8-M)
#define ARCHPART_MTB          0xA31         // ARCHID: MTB (v8-M)
#define ARCHPART_ROM          0xAA5         // ARCHID: ROM Table (v8-M)

#define ARCHPART_TPIU         0x000         // ARCHID: TPIU (not assigned)

// Device Types (DEVTYPE)   0xSUB|MAJOR
#define DEVTYPE_MISC_OTHER    0x00          // DEVTYPE: Miscellaneous, other (v8-M DWT, FPB, SCS, requires DEVARCH?)
#define DEVTYPE_TPIU          0x11          // DEVTYPE: Trace Sink, Trace Port (v8-M TPIU, work out SWO via other regs??)
#define DEVTYPE_MTB           0x31          // DEVTYPE: Basic Trace Router
#define DEVTYPE_ETM           0x13          // DEVTYPE: Trace Source, processor core
#define DEVTYPE_ITM           0x43          // DEVTYPE: Trace Source, stimulus derived from bus activity (v8-M ITM)
#define DEVTYPE_CTI           0x14          // DEVTYPE: Debug Control, trigger matrix

// Read CID
static int ReadCID (DWORD *cid, DWORD adr, DWORD r32) {
  BYTE  buf[16];
  DWORD n;
  int   status;

#if DBGCM_V8M
  if (r32) {
    status = ReadBlock(adr, buf, 4*4, BLOCK_SECTYPE_ANY);
    if (status) return (status);
  } else {
    for (n = 0; n < 16; n += 4) {
      status = ReadD8(adr + n, &buf[n], BLOCK_SECTYPE_ANY);
      if (status) return (status);
    }
  }
#else // DBGCM_V8M
  if (r32) {
    status = ReadBlock(adr, buf, 4*4, 0 /*attrib*/);
    if (status) return (status);
  } else {
    for (n = 0; n < 16; n += 4) {
      status = ReadD8(adr + n, &buf[n]);
      if (status) return (status);
    }
  }
#endif // DBGCM_V8M

  *cid = buf[0] | (buf[4] << 8) | (buf[8] << 16) | (buf[12] << 24);

  return (0);
}

// Read PID
static int ReadPID (UINT64 *pid, DWORD adr, DWORD r32) {
  BYTE  buf[32];
  DWORD n;
  int   status;

#if DBGCM_V8M
  if (r32) {
    status = ReadBlock(adr, buf, 8*4, BLOCK_SECTYPE_ANY);
    if (status) return (status);
  } else {
    for (n = 0; n < 32; n += 4) {
      status = ReadD8(adr + n, &buf[n], BLOCK_SECTYPE_ANY);
      if (status) return (status);
    }
  }
#else // DBGCM_V8M
  if (r32) {
    status = ReadBlock(adr, buf, 8*4, 0 /*attrib*/);
    if (status) return (status);
  } else {
    for (n = 0; n < 32; n += 4) {
      status = ReadD8(adr + n, &buf[n]);
      if (status) return (status);
    }
  }
#endif // DBGCM_V8M

  *pid = (((UINT64)(buf [0] | (buf [4] << 8) | (buf [8] << 16) | (buf[12] << 24))) << 32) |
                    buf[16] | (buf[20] << 8) | (buf[24] << 16) | (buf[28] << 24);
  return (0);
}

static int    level = 0;
static DWORD  rompn = 0;

// Process ROM Table
int ROM_Table (DWORD ptr) {
  int    status;
  DWORD  adr, val, cnt, n, type;
  DWORD  cid;
  UINT64 pid;
  DWORD  r32;
  BYTE   buf[32];
  DWORD  dev;
  DWORD  cpupn = 0;

  if (level == 0) {                     // Debug Base Address Register
    // Check legacy formats
    if (ptr == 0xFFFFFFFF) {
      return (EU12);                    // Invalid ROM Table (No debug entries)
    }
    if ((ptr & 0x00000002) == 0) {      // Check ARM Debug Interface V5 format
      ptr |= 0x00000003;                // Force ARM Debug Interface V5 format
    }
  }

  if (level == 10) return (EU12);       // Invalid ROM Table (Max 10 levels are scanned)

  if (ptr & 0x00000001) {               // Entry exists

    r32 = ptr & 0x00000002;
    adr = ptr & 0xFFFFF000;

    // Read CID's
    status = ReadCID(&cid, adr + 0xFF0, r32);
    if (status) return (status);

    // Read PID's
    status = ReadPID(&pid, adr + 0xFD0, r32);
    if (status) return (status);

    if (cid == CID_ROMTAB) {            // ROM Table

      switch (pid & (PID_MASKJEP | PID_MASKPN)) {
        case (PID_ARMJEP | PN_ROM_CM0P):
        case (PID_ARMJEP | PN_ROM_CM0):
        case (PID_ARMJEP | PN_ROM_CM1):
        case (PID_ARMJEP | PN_ROM_CM3):
        case (PID_ARMJEP | PN_ROM_CM4):
        case (PID_ARMJEP | PN_ROM_CM7):
        case (PID_ARMJEP | PN_ROM_CM23):
        case (PID_ARMJEP | PN_ROM_CM33):
        case (PID_ARMJEP | PN_ROM_CM35P):
          rompn = pid & PID_MASKPN;     // Store ROM PN if known
          break;
        default:
          rompn = 0;                    // Unknown ROM PN
      }

      cnt = r32 ? 960 : 240;
      for (n = 0; n < cnt; n++) {

#if DBGCM_V8M
        if (r32) {
          status = ReadD32(adr + 4*n, &val, BLOCK_SECTYPE_ANY);
          if (status) return (status);
        } else {
          status = ReadD8 (adr + 16*n +  0, &buf[0], BLOCK_SECTYPE_ANY);
          if (status) return (status);
          status = ReadD8 (adr + 16*n +  4, &buf[1], BLOCK_SECTYPE_ANY);
          if (status) return (status);
          status = ReadD8 (adr + 16*n +  8, &buf[2], BLOCK_SECTYPE_ANY);
          if (status) return (status);
          status = ReadD8 (adr + 16*n + 12, &buf[3], BLOCK_SECTYPE_ANY);
          if (status) return (status);
          val = *((DWORD *)&buf[0]);
        }
        if (val == 0) return (0);       // End of Table Marker
        level++;
        status = ROM_Table(adr + val);
        level--;
        if (status) return (status);
#else // DBGCM_V8M
        if (r32) {
          status = ReadD32(adr + 4*n, &val);
          if (status) return (status);
        } else {
          status = ReadD8 (adr + 16*n +  0, &buf[0]);
          if (status) return (status);
          status = ReadD8 (adr + 16*n +  4, &buf[1]);
          if (status) return (status);
          status = ReadD8 (adr + 16*n +  8, &buf[2]);
          if (status) return (status);
          status = ReadD8 (adr + 16*n + 12, &buf[3]);
          if (status) return (status);
          val = *((DWORD *)&buf[0]);
        }
        if (val == 0) return (0);       // End of Table Marker
        level++;
        status = ROM_Table(adr + val);
        level--;
        if (status) return (status);
#endif // DBGCM_V8M

      }

    } else {                            // Component

      n = ((1 << ((pid >> 36) & 0x0F)) - 1) * 4096;

      switch (pid & (PID_MASKJEP | PID_MASKPN)) {

        case (PID_ARMJEP | PN_SCS_7M):
        case (PID_ARMJEP | PN_SCS_7MF):
        case (PID_ARMJEP | PN_SCS_7MS):
        case (PID_ARMJEP | PN_SCS_6M):
        case (PID_ARMJEP | PN_SCS_6MS):
          // SCS (with NVIC)
          NVIC_Addr = adr - n;
          DBG_Addr  = NVIC_Addr + DBG_OFS;

#if DBGCM_V8M
          status = WriteD32 (DBG_EMCR, TRCENA, BLOCK_SECTYPE_ANY);  // Enable (DWT, ITM, ETM, TPIU)
          if (status) return (status);
#else // DBGCM_V8M
          status = WriteD32 (DBG_EMCR, TRCENA);  // Enable (DWT, ITM, ETM, TPIU)
          if (status) return (status);
#endif // DBGCM_V8M

          break;

        case (PID_ARMJEP | PN_DWT_7M):
        case (PID_ARMJEP | PN_DWT_7MS):
        case (PID_ARMJEP | PN_DWT_6M):
          // DWT
          DWT_Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_FPB_7M):
        case (PID_ARMJEP | PN_FPB_7MS):
        case (PID_ARMJEP | PN_BPU_6M):
        case (PID_ARMJEP | PN_BPU_7M):
          // FPB/BPU
          FPB_Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_ITM_7M):
        case (PID_ARMJEP | PN_ITM_7MS):
          // ITM
          ITM_Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_TPIU_L):
        case (PID_ARMJEP | PN_TPIU):
          // Standard CoreSight TPIU (no SWO mode)
          if (TPIU_Location.Addr != 0x00000000) {                   // SDMDK-8638: Use first detected
            break;
          }
          if (TraceConf.Protocol == TPIU_TRACE_PORT) {
            if ((pid & PID_MASKPN) == PN_TPIU_L) {
              TPIU_Type = TPIU_TYPE_LITE;
            } else {
              TPIU_Type = TPIU_TYPE_CS;
            }
            TPIU_Location.AP   = AP_SEL2IDX(AP_Sel);
            TPIU_Location.Addr = adr - n;
          }
          break;

        case (PID_ARMJEP | PN_TPIU_CM3):
        case (PID_ARMJEP | PN_TPIU_CM4):
        case (PID_ARMJEP | PN_TPIU_SC3):
        case (PID_ARMJEP | PN_TPIU_CM7):
          // Cortex-M TPIU
          TPIU_Location.Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_ETM_CM3):
        case (PID_ARMJEP | PN_ETM_CM4):
        case (PID_ARMJEP | PN_ETM_SC3):
        case (PID_ARMJEP | PN_ETM_CM7):
          // ETM
          if ((pid & PID_MASKPN) == PN_ETM_CM7) {
            ETM_Version = 4;
          }
          ETM_Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_ETB):
          // ETB
          if (ETB_Location.Addr != 0x00000000) {                  // Use first detected
            break;
          }
          ETB_Location.AP   = AP_SEL2IDX(AP_Sel);
          ETB_Location.Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_TMC):
          // Trace Memory Controller
          if (ETB_Location.Addr != 0x00000000) {                  // Use first detected
            break;
          }

          ETB_Location.AP   = AP_SEL2IDX(AP_Sel);
          ETB_Location.Addr = adr - n;
#if DBGCM_V8M
          status   = ReadD32(TMC_DEVID, &val, BLOCK_SECTYPE_ANY);
          if (status) return (status);
#else // DBGCM_V8M
          status   = ReadD32(TMC_DEVID, &val);
          if (status) return (status);
#endif // DBGCM_V8M
          type = (val & TMC_CONFIGTYPE_MASK);
          if (type == TMC_CONFIGTYPE_ETB) {
            ETB_TMC  = TRUE;
          } else {
            ETB_Location.Addr = 0;
          }
          break;

        case (PID_ARMJEP | PN_TF):
          // Trace Funnel
          if (CSTF_Addr == 0) {
            CSTF_Addr = adr - n;
          } else {
            // More than one Trace Funnel in system, too complex to handle
            // need to do this via INI script
            CSTF_Single = FALSE;
            CSTF_Addr   = 0;
          }
          break;

        case (PID_ARMJEP | PN_MTB_CM0):
          // MTB Cortex-M0
          MTB_Addr = adr - n;
          break;

        case (PID_ARMJEP | PN_MTB_CM0P):
          // MTB Cortex-M0+
          MTB_Addr = adr - n;
          MTB_CM0P = TRUE;
          break;

        case (PID_ARMJEP | PN_CTI):
        case (PID_ARMJEP | PN_CTI_CM0P):
          // Cross Trigger Interface
          CTI_AddInstance(adr - n, AP_SEL2IDX(AP_Sel), JTAG_devs.com_no);
          break;

        case (PID_ARMJEP | PN_SWO):
          // SWO, reuse TPIU mechanisms (same programming interface)
          if (TraceConf.Protocol == TPIU_SWO_MANCHESTER || TraceConf.Protocol == TPIU_SWO_UART) {
            TPIU_Type = TPIU_TYPE_SWO;
            TPIU_Location.Addr = adr - n;
          }
          break;

        case (PID_ARMJEP | PN_CPU_CM33):
        case (PID_ARMJEP | PN_CPU_CM23):
        case (PID_ARMJEP | PN_CPU_CM35P):
          // CoreSight Component for specified CPU
          cpupn = pid & PID_MASKPN;
          break;

        case (PID_ARMJEP | PN_ICACHE_CM35P):
          // Cache Present
          Cache_Addr = adr - n;
          break;
      }

      if (cpupn) {

#if DBGCM_V8M
        // Check DEVARCH/DEVTYPE for details
        status = ReadD32(adr + 0xFBC, &dev, BLOCK_SECTYPE_ANY);
        if (status) return (status);
#else // DBGCM_V8M
        // Check DEVARCH/DEVTYPE for details
        status = ReadD32(adr + 0xFBC, &dev);
        if (status) return (status);
#endif // DBGCM_V8M

        if (dev & DEVARCH_PRESENT_M) {
          // Evaluate DEVARCH value
          switch (dev & (DEVARCH_ARCHITECT_M | DEVARCH_ARCHPART_M)) {
          case (DEVARCH_ID_ARM | ARCHPART_ITM):   // ITM (v8-M)
            ITM_Addr = adr - n;
            break;
          case (DEVARCH_ID_ARM | ARCHPART_DWT):   // DWT (v8-M)
            DWT_Addr = adr - n;
            break;
          case (DEVARCH_ID_ARM | ARCHPART_FPB):   // FPB (v8-M)
            FPB_Addr = adr - n;
            break;
          case (DEVARCH_ID_ARM | ARCHPART_SCS):   // SCS (v8-M)
            NVIC_Addr = adr - n;
            DBG_Addr  = NVIC_Addr + DBG_OFS;

#if DBGCM_V8M
            status = WriteD32 (DBG_EMCR, TRCENA, BLOCK_SECTYPE_ANY);  // Enable (DWT, ITM, ETM, TPIU)
            if (status) return (status);
#else // DBGCM_V8M
            status = WriteD32 (DBG_EMCR, TRCENA);  // Enable (DWT, ITM, ETM, TPIU)
            if (status) return (status);
#endif // DBGCM_V8M

            break;
          case (DEVARCH_ID_ARM | ARCHPART_ETM):   // ETM
            ETM_Version = (dev & DEVARCH_ARCHVER_M) >> DEVARCH_ARCHVER_P;
            if (ETM_Version == 2) {                // ETM v3 reports with ARCHVER 2
              ETM_Version++;
            }
            ETM_Addr = adr - n;
            break;
          default:
            // return (EU12);                // Invalid ROM Table
            break;                           // 27.02.2020: Don't abort because of unknown component
          }
        } else {

#if DBGCM_V8M
          status = ReadD32(adr + 0xFCC, &dev, BLOCK_SECTYPE_ANY);
          if (status) return (status);
#else // DBGCM_V8M
          status = ReadD32(adr + 0xFCC, &dev);
          if (status) return (status);
#endif // DBGCM_V8M

          if (dev) {
            // Evaluate DEVTYPE
            switch (dev & DEVTYPE_TYPE) {
            case DEVTYPE_MISC_OTHER:        // DEVTYPE: Miscellaneous, other (v8-M DWT, FPB, SCS, requires DEVARCH?)
              break;                        // Nothing to do (I hope, otherwise we need further detection mechanisms here)
            case DEVTYPE_TPIU:              // DEVTYPE: Trace Sink, Trace Port (v8-M TPIU, work out SWO via other regs??)
              TPIU_Location.Addr = adr - n;
              break;
            case DEVTYPE_ETM:               // DEVTYPE: Trace Source, processor core
              ETM_Version = 3;              // ETM without DEVARCH register => must be ETMv3
              ETM_Addr    = adr - n;        // TODO: Check if we can find out the version in anoter ETM register
              break;
            case DEVTYPE_ITM:               // DEVTYPE: Trace Source, stimulus derived from bus activity (v8-M ITM)
              ITM_Addr = adr - n;
              break;
            default:
              // return (EU12);             // Invalid ROM Table
              break;                        // 27.02.2020: Don't abort because of unknown component
            }
          }
          // 27.02.2020: Don't abort because of unknown component
          // else {
          //   return (EU12);               // Invalid ROM Table
          // }
        }
      }
    }
  }

  return (0);
}



// Debugger Initialization
//   return value: error status
int Debug_Init (void) {
  int   status;
  DWORD val;
  DWORD v0, v1;
  DWORD ticks;
  AP_CONTEXT *apCtx;

  MemAccX_AP = FALSE;

  // Variable Initialization
  DHCSR_MaskIntsStop = 0;

  switch (DP_Type) {
    case JTAG_DP:
      ReadDP       = JTAG_ReadDP;
      WriteDP      = JTAG_WriteDP;
      ReadAP       = JTAG_ReadAP;
      WriteAP      = JTAG_WriteAP;
      ReadD32      = JTAG_ReadD32;
      WriteD32     = JTAG_WriteD32;
      ReadD16      = JTAG_ReadD16;
      WriteD16     = JTAG_WriteD16;
      ReadD8       = JTAG_ReadD8;
      WriteD8      = JTAG_WriteD8;
      ReadBlock    = JTAG_ReadBlock;
      WriteBlock   = JTAG_WriteBlock;
      VerifyBlock  = JTAG_VerifyBlock;
      ReadARMMem   = JTAG_ReadARMMem;
      WriteARMMem  = JTAG_WriteARMMem;
      VerifyARMMem = JTAG_VerifyARMMem;
      GetARMRegs   = JTAG_GetARMRegs;
      SetARMRegs   = JTAG_SetARMRegs;
      SysCallExec  = JTAG_SysCallExec;
      SysCallRes   = JTAG_SysCallRes;
      TestSizesAP  = JTAG_TestSizesAP;
      DAPAbortVal    = JTAG_DAPAbortVal;
      ReadBlockD8    = JTAG_ReadBlockD8;
      ReadBlockD16   = JTAG_ReadBlockD16;
      ReadBlockD32   = JTAG_ReadBlockD32;
      WriteBlockD8   = JTAG_WriteBlockD8;
      WriteBlockD16  = JTAG_WriteBlockD16;
      WriteBlockD32  = JTAG_WriteBlockD32;
      ReadARMMemD8   = JTAG_ReadARMMemD8;
      ReadARMMemD16  = JTAG_ReadARMMemD16;
      ReadARMMemD32  = JTAG_ReadARMMemD32;
      WriteARMMemD8  = JTAG_WriteARMMemD8;
      WriteARMMemD16 = JTAG_WriteARMMemD16;
      WriteARMMemD32 = JTAG_WriteARMMemD32;
      SwitchDP       = JTAG_SwitchDP;
      SWJ_Sequence   = JTAG_SWJ_Sequence;
      SWJ_Clock      = JTAG_SWJ_Clock;
      status         = JTAG_DebugInit();
      break;
    case SW_DP:
      ReadDP       = SWD_ReadDP;
      WriteDP      = SWD_WriteDP;
      ReadAP       = SWD_ReadAP;
      WriteAP      = SWD_WriteAP;
      ReadD32      = SWD_ReadD32;
      WriteD32     = SWD_WriteD32;
      ReadD16      = SWD_ReadD16;
      WriteD16     = SWD_WriteD16;
      ReadD8       = SWD_ReadD8;
      WriteD8      = SWD_WriteD8;
      ReadBlock    = SWD_ReadBlock;
      WriteBlock   = SWD_WriteBlock;
      VerifyBlock  = SWD_VerifyBlock;
      ReadARMMem   = SWD_ReadARMMem;
      WriteARMMem  = SWD_WriteARMMem;
      VerifyARMMem = SWD_VerifyARMMem;
      GetARMRegs   = SWD_GetARMRegs;
      SetARMRegs   = SWD_SetARMRegs;
      SysCallExec  = SWD_SysCallExec;
      SysCallRes   = SWD_SysCallRes;
      TestSizesAP  = SWD_TestSizesAP;
      DAPAbortVal    = SWD_DAPAbortVal;
      ReadBlockD8    = SWD_ReadBlockD8;
      ReadBlockD16   = SWD_ReadBlockD16;
      ReadBlockD32   = SWD_ReadBlockD32;
      WriteBlockD8   = SWD_WriteBlockD8;
      WriteBlockD16  = SWD_WriteBlockD16;
      WriteBlockD32  = SWD_WriteBlockD32;
      ReadARMMemD8   = SWD_ReadARMMemD8;
      ReadARMMemD16  = SWD_ReadARMMemD16;
      ReadARMMemD32  = SWD_ReadARMMemD32;
      WriteARMMemD8  = SWD_WriteARMMemD8;
      WriteARMMemD16 = SWD_WriteARMMemD16;
      WriteARMMemD32 = SWD_WriteARMMemD32;
      SwitchDP       = SWD_SwitchDP;
      SWJ_Sequence   = SWD_SWJ_Sequence;
      SWJ_Clock      = SWD_SWJ_Clock;
      status       = SWD_DebugInit();
      break;
    default:
      return (EU01);
  }

  if (status) return (status);

  DbgInit = TRUE;

#if 0  // 27.06.2019: Changed AP handling
  // Determine supported AP bus access sizes
  status = TestSizesAP();
  if (status) return (status);

  CSW_Val = (CSW_Val_Base|CSW_SIZE32|CSW_SADDRINC);
  status = WriteAP(AP_CSW, CSW_Val);
  if (status) return (status);
#endif

  // Now properly init CSW
  status = AP_Switch(&apCtx);             // 08.11.2018: No need to make thread-safe, competing threads not started yet
  if (status) return (status);

  status = WriteAP(AP_CSW, (apCtx->CSW_Val_Base|CSW_SIZE32|CSW_SADDRINC));
  if (status) return (status);

  if (!(MonConf.Opt & INIT_RST_HOLD)) {     // 02.10.2019: Only for connections NOT under reset
    if (pCbFunc(AG_CB_CHECKDEVICE, NULL)) { // Genuine device check, called here after DAP is ready for accesses
      return (EU52);                        // Connection refused due to device mismatch!
    }
  }

  status = ReadAP (AP_ROM, &val);
  if (status) return (status);

  status = ROM_Table(val);
  if (status) return (status);

  if (DBG_Addr == 0) return (EU12);     // Invalid ROM Table

  xFPU = FALSE;

#if DBGCM_V8M
  status = ReadD32(NVIC_CPUID, &val, BLOCK_SECTYPE_ANY);
  if (status) return (status);
#else // DBGCM_V8M
  status = ReadD32(NVIC_CPUID, &val);
  if (status) return (status);
#endif // DBGCM_V8M

  switch (val & (CPUID_IMPL|CPUID_PARTNO)) {
      // Detect Core and set the following characteristics
      //  - Enforce usage/removal of trace features like TPIU formatter (based on their support)
      //  - AP Automatic Address Increment Page Size
      //  - FPU Support
      //  - Device/AP Specific Protection Bits for CSW Register
      //  - Address Mask Max Bit Count for Watchpoints (??)
    case (CPUID_IMPL_ARM|0xC330):
      xxCPU  = ARM_SC300;
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      break;
    case (CPUID_IMPL_ARM|0xC300):
      xxCPU  = ARM_SC000;
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|0xC270):
      xxCPU  = ARM_CM7;
      if ((MonConf.Opt & INIT_RST_HOLD) == 0) {
        // Cortex-M7 hardware reset keeps the core in reset for a while. Wait for
        // S_RESET_ST to clear on read (and hence for it to finish reset).
        ticks = GetTickCount();
        do {

#if DBGCM_V8M
          status = ReadD32(DBG_HCSR, &v0, BLOCK_SECTYPE_ANY);
          if (status) return (status);
          if ((v0 & S_RESET_ST) == 0) break;
#else // DBGCM_V8M
          status = ReadD32(DBG_HCSR, &v0);
          if (status) return (status);
          if ((v0 & S_RESET_ST) == 0) break;
#endif // DBGCM_V8M

        } while (GetTickCount() - ticks < 500);
        if (v0 & S_RESET_ST) return (EU13);       // Cannot enter Debug Mode
      }

#if DBGCM_V8M
      status = ReadD32(MVFR0, &v0, BLOCK_SECTYPE_ANY);
      if (status) return (status);
#else // DBGCM_V8M
      status = ReadD32(MVFR0, &v0);
      if (status) return (status);
#endif // DBGCM_V8M

      // Change this to only check the Single and Double Precision Support fields
      // of MVFR0. Values changed between Cortex-M7 FPGA revisions, so this should
      // more reliable.
      if (v0 & (MVFR0_SPF|MVFR0_DPF)) {
        // TODO: Distinction between SFPU and DFPU for register view
        xFPU = TRUE;
      }

      // Extend CSW value to used cacheable memory accesses
      CSW_Val |= CSW_HPROT_CACHE;
      CSW_Val_Base |= CSW_HPROT_CACHE;
      apCtx->CSW_Val_Base |= CSW_HPROT_CACHE;

      // DAP for current Cortex-M7 FPGA based on M0+-DAP, auto-increment boundary is 1K opposed to M4's 4K.
      // This may change if silicon vendors decide to use a full DAP (the FPGA uses a MinDP).
      RWPage = 0x0400;

      switch (val & (CPUID_VARIANT|CPUID_REVISION)) {
      case 0x00000000:  // r0p0
      case 0x00000001:  // r0p1
        // Also mask interrupts when stopping
        DHCSR_MaskIntsStop = C_MASKINTS;
        break;
      }

      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      DWT_ITM_F_R_W = TRUE;             // Separate DWT ITM Functions for Data R/W
      break;
    case (CPUID_IMPL_ARM|0xC240):
      xxCPU  = ARM_CM4;

#if DBGCM_V8M
      status = ReadD32(MVFR0, &v0, BLOCK_SECTYPE_ANY);
      if (status) return (status);
      status = ReadD32(MVFR1, &v1, BLOCK_SECTYPE_ANY);
      if (status) return (status);
#else // DBGCM_V8M
      status = ReadD32(MVFR0, &v0);
      if (status) return (status);
      status = ReadD32(MVFR1, &v1);
      if (status) return (status);
#endif // DBGCM_V8M

      if ((v0 == MVFR0_CM4_VAL) && (v1 == MVFR1_CM4_VAL)) {
        xFPU = TRUE;
      }
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      DWT_ITM_F_R_W = TRUE;             // Separate DWT ITM Functions for Data R/W
      break;
    case (CPUID_IMPL_ARM|0xC230):
      xxCPU  = ARM_CM3;
      if ((val & (CPUID_VARIANT | CPUID_REVISION)) == 0x00000001) {
        // r0p1 (0x410FC231)
        //  Enable TPIU Formatter (otherwise Trace Output doesn't work)
        TraceConf.Opt |=  TPIU_FORMAT;
      } else {
        // r0p0 (0x410FC230), r1p1 (0x411FC231), r2p0 (0x412FC230), r2p1 (0x412FC231)
        if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
          TraceConf.Opt |=  TPIU_FORMAT;  // Enable TPIU Formatter
        } else {
          TraceConf.Opt &= ~TPIU_FORMAT;  // Bypass TPIU Formatter
        }
      }
      if ((val & CPUID_VARIANT) >= (2 << 20)) {
        // r2p0 or higher
        DWT_ITM_F_R_W = TRUE;           // Separate DWT ITM Functions for Data R/W
      }
      break;

    case (CPUID_IMPL_ARM|0xC210):
      xxCPU  = ARM_CM1;
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|0xC200):
      xxCPU  = ARM_CM0;
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|0xC600):  // Cortex-M0+
      xxCPU  = ARM_CM0P;
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;

#if DBGCM_V8M
    case (CPUID_IMPL_ARM|CPUID_PN_ARM_CM23):    // Cortex-M23 (v8-M Baseline)
      xxCPU     = ARM_CM23;
      xMainline = FALSE;

      // 1 KByte Auto-Increment Page Size
      RWPage = 0x400;
      TraceConf.Opt &= TRACE_BASELINE_SUPP;  // Remove unsupported trace features from options
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }

      // No DWT with CYCCNT to sync/adjust
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|CPUID_PN_ARM_CM33):     // Cortex-M33 (v8-M Mainline)
      xxCPU     = ARM_CM33;
      xMainline = TRUE;

      // 1 KByte Auto-Increment Page Size
      RWPage = 0x400;
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      break;
    case (CPUID_IMPL_ARM|CPUID_PN_ARM_CM35P):    // ARM Cortex-M35P (v8-M Mainline with Physcial Security)
      xxCPU     = ARM_CM35P;
      xMainline = TRUE;

      if (Cache_Addr != 0) {
        // Extend CSW value to use cacheable memory accesses (Cortex-M35P has optional I-cache, cachelines invalidated by debugger access with this bit set)
        CSW_Val |= CSW_HPROT_CACHE;
        CSW_Val_Base |= CSW_HPROT_CACHE;
        apCtx->CSW_Val_Base |= CSW_HPROT_CACHE;
      }

      // 1 KByte Auto-Increment Page Size
      RWPage = 0x400;
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      break;
#endif // DBGCM_V8M

    default:
      return (EU46);
  }

#if DBGCM_V8M
  if (IsV8M()) {
    // Check Availability of Security Extensions
    status = DetectSecurityExtensions();
    if (status) return (status);
  }

  // Check FPU Support
  status = ReadD32(MVFR0, &val, BLOCK_SECTYPE_ANY);
  if (status) return (status);
#else // DBGCM_V8M
  // Check FPU Support
  status = ReadD32(MVFR0, &val);
  if (status) return (status);
#endif // DBGCM_V8M

  if (val & (MVFR0_SPF | MVFR0_DPF)) {  // FPU Support present?
    xFPU = TRUE;
  }

  status = ReadDP (DP_CTRL_STAT, &val);
  if (status) return (status);

  if (val & STICKYERR) {
    switch (DP_Type) {
      case JTAG_DP:
        status = WriteDP(DP_CTRL_STAT, val);
        break;
      case SW_DP:
        status = WriteDP(DP_ABORT, STKCMPCLR|STKERRCLR|WDERRCLR|ORUNERRCLR);
        break;
      default:
        status = (EU20);   // Internal DLL Error: Unsupported Debug Protocol
        break;
    }
    if (status) return (status);
    return (EU14);
  }

  return (0);
}


// Debugger Uninitialization
//   return value: error status
int Debug_UnInit (void) {
  int   status = 0;

#if DBGCM_MEMACCX
  unsigned int i;
#endif // DBGCM_MEMACCX

  if (DbgInit == FALSE) return (0);

  DbgInit = FALSE;

#if DBGCM_MEMACCX
  // Disconnect from accessed debug port
  for (i = 0; i < JTAG_devs.cnt; i++) {
    if (JTAG_devs.icacc[i] == 0) {
      // Not accessed by this debugger instance
      continue;
    }

    // Select Debug Port for power-down
    status = SwitchDP(i, false);
    if (status) goto end;

    status = WriteDP(DP_CTRL_STAT, 0);    // Disable Debug Power
    if (status) goto end;

    JTAG_devs.icacc[i] = 0;
  }

end:
  // Restore device index for standard accesses
  JTAG_devs.com_no = nCPU;
#else // DBGCM_MEMACCX
  status = WriteDP(DP_CTRL_STAT, 0);    // Disable Debug Power
#endif // DBGCM_MEMACCX

  return (status);
}

// Read and evaluate selected Access Port ID
int AP_ReadID(void) {
  int          status;
  DWORD val, AP_class;
  AP_CONTEXT   *apCtx;

  status = AP_CurrentCtx(&apCtx);    // 08.11.2018: No need to protect, AP_ReadID() is always called from a locked code section or before starting competing threads
  if (status) return (status);

  if (apCtx->ID != 0) {
    // AP analyzed before. Nothing left to do.
    return (0);
  }

  // Read AP IDR Register
  status = ReadAP(AP_IDR, &val);
  if (status) return (status);

  // Save AP ID
  apCtx->ID = val;
  AP_class  = (apCtx->ID & APIDR_CLASS) >> APIDR_CLASS_P;

  // Evaluate AP class
  switch (AP_class) {
  case APIDR_CLASS_MEM:
    break;
  default:                 // AP class not supported
    // return (EU51);      // Some devices can return odd values in low-power scenarios that are not fatal, hence no longer an error
    return (0);            // Fall back to Cortex-M AHB-AP defaults
  }

  // Evaluate AP Designer+Type+Variant
  switch (apCtx->ID & (APIDR_DESIGNER|APIDR_ID)) {
    // Knowingly supported AHB-APs without SPROT
  case (APIDR_DESIGNER_ARM|APIDR_AHB_CM3):         // Cortex-M3 AHB-AP
  case (APIDR_DESIGNER_ARM|APIDR_AHB_CM0):         // Cortex-M0 AHB-AP
  case (APIDR_DESIGNER_ARM|APIDR_AHB_CM0P):        // Cortex-M0+ AHB-AP
  case (APIDR_DESIGNER_ARM|APIDR_AHB_CM7):         // Cortex-M7 AHB-AP
    break;
    // Knowingly supported AHB-APs with SPROT
  case (APIDR_DESIGNER_ARM|APIDR_AHB5_SOC600):     // SoC-600 AHB5-AP
  case (APIDR_DESIGNER_ARM|APIDR_AHB5_CM33):       // Cortex-M33 AHB5-AP
  case (APIDR_DESIGNER_ARM|APIDR_AHB5_CM23):       // Cortex-M23 AHB5-AP
    break;
    // Knowingly support AHB-APs with no knowledge about SPROT support, e.g. NXP LPC55xx
  case (APIDR_DESIGNER_ARM|APIDR_AHB_CS):  // CoreSight/Cortex-M1 AHB-AP
    apCtx->CSW_Val_Base &= ~CSW_MSTRDBG;   // Not recommended to use on CoreSight AHB-AP, special for Cortex-M AHB-APs
    status = ReadAP(AP_CSW, &val);         // Read AP CSW to check read-only bits
    if (status) return (status);
    if ((val & CSW_SPIDEN) == 0) {         // Secure Privileged Debug disabled => use non-secure access only
      apCtx->CSW_Val_Base |= CSW_SPROT;    // Start auto-detection with non-secure accesses, to be re-evaluated after reading CPU features
      apCtx->KeepSPROT  = TRUE;            // Keep SPROT regardless of BLOCK_SECTYPE_xxx attributes
    } else {                               // Secure Privileged Debug enabled => anything is possible without further a-priori knowledge
      apCtx->CSW_Val_Base &= ~CSW_SPROT;   // Start auto-detection with secure accesses
      apCtx->KeepSPROT  = FALSE;           // Allow adjustment as per BLOCK_SECTYPE_xxx attributes
    }
    break;
  case (APIDR_DESIGNER_ARM|APIDR_APB_CS):  // CoreSight APB-AP
    // Significantly differs from AHB-AP CSW settings. Important to set DbgSwEnable! This can impact
    // debugger accesses through an AHB-AP (for CPU debug) that access the APB via a bridge.
    apCtx->CSW_Val_Base = CSW_DBGSWENABLE; // Only other configurable part is AddrInc
    apCtx->PT           = FALSE;           // No 8-Bit/16-Bit access and hence no packed transfer.
    break;
  case (APIDR_DESIGNER_ARM|APIDR_AXI_CS):  // CoreSight AXI-AP (for heterogeneous multi-core systems)
    apCtx->RWBits    = CSW_AXI_RWBITS;        // AXI-AP has other RW Bits
    apCtx->SPROT     = CSW_AXI_SPROT;         // AXI-AP has other SPROT position
    apCtx->CSW_Val_Base &= ~(CSW_MSTRDBG|CSW_HPROT|CSW_AXI_DOM|CSW_AXI_ACE_ENA);  // Clear Reserved|Prot|Cache|Domain|ACEEnable fields of AXI-AP CSW
    status = ReadAP(AP_CSW, &val);                                                // Read AP CSW for SPIStatus
    if (status) return (status);
    apCtx->CSW_Val_Base |= CSW_AXI_PRIV;      // Set privileged access
    apCtx->CSW_Val_Base |= CSW_AXI_DOM_SHSYS; // Set Shareable transaction encoding to System Domain (ACEEnable kept as 0)
    if ((val & CSW_SPIDEN) == 0) {            // Secure Privileged Debug disabled => use non-secure access only
      apCtx->CSW_Val_Base |= CSW_AXI_SPROT;   // Non-secure accesses
      apCtx->KeepSPROT = TRUE;                // Keep SPROT regardless of BLOCK_SECTYPE_xxx attributes
    } else {                                  // Secure Privileged Debug enabled => anything is possible without further a-priori knowledge
      apCtx->CSW_Val_Base &= ~CSW_AXI_SPROT;  // Secure accesses
      apCtx->KeepSPROT = FALSE;               // Allow adjustment as per BLOCK_SECTYPE_xxx attributes
    }
    break;
  default:                                    // Unknown MEM-APs or MEM_APs that don't need special attention
    // Let's give a try with the defaults. Shall avoid locking out APs
    // that are unknowingly working already with the defaults.
    break;
  }

  return (0);
}


// Get current auto-address increment size
extern DWORD AP_CurrentRWPage (void) {
  if ((nCPU == JTAG_devs.com_no) && (AP_SEL2IDX(AP_Sel) == MonConf.AP)) {
    return RWPage;
  }
  return 0x400;                               // 1Kbyte for APs we have analyzed in detail
}


// Get Current AP Context
int AP_CurrentCtx (AP_CONTEXT** apCtx) {
  BYTE  apIdx;

  if (apCtx == NULL) return (EU01);                // Internal DLL Error

  apIdx = AP_SEL2IDX(AP_Sel);

  if (JTAG_devs.com_no >= DP_NUM) return (EU50);   // Invalid Access Port selected
  if (apIdx       >= AP_NUM) return (EU50);        // Invalid Access Port selected

  *apCtx = &AP_Context[JTAG_devs.com_no][apIdx];

  return (0);
}


// Get AP Context for debugged CPU
int AP_CpuCtx(AP_CONTEXT** apCtx) {
  if (apCtx == NULL) return (EU01);          // Internal DLL Error
  if (nCPU       >= DP_NUM) return (EU50);   // Invalid Access Port selected
  if (MonConf.AP >= AP_NUM) return (EU50);   // Invalid Access Port selected

  *apCtx = &AP_Context[nCPU][MonConf.AP];
  return (0);
}


// Switch to currently selected AP and get its context
int AP_Switch(AP_CONTEXT** apCtx) {
  int status;

  status = AP_CurrentCtx(apCtx);
  if (status) return (status);

  if ((*apCtx)->ID == 0) {
    status = AP_ReadID();
    if (status) return (status);

    if (((*apCtx)->ID & (APIDR_DESIGNER|APIDR_ID)) != (APIDR_DESIGNER_ARM|APIDR_APB_CS)) {  // Save the effort for APB which always is 32-Bit access and never has Packed Transfer
      status = TestSizesAP();
      if (status) return (status);
    }
  }

  return (0);
}


void InitDebug() {
  int i, j;

  DbgInit = FALSE;             // Debug Initialization

  DP_Type = 0;                 // Debug Port Type
  DP_Ver  = 0;                 // Debug Port Version (V0, V1, V2)
  DP_Min  = FALSE;             // Minimal DP (without Pushed Verify/Compare,
                                     //                     Transaction Counter)
  AP_Sel  = 0x00000000;        // Current AP
  AP_Bank = 0x00;              // Current AP Bank
  AP_PT       = TRUE;          // AP Packed Transfer Support in HW
  AP_AccSizes = AP_ACCSZ_WORD; // AP Supported Access Sizes
  CSW_Val = CSW_RESERVED   |   // Current CSW Value
            CSW_MSTRDBG    |
            CSW_HPROT_PRIV |
            CSW_DBGSTAT    |
            CSW_SADDRINC   |
            CSW_SIZE32;

  NumBP = 0;                       // Number of HW Breakpoints
  NumWP = 0;                       // Number of HW Watchpoints
  NTrWP = 0;                       // Number of Trace Watchpoints
  MTrWP = 0;                       // Mask of Trace Watchpoints used
  MDtWP = 0;                       // Mask of Data Watchpoints used
  AM_WP = 0x0F;                // Address Mask Max Bit Count for Watchpoints

  // 27.01.2020: Tracepoint support
  NTrRP = 0;                   // Number of Run  TracePoints (input to ETM)
  NTrSP = 0;                   // Number of Stop TracePoints (input to ETM)
  NTrHP = 0;                   // Number of Halt TracePoints (input to ETM)
  NTrDP = 0;                   // Number of Data TracePoints (input to ETM)  // Separate from LA Points

  MTrRP = 0;                   // Mask of Run  TracePoints (input to ETM)
  MTrSP = 0;                   // Mask of Stop TracePoints (input to ETM)
  MTrHP = 0;                   // Mask of Halt TracePoints (input to ETM)
  MTrDP = 0;                   // Mask of Data TracePoints (input to ETM)    // Separate from LA Points

  FPB_Ver       = 0;           // FPB Version (0-Cortex-M0/1/3/4, 1-Cortex-M7)
  FPB_CompMask  = FPB_COMP_M;  // Mask for comparator address value
  DWT_ITM_F_R_W = FALSE;       // Separate DWT ITM Functions for Data R/W

  NumIRQ = 0;                  // Number of External IRQ

  RWPage = 0x1000;             // R/W Page (Auto Increment)

  SWBKPT32 = 0xE1200070;       // SW Breakpoint: ARM Instruction BKPT 0
  SWBKPT16 = 0xBE00;           // SW Breakpoint: Thumb Instruction BKPT 0

  DHCSR_MaskIntsStop = 0;      // Mask Interrupts on stop (0 - don't mask, C_MASKINTS - do mask)
  SWJ_SwitchSeq      = 0;      // Succeeding Switch Sequence (for later recovery)

  memset(&RegFPB, 0, sizeof(RegFPB));                      // FPB Registers
  memset(&RegDWT, 0, sizeof(RegDWT));                      // DWT Registers

  // Debug Block Addresses
  DBG_Addr  = 0;               // Core Debug Base Address
  DWT_Addr  = 0;               // DWT Base Address
  FPB_Addr  = 0;               // FPB Base Address
  ITM_Addr  = 0;               // ITM Base Address
//TPIU_Addr = 0;               // TPIU Base Address
  ETM_Addr  = 0;               // ETM Base Address
  NVIC_Addr = 0;               // NVIC Base Address
  MTB_Addr  = 0;               // MTB Base Address
  TPIU_Location = { 0, 0, 0 }; // TPIU Base Location

  // System Block Addresses
  Cache_Addr = 0;              // Cache Base Address (i.e. Cortex-M35P I-Cache present in ROM Table)

  // Debug Block Versions
  MTB_CM0P    = FALSE;         // MTB Cortex-M0+
  TPIU_Type   = TPIU_TYPE_CM;  // Trace Interface Type (TPIU_TYPE_CM, TPIU_TYPE_CS, TPIU_TYPE_SWO)
  ETM_Version = 3;             // ETM Version (can be 3 or 4)

  // Debug Functions
  DAPAbort = NULL;        // DAP Abort
  ReadDP = NULL;          // Read DP Register
  WriteDP = NULL;         // Write DP Register
  ReadAP = NULL;          // Read AP Register
  WriteAP = NULL;         // Write AP Register
  ReadD32 = NULL;         // Read 32-bit Data
  WriteD32 = NULL;        // Write 32-bit Data
  ReadD16 = NULL;         // Read 16-bit Data
  WriteD16 = NULL;        // Write 16-bit Data
  ReadD8 = NULL;          // Read 8-bit Data
  WriteD8 = NULL;         // Write 8-bit Data
  ReadBlock = NULL;       // Read Data Block
  WriteBlock = NULL;      // Write Data Block
  VerifyBlock = NULL;     // Verify Data Block
  ReadARMMem = NULL;      // Read ARM Memory
  WriteARMMem = NULL;     // Write ARM Memory
  VerifyARMMem = NULL;    // Verify ARM Memory
  GetARMRegs = NULL;      // Get ARM Registers
  SetARMRegs = NULL;      // Set ARM Registers
  SysCallExec = NULL;     // System Call Execute
  SysCallRes = NULL;      // System Call Result
  TestSizesAP = NULL;     // Test Sizes Supported in AP CSW
  DAPAbortVal    = NULL;  // DAP Abort with value to write
  ReadBlockD8    = NULL;  // Read Data Block of nMany 8-bit accesses
  ReadBlockD16   = NULL;  // Read Data Block of nMany 16-bit accesses
  ReadBlockD32   = NULL;  // Read Data Block of nMany 32-bit accesses
  WriteBlockD8   = NULL;  // Write Data Block of nMany 8-bit accesses
  WriteBlockD16  = NULL;  // Write Data Block of nMany 16-bit accesses
  WriteBlockD32  = NULL;  // Write Data Block of nMany 32-bit accesses
  ReadARMMemD8   = NULL;  // Read ARM Memory (nMany 8-bit accesses)
  ReadARMMemD16  = NULL;  // Read ARM Memory (nMany 16-bit accesses)
  ReadARMMemD32  = NULL;  // Read ARM Memory (nMany 32-bit accesses)
  WriteARMMemD8  = NULL;  // Write ARM Memory (nMany 8-bit accesses)
  WriteARMMemD16 = NULL;  // Write ARM Memory (nMany 16-bit accesses)
  WriteARMMemD32 = NULL;  // Write ARM Memory (nMany 32-bit accesses)
  SwitchDP       = NULL;  // Switch DP
  SWJ_Sequence   = NULL;  // Execute SWJ (SWDIO_TMS) Sequence
  SWJ_Clock      = NULL;  // Change Debug Clock Frequency
  level = 0;
  rompn = 0;
  CSW_Val_Base = CSW_RESERVED   | // Basic CSW Value
                 CSW_MSTRDBG    |
                 CSW_HPROT_PRIV |
                 CSW_DBGSTAT;
  DSCSR_Has_CDSKEY = FALSE;    // DSCSR has CDSKEY Bit (Security Extensions only)

  for (i = 0; i < DP_NUM; i++) {
    for (j = 0; j < AP_NUM; j++) {
      AP_Context[i][j].ID = 0;
      AP_Context[i][j].AccSizes     = AP_ACCSZ_WORD;
      AP_Context[i][j].CSW_Val_Base = CSW_RESERVED   |
                                      CSW_MSTRDBG    |
                                      CSW_HPROT_PRIV |
                                      CSW_DBGSTAT;
      AP_Context[i][j].SPROT        = CSW_SPROT;
      AP_Context[i][j].KeepSPROT    = FALSE;
      AP_Context[i][j].PT           = TRUE;
      AP_Context[i][j].RWBits       = CSW_RWBITS;
    }
  }

  MemAccX_AP = FALSE;
}
