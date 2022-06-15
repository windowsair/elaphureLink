/**************************************************************************/ /**
 *           AGDI.H: Advanced Generic Debug Interface (AGDI)
 *                   definitions.
 * 
 * @version  V1.0.46
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 1999-2009 KEIL, 2009-2020 ARM Limited. All rights reserved.
 * 
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision.
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

#ifndef __AGDI__INCED___
#define __AGDI__INCED___


//---Revision History: ----------------------------------------------
//   17.10.2000, added AG_SYM_SFR
//   15.01.2008, added AG_CB_MSGBOX
//-------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h> // wg. HWND, RECT, CALLBACK, HANDLE

#define _EXPO_ __declspec(dllexport)


#include "agdi_types.h"



typedef union {
    U32  u32; // 32-Bit signed int
    I32  i32; // 32-Bit unsigned int
    UL32 ul;  // 32-Bit unsigned long
    SL32 sl;  // 32-Bit signed long
    UC8  uc;  // 8-Bit unsigned char
    SC8  sc;  // 8-Bit signed char
    U16  u16; // 16-Bit unsigned short int
    I16  i16; // 16-Bit signed short int
    U64  u64; // 64-Bit unsigned int
    I64  i64; // 64-Bit signed int
    F32  f32; // 32-Bit single precision float
    F64  f64; // 64-Bit single precision float
    UL32 ul2[2];
    SL32 sl2[2];
    U16  u16a[4];
    I16  i16a[4];
    UC8  uc8[8];
    SC8  sc8[8];
    SC8 *pS;
    UC8 *pU;
    U16 *pW;
    U32 *pD;
} GVAL;


/*
 * 166/167 specific's
 */

#define mWREG 0x100 // 0x100...0x10F := R0...R15
#define mBREG 0x200 // 0x200...0x20F := RL0...RH7
#define mDREG 0x300 // not used.
#define mDPP  0x400 // 0x400...0x403 := DPP0...DPP3
#define mPC   0x500 // PC
#define mPSW  0x600 // PSW
#define mSP   0x700 // SP
#define mCP   0x800 // CP
#define mCYC  0x900 // Cycles value
#define mEPTR 0xA00


#pragma pack(1)
typedef struct {
    union {
        U16 wregs[16]; // R0  ... R15
        UC8 bregs[16]; // RL0 ... RH7
    } r;
    U32 Ndpp[4]; // full linear base address values
    U32 nPC;     // full address !
    U16 cPsw;    // current Psw
    U16 cSP;
    U16 cMDL;    // MDL
    U16 cMDH;    // MDH
    U32 cCP;     // CP
    I64 nCycles; // cycle counter (or time ???)

    INT64 macc; // 40-Bit value
    U16   mah;  // MAH MAC-Unit Accumulator High
    U16   mal;  // MAL MAC-Unit Accumulator Low
    U16   mas;  // MAS limited MAH/signed
    U16   msw;  // MSW MAC-Unit Status Word
    U16   mcw;  // MCW MAC-Unit Control Word
    U16   mrw;  // MRW MAC-Unit Repeat Word

    U16 idx0;
    U16 idx1;
    U16 qx0;
    U16 qx1;
    U16 qr0;
    U16 qr1;
    U16 cSPSEG; // HS 31.03.2005 XC16x support added
} RG166;


/*
 * Sle66+ specific's
 */

#define nnR0 0x00 // R0
#define nnR1 0x01 // R1
#define nnR2 0x02 // R2
#define nnR3 0x03 // R3
#define nnR4 0x04 // R4
#define nnR5 0x05 // R5
#define nnR6 0x06 // R6
#define nnR7 0x07 // R7
//#define niR0    0x10     // @R0
//#define niR1    0x11     // @R1
//#define nrAB    0x12     // AB
#define nrCY   0x13 // C
#define nrA    0x14 // A
#define nrDPTR 0x18 // DPTR

#define nnPR0  0x1C // PR0 (80C51MX)
#define nnPR1  0x1D // PR1 (80C51MX)

#define nnWR0  0x40 // WR0 (SmartMX2)
#define nnWR2  0x41 // WR2 (SmartMX2)
#define nnWR4  0x42 // WR4 (SmartMX2)
#define nnWR6  0x43 // WR6 (SmartMX2)
#define nnDR0  0x50 // DR0 (SmartMX2)
#define nnDR4  0x51 // DR4 (SmartMX2)



typedef struct { // iMCS51 - SLE66Cxx Registers
    UL32 nPC;
    I64  nCycles; // cycle counter
    UC8  psw;
    UC8  sp;
    UC8  b;
    UC8  acc;
    UC8  dpl;
    UC8  dph;
    UC8  bregs[8];
    UC8  dpsel;
    UC8  adrxh;   // page for MOVX @Ri
    U16  dptx[8]; // DPTR save area
} RGSLE66;

typedef struct {  // iMCS51-Registers
    BYTE  Rn[16]; // R0 ... R7
    DWORD nPC;    // full address !
    BYTE  sp;     // SP
    BYTE  psw;    // PSW-sfr
    BYTE  b;      // B-sfr
    BYTE  acc;    // ACC-sfr
    BYTE  dpl;    // DPL-sfr
    BYTE  dph;    // DPH-sfr
    BYTE  ports[8];
    I64   nCycles; // cycle counter
} RG51;


typedef struct {   // iMCSARM Registers
    DWORD cur[16]; // Current Mode:   R0..R15(PC)
    DWORD cpsr;    // CPSR
    DWORD spsr;    // Current SPSR
    DWORD usr[7];  // User & System:  R8..R14
    DWORD fiq[8];  // Fast Interrupt: R8..R14, SPSR
    DWORD irq[3];  // Interrupt:      R13,R14, SPSR
    DWORD svc[3];  // Supervisor:     R13,R14, SPSR
    DWORD abt[3];  // Abort:          R13,R14, SPSR
    DWORD und[3];  // Undefined:      R13,R14, SPSR
    INT64 nCycles; // cycle counter
} RgARM;


typedef struct {  // iMCSARM Cortex-M Registers
    DWORD Rn[16]; // R0..R12,R13(SP),R14(LR),R15(PC)
    DWORD xPSR;   // xPSR (Program Status Register)
    DWORD MSP;    // MSP (Main SP)
    DWORD PSP;    // PSP (Process SP)
    DWORD DSP;    // DSP (Deep SP)
    union {       // System Registers
        DWORD P;
        struct {
            BYTE PRIMASK;   // PRIMASK
            BYTE BASEPRI;   // BASEPRI
            BYTE FAULTMASK; // FAULTMASK
            BYTE CONTROL;   // CONTROL
        } R;
    } SYS;
    INT64 nCycles; // Cycle Counter
} RgARMCM;


typedef struct {     // iMCSARM v8-M Security Extension Registers
    DWORD MSP_NS;    // MSP_NS (Non-Secure MSP)
    DWORD PSP_NS;    // PSP_NS (Non-Secure PSP)
    DWORD MSP_S;     // MSP_S (Secure MSP)
    DWORD PSP_S;     // PSP_S (Secure PSP)
    DWORD MSPLIM_S;  // MSPLIM_S (Secure MSP Limit)
    DWORD PSPLIM_S;  // PSPLIM_S (Secure PSP Limit)
    DWORD MSPLIM_NS; // MSPLIM_NS (Non-Secure MSP)
    DWORD PSPLIM_NS; // PSPLIM_NS (Non-Secure PSP)
    union {          // System Registers (Secure)
        DWORD P;
        struct {
            BYTE PRIMASK;   // PRIMASK
            BYTE BASEPRI;   // BASEPRI
            BYTE FAULTMASK; // FAULTMASK
            BYTE CONTROL;   // CONTROL
        } R;
    } SYS_S;
    union { // System Registers (Non-Secure)
        DWORD P;
        struct {
            BYTE PRIMASK;   // PRIMASK
            BYTE BASEPRI;   // BASEPRI
            BYTE FAULTMASK; // FAULTMASK
            BYTE CONTROL;   // CONTROL
        } R;
    } SYS_NS;
    union { // Security and Authentication
            // (Update with AG_AllReg(AG_READSEC, ...))
        DWORD V;
        struct {
            DWORD isSecure : 1; // CPU Security State
            DWORD permitS  : 1; // Permit Secure Invasive Debug
            DWORD permitNS : 1; // Permit Non-Secure Invasive Debug
            DWORD          : 29;
        } STATE;
    } SECURITY;
} RgARMV8MSE;

typedef struct {  // iMCSARM Cortex-M4 FPU Registers
    DWORD Sn[32]; // S0..S31
    DWORD FPSCR;  // FPSCR (Floating Point Status Configuration Register)
} RgARMFPU;

typedef struct { // Further extension registers for v8.1-M and later
    union {      // Vector Predication Status and Control Register
        DWORD P;
        struct {
            SHORT PredicationP0; //
            BYTE  MASK01 : 4;    //
            BYTE  MASK23 : 4;    //
            BYTE         : 8;    //
        } R;
    } VPR;
    DWORD RESERVED[63];
} RgARMV8MEXT;



typedef struct { // SmartMX-Registers
    BYTE Rn[32]; // 7F:0000 .. 7F:001F (D:0 .. D:1F)
    BYTE acc, pcl, pch, pcx, psw, pswh;

    // System Mode Registers
    BYTE s_B, s_EPL, s_EPM, s_EPH, s_DPL0, s_DPH0;
    BYTE s_DPL1, s_DPH1, s_MXCON, s_AUXR1, s_XRAMP;
    BYTE s_SP, s_SPE, s_SPLOW, s_SPELOW, s_SPHIGH, s_SPEHIGH;
    //User Context Registers
    BYTE u_B, u_EPL, u_EPM, u_EPH, u_DPL0, u_DPH0;
    BYTE u_DPL1, u_DPH1, u_MXCON, u_AUXR1, u_XRAMP;
    BYTE u_SP, u_SPE, u_SPLOW, u_SPELOW, u_SPHIGH, u_SPEHIGH;
    // Physical PC Value
    BYTE YPCL, YPCH, YPCX;
    BYTE MMUCTRL;
    BYTE MMUPTRL, MMUPTRM, MMUPTRH;
} RGSMX;

typedef struct { // SmartMX2-Registers
    BYTE Rn[32]; // 7F:0000 .. 7F:001F (D:0 .. D:1F)
    BYTE acc, pcl, pch, pcx, psw, pswh, cmod;

    // System Mode Registers
    BYTE s_B, s_EPL, s_EPM, s_EPH, s_DPL0, s_DPH0;
    BYTE s_DPL1, s_DPH1, s_MXCON, s_AUXR1, s_XRAMP;
    BYTE s_SP, s_SPE, s_SPLOW, s_SPELOW, s_SPHIGH, s_SPEHIGH;
    //User Context Registers
    BYTE u_B, u_EPL, u_EPM, u_EPH, u_DPL0, u_DPH0;
    BYTE u_DPL1, u_DPH1, u_MXCON, u_AUXR1, u_XRAMP;
    BYTE u_SP, u_SPE, u_SPLOW, u_SPELOW, u_SPHIGH, u_SPEHIGH;
    // Physical PC Value
    BYTE YPCL, YPCH, YPCX;
    BYTE MMUCTRL;
    BYTE MMUPTRL, MMUPTRM, MMUPTRH;
} RGSMX2;

#pragma pack()



/*
 * Cortex-M Register-Definitons for ARM   /24.5.2010/
 */
#define AR_nR0  0x00 // R0
#define AR_nR1  0x01 // R1
#define AR_nR2  0x02 // R2
#define AR_nR3  0x03 // R3
#define AR_nR4  0x04 // R4
#define AR_nR5  0x05 // R5
#define AR_nR6  0x06 // R6
#define AR_nR7  0x07 // R7
#define AR_nR8  0x08 // R8
#define AR_nR9  0x09 // R9
#define AR_nR10 0x0A // R10
#define AR_nR11 0x0B // R11
#define AR_nR12 0x0C // R12
#define AR_nR13 0x0D // R13 (SP)
#define AR_nR14 0x0E // R14 (LR)
#define AR_nR15 0x0F // R15 (PC)

#define AR_nPSR 0x10 // xPSR
#define AR_nMSP 0x11 // MSP (Main SP)
#define AR_nPSP 0x12 // PSP (Processor SP)
#define AR_nDSP 0x13 // DSP (Deep SP)
#define AR_nSYS 0x14 // System Registers


/*
 * v8-M Security Extensions for Cortex-M Register-Definitons for ARM
 */
#define AR_nMSP_NS    0x15 // MSP_NS (Non-Secure MSP)
#define AR_nPSP_NS    0x16 // PSP_NS (Non-Secure PSP)
#define AR_nMSP_S     0x17 // MSP_S (Secure MSP)
#define AR_nPSP_S     0x18 // PSP_S (Secure PSP)
#define AR_nMSPLIM_S  0x19 // MSPLIM_S (Secure MSP Limit)
#define AR_nPSPLIM_S  0x1A // PSPLIM_S (Secure PSP Limit)
#define AR_nMSPLIM_NS 0x1B // MSPLIM_NS (Non-Secure MSP Limit)
#define AR_nPSPLIM_NS 0x1C // PSPLIM_NS (Non-Secure PSP Limit)
#define AR_nSYS_S     0x1D // System Registers (Secure)
#define AR_nSYS_NS    0x1E // System Registers (Non-Secure)


