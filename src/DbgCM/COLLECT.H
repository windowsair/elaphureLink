/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.1.18
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 * 
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
 *
 * @brief     UV specific headerfile, needed for AGDI 
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

#ifndef __COLLECT_INCED__
  #define __COLLECT_INCED__


// Choose between this three #defines when developing the driver
//#define DEVELOP_MSG             AfxMessageBox
//#define DEVELOP_MSG             OutputDebugString
#define DEVELOP_MSG             txtout

// Optional Feature Support
//   Advanced DLL Capabilities
#define DBGCM_V8M                 1   // Support for ARM v8-M Architecture
#define DBGCM_MEMACCX             1   // Support for Extended Memory Access Interface (required for v8-M Support!)
#define DBGCM_DBG_DESCRIPTION     1   // Support for CMSIS Pack Debug Description, requires DBGCM_MEMACCX
#define DBGCM_DS_MONITOR          1   // Support for Device State Monitor
#define DBGCM_WITHOUT_STOP        1   // Support for "without Stop" Connection Type, requires DBGCM_DS_MONITOR
#define DBGCM_RECOVERY            1   // Support for Recovery from Target Connection Loss

#if (DBGCM_DBG_DESCRIPTION && !DBGCM_MEMACCX)
  #error "You must also enable feature DBGCM_MEMACCX if using feature DBGCM_DBG_DESCRIPTION"
#endif
#if (DBGCM_WITHOUT_STOP && !DBGCM_DS_MONITOR)
  #error "You must also enable feature DBGCM_DS_MONITOR if using feature DBGCM_WITHOUT_STOP"
#endif

//   Instruction Trace Based Features. Only activate if debug probe supports gapless ETM streaming trace.
#define DBGCM_FEATURE_ETM         0   // Support for ETM trace capture
#define DBGCM_FEATURE_COVERAGE    0   // Support for Code Coverage Features
#define DBGCM_FEATURE_PAPROF      0   // Support for Performance Analysis and Execution Profiling Features
#define DBGCM_FEATURE_TRCDATA_WIN 0   // Support for Trace Data window and related views

#include "..\AGDI.h"


// Error Codes
#define EU01         1            // Internal DLL Error
#define EU02         2            // No Debug Unit Found
#define EU03         3            // No JTAG Devices Found
#define EU04         4            // Too Many JTAG Devices in Chain
#define EU05         5            // JTAG Communication Error
#define EU06         6            // JTAG Device Chain Error
#define EU07         7            // JTAG RTCK Failure
#define EU08         8            // SWD Communication Error
#define EU09         9            // No Cortex-M Device found in JTAG chain
#define EU10         10           // No Cortex-M SW Device Found
#define EU11         11           // Device could not be powered up
#define EU12         12           // Invalid ROM Table
#define EU13         13           // Cannot enter Debug Mode
#define EU14         14           // Cannot access Memory
#define EU15         15           // Trace HW not present
#define EU16         16           // Trace available only on SW
#define EU17         17           // Selected SWO Clock not supported
#define EU18         18           // Not possible while target is executing
#define EU19         19           // Selected Trace Port is not supported (e.g. SWO in Manchester mode)
#define EU20         20           // Internal DLL Error: Unsupported Debug Protocol
#define EU21         21           // Unsupported Memory Access Size
#define EU22         22           // PDSC: Debug Description not available
#define EU23         23           // PDSC: Cannot stop target
#define EU24         24           // PDSC: Cannot recover from reset
#define EU25         25           // PDSC: Unknown Debug Port ID.\nCannot switch to Debug Port.
#define EU26         26           // PDSC: Sequence not implemented
#define EU27         27           // PDSC: Sequence Execution failed
#define EU28         28           // PDSC: Data Patch failed
#define EU29         29           // PDSC: Multiple SW Debug Port definitions.\nMulti-Drop SWD not supported.
#define EU30         30           // PDSC: Debug Port name redifinition
#define EU31         31           // PDSC: JTAG Debug Port ID Code redifinition
#define EU32         32           // PDSC: JTAG Debug Port IR Length redifinition
#define EU33         33           // PDSC: JTAG TAP Index out of bounds
#define EU34         34           // PDSC: Multiple Implementations valid for same Sequence Context
#define EU35         35           // PDSC: Unknown Sequence ID.\nCannot execute Sequence.
#define EU36         36           // PDSC: Debug Description disabled
#define EU37         37           // PDSC: Sequence disabled
#define EU38         38           // PDSC: No data patch available
#define EU39         39           // Parameter Error
#define EU40         40           // Cannot recover Debug Connection
#define EU41         41           // Atomic sequences not supported
#define EU42         42           // Atomic sequence assembly error
#define EU43         43           // Atomic sequence execution error
#define EU44         44           // Debug Server lost
#define EU45         45           // CPU locked up, stopping target
#define EU46         46           // Unknown CPU
#define EU47         47           // Cannot change security view while target executes
#define EU48         48           // Could not reset target with non-secure debugger
#define EU49         49           // TRACE_CLK is read-only for option 'Use Core Clock', use CORE_CLK instead /* 02.04.2019 */
#define EU50         50           // Invalid Access Port selected
#define EU51         51           // Unsupported Access Port
#define EU52         52           // Connection refused due to device mismatch!
#define EU53         53           // ETB Trace Error
#define EU54         54           // Trace flush failed
#define EU55         55           // No supported CPU behind selected debug access port


