/**************************************************************************//**
 *           COMTYP.H: µVision interface functions and globally
 *                     used structures and type definitions.
 *
 * @version  V1.0.8
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

#ifndef COMTYP_INCED
  #define COMTYP_INCED

/*
 * Common type defintions
 */

#ifndef _WINDEF_        // WinDef.h not included
 typedef   signed char         CHAR;
 typedef unsigned int         DWORD;
 typedef   signed int         INT32;
 typedef unsigned short int  UINT16;
// typedef unsigned short int    WORD;
 typedef signed int           INT32;
 typedef unsigned char         BOOL;
#endif

typedef unsigned char          BYTE;
typedef   signed short int    INT16;
typedef unsigned short int   WORD16;
typedef __int64               INT64;
typedef unsigned __int64     UINT64;
typedef WORD16                 *WFP;
typedef WORD16     const      *WCFP;
typedef BYTE                   *CFP;


#define iMCS51   0           // ToolSet Definitions
#define iMCS251  1
#define iMCS166  2
#define iMCSARM  3           // Gnu-ARM tools
#define iARMADS  4           // ARM-ADS tools
#define iARMKEIL 5           // Keil-ARM tools

//#define LIMMCS   iARMADS   // last valid environment
#define LIMMCS   iARMKEIL    // last valid environment


#define MAXTDRV  40          // max. target drivers per [Section]
#define MAXTSIM  10          // max. Sim DLLs per [Section]
#define MAXRTOS  12          // max. RTOS entries per [Section]
#define MAXAGSI  20          // max. automatically loaded AGSI DLL's per [Section]

#define MAXTIP   1024



union bw  {
  WORD16    word;        // UINT16
  BYTE   byte[2];
};

union uw  {
  WORD16   ui[2];
  DWORD       ul;
};


/*
 * Helper for New Books-Property Page
 */
typedef struct Bki  {
  char      *pTitle;
  char       *pPath;
  int          lang;
  struct Bki  *next;
} BOOKITEM;


#pragma pack(1)


/*
 * 28.10.2001: MainFrame.CPP:
 */
extern int GetTabSize (int nTools, int fType);


/*
 * Message-Subtypes for Registered Message 'WM_OUTOUT':
 */

#define MSG_UV2_TEXTOUT         0x0002
#define MSG_UV2_SERBOUT         0x0003
#define MSG_UV2_SERWOUT         0x0004
#define MSG_UV2_TERMINATE       0x0005
#define MSG_UV2_TRNLINE         0x0006   // Write into Build-Output window
#define MSG_UV2_AFTERBLD        0x0007   // post build/rebuild/translate
#define MSG_UV2_AFTERSVCS       0x0008   // post SVCS command
#define MSG_UV2_AFTERLINT       0x0009   // post LINT command
#define MSG_UV2_NOTIFY          0x000A   // Notify message
#define MSG_UV2_SERVWKEY        0x000B   // Key pressed in Serial View
#define MSG_UV2_CMDLINE         0x000C   // Debug: write into command window
#define MSG_UV2_MSGSTRING       0x000D   // Setup Message String
#define MSG_UV2_PRJCHANGE       0x000E   // Project File has changed
#define MSG_UV2_SERBPACK        0x000F   // packed serial byte out
#define MSG_UV2_CMDBUFLINE      0x0010   // Write puffered line into Command-Window
#define MSG_UV2_STARTDBG        0x0011   // Start Debugging
#define MSG_UV2_REBUILD         0x0012   // Start Build/Rebuild
#define MSG_UV2_STOPBUILD       0x0013   // Stop Build/Rebuild
#define MSG_UV3_CLEANTARG       0x0014   // 'Clean' current target
#define MSG_UV5_RTECHANGE       0x0015   // Rte index file has changed (packs are added/removed)
#define MSG_UV5_GPDSCCHANGE     0x0016   // gpdsc file associated with the project  has changed
#define MSG_UV2_TRNLINE_N       0x0017   // Write into Build-Output window
#define MSG_UV2_AFTEMERGE       0x0018   // Merge-Utility has been finished
#define MSG_UV2_EVTREC_STAT     0x0019   // Event Recorder Status Update ready to be polled
#define MSG_UV2_SA_CH_UPDATE    0x0020   // System Analyzer Channel Info Update Available
                                         // Use with ::PostMessage(), e.g. ::PostMessage (pio->hwnd, pio->hmsg, (WPARAM)MSG_UV2_SA_CHANNEL_UPDT, (LPARAM)0x00000001);
                                         // LPARAM Values:
                                         //   - SA_CH_UPDATE_RESET : Clear all channels and refresh full channel information
                                         //   - 0x000mnnnn
                                         //      - m    = Update Operation, see SA_CH_UPDATE_xxx codes
                                         //      - nnnn = Channel ID



