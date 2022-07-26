/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.12
 * @date     $Date: 2020-08-10 16:35:34 +0200 (Mon, 10 Aug 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
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



#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "COLLECT.H"

// Debug Port Interface
#define JTAG_DP 0 // JTAG
#define SW_DP   1 // SW

#define DP_NUM  64 // Max Number of possible DPs
#define AP_NUM  32 // Max Number of possible APs

// Debug Port ID Register (DPIDR) definitions
#define DPID_REV_M    0xF0000000 // Revision Mask
#define DPID_REV_P    28         // Revision Position
#define DPID_PARTNO_M 0x0FF00000 // Part Number Mask
#define DPID_PARTNO_P 20         // Part Number Position
#define DPID_MIN      0x00010000 // Minimal DP
#define DPID_VER_M    0x0000F000 // Version Mask
#define DPID_VER_P    12         // Version Position
#define DPID_DESIGN_M 0x00000FFE // Designer ID Mask
#define DPID_DESIGN_P 1          // Designer ID Position
#define DPID_DESIGNER 0x23B      // Designer ID (ARM)

// Debug Port Register Addresses
#define DP_IDCODE    0x00 // IDCODE Register (Read only)
#define DP_ABORT     0x00 // Abort Register (Write only)
#define DP_CTRL_STAT 0x04 // Control & Status
#define DP_WCR       0x04 // Wire Control Register (SW Only)
#define DP_DLCR      0x04 // Data Link Control Register (Renamed WCR)
#define DP_TARGETID  0x04 // Target ID Register (Read only, DPv2)
#define DP_DLPIDR    0x04 // Data Link Protocol ID Register (Read only, DPv2)
#define DP_EVENTSTAT 0x04 // Event Status Register (Read only, DPv2)
#define DP_SELECT    0x08 // Select Register (JTAG R/W & SW W)
#define DP_RESEND    0x08 // Resend (SW Read Only)
#define DP_RDBUFF    0x0C // Read Buffer (Read Only)
#define DP_TARGETSEL 0x0C // Target Selection Register (Write only, SWD only, DPv2)

// Abort Register definitions
#define DAPABORT   0x00000001 // DAP Abort
#define STKCMPCLR  0x00000002 // Clear STICKYCMP Flag (SW Only)
#define STKERRCLR  0x00000004 // Clear STICKYERR Flag (SW Only)
#define WDERRCLR   0x00000008 // Clear WDATAERR Flag (SW Only)
#define ORUNERRCLR 0x00000010 // Clear STICKYORUN Flag (SW Only)

// Target ID Register definitions (Read only, DPv2)
// Target Selection Register definitions (Write only, SWD only, DPv2)
// Data Link Protocol ID Register (Read only, DPv2)
#define TDESIGNER   0x00000FFE // Target Designer Mask (Target ID only)
#define TDESIGNER_P 1          // Target Designer Position (Target ID only)
#define TPARTNO     0x0FFFF000 // Target Part Number Mask (Target ID + Target Selection)
#define TPARTNO_P   12         // Target Part Number Position (Target ID + Target Selection)
#define TREVISION   0xF0000000 // Target Revision Mask (Target ID only)
#define TREVISION_P 28         // Target Revision Position (Target ID only)
#define TINSTANCE   0xF0000000 // Target Revision Mask (Target Selection + Data Link Prot ID)
#define TINSTANCE_P 28         // Target Revision Position (Target Selection + Data Link Prot ID)
#define PROTVSN     0x0000000F // Protocol Version Mask (Data Link Prot ID)
#define PROTVSN_V2  0x00000001 // SWD Protocol Version Version 2

// Debug Control and Status definitions
#define ORUNDETECT   0x00000001 // Overrun Detect
#define STICKYORUN   0x00000002 // Sticky Overrun
#define TRNMODE      0x0000000C // Transfer Mode Mask
#define TRNNORMAL    0x00000000 // Transfer Mode: Normal
#define TRNVERIFY    0x00000004 // Transfer Mode: Pushed Verify
#define TRNCOMPARE   0x00000008 // Transfer Mode: Pushed Compare
#define STICKYCMP    0x00000010 // Sticky Compare
#define STICKYERR    0x00000020 // Sticky Error
#define READOK       0x00000040 // Read OK (SW Only)
#define WDATAERR     0x00000080 // Write Data Error (SW Only)
#define MASKLANE     0x00000F00 // Mask Lane Mask
#define MASKLANE0    0x00000100 // Mask Lane 0
#define MASKLANE1    0x00000200 // Mask Lane 1
#define MASKLANE2    0x00000400 // Mask Lane 2
#define MASKLANE3    0x00000800 // Mask Lane 3
#define TRNCNT       0x001FF000 // Transaction Counter Mask
#define CDBGRSTREQ   0x04000000 // Debug Reset Request
#define CDBGRSTACK   0x08000000 // Debug Reset Acknowledge
#define CDBGPWRUPREQ 0x10000000 // Debug Power-up Request
#define CDBGPWRUPACK 0x20000000 // Debug Power-up Acknowledge
#define CSYSPWRUPREQ 0x40000000 // System Power-up Request
#define CSYSPWRUPACK 0x80000000 // System Power-up Acknowledge

// Debug Wire Control Register definitions (SW Only)
#define WIREMODE     0x000000C0 // Wire Mode Mask
#define WIREMODESYNC 0x00000040 // Wire Mode: Synchronous
#define TURNROUND    0x00000300 // Turnaround Mask
#define TURNROUND1   0x00000000 // Turnaround 1 sample period
#define TURNROUND2   0x00000100 // Turnaround 2 sample period
#define TURNROUND3   0x00000200 // Turnaround 3 sample period
#define TURNROUND4   0x00000300 // Turnaround 4 sample period

// Debug Select Register definitions
#define CTRLSEL   0x00000001 // CTRLSEL (SW Only)
#define APBANKSEL 0x000000F0 // APBANKSEL Mask
#define APSEL     0xFF000000 // APSEL Mask
#define APSEL_P   24         // APSEL Position


// Access Port Register Addresses
#define AP_CSW 0x00 // Control and Status Word
#define AP_TAR 0x04 // Transfer Address
#define AP_DRW 0x0C // Data Read/Write
#define AP_BD0 0x10 // Banked Data 0
#define AP_BD1 0x14 // Banked Data 1
#define AP_BD2 0x18 // Banked Data 2
#define AP_BD3 0x1C // Banked Data 3
#define AP_CFG 0xF4 // Configuration Register
#define AP_ROM 0xF8 // Debug ROM Address
#define AP_IDR 0xFC // Identification Register

// AP Control and Status Word definitions
#define CSW_SIZE          0x00000007 // Access Size: Selection Mask
#define CSW_SIZE8         0x00000000 // Access Size: 8-bit
#define CSW_SIZE16        0x00000001 // Access Size: 16-bit
#define CSW_SIZE32        0x00000002 // Access Size: 32-bit
#define CSW_ADDRINC       0x00000030 // Auto Address Increment Mask
#define CSW_NADDRINC      0x00000000 // No Address Increment
#define CSW_SADDRINC      0x00000010 // Single Address Increment
#define CSW_PADDRINC      0x00000020 // Packed Address Increment
#define CSW_DBGSTAT       0x00000040 // Debug Status
#define CSW_TINPROG       0x00000080 // Transfer in progress
#define CSW_MODE          0x00000F00 // Operation Mode (Mask)
#define CSW_HPROT         0x7F000000 // Bus Access Protection Bits (Mask)
#define CSW_HPROT_PRIV    0x02000000 // User/Privilege Control
#define CSW_HPROT_BUFF    0x04000000 // Bufferable (1) and unbufferable (0) (Cortex-M7)
#define CSW_HPROT_CACHE   0x08000000 // Cacheable  (1) and uncacheable  (0) (Cortex-M7)
#define CSW_MSTRTYPE      0x20000000 // Master Type Mask    (Reserved for Cortex-M7)
#define CSW_MSTRCORE      0x00000000 // Master Type: Core   (Reserved for Cortex-M7)
#define CSW_MSTRDBG       0x20000000 // Master Type: Debug  (Reserved for Cortex-M7)
#define CSW_RESERVED      0x01000000 // Reserved Value
#define CSW_SPROT         0x40000000 // SProt (ignore SPIDEN signal when setting HPROT[6]), CoreSight AHB-AP only
#define CSW_DBGSWENABLE   0x80000000 // Debug Software Access Enable
#define CSW_RWBITS        0xFF000F77 // CSW Bits with R/W Attribute (DBGSTAT actually RO)
#define CSW_SPIDEN        0x00800000 // Secure Privileged Debug Enabled (read-only)
#define CSW_AXI_RWBITS    0xFF007F77 // CSW Bits with R/W Attribute (DBGSTAT actually RO) for AXI-AP
#define CSW_AXI_SPROT     0x20000000 // Non-Secure (AWPROT[1]) for AXI-AP \
                                     // Effectively: 0 - Secure Access, 1 - Non-Secure Access
#define CSW_AXI_PRIV      0x10000000 // Privileged (AWPROT[0]) for AXI-AP
#define CSW_AXI_ACE_ENA   0x00001000 // ACEEnable Bit for AXI-AP
#define CSW_AXI_DOM       0x00006000 // Domain Bit Mask for AXI-AP
#define CSW_AXI_DOM_SHIN  0x00002000 // SHareable, INner domain, includes additional masters
#define CSW_AXI_DOM_SHOUT 0x00004000 // SHareable, OUTer domain, also includes inner or additional masters
#define CSW_AXI_DOM_SHSYS 0x00006000 // SHareable, SYStem domain, all masters included

// AP Configuration Register definitions
#define CFG_BIGENDIAN 0x00000001 // MEM-AP Endianess (Little-Endian for Cortex-M)

// AP Supported Access Size definitions (not register related)
#define AP_ACCSZ_BYTE  1 // Byte Access Supported
#define AP_ACCSZ_HWORD 2 // Half-Word Access Supported
#define AP_ACCSZ_WORD  4 // Word Access Supported

// AP Identification Register definitions
#define APIDR_REV        0xF0000000 // AP Revision Mask
#define APIDR_REV_P      28         // AP Revision Position
#define APIDR_DESIGNER   0x0FFE0000 // AP Designer Mask
#define APIDR_DESIGNER_P 17         // AP Designer Position
#define APIDR_CLASS      0x0001E000 // AP Class Mask
#define APIDR_CLASS_P    13         // AP Class Position
#define APIDR_VARIANT    0x000000F0 // AP Variant Mask
#define APIDR_VARIANT_P  4          // AP Variant Position
#define APIDR_TYPE       0x0000000F // AP Type Mask
#define APIDR_ID         0x000000FF // AP ID (Type+Variant)

// AP IDR Designer definitions
#define APIDR_DESIGNER_ARM 0x04760000 // ARM

// AP IDR Class definitions
#define APIDR_CLASS_JTAG 0x0 // JTAG-AP Class
#define APIDR_CLASS_MEM  0x8 // MEM-AP Class

// AP IDR Type definitions
#define APIDR_JTAG 0x0 // JTAG AP
#define APIDR_AHB  0x1 // AHB-AP
#define APIDR_APB  0x2 // APB-AP
#define APIDR_AXI  0x4 // AXI-AP
#define APIDR_AHB5 0x5 // AHB-AP (v5)
#define APIDR_APB6 0x6 // APB-AP (v4)

// AP IDR ID definitions

// AHB-APs (supported)
#define APIDR_AHB_CS      0x01 // CoreSight/Cortex-M1 AHB-AP
#define APIDR_AHB_CM3     0x11 // Cortex-M3 AHB-AP
#define APIDR_AHB_CM0     0x21 // Cortex-M0 AHB-AP
#define APIDR_AHB_CM0P    0x31 // Cortex-M0+ AHB-AP
#define APIDR_AHB_CM7     0x41 // Cortex-M7 AHB-AP
#define APIDR_AHB5_SOC600 0x05 // SoC-600 AHB5-AP
#define APIDR_AHB5_CM33   0x15 // Cortex-M33 AHB5-AP
#define APIDR_AHB5_CM23   0x25 // Cortex-M23 AHB5-AP
// APB-APs (supported)
#define APIDR_APB_CS 0x02 // CoreSight APB-AP
// Other
#define APIDR_AXI_CS 0x04 // CoreSight AXI-AP

// AP Auxiliary Macros
#define AP_SEL2IDX(ap) ((ap & APSEL) >> APSEL_P)

// Debug Variables
extern DWORD DP_Type;     // Debug Port Type (JTAG/SW)
extern BYTE  DP_Ver;      // Debug Port Version (V0, V1, V2)
extern BOOL  DP_Min;      // Minimal DP (without Pushed Verify/Compare,
                          //                     Transaction Counter)
extern BOOL  AP_PT;       // AP Packed Transfer Support in HW
extern DWORD AP_Sel;      // Current AP
extern BYTE  AP_Bank;     // Current AP Bank
extern BYTE  AP_AccSizes; // AP Supported Access Sizes
// 02.07.2019: Deprecated CSW_Val - Use AP_CONTEXT::CSW_Val_Base mechanism instead (see "Usage of AP_Context" in JTAG.CPP/SWD.CPP)
extern DWORD CSW_Val; // Current CSW Value, use to store latest AP CSW value of currently selected AP

extern BYTE NumBP; // Number of HW Breakpoints
extern BYTE NumWP; // Number of HW Watchpoints
extern BYTE NTrWP; // Number of Trace Watchpoints
extern BYTE MTrWP; // Mask of Trace Watchpoints used
extern BYTE MDtWP; // Mask of Data Watchpoints used
extern BYTE AM_WP; // Address Mask Max Bit Count for Watchpoints

// 27.01.2020: Tracepoint support
extern BYTE MTrRP; // Mask of Run  TracePoints (input to ETM)
extern BYTE MTrSP; // Mask of Stop TracePoints (input to ETM)
extern BYTE MTrHP; // Mask of Halt TracePoints (input to ETM)
extern BYTE MTrDP; // Mask of Data TracePoints (input to ETM)    // Separate from LA Points

extern BYTE NTrRP; // Number of Run  TracePoints (input to ETM)
extern BYTE NTrSP; // Number of Stop TracePoints (input to ETM)
extern BYTE NTrHP; // Number of Halt TracePoints (input to ETM)
extern BYTE NTrDP; // Number of Data TracePoints (input to ETM)  // Separate from LA Points

extern BYTE  FPB_Ver;       // FPB Version (0-Cortex-M0/1/3/4, 1-Cortex-M7)
extern DWORD FPB_CompMask;  // Mask for comparator address value
extern BOOL  DWT_ITM_F_R_W; // Separate DWT ITM Functions for Data R/W

extern int NumIRQ; // Number of External IRQ

extern DWORD RWPage; // R/W Page (Auto Increment)

extern DWORD SWBKPT32; // SW Breakpoint: ARM Instruction BKPT 0
extern WORD  SWBKPT16; // SW Breakpoint: Thumb Instruction BKPT 0

extern DWORD DHCSR_MaskIntsStop; // Mask Interrupts on stop (0 - don't mask, C_MASKINTS - do mask)
extern BOOL  DSCSR_Has_CDSKEY;   // DSCSR has CDSKEY Bit (Security Extensions only)

extern WORD SWJ_SwitchSeq; // Succeeding Switch Sequence (for later recovery)
// 02.07.2019: Deprecated CSW_Val_Base - Use AP_CONTEXT::CSW_Val_Base mechanism instead (see "Usage of AP_Context" in JTAG.CPP/SWD.CPP)
extern DWORD CSW_Val_Base; // Basic CSW Value, bits to use for MEM-AP accesses except from access size and address increment setting

// AP Context Information - See also "Usage of AP_Context" in JTAG.cpp/SWD.cpp.
typedef struct {
    DWORD ID;           // AP IDR Value, 0 if context not known yet
    DWORD CSW_Val_Base; // AP CSW Value to use as base for accesses
    BYTE  AccSizes;     // AP Supported Access Sizes
    DWORD SPROT;        // AP SPROT Bit
    BOOL  KeepSPROT;    // AP CSW.SPROT: Do not modify with BLOCK_SECTYPE_xxx
    BOOL  PT;           // AP Supports Packed Transfers
    DWORD RWBits;       // AP Bits that are Read/Write
} AP_CONTEXT;

extern AP_CONTEXT AP_Context[DP_NUM][AP_NUM];

extern BOOL MemAccX_AP; // Extended Memory Access with explicit AP in progress

// CoreSight Component Location Info
typedef struct {
    DWORD DP;   // Debug Port
    DWORD AP;   // Access Port
    DWORD Addr; // Component Offset
} CS_LOCATION;


#define RWBlock 1024 // R/W Block Size

#define PPBAddr 0xE0000000 // Private Peripheral Bus Address


// Core Debug Register Address Offsets
#define DBG_OFS      0x0DF0 // Debug Register Offset inside NVIC
#define DBG_HCSR_OFS 0x000  // Debug Halting Control & Status Register
#define DBG_CRSR_OFS 0x004  // Debug Core Register Selector Register
#define DBG_CRDR_OFS 0x008  // Debug Core Register Data Register
#define DBG_EMCR_OFS 0x00C  // Debug Exception & Monitor Control Register
// Core Debug Register Address Offsets - v8-M Extensions
#define DBG_AUTHCTRL_OFS   0x014 // Debug Authentication Control Register
#define DBG_SCSR_OFS       0x018 // Debug Security Control and Status Register
#define DBG_AUTHSTATUS_OFS 0x1C8 // Debug Authentication Status Register

// Core Debug Register Addresses
#define DBG_HCSR (DBG_Addr + DBG_HCSR_OFS)
#define DBG_CRSR (DBG_Addr + DBG_CRSR_OFS)
#define DBG_CRDR (DBG_Addr + DBG_CRDR_OFS)
#define DBG_EMCR (DBG_Addr + DBG_EMCR_OFS)
// Core Debug Register Addresses - v8-M Extensions
#define DBG_AUTHCTRL   (DBG_Addr + DBG_AUTHCTRL_OFS)
#define DBG_SCSR       (DBG_Addr + DBG_SCSR_OFS)
#define DBG_AUTHSTATUS (DBG_Addr + DBG_AUTHSTATUS_OFS)

// Debug Halting Control and Status Register definitions
#define C_DEBUGEN   0x00000001 // Debug Enable
#define C_HALT      0x00000002 // Halt
#define C_STEP      0x00000004 // Step
#define C_MASKINTS  0x00000008 // Mask Interrupts
#define C_SNAPSTALL 0x00000020 // Snap Stall
#define S_REGRDY    0x00010000 // Register R/W Ready Flag
#define S_HALT      0x00020000 // Halt Flag
#define S_SLEEP     0x00040000 // Sleep Flag
#define S_LOCKUP    0x00080000 // Lockup Flag
#define S_RETIRE_ST 0x01000000 // Sticky Retire Flag
#define S_RESET_ST  0x02000000 // Sticky Reset Flag
#define DBGKEY      0xA05F0000 // Debug Key
// Debug Halting Control and Status Register definitions - v8-M Extensions
#define S_SDE        0x00100000 // Secure Debug Enabled Flag (v8-M)
#define S_RESTART_ST 0x04000000 // Sticky Restart Flag ("Sticky C_HALT cleared bit")

// Debug Core Register Selector Register definitons
#define REGSEL 0x0000001F // Register Select Mask
#define REGWR  0x00010000 // Register Write (REGWnR = 1)
#define REGRD  0x00000000 // Register Read  (REGWnR = 0)

// Debug Exception and Monitor Control Register definitions
#define VC_CORERESET 0x00000001 // Reset Vector Catch
#define VC_MMERR     0x00000010 // Debug Trap on MMU Fault
#define VC_NOCPERR   0x00000020 // Debug Trap on No Coprocessor Fault
#define VC_CHKERR    0x00000040 // Debug Trap on Checking Error Fault
#define VC_STATERR   0x00000080 // Debug Trap on State Error Fault
#define VC_BUSERR    0x00000100 // Debug Trap on Bus Error Fault
#define VC_INTERR    0x00000200 // Debug Trap on Interrupt Error Fault
#define VC_HARDERR   0x00000400 // Debug Trap on Hard Fault
#define MON_EN       0x00010000 // Monitor Enable
#define MON_PEND     0x00020000 // Monitor Pend
#define MON_STEP     0x00040000 // Monitor Step
#define MON_REQ      0x00080000 // Monitor Request
#define TRCENA       0x01000000 // Trace Enable (DWT, ITM, ETM, TPIU)
// Debug Exception and Monitor Control Register definitions - v8-M Extensions
#define VC_SFERR 0x00000800 // Debug Trap on Secure Fault
#define SDME     0x00100000 // Secure Debug Monitor Enable

// Debug Authentication Control Register (v8-M)
#define DAUTH_SPIDENSEL  0x00000001 // Secure Invasive Debug Enable Select
#define DAUTH_INTSPIDEN  0x00000002 // Internal Secure Invasive Debug Enable
#define DAUTH_SPNIDENSEL 0x00000004 // Secure Non-Invasive Debug Enable Select
#define DAUTH_INTSPNIDEN 0x00000008 // Internal Non-Secure Invasive Debug Enable

// Debug Security Control and Status Register (v8-M)
#define SBRSELEN 0x00000001 // Secure Bank Register Select Enable
#define SBRSEL   0x00000002 // Secure Bank Register Select
#define CDS      0x00010000 // Current Domain Secure
#define CDSKEY   0x00020000 // Current Domain Secure Key (Locks CDS value if 1)

// Debug Authentication Status Register (v8-M)
#define DAUTH_NSE  0x00000001 // Non-Secure Invasive Debug Enabled
#define DAUTH_NSI  0x00000002 // Non-Secure Invasive Debug Implemented
#define DAUTH_NSNE 0x00000004 // Non-Secure Non-Invasive Debug Enabled
#define DAUTH_NSNI 0x00000008 // Non-Secure Non-Invasive Debug Implemented
#define DAUTH_SE   0x00000010 // Secure Invasive Debug Enabled
#define DAUTH_SI   0x00000020 // Secure Invasive Debug Implemented
#define DAUTH_SNE  0x00000040 // Secure Non-Invasive Debug Enabled
#define DAUTH_SNI  0x00000080 // Secure Non-Invasive Debug Implemented

// NVIC: Interrupt Controller Type Register
#define NVIC_ICT    (NVIC_Addr + 0x0004)
#define INTLINESNUM 0x0000001F // Interrupt Line Numbers

// NVIC: CPUID Base Register
#define NVIC_CPUID     (NVIC_Addr + 0x0D00)
#define CPUID_PARTNO   0x0000FFF0 // Part Number Mask
#define CPUID_REVISION 0x0000000F // Revision Mask
#define CPUID_VARIANT  0x00F00000 // Variant Mask
#define CPUID_ARCH     0x000F0000 // Architecture Mask
#define CPUID_IMPL     0xFF000000 // Implementer Mask
#define CPUID_IMPL_ARM 0x41000000 // Implementer ARM

// ARM CPUIDs
#define CPUID_PN_ARM_CM23  0x0000D200 // CPUID Part Number ARM Cortex-M23 (v8-M Baseline)
#define CPUID_PN_ARM_CM33  0x0000D210 // CPUID Part Number ARM Cortex-M33 (v8-M Mainline)
#define CPUID_PN_ARM_CM35P 0x0000D310 // CPUID Part Number ARM Cortex-M35P (v8-M Mainline with Physical Security)

// NVIC: Application Interrupt/Reset Control Register
#define NVIC_AIRCR    (NVIC_Addr + 0x0D0C)
#define VECTRESET     0x00000001 // Reset Cortex-M (except Debug), v7-M only
#define VECTCLRACTIVE 0x00000002 // Clear Active Vector Bit
#define SYSRESETREQ   0x00000004 // Reset System (except Debug)
#define VECTKEY       0x05FA0000 // Write Key
// NVIC: Application Interrupt/Reset Control Register - v8-M Extensions
#define SYSRESETREQS 0x00000008 // System Reset Secure

// NVIC: Debug Fault Status Register
#define NVIC_DFSR (NVIC_Addr + 0x0D30)
#define HALTED    0x00000001 // Halt Flag
#define BKPT      0x00000002 // BKPT Flag
#define DWTTRAP   0x00000004 // DWT Match
#define VCATCH    0x00000008 // Vector Catch Flag
#define EXTERNAL  0x00000010 // External Debug Request

// NVIC: Processor Feature Register 0
#define NVIC_PFR0 (NVIC_Addr + 0x0D40)

// NVIC: Processor Feature Register 1
#define NVIC_PFR1       (NVIC_Addr + 0x0D44)
#define NVIC_PF_SEC     0x000000F0 // Security Field Mask
#define NVIC_PF_SEC_IMP 0x00000010 // Security Field: Security Extensions Implemented

// NVIC: Secure Fault Status Register (v8-M)
#define NVIC_SFSR (NVIC_Addr + 0x0DE4)
#define INVEP     0x00000001 // Invalid Entry Point Flag
#define INVMS     0x00000002 // Invalid Magic Signature Flag
#define INVER     0x00000004 // Invalid Exception Return Flag
#define AUVIOL    0x00000008 // Attribution Unit Violation Flag
#define INVTRAN   0x00000010 // Invalid Transition Flag
#define LSPERR    0x00000020 // Lazy State Preservation Error Flag
#define SFARVALID 0x00000040 // Secure Fault Address Valid Flag
#define LSERR     0x00000080 // Lazy State Error Flag

// NVIC: Secure Fault Address Register
#define NVIC_SFAR (NVIC_Addr + 0x0DE8)

// Media and VFP Feature Register 0 and 1
#define MVFR0     (NVIC_Addr + 0x0F40)
#define MVFR1     (NVIC_Addr + 0x0F44)
#define MVFR2     (NVIC_Addr + 0x0F48) // Cortex-M7 Only
#define MVFR0_VAL 0x10110021
#define MVFR1_VAL 0x11000011
// ARM Cortex-M4 values
#define MVFR0_CM4_VAL 0x10110021
#define MVFR1_CM4_VAL 0x11000011
// ARM Cortex-M7 values
#define MVFR0_CM7_SFPU_VAL 0x10110021
#define MVFR0_CM7_DFPU_VAL 0x10110221
#define MVFR1_CM7_SFPU_VAL 0x11000011
#define MVFR1_CM7_DFPU_VAL 0x12000011
#define MVFR2_CM7_VAL      0x00000040

// Media and VFP Feature Register 0
#define MVFR0_SPF     0x000000F0 // Single Precision FP Support Mask
#define MVFR0_DPF     0x00000F00 // Double Precision FP Support Mask
#define MVFR0_SPF_SUP 0x00000020 // Single Precision FP Supported
#define MVFR0_DPF_SUP 0x00000200 // Double Precision FP Supported


// Flash Patch and Breakpoint (FPB) Registers
typedef struct {
    DWORD CTRL;     // Control Register
    DWORD REMAP;    // Remap Register (Cortex-M3/M4/M7)
    DWORD COMP[16]; // Comparator Registers
} RgFPB;

extern RgFPB RegFPB;

#define FPB_CTRL  (FPB_Addr + 0x00)
#define FPB_REMAP (FPB_Addr + 0x04)
#define FPB_COMP  (FPB_Addr + 0x08)

// FPB Control Register definitions
#define FPB_ENABLE     0x00000001 // Enable
#define FPB_KEY        0x00000002 // Key
#define FPB_NUM_CODE   0x000000F0 // Number of Code Slots Mask
#define FPB_NUM_CODE_P 4          // Number of Code Slots Position
#define FPB_NUM_LIT    0x00000F00 // Number of Literal Slots Mask
#define FPB_NUM_LIT_P  8          // Number of Literal Slots Position
#define FPB_REV        0xF0000000 // Revision Mask (Cortex-M7, otherwise Reserved/RAZ)
#define FPB_REV_P      28         // Revision Mask Position (Cortex-M7, otherwise Reserved/RAZ)

// FPB Comparator Register definitions
#define FPB_COMP_M  0x1FFFFFFC // COMP Mask
#define FPB_REPLACE 0x00000000 // REPLACE: Remap
#define FPB_BKPT_L  0x40000000 // REPLACE: BKPT on lower halfword
#define FPB_BKPT_H  0x80000000 // REPLACE: BKPT on upper halfword
#define FPB_BKPT_LH 0xC0000000 // REPLACE: BKPT on lower & upper halfword

// FPB Comparator Register definitions (FPB v1 and later)
#define FPB_V1_COMP_M 0xFFFFFFFE // COMP Mask
#define FPB_V1_FE     0x80000000 // Flash Patch Enable (only if FPB_ENABLE == 0 and remap supported)

// Data Watchpoint and Trace (DWT) Registers
typedef struct {
    DWORD CTRL;     // Control Register
    DWORD CYCCNT;   // Cycle Count Register
    DWORD CPICNT;   // CPI Count Register
    DWORD EXCCNT;   // Exception Overhead Count Register
    DWORD SLEEPCNT; // Sleep Count Register
    DWORD LSUCNT;   // LSU Count Register
    DWORD FOLDCNT;  // Fold Count Register
    DWORD PCSR;     // Program Counter Sample Register (optional)
    struct {
        DWORD COMP; // Comparator Register
        DWORD MASK; // Mask Register
        DWORD FUNC; // Function Register
        DWORD Rsrvd;
    } CMP[16]; // Comparators
} RgDWT;

extern RgDWT RegDWT;

#define DWT_CTRL     (DWT_Addr + 0x00)
#define DWT_CYCCNT   (DWT_Addr + 0x04)
#define DWT_CPICNT   (DWT_Addr + 0x08)
#define DWT_EXCCNT   (DWT_Addr + 0x0C)
#define DWT_SLEEPCNT (DWT_Addr + 0x10)
#define DWT_LSUCNT   (DWT_Addr + 0x14)
#define DWT_FOLDCNT  (DWT_Addr + 0x18)
#define DWT_PCSR     (DWT_Addr + 0x1C)
#define DWT_CMP      (DWT_Addr + 0x20)
#define DWT_COMP     (DWT_Addr + 0x20)
#define DWT_MASK     (DWT_Addr + 0x24)
#define DWT_FUNC     (DWT_Addr + 0x28)


// DWT Control Register definitions
#define DWT_CYCCNTEN   0x00000001 // Enable DWT_CYCCNT
#define DWT_POSTSET    0x0000001E // Reload value for POSTCNT Mask
#define DWT_POSTSET_P  1          // Reload value for POSTCNT Position
#define DWT_POSTCNT    0x000001E0 // Post-scalar Counter Mask
#define DWT_POSTCNT_P  5          // Post-scalar Counter Position
#define DWT_CYCTAP     0x00000200 // Cycle Tap 10/6
#define DWT_SYNCTAP    0x00000C00 // Sync Cycle Tap Mask
#define DWT_SYNCTAP24  0x00000400 // Sync Cycle Tap 24
#define DWT_SYNCTAP26  0x00000800 // Sync Cycle Tap 26
#define DWT_SYNCTAP28  0x00000C00 // Sync Cycle Tap 28
#define DWT_PCSAMPLEEN 0x00001000 // Enable PC Sampling Event
#define DWT_EXCTRCEN   0x00010000 // Enable Interrupt Event Tracing
#define DWT_CPIEVTEN   0x00020000 // Enable CPI Count Event
#define DWT_EXCEVTEN   0x00040000 // Enable Interrupt Overhead Event
#define DWT_SLEEPEVTEN 0x00080000 // Enable Sleep Count Event
#define DWT_LSUEVTEN   0x00100000 // Enable LSU Count Event
#define DWT_FOLDEVTEN  0x00200000 // Enable Fold Count Event
#define DWT_CYCEVTEN   0x00400000 // Enable Cycle Count Event
#define DWT_NOEXTTRIG  0x0F000000 // CMPMATCH[n] not supported
#define DWT_NUMCOMP    0xF0000000 // Number of Comparators Mask
#define DWT_NUMCOMP_P  28         // Number of Comparators Position
// DWT Control Register definitions - v8-M Extensions
#define DWT_CYCDISS   0x00800000 // Disable Cycle Count in Secure
#define DWT_NOEXTTRIG 0x0F000000 // CMPMATCH[n] not supported

// DWT Function Register - v6-M/v7-M
#define DWT_FUNCTION   0x0000000F // Function Mask
#define DWT_DISABLED   0x00000000 // Function: Disabled
#define DWT_ITM_PC     0x00000001 // Function: ITM Emit PC
#define DWT_ITM_DRW    0x00000002 // Function: ITM Emit Data (R/W)
#define DWT_ITM_DRW_PC 0x00000003 // Function: ITM Emit Data (R/W) with PC
#define DWT_WP_PC      0x00000004 // Function: Watchpoint on PC Match
#define DWT_WP_R       0x00000005 // Function: Watchpoint on Read
#define DWT_WP_W       0x00000006 // Function: Watchpoint on Write
#define DWT_WP_RW      0x00000007 // Function: Watchpoint on Read or Write
#define DWT_ETM_PC     0x00000008 // Function: ETM Trigger on PC Match
#define DWT_ETM_R      0x00000009 // Function: ETM Trigger on Read
#define DWT_ETM_W      0x0000000A // Function: ETM Trigger on Write
#define DWT_ETM_RW     0x0000000B // Function: ETM Trigger on Read or Write
#define DWT_ITM_DR     0x0000000C // Function: ITM Emit Data (R)
#define DWT_ITM_DW     0x0000000D // Function: ITM Emit Data (W)
#define DWT_ITM_DR_PC  0x0000000E // Function: ITM Emit Data (R) with PC
#define DWT_ITM_DW_PC  0x0000000F // Function: ITM Emit Data (W) with PC
#define DWT_EMITRANGE  0x00000020 // Emit Range Field
#define DWT_CYCMATCH   0x00000080 // CYCCNT Match
#define DWT_DATAVMATCH 0x00000100 // Data Value Match
#define DWT_DATAVSIZEB 0x00000000 // Data Value Size - Byte
#define DWT_DATAVSIZEH 0x00000400 // Data Value Size - Half Word
#define DWT_DATAVSIZEW 0x00000800 // Data Value Size - Word
#define DWT_LNK1ENA    0x00000200 // Second linked comparator support
#define DWT_DATAVADDR0 12         // Data Value Address 0 Bit Position
#define DWT_DATAVADDR1 16         // Data Value Address 1 Bit Position
#define DWT_MATCHED    0x01000000 // Watchpoint Matched

// DWT Mask Register - v6-M/v7-M
#define DWT_MASK_MAX 0x0000001F // Maximum number of mask bits


// v8-M DWT Function Register FUNCTION + MATCH Combinations

// v8-M Baseline
// MATCH | Description         | ACTION: TRIG        DBG        TRACE0     TRACE1         Trigger
// -----------------------------------------------------------------------------------------------
// 0b0000 | Disabled           |         Valid     Reserved    Reserved   Reserved     |
// 0b0001 | Reserved           |        Reserved   Reserved    Reserved   Reserved     |  Reserved
// 0b0010 | Instr. Addr.       |         Valid     Debug Evt   Reserved   Reserved     |    x
// 0b0011 | Linked Instr. Addr.|         Valid     Reserved    Reserved   Reserved     |  Unpredictable
// 0b01xx | Data Addr.         |         Valid     Debug Evt   Reserved   Reserved     |    x
// 0b0111 | Linked Data Addr.  |         Valid     Reserved    Reserved   Reserved     |  Unpredictable
// 0b1xxx | Reserved           |        Reserved   Reserved    Reserved   Reserved     |  Reserved

// v8-M Mainline
// MATCH | Description         | ACTION: TRIG        DBG        TRACE0     TRACE1         Trigger
// -----------------------------------------------------------------------------------------------
// 0b0000 | Disabled           |         Valid     Reserved    Reserved   Reserved     |
// 0b0001 | Cycle Counter      |         Valid     Debug Evt   Match      PC           |    x
// 0b0010 | Instr. Addr.       |         Valid     Debug Evt   Match      PC           |    x
// 0b0011 | Instr. Addr. Limit |         Valid     Reserved    Reserved   Reserved     |  Unpredictable
// 0b01xx | Data Addr.         |         Valid     Debug Evt   Match      PC           |    x
// 0b0111 | Data Addr. Limit   |         Valid     Reserved    Reserved   DAddr        |  Unpredictable
// 0b10xx | Data Value         |         Valid     Debug Evt   Match      DValue       |    x
// 0b1011 | Linked Data Value  |         Valid     Reserved    Reserved   DValue       |  Unpredictable
// 0b11xx | Data Addr. + Value |        Reserved   Reserved    DValue     PC + DValue  |    x
// 0b1111 | Reserved           |        Reserved   Reserved    Reserved   Reserved     |  Reserved

// DWT Functon Register Defines (v8-M)
#define DWTv8_DISABLED     0x00000000                   // Comparator Disabled
#define DWTv8_FUNCTION_ID  0xF8000000                   // Function ID Mask (Caps)
#define DWTv8_ID_LIMIT     0x80000000                   // Limit Address Support
#define DWTv8_ID_DADDR     0x40000000                   // Data Address Support
#define DWTv8_ID_DVAL      0x20000000                   // Data Value Support (Always combined with DWTv8_ID_LIMIT)
#define DWTv8_ID_IADDR     0x10000000                   // Instruction Address Support
#define DWTv8_ID_CYCCNT    0x08000000                   // Data Value Support
#define DWTv8_MATCHED      0x01000000                   // Sticky Comparator Match Bit
#define DWTv8_DATAVSIZE    0x00000C00                   // Data Value Match Size Mask
#define DWTv8_DATAVSIZE_P  10                           // Data Value Match Size Position
#define DWTv8_DATAVSIZEB   0x00000000                   // Data Value Match Size - Byte
#define DWTv8_DATAVSIZEH   0x00000400                   // Data Value Match Size - Half word
#define DWTv8_DATAVSIZEW   0x00000800                   // Data Value Match Size - Word
#define DWTv8_ACTION       0x00000030                   // Comparator Action Mask
#define DWTv8_ACTION_P     4                            // Comparator Action Position
#define DWTv8_TRIGGER      0x00000000                   // Comparator Action - Trigger Only (CMPMATCH[n])
#define DWTv8_DBGEVT       0x00000010                   // Comparator Action - Debug Event
#define DWTv8_TRACE0       0x00000020                   // Comparator Action - Trace Action 0 (see table)
#define DWTv8_TRACE1       0x00000030                   // Comparator Action - Trace Action 1 (see table)
#define DWTv8_MATCH        0x0000000F                   // Comparator Match Mask
#define DWTv8_CYCLE        0x00000001                   // Comparator Match - Cycle Counter
#define DWTv8_IADDR        0x00000002                   // Comparator Match - Instruction Address
#define DWTv8_IADDR_LIMIT  0x00000003                   // Comparator Match - Instruction Address Limit / Linked Instruction Address
#define DWTv8_DADDR_RW     0x00000004                   // Comparator Match - Data Address (Read/Write Access)
#define DWTv8_DADDR_W      0x00000005                   // Comparator Match - Data Address (     Write Access)
#define DWTv8_DADDR_R      0x00000006                   // Comparator Match - Data Address (Read       Access)
#define DWTv8_DADDR_LIMIT  0x00000007                   // Comparator Match - Data Address Limit / Linked Data Address
#define DWTv8_DVALUE_RW    0x00000008                   // Comparator Match - Data Value   (Read/Write Access)
#define DWTv8_DVALUE_W     0x00000009                   // Comparator Match - Data Value   (     Write Access)
#define DWTv8_DVALUE_R     0x0000000A                   // Comparator Match - Data Value   (Read       Access)
#define DWTv8_DVALUE_LINK  0x0000000B                   // Comparator Match - Linked Data Value
#define DWTv8_DADDR_VAL_RW 0x0000000C                   // Comparator Match - Data Address + Value (Read/Write Access)
#define DWTv8_DADDR_VAL_W  0x0000000D                   // Comparator Match - Data Address + Value (     Write Access)
#define DWTv8_DADDR_VAL_R  0x0000000E                   // Comparator Match - Data Address + Value (Read       Access)
#define DWTv8_FUNCTION_CFG (DWTv8_MATCH | DWTv8_ACTION) // Comparat Match + Action Configuration

#define DWTv8_MATCH_N_PC   0x00000001 // Match/PC Trace Packet Payload indicates Match Packet


// Instrumentation Trace Macrocell (ITM) Registers
#define ITM_STIMULUS(n)    (ITM_Addr + (n << 2))
#define ITM_TRACEENABLE    (ITM_Addr + 0x0E00)
#define ITM_TRACEPRIVILEGE (ITM_Addr + 0x0E40)
#define ITM_TRACECONTROL   (ITM_Addr + 0x0E80)
#define ITM_ITWRITE        (ITM_Addr + 0x0EF8) // ITM Integration Write Register
#define ITM_ITREAD         (ITM_Addr + 0x0EFC) // ITM Integration Read Register
#define ITM_ITMODE         (ITM_Addr + 0x0F00) // ITM Integration Mode Register
#define ITM_LOCKACCESS     (ITM_Addr + 0x0FB0)
#define ITM_LOCKSTATUS     (ITM_Addr + 0x0FB4)

// ITM Trace Control Register definitions
#define ITM_ITMENA    0x00000001 // Enable ITM
#define ITM_TSENA     0x00000002 // Enable Timestamp
#define ITM_SYNCENA   0x00000004 // Enable Sync packets for TPIU
#define ITM_DWTENA    0x00000008 // Enable DWT stimulus
#define ITM_SWOENA    0x00000010 // Enable SWV (count on TPIUEMIT & TPIUBAUD)
#define ITM_TSPRESC   0x00000300 // Timestamp Prescaler Mask
#define ITM_TSPRESC4  0x00000100 // Timestamp Prescaler: Divide by 4
#define ITM_TSPRESC16 0x00000200 // Timestamp Prescaler: Divide by 16
#define ITM_TSPRESC64 0x00000300 // Timestamp Prescaler: Divide by 64
#define ITM_ATBIDMSK  0x007F0000 // ATBID Mask
// ITM Trace Control Register definitions - v8-M Extensions
#define ITM_TXENA    0x00000008 // Enable DWT stimulus (same as ITM_DWTENA)
#define ITM_STALLENA 0x00000020 // Enable Processor Stall on FIFO full
#define ITM_BUSY     0x00800000 // Busy Flag, DWT or ITM generate packets

// ITM Integration Write Register definitions
#define ITM_ITWRITE_ATVALIDM 0x00000001 // ATVALID master signal

// ITM Integration Read Register definitions
#define ITM_ITREAD_ATREADYM 0x00000001 // ATREADY master signal

// ITM Integration Mode Register definitions
#define ITM_ITMODE_EN 0x00000001 // Enable Integration Mode

#define ITM_ATBID     1 // ATBID

#define ITM_UNLOCK    0xC5ACCE55 // ITM Unlock Code for ITM_LOCKACCESS


// Embedded Trace Macrocell (ETM) Registers, ETM v3.x
#define ETMv3_CONTROL     (ETM_Addr + 0x0000) // ETM Main Control Register
#define ETMv3_CFGCODE     (ETM_Addr + 0x0004) // ETM Configuration Code Register
#define ETMv3_TRIGEVT     (ETM_Addr + 0x0008) // ETM Trigger Event Register
#define ETMv3_STATUS      (ETM_Addr + 0x0010) // ETM Status Register
#define ETMv3_SYSCFG      (ETM_Addr + 0x0014) // ETM Sytem Configuration Register
#define ETMv3_TRCENEVT    (ETM_Addr + 0x0020) // ETM TraceEnable Event Register
#define ETMv3_TRCENCTRL1  (ETM_Addr + 0x0024) // ETM TraceEnable Control 1 Register
#define ETMv3_FIFOFULLLEV (ETM_Addr + 0x002C) // ETM FIFO Full Level Register
#define ETMv3_SYNCFREQ    (ETM_Addr + 0x01E0) // ETM Synchronization Frequency Register
#define ETMv3_ETM_ID      (ETM_Addr + 0x01E4) // ETM ID Register
#define ETMv3_CFGCODEEXT  (ETM_Addr + 0x01E8) // ETM Configuration Code Extension Register
#define ETMv3_TSS_ICE     (ETM_Addr + 0x01F0) // ETM TraceEnable Start/Stop EmbeddedICE Control (DWT Match Signal Inputs) Register
#define ETMv3_TRACE_ID    (ETM_Addr + 0x0200) // ETM CoreSight Trace ID Register
#define ETMv3_PDSR        (ETM_Addr + 0x0314) // ETM Device Power-Down Status Register
#define ETMv3_ITATBCTR2   (ETM_Addr + 0x0EF0) // ETM Integration Test ATB Control 2 Register
#define ETMv3_ITATBCTR0   (ETM_Addr + 0x0EF8) // ETM Integration Test ATB Control 0 Register
#define ETMv3_ITCTRL      (ETM_Addr + 0x0F00) // ETM Integration Mode Control Register
#define ETMv3_LOCKACCESS  (ETM_Addr + 0x0FB0) // ETM Lock Access Register
#define ETMv3_LOCKSTATUS  (ETM_Addr + 0x0FB4) // ETM Lock Status Register
#define ETMv3_AUTHSTATUS  (ETM_Addr + 0x0FB8) // ETM Authentication Status Register

#define ETM_ATBID         2 // ATBID (ETMv4 implicitly allocating (ETM_ATBID + 1) for data trace)

#define ETM_UNLOCK        0xC5ACCE55 // ETM Unlock Code for ETM_LOCKACCESS (ETM v3 and v4)

// ETM Control Register definitions
#define ETMv3_ETMEN     0x00000800 // ETM Enable
#define ETMv3_PROGRAM   0x00000400 // ETM Programming
#define ETMv3_DBGREQ    0x00000200 // Debug Request Control
#define ETMv3_BRANCHOUT 0x00000100 // Branch Output
#define ETMv3_STALLPROC 0x00000080 // Stall Processor
#define ETMv3_PORTMASK  0x00232070 // Port Mode and Size Mask
#define ETMv3_PORTMODE  0x00002000 // 1:1 Half-rate Clock
#define ETMv3_PORTSIZE8 0x00000010 // 8-bit Port Size
#define ETMv3_POWERDOWN 0x00000001 // ETM Power down

// ETM Configuration Code Register definitions
#define ETMv3_CFGCODE_SS 0x04000000 // ETM Start/Stop present

// ETM Configuration Code Extension Register definitions
#define ETMv3_CFGCODEEXT_NUM_ICE_M 0x000F0000 // Number of Embedded ICE inputs mask
#define ETMv3_CFGCODEEXT_NUM_ICE_P 16         // Number of Embedded ICE inputs position
#define ETMv3_CFGCODEEXT_SS_ICE    0x00100000 // ETM Start/Stop logic uses Embedded ICE input

// ETM Trigger Event definitions
#define ETMv3_EVT_DWT   0x20      // DWT Comparator
#define ETMv3_EVT_TSS   0x5F      // Trace Start/Stop
#define ETMv3_EVT_EXT   0x60      // ExtIn
#define ETMv3_EVT_TRUE  0x6F      // True (HardWired)
#define ETMv3_RES_A     0         // Resource A Position
#define ETMv3_RES_B     7         // Resource B Position
#define ETMv3_FNC_A     (0 << 14) // Function: A
#define ETMv3_NOT_A     (1 << 14) // Function: NOT(A)
#define ETMv3_A_AND_B   (2 << 14) // Function: A AND B
#define ETMv3_NA_AND_B  (3 << 14) // Function: NOT(A) AND B
#define ETMv3_NA_AND_NB (4 << 14) // Function: NOT(A) AND NOT(B)
#define ETMv3_A_OR_B    (5 << 14) // Function: A OR B
#define ETMv3_NA_OR_B   (6 << 14) // Function: NOT(A) OR B
#define ETMv3_NA_OR_NB  (7 << 14) // Function: NOT(A) OR NOT (B)

// ETM Status Register definitions
#define ETMv3_TRIG_FLG 0x00000008 // Trigger Flag
#define ETMv3_SS_STAT  0x00000004 // Start/Stop Status
#define ETMv3_PRG_BIT  0x00000002 // Programming Bit Status
#define ETMv3_UT_OVF   0x00000001 // Untraced Overflow

// ETM Trace Control 1 Register definitions
#define ETMv3_SS_EN 0x02000000 // Trace Start/Stop Enable

// ETM Power-Down Status Register definitions
#define ETMv3_POWERUP 0x00000001 // ETM Power-Up Bit

// ETM Integration Test ATB Control 2 Register definitions
#define ETMv3_ITATBCTR2_ATREADY 0x00000001 // ATREADY signal

// ETM Integration Test ATB Control 0 Register definitions
#define ETMv3_ITATBCTR0_ATVALID 0x00000001 // ATVALID signal

// ETM Integration Mode Control Register definitions
#define ETMv3_ITCTRL_EN 0x00000001 // Enable integration mode


// Embedded Trace Macrocell (ETM) Registers, ETM v4.0
#define ETMv4_PROGCTRL      (ETM_Addr + 0x0004) // Programming Control Register
#define ETMv4_PROCSEL       (ETM_Addr + 0x0008) // PE Select Control Register
#define ETMv4_STATUS        (ETM_Addr + 0x000C) // Trace Status Register
#define ETMv4_CFG           (ETM_Addr + 0x0010) // Trace Configuration Register
#define ETMv4_AUXCTRL       (ETM_Addr + 0x0018) // Auxiliary Control Register (implementation defined)
#define ETMv4_EVENTCTRL0    (ETM_Addr + 0x0020) // Event Control 0 Register
#define ETMv4_EVENTCTRL1    (ETM_Addr + 0x0024) // Event Control 0 Register
#define ETMv4_STALLCTRL     (ETM_Addr + 0x002C) // Stall Control Register
#define ETMv4_GTSCTRL       (ETM_Addr + 0x0030) // Global Timestamp Control Register
#define ETMv4_SYNCPERIOD    (ETM_Addr + 0x0034) // Synchronization Period Register
#define ETMv4_CYCCNTCTRL    (ETM_Addr + 0x0038) // Cycle Count Control Register

#define ETMv4_BRANCHCTRL    (ETM_Addr + 0x003C) // Branch Broadcast Control Register
#define ETMv4_TRACE_ID      (ETM_Addr + 0x0040) // Trace ID Register
#define ETMv4_QCTRL         (ETM_Addr + 0x0044) // Q Element Control Register

#define ETMv4_VICTRL        (ETM_Addr + 0x0080) // ViewInst Main Control Register
#define ETMv4_VIIECTRL      (ETM_Addr + 0x0084) // ViewInst Include/Exclude Register
#define ETMv4_VISSCTRL      (ETM_Addr + 0x0088) // ViewInst Start/Stop Control Register
#define ETMv4_VIPCSSCTRL    (ETM_Addr + 0x008C) // ViewInst PE Comparator Control Register

#define ETMv4_EXTINSELR     (ETM_Addr + 0x0120) // External Input Select Register

#define ETMv4_IDR8          (ETM_Addr + 0x0180) // ID Register  8 (Features)
#define ETMv4_IDR9          (ETM_Addr + 0x0184) // ID Register  9 (Features)
#define ETMv4_IDR10         (ETM_Addr + 0x0188) // ID Register 10 (Features)
#define ETMv4_IDR11         (ETM_Addr + 0x018C) // ID Register 11 (Features)
#define ETMv4_IDR12         (ETM_Addr + 0x0190) // ID Register 12 (Features)
#define ETMv4_IDR13         (ETM_Addr + 0x0194) // ID Register 13 (Features)
#define ETMv4_IDR0          (ETM_Addr + 0x01E0) // ID Register  0 (Features)
#define ETMv4_IDR1          (ETM_Addr + 0x01E4) // ID Register  1 (Features)
#define ETMv4_IDR2          (ETM_Addr + 0x01E8) // ID Register  2 (Features)
#define ETMv4_IDR3          (ETM_Addr + 0x01EC) // ID Register  3 (Features)
#define ETMv4_IDR4          (ETM_Addr + 0x01F0) // ID Register  4 (Features)
#define ETMv4_IDR5          (ETM_Addr + 0x01F4) // ID Register  5 (Features)
#define ETMv4_IDR6          (ETM_Addr + 0x01F8) // ID Register  6 (Features)
#define ETMv4_IDR7          (ETM_Addr + 0x01FC) // ID Register  7 (Features)

#define ETMv4_PWRDOWNCTRL   (ETM_Addr + 0x0310) // PowerDown Control Register
#define ETMv4_PWRDOWNSTATUS (ETM_Addr + 0x0314) // PowerDown Control Status

#define ETMv4_LOCKACCESS    (ETM_Addr + 0x0FB0) // Software Lock Access Register
#define ETMv4_LOCKSTATUS    (ETM_Addr + 0x0FB4) // Software Lock Status Register
#define ETMv4_AUTHSTATUS    (ETM_Addr + 0x0FB8) // Authentication Status Register

// ETM Programming Control Register definitions
#define ETMv4_ETMEN 0x00000001 // ETM Enable

// ETM Trace Status Register
#define ETMv4_IDLE     0x00000001 // ETM Idle
#define ETMv4_PMSTABLE 0x00000002 // Programmer's Model Stable

// ETM Configuration Register definitions
#define ETMv4_INSTP0       0x00000006 // Instruction P0 Config Mask
#define ETMv4_INSTP0_LOAD  0x00000002 // Instruction P0: Trace Load instructions as P0
#define ETMv4_INSTP0_STORE 0x00000004 // Instruction P0: Trace Store instructions as P0
#define ETMv4_INSTP0_LS    0x00000006 // Instruction P0: Trace Load and Store instructions as P0
#define ETMv4_BRANCHBC_EN  0x00000008 // Branch Broadcast mode enable
#define ETMv4_CYCCNT_EN    0x00000010 // Cycle Count trace enable
#define ETMv4_CID_EN       0x00000040 // Context ID trace enable
#define ETMv4_VMID_EN      0x00000080 // VMID trace enable
#define ETMv4_COND         0x00000700 // Conditional instruction trace mask
#define ETMv4_COND_LOAD    0x00000100 // Conditional instruction trace: Load instructions
#define ETMv4_COND_STORE   0x00000200 // Conditional instruction trace: Store instructions
#define ETMv4_COND_LS      0x00000300 // Conditional instruction trace: Load/Store instructions
#define ETMv4_COND_ALL     0x00000700 // Conditional instruction trace: all conditional instructions
#define ETMv4_GTS_EN       0x00000800 // Global Timestamps enable
#define ETMv4_RETSTACK_EN  0x00001000 // Return Stack enable
#define ETMv4_QE           0x00006000 // Q Element enabe mask
#define ETMv4_DA_EN        0x00010000 // Data Address trace enable
#define ETMv4_DV_EN        0x00020000 // Data Value trace enable


// ETMv4 Stall Control Register
#define ETMv4_STALL_LEVEL_MASK 0x0000000F // Stall Threshold Level (Suppress Global Timestamps and Cycle Count + threshold for other enabled stall features)

// ETMv4 ViewInst Main Control Register definitions
#define ETMv4_VI_EXLEVEL_NS_MASK 0x00F00000 // Suppress instruction trace on EXLEVELs in Non-Secure state
#define ETMv4_VI_EXLEVEL_S_MASK  0x000F0000 // Suppress instruction trace on EXLEVELs in Secure state
#define ETMv4_VI_TRCERR          0x00000800 // Trace System errors
#define ETMv4_VI_TRCRESET        0x00000400 // Trace Reset exceptions
#define ETMv4_VI_SSTATUS         0x00000200 // Start/Stop Logic state
#define ETMv4_VI_EVENT_MASK      0x000000FF // Start/Stop Logic state

// ETM Synchronization Period Register definitions
#define ETMv4_PERIOD 0x0000000F // Trace Synchronization Period Mask

// PowerDown Control Register
#define ETMv4_POWERUP 0x00000008 // PowerUp bit

// PowerDown Control Status
#define ETMv4_PWRDOWN_POWER    0x00000001 // OS Lock locked
#define ETMv4_PWRDOWN_STICKYPD 0x00000002 // OS Lock locked
#define ETMv4_PWRDOWN_OSLOCK   0x00000020 // OS Lock locked

// ETMv4 IDR0 Register Definitions
#define ETMv4_COMMOPT_M  0x20000000 // Commit Mode [0|1] Mask, affects Cycle Count Packets
#define ETMv4_COMMOPT_P  29         // Commit Mode [0|1] Position, affects Cycle Count Packets
#define ETMv4_QSUPP_M    0x00018000 // Q Element Support Mask
#define ETMv4_QSUPP_P    15         // Q Element Support Position
#define ETMv4_CONDTYPE_M 0x00003000 // Condition Type Mask     (0-Pass/Fail, 1-APSR Flags)
#define ETMv4_CONDTYPE_P 12         // Condition Type Position (0-Pass/Fail, 1-APSR Flags)
#define ETMv4_NUMEVENT_M 0x00000600 // Number Of Events Mask
#define ETMv4_NUMEVENT_P 10         // Number Of Events Position
#define ETMv4_RETSTACK_M 0x00000200 // Return Stack Support Mask
#define ETMv4_RETSTACK_P 9          // Return Stack Support Position
#define ETMv4_TRCDATA_M  0x00000018 // Data Trace Support Mask
#define ETMv4_TRCDATA_P  3          // Data Trace Support Position

// ETMv4 IDR3 Register Definitions
#define ETMv4_STALLCTRLR 0x04000000 // Stall Control Register supported (TRCSTALLCTRL)
#define ETMv4_SYNCPRFIX  0x02000000 // Sync Period is fixed and cannot be changed
#define ETMv4_CCITMIN    0x00000FFF // Minimum Cycle Counter Threshold (Instruction Trace)

// ETMv4 Packet Definitions
#define ETMv4_INSTR_LOAD  0x01 // Load Instruction (Trace Info INFO.cond_enabled, INFO.p0_load/p0_store)
#define ETMv4_INSTR_STORE 0x02 // Store Instruction (Trace Info INFO.cond_enabled, INFO.p0_load/p0_store)
#define ETMv4_INSTR_ANY   0x04 // Any Instruction (Trace Info INFO.cond_enabled)



// Trace Port Interface Unit (TPIU) Registers
#define TPIU_SUPPSYNPORTSZ (TPIU_Location.Addr + 0x0000)
#define TPIU_CURRSYNPORTSZ (TPIU_Location.Addr + 0x0004)
#define TPIU_ASYNCLKPRES   (TPIU_Location.Addr + 0x0010)
#define TPIU_PINPROTOCOL   (TPIU_Location.Addr + 0x00F0)
#define TPIU_FMTFLSHCTRL   (TPIU_Location.Addr + 0x0304)
// Trace Port Interface Unit (TPIU) Registers - v8-M Extensions
#define TPIU_PSCR (TPIU_Location.Addr + 0x0308)

// TPIU Pin Protocol
#define TPIU_TRACE_PORT     0 // Trace Port Mode
#define TPIU_SWO_MANCHESTER 1 // SWO Manchester Protocol
#define TPIU_SWO_UART       2 // SWO UART Protocol
#define TPIU_ETB            3 // Embedded Trace Buffer (not a TPIU!)

// Debug Block Addresses
extern DWORD DBG_Addr; // Core Debug Base Address
extern DWORD DWT_Addr; // DWT Base Address
extern DWORD FPB_Addr; // FPB Base Address
extern DWORD ITM_Addr; // ITM Base Address
//extern DWORD TPIU_Addr;            // TPIU/SWO Base Address
extern DWORD ETM_Addr;  // ETM Base Address
extern DWORD NVIC_Addr; // NVIC Base Address
// CS Location (Potentially Shared in Multi-Core System)
extern CS_LOCATION TPIU_Location; // TPIU/SWO Base Location

// System Block Addresses
extern DWORD Cache_Addr; // Cache Base Address (i.e. Cortex-M35P I-Cache present in ROM Table)

// Trace Interface Types (not all are TPIUs but share programming interface)
#define TPIU_TYPE_CM   0 // Cortex-M TPIU           (Trace Port and SWO modes)
#define TPIU_TYPE_CS   1 // Standard CoreSight TPIU (Trace Port mode only)
#define TPIU_TYPE_SWO  2 // SWO                     (SWO mode only)
#define TPIU_TYPE_LITE 3 // TPIU Lite               (Trace Port mode only, no formatter)

// Debug Block Versions
extern DWORD TPIU_Type;   // Trace Interface Type (TPIU_TYPE_CM, TPIU_TYPE_CS, TPIU_TYPE_SWO)
extern DWORD ETM_Version; // ETM Version, can be 3 (Cortex-M3/M4) or 4 (Cortex-M7)


// Memory Access Attributes
#define BLOCK_NADDRINC        0x1 // No Address Increment (e.g. for ETB Data Register Polling)
#define BLOCK_SECTYPE         0x6 // Access Security Type
#define BLOCK_SECTYPE_P       1   // Access Security Type Position
#define BLOCK_SECTYPE_ANY     0x0 // Default, debugger decides, use for normal debug access to \
                                  // unbanked memory/memory-mapped registers                   \
                                  //   - Currently configured SCS register view                \
                                  //   - Secure access on MEM-AP
#define BLOCK_SECTYPE_NSECURE 0x2 // Non-Secure Access, use to force a non-secure access to \
                                  // all memory/memory-mapped registers                     \
                                  //   - Non-Secure SCS register view                       \
                                  //   - Non-Secure access on MEM-AP
#define BLOCK_SECTYPE_SECURE  0x4 // Secure Access, use to force a secure access to all \
                                  // memory/memory-mapped registers                     \
                                  //   - Secure SCS register view                       \
                                  //   - Secure access on MEM-AP
#define BLOCK_SECTYPE_CPU     0x6 // CPU Security State, use to access banked memory/memory-mapped \
                                  // registers as per current CPU state                            \
                                  //   - SCS register view as per CPU state                        \
                                  //   - Secure access on MEM-AP

// Debug Functions
extern int Debug_Init(void);                // Init Debugger
extern int Debug_UnInit(void);              // UnInit Debugger
extern int (*DAPAbort)(void);               // DAP Abort
extern int (*ReadDP)(BYTE adr, DWORD *val); // Read DP Register
extern int (*WriteDP)(BYTE adr, DWORD val); // Write DP Register
extern int (*ReadAP)(BYTE adr, DWORD *val); // Read AP Register
extern int (*WriteAP)(BYTE adr, DWORD val); // Write AP Register
#if DBGCM_V8M
extern int (*ReadD32)(DWORD adr, DWORD *val, BYTE attrib);                           // Read 32-bit Data
extern int (*WriteD32)(DWORD adr, DWORD val, BYTE attrib);                           // Write 32-bit Data
extern int (*ReadD16)(DWORD adr, WORD *val, BYTE attrib);                            // Read 16-bit Data
extern int (*WriteD16)(DWORD adr, WORD val, BYTE attrib);                            // Write 16-bit Data
extern int (*ReadD8)(DWORD adr, BYTE *val, BYTE attrib);                             // Read 8-bit Data
extern int (*WriteD8)(DWORD adr, BYTE val, BYTE attrib);                             // Write 8-bit Data
extern int (*ReadBlock)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);              // Read Data Block
extern int (*WriteBlock)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);             // Write Data Block
extern int (*VerifyBlock)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);            // Verify Data Block
extern int (*ReadARMMem)(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);           // Read ARM Memory
extern int (*WriteARMMem)(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);          // Write ARM Memory
extern int (*VerifyARMMem)(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);         // Verify ARM Memory
extern int (*GetARMRegs)(RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask); // Get ARM Registers
extern int (*SetARMRegs)(RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask); // Set ARM Registers
#else                                                                                // DBGCM_V8M
extern int (*ReadD32)(DWORD adr, DWORD *val);                            // Read 32-bit Data
extern int (*WriteD32)(DWORD adr, DWORD val);                            // Write 32-bit Data
extern int (*ReadD16)(DWORD adr, WORD *val);                             // Read 16-bit Data
extern int (*WriteD16)(DWORD adr, WORD val);                             // Write 16-bit Data
extern int (*ReadD8)(DWORD adr, BYTE *val);                              // Read 8-bit Data
extern int (*WriteD8)(DWORD adr, BYTE val);                              // Write 8-bit Data
extern int (*ReadBlock)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);  // Read Data Block
extern int (*WriteBlock)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib); // Write Data Block
extern int (*VerifyBlock)(DWORD adr, BYTE *pB, DWORD nMany);             // Verify Data Block
extern int (*ReadARMMem)(DWORD *nAdr, BYTE *pB, DWORD nMany);            // Read ARM Memory
extern int (*WriteARMMem)(DWORD *nAdr, BYTE *pB, DWORD nMany);           // Write ARM Memory
extern int (*VerifyARMMem)(DWORD *nAdr, BYTE *pB, DWORD nMany);          // Verify ARM Memory
extern int (*GetARMRegs)(RgARMCM *regs, RgARMFPU *rfpu, U64 mask);       // Get ARM Registers
extern int (*SetARMRegs)(RgARMCM *regs, RgARMFPU *rfpu, U64 mask);       // Set ARM Registers
#endif                                                                               // DBGCM_V8M
extern int (*SysCallExec)(RgARMCM *regs);                                            // System Call Execute
extern int (*SysCallRes)(DWORD *rval);                                               // System Call Result
extern int (*TestSizesAP)(void);                                                     // Test Sizes Supported in AP CSW
extern int (*DAPAbortVal)(DWORD val);                                                // DAP Abort with value to write
extern int (*ReadBlockD8)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);            // Read Data Block of nMany 8-bit accesses
extern int (*ReadBlockD16)(DWORD adr, U16 *pB, DWORD nMany, BYTE attrib);            // Read Data Block of nMany 16-bit accesses
extern int (*ReadBlockD32)(DWORD adr, U32 *pB, DWORD nMany, BYTE attrib);            // Read Data Block of nMany 32-bit accesses
extern int (*WriteBlockD8)(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib);           // Write Data Block of nMany 8-bit accesses
extern int (*WriteBlockD16)(DWORD adr, U16 *pB, DWORD nMany, BYTE attrib);           // Write Data Block of nMany 16-bit accesses
extern int (*WriteBlockD32)(DWORD adr, U32 *pB, DWORD nMany, BYTE attrib);           // Write Data Block of nMany 32-bit accesses
extern int (*ReadARMMemD8)(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);         // Read ARM Memory (nMany 8-bit accesses)
extern int (*ReadARMMemD16)(DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib);         // Read ARM Memory (nMany 16-bit accesses)
extern int (*ReadARMMemD32)(DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib);         // Read ARM Memory (nMany 32-bit accesses)
extern int (*WriteARMMemD8)(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib);        // Write ARM Memory (nMany 8-bit accesses)
extern int (*WriteARMMemD16)(DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib);        // Write ARM Memory (nMany 16-bit accesses)
extern int (*WriteARMMemD32)(DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib);        // Write ARM Memory (nMany 32-bit accesses)
extern int (*SwitchDP)(DWORD id, bool force);                                        // Switch DP
extern int (*SWJ_Sequence)(int cnt, U64 val);                                        // Execute SWJ (SWDIO_TMS) Sequence
extern int (*SWJ_Clock)(BYTE cid, BOOL rtck);                                        // Change Debug Clock Frequency
extern WORD  Swap16(WORD v);                                                         // 16-bit Endian Swap
extern DWORD Swap32(DWORD v);                                                        // 32-bit Endian Swap
extern int   ROM_Table(DWORD ptr);                                                   // Read ROM Table
extern BYTE  OverlapSCSv8M(DWORD adr, DWORD many);                                   // Address in v8-M SCS Register Range
extern int   AP_ReadID(void);                                                        // Read and evaluate selected Access Port ID
extern DWORD AP_CurrentRWPage(void);                                                 // Get current auto-address increment size
extern int   AP_CurrentCtx(AP_CONTEXT **apCtx);                                      // Get Current AP Context
extern int   AP_CpuCtx(AP_CONTEXT **apCtx);                                          // Get AP Context for debugged CPU
extern int   AP_Switch(AP_CONTEXT **apCtx);                                          // Switch to currently selected AP and get its context

#endif
