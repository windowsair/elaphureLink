/**************************************************************************//**
 *           AGSI.h: Advanced Generic Simulator Interface (AGSI)
 *                   definitions.
 * 
 * @version  V1.0.0
 * @date     $Date: 2015-04-28 15:09:20 +0200 (Tue, 28 Apr 2015) $
 *
 * @note
 * Copyright (C) 1999-2009 KEIL, 2009-2015 ARM Limited. All rights reserved.
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

#ifndef __AGSI_H__
#define __AGSI_H__


#include <windows.h>


#define AGSIEXPORT __declspec( dllexport )
#define AGSIAPI    WINAPIV


#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//
// AGSI: Advanced Generic Simulator Interface (Simulation Interface Part)
//

// ACCESS REASON FOR MEMORY ACCESS CALLBACK FUNCTIONS
#ifndef __AGSICB_REASON
  #define __AGSICB_REASON

  typedef enum {
    CB_VOID     = 0,      // no access
    CB_READ     = 0x02,   // adr = memory address
    CB_WRITE    = 0x04,   // adr = memory address
    CB_VT_READ  = 0x12,   // adr = VTR ** = AGSIVTR
    CB_VT_WRITE = 0x14,   // adr = VTR ** = AGSIVTR
    CB_IFETCH   = 0x08,   // adr = memory address
  } AGSICB_REASON;  
#endif

#ifndef PSIAPI
typedef enum CALLERID {				// BGN 09.09.2003 added for Rom Monitor
	Simulator, RomMonitor, EmulatorKSC, EmulatorHitex
}	CALLERID;

typedef enum RMWID {					// BGN 10.09.2003 added for Rom Monitor
	RMW_ANL,RMW_ORL,RMW_XRL,RMW_CPL,RMW_INC,RMW_DEC,RMW_MOV,RMW_CLR,RMW_SETB
} RMWID;
#endif // PSIAPI

typedef DWORD AGSIVTR;
typedef DWORD AGSIADDR;
typedef DWORD AGSITIMER;

typedef void (* AGSICALLBACK) (void);                         // Timer
typedef void (* AGSIEXECCALLBACK) (AGSIADDR adr);             // instruction execution callback
typedef void (* AGSICALLBACKA)(AGSIADDR adr, AGSICB_REASON r, DWORD size); // memory Access

typedef struct {
  HINSTANCE     m_hInstance;
  const char*   m_pszProjectPath;
  const char*   m_pszDevice;
  const char*   m_pszConfiguration;
  const char*   m_pszAppFile;
  HWND          m_hwnd;
  CALLERID      m_callerId;	// BGN 29.04.2004 for RomMonitor and Emulators
} AGSICONFIG;

typedef struct {         // access to internal structures
  DWORD           Size;  // Size of AGSIEXTINFO in bytes
  struct dbgblk * pdbg;
  struct bom    * bom;
} AGSIEXTINFO;

typedef enum {
  AGSIBYTE = 1,
  AGSIWORD = 2,
  AGSILONG = 4,
  AGSIBIT  = 5,
  AGSIVTRCHAR  = 10,
  AGSIVTRWORD  = 11,
  AGSIVTRLONG  = 12,
  AGSIVTRFLOAT = 13,
  AGSIVTRCHARARRAY  = 20,
  AGSIVTRWORDARRAY  = 21,
  AGSIVTRLONGARRAY  = 22,
  AGSIVTRFLOATARRAY = 23
} AGSITYPE;

typedef union {          // typedef for AgsiWriteVTREx and AgsiReadVTREx
  DWORD            l;    // for type AGSIVTRLONG
  WORD             w;    // for type AGSIVTRWORD
  BYTE             c;    // for type AGSIVTRCHAR
  float            f;    // for type AGSIVTRFLOAT
} AGSIVTRVAL;

typedef enum {
  AGSIREAD,
  AGSIWRITE,
  AGSIREADWRITE,
} AGSIACCESS;

// function code numbers for AgsiEntry function
typedef enum {
  AGSI_CHECK       = 0,
  AGSI_INIT        = 1,
  AGSI_TERMINATE   = 2,
  AGSI_RESET       = 3,
  AGSI_PREPLL      = 4,
  AGSI_POSTPLL     = 5,
  AGSI_PRERESET    = 6,    // before CPU RESET                   24.6.2003
  AGSI_CMDOUT      = 7,    // Command output of 'exec' commands  24.6.2003
  AGSI_ENTERSLEEP  = 8,    // HS 08.03.2004 currently only used for smartcards
  AGSI_EXITSLEEP   = 9,    // HS 08.03.2004 currently only used for smartcards
  AGSI_ONINTERRUPT = 10,   // HS 08.03.2004 currently only used for smartcards
  AGSI_ONRETI      = 11,   // HS 08.03.2004 currently only used for smartcards
  AGSI_MODE        = 12,   // HS 08.05.2008 used to distinguish simulation and target mode. See AGSI_MODE below.
  AGSI_STARTAPP    = 13,   // HS 13.05.2008 used to notify AGSI DLLs when application is started
  AGSI_STOPAPP     = 14,   // HS 13.05.2008 used to notify AGSI DLLs when application is stopped
  AGSI_TARGETREADY = 15,   // HS 23.05.2008 used to notify AGSI DLLs when target debugger has connected (before load application)
  AGSI_SETUP       = 16,   // HS 22.07.2008 added
  AGSI_LOADSTART   = 17,   // CL 06.04.2011 added: to notify AGSI DLLs when application load is started/finished
  AGSI_LOADEND     = 18,   
  AGSI_STOPREQ     = 19,   // HS 09.10.2014 used to notify AGSI DLLs when the stop button was pressed. Currently only implemented for NXP smartcards
} AGSIFUNC;


/*
 * 80x51 memory spaces
 */