/*
 * CM4 - floating-point registers
 */
#define AR_nS0  0x90 // S0
#define AR_nS1  0x91 // S1
#define AR_nS2  0x92 // S2
#define AR_nS3  0x93 // S3
#define AR_nS4  0x94 // S4
#define AR_nS5  0x95 // S5
#define AR_nS6  0x96 // S6
#define AR_nS7  0x97 // S7
#define AR_nS8  0x98 // S8
#define AR_nS9  0x99 // S9
#define AR_nS10 0x9A // S10
#define AR_nS11 0x9B // S11
#define AR_nS12 0x9C // S12
#define AR_nS13 0x9D // S13
#define AR_nS14 0x9E // S14
#define AR_nS15 0x9F // S15
#define AR_nS16 0xA0 // S16
#define AR_nS17 0xA1 // S17
#define AR_nS18 0xA2 // S18
#define AR_nS19 0xA3 // S19
#define AR_nS20 0xA4 // S20
#define AR_nS21 0xA5 // S21
#define AR_nS22 0xA6 // S22
#define AR_nS23 0xA7 // S23
#define AR_nS24 0xA8 // S24
#define AR_nS25 0xA9 // S25
#define AR_nS26 0xAA // S26
#define AR_nS27 0xAB // S27
#define AR_nS28 0xAC // S28
#define AR_nS29 0xAD // S29
#define AR_nS30 0xAE // S30
#define AR_nS31 0xAF // S31


#define AR_nD0  0x100 // double register D0
#define AR_nD31 0x11F // double register D31




#pragma pack(1)

#define RGROUP struct rGroup
#define RITEM  struct rItem
#define REGDSC struct RegDsc

struct rGroup { // Register Group Descriptor
    UC8 desc;   // always 0x00
    UC8 ShEx;   // Bit.0 = 1  Show Group initially expanded
                // Bit.1 = 1  Show Group description 'bold'
    char *name; // Group name
};

struct rItem {      // Register Item Descriptor
    UC8  desc;      // always 0x01
    U16  nGi;       // Group-Index (0...nGroups-1)
    U16  nItem;     // Item indicator
    char szReg[16]; // Name of Register
    UC8  isPC;      // is this the PC
    UC8  canChg;    // can this Reg be changed in Reg-Window
    UC8  iHigh;     // highlight the value
                    // Bit 0..6: Show highlight
                    // Bit 7: Highlight type (0 - Color Highlight, 1 - Bold Font Highlight)
    UC8  iDraw;     // item needs to be repainted
    char szVal[32]; // it's value in Ascii
    GVAL v;         // it's binary item value
};

struct RegDsc {
    I32     nGitems;                   // number of group items
    I32     nRitems;                   // number of register items
    RGROUP *GrpArr;                    // array of group descriptors
    RITEM * RegArr;                    // array of register descriptors
    void (*RegGet)(RITEM *vp, int nR); // get RegItem's value
    I32 (*RegSet)
    (RITEM *vp, GVAL *pV); // set RegItem's value
};

#pragma pack()

#define UPR_NORMAL 0x80000000 // update normal registers
#define UPR_HIST   0x40000000 // update history registers




/*
 * Advanced GDI Functions
 */

/*
 * Error codes
 */

#define AG_OK       0 // Ok.
#define AG_NOACCESS 1 // Cannot access while running.
#define AG_RDFAILED 2 // memory read failed.
#define AG_INVALOP  3 // invalid operation code
#define AG_RO       4 // attempt to write Read-only item
#define AG_WRFAILED 5 // memory write failed.
#define AG_CANTMAP  6 // cannot map memory
#define AG_NOTARGET 7 // Command not supported if in "no debug target" mode


/*
 * 80x51 + Sle66(+) memory spaces, added 12/19/99
 */

#define amNONE   0x0000 // not spaced
#define amXDATA  0x0001 // XDATA
#define amBANK0  0x0080 // BANK0
#define amBANK31 0x009F // BANK31
#define amDATA   0x00F0 // DATA
#define amBIT    0x00F1 // BIT
#define amEDATA  0x00F2 // EDATA (i251)
#define amIDATA  0x00F3 // IDATA
#define amECODE  0x00F4 // 251 ecode
#define amHDATA  0x00F5 // 251 hdata
#define amHCONS  0x00F6 // 251 hconst
#define amCONST  0x00F7 // 251 const
#define amPDATA  0x00FE // PDATA (c51 macht das bei generic)
#define amCODE   0x00FF // CODE
#define amPHYS   0x0100 // Physical SLE66+ Memory
#define amPHLIM  0x0110 // 1MB Physical Memory

#define amSSFR   0x00EF // Silabs EFM8 banked sfr

#define amYMEM   0x0040 // Mifare ProX Y: (physical code 1MB)
#define amZMEM   0x0041 // Mifare ProX Z: (physical xdata)
#define amSMEM   0x0042 // Mifare ProX S: (physical sfr)

#define amU1MEM  0x00F8 // UsrMem #1 (E2-PROM)
#define amU2MEM  0x00F9 // UsrMem #2 (reserved for future use)
#define amU3MEM  0x00FA // UsrMem #3 (reserved for future use)
#define amU4MEM  0x00FB // UsrMem #4 (reserved for future use)
#define amU5MEM  0x00FC // UsrMem #5 (reserved for future use)
#define mmU6MEM  0x00FD // UsrMem #6 (reserved for future use)

/*
 * Features
 */

typedef struct {
    U32 MemAccR          : 1; // memory-access while running supported
    U32 RegAccR          : 1; // register-access while running supported
    U32 hTrace           : 1; // trace-recording supported
    U32 hCover           : 1; // code coverage supported
    U32 hPaLyze          : 1; // Performance-Analyzer support
    U32 hMemMap          : 1; // Memory-Map support
    U32 ResetR           : 1; // Reset possible while running
    U32 ExtBrk           : 1; // OCDS/JTAG: Break in EXTx allowed /10.3.2003/
    U32 LaSupp           : 1; // Logic-Analyzer supported

    U32 ExtBrkSupp       : 1;  // extended Breakpoint support (ins/rem/ena/dis while executing etc.)
    U32 ExcTrcSupp       : 1;  // [TdB: 20.11.2012] Support for Exception Trace Window
    U32 DbgRecoverySupp  : 1;  // Target DLL supports debug recovery
    U32 SwBreakConfSupp  : 1;  // Target DLL supports SW Breakpoint Configuration
    U32 PowerMeasurement : 1;  // ULINKplus power measurement and pin state support
    U32 TargetMap        : 1;  // Target DLL supports Target Memory Map
    U32 ItmTrace         : 1;  // ITM + DWT trace support
    U32 DbgUnitTimer     : 1;  // [TdB: 10.10.2018] Debug Unit Timer support
    U32 TpData           : 1;  // Tracepoint support: Trace Data/Access Point (TraceDataPoint/TraceAccessPoint commands)
    U32 TpInstrRunSusp   : 1;  // Tracepoint support: Instruction Trace Run/Suspend (TraceRun/TraceSuspend commands)
    U32 TpInstrHalt      : 1;  // Tracepoint support: Instruction Trace Halt (TraceHalt command)
    U32                  : 12; // Reserved
} SUPP;


#pragma pack(1)
typedef struct {
    UL32 Adr;    // linear address
    UL32 ErrAdr; // memory access failed address
    UL32 nLen;   // address range, used for memory-map
    U16  mSpace; // memory space (not used on 166/167)
} GADR;


typedef struct {   // Assemble/Disassemble
    UL32 Adr;      // linear address
    UC8  Opc[11];  // 167 needs max. 4 Opcode bytes
    I32  OpcLen;   // Result length in assemble mode
    SC8  szB[256]; // DisAsm-result or Asm-input line
    I32  Result;   // 0:=Ok, otherwise error
} DAAS;


typedef struct {   // 11.1.2005, DisAssemble Mixed-Mode into File
    UL32 nStart;   // Start-Address for disassemble
    UL32 nEnd;     // Stop-Address for disassemble
    I32  Result;   // 0:=Ok, 1:=can't create file, 2:=file write error
    I32  nMode;    // 0:=Asm-Mode, 1:=Mixed-Mode
    UL32 nRes[8];  // reserved
    char szN[300]; // name of file to create and write disassembly
} DAS_MIXED;


typedef struct { // Memory-Range descriptor
    UC8  mTyp;   // 0=RAM, 1=ROM
    UL32 nStart; // Memory start address
    UL32 nSize;  // Size of memory block (0=unused)
} OCM;


typedef struct {        // 166/167 Device Info
    UC8  Vendor[64];    // Device-Vendor: example 'Siemens'
    UC8  Device[64];    // Device-Name: example 'C167CR-16FM'
    UL32 Clock;         // clock frequency
    UC8  RestoreBp;     // 1 := restore Breakpoints
    UC8  Rtos;          // OS: 0:=none, 1:=RtxTiny, 2:=RtxFull
    UC8  Mod167;        // 0:=166-Instr. Set 1:=167-Instr Set
    UC8  useOnChipRom;  // 1:=use on chip Rom
    UC8  useOnChipXper; // 1:=use on chip X-peripherals (XRAM+CAN)
    UC8  useMAC;        // 0:=no MACC, 1:=MACC instr. used

    OCM ExtMem[6]; // up to 6 external memory ranges
    OCM Ican;      // address range of on-chip CAN
    OCM Irom;      // address range of on-chip internal Rom
    OCM Xram1;     // address range of on-chip internal Xram
    OCM Xram2;     // address range of on-chip internal Xram
    OCM Iram;      // address range of on-chip internal Ram

    UC8 PrjPath[260]; // path of project
    UC8 AppName[260]; // path and name of loaded application
} DEV_X66;

#pragma pack()




/*
 * Symbol search masks (may be combined using |) :
 */

#define AG_SYM_VAR 0x0001 // search for non-bit Variables
#define AG_SYM_CON 0x0002 // search for named Constants
#define AG_SYM_BIT 0x0004 // search for Bit in Memory
#define AG_SYM_LOC 0x0008 // search for Function/Label
#define AG_SYM_SFR 0x0200 // search for SFR name

/*
 * Type of found symbol:
 */

#define AG_TP_VOID   0
#define AG_TP_BIT    1
#define AG_TP_CHAR   2
#define AG_TP_UCHAR  3
#define AG_TP_INT    4
#define AG_TP_UINT   5
#define AG_TP_SHORT  6
#define AG_TP_USHORT 7
#define AG_TP_LONG   8
#define AG_TP_ULONG  9
#define AG_TP_FLOAT  10
#define AG_TP_DOUBLE 11
#define AG_TP_PTR    12
#define AG_TP_UNION  13
#define AG_TP_STRUCT 14
#define AG_TP_FUNC   15
#define AG_TP_STRING 16
#define AG_TP_ENUM   17
#define AG_TP_FIELD  18

#pragma pack(1)

typedef struct {      // Search for Sym by Name or Value.
    UL32 nMask;       // search mask (AG_SYM_LOC | ...)
    UC8  szName[256]; // search/found name (zero-terminated
    U64  val;         // search/found Adr/Value
    UL32 type;        // type of found symbol (AG_TP_???)
    UL32 Ok;          // 1:=Ok, else find failed.
} SYMDSC;

#pragma pack()


/*
 * Progress-Control Structure
 */

#define PROGRESS_INIT    1
#define PROGRESS_KILL    2
#define PROGRESS_SETPOS  3
#define PROGRESS_INITTXT 4 // 15.3.2003, init progress in text mode
#define PROGRESS_SETTEXT 5 // =========

#pragma pack(1)
typedef struct PgRess { // Progress-Setup and Control structure
    I32  Job;           // PROGRESS_INIT/KILL/SETPOS
    I32  pos;           // PROGRESS_SETPOS: position to set
    I32  low;           // low percent (normally 0)
    I32  hig;           // high percent (normally 100)
    SC8 *label;         // text-label before progress-bar or NULL
    SC8 *ctext;         // 15.3.2003, Text instead of % display
} OIL;
#pragma pack()


/*
 * Flash-Parameter-Block, used in AG_CB_GETFLASHPARAM CallBack
 */
#pragma pack(1)
typedef struct FlashBlock {
    UL32 start;    // Start-Address
    UL32 many;     // Number of Bytes
    UC8 *image;    // Content
    UL32 ActSize;  // total number of bytes
    UL32 Stop : 1; // cancel FlashDownLoad
    UL32 Res[16];  // reserved, unused
} FLASHPARM;
#pragma pack()


// ACCESS REASON FOR MEMORY ACCESS CALLBACK FUNCTIONS
#ifndef __AGDICB_REASON
#define __AGDICB_REASON
typedef enum {             // !! numbers conform with ATR_READ & ATR_WRITE
    AG_CB_VOID     = 0,    // no access            (currently not in use)
    AG_CB_READ     = 0x02, // adr = memory address (currently not in use)
    AG_CB_WRITE    = 0x04, // adr = memory address (currently not in use)
    AG_CB_VT_READ  = 0x12, // adr = VTR-Handle
    AG_CB_VT_WRITE = 0x14, // adr = VTR-Handle
    AG_CB_IFETCH   = 0x08, // adr = memory address (currently not in use)
} AGDICB_REASON;
#endif