#define EBRK  struct edbrk

struct edbrk  {
  EBRK          *next;
  DWORD         nLine;       // line number
  DWORD          nAdr;       // and it's address

  WORD16     type : 3;       // 0:=AdrBrk, 1:=CondBrk, 2:=WatchBrk
  WORD16  enabled : 1;       // Break is enabled/disabled
  WORD16   BytObj : 1;       // WatchBrk: 0:=Bytes, 1:=Objects
  WORD16  HtxType : 8;       // IFX/Hitex Breakpoint type  /26.1.2012/

  DWORD          many;       // WatchBrk: many objects or bytes
  DWORD         tsize;       // WatchBrk: size of object
  WORD16          acc;       // WatchBrk: 1:=Read, 2:=Write, 3:=Both

//WORD16       number;       // BreakPoint-Number
//WORD16       BitPos;
  int          rcount;       // Break is taken when rcount = 1
  char           *cmd;       // Exec-Command
  char          *Line;       // Breakpoint-Expression Line for Display

  EBRK         *enext;       // for editor internal use
  BYTE         killed;       // for editor internal use
  char  pF [MAX_PATH + 2];   // name of file
};


#define SITEM  struct sItem

struct sItem  {
  struct sItem  *next;
  char            *pS;       // item text
  char            *pL;       // button label
  WORD16         nTyp;       // SubType
};

#pragma pack()


struct pax  {
  INT64       cyc;
  double      tim;
};


#pragma pack(1)
typedef struct  {            // Memory-Range descriptor
  BYTE          mTyp;        // 0=RAM, 1=ROM
  DWORD       nStart;        // Memory start address
  DWORD        nSize;        // Size of memory block (0=unused)
} MEMRANGE;


typedef struct xopt  {       // list target/flash settings /Note: do not change this structure !!!/
  struct xopt      *next;    // next item in list
  char              *key;    // key:  e.g. 'UL2CM3' / 'ULP2CM3' etc.
  char             *name;    // name: e.g. '-U -O142 -S0 -C-1 -TO18 -TC10000000 ...'
} XOPT;
#pragma pack()



/*
 * Debug-Startup values
 */

struct dbgblk  {
  char    MainDll [MAX_PATH];    // 's166.dll'
  char    PeriDll [MAX_PATH];    // '80167.dll'
  char   MainArgs [MAX_PATH];    // args to MainDll
  char   PeriArgs [MAX_PATH];    // args to PeriDll

  char    TargDll [MAX_PATH];    // target dll (MON, Emu,...)
  char       TargArgs [1024];    // Arguments to/from MON,Emu,...
  char       tKey [MAX_PATH];    // ...Mon166.DLL --> Mon166

  char         *pjPath;          // Project Path
  char    AppName [MAX_PATH];    // App name to load
  char    IniFile [MAX_PATH];    // INI-file to execute
  char    FncFile [MAX_PATH];    // File with target specific functions