#define   amXDATA  0x0001           // XDATA
#define   amYMEM   0x0040           // 16MB physical memory without MMU
#define   amBANK0  0x0080           // BANK0
#define   amBANK31 0x009F           // BANK31
#define   amDATA   0x00F0           // DATA
#define   amBIT    0x00F1           // BIT
#define   amEDATA  0x00F2           // EDATA (i251)
#define   amIDATA  0x00F3           // IDATA
#define   amECODE  0x00F4           // 251 ecode
#define   amHDATA  0x00F5           // 251 hdata
#define   amHCONS  0x00F6           // 251 hconst
#define   amCONST  0x00F7           // 251 const
#define   amPDATA  0x00FE           // PDATA (c51 macht das bei generic)
#define   amCODE   0x00FF           // CODE 
#define	  amPHYS   0x0100           // Physical SLE66+ Memory

// Flags for "special" from "inttab"
#define MODE4ENABLE    0x10   // Mode bit used as second Enable bit
#define INVMODE4ENABLE 0x20   // Inverted Mode bit used as second Enable bit


typedef struct {
#if defined (__ARMCM3__)    // for ARM Cortex-M3
   WORD            idx;     // Interrupt Index
   char          *mess;     // Interrupt Message
   char          *name;     // Interrupt Name
   DWORD          rsfr;     // Interrupt Request Register
   DWORD         rmask;     // Interrupt Request Bit Mask
#elif defined (__ARMSSC__)  // for Samsung SC ARM
   WORD           num;      // Interrupt Number
   char          *mess;     // Interrupt Message
   char          *name;     // Interrupt Name
   DWORD          bmsk;     // Interrupt Registers Bit Mask
   AGSIADDR       rsfr;     // interrupt request sfr 
   const char   *rname;     // name of interrupt request bit
   AGSIADDR       esfr;     // interrupt enable sfr
   const char   *ename;     // name of interrupt enable bit
#elif defined (__ARMAT__)   // for Atmel ARM
   WORD            idx;     // Interrupt Index
   char          *mess;     // Interrupt Message
   char          *name;     // Interrupt Name
   DWORD          rsfr;     // Interrupt Request Register
   DWORD          esfr;     // Interrupt Enable Register
   WORD         extint;     // External Interrupt
#elif defined (__ARMP__)    // for Philips ARM
   WORD           chan;     // Interrupt Channel
   char          *mess;     // Interrupt Message
   char          *name;     // Interrupt Name
   DWORD          rsfr;     // Interrupt Request Register
   DWORD         rmask;     // Interrupt Request Bit Mask
   DWORD          esfr;     // Interrupt Enable Register
   DWORD         emask;     // Interrupt Enable Bit Mask
#elif defined (__ARMAD__)   // for Analog Devices ARM
   WORD            idx;     // Interrupt Index
   char          *mess;     // Interrupt Message
   char          *name;     // Interrupt Name
   DWORD          rsfr;     // Interrupt Request Register
   DWORD         rmask;     // Interrupt Request Bit Mask
#elif defined (__ARMSS9__)  // for Samsung ARM9 (S3C24xx)
   BYTE            idx;     // Interrupt Index
   BYTE           sidx;     // Interrupt Sub Index
   char          *mess;     // Interrupt Message
   char          *name;     // Interrupt Name
   DWORD          rsfr;     // Interrupt Request Register
   DWORD          msfr;     // Interrupt Mask Register
   DWORD         bmask;     // Interrupt Bit Mask
#elif defined (__C166__)
   AGSIADDR        vec;
   char          *mess;     // interrupt name
   AGSIADDR        sfr;     // interrupt control sfr 
#elif defined (__SLE66__)   // for Infineon SLE66
   AGSIADDR        vec;
   char          *mess;     // Interrupt name
   const char   *rname;     // name of interrupt request bit
   const char   *ename;     // name of interrupt enable bit
   const char   *pname;     // name of interrupt priority bit
   DWORD           num;     // Interrupt Number
   DWORD           pwl;     // priority within level
#else                       // other targets (x51 based)
   AGSIADDR        vec;
   char          *mess;     // interrupt name
   AGSIADDR       msfr;     // interrupt mode sfr 
   WORD          mmask;     // interrupt mode bit mask
   const char   *mname;     // name of interrupt mode bit
   AGSIADDR       rsfr;     // interrupt request sfr 
   WORD          rmask;     // interrupt request bit mask
   const char   *rname;     // name of interrupt request bit
   AGSIADDR       esfr;     // interrupt enable sfr
   WORD          emask;     // interrupt enable bit mask
   const char   *ename;     // name of interrupt enable bit
   AGSIADDR      p0sfr;     // interrupt priority 0 sfr
   WORD         p0mask;     // interrupt priority 0 bit mask
   const char   *pname;     // name of interrupt priority bit
   AGSIADDR      p1sfr;     // interrupt priority 1 sfr
   WORD         p1mask;     // interrupt priority 1 bit mask
   WORD            pwl;     // priority within level
   WORD     auto_reset;     // reset interrupt request flag on interrupt entry
   WORD        special;     // added to extend interrupt capability (see flags above)
#endif
} AGSIINTERRUPT;