// xxCPU definitions
#define CPU_UNKNOWN  0            // Unknown CPU
#define ARM_CM0      1            // ARM Cortex-M0
#define ARM_CM0P     2            // ARM Cortex-M0+
#define ARM_CM1      3            // ARM Cortex-M1
#define ARM_CM3      4            // ARM Cortex-M3
#define ARM_CM4      5            // ARM Cortex-M4
#define ARM_SC000    6            // ARM SC000 (Secure Cortex-M0)
#define ARM_SC300    7            // ARM SC300 (Secure Cortex-M3)
#define ARM_CM7      8            // ARM Cortex-M7
#define ARM_CM23     9            // ARM Cortex-M23 (v8-M Baseline)
#define ARM_CM33    10            // ARM Cortex-M33 (v8-M Mainline)
#define ARM_CM35P   11            // ARM Cortex-M35P (v8-M Mainline with Physical Security)


// Memory Attributes

#define ATRX_EXEC    0x00000001   // 'executable' Attribute
#define ATRX_READ    0x00000002   // 'readable' Attribute
#define ATRX_WRITE   0x00000004   // 'writable' Attribute
#define ATRX_WATCH   0x00000008   // Location has a Watch

#define ATRX_THUMB   0x00000080   // iMCSARM: 'Thumb' code
#define ATRX_ARM     0x00008000   // iMCSARM: 'ARM' code

#define ATRX_WRBRK   0x00010000   // Loc has a write-access break
#define ATRX_RDBRK   0x00020000   // Loc has a read-access break
#define ATRX_PAP     0x00040000   // Location has a Perf.-Analyzer point
#define ATRX_NOINST  0x00080000   // iMCSARM: 'No instruction' Attribute

#define ATRX_EXECD   0x00000100   // 'Executed' Attribute
#define ATRX_ITAKEN  0x00000200   // iMCSARM: Instruction-taken Attribute
#define ATRX_BREAK   0x00000400   // 'Exec-Break' Attribute
#define ATRX_BPDIS   0x00000800   // 'disabled Exec-Break' Attribute
#define ATRX_EXECDO  0x01000000   // iMCSARM: 'Executed' Attribute for odd WORD address (Thumb)
#define ATRX_ITAKENO 0x02000000   // iMCSARM: Instruction-taken Attribute for odd WORD address (Thumb)
#define ATRX_BREAKO  0x04000000   // iMCSARM: 'Exec-Break' Attribute for odd WORD address (Thumb)
#define ATRX_BPDISO  0x08000000   // iMCSARM: 'disabled Exec-Break' Attribute for odd WORD address (Thumb)

#define ATRX_UPTD    0x10101010   // 'Up to date' Attribute for DWORD
#define ATRX_UPTD0   0x00000010   // 'Up to date' Attribute for DWORD-Byte0
#define ATRX_UPTD1   0x00001000   // 'Up to date' Attribute for DWORD-Byte1
#define ATRX_UPTD2   0x00100000   // 'Up to date' Attribute for DWORD-Byte2
#define ATRX_UPTD3   0x10000000   // 'Up to date' Attribute for DWORD-Byte3