#pragma pack(1)
/*
 * VTR definition-Parameter-Block, used in AG_CB_CREATEVTR CallBack
 */
typedef struct {
    char *Name;                                            // Name of virtual register (VTR)
    U32   Type;                                            // Type of VTR (e.g. AG_TP_ULONG )
    void (*pFunc)(DWORD adr, AGDICB_REASON r, DWORD size); // Callback function on read/write access. NULL if not needed
    U32 AccType;                                           // Callback trigger (read/write)
} VTRDESCR;

/*
 * VTR read/write access Parameter-Block, used in AG_CB_READVTR and AG_CB_WRITEVTR CallBack
 */
typedef struct {
    U32 VTRHandle; // Name of virtual register (VTR)
    U32 Value;     // Value of VTR
} VTRACC;
#pragma pack()

/*
 * Running-Parameter-Block, used in AG_CB_RUNNING CallBack
 */

#define RUNNING_AFTER_CONNECT 0x0000 // Target running after debug connection
#define RUNNING_DLL_READY     0x0001 // AGDI DLL is set up to receive commands from upper SW layers
#define RUNNING_DETECTED      0x0002 // Externally started target detected
#define RUNNING_AFTER_STEP    0x0003 // Target running after single instruction step

#pragma pack(1)
typedef struct RunningBlock {
    UL32 info;    // Information code for this callback item
    UL32 Res[16]; // reserved, unused
} RUNNINGPARM;
#pragma pack()



/*
 * Init/Query Target features
 */

#define AG_INITFEATURES 0x0100 // Init target features
#define AG_GETFEATURE   0x0200 // extract a single feature
#define AG_INITITEM     0x0300 // Init item
#define AG_EXECITEM     0x0400 // Execute item
#define AG_CUSTOMCMD    0x0500 // Execute custom command:                                     \
                               // - Use lower 8 bit for the custom command subcode.           \
                               // - AG_Init() must return '0' if command is not supported.    \
                               // - Usage of AG_CUSTOMCMD is AGDI DLL implementation defined. \
                               //   ARM will NOT support third party AG_CUSTOMCMD commands in \
                               //   its DLL implementations.

/*
 * Supported Target Features:
 */
#define AG_F_MEMACCR  0x0001    // memory-access while running
#define AG_F_REGACCR  0x0002    // register-access while running
#define AG_F_TRACE    0x0003    // back-trace support
#define AG_F_COVERAGE 0x0004    // code-coverage support
#define AG_F_PALYZE   0x0005    // Performance-Analyzer support
#define AG_F_MEMMAP   0x0006    // Memory-Map support
#define AG_F_RESETR   0x0007    // Reset while running possible feature
#define AG_F_251MODE  0x0008    // 251: get mode (SRC_MODE/BIN_MODE)
#define AG_F_251FRMSZ 0x0009    // 251: get framesize (FRM_SIZE2/4)
#define AG_F_251CPU   0x000A    // 251: get CPU type (CPU_251/CPU_51)
#define AG_F_16XBIE   0x000B    // 167: brk within EXTx-Sequ allowed \
                                //      ret: 1:=yes, 0:=no
#define AG_F_LANALYZER   0x000C // Logic-Analyzer support
#define AG_F_EXTBRKSUPP  0x000E // Is extended Breakpoint support available
#define AG_F_EXCTRC      0x000F // [TdB: 20.11.2012] Support for Exception Trace Window
#define AG_F_DBGRECOVERY 0x0010 // Target DLL supports debug recovery
#define AG_F_SWBREAKCONF 0x0011 // Target DLL supports SW Breakpoint Config via \
                                // AG_EXECITEM|AG_SWBREAKCONF

#define AG_F_TARGETMAP        0x0013 // Target Memory-Map support
#define AG_F_POWERMEASUREMENT 0x0014 // Power Measurement and pin state support
#define AG_F_ITM_TRACE        0x0015 // ITM&DWT support
#define AG_F_DBG_UNIT_TIMER   0x0016 // Clock stream and sync point data
#define AG_F_TP_DATA          0x0017 // Tracepoint support: Trace Data/Access Point (TraceDataPoint/TraceAccessPoint commands)
#define AG_F_TP_INSTR_RUNSUSP 0x0018 // Tracepoint support: Instruction Trace Run/Suspend (TraceRun/TraceSuspend commands)
#define AG_F_TP_INSTR_HALT    0x0019 // Tracepoint support: Instruction Trace Halt (TraceHalt command)


/*
 * Connection and Communication Items used for 'AG_INITITEM':
 */
#define AG_INITMENU       0x0007 // init extension menu
#define AG_INITEXTDLGUPD  0x0008 // init modeless extesion dlg update function
#define AG_INITMHANDLEP   0x0009 // setup ptr to HWND of active modeless dlg
#define AG_INITPHANDLEP   0x000A // pointer to parent handle (MainFrame)
#define AG_INITINSTHANDLE 0x000B // pointer to Agdi-instance handle
#define AG_INITBPHEAD     0x000E // pointer to Bp-head
#define AG_INITCURPC      0x000F // pointer to Program counter
#define AG_INITDOEVENTS   0x0010 // pointer do DoEvents()
#define AG_INITUSRMSG     0x0011 // Registered Message for SendMessage
#define AG_INITCALLBACK   0x0012 // pointer to callback function

//---19.7.2002: added for Flash DownLoad:
#define AG_INITFLASHLOAD  0x0013 // Prepare for Flash Download
#define AG_STARTFLASHLOAD 0x0014 // Start Flash DownLoad

//---2.11.2004: added for Hitex-AGDI:
#define AG_INITCWINAPP   0x0020 // sent Item is 'CWinApp *'
#define AG_INITSTARTLOAD 0x0021 // 'Load about to start' (item := 'LOADPARMS *')
#define AG_INITENDLOAD   0x0022 // 'Load finished' (item := NULL)



/*
 * AG_EXECITEM items:
 */
#define AG_UNINIT  0x000C // Clean up target
#define AG_RESET   0x000D // Reset target system
#define AG_GETMODE 0x000E // for S8051.DLL: Get Dallas Contigious Mode
//---26.10.2000: added for Hitex Emulators:
#define AG_RUNSTART 0x0010 // Announcement: Go/Step about to start
#define AG_RUNSTOP  0x0011 //             : Go/Step completed.

//---30.6.2005: added for Hitex Emulators:
#define AG_STEPINTO_START 0x0014 //             : StepInto starts (after end: AG_RUNSTOP)
#define AG_STEPLINE_START 0x0015 //             : StepLine starts (after end: AG_RUNSTOP)
#define AG_STEPOUT_START  0x0019 //             : StepOut starts
//---

#define AG_QUERY_LASIG  0x0012 // 27.1.2005
#define AG_KILLED_LASIG 0x0013 // 27.1.2005
//---

#define AG_PROFILEINFO 0x0016 // UlPro-Profiling, derrived from Trace  /1.7.2009/

#define AG_ULPTRACE    0x0017 // ULINKpro Specific Trace Requests /24.01.2012/
#define AG_EXCTRC      0x0018 // [TdB: 20.11.2012] Support for Exception Trace Window



#define AG_EEPROM_CLR  0x0020 // 25.11.2005, Sle66/Sle66PEMon: clear eeprom
#define AG_EEPROM_ERA  0x0021 // 25.11.2005, Sle66/Sle66PEMon: erase eeprom
#define AG_EEPROM_FMT  0x0022 // 25.11.2005, Sle66/Sle66PEMon: format eeprom
#define AG_INITMF      0x0023 // 14.9.2006,  Sle66/Sle66PEMon: init mifare sector

//---1.10.2006: RESGO [ON/OFF] - for Sle66PEMON
#define AG_RESGO_QUERY 0x0024 // Read RESGO
#define AG_RESGO_ON    0x0025 // RESGO ON
#define AG_RESGO_OFF   0x0026 // RESGO OFF
//---End 1.10.2006

#define AG_CLEAR_CCACHE  0x0030 // clear code cache: issued when 'Update Windows' in ToolBox is pressed
#define AG_TARGETREADY   0x0040 // HS 23.06.2008 added for AGSI notification when target system is connected
#define AG_STOPREQ       0x0041 // HS 09.10.2014 added for AGSI notification when stop button is pressed

#define AG_LOADSTART     0x0050 // CL 06.04.2011 added for AGSI notification when LOAD file is started/finished
#define AG_LOADEND       0x0051

#define AG_RECOVERY_CONF 0x0060    // JR 07.08.2013: Configure Debug Access Recovery
#define AG_SWBREAKCONF   0x0061    // JR 20.01.2015: Configure SW Breakpoint Support for Memory Ranges
#define AG_START_DBGEXIT 0x0062    // JR 05.11.2015: Announce debug exit to give DLLs a chance to react \
                                   //                accordingly to certain interface calls
#define AG_OVERSTEP_START   0x0063 // BY 12.02.2015: Assembly mode overstepping

#define AG_TARGETMAP        0x0066 // 04.10.2016: Target Memory Map support
#define AG_REGTREE_UPDT_GRP 0x0067 // Update properties of struct RGROUP
#define AG_TRACESETUP_REQ   0x0068 // Trace setup request


/*
 * AG_PROFILEINFO - commands:   /1.7.2009/
 *  used in 'case AG_EXECITEM' (execute various commands)
 *    subcase: case AG_PROFILEINFO:    // UlPro-Profiling, derrived from Trace
 */
#define AG_PROF_CALCCYCLES  1 // calc cycles & time over address range
#define AG_PROF_TOTALCYCLES 2 // get total runtime
#define AG_PROF_EXECCOUNT   3 // get number of times executed for address 'nStart'
#define AG_PROF_ADDINFO     4 // add profiling counter info for address 'nStart' (cycles, count)

#pragma pack(1) // 12.09.2016: Added missing #pragma pack directive, nRes extended from 8 to 9 elements to \
                // reflect previous implicit insertion of a DWORD by the compiler for aligned UINT64 cycles field.
typedef struct agproftm {
    int nCmd; // Command: AG_PROF_EXECCOUNT etc.

    DWORD  nStart;  // Start-Address
    DWORD  nEnd;    // End-Address   (Adr >= nStart && Adr < nEnd)
    DWORD  nRes[9]; // reserved...
    UINT64 cycles;  // Input (AG_PROF_ADDINFO) / Result (other commands): cycles
    double time;    //         time in Sec
    UINT64 count;   //         number of executions
} AG_PROFINFO;
#pragma pack()

#define AG_PROFINFO_NP  0 // default return-code when 'not implemented'
#define AG_PROFINFO_BAD 1 // return-code when failed
#define AG_PROFINFO_OK  2 // return-code when Ok, 0 or 1 otherwise when not present/or failed


#if 0 // Howto attach the profile functionality to AGDI:

static DWORD TargetProf (PROFINFO *pI)  {
  LIN       *pLi;
  DWORD       nR = AG_PROFINFO_OK;

  switch (pI->nCmd)  {
    case AG_PROF_CALCCYCLES:    // calc cycles & time over address range
      ExeProfCycles (pI);       // TODO: same as in SarmCM3
      pI->time = PACalcTime (pI->cycles);
      break;

    case AG_PROF_TOTALCYCLES:   // get total runtime
      pI->cycles = pio->cycles;
      pI->time   = CalcTime (pI->cycles);        // TODO: same as in SarmCM3
      break;

    case AG_PROF_EXECCOUNT:     // get 'number of times executed' for address 'nStart'
      if (pI->nStart == 0 && pI->nEnd == 0)  {    // 5.1.2004
        if (pI->nRes[7] != 0)  {
          pLi = FindLine (0);                     // search for Line number
          if (pLi && pLi->line == pI->nRes[7])  {
            pI->nEnd = 3;                         // ARM-Mode anyway 
          }
        }
      }                                           // ========
      ExeProfCount (pI);        // TODO: same as in SarmCM3
      break;

    default:
      nR = 0;                   // incorrect call
      break;
  }
  return (nR);
}


U32 _EXPO_ AG_Init (U16 nCode, void *vp)  {

  nE = 0;                         // default to 'AG_PROFINFO_NP' no-present code.
  switch (nCode & 0xFF00)  {
    ...

    case AG_EXECITEM:             // execute various commands
      switch (nCode & 0x00FF)  {
        ...
        case AG_PROFILEINFO:      // UlPro-Profiling, derrived from Trace
          nE = TargetProf ((AG_PROFINFO) vp);
//---Note: return AG_PROFINFO_OK when Ok, otherwise AG_PROFINFO_BAD.
          break;

      }
      break;
  }

}

#endif



/*
 * AG_ULPTRACE - commands:   /24.01.2012/
 *  used in 'case AG_EXECITEM' (execute various commands)
 *    subcase: case AG_ULPTRACE:    // ULINKpro Specific Trace Requests
 */
#define AG_ULPTRACE_RAWTPIUPATH 0x00000001 /* Set/get path to store the raw TPIU data to
                                                nFlags[0] == 0 : get path.
                                                nFlags[0] == 1 : set path. */

#define AG_ULPTRACE_INFO        0x80000001 // Get information about available trace data
#define AG_ULPTRACE_LOADRANGE   0x80000002 // Load data from unlimited trace by time
#define AG_ULPTRACE_LOADBLOCK   0x80000003 // Load data from unlimited trace by block