  EBRK           *pBrk;          // list of breakpoints
  SITEM          *pWa1;          // Watch #1 Items
  SITEM          *pWa2;          // Watch #2 Items
  SITEM         *pMem1;          // Memory #1 Item
  SITEM         *pMem2;          // Memory #2 Item
  SITEM          *pBox;          // ToolBox Buttons
  SITEM          *pPad;          // PA definitions
  SITEM          *pSym;          // SymDlg Settings
//SITEM          *pCov;          // Coverage Dialog attrib(s)

  DWORD       targ : 1;
  DWORD      LdApp : 1;          // load app at on startup
  DWORD     Gomain : 1;          // Sim:  Go till main()
  DWORD     Rbreak : 1;          // Sim: restore breakpoints
  DWORD     Rwatch : 1;          // Sim: restore watchpoints
  DWORD       Rmem : 1;          // Sim: restore memory display
  DWORD      Rfunc : 1;          // Sim: restore functions
  DWORD       Rbox : 1;          // Sim: restore Toolbox
  DWORD   AutoFail : 1;          // Sim: Autoload failed
  DWORD   NoTarget : 1;          // Connect to debug unit only, not to target
                                 // 22 Bits unused


  SITEM         *pMem3;          // Memory #3 Item
  SITEM         *pMem4;          // Memory #4 Item

  char     RtxDll [MAX_PATH];    // Rtx-Display dll
  char    RtxArgs [1024];        // Arguments to/from Rtx-Dll,...
  char     RtxKey [MAX_PATH];    // ...RtxTiny.DLL --> RtxTiny

                                 // iMCS166
  MEMRANGE   ExtMem[6];          // up to 6 external memory ranges
  MEMRANGE        Ican;          // address range of on-chip CAN
  MEMRANGE        Irom;          // address range of on-chip internal Rom
  MEMRANGE       Xram1;          // address range of on-chip internal Xram
  MEMRANGE       Xram2;          // address range of on-chip internal Xram
  MEMRANGE        Iram;          // address range of on-chip internal Ram
  MEMRANGE      Dflash;          // address range of on-chip data flash
  BYTE          uocROM;          // use on-chip rom (166+51)
  BYTE         uocXCAN;          // use on-chip xram+can
  BYTE            Rtos;          // OS: 0:=none, 1:=RtxTiny, 2:=RtxFull
  BYTE          Mod167;          // 1:=use 167 instructions
                                 // Common
  DWORD          Clock;          // clock frequency
  char     Vendor [64];          // Device-Vendor: example 'Siemens'
  char     Device [64];          // Device-Name: example 'C167CR-16FM'

                                 // iMCS51
  WORD16         RomSz;          // 0:=Rom(Sm), 1:=Rom(Cp), 2:=Rom(La)
  DWORD    uocArit : 1;   // use on chip Arithmetic Unit
  DWORD    uocMdpt : 1;   // use multiple data pointers
  DWORD    uocXRAM : 1;   // use on chip XRAM
  DWORD    hadIRAM : 1;   // IRAM(...) given per database
  DWORD    hadXRAM : 1;   // XRAM(...) given per database
  DWORD    hadIROM : 1;   // IROM(...) given per database
  DWORD      Moda2 : 1;   // MODA2 (Atmel dual dptr)
  DWORD     Moddp2 : 1;   // MODDP2 (Dallas dual dptr)
  DWORD      Modp2 : 1;   // MODP2 (Philips+Temic dual dptr)
  DWORD   Mod517dp : 1;   // multiple dptr for Infineon C500
  DWORD   Mod517au : 1;   // Arithmetic Unit for Infineon C500
  DWORD      Mode2 : 1;   // MODE2 given per database
  DWORD     ModAB2 : 1;   // Analog Devices B2 Series given per Database
  DWORD      useCB : 1;   // CODE-banking (1:=activated)
  DWORD      useXB : 1;   // XDATA-banking (1:=activated)
//DWORD    useL251 : 1;   // use L251 instead of BL51
//DWORD    useA251 : 1;   // use A251 instead of A51
  DWORD    Moddp2x : 1;   // Dallas 89C420
  DWORD     Mod390 : 1;   // Dallas 89C390
  DWORD    ModC812 : 1;   // ADuC812 DPP Register freigeben
  DWORD      Mx51P : 1;   // 80C51MX+Ext
  DWORD     Mod400 : 1;   // Dallas 89C400
  DWORD   hadXRAM2 : 1;   // SmartBull
  DWORD      ModC2 : 1;   // Cast/Evatronix R80515 dual DPTR
  DWORD      ModH2 : 1;   // Hynix/ST uPSD         dual DPTR
  DWORD   Mdu_R515 : 1;   // Cast/Evatronix R80515 MDU
  DWORD   Mdu_F120 : 1;   // Silicon Labs C8051F12x MAC
  DWORD    Mod208x : 1;   // Microchip IS208x                               2.1.2018 BGN