typedef struct  {
#if defined (__ARMCM3__)    // for ARM Cortex-M3
  DWORD  Rn[16];            // R0..R12,R13(SP),R14(LR),R15(PC)
  DWORD    xPSR;            // xPSR (Program Status Register)
  DWORD     MSP;            // MSP (Main SP)
  DWORD     PSP;            // PSP (Process SP)
  DWORD     DSP;            // DSP (Deep SP)
  union  {                  // System Registers
    DWORD     P;
    struct {
      BYTE   PRIMASK;       // PRIMASK
      BYTE   BASEPRI;       // BASEPRI
      BYTE FAULTMASK;       // FAULTMASK
      BYTE   CONTROL;       // CONTROL
    } R;
  } SYS;
  INT64  nCycles;           // Cycle Counter
// for      Samsung SC ARM             Atmel ARM         Philips ARM       Analog Devices ARM     Samsung ARM9 (S3C24xx)
#elif defined (__ARMSSC__) || defined (__ARMAT__) || defined (__ARMP__) || defined (__ARMAD__) || defined (__ARMSS9__)
  DWORD  cur[16];           // Current Mode:   R0..R15(PC)
  DWORD  cpsr;              // CPSR
  DWORD  spsr;              // Current SPSR
  DWORD  usr[7];            // User & System:  R8..R14
  DWORD  fiq[8];            // Fast Interrupt: R8..R14, SPSR
  DWORD  irq[3];            // Interrupt:      R13,R14, SPSR
  DWORD  svc[3];            // Supervisor:     R13,R14, SPSR
  DWORD  abt[3];            // Abort:          R13,R14, SPSR
  DWORD  und[3];            // Undefined:      R13,R14, SPSR
  INT64  nCycles;           // cycle counter
#elif defined (__C166__)    // Infineon C16x / ST10
  union  {
    WORD16  wregs [16];     // R0  ... R15
    BYTE    bregs [16];     // RL0 ... RH7
  } r;
  DWORD        Ndpp[4];     // full linear base address values
  DWORD            nPC;     // full address
  WORD16          cPsw;     // PSW
  WORD16           cSP;     // SP
  WORD16          cMDL;     // MDL
  WORD16          cMDH;     // MDH
  DWORD            cCP;     // CP
  INT64        nCycles;     // cycle counter

  INT64           macc;     // 40-Bit value
  WORD16           mah;     // MAH MAC-Unit Accumulator High
  WORD16           mal;     // MAL MAC-Unit Accumulator Low
  WORD16           mas;     // MAS limited MAH/signed
  WORD16           msw;     // MSW MAC-Unit Status Word
  WORD16           mcw;     // MCW MAC-Unit Control Word
  WORD16           mrw;     // MRW MAC-Unit Repeat Word

  WORD16          idx0;
  WORD16          idx1;
  WORD16           qx0;
  WORD16           qx1;
  WORD16           qr0;
  WORD16           qr1;
  WORD16        cSPSEG;
#else                       // other targets (x51 based)
  BYTE         Rn [16];     // R0 ... R7
  DWORD            nPC;     // full address !
  BYTE              sp;     // SP
  BYTE             psw;     // PSW-sfr
  BYTE               b;     // B-sfr
  BYTE             acc;     // ACC-sfr
  BYTE             dpl;     // DPL-sfr
  BYTE             dph;     // DPH-sfr
  BYTE        ports[8];
  INT64        nCycles;     // cycle counter
#endif
} CPUREGISTER;