// Error codes
#define AG_ULPTRACE_OK         0 // Call succeeded
#define AG_ULPTRACE_ERR_GEN    1 // General error
#define AG_ULPTRACE_ERR_SUP    2 // Call not supported
#define AG_ULPTRACE_ERR_DIS    3 // Unlimited trace disabled
#define AG_ULPTRACE_ERR_PATH   4 // Received path is not what was expected
#define AG_ULPTRACE_ERR_ETBSUP 5 // Call not supported using ETB

#pragma pack(1)
typedef struct ulptraceinfo {
    UINT32 nCmd;   // Command ID
    UINT8  nFlags; // Flags for various commands, depends on use mode

    UINT64 nFirstCycle; // First Cycle of Trace Data, in/out for AG_ULPTRACE_LOAD, in for AG_ULPTRACE_INFO and AG_ULPTRACE_LOADBLOCK
    UINT64 nLastCycle;  // Last Cycle of Trace Data, in/out for AG_ULPTRACE_LOAD, in for AG_ULPTRACE_INFO and AG_ULPTRACE_LOADBLOCK
    UINT64 nRecCount;   // Number of trace records, in/out for AG_ULPTRACE_LOAD, in for AG_ULPTRACE_INFO and AG_ULPTRACE_LOADBLOCK

    char * pPath;          // Path to unlimited trace storage  (AG_ULPTRACE_LOADBLOCK, AG_ULPTRACE_RAWTPIUPATH)
    UINT32 nFileIdx;       // File index into unlimited trace storage  (AG_ULPTRACE_LOADBLOCK)
    UINT32 nBlockIdx;      // Block index starting from nFileIdx (may as well be in a different file)  (AG_ULPTRACE_LOADBLOCK)
    UINT32 nBlocks;        // Number of blocks to load  (AG_ULPTRACE_LOADBLOCK)
    UINT8  nReserved8[3];  // Reserved
    UINT32 nReserved32[8]; // Reserved
} AG_ULPTRACEINFO;
#pragma pack()



typedef struct agdi_la { // Query LA-Signal  /27.1.2005/
    struct agdi_la *next;
    U64             nAdr;                   // LA-Signal: memory address
    U64             nSize;                  // LA-Signal: size of analyzed item in Bytes
    GVAL            v;                      // value
    U32             SigNo;                  // internal LA signal number (don't change)
    U32             killed;                 //
    U32             targVtrNo          : 8; // [TdB: 20.02.2016] (SDMDK-6644) Display Power Measurement results in Logic Analyzer like window
    U32             sigTyp             : 4; // Signal Type for power measurements etc (normal, V, A, W, ...)
    U32             significantNumbers : 4; // significant numbers (i.e.: 4 : 1.234)
    U32             nRes2              : 16;
    U32             nRes[15]; // reserved

    char szDname[128]; // Signal's display name
    char szExp[128];   // the expr.-text: LA CREATE <expr>
    char szError[256]; // Error-Text
} AGDI_LA;


typedef struct {        // Write-Data-Record
    I64  tStamp;        // Time-Stamp of write
    I64  totTime;       // total running time
    U64  nPC;           // PC-value at time of write
    U64  nAdr;          // Write-Address
    GVAL v;             // Write-value @nAdr
    U32  wType;         // 1:=Byte, 2:=W16, 4=U32, 8=U64
    U32  SigNo;         // LASIG-Number (1...255)
    U32  targVtrNo : 8; // [TdB: 20.02.2016] (SDMDK-6644) Display Power Measurement results in Logic Analyzer like window
    U32  nRes2     : 24;
    U32  nRes[15]; // reserved
} AGDI_LAREC;
//---TODO: define - tStamp/totTime uSec or cycles ???


/*
 * Errors returned from AG_QUERY_LASIG:
 */
#define AGDI_LA_OK           1     // LA-Signal was accepted by target
#define AGDI_LA_NOTSUPPORTED (-10) // LA-Sig Address-Range not accepted
#define AGDI_LA_NORESOURCE   (-11) // No resources available in target for LA_Signal




/*
 * 26.1.2005, Info given on AG_INITSTARTLOAD: (see example dated /26.1.2005/ in this file)
 */
typedef struct {
    char szFile[512]; // full path name of App to load
    BYTE Incremental; // 1:=incremental load
    BYTE NoCode;      // 1:=no code
    BYTE Reserved[32];
} LOADPARMS;


#pragma pack(1)

typedef struct ag_line {
    U16  line;
    UL32 addr;
} AG_LINE;

typedef struct ag_line32 {
    UL32 line  : 28; // 28-bit linennumber
    UL32 bStmt : 1;  // line number is a statement
    UL32       : 3;
    UL32 addr; // linenumber-offset
} AG_LINE32;


typedef struct ag_scope {
    DWORD AddrIn;        // Input: Address to be mapped to Scope
                         // Output values:
    const char *pModule; // name of Module
    const char *pFunc;   // name of Function

    DWORD lowPC;   // Scope-Block lower bound
    DWORD higPC;   // Scope-Block upper bound (up to and including)
    DWORD nLines;  // number of lines
    DWORD curLine; // current line (or 0 if none)
    union {
        AG_LINE *  pLines; // Line-Number Array (nRes[29].0:=0)
        AG_LINE32 *pLin32; // used if nRes[29].0 is set
    };
    char szPath[512]; // full path name of source file

    DWORD nRes[32]; // reserved
                    // When used with AG_CB_GETSCOPEINFO for Cortex-M,
                    // setting nRes[31] to 0xFFFFFFFF requests information
                    // if an address is the first address associated
                    // with a High-Level Language source line. In this case
                    // no members of the struct are manipulated but the AG
                    // callback returns 1 if this is true, 0 otherwise.
} AG_SCOPE;


typedef struct ag_block { // App/Module/Func Enumeration
    char const *pApp;     // App-Name (wg. incremental load)
    char const *pModule;  // Module-Name
    char const *pFunc;    // Function-Name
    UL32        lowPC;    // Function starts at 'lowPC'
    UL32        higPC;    //          and extends up to and including 'higPC'
                          // ***DO NOT MODIFY these:
    void *pA;             // internal reserved data  /App/
    void *pT;             // internal reserved data  /Theadr/
    void *pF;             // internal reserved data  /Func/
    UL32  m1 : 1;
    UL32     : 31; // reserved for future use
} AG_BLOCK;

#pragma pack()


/*
 * Structure for debug access recovery configuration:   /07.08.2013/
 *  used in 'case AG_EXECITEM' (execute various commands)
 *    subcase: case AG_RECOVERY_CONF:    // Configure Debug Access Recovery
 */
#pragma pack(1)
typedef struct ag_recovery {
    DWORD underReset : 1;  // Try to recover under HW reset
    DWORD exitDebug  : 1;  // Debug exit requested from Recovery Dialog
    DWORD            : 30; // Reserved
    DWORD nRes[16];        // Reserved
} AG_RECOVERY;
#pragma pack()

// AG_RECOVERY_CONF Error Codes
#define AG_RECOVCONF_ERR_NOSUPP 0 // AG_RECOVERY_CONF unsupported
#define AG_RECOVCONF_ERR_OK     1 // Success

/*
 * Structure for SW Breakpoint configuration:   /20.01.2015/
 *  used in 'case AG_EXECITEM' (execute various commands)
 *    subcase: case AG_SWBREAKCONF:    // Configure SW Breakpoint Support for Memory Ranges
 *      in: (AG_SWBREAKCONF_REQ*)
 */

/*
 * SW Breakpoint
 */
#pragma pack(1)
typedef struct ag_swbreakconfitem {
    struct ag_swbreakconfitem *next; // Next SW Break Conf item in list
    U16                        nID;  // ID in SW Break Conf List; dynamic and can be renumerated on list changes.

    // SW Break Conf Memory Range (Active SW Break Confs MUST NOT overlap!)
    UL32 nStartAddr; // Start Address
    UL32 nEndAddr;   // End Address

    // SW Break Conf Settings
    U16 bEnableSWBreaks : 1;  // SW Breaks enabled
    U16 bReserved       : 15; // reserved for future use
    U16 nAccSz;               // Memory Access Size in Bytes to use by Debugger (choses best suitable size if '0')
    U16 nAccAlign;            // Memory Alignment to consider for SW Breakpoint insertion. Concerns both start address
                              // and number of bytes to write (e.g. to trigger an ECC recalculation by writing a
                              // chunk of a specific size). It can result in a read-modify-write operation on the
                              // target memory. Max supported value 1024.
                              // No memory alignment taken into account if '0'.

    DWORD nReserved[16]; // reserved for future use
} AG_SWBREAKCONF_ITEM;
#pragma pack()


// AG_SWBREAKCONF commands
#define AG_SWBREAKCONF_LIST 0 // Get head of SW Breakpoint Config list \
                              // in  : None                            \
                              // out : Pointer to head of SW Break Conf list (AG_SWBREAKCONF_REQ::pConf)
#define AG_SWBREAKCONF_ADD 1  // Add SW Breakpoint Config                                                \
                              // in  : Pointer to settings to add            (AG_SWBREAKCONF_REQ::pConf) \
                              // out : Pointer to input with assigned nID    (AG_SWBREAKCONF_REQ::pConf)
#define AG_SWBREAKCONF_REM 2  // Remove SW Breakpoint Config by ID                                  \
                              // in  : NULL - Remove all, else pointer to struct with nID to remove \
                              // out : None


#pragma pack(1)
typedef struct ag_swbreakconfreq {
    DWORD                nCmd;          // AG_SWBREAKCONF sub-command
    AG_SWBREAKCONF_ITEM *pConf;         // Pointer to SW Breakpoint Config (in/out: see AG_SWBREAKCONF commands)
    DWORD                nReserved[16]; // reserved for future use
} AG_SWBREAKCONF_REQ;
#pragma pack()


// AG_SWBREAKCONF Error Codes
#define AG_SWBC_ERR_NOSUPP      0  // Unsupported
#define AG_SWBC_ERR_OK          1  // Success
#define AG_SWBC_ERR_INTERNAL    2  // Internal Error
#define AG_SWBC_ERR_GEN         3  // General Error
#define AG_SWBC_ERR_CMD         4  // Unknown Command
#define AG_SWBC_ERR_SET         5  // Cannot Set SW Break Configuration
#define AG_SWBC_ERR_SET_DIRTY   6  // Cannot Disable SW Breaks: Breakpoints set in range (only if disable request)
#define AG_SWBC_ERR_REM         7  // Cannot Remove SW Break Configuration
#define AG_SWBC_ERR_REM_DIRTY   8  // Cannot Enable SW Breaks: Breakpoints set in range (only if remove config disabled SW breaks)
#define AG_SWBC_ERR_REM_ID      9  // Cannot Remove SW Break Configuration: Unknown ID
#define AG_SWBC_ERR_ADDR        10 // Cannot Set SW Break Configuration for address range
#define AG_SWBC_ERR_ADDR_OVL    11 // Cannot Set SW Break Configuration: Address range overlaps existing configuration
#define AG_SWBC_ERR_ADDR_MINMAX 12 // Cannot Set SW Break Configuration: Start address larger than Address range overlaps existing configuration
#define AG_SWBC_ERR_ACCSZ       13 // Unsupported Access Size
#define AG_SWBC_ERR_ALIGN       14 // Unsupported Alignment
#define AG_SWBC_ERR_ALIGNP2     15 // Unsupported Alignment: Must be a power of 2
#define AG_SWBC_ERR_ALIGNMAX    16 // Unsupported Alignment: Exceeds maximum allowed value
#define AG_SWBC_ERR_ALIGNSTART  17 // Unaligned start address
#define AG_SWBC_ERR_ALIGNBLOCK  18 // Memory range must be multiple of alignment size
#define AG_SWBC_ERR_ALIGNACCSZ  19 // Unsupported Alignment: Must be multiple of access size

/*
* AG_TARGETMAP - commands:   /04.10.2016/
*  used in 'case AG_EXECITEM' (execute various commands)
*    subcase: case AG_TARGETMAP:    // Target Memory Map support
*      in: (AG_TARGETMAP_ITEM*)
*/

// AG_TMAP_ITEM attributes
#define AG_TMAP_ATTR_SECURE 0x00000008 // Secure Memory

/*
* AG_TARGETMAP_ITEM - Target Memory Map item structure 
*/
#pragma pack(1)
typedef struct ag_tmapitem {
    struct ag_tmapitem *next;         // Pointer to next item
    U32                 id;           // Item ID; dynamic and can be renumerated on list changes.
    U32                 startAddr;    // Start address for Target Memory Map item
    U32                 endAddr;      // End address for Target Memory Map item
    U32                 attrib;       // Attributes for Target Memory Map item
    U32                 reserved[16]; // Reserved for future use
} AG_TARGETMAP_ITEM;
#pragma pack()


// AG_TARGETMAP sub-commands
#define AG_TARGETMAP_LIST 0 // Get head of Target Memory Map Item list \
                            // in  : None                              \
                            // out : Pointer to head of Target Memory Map Item list (AG_TARGETMAP_CMD::items)
#define AG_TARGETMAP_ADD 1  // in  : Pointer to new Target Memory Map item          (AG_TARGETMAP_CMD::items) \
                            // out : Input item with id set to assigned ID   (AG_TARGETMAP_CMD::items)