  WORD16        cBanks;   // 0:=2, 1:=4, 2:=8, 3:=16, 4:=32
  WORD16        xBanks;   // 0:=2, 1:=4, 2:=8, 3:=16, 4:=32

  MEMRANGE         rCB;   // CODE-bank  range
  MEMRANGE         rXB;   // XDATA-bank range
  MEMRANGE    eprom[3];   // off-chip code memory
  MEMRANGE    exram[3];   // off-chip xram memory
  MEMRANGE        irom;   // IROM range
  MEMRANGE        iram;   // IRAM range
  MEMRANGE        xram;   // XRAM range

  char     CanDll [MAX_PATH];    // Rtx-Display dll
  char    CanArgs [1024];        // Arguments to/from Can-Dll,...
//char     CanKey [MAX_PATH];    // ...Can.DLL

  char    szMsg [1024];          // for error-message transfer to Uv2
  char     ** AgsiDlls;   // HS 9.4.2002 line added: Pointer to string array containing names of AGSI DLL's

//---more possible Bitfields go on here...
//DWORD          n1Res;   // reserved for future devices
  DWORD     BigEnd : 1;   // 26.3.2003, ARM: 1:=Big, 0:=Little Endian
  DWORD       Lx51 : 1;   // 11.11.2005: Lx51 used to link 51-Project.
  DWORD    ModSmx2 : 1;   // SmartMX2-Instruction-Sim-Mode
  DWORD     Cm4Fpu : 1;   // 1:Use the Cortex-M4 FPU
  DWORD    Cmv5Fpu : 2;   // 0:No Floating Point Unit, 1:single-precision floating-point, 2:double-precision floating-point
  DWORD       unu1 :26;   // Rest of DWORD is unused.

  SITEM           *pLa;   // LA Items
  DWORD          n3Res;   // reserved for future devices
  DWORD          n4Res;   // reserved for future devices
  DWORD          n5Res;   // reserved for future devices
  DWORD          n6Res;   // reserved for future devices

//DWORD          n7Res;   // used for 'pDbgX' below.
  struct dbgext *pDbgX;   // extended Info  /11.11.2005/
//---
};


/*
 * Extended dbg-info  /11.11.2005/
 */