// Extended Memory Management
struct EMM {
  DWORD  *mem;       // Pointer to Memory Image
  DWORD  *atr;       // Pointer to Memory Attributes
};

#define _MSGM (65536 + 4)


// Register Definitions

#define nR0     0x00            // R0
#define nR1     0x01            // R1
#define nR2     0x02            // R2
#define nR3     0x03            // R3
#define nR4     0x04            // R4
#define nR5     0x05            // R5
#define nR6     0x06            // R6
#define nR7     0x07            // R7
#define nR8     0x08            // R8
#define nR9     0x09            // R9
#define nR10    0x0A            // R10
#define nR11    0x0B            // R11
#define nR12    0x0C            // R12
#define nR13    0x0D            // R13 (SP)
#define nR14    0x0E            // R14 (LR)
#define nR15    0x0F            // R15 (PC)

#define nPSR    0x10            // xPSR
#define nMSP    0x11            // MSP (Main SP)
#define nPSP    0x12            // PSP (Processor SP)
#define nDSP    0x13            // DSP (Deep SP)
#define nSYS    0x14            // System Registers

// v8-M Security Extensions
#define nMSP_NS     0x15        // MSP_NS (Non-Secure MSP)
#define nPSP_NS     0x16        // PSP_NS (Non-Secure PSP)
#define nMSP_S      0x17        // MSP_S (Secure MSP)
#define nPSP_S      0x18        // PSP_S (Secure PSP)
#define nMSPLIM_S   0x19        // MSPLIM_S (Secure MSP Limit)
#define nPSPLIM_S   0x1A        // PSPLIM_S (Secure PSP Limit)
#define nMSPLIM_NS  0x1B        // MSPLIM_NS (Non-Secure MSP Limit)
#define nPSPLIM_NS  0x1C        // PSPLIM_NS (Non-Secure PSP Limit)
#define nSYS_S      0x1D        // System Registers (Secure)
#define nSYS_NS     0x1E        // System Registers (Non-Secure)

#define nFPSCR  0x1F            // FPU: FPSCR
#define nFPUSx  0x20            // FPU: S0..S31 (0x20..0x3F)

// FPU: S0..S31 ID's as defined in AGDI.h
#define nS0     0x90            // FPU: S0
#define nS1     0x91            // FPU: S1
#define nS2     0x92            // FPU: S2
#define nS3     0x93            // FPU: S3
#define nS4     0x94            // FPU: S4
#define nS5     0x95            // FPU: S5
#define nS6     0x96            // FPU: S6
#define nS7     0x97            // FPU: S7
#define nS8     0x98            // FPU: S8
#define nS9     0x99            // FPU: S9
#define nS10    0x9A            // FPU: S10
#define nS11    0x9B            // FPU: S11
#define nS12    0x9C            // FPU: S12
#define nS13    0x9D            // FPU: S13
#define nS14    0x9E            // FPU: S14
#define nS15    0x9F            // FPU: S15
#define nS16    0xA0            // FPU: S16
#define nS17    0xA1            // FPU: S17
#define nS18    0xA2            // FPU: S18
#define nS19    0xA3            // FPU: S19
#define nS20    0xA4            // FPU: S20
#define nS21    0xA5            // FPU: S21
#define nS22    0xA6            // FPU: S22
#define nS23    0xA7            // FPU: S23
#define nS24    0xA8            // FPU: S24
#define nS25    0xA9            // FPU: S25
#define nS26    0xAA            // FPU: S26
#define nS27    0xAB            // FPU: S27
#define nS28    0xAC            // FPU: S28
#define nS29    0xAD            // FPU: S29
#define nS30    0xAE            // FPU: S30
#define nS31    0xAF            // FPU: S31

#define mREGS   0x0017FFFF      // All Registers Mask (without DSP and FPU Regs)
#define mRFPU   0xFFFFFFFF80000000 // FPU Register Mask (FPCSR, S0..S31)
#define mRSEC   0x7FE00000      // Security Extensions (Banked Secure/Non-Secure Registers)