#define AG_TARGETMAP_REM 2  // in  : NULL - Remove all, else remove Target Memory Map items as per specified \
                            //       AG_TARGETMAP_ITEM::id fields in AG_TARGETMAP_CMD::items list

/*
* AG_TARGETMAP_CMD - Target Memory Map command structure 
*/
#pragma pack(1)
typedef struct ag_targetmapcmd {
    U32                cmd;          // AG_TARGETMAP_CMD sub-command
    AG_TARGETMAP_ITEM *items;        // Head of Target Memory Map item list
    U32                reserved[16]; // Reserved for future use
} AG_TARGETMAP_CMD;
#pragma pack()

// AG_TARGETMAP Error Codes
#define AG_TMAP_ERR_NOSUPP   0 // Unsupported
#define AG_TMAP_ERR_OK       1 // Success
#define AG_TMAP_ERR_CMD      2 // Unknown Command
#define AG_TMAP_ERR_BADPARM  3 // Bad Command Parameter
#define AG_TMAP_ERR_INTERNAL 4 // Internal Error
#define AG_TMAP_ERR_GEN      5 // General Error

/*
 * 15.01.2008, Structure to pass to AG_CB_MSGBOX callback: (see example dated /15.01.2008/ in this file)
 */

#pragma pack(1)

typedef struct ag_msgbox {
    HWND        hWnd;        // Pass a window handle to show the MB on, if NULL main UV3 window is used
    const char *pMsg;        // Message text pointer
    const char *pCaption;    // Message box caption string pointer
    UINT        uiType;      // Message box type (Buttons, graphic)
    UINT        uiDefAnswer; // Default answer to a message box if it cannot be displayed
} AG_MSGBOX;

#pragma pack()

UINT AGDIMsgBox(HWND        hWnd,
                const char *pMsg,
                const char *pCaption,
                UINT        uiType,
                UINT        uiDefAnswer);


/*
 * Call-Back-Function in S166 and supported sub-functions
 */

typedef U32 (*pCBF)(U32 nCode, void *vp);
#define AG_CB_TRUEXPR       1  // vp := 'EXP *' (use for Bp->ep)
#define AG_CB_PROGRESS      2  // vp := 'struct PgRess *'
#define AG_CB_INITREGV      3  // vp := 'REGDSC *' (into RegView)
#define AG_CB_EXECCMD       4  // vp := 'char *' command string
#define AG_CB_FORCEUPDATE   5  // vp := NULL, force general windows update
#define AG_CB_DISASM        6  // vp := 'DAAS *', disasm opcodes
#define AG_CB_INLASM        7  // vp := 'DAAS *', assemble szB[] into Opc[]
#define AG_CB_MSGSTRING     8  // vp := 'char *' text for message pane
#define AG_CB_GETDEVINFO    9  // vp := 'DEV_X66 *', get device info
#define AG_CB_SYMBYVAL      10 // vp := 'SYMDESC *', find symbol by value
#define AG_CB_SYMBYNAME     11 // vp := 'SYMDESC *', find symbol by name
#define AG_CB_SLE66MM       12 // vp := &slots[0] out of [512]
#define AG_CB_PHYS2MMU      13 // vp := (void *) ((DWORD) physAdr)
#define AG_CB_MMU2PHYS      14 // vp := (void *) ((DWORD) logicalAdr)
#define AG_CB_GETFLASHPARAM 15 // vp := (FLASHPARM *) or NULL
#define AG_CB_GETBOMPTR     16 // vp := &ioc  /5.3.2003/

// Note: special callback for DTC-Messaging
#define AG_CB_DCTMSG_WRITE  17 // vp := write-access address

#define AG_CB_DISASM_EXT    18 // vp := 'DAS_MIXED *', disasm to file
#define AG_CB_LAREC_DATA    19 // vp := 'AGDI_LAREC *', send data-record to Logic-Analyzer
#define AG_CB_SHUTDOWN      20 //
#define AG_CB_GETSCOPEINFO  21 // vp := 'AG_SCOPE *', get Scope Info
#define AG_CB_ENUMFUNCTIONS 22 // vp := 'AG_BLOCK *'. enumerate App/Modules/Functions
#define AG_CB_MSGBOX        23 // vp := '', show a message box
#define AG_CB_ITMDATA       24 // vp := 'struct SerAS *'
#define AG_CB_GETPUBHEAD    25 // vp := 'struct sym **', get head of public symbols
#define AG_CB_CREATEVTR     26 // vp := 'VTRDESCR', create virtual register (VTR) and its callback function
#define AG_CB_READVTR       27 // vp := 'VTRACC', read value of virtual register (VTR)
#define AG_CB_WRITEVTR      28 // vp := 'VTRACC', write value into virtual register (VTR)

#define AG_CB_EXECHOOK      32 // vp := 'char *' hook function string, name without parenthesis (no arguments)
#define AG_CB_RUNNING       33 // Notification: AGDI DLL detected a running target

#define AG_CB_FETCHMODINFO  34 // vp := &uint_32_t, bit-0: 1:=CASE, 0:=NOCASE              \
                               //                   bit-1: 1:=FLOAT_USED, 0:=NOFLOATUSED   \
                               //                   bit-2: 1:=DOUBLE_USED, 0:=NODOUBLEUSED \
                               //
#define AG_CB_ITMTRCDATA 35    // vp: struct AG_ITMTRCDATA \
                               // All ITM data (including channel 0 and 31, split in SARMxxx.DLL)

#define AG_CB_EXTRCDATA   36 // vp: struct AG_EXCTRCDATA
#define AG_CB_CFG_TRC     37 // Debug Trace Configuration Update
#define AG_CB_CHECKDEVICE 41 // Check device debug is allowed (return value: 0 - OK, other - not allowed)



#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_Init(U16 nCode, void *vp);
#else // S166
extern U32 (*AG_Init)(U16 nCode, void *vp);
#endif



/*
 * Memory attributes
 */

#define AG_ATR_EXEC      0x01   // 'executable ' Attribute
#define AG_ATR_READ      0x02   // 'readable' Attribute
#define AG_ATR_WRITE     0x04   // 'writable' Attribute
#define AG_ATR_BREAK     0x08   // 'Exec-Break' Attribute
#define AG_ATR_EXECD     0x10   // 'Executed' Attribute
#define AG_ATR_WATCH     0x20   // Location has a Watch
#define AG_ATR_BPDIS     0x40   // 'disabled Exec-Break' Attribute
#define AG_ATR_PAP       0x80   // Location has a Perf.-Analyzer point
#define AG_ATR_WRBRK     0x100  // Loc has a write-access break
#define AG_ATR_RDBRK     0x200  // Loc has a read-access break
#define AG_ATR_COMC      0x400  // iMCS51/251: Common code marker
#define AG_ATR_VNM       0x800  // iMCS51: von Neumann mapped
#define AG_ATR_BEP       0x1000 // iMCS51: Bank-Entry Point
#define AG_ATR_EXTR      0x2000 // 166/167: within range of an EXTR sequence
#define AG_ATR_JTAKEN    0x4000 // Jump-taken attribute



#define AG_MEMMAP        0x01 // map memory
#define AG_GETMEMATT     0x02 // get memory attribute
#define AG_SETMEMATT     0x03 // set memory attribute
#define AG_GETATTRSEG    0x04 // get attribute segment (e.g. for coverage  /23.7.2010/)
#define AG_CLEARCOVERAGE 0x05 // clear complete coverage  /23.7.2010/




#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_MemAtt(U16 nCode, UL32 nAttr, GADR *pA);
#else
extern U32 (*AG_MemAtt)(U16 nCode, UL32 nAttr, GADR *pA);
#endif



/*
 * Breakpoint query and access
 */

#define AG_ABREAK 0 // simple code address break
#define AG_CBREAK 1 // conditional break (check after each instr.)
#define AG_WBREAK 2 // data access break
#define AG_RBREAK 3 // Address-Range Break (currently not used)
#define AG_TBREAK 4 // Tracepoint


#pragma pack(1)
struct AG_Bps {
    struct AG_Bps *next;
    struct AG_Bps *prev;

    UL32 type     : 4;  // AG_ABREAK, AG_CBREAK, AG_WBREAK, AG_TBREAK
    UL32 enabled  : 1;  // 1:=enabled, 0:=disabled
    UL32 ReCalc   : 1;  // recalc expr flag
    UL32 BytObj   : 1;  // WatchBrk: 0:=Bytes, 1:=Objects
    UL32 ExtInfo  : 1;  // 1:=extended info is present
    UL32 bUnused  : 1;  // for compatibility with 'bom - struct brk'
    UL32 HtxType  : 8;  // User-specific data  /26.1.2012/
    UL32 fromIni  : 1;  // *** reserved
    UL32 bAlloced : 1;  // *** reserved
    UL32 bKilled  : 1;  // *** reserved
    UL32          : 12; // *** reserved for future use

    UL32  Adr;    // Address (or Range)
    UL32  mSpace; // memory-space
    void *pV;     // used on VTR-access breakpoints

    UL32 tsize;  // WatchBrk: size of one object
    UL32 many;   // WatchBrk: many objects or bytes
    U16  acc;    // WatchBrk: 1:=Read, 2:=Write, 3:=ReadWrite
    U16  BitPos; // currently not used
    UL32 number; // BreakPoint-Number
    I32  rcount; // Break is taken when rcount = 1

    I32   ocount; // Original Count
    void *ep;     // conditional-Expression
    char *cmd;    // Exec-Command
    char *Line;   // Breakpoint-Expression Line for Display
    char *pF;     // module file name
    UL32  nLine;  // line number
    UC8   Opc[8]; // Opcode-Save Area for Monitors

    //---7.12.2006: extended info - present only if 'ExtInfo' is set
    UL32 ExtInf0;
    UL32 ExtInf1;
    UL32 ExtInf2;
    UL32 ExtInf3;
    UL32 ExtInf4;
    UL32 ExtInf5;
    UL32 ExtInf6;
    UL32 ExtInf7;
    UL32 ExtInf8;
    UL32 ExtInf9;
    //------------
    void *pRes;      // internal use only
    UL32  nTickMark; // Break via Socket: TimeStamp to identfy later
    UL32  nRes[15];  // reserved for future internal use
};
#pragma pack()

#define AG_BP struct AG_Bps


/*
 *  AG_BpInfo command codes
 */
#define AG_BPQUERY    0x01
#define AG_BPTOGGLE   0x02 // not sent to target
#define AG_BPINSREM   0x03 // not sent to target
#define AG_BPACTIVATE 0x04 // not sent to target
#define AG_BPDISALL   0x05 // Notification: all Bp's have been disabled
#define AG_BPKILLALL  0x06 // Notification: all Bp's have been killed
#define AG_BPEXQUERY  0x07
#define AG_CADRVALID  0x10  // Is code address valid ? \
                            // added these /8.2.99/
#define AG_BPENABLE    0x08 // Notification: Enable Bp at address
#define AG_BPDISABLE   0x09 // Notification: Disable Bp at address
#define AG_BPKILL      0x0A // Notification: Kill Bp at address
#define AG_BPSET       0x0B // Notification: Set Bp at address
#define AG_CANHANDLEBP 0x0C // Query: can set/clear Bp currently  /22.12.2005/ \
                            //        even if target is running ?

/*
 *  AG_BreakFunc command codes
 */
#define AG_BF_LINK      0x01 // Link break
#define AG_BF_UNLINK    0x02 // Unlink break
#define AG_BF_QUERY     0x03 // Query break
#define AG_BF_CHANGE    0x04 // Enable/Disable break
#define AG_BF_ACCEPT    0x05 // AGDI DLL can currently accept break
#define AG_BF_CLEARATTR 0x06 // Clear break from mem attribute cache
#define AG_BF_SETATTR   0x07 // Set break to mem attribute cache
#define AG_BF_GETHEAD   0x08 // Get head of linked AG_BP list
#define AG_BF_TARGET    0x09 // Install/uninstall break on target


#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_BpInfo(U16 nCode, void *vp);
extern _EXPO_ AG_BP *AG_BreakFunc(U16 nCode, U16 n1, GADR *pA, AG_BP *pB);
#else
extern U32 (*AG_BpInfo)(U16 nCode, void *vp);
extern AG_BP *(*AG_BreakFunc)(U16 nCode, U16 n1, GADR *pA, AG_BP *pB);
#endif

/*
 * U32 AG_BpInfo (U16 nCode, void *vp);
 *   nCode := AG_BPQUERY, AG_BPINSREM, ...
 *      n1 :=  
 */

/*
 * AG_BP *AG_BreakFunc (U16 nCode, U16 n1, GADR *pA, AG_BP *pB);
 *   nCode := AG_BF_LINK, AG_BF_UNLINK, ...
 *      n1 :=  
 */

/*
 * Go/Step function
 */

#define AG_STOPRUN 0x01    // force target to stop Go/Step. \
                           //   nSteps = unused (always 0)  \
                           //   pA     = unused (always NULL)
#define AG_NSTEP 0x02      // execute 'n' steps                                \
                           //   nSteps = number of single instructions to step \
                           //   pA     = unused (always receives GADR struct with adr = 0x0)
#define AG_GOTILADR 0x03   // go til address               \
                           //   nSteps = unused (always 0) \
                           //   pA     = address for temporary breakpoint
#define AG_GOFORBRK 0x04   // go forever or some Bp fires  \
                           //   nSteps = unused (always 0) \
                           //   pA     = unused (always receives GADR struct with adr = 0xFFFFFFFF)