struct dbgext  {          // eXtended dbg-info
  char       szPrjPath[MAX_PATH]; // full project path and name
  char      szCpu[256];   // 'ARM7TDMI', 'ARM9E-S', 'ARM966E-S' etc.
  char       *pRes[31];   // reserved
  char      *pTargName;   // name of current target
  DWORD          nRes1;   // reserved
  DWORD          nRes2;
  DWORD          nRes3;
  DWORD          nRes4;
  DWORD          nRes5;

//DWORD          nRes6;
  struct mpwinfo *pMPI;   // .uvmpw-info  /15.10.2010/

//DWORD          nRes7;   // will be 'theApp.m_pMainWnd'  /6.10.2009/
  void       *pMainWnd;   // will be 'theApp.m_pMainWnd'  /6.10.2009/
//DWORD          nRes8;   // used for 'pBom' below  /9.10.2006/
  void           *pBom;   // will be '&ioc'         /9.10.2006/

//---4.1.2007:
  MEMRANGE    rOCR[10];   // [0]:=ROM1, [1]:=ROM2, [2]:=ROM3, [3]:=IROM1, [4]:=IROM2
                          // [5]:=RAM1, [6]:=RAM2, [7]:=RAM3, [8]:=IRAM1, [9]:=IRAM2
  DWORD      uROM1 : 1;   // 1:=ROM1 used
  DWORD      uROM2 : 1;   // 1:=ROM2 used
  DWORD      uROM3 : 1;   // 1:=ROM3 used
  DWORD     uIROM1 : 1;   // 1:=IROM1 used
  DWORD     uIROM2 : 1;   // 1:=IROM2 used
  DWORD      uRAM1 : 1;   // 1:=RAM1 used
  DWORD      uRAM2 : 1;   // 1:=RAM2 used
  DWORD      uRAM3 : 1;   // 1:=RAM3 used
  DWORD     uIRAM1 : 1;   // 1:=IRAM1 used
  DWORD     uIRAM2 : 1;   // 1:=IRAM2 used

  DWORD     bIsMpw : 1;   // 1:= a '.uvmpw' project
  DWORD     Rtrace : 1;   // 1:= restore tracepoints
  DWORD    CmCdeCp : 8;   // CDE CP enabled vector (Bit 0: CP0, Bit 1: CP1, ..., Bit 7: CP7)
  DWORD            :12;
//---
  char       szMpwPath[MAX_PATH]; // full .uvmpw project path and name
  char       szMaskRev[MAX_PATH]; // IFX-MASK_REV: 'A11S'

  XOPT        *pOpUtil;    // Head of Util's page flash/target settings
  XOPT        *pOpTarg;    // Head of Debug page target settings

//BYTE     szRes[2048 - sizeof(DWORD)/*uROMx*/ - (10*sizeof (MEMRANGE))];   // reserved

  SITEM          *pTP;        /* List of tracepoint items,
                              nTyp: Tracepoint subtype,
                              pS: tracepoint expression + enable status */
  DWORD      nTHDelay;        // TraceHalt trigger delay
  char        szTRoot[MAX_PATH];  // Tool-Root, e.g: C:\KeilV5\ARM

#define __XRSZ__ (sizeof (DWORD)  /*uROMx*/    + \
                  (10 * sizeof (MEMRANGE))     + \
                  MAX_PATH                     + \
                  MAX_PATH                     + \
                  (2 * sizeof (XOPT *)         + \
                  sizeof (SITEM*) /* pTp*/     + \
                  sizeof (DWORD)/* nTHDelay*/) + \
                  MAX_PATH                       \
                 )
  BYTE     szRes[2048 - __XRSZ__];     // reserved
};


/*
 * System-Viewer data
 */
typedef struct sysvdt  {
  struct dbgblk      *p_dbg;
  struct dbgext     *p_dbgX;      // eXtended dbg-info

  DWORD          bHfile : 1;      // master .h file is present
  DWORD          bSfrOk : 1;      // precompiled .h -> .sfr file is present and ok.
  DWORD                 :30;

  char          szDevH[512];      // master .h/.sfr file for given device
                                  // e.g. C:\Keil\Arm\INC\Nxp\Lpc21xx.h/.sfr
  char       szInc[32][512];      // reserved for include folders
  DWORD            nRes[32];      // reserved

  void          *m_pMainWnd;      // Uv4's theApp.m_pMainWnd
  void           *vpRes[31];      // reserved
  char          szRes[4096];
} SYSVDT;





#define __UVMPW_VSN    100        // V1.0

typedef struct subprj  {          // Sub-Project-Descriptor
  struct subprj       *next;      // next sub-project of .uvmpw
  char             *pszPath;      // path of .uvproj subproject file
  char                *pObj;      // output file (.elf/.axf)

  DWORD         bActive : 1;      // 1:=the active project from the .uvmpw
  DWORD                 :30;      // currently unused
  DWORD            nRes[12];      // reserved, currently unused
} SUBPRJ;