/*
 * Symbol search masks (may be combined using |) :
 */

typedef enum {
  AGSI_SYM_VAR = 0x0001,         // search for non-bit Variables
  AGSI_SYM_CON = 0x0002,         // search for named Constants
  AGSI_SYM_BIT = 0x0004,         // search for Bit in Memory
  AGSI_SYM_LOC = 0x0008,         // search for Function/Label
  AGSI_SYM_SFR = 0x0200          // search for SFR name
} AGSISYMMASK;

/*
 * Type of found symbol:
 */

typedef enum {
  AGSI_TP_VOID   = 0,
  AGSI_TP_BIT    = 1,
  AGSI_TP_CHAR   = 2,
  AGSI_TP_UCHAR  = 3,
  AGSI_TP_INT    = 4,
  AGSI_TP_UINT   = 5,
  AGSI_TP_SHORT  = 6,
  AGSI_TP_USHORT = 7,
  AGSI_TP_LONG   = 8,
  AGSI_TP_ULONG  = 9,
  AGSI_TP_FLOAT  = 10,
  AGSI_TP_DOUBLE = 11,
  AGSI_TP_PTR    = 12,
  AGSI_TP_UNION  = 13,
  AGSI_TP_STRUCT = 14,
  AGSI_TP_FUNC   = 15,
  AGSI_TP_STRING = 16,
  AGSI_TP_ENUM   = 17,
  AGSI_TP_FIELD  = 18,

//---HP. added  /28.7.2005/
  AGSI_TP_INT64  = 19,
  AGSI_TP_UINT64 = 20,
  AGSI_TP_ARRAY  = 30
//---

} AGSISYMTYPE;

/*
 * Type of lock
 */

typedef enum {
	AGSI_LK_INFO   = 0,
	AGSI_LK_LOCK   = 1,
	AGSI_LK_UNLOCK = -1
} AGSILOCK;

typedef struct  {                  // Search for Sym by Name or Value.
  AGSISYMMASK nMask;               // search mask (AG_SYM_LOC | ...)
  char szName [256];               // search/found name (zero-terminated
  UINT64        val;               // search/found Adr/Value
  AGSISYMTYPE  type;               // type of found symbol (AGSI_TP_???)
  DWORD          Ok;               // 1:=Ok, else find failed.
} AGSISYMDSC;