#define AG_WAITFORBRK 0x05 // AGDI DLL to wait forever or some Bp fires (target already running)                                \
                           // Used when connecting to running target without stopping it,                                       \
                           // or when detecting that a target was set running by external stimulus (e.g. Reset button on board) \
                           //   nSteps = unused (always 0)                                                                      \
                           //   pA     = unused (always NULL)


#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_GoStep(U16 nCode, U32 nSteps, GADR *pA);
#else
extern U32 (*AG_GoStep)(U16 nCode, U32 nSteps, GADR *pA);
#endif



/*
 * Serial-Window I/O (thread-safe)
 */


struct SerAS {
    DWORD n1;
    DWORD n2;
    DWORD n3;
    DWORD n4;
    GVAL  v;
};

#define AG_SERBOUT 0x01 // write nMany bytes to Serial #1 Window
#define AG_SERWOUT 0x02 // write nMany words to Serial #1 Window
#define AG_SERXIN  0x03 // Key was pressed in Serial Window #1 or #2

#define AG_ITMBOUT 0x04 // ITM-Channel data for logging


#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_Serial(U16 nCode, U32 nSerNo, U32 nMany, void *vp);
// nCode := AG_SER?OUT / AG_SERXIN
// nSerNo:  0:=Serial #1, 1:=Serial #2
// nMany:   number of items
// vp:      pointer to items (UC8 or U16)
#else
extern U32 (*AG_Serial)(U16 nCode, U32 nSerNo, U32 nMany, void *vp);
#endif


/*
 * Register and Memory access functions
 */

#define AG_READ     0x01 // read operation
#define AG_WRITE    0x02 // write operation
#define AG_WROPC    0x03 // write opcodes
#define AG_RDOPC    0x04 // read opcodes
#define AG_RDMMU66  0x05 // 19.12.99, read Sle66 MMU content
#define AG_WRMMU66  0x06 // 19.12.99, write single Sle66 MMU-Descr.
#define AG_RCRC     0x07 // Sle66 read CRC
#define AG_RDSHIELD 0x08 // Sle66 read Shield
#define AG_RDACE    0x09 // Sle66 read ACE

//--- 8.7.2002: added for flash download, otherwise like 'AG_WRITE'
#define AG_F_WRITE  0x0A // Write to Flash memory (download)
#define AG_F_VERIFY 0x0B // Verify FLash Memory
#define AG_F_ERASE  0x0C // Erase Flash
#define AG_F_RUN    0x0D // Start flashed App.

//---2.4.2003: added for SmartMX Peripheral Access
#define AG_RD_XDES  0x10 // MXP: read DES (16 Bytes)
#define AG_RD_XAES  0x11 // MXP: read AES (16 Bytes)
#define AG_RD_XCRC  0x12 // MXP: read CRC (2 Bytes)
#define AG_RD_XVECT 0x13 // MXP: read Vectors (10 DWORDS, prepared)

//---22.7.2003: added for SLE66 RF Interface
#define AG_RDRF   0x14 // SLE66: Read RF Fifo registers
#define AG_WRRF   0x15 // SLE66: Write RF Fifo registers
#define AG_RDMOVB 0x16 // SLE66: Read  SFR via MOVB
#define AG_WRMOVB 0x17 // SLE66: Write SFR via MOVB

//---26.8.2004: added for SLE66 Special Access
#define AG_BONVMREAD  0x18 // SLE66: BoNVMRead Macro
#define AG_BONVMWRITE 0x19 // SLE66: BoNVMWrite Macro


/*
 * added commands to read/write CM4-FPU registers   /24.5.2010/
 */
#define AG_READFPU  0x1A // read FPU registers
#define AG_WRITEFPU 0x1B // write FPU registers


/*
 * v8-M Security Extensions for "Register and Memory Access Functions"
 */
#define AG_READSEC  0x1C // read Security Extension and Authentication related registers
#define AG_WRITESEC 0x1D // write Security Extension and Authentication related registers (if writeable)

// MVE extensions
#define AG_READEXT  0x1E // read MVE registers
#define AG_WRITEEXT 0x1F // write MVE registers



#pragma pack(1)

struct BoNvmFunc {         // for BoNVMRead/BoNVMWrite
    unsigned char Mode;    // access mode
    unsigned char buf[85]; // read/write buffer
};
#pragma pack()



/*
 * Interface for Memory-accesses where the access size  /24.5.2010/
 * can be specified in the command parameter block
 */

//---Access-commands:
#define ACCMX_READ      1   // read memory
#define ACCMX_WRITE     2   // write memory
#define ACCMX_QUERY_ACC 3   // query access attributes for memory range,        \
                            // use access buffer to fetch the access attributes \
                            // others may specified...
#define ACCMX_WRITE_FLASH 4 // Write data from AGDI DLL flash download buffer to target memory

//---Access-sizes:
#define ACCMX_U8    1    // BYTE access
#define ACCMX_U16   2    // U16 access
#define ACCMX_U32   4    // U32 access
#define ACCMX_ANYSZ 0x80 // Debugger decides which access size(s) to use. nMany is in Bytes.

//---Access-types:
#define ACCMX_TYPE_MEM 0 // Memory access
#define ACCMX_TYPE_AP  1 // Access Port access
#define ACCMX_TYPE_DP  2 // Debug Port access

//---Access-attributes for ACCMX_QUERY_ACC:
//#define ACCMX_U8      1      // BYTE access
//#define ACCMX_U16     2      // U16 access
//#define ACCMX_U32     4      // U32 access
//#define ACCMX_ANYSZ 0x80     // Debugger decides which access size(s) to use. nMany is in Bytes.

//---Access-security-types:
#define ACCMX_SECTYPE_ANY 0     // Any access, use for normal debug access to   \
                                // unbanked memory/memory-mapped registers      \
                                //   for ARMv8-M:                               \
                                //     - Currently configured SCS register view \
                                //     - Secure access on MEM-AP
#define ACCMX_SECTYPE_NSECURE 1 // Non-Secure Access, use to force a non-secure access to \
                                // all memory/memory-mapped registers                     \
                                //   for ARMv8-M:                                         \
                                //     - Non-Secure SCS register view                     \
                                //     - Non-Secure access on MEM-AP
#define ACCMX_SECTYPE_SECURE 2  // Secure Access, use to force a secure access to all \
                                // memory/memory-mapped registers                     \
                                //   for ARMv8-M:                                     \
                                //     - Secure SCS register view                     \
                                //     - Secure access on MEM-AP
#define ACCMX_SECTYPE_CPU 3     // CPU Security State, use to access banked memory/memory-mapped \
                                // registers as per current CPU state                            \
                                //   for ARMv8-M:                                                \
                                //     - SCS register view as per CPU state                      \
                                //     - Secure access on MEM-AP

//---MemAccX/AG_MemAccX return codes:
#define ACCMX_NOTSUPP    0   // function not supported
#define ACCMX_DONE       1   // AG_MemAccX executed normally.
#define ACCMX_FAILED     2   // AG_MemAccX failed. Check ACCMEMX for more details.
#define ACCMX_CONNECTION 3   // No target connection
#define ACCMX_BADPARM    100 // invalid input parameters
#define ACCMX_BADCMD     101 // invalid input command

// ---MemAccX/AG_MemAccX error codes (nErr field):
#define ACCMX_ERR_GENERAL    0  // Target access failed
#define ACCMX_ERR_RUN        1  // Cannot access resource while target is running
#define ACCMX_ERR_NOBLOCK    3  // Block access not allowed for this resource
#define ACCMX_ERR_ALIGN      4  // Unsupported unaligned resource access
#define ACCMX_ERR_ACC_SIZE   5  // Unsupported access size for this resource
#define ACCMX_ERR_ACC_BOUNDS 6  // Access exceeds supported bounds of requested resource
#define ACCMX_ERR_NOSUPP_DP  7  // DP access not supported (also if PDSC Debug disabled)
#define ACCMX_ERR_NOSUPP_AP  8  // AP access not supported
#define ACCMX_ERR_INVALID_DP 9  // Invalid DP ID selected
#define ACCMX_ERR_INVALID_AP 10 // Invalid AP index selected
#define ACCMX_ERR_SWITCH_DP  11 // Switching DP failed
#define ACCMX_ERR_ACC_CHANGE 12 // Access attributes changed (ACCMX_QUERY_ACC - if change in attributes, \
                                // ACCMX_READ/ACCMX_WRITE - if change in attributes and access cannot be \
                                // completed)
#define ACCMX_ERR_SECURE 13     // Debug access failed because of security settings

#pragma pack(1)
//---Command parameter block:
typedef struct accmemx {
    DWORD accCmd      : 8; // in:  ACCMX_xxx
    DWORD accSize     : 8; // in:  ACCMX_U8 / ACCMX_U16 / ACCMX_U32 / ACCMX_ANYSZ, use ACCMX_U8 for accType ACCMX_TYPE_PINS
    DWORD bErr        : 1; // out: memory access error, see nErr for detailed code
    DWORD bNoAddrIncr : 1; // in:  do not increment address for block reads/writes;
                           //      nAddr must be aligned to the access size if this bit is set.
    DWORD bAtom : 1;       // out: access is part of atomic command sequence, result can be fetched after
                           //      execution of ACCDAP::ACCDAP_ATOMIC_EXEC
    DWORD bSkipAccErr : 1; // in:  skip target access error; only applies to errors during communication with target,
                           //      for example invalid parameters still trigger an error (01.03.2019)
    DWORD : 12;            //      currently unused

    UINT64 nAddr; // in: memory address
    DWORD  nMany; // in: number of items to read/write
                  //     item size for ACCMX_ANYSZ is in Bytes

    union {        // pointer to 'nMany * accSize' bytes
        UC8 *pB8;  // U8 buffer
        U16 *pB16; // U16 buffer
        U32 *pB32; // U32 buffer
    };

    UINT64 nErrAddr; // out: error address

    // JR, 18.12.2013: Extensions for PDSC Debug Engine
    DWORD accType     : 8;  // in: ACCMX_TYPE_MEM / ACCMX_TYPE_AP / ACCMX_TYPE_DP
    DWORD bAccessPort : 1;  // in: accessPort field active
    DWORD bDebugPort  : 1;  // in: debugPort field active
    DWORD secureType  : 2;  // in: Security type of access (ACCMX_SECTYPE_ANY / ACCMX_SECTYPE_NSECURE / ACCMX_SECTYPE_SECURE / ACCMX_SECTYPE_CPU)
    DWORD apVersion   : 8;  // in: AP architecture version
    DWORD bAccessAP   : 1;  // in: Access (top-level) AP access (APv2 only)
    DWORD             : 11; //     currently unused
    DWORD accessPort;       // in: Access Port to use for the register/memory access, don't set for accType ACCMX_TYPE_PINS
    DWORD debugPort;        // in: Debug Port ID to use for the register/memory access (as defined by PDSC debug engine), don't set for accType ACCMX_TYPE_PINS

    // Extended error handling
    DWORD nErr;   // out: Detailed error code if bErr == 1
    DWORD nErrDP; // out: DP ID for which the access failed
    DWORD nErrAP; // out: AP index for which the access failed

    DWORD nOffset; // in:  Buffer offset ( ACCMX_WRITE_FLASH )

    DWORD nRes[25]; //  SBZ - reserved, currently unused
} ACCMEMX;
#pragma pack()


#pragma pack(1)
/*
 * Interface for DAP-accesses and DAP related commands /JR, 23.06.2014/
 */

//---DAP-commands:
#define ACCDAP_PINS          1  // Direct Debug Pin Access
#define ACCDAP_DELAY         2  // Delay in debugger firmware
#define ACCDAP_SEQ_SWJ       3  // SWJ Sequence (Value sequence on SWDIO/TMS line)
#define ACCDAP_SEQ_JTAG      4  // JTAG Sequence (Fixed TMS, Value sequence on TDI line)
#define ACCDAP_ABORT         5  // Write 32-bit value to DAP ABORT (SWD-DP: Register access, JTAG-DP: ABORT instruction)
#define ACCDAP_CLOCK         6  // Set debug clock
#define ACCDAP_ATOMIC_START  7  // Start of atomic sequence, all following ACCDAP and ACCMEMX commands are collected until ACCDAP_ATOMIC_EXEC command
#define ACCDAP_ATOMIC_EXEC   8  // End/execute atomic sequence, all commands collected from the last ACCDAP_ATOMIC_START command are executed
#define ACCDAP_ATOMIC_RESULT 9  // Fetch result from previously executed atomic sequence
#define ACCDAP_ATOMIC_ABORT  10 // Abort atomic sequence capture without execution

//---I/O Pin Mapping for DAP-command ACCDAP_PINS:
#define ACCDAP_PIN_TCK_SWCLK 0x01 // TCK_SWCLK
#define ACCDAP_PIN_TMS_SWDIO 0x02 // TMS_SWDIO
#define ACCDAP_PIN_TDI       0x04 // TDI
#define ACCDAP_PIN_TDO_SWO   0x08 // TDO_SWO
#define ACCDAP_PIN_nTRST     0x20 // nTRST
#define ACCDAP_PIN_nRESET    0x80 // nRESET