typedef struct mpwinfo  {         // UVMPW-Descriptor
  DWORD               nVers;      // internal version 100:=V1.0
  DWORD              numPrj;      // number of projects in .uvmpw
  char             *pszPath;      // .uvmpw path

  SUBPRJ              *head;      // head of sub-projects
  SUBPRJ              *tail;      // tail of sub-projects
  DWORD            nRes[12];      // reserved, currently unused
} MPWINFO;


/*
 * Language-defines for INI-Env strings
 */

#define LANG_N      0       // undefined language
#define LANG_C      1       // C
#define LANG_A      2       // Asm
#define LANG_CPP    3       // C++
#define LANG_UT     4       // Utils
#define LANG_DEV    5       // Device specific
#define LANG_TOOL   6       // Tool specific
#define LANG_GEN    7       // General
#define LANG_BOARD  8       // Board specific



/*
 * Exec: Translators
 */

#define  xxWHAT    0        /* unknown translator */
#define  xxC251    1
#define  xxC51     2
#define  xxC166    3
#define  xxPLM51   4
#define  xxL251    5
#define  xxL51     6
#define  xxL166    7
#define  xxA51     8
#define  xxA251    9
#define  xxA166   10
#define  xxA66    11
#define  xxC66    12
#define  xxL66    13
#define  xxLIB166 14
#define  xxLIB66  15
#define  xxLIB51  16
#define  xxLIB251 17
#define  xxOH51   18
#define  xxOH251  19
#define  xxOH166  20
#define  xxOH66   21
#define  xxOC51   22
#define  xxAX51   23      // Ax51 (Philips 80C51MX)
#define  xxLX51   24
#define  xxCX51   25
#define  xxLIBX51 26
#define  xxOHX51  27
#define  xxLP51   28
#define  xxCC51   29  // Ceibo ECPP Preprocessor
#define  xxECP166 30  // Edg-EC++ for 166/167
#define  xxGARMCC 31  // Gnu-ARM-CC
#define  xxGARMAS 32  // Gnu-ARM-AS
#define  xxGARMLD 33  // Gnu-ARM-LD
#define  xxGARMOH 34  // Gnu-ARM-ObjCopy (Obj-Elf/Hex)
#define  xxGARMAR 35  // Gnu-ARM-AR (Library Manager)

#define  xxADSCC  36  // ARM-ADS: armcc
#define  xxADSAS  37  // ARM-ADS: armas
#define  xxADSLD  38  // ARM-ADS: armlink
#define  xxADSFE  39  // ARM-ADS: fromelf
#define  xxADSAR  40  // ARM-ADS: armar
#define  xxADSCPP 41  // ARM-ADS: armcpp
#define  xxADSTCC 42  // ARM-ADS: tcc (Thumb-Mode-Compiler)

#define  xxKCA    43  // Keil-ARM: CA (Compiler)
#define  xxKAA    44  // Keil-ARM: AA (Assembler)
#define  xxKLA    45  // Keil-ARM: LA (Linker/Locator)
#define  xxKLIBA  46  // Keil-ARM: LIBA (Library-Manager)
#define  xxKOH    47  // Keil-ARM: OH

#define  xxPL70   48  // 251-CC7 PL70.Exe

#define  xxAX2    49  // AX2 (NXP SmartMX2)
#define  xxCX2    50  // CX2 (NXP SmartMX2)
#define  xxLX2    51  // LX2 (NXP SmartMX2)
#define  xxOHX2   52  // OHX2 (NXP SmartMX2)

#define  xxSYSVCC 53  // SfrCC
#define  xxPL79   54  // 251-CC7 PL79.Exe

#define  xxUSER   55
#define  xxLINT   56


extern char szTrn[][132];   // Exec.C, /8.12.2009/