typedef enum {
  AGSI_GETACTSFRSEL = 1,
  AGSI_ACEREAD      = 2,
  AGSI_RFREAD       = 3,
  AGSI_RFWRITE      = 4,
  AGSI_BONVMREAD    = 5,
  AGSI_BONVMPROG    = 6
} AGSISPFUNC;

typedef enum {
  AGSI_SIMULATION      = 0,
  AGSI_TARGETDEBUGGING = 1
} AGSI_MODE_TYPE;                  // used as parameter for AGSI_MODE

/*
 * Dynamic Menu construction and maintanance
 */

//#pragma pack(1)

#define AGSIDLGD struct AgsiDlgDat
struct AgsiDlgDat {                      // every dialog has it's own structure
  DWORD             iOpen;               // auto reopen dialog (pos := 'rc')
  HWND                 hw;               // Hwnd of Dialog
  BOOL (CALLBACK *wp) (HWND hw, UINT msg, WPARAM wp, LPARAM lp);
  RECT                 rc;               // Position rectangle
  void   (*Update) (void);               // Update dialog content
  void (*Kill) (AGSIDLGD *pM);           // Kill dialog
  void                *vp;               // reserved for C++ Dialogs (Dlg *this)
};

#define AGSIMENU struct AgsiDynaM
struct AgsiDynaM  {                      // Menu item data structure
  int              nDelim;               // Menu template delimiter
  char            *szText;               // Menu item text
  void    (*fp) (AGSIMENU *pM);          // create/bringDlgtoTop function
  DWORD               nID;               // uv3 assigned ID_xxxx
  DWORD            nDlgId;               // Dialog ID
  AGSIDLGD          *pDlg;               // link to dialog attributes
};
//#pragma pack()

// extensions for Philips MX SmartCards
#define AGSIHOOK_PC        0x01
#define AGSIHOOK_PREEXEC   0x02
#define AGSIHOOK_POSTEXEC  0x04
#define AGSIHOOK_DELETE    0x10

typedef struct tagAGSIHOOKCONTEXT
{
	BYTE		bEvent;
	AGSIADDR	Pc;
	AGSIADDR	Ypc;
	UINT64		Advance;
} AGSIHOOKCONTEXT;

typedef void ( *AGSIHOOKFUNC )(AGSIHOOKCONTEXT *);

// extensions for 8051 simulator
#define AGSICALLBACK_PC        0x01
#define AGSICALLBACK_PREEXEC   0x02
#define AGSICALLBACK_POSTEXEC  0x04
#define AGSICALLBACK_DELETE    0x10

typedef struct tagAGSICALLBACKCONTEXT
{
	BYTE		Event;
	AGSIADDR	PC;
	DWORD   	Opcodes;     // next 4 8051 opcodes
	UINT64		Advance;
} AGSICALLBACKCONTEXT;

typedef void ( *AGSIEXECCALLBACKEX )(AGSICALLBACKCONTEXT *);


typedef struct sAGSIPERIPARA {
  DWORD  size;               // sizeof this structure
  DWORD  FamexRamSize;       // Famex Ramsize specified with -f parameter
  DWORD  UartEnablePara;
  DWORD  EESizePara;
  DWORD  EEPTimePara;
  DWORD  EEETimePara;
  DWORD  RNGTimePara;
  DWORD  RNGSeedPara;
  DWORD  RNGPeriodPara;
  DWORD  EEPageSizePara;
  DWORD  CXRAMSizePara;   // do not change the size of this structure to keep this DLL compatible
} AGSIPERIPARA;
 

/*
 * nDelim:  1 := normal Menu entry
 *          2 := Popup-Entry (nested submenu)
 *         -2 := end of Popup-Group-List
 *         -1 := total end of Menu-List
 *  text:   the name for the menu/popup-menu entry
 *    fp:   Function to be activated on menu-selection
 */


// Function that must be exported by the loaded DLL
AGSIEXPORT DWORD        AGSIAPI AgsiEntry (DWORD nCode, void *vp);