//---DapAcc/AG_DapAcc return codes:
#define ACCDAP_NOTSUPP    0   // function not supported
#define ACCDAP_DONE       1   // AG_DapAcc executed normally.
#define ACCDAP_FAILED     2   // AG_DapAcc failed. Check ACCDAP for more details.
#define ACCDAP_CONNECTION 3   // No target connection
#define ACCDAP_BADPARM    100 // invalid input parameters
#define ACCDAP_BADCMD     101 // invalid input command

// ---DapAcc/AG_DapAcc error codes (nErr field):
#define ACCDAP_ERR_GENERAL             0  // Access failed
#define ACCDAP_ERR_PIN_INVALID         1  // Input value contains an invalid pin select bit
#define ACCDAP_ERR_PIN_NOTSUPP         2  // Selected pin is not supported
#define ACCDAP_ERR_ATOM_NOTSUPP        3  // Atomic sequence execution not supported by debugger
#define ACCDAP_ERR_ATOM_TOOLONG        4  // Atomic sequence too long, cannot be processed
#define ACCDAP_ERR_ATOM_CMDNOTSUPP     5  // Access command not supported for atomic sequence
#define ACCDAP_ERR_ATOM_ASM            6  // Error assembling atomic sequence
#define ACCDAP_ERR_ATOM_EXEC           7  // Error executing atomic sequence
#define ACCDAP_ERR_PROT_CMDNOTSUPP     8  // Access command not supported for selected protocol
#define ACCDAP_ERR_NOSUPP_DP           9  // DP access not supported (also if PDSC Debug disabled)
#define ACCDAP_ERR_INVALID_DP          10 // Invalid DP ID selected
#define ACCDAP_ERR_SWITCH_DP           11 // Switching DP failed
#define ACCDAP_ERR_CMDNOTSUPP          12 // Access command not supported
#define ACCDAP_ERR_ATOM_CMDNOTSUPPUNIT 13 // Access command not supported for atomic sequence by this debug unit


typedef struct accdap {
    DWORD accCmd     : 8;  // in: ACCDAP_xxx
    DWORD tms        : 1;  // in: Fixed TMS value for ACCDAP_SEQ_JTAG
    DWORD bDebugPort : 1;  // in: debugPort field active
    DWORD bAtom      : 1;  // out: access is part of atomic command sequence, result can be fetched after
                           // execution of ACCDAP::ACCDAP_ATOMIC_EXEC
    DWORD bErr        : 1; // out: DAP access error, see nErr for detailed code
    DWORD bSkipAccErr : 1; // in:  skip target access error; only applies to errors during communication with target,
                           //      for example invalid parameters still trigger an error (01.03.2019)
    DWORD : 19;            // Currently unused
    DWORD reserved;        // Currently unused

    U32 nCnt;      // in : Additional count, e.g. count of bits (ACCDAP_SEQ_SWJ and ACCDAP_SEQ_JTAG)
                   // out: Number of executed operations in single result of Atomic Sequence command (ACCDAP_ATOMIC_RESULT)
    U64 nMask;     // in: Mask (ACCDAP_PINS)
    U32 nDelay;    // in: Delay time in microseconds (ACCDAP_PINS, ACCDAP_DELAY)
    U32 debugPort; // in: Debug Port ID to use for access (as defined by PDSC debug engine)

    DWORD nManyOut;  // in : Number of bytes in output buffer
                     // out: Longest result length in bytes of AG_MemAccX/AG_DapAccAGDI commands executed in atomic sequence (ACCDAP_ATOMIC_EXEC)
    union {          // in: Pointer to 'nMany' output bytes, NULL if unused
        UC8 *pOut8;  // U8  buffer
        U16 *pOut16; // U16 buffer
        U32 *pOut32; // U32 buffer
        U64 *pOut64; // U64 buffer
    };

    DWORD nManyIn;  // out: Number of bytes in input buffer
    union {         // out: Pointer to 'nMany' input bytes, NULL if unused
        UC8 *pIn8;  // U8  buffer
        U16 *pIn16; // U16 buffer
        U32 *pIn32; // U32 buffer
        U64 *pIn64; // U64 buffer
    };

    // Error handling
    DWORD nErr; // out: Error code, 0 means success

    DWORD nRes[32]; // Reserved for future use
} ACCDAP;
#pragma pack()



#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_MemAcc(U16 nCode, UC8 *pB, GADR *pA, UL32 nMany);
extern _EXPO_ U32 AG_RegAcc(U16 nCode, U32 nReg, GVAL *pV);
extern _EXPO_ U32 AG_AllReg(U16 nCode, void *pR);
extern _EXPO_ U32 AG_MemAccX(ACCMEMX *pIn);
extern _EXPO_ U32 AG_DapAcc(ACCDAP *pIn);
extern _EXPO_ U32 AG_BreakResAlloc(U32 nAlloc, AG_BP *pBp); // Optional Function
#else
extern U32 (*AG_MemAcc)(U16 nCode, UC8 *pB, GADR *pA, UL32 nMany);
extern U32 (*AG_RegAcc)(U16 nCode, U32 nReg, GVAL *pV);
extern U32 (*AG_AllReg)(U16 nCode, void *pR);
extern U32 (*AG_MemAccX)(ACCMEMX *pIn);
extern U32 (*AG_DapAcc)(ACCDAP *pIn);
extern U32 (*AG_BreakResAlloc)(U32 nAlloc, AG_BP *pBp); // Optional Function
#endif


/*
 * extract trace history
 */

#ifdef _IN_TARG_ // define if used in Mon166,Emu...
extern _EXPO_ U32 AG_HistFunc(U32 nCode, I32 indx, I32 dir, void *vp);
#else
extern U32 (*AG_HistFunc)(U32 nCode, I32 indx, I32 dir, void *vp);
#endif



#pragma pack(1)

#define DIAD struct DlgD
struct DlgD {   // every dialog has it's own structure
    UL32 iOpen; // auto reopen dialog (pos := 'rc')
    HWND hw;    // Hwnd of Dialog
    BOOL(CALLBACK *wp)
    (HWND hw, UINT msg, WPARAM wp, LPARAM lp);
    RECT rc;                // Position rectangle
    void (*Update)(void);   // Update dialog content
    void (*Kill)(DIAD *pM); // Kill dialog
    void *vp;               // reserved for C++ Dialogs (Dlg *this)
};

#define DYMENU struct DyMenu
struct DyMenu {             // Menu item data structure
    I32   nDelim;           // Menu template delimiter
    char *szText;           // Menu item text
    void (*fp)(DYMENU *pM); // create/bringDlgtoTop function
    UL32  nID;              // uv3 assigned ID_xxxx
    UL32  nDlgId;           // Dialog ID
    DIAD *pDlg;             // link to dialog attributes
};
#pragma pack()

/*
 * nDelim:  1 := normal Menu entry
 *          2 := Popup-Entry (nested submenu)
 *         -2 := end of Popup-Group-List
 *         -1 := total end of Menu-List
 *  text:   the name for the menu/popup-menu entry
 *    fp:   Function to be activated on menu-selection
 */




#if 0 // ----- Examples for AGDI's CallBack functions -----

//--- Example for Force Update:
  pCbFunc (AG_CB_EXECCMD, "dir public");  // execute dir public command
//--- Note: almost any dScope command but 'Exit' can be given.
//---       The 'Exit' command should not be executed !

//--- Example for Force Update:
  pCbFunc (AG_CB_FORCEUPDATE, NULL);
//--- Note: function returns when update is completed.


//--- Example for Set Message String:
  pCbFunc (AG_CB_MSGSTRING, "Running...");  // up to 20 characters
//---       up to 20 characters may be displayed in the statusbar pane.


//--- Example for DisAssemble Opcodes:
  DAAS    parms;

  parms.Adr = 0x10000;                     // disassemble address
  parms.Opc[0] = 0xC0;
  parms.Opc[1] = 0xF0;                     // MovBZ RH7,R0
  parms.Opc[2] = 0;
  parms.Opc[3] = 0;

  pCbFunc (AG_CB_DISASM, (void *) &parms); // disassemble...
//--- on return:  parms.OpcLen := length of opcode in bytes
//---             parms.szB[]  := disassembled instruction in ascii
//--- Note:       parms.Adr is used for ascii-address and relative jmps only.
//---             parms.Result always 0.



//---Example for DisAssemble to file:  /11.1.2005/
  DAS_MIXED  parms;

  memset (&parms, 0, sizeof (parms));      // clear
  parms.nStart = (amCODE << 24) | (amCODE << 16) | 0x0C6A;  // start at C:0x0C6A
  parms.nEnd   = (amCODE << 24) | (amCODE << 16) | 0x0DE7;  // end at C:0x0DE7
  parms.nMode  = 1;                            // use 'mixed' mode
  strcpy (parms.szN, "MixDis.Dis");            // Name of file to create and write
  pCbFunc (AG_CB_DISASM_EXT, (void *) &parms); // disassemble to file...
  switch (parms.Result)  {
    case 0:                                // Ok.
      break;
    case 1:                                // could not create file
      // TODO: error handling
      break;
    case 2:                                // could not write to file (disk full etc.)
      // TODO: error handling
      break;
  }
//------------------------------------------------




typedef struct  {            // 11.1.2005, DisAssemble Mixed-Mode into File
  UL32        nStart;        // Start-Address for disassemble
  UL32          nEnd;        // Stop-Address for disassemble
  I32         Result;        // 0:=Ok, 1:=can't create file, 2:=file write error
  I32          nMode;        // 0:=Asm-Mode, 1:=Mixed-Mode
  UL32       nRes[8];        // reserved
  char     szN [300];        // name of file to create and write disassembly
} DAS_MIXED;


//--- Example for Inline Assemble:
  DAAS    parms;

  parms.Adr = 0x10000;                     // inline assemble address
  strcpy (parms.szB, "MOV [R8+#0x1200],R1");  // instruction to assemble

  pCbFunc (AG_CB_INLASM, (void *) &parms);    // assemble...
//--- on return:
//---   parms.Result = 0 if successful
//---     parms.OpcLen := length of opcode in bytes
//---     parms.Opc[]  := 'parms.OpcLen' Opcode bytes
//---   parms.Result != 0 if inline assembly failed
//---     everything of parms but 'Result' is invalid.


//--- Example for GetDeviceInfo:
  DEV_X66   parms;

  memset (&parms, 0, sizeof (parms));
  pCbFunc (AG_CB_INLASM, (void *) &parms);    // get device info
//--- On return:
//---   values in DEV_X66 structure.
//--- Note:
//---   AppName[] may be used to store a project target specific ini file
//---     AppName[] should be renamed (example: strcat (AppName, "hitex")
//---     before read or write operations can take place.


//--- Example for Find Symbol by value:

  SYMDSC   parms;

  parms.nMask = AG_SYM_LOC | AG_SYMCON;       // functions  and constants
  parms.val   = 0x10000;                      // some value
  pCbFunc (AG_CB_SYMBYVAL, (void *) &parms);  // search for symbol
//--- On return:
//---   parms.Ok = 1, search was successful
//---     parms.szName[] = name of symbol ('main' for example)
//---     parms.type     = type of symbol (AG_TP_FUNC for example)
//---     parms.val      = value of symbol
//---   parms.Ok = 0, search failed.
//--- Note: publics are searched only


//--- Example for Find Symbol by name:
  SYMDSC   parms;

  strcpy (parms.szName, "main");
  pCbFunc (AG_CB_SYMBYNAME, (void *) &parms);  // search for symbol by name
//--- On return:
//---   parms.Ok = 1, search was successful
//---     parms.szName[] = name of symbol ('main' for example)
//---     parms.type     = type of symbol (AG_TP_FUNC for example)
//---     parms.val      = value of symbol
//---   parms.Ok = 0, search failed.
//--- Note: publics are searched only


//--- Example for Progress-Bar handling:

static OIL  ProgressBar;

//--- initialize progress bar
  ProgressBar.pos   = 0;        // initial position
  ProgressBar.low   = 0;        //  low position percent (0%)
  ProgressBar.hig   = 100;      // high position percent (100%)
  ProgressBar.label = "Loading Monitor...";  // progress label text
  ProgressBar.Job   = PROGRESS_INIT;         // initialize
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);    // do initialization.

//--- Step 1: display progress bar position:
  while (processing)  {
//  ...
    ProgressBar.Job = PROGRESS_SETPOS;
    ProgressBar.pos++;
    pCbFunc (AG_CB_PROGRESS, &ProgressBar);  // set bar to position
//  ...
  }

//--- Step 2: remove progress bar:
  ProgressBar.Job = PROGRESS_KILL;
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);    // kill Progress Bar.



//--- Example for Message Box handling:

/* Add this helper function */
UINT AGDIMsgBox (HWND hWnd, 
                 const char *pMsg,
                 const char *pCaption,
                 UINT uiType,
                 UINT uiDefAnswer) {
  AG_MSGBOX mb;


  mb.hWnd = hWnd;
  mb.pMsg = pMsg;
  mb.pCaption = pCaption;
  mb.uiType = uiType;
  mb.uiDefAnswer = uiDefAnswer;

  return ((UINT)pCbFunc (AG_CB_MSGBOX, &mb));   // Call AGDI messagebox function
}

/* Call in place of MessageBox() */
i = AGDIMsgBox (hMfrm,"Configuration has been changed, Stop Target and take new values ?",
                "Target Monitor Notification",
                MB_OKCANCEL | MB_ICONWARNING, 
                IDOK);