#define A1      Rn[0]           // R0
#define A2      Rn[1]           // R1
#define A3      Rn[2]           // R2
#define A4      Rn[3]           // R3
#define V1      Rn[4]           // R4
#define V2      Rn[5]           // R5
#define V3      Rn[6]           // R6
#define V4      Rn[7]           // R7
#define V5      Rn[8]           // R8
#define SB      Rn[9]           // R9  (V6)
#define SL      Rn[10]          // R10 (V7)
#define FP      Rn[11]          // R11 (V8)
#define IP      Rn[12]          // R12
#define SP      Rn[13]          // R13 (SP)
#define LR      Rn[14]          // R14 (LR)
#define PC      Rn[15]          // R15 (PC)

#define T_Bit   0x01000000      // T Bit Mask in xPSR


union mamap {
  UL32      a32;
  U16       w16;
  UC8     ub[4];
};
#define MAMAP   union mamap


// Monitor Options
#define CODE_VERIFY    0x0001
#define CACHE_CODE     0x0002
#define CACHE_MEM      0x0004
#define BOOT_RESET     0x0008
#define JTAG_MANUAL    0x0010
#define FLASH_LOAD     0x0020
#define PORT_SW        0x0040
#define USE_SWJ        0x0080
#define RESET_TYPE     0x0300
#define RESET_HW       0x0100
#define RESET_SW_SYS   0x0200
#define RESET_SW_VECT  0x0300  // Unsupported in v8-M
#define INIT_RST_PULSE 0x0400
#define INIT_RST_HOLD  0x0800
#define BOOT_RUN       0x1000
#define RST_VECT_CATCH 0x4000  // Permanent Reset Vector Catch
#define CONN_NO_STOP   0x8000  // Connection without stopping target


// SW/JTAG Clock ID
//   Bit 7: Use RTCK (JTAG Only)
//   Bit 6..0:
//     0..10 = 10MHz,5MHz,2MHz,1MHz,500kHz,200kHz,100kHz,50kHz,20kHz,10kHz,5kHz

// Permant Reset Vector Catch
#define MONCONF_RST_VECT_CATCH ((MonConf.Opt & RST_VECT_CATCH) ? VC_CORERESET : 0)

// Monitor Configuration
struct MONCONF  {
  char   DriverPath[MAX_PATH];   // Path name with terminating backslash
  char   UnitSerNo[10];          // Unit Serial Number
  DWORD  SWJ_Clock;              // SW/JTAG Clock ID
  DWORD  JtagCpuIndex;           // CPU index in JTAG chain
  DWORD  Opt;                    // Monitor Options
  DWORD  SFRStart;               // SFR Start Address
  DWORD  SFREnd;                 // SFR End Address
  BYTE   AP;                     // Access Port
}; 
extern struct MONCONF MonConf;


extern void  AnalyzeParms (char *pPath, char *pArgs);
extern void    WriteParms (char *pArgs);

extern int     DoDlgSetup (void);  // target-config setup dialog

extern U32   ReInitTarget (void);
extern void    StopTarget (void);
extern DWORD  ResetTarget (void);
extern void     ExitDebug (void);
extern void      InitRegs (void);
extern void       GetRegs (U64 mask);
#if DBGCM_V8M
extern void       SetRegs (RgARMCM *pR, RgARMFPU *pF, RgARMV8MSE *pS, U64 mask);
#else // DBGCM_V8M
extern void       SetRegs (RgARMCM *pR, RgARMFPU *pF, U64 mask);
#endif // DBGCM_V8M
extern void    Invalidate (void);         // Invalidate all cached target resources

// for DBGCM_WITHOUT_STOP
extern int UnInitRunningTarget (void);

// for DBGCM_DS_MONITOR
extern void     RunningCB (U32 info);


extern int   UpdateCycles (void);
extern void  UpdateAllDlg (void);

extern int   DetectSecurityExtensions (void);
extern int   UpdateSecurityState      (void);
extern int   UpdateDebugAuthentication(void);
extern __inline BOOL NonSecureCantStop(void);    // Non-Secure Debugger, CPU in Secure Mode

extern "C" struct bom     *pio;
extern struct dbgblk *pdbg;      // startup values

extern HWND         *pHwnd;
extern HWND          hMfrm;      // pointer to parent handle (CMainFrame)
extern HMODULE       hInst;      // this DLL's instance handle
extern UL32         Uv2Msg;      // Uv2-registered Message token
extern pCBF        pCbFunc;      // call-back function of sarm