// Callback functions that are ONLY allowed to call during the initialisation process
AGSIEXPORT BOOL         AGSIAPI AgsiDefineSFR(const char* pszSfrName, AGSIADDR dwAddress, AGSITYPE eType, BYTE bBitPos);
AGSIEXPORT BOOL         AGSIAPI AgsiDefineSFREx(const char* pszSfrName, AGSIADDR dwAddress, AGSITYPE eType, BYTE bBitPos, DWORD dwAccessMask);
AGSIEXPORT AGSIVTR      AGSIAPI AgsiDefineVTR(const char* pszVtrName, AGSITYPE eType, DWORD dwValue);
AGSIEXPORT BOOL         AGSIAPI AgsiDeclareInterrupt(AGSIINTERRUPT *pInterrupt);
AGSIEXPORT BOOL         AGSIAPI AgsiSetWatchOnSFR(AGSIADDR SFRAddress, AGSICALLBACKA pfnReadWrite, AGSIACCESS eAccess);
AGSIEXPORT BOOL         AGSIAPI AgsiSetWatchOnVTR(AGSIVTR hVTR, AGSICALLBACKA pfnReadWrite, AGSIACCESS eAccess);
AGSIEXPORT BOOL         AGSIAPI AgsiSetWatchOnMemory(AGSIADDR StartAddress, AGSIADDR EndAddress, AGSICALLBACKA pfnReadWrite, AGSIACCESS eAccess);
AGSIEXPORT AGSITIMER    AGSIAPI AgsiCreateTimer(AGSICALLBACK pfnTimer);
AGSIEXPORT BOOL         AGSIAPI AgsiDefineMenuItem(AGSIMENU *pDym);
AGSIEXPORT void         AGSIAPI AgsiExecuteCommand(const char* pszCommand);