// Defines for MSG_UV2_EVTREC_STAT parameter LPARAM
#define EVTREC_STAT_STATUS_FLAGS 0x00000000 // Event Recorder status flags changed (initialized, overflow, recording, missed events, etc). Poll for update.
#define EVTREC_STAT_TARG_STATUS  0x00000001 // Event Recorder target status changed (e.g. timestamp frequency). Poll for update.

// Defines for MSG_UV2_SA_CH_UPDATE parameter LPARAM
#define SA_CH_UPDATE_RESET     0xFFFFFFFF   // All channels reset, refresh all channel infos
#define SA_CH_UPDATE_ID_MSK    0x0000FFFF   // Mask for channel ID
#define SA_CH_UPDATE_CODE_MSK  0x000F0000   // Mask for channel update code
#define SA_CH_UPDATE_CH_ADD    0x00000000   // Channel added, use ID to look up info
#define SA_CH_UPDATE_CH_DEL    0x00010000   // Channel with ID deleted
#define SA_CH_UPDATE_CH_INFO   0x00020000   // Channel info updated, use ID to look up info
#define SA_CH_UPDATE_SUB_ADD   0x00030000   // Sub-channel added, use ID to look up modified sub-channel list
#define SA_CH_UPDATE_SUB_DEL   0x00040000   // Sub-channel deleted, use ID to look up modified sub-channel list
#define SA_CH_UPDATE_GRP_ADD   0x00050000   // Group added, use ID to add channels
#define SA_CH_UPDATE_GRP_DEL   0x00060000   // Group with ID deleted


#ifdef __cplusplus
  extern "C"  {
#endif


/*
 * in HelpFox.Asm: (in any Syntax-builder help file)
 */

extern int         HELPEXPR;
extern int         CANTHELP;
extern int        HELPSTART;



extern void  ShowError (BOOL box, char *fmt, ...);
extern void TrnOutLine (char *pLine);    // into Build Window
extern void    TrnOutL (char *pLine);    // into Build Window (thread-safe)
extern void CmdOutLine (char *pLine);    // into Command Window
extern void  CmdCmdOut (char *pLine);    // into Command Window
extern void OutOutLine (char *pLine);    // into Output Window
extern void   DoEvents (void);           // keep System alive

extern DWORD   Notify1 (DWORD nCode, void *lp1);  // within thread
extern DWORD    Notify (DWORD nCode, void *lp1);  // thread-safe.


extern int  SerByteOut (int nCom, BYTE *pS, DWORD nMany);
extern int  SerWordOut (int nCom, WORD16 *pW, DWORD nMany);
extern void   BootHelp (DWORD hlpExpr, DWORD noHelp, DWORD hStart);



extern WORD16 HandleCommand (char *pC, int nLen);   // Uv3.cpp




struct vExec  {
  WORD16       nTrans;          // Translator number
  char          *pCmd;          // Command line

  char         *pUnit;          // unit (unit name - src-name)
  char        *pfUnit;          // full path of unit

  char       *pExPath;          // Executable path (with \)
  char        *pSpawn;          // Spawn command and path
  char       *cmdFile;          // pathname of command file
  char       *objFile;          // pathname of object file
  char          *pEnv;          // Environment or NULL
  char        *cpptrn;          // name of ECPP trn via Tools.ini
  char        *pRvCpp;          // RVCT Preprocessor command line

  int          Status;          // Process status
  int          Errors;          // Errors
  int        Warnings;          // Warnings
  int        exitCode;
  int             PID;          // Process ID

  WORD16    isObj : 1;          // File is an OBJ file, can't create
  WORD16    isLib : 1;          // File is a  LIB file, can't create
  WORD16  PubOnly : 1;
  WORD16   GnuArm : 4;          // 1;
  WORD16   GnuHex : 1;          // ObjCopy: gen hex file
  WORD16   Rvds22 : 1;          // RVDS 2.2 Flag  /25.7.2005/
  WORD16  Rvct_PP : 1;          // RVCT generate Preprocessor File
  WORD16   SpcOvf : 1;          // address space overflow from linker detected
  WORD16    V6Bit : 1;          // ARM-V6 Bit
  WORD16    V6Asm : 1;          // Assemble .s using ArmClang
  WORD16   V6List : 1;          // generate V6 listing file
  WORD16 bLintAll : 1;          // 1:=Lint all files, 0:=Lint one file
  WORD16 bForLint : 1;          // 1:=prepare .h file for lint input
  WORD16 bStatCheck : 1;

  WORD16         mark;          // RegColor-Mark from DEPLST
  WORD16        nBank;          // BankNo

};


extern void ExecTrn (struct vExec *pX);      // run translator

#ifdef __cplusplus
  }                           // End of 'extern "C"'