extern BYTE           PlayDead;  // Disables the driver after the communication breaks down.
extern const char* PlayDeadMsg;  // Message to send if processing PlayDead.
extern BYTE           bootflag;  // Specifies whether first boot loader is in progress or not
extern U64         RegUpToDate;  // Specifies whether register structure is up to date
extern bool           bAnyUnit;  // Select "Any" (first) DbgCM Unit
extern BYTE    PlayDeadRegular;  // PlayDead set because of a regular shutdown


extern BYTE      SetupMode;      // 1:=Remote Setup Mode via Options-Debug

extern UC8          ReInit;      // Reinitializing target
extern UC8            iRun;      // target is currently executing
extern UC8         StopRun;      // Flag: stop execution
extern UC8          GoMode;
extern UC8          iBpCmd;      // currently executing breakpoint command
extern UL32        *pCURPC;

extern RgARMCM      RegARM;      // ARM Cortex-M Registers
extern RgARMFPU     RegFPU;      // FPU Cortex-M Registers

extern DWORD         xxCPU;      // CPU Type
extern BOOL           xFPU;      // FPU Flag
extern DWORD          nCPU;      // Selected CPU within the JTAG chain

extern RgARMV8MSE  RegV8MSE;     // ARM v8-M Security and Authentication Registers
extern BOOL       xMainline;     // v8-M CPU is Mainline, Baseline otherwise

extern UC8 ReInitInProgress;     // Reinit in progress

extern char *StatusText (int status);
extern void txtout   (char *fmt, ...);   // this is a debug 'printf'
extern void errtxtout  (int status);     // this is a debug 'printf' for an error
extern void warntxtout (int status);     // this is a debug 'printf' for a warning
extern void OutMsg   (char *txt);        // Output to Status Bar Pane (RTA)
extern void OutError (int status);       // Display Error Text 
extern void OutErrorDelayed (void);      // Display Error Text (after CheckCom is released!)
extern void OutErrorMessage (int status);
extern void OutTraceMsg     (BYTE msg);    // Display Trace Status in Status Bar Pane (RTA)

extern void SetPlayDead(const char* MsgTxt);  // Sets PlayDead and PlayDeadMsg, MsgTxt can be NULL
extern void ExecPlayDead(void);               // Do what's required to play dead
extern BOOL HardPlayDead(void);               // PlayDead but not a regular shutdown

extern U32  SendLaDataRecord(AGDI_LAREC *pLA);

extern DWORD *MGetAttr (DWORD nAdr);

extern BOOL IsV8M(void);

#define SEV_ERR_STOP         1                // Cannot stop target
#define SEV_ERR_RESET        2                // Cannot reset target
#define SEV_ERR_ACCESS       3                // Cannot access target
#define SEV_ERR_REINIT       4                // Cannot reinit target
extern int SevereTargetError(unsigned int err_type); // Report severe error, show warning if Non-Secure Debugger + Secure CPU


#define PWND ((CWnd*) CWnd::FromHandle(pio->hwnd))

/* Extended status information (i.e. to report addr on "Cannot access memory")
 */
#define STATUS_MEMREAD      1
#define STATUS_MEMWRITE     2

typedef struct {
  DWORD         valid :  1;
  DWORD         Res   : 31;
  DWORD               type;
  unsigned int       nAddr;
  unsigned int       nSize;
  unsigned int       nMany;
} statusInfoMem_t;

// see AGDI.cpp
extern  statusInfoMem_t statusInfoMem;
int     SetStatusMem   (int status, DWORD nAddr, DWORD type=0, int nSize=0);
int     ClearStatusMem (int status=0);

#endif

extern "C" void InitAGDI();
extern "C" void CrtInitBreakResources();
extern "C" void InitCSTF();
extern "C" void InitCTI();
extern "C" void InitDebug();
extern "C" void InitDebugAccess();
extern "C" void InitDSMonitor();
extern "C" void InitELF();
extern "C" void InitETB();
extern "C" void InitFlash();
extern "C" void InitJTAG();
extern "C" void InitPDSCDebug();
extern "C" void InitSetupDbg();
extern "C" void InitSetupFD();
extern "C" void InitSWD();
extern "C" void InitSWV();
extern "C" void InitTrace();
extern "C" void InitTraceExc();
extern "C" void InitTraceRec();
extern "C" void TraceWinConnect();

extern "C" int  LinkCom (int stat);