// Callback functions that are NOT allowed to call during the initialisation process
AGSIEXPORT BOOL         AGSIAPI AgsiWriteSFR(AGSIADDR SFRAddress, DWORD dwValue, DWORD dwMask);
AGSIEXPORT BOOL         AGSIAPI AgsiReadSFR (AGSIADDR SFRAddress, DWORD* pdwCurrentValue, DWORD* pdwPreviousValue, DWORD dwMask);
AGSIEXPORT BOOL         AGSIAPI AgsiWriteVTR(AGSIVTR hVTR, DWORD dwValue);
AGSIEXPORT BOOL         AGSIAPI AgsiReadVTR (AGSIVTR hVTR, DWORD* pdwCurrentValue);
AGSIEXPORT BOOL         AGSIAPI AgsiWriteVTREx(AGSIVTR hVTR, AGSIVTRVAL *pVTRValue);
AGSIEXPORT BOOL         AGSIAPI AgsiReadVTREx (AGSIVTR hVTR, AGSIVTRVAL *pVTRValue);
AGSIEXPORT BOOL         AGSIAPI AgsiSetSFRReadValue(DWORD dwValue);
AGSIEXPORT BOOL         AGSIAPI AgsiReadMemory (AGSIADDR Address, DWORD dwCount, BYTE* pbValue);
AGSIEXPORT BOOL         AGSIAPI AgsiWriteMemory(AGSIADDR Address, DWORD dwCount, BYTE* pbValue);
AGSIEXPORT AGSIADDR     AGSIAPI AgsiGetLastMemoryAddress(void);
AGSIEXPORT BOOL         AGSIAPI AgsiIsSimulatorAccess(void);
AGSIEXPORT BOOL         AGSIAPI AgsiSetTimer(AGSITIMER hTimer, DWORD dwClock);
AGSIEXPORT UINT64       AGSIAPI AgsiGetStates(void);
AGSIEXPORT AGSIADDR     AGSIAPI AgsiGetProgramCounter(void);
AGSIEXPORT BOOL         AGSIAPI AgsiSetProgramCounter(AGSIADDR pc, AGSIADDR Ypc);
AGSIEXPORT DWORD        AGSIAPI AgsiIsInInterrupt(void);
AGSIEXPORT BOOL         AGSIAPI AgsiIsSleeping(void);
AGSIEXPORT void         AGSIAPI AgsiStopSimulator(void);
AGSIEXPORT void         AGSIAPI AgsiTriggerReset(void);
AGSIEXPORT void         AGSIAPI AgsiTriggerResetEx(int ResetLevel);
AGSIEXPORT void         AGSIAPI AgsiUpdateWindows(void);
AGSIEXPORT void         AGSIAPI AgsiHandleFocus (HWND hwndDialog);
AGSIEXPORT DWORD        AGSIAPI AgsiGetExternalClockRate(void);
AGSIEXPORT BOOL         AGSIAPI AgsiSetExternalClockRate(DWORD clock);
AGSIEXPORT DWORD        AGSIAPI AgsiGetInternalClockRate(void);
AGSIEXPORT double       AGSIAPI AgsiGetClockFactor(void);
AGSIEXPORT void         AGSIAPI AgsiMessage(const char* pszFormat, ...);
AGSIEXPORT void         AGSIAPI AgsiMessageDbgV(const char* pszFormat, ...);
AGSIEXPORT const char * AGSIAPI AgsiGetTargetKey(const char* pszKey);
AGSIEXPORT BOOL         AGSIAPI AgsiSetTargetKey(const char* pszKey, const char *pszString);
AGSIEXPORT DWORD        AGSIAPI AgsiGetSymbolByName (AGSISYMDSC *vp);
AGSIEXPORT DWORD        AGSIAPI AgsiGetSymbolByValue(AGSISYMDSC *vp);
AGSIEXPORT BOOL         AGSIAPI	AgsiBurstWriteSFR(AGSIADDR SFRAddress, DWORD dwCount, BYTE *pbValue);
AGSIEXPORT BOOL         AGSIAPI	AgsiBurstReadSFR(AGSIADDR SFRAddress, DWORD dwCount, BYTE *pbValue);
AGSIEXPORT BOOL         AGSIAPI	AgsiReadModWriteSFR(AGSIADDR SFRAddress, RMWID rmwType, BYTE bValue);
AGSIEXPORT DWORD        AGSIAPI	AgsiSpecialFunction(CALLERID hwType, BYTE bFuncNum, int nOutParam, void *pOutParam, int nInParam, void *pInParam);
AGSIEXPORT int          AGSIAPI AgsiLock(AGSILOCK lock);
AGSIEXPORT void         AGSIAPI AgsiContinue(void);
AGSIEXPORT AGSIEXTINFO* AGSIAPI AgsiGetExtInfo(void);
AGSIEXPORT BOOL         AGSIAPI AgsiRegisterHook(BYTE bEvent, BYTE bOpcode, AGSIADDR pc, AGSIHOOKFUNC hook);
AGSIEXPORT BOOL         AGSIAPI AgsiRegisterExecCallBack  (AGSIEXECCALLBACK *fp);
AGSIEXPORT BOOL         AGSIAPI AgsiRegisterExecCallBackEx(BYTE Event, BYTE Opcode, AGSIADDR pc, AGSIEXECCALLBACKEX callbackfunc);
AGSIEXPORT BOOL         AGSIAPI AgsiPeriPara(AGSIPERIPARA *pPeriPara);
AGSIEXPORT double       AGSIAPI AgsiGetTime(void);
AGSIEXPORT BOOL         AGSIAPI AgsiGetCpuRegister(CPUREGISTER* pReg);
AGSIEXPORT BOOL         AGSIAPI AgsiSetCpuRegister(CPUREGISTER* pReg);
AGSIEXPORT void         AGSIAPI AgsiEnterSleep(void);
AGSIEXPORT void         AGSIAPI AgsiExitSleep(void);
AGSIEXPORT BOOL         AGSIAPI AgsiAddStates(int states);
AGSIEXPORT void         AGSIAPI AgsiExitSuspend(void);
AGSIEXPORT void         AGSIAPI AgsiEnterSuspend(void);
AGSIEXPORT void         AGSIAPI AgsiSetPowerSaveMode(DWORD IdleMode, DWORD PowerDownMode);
AGSIEXPORT DWORD        AGSIAPI AgsiRegisterCVec(DWORD nVec, DWORD (*pFunc)(DWORD nVec));
AGSIEXPORT BOOL         AGSIAPI AgsiSetInstrCountCallback(AGSICALLBACK pfnTimer, DWORD nInstr);
AGSIEXPORT void         AGSIAPI AgsiLoadInstructionTimes(DWORD pExecTimes[256]);
AGSIEXPORT void         AGSIAPI AgsiKeepAlive(void);

#ifdef __cplusplus
}
#endif


#endif // __AGSI_H__