#endif

extern char   szFileCT [512];                // current TargApp's pathname - Freiwald 15.02.06: moved outside extern "C" block for managed code





#define SN_NO_CHG     0
#define SN_REMOVE_NL  1
#define SN_ADD_NL     2
//...                 4

/*
 * Buffer-overflow protected version of _vsnprintf  /11.8.2014/
 */
#ifdef __cplusplus
  extern "C" int Do_SnPrintf (char *pOut, int nOutSize, DWORD nFlags, const char *fmt, ...);
#else
  extern     int Do_SnPrintf (char *pOut, int nOutSize, DWORD nFlags, const char *fmt, ...);
#endif




#endif




#if 0

/*
 * 9.10.2006, Extension to 'dbgext' struct -
 *            member 'Res8' replaced by 'void *pBom'.
 *            pBom will be initialized to '&ioc' in CMainFrame
 *            before entering flash-download and Target-Setup.
 * Change is in ComTyp.H
 */

struct bom              *pio;

/*
 * pio-bom-pointer now a member of 'struct dbgext' (former nRes8 member)
 */
void InitBomp (struct dbgblk *pD)  {
  pio = NULL;
  if (pD && pD->pDbgX)  {
    pio = (struct bom *) pD->pDbgX->pBom;   // Bom-pointer
  }
}


/*
 * Text-Out into Command/Build Window via 'pio'
 */
void txtout (char *fmt, ...) {
  va_list   marker;
  char      txtbuf [2048];

  va_start (marker, fmt);
  vsprintf (&txtbuf[0], fmt, marker);
  if (pio != NULL)  {               // make it bomp-proof
    SendMessage (pio->hwnd,
                 pio->hmsg,
                 MSG_UV2_TRNLINE,   // MSG_UV2_CMDLINE
                (LPARAM) &txtbuf[0]);
  }
}



/*
 * Progress-Bar functions via 'pio'
 */
static OIL  oil;                   // Progress-Bar data

static void InitProgress (char *pText)  {
  oil.Job   = PROGRESS_INITTXT;
  oil.pos   = 0;
  oil.low   = 0;
  oil.hig   = 100;
  oil.label = (signed char *) pText;
  if (pio != NULL)  {               // make it bomp-proof
    pio->Notify (UV_PROGRESS_SETUP, &oil);
  }
}
static void StopProgress (void)  {
  oil.Job = PROGRESS_SETPOS;
  oil.pos = 100;
  pCbFunc (AG_CB_PROGRESS, &oil);
  oil.Job = PROGRESS_KILL;
  if (pio != NULL)  {               // make it bomp-proof
    pio->Notify (UV_PROGRESS_SETUP, &oil);
  }
}
static void UpdatePos (int nPercent)  {
  oil.Job = PROGRESS_SETPOS;
  oil.pos = (nPercent);
  if (pio != NULL)  {               // make it bomp-proof
    pio->Notify (UV_PROGRESS_SETUP, &oil);
  }
}
static void SetText (char *pText) {
  oil.Job = PROGRESS_SETTEXT;
  oil.ctext = (SC8 *) pText;
  if (pio != NULL)  {               // make it bomp-proof
    pio->Notify (UV_PROGRESS_SETUP, &oil);
  }
}
#endif