#endif // End of CallBack Samples.



#if 0 // /30.1.2005/ Logic-Analyzer Example

static DWORD SaveAdr;

I32 QueryLaSig (AGDI_LA *pLA)  {
  DWORD     nAdr;
  DWORD    nSize;

  nAdr  = (U32) pLA->nAdr;         // Memory-Address
  nSize = (U32) pLA->nSize;        // Size in Bytes
  SaveAdr = nAdr;        // Note: this is a hook for the example-code below.

//---TODO:
//if (cannot_set tracking-range nAdr...nAdr+nSize-1)  {
//  strcpy (pLA->szError, "Error-Text");   // Reason for 'not supported'
//  return (AGDI_LA_NOTSUPPORTED);
//}

//---OK:
  return (AGDI_LA_OK);             // LA-Signal accepted
}


/*
 * Given LA-Signal has been killed.
 */
U32 KilledLaSig (AGDI_LA *pLA)  {
//---TODO: whatever cleanup is required.
  return (AGDI_LA_OK);
}


/*
 * Send a Data-Record to uVision
 *  uVision will look for the matching LA-Signal based on the write-address
 *  and assign the data record to that La-signal.
 */
U32 SendLaDataRecord (AGDI_LAREC *pLA)  {
  U32    nE;

  nE = pCbFunc (AG_CB_LAREC_DATA, (void *) pLA);  // send data to uVision
  switch (nE)  {
    default:
    case 0:                       // could not find a matching Signal (pLA->nAdr)
      break;
    case AGDI_LA_OK:
      break;
  }
  return (nE);
}

//---TODO: whenever the target detects a write to an address
//         specified by one or more LA-definition specifed address
//         ranges, then a data record needs to be prepared and
//         sent to uVision

// Here is an example which simulates a 1000 hertz sine wave.  It is
// assumed that the loaded user program contains a float variable
// such as:
//    float  f32Test;
//
// That variable can then be specified for logic analyzer in uVision
// Debugging mode in the command window:
//
//   LA f32Test;      // some float variable in user application
//
// The hooked address 'SaveAdr' is the location of 'f32Test' in memory. 

static AGDI_LAREC  la;

static void SineSig (void)  {
  int       i;
  double    volts, frequ, offs, dur, val;

  volts =  2.0;         // peak-to-peak voltage
  offs  =  2.5;         // voltage offset
  frequ = 1000;         // frequency
  dur   =  0.2;         // duration in seconds

  memset (&la, 0, sizeof (la));
  la.nAdr    = SaveAdr; // use the previously hooked memory address '&f32Test'
  la.tStamp  = 10;      // simulate the time-stamp
  la.totTime = 10;      // simulate the total running time

  for ( i = 0 ; i < (dur * 100000) ; ++i )  {
    val = (float) sin (frequ * (((double) la.totTime) / 33e6) * 2 * 3.14159);
    la.v.f32  = (float) ((val * volts) + offs);  // setup the 'written' value
    SendLaDataRecord (&la);                      // send data to uVision
    la.totTime += 10;              // and incrase 'total time'
    la.tStamp  += 10;              // simulate next time-stamp
  }                     // Note: total Time and time-stamp
}




U32 _EXPO_ AG_Init (U16 nCode, void *vp)  {
  U32     nE;

  nE = 0;
  switch (nCode & 0xFF00)  {
    case AG_INITFEATURES:     // Initialize & start the target
      PlayDead = 0;           // clear some variables...
      ...
      supp.LaSupp   = 1;          // enable: Logic-Analyzer support
      break;

    case AG_EXECITEM:             // execute various commands
      switch (nCode & 0x00FF)  {
        case AG_QUERY_LASIG:      // is LA-Signal acceptable ?
          if (vp == NULL)  {      // NOTE: just for Test...
//---NOTE: vp == NULL is just for test only, it is not a real case !!!
            SineSig();            // generate a sine Wave for test data
            break;                // LA should show the sine wave...
          }
          nE = QueryLaSig ((AGDI_LA *) vp);
          break;
        case AG_KILLED_LASIG:     // LA-Signal was killed
          nE = KilledLaSig ((AGDI_LA *) vp);
          break;

      ...
  }
  return (nE);
}


#endif




#if 0 // /26.1.2005/ LOADPARMS-Example
/*
 * AG_INITSTARTLOAD with load-parameters Example
 */
LOADPARMS       lParms;           // LOAD-Parameters

U32 _EXPO_ AG_Init (U16 nCode, void *vp)  {
  U32     nE;

  nE = 0;
  switch (nCode & 0xFF00)  {
    case AG_INITFEATURES:         // Initialize & start the target
      PlayDead = 0;               // clear some variables...
      // ...
      break;

    case AG_INITITEM:             // init item
      switch (nCode & 0x00FF)  {
        case AG_INITMENU:         // init extension menu
          *((DYMENU **) vp) = (DYMENU *) Menu;
          break;
        // ...
        case AG_INITCALLBACK:     // pointer to callback function
          pCbFunc = (pCBF) vp;    // call-back function of s166
          break;

        case AG_INITSTARTLOAD:    // about to start 'load file'
          if (vp != NULL)  {      // Load-Parameters specified
            lParms = *((LOADPARMS *) vp);
//  lParms.szFile[]    : full path name of App to load
//  lParms.Incremental : 1:= incremental load
//  lParms.NoCode      : 1:= load debug info only, no code (currently not used by uVision)
// TODO: process the given load parameters if required...

          }
          break;

        case AG_INITENDLOAD:      // Load is now completed.
          break;

      }
      break;
    // ...
    case AG_GETFEATURE:           // uVision2 want's details about features...
      // ...

  }
  return (nE);
}
#endif




#if 0                       // TestCode for AG_CB_ENUMFUNCTIONS / AG_CB_GETSCOPEINFO   /21.9.2010/
// This code can be directly used in the AGDI-driver:

static AG_SCOPE   ac;
static AG_BLOCK   ab;



//#define ADR_MASK 0xFFFFFFFF                 // Cortex/Arm7 etc.
#define ADR_MASK 0x00FFFFFF // 51/251



/*
 * Application: map address to scope-info (Function-address range, line-numbers and full-path-src)
 */
static void TestScope (DWORD nAddr)  {
  U32        nE, line;
  DWORD       n;
  AG_LINE   *pL;
  char      szB[512], *pSrc;

  ac.AddrIn   = nAddr;            // [I] - Address to get Scope-Info for.
  ac.nRes[31] = 1;                // [I] - want source line
  nE = pCbFunc (AG_CB_GETSCOPEINFO, (void *) &ac);    // send data to uVision
  if (nE == 0)  {                 // Scope-Mapping failed...
    return;
  }

//---Now, ag_scope 'ac' has the following info:
// 
//  ac.pModule    := Name of Module
//  ac.pFunc      := Name of Function
//  ac.lowPC      := start address of function
//  ac.higPC      := last valid address of function
//  ac.nLines     := number of lines in Module 'pModule'
//  ac.curLine    := nAddr maps to line 'ac.curLine' in File 'ac.szPath[]'
//  ac.pLines     := Array[ac.nLines] of Linenumber/address pairs for module 'ac.pModule' 
//  ac.szPath     := full path name of associated source file
//  if (ac.nRes[31] was != 0 on input)  {  // source line requested
//    char *pHll = (char *) ac.nRes[30];   // nRes[30] := &src_line_buffer[0]
//  }
//  otherwise ac.nRes[30] would be NULL.

//---could now walk through lines of module 'ac.pModule':
  if (ac.nLines != 0)  {           // array of line/address pairs is present
    pL = ac.pLines;                // array of line/address pairs
    _snprintf (szB, sizeof (szB)-2, "  %d lines in module '%s' (%s)\n", ac.nLines, ac.pModule, ac.szPath);
    txtout (szB);

    for ( n = 0 ; n < ac.nLines ; ++n, ++pL )  {
      _snprintf (szB, sizeof (szB)-2, "    A:=0x%08X, L:=%d\n", pL->addr & ADR_MASK, pL->line);
      txtout (szB);
    }
  }
}


static const DWORD SizeFromTp[] =  {
   0,  // #define  bt_void      0
   0,  // #define  bt_bit       1
   1,  // #define  bt_char      2
   1,  // #define  bt_uchar     3
   2,  // #define  bt_int       4
   2,  // #define  bt_uint      5
   2,  // #define  bt_short     6
   2,  // #define  bt_ushort    7
   4,  // #define  bt_long      8
   4,  // #define  bt_ulong     9
   4,  // #define  bt_float    10
   8,  // #define  bt_double   11
   4,  // #define  bt_ptr      12
   0,  // #define  bt_union    13
   0,  // #define  bt_struct   14
   0,  // #define  bt_func     15
   0,  // #define  bt_string   16
   0,  // #define  bt_enum     17
   0,  // #define  bt_field    18
   8,  // #define  bt_int64    19
   8,  // #define  bt_uint64   20
};

static DWORD CalcSize (TYP *tp)  {
  DWORD     nSize = 0;

  if (tp == NULL)  {
    return (0);
  }
  nSize = tp->size;                              // size of type in bytes
  switch (tp->type)  {
    case bt_ptr:
      if (tp->v_flag)  {                         // an array of ...
        nSize *= CalcSize (tp->btp);
      }
      break;
    case bt_func:
    case bt_void:
    case bt_bit:
    case bt_string:
    case bt_field:
      nSize = 0;                                 // these are illegal for CalcSize.
      break;
  }
  return (nSize);
}



/*
 * Dump symbol classes, addresses, type-sizes and names
 */
static void DumpModuleSymbols (const char *pModule, SYM *sp)  {
  char      szB[512];
  DWORD   nSize;

  _snprintf (szB, sizeof (szB)-2, " Symbol-dump for module '%s':\n", pModule);
  txtout (szB);

  for ( ; sp ; sp = sp->next )  {           // for all symbols in chain
    nSize = CalcSize (sp->tp);              // calc size of symbols type
    switch (sp->type)  {                    // symbol-classifier
      default:                              // not relevant
        break;
      case F66_BIT:                         // some bit
        _snprintf (szB, sizeof (szB)-2, "  Bit - A:=0x%08X, '%s'\n",       sp->v.ul & ADR_MASK, sp->name);
        txtout (szB);
        break;
      case F66_VAR:                         // variable
        _snprintf (szB, sizeof (szB)-2, "  Var - A:=0x%08X, S=%d, '%s'\n", sp->v.ul & ADR_MASK, nSize, sp->name);
        txtout (szB);
        break;
      case F66_LOC:                         // some function or label
        _snprintf (szB, sizeof (szB)-2, "  Loc - A:=0x%08X, S=%d, '%s'\n", sp->v.ul & ADR_MASK, nSize, sp->name);
        txtout (szB);
        break;
    }
  }
}



/*
 * Enumerate all scopes of the loaded application(s)
 */
static void TestEnumScopes (void)  {
  UL32     nE;
  ZBK     *pB = NULL;
  char    szB[512];

  memset (&ab, 0, sizeof (ab));             // clear -->  ***REQUIRED!!!***
  nE = pCbFunc (AG_CB_ENUMFUNCTIONS, (void *) &ab);    // initialize for enumeration
  if (nE == 0)  {                                      // failed...
    return;
  }
  while (1)  {                              // while more enumerations...
    nE = pCbFunc (AG_CB_ENUMFUNCTIONS, (void *) &ab);    // ask core-driver
    if (nE == 0) break;                     // Enumeration finished.
    if (ab.pT != (void *) pB)  {            // Theader changed (another module)
      pB = (ZBK *) ab.pT;
      if (pB && pB->shd)  {                 // THeader symbols - publics and statics
        DumpModuleSymbols (ab.pModule, pB->shd);
      }
      pB = (ZBK *) ab.pT;
    }

    _snprintf (szB, sizeof(szB)-2, "AGDI: \\\\%s\\%s\\%s(): lowPC=0x%08X, higPC=0x%08X\n",
                    ab.pApp, ab.pModule, ab.pFunc, ab.lowPC, ab.higPC);
    txtout (szB);                           // write result into command-output-Window.
  }
}




U32 _EXPO_ AG_Init (U16 nCode, void *vp)  {
 …

        case AG_RESET:            // perform a reset on the target system
          if (PlayDead || hMfrm == NULL) break;  // target is disconnected
          ResetTarget();          // reset the target
          if (pCbFunc)  {
            pCbFunc (AG_CB_EXECCMD, "U $\n");        // dasm $
//---Hack: for test, dump/map scopes on AG_RESET:
            TestScope (0x01000290);            // map address to scope
            TestEnumScopes();                  // enumerate all scopes
//--------
          }
          break;
  …
}


#endif


// menu item identifiers for external target DLLs:
#define AG_EXT_CMD_DEBUG_VIEW_TRACE_RECORDING 32000 // View Trace-Records
#define AG_EXT_CMD_DEBUG_ENA_DISA_BREAKPOINTS 32001
#define AG_EXT_CMD_DEBUG_BREAKPOINTS          32002

#define AG_EXT_CMD_DEBUG_DBGSETTINGS          32004
#define AG_EXT_CMD_DEBUG_ENA_DIS_TRACE        32005 // Enable/Disable Trace-Recording




#ifdef __cplusplus
}
#endif




#endif // __AGDI__INCED___
