/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.50
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
 *
 * @brief     Main UV communication file for the AGDI Driver, UV calls come in here
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
#include "resource.h"

#define _IN_TARG_ // define if used within target
#include <stdio.h>
#include <math.h>
#include "..\AGDI.h"
#include "..\BOM.h"
#include "..\ComTyp.h"
#include "Collect.h"
#include "Flash.h"
#include "Debug.h"
#include "Trace.h"
#include "CSTF.h"
#include "CTI.h"
#include "ETB.h"
#include "TraceRec.h"
#include "TraceExc.h"
#include "JTAG.h"
#include "SWD.h"
#include "SWV.h"
#include "TraceWinConnect.h"
#include "BreakResources.h"
#include "..\TracePointDefs.h"

#if DBGCM_RECOVERY
#include "DebugAccess.h"
#endif // DBGCM_RECOVERY

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
#include "DSMonitor.h"
#endif // DBGCM_DS_MONITOR


RgARMCM  RegARM; // ARM Cortex-M Registers
RgARMFPU RegFPU; // FPU Cortex-M Registers

U64         RegUpToDate;        // Specifies whether register structure is up to date
BYTE        bootflag;           // Specifies whether first boot loader is in progress or not
BYTE        PlayDead;           // Disables the driver after the communication breaks down.
const char *PlayDeadMsg;        // Message to send if processing PlayDead.
BYTE        PlayDeadShut;       // Shutting down because of PlayDead
BYTE        PlayDeadRegular;    // PlayDead set because of a regular shutdown
BYTE        PlayDeadDelayedMsg; // Error message shall be shown during secon call to ExecPlayDead
BYTE        FirstReset;         // Is set during initialization and reset during the first reset

DWORD xxCPU; // CPU Type (ARM Cortex-M0/M1/M3/M4/M23/M33 ARM SC300)
BOOL  xFPU;  // FPU Flag
DWORD nCPU;  // Number of selected CPU within the JTAG chain

DYMENU *pDyMenu; // Dynamic menu pointer

struct bom *pio;

extern HANDLE     PlayDeadShut_mtx;  // PlayDeadShut mutex
static struct lPa PlayDeadNotifyMsg; // Notify Message to post as a trigger for PlayDead through GUI thread

/*
 * AGDI-Data
 */

SUPP      supp;    // supported features
UC8       iRun;    // target is currently executing
UC8       StopRun; // Flag: stop execution
UC8       GoMode;
UC8       ReInit;
AG_BP   **pBhead;
UL32     *pCURPC;
LOADPARMS lParms; // LOAD-Parameters

static UL32 abreaks;   // Number of Code-Address BPs
static UL32 cbreaks;   // Number of Conditional BPs
static UL32 hbreaks;   // Number of HW Breakpoints
static UL32 wbreaks;   // Number of Watchpoint BPs
static UL32 vbreaks;   // Number of Watchpoint BPs with value match
static UL32 lbreaks;   // Number of Watchpoint BPs with link capabilities (v8-M)
static UL32 limbreaks; // Number of Watchpoint BPs with address limit capabilities (v8-M)

static AG_BP    *pBStart;  // pointer to breakpoint chain with temporary break
static AG_BP     TBp;      // Temporary BreakPoint
static BYTE      UseSWBP;  // Use SW Breakpoints
/*static*/ BYTE  CntBP;    // HW Breakpoint Count
/*static*/ BYTE  CntWP;    // HW Watchpoint Count
/*static*/ DWORD UseWP[4]; // HW Watchpoint Multi-usage Count
static BYTE      bNoCache; // [TdB 13.03.2012] PH: 3.3.2012, 1:=cache temporarily turned off

static DWORD Opcode;    // 1 - Opcode Access, 0 - Data Access
static DWORD MemErr;    // Memory Error (Read/Write or Verify)
static BOOL  AGDIError; // Indicates an AGDI error occurred during this AGDI function
                        // ... for delayed error message sending (outside the CheckCom() mutex)
int AGDIErrorCode;      // ADGI error code set is AGDIError is true

static int NumRecs; // number of trace records

/*static*/ VTR *pCoreClk; // Core Clock VTR


UL32 Uv2Msg;  // Uv2-registered Message token
pCBF pCbFunc; // call-back function of SARM

// if an extension modeless dialog is activated, then '*pHwnd' *MUST*
// receive the HWND (m_hWnd in MFC) of the dialog.  If the dialog
// looses focus or is closed, then '*pHwnd' *MUST* be set to NULL.
// This is to ensure the proper message chain within Uv2.

HWND   *pHwnd;
HWND    hMfrm; // parent handle (CMainFrame)
HMODULE hInst; // this DLL's instance handle

UC8         iBpCmd;             // 13.01.2015: currently executing breakpoint command
static UL32 GoUntilAdr;         // 0xFFFFFFFF - until break/stop, set in GoUntil()
UC8         DbgExitStarted = 0; // Indicator that we are exiting debug
static BOOL MultipleGoStep;     // Multiple Go/Step calls for one debugger operation
static BOOL BkpInstructionSkip; // Skip SW Breakpoint instruction

extern DWORD nCPU; // Selected CPU within the JTAG chain

static UC8 ExtendedStep; // WaitUntil() was an extended single instruction step

// v8-M Extensions
RgARMV8MSE RegV8MSE;  // ARM v8-M Security and Authentication Registers
BOOL       xMainline; // v8-M CPU is Mainline, Baseline otherwise
BOOL       xSecure;   // v8-M Security Extensions Flag

VTR *pTraceClk;        // Trace Clock VTR /* 02.04.2019 */
UC8  ReInitInProgress; // Reinit in progress (to avoid race conditions due to new trace setup path from UV)

static const char ErrTitle[] = "Debugger - Cortex-M Error";

static const char JTAGErr[] =
    "Could not find an Cortex-M device in the JTAG chain!\n"
    "Please check the JTAG cable and the connected devices.";
static const char StopErr[] =
    "Could not stop Cortex-M device!\n"
    "Please check the JTAG cable.";
static const char VerErr[] =
    "Memory Mismatch!\n"
    "Address: 0x%08X\n"
    "  Value = 0x%02X\n"
    "  Expected = 0x%02X\n\n";
static const char FatalAccErr[] =
    "Cannot access target.\n"
    "Shutting down debug session.";
static const char FatalRstErr[] =
    "Cannot reset target.\n"
    "Shutting down debug session.";
static const char FatalReInitErr[] =
    "Cannot reconfigure target.\n"
    "Shutting down debug session.";


static const char *BPErr[] = {
    /*0*/
    "This target device does not support all the defined breakpoints!\n"
    "Please reduce the number of breakpoints and start again.\n",
    /*1*/
    "This target device does not support conditional breakpoints!\n"
    "Please remove all conditional breakpoints and start again.",
    /*2*/
    "This target device does not support all the defined watchpoints!\n"
    "Please reduce the number of watchpoints and start again.",
    /*3*/
    "This target device supports only the following watchpoints:\n"
    "     size = 2^n; n = 0..15\n"
    "     start must be 2^n aligned\n"
    "     with optional value match\n"
    "Please change the watchpoint definitions and start again.",
    /*4*/
    "Operation not possible while the target device is executing.\n"
    "Please stop the target to perform this operation.\n",
    /*5*/
    "It was not possible to disable all breakpoints.\n"
    "Please stop the target to perform this operation.\n",
    /*6*/
    "It was not possible to kill all breakpoints.\n"
    "Please stop the target to perform this operation.\n",
};

static const char SwBpRestoreWarn[] = "\nWarning: BKPT instruction at 0x%08X externally modified! May have missed requested breakpoint.\n";

/* Specialized Error Messages for Non-Secure Debug */
static const char NonSecureStopErr[]   = "Secure Cortex-M device stop can be delayed!";
static const char NonSecureAccErr[]    = "Cannot access secure target.";
static const char NonSecureRstErr[]    = "Cannot reset secure target.";
static const char NonSecureReInitErr[] = "Cannot reconfigure secure target.";

static const char InfoTitle[] = "Debugger - Cortex-M";

static const char *TrRunWarn =
    "'Suspend' tracepoint set without 'Run' tracepoint. ETM instruction trace capture will not start.\n"
    "Do you want to continue?";


//*************************************************************************
// Memory Management Functions
//*************************************************************************


// Cache Memory
struct EMM (*slots[256])[256] = {
    /*        0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
    /* 00 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 10 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 20 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 30 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 40 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 50 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 60 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 70 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 80 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 90 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* A0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* B0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* C0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* D0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* E0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* F0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


/*
 * Map Segment
 */

void MMapSeg(DWORD nSeg, DWORD nSof, DWORD nLen, DWORD nAtr)
{
    MAMAP adr;
    struct EMM(*s)[256];
    struct EMM acc;
    DWORD     *pA;
    DWORD      n;

    if (nLen == 0)
        return;
    adr.a32 = (nSeg << 16) + nSof;
    s       = slots[adr.ub[3]];
    if (s == NULL) { // if slot is not yet allocated
        s                = (struct EMM(*)[256])calloc(sizeof(struct EMM) * 256, 1);
        slots[adr.ub[3]] = s;
        if (s == NULL)
            return;
    }
    acc = (*s)[adr.ub[2]]; // slot available?
    if (acc.mem == NULL) {
        acc.mem         = (DWORD *)calloc(_MSGM, 1); // map 64K + 4
        acc.atr         = (DWORD *)calloc(_MSGM, 1); // map 64K + 4
        (*s)[adr.ub[2]] = acc;
        if ((acc.mem == NULL) || (acc.atr == NULL))
            return;
    }

    nLen = (nLen + 3) >> 2;
    pA   = &acc.atr[adr.w16 >> 2];
    for (n = 0; n < nLen; ++n, ++pA) { // initialize attribs
                                       //  *pA = nAtr;                                // RR 16.05.2008: Merge attributes (doesn't destroy BP)
        if (nAtr == 0) {
            *pA = nAtr;
        } else {
            *pA |= nAtr;
        }
    }
}


/*
 * Map Memory
 */

DWORD MMapMem(DWORD nAtr, DWORD nAdr, DWORD nLen)
{
    DWORD nSof, nSeg;

    nSeg = nAdr >> 16; // Start-Segment
    nSof = nAdr & 0xFFFF;
    while ((nSof + nLen) > 0x10000) {
        MMapSeg(nSeg, nSof, 0x10000 - nSof, nAtr);
        ++nSeg;
        nLen -= (0x10000 - nSof);
        nSof = 0;
    }
    if (nLen != 0) {
        MMapSeg(nSeg, nSof, nLen, nAtr); // Segments are never > than 64K !
    }
    return (0);
}


/*
 * Get Pointer to Attributes for addresss 'nAdr'
 */

DWORD *MGetAttr(DWORD nAdr)
{
    MAMAP adr;
    struct EMM(*s)[256];
    struct EMM acc;
    DWORD     *pA;

    pA      = NULL;
    adr.a32 = nAdr;
    s       = slots[adr.ub[3]]; // a24-a31
    if (s) {
        acc = (*s)[adr.ub[2]];
        if (acc.atr) {
            pA = &acc.atr[adr.w16 >> 2]; // attributes for 'nAdr'
        }
    }
    return (pA);
}

WORD *MGetAttr16(DWORD nAdr)
{
    MAMAP adr;
    struct EMM(*s)[256];
    struct EMM acc;
    WORD      *pA;

    pA      = NULL;
    adr.a32 = nAdr;
    s       = slots[adr.ub[3]]; // a24-a31
    if (s) {
        acc = (*s)[adr.ub[2]];
        if (acc.atr) {
            pA = (WORD *)acc.atr + (adr.w16 >> 1);
        }
    }
    return (pA);
}


//--- Memory cache handling.
//--- Simulate the (non-existing) target memory
// Memory caching is used to increase debugger performance.
// When uVision2 requests data, it is taken out from cache memory buffer
// instead of reading it from the target hardware. This performance
// is increased slightly if fast interface is used (USB, Ethernet,...)


/*
 * Free all allocated memory by this driver before driver shutdown.
 */

/*static*/ void FreeCache(void)
{
    struct EMM(*s)[256];
    struct EMM acc;
    int        i, j;

    for (i = 0; i < 256; i++) {
        s = slots[i]; // a24-a31
        if (s) {
            for (j = 0; j < 256; j++) {
                acc = (*s)[j]; // slot available?
                if (acc.mem) {
                    free(acc.mem);
                    acc.mem = NULL; // RK 4.2.2004 avoid BoundChecker messages
                }
                if (acc.atr) {
                    free(acc.atr);
                    acc.atr = NULL; // RK 4.2.2004 avoid BoundChecker messages
                }
            }
            free(s);
            slots[i] = NULL;
        }
    }
}


/*
 * Invalidate Cache Memory: clear ATRX_UPDT
 */

static void InvalidateCache(void)
{
    struct EMM(*s)[256];
    struct EMM acc;
    int        i, j, k;

    for (i = 0; i < 256; i++) {
        s = slots[i]; // a24-a31
        if (s) {
            for (j = 0; j < 256; j++) {
                acc = (*s)[j]; // slot available?
                if (acc.atr) {
                    for (k = 0; k < 0x4000; k++) {
                        acc.atr[k] &= ~ATRX_UPTD;
                    }
                }
            }
        }
    }
}


/*
 * Write to Cache Memory
 */

static void WriteCache(DWORD nAdr, BYTE *pB, DWORD nMany)
{
    MAMAP adr;
    struct EMM(*s)[256];
    struct EMM acc;

    if (iRun && !Opcode)
        return;

    for (adr.a32 = nAdr; nMany != 0; nMany--, adr.a32++) {
        s = slots[adr.ub[3]];
        if (s == NULL) { // if slot is not yet allocated
            s                = (struct EMM(*)[256])calloc(sizeof(struct EMM) * 256, 1);
            slots[adr.ub[3]] = s;
            if (s == NULL)
                return;
        }
        acc = (*s)[adr.ub[2]]; // slot available?
        if (acc.mem == NULL) {
            acc.mem         = (DWORD *)calloc(_MSGM, 1); // map 64K + 4
            acc.atr         = (DWORD *)calloc(_MSGM, 1); // map 64K + 4
            (*s)[adr.ub[2]] = acc;
            if ((acc.mem == NULL) || (acc.atr == NULL))
                return;
            //acc.atr[adr.w16 >> 2] = ATRX_EXEC | ATRX_READ | ATRX_WRITE;
        }
        if ((acc.atr[adr.w16 >> 2] & (ATRX_EXEC | ATRX_READ | ATRX_WRITE)) == 0) {
            acc.atr[adr.w16 >> 2] |= ATRX_EXEC | ATRX_READ | ATRX_WRITE;
        }
        ((BYTE *)acc.mem)[adr.w16] = *pB++;
        ((BYTE *)acc.atr)[adr.w16] |= ATRX_UPTD0;
    }
}


/*
 * Read from Cache Memory
 */

static int ReadCache(DWORD nAdr, BYTE *pB, DWORD nMany)
{
    MAMAP adr;
    struct EMM(*s)[256];
    struct EMM acc;

    if (bNoCache) { // [TdB 13.03.2012] PH: 3.3.2012 - access-breakpoint - expr. access
        return (0); // condexpr-recalc - access memory in any case...
    }
    if (Opcode) {
        if (!(MonConf.Opt & CACHE_CODE))
            return (0);
    } else {
        if (!(MonConf.Opt & CACHE_MEM))
            return (0);
        if (iRun)
            return (0);
    }

    for (adr.a32 = nAdr; nMany != 0; nMany--, adr.a32++) {
        if ((adr.a32 >= MonConf.SFRStart) && (adr.a32 <= MonConf.SFREnd)) {
            return (0); // SFRs are not cached
        }
        if (adr.a32 >= PPBAddr) {
            return (0); // Private Peripheral Bus is not cached
        }
        s = slots[adr.ub[3]];
        if (s == NULL)
            return (0);
        acc = (*s)[adr.ub[2]];
        if ((acc.mem == NULL) || (acc.atr == NULL))
            return (0);
        if (!(((BYTE *)acc.atr)[adr.w16] & ATRX_UPTD0)) {
            return (0); // Cached Data Invalid
        }
        *pB++ = ((BYTE *)acc.mem)[adr.w16];
    }
    return (1); // Cached Data Valid
}



/*
 * Invalidate everything which may be invalid after Go or Step
 *  (Registers, Caches, etc.)
 */

void Invalidate(void)
{
    RegUpToDate = 0;            // Invalidate Registers
    InvalidateCache();          // Invalidate Cahche
    InvalidateBreakResources(); // Invalidate cached breakpoint resources
}

/*
 * Check if address is the start of a HLL statement
 */
BOOL CodeIsHLL(DWORD nAdr)
{
    AG_SCOPE acp;
    DWORD    nR;

    memset(&acp, 0, sizeof(acp));
    acp.AddrIn = nAdr;
    //---4.3.2011: setup for 'query hll @address' mode without breaking
    //             the normal AG_CB_GETSCOPEINFO functionality:
    acp.nRes[31] = 0xFFFFFFFF; // query for 'hll @address' only
    nR           = pCbFunc(AG_CB_GETSCOPEINFO, (void *)&acp);
    if (nR == 1) {
        return (TRUE);
    }
    return (FALSE);
}

#if DBGCM_FEATURE_PAPROF
/*
 * Performace Analyzer Profile Information
 */

int PAProfInfo(AG_PROFINFO *pI)
{
    // AMAP          x;
    BYTE noProf = (TraceConf.Protocol == TPIU_ETB);

    switch (pI->nCmd) {
        case AG_PROF_CALCCYCLES:

            if (noProf)
                return (AG_PROFINFO_NP); // Disable profiling for now

            pI->cycles = 0;
            if (pI->nStart & 0x00000001)
                return (AG_PROFINFO_BAD);

            //---TODO: Accumulate execution cycles for requested address range into pI->cycles
            DEVELOP_MSG("Todo: \nAccumulate execution cycles over requested address range");
            // for (x.a32 = pI->nStart; (x.a32 < pI->nEnd && x.a32 >= pI->nStart); x.a32 += 2) {
            // pI->cycles += ....
            // if (32-bit opcode) {
            //   x.a32 += 2;
            // }
            // }

            if (TraceConf.Clk) {
                pI->time = (double)(pI->cycles) / TraceConf.Clk;
            }
            break;

        case AG_PROF_TOTALCYCLES:

            if (pio->iRun) {
                //---TODO: Review if total cycle count information is available at finer granularity and set
                //         pI->cycles accordingly.
                DEVELOP_MSG("Todo: \nTotal number of cycles for Execution Profiling while target is running");
                // pI->cycles = ...
            } else {
                pI->cycles = RegARM.nCycles;
            }
            if (TraceConf.Clk) {
                pI->time = (double)(pI->cycles) / TraceConf.Clk;
            }
            break;

        case AG_PROF_EXECCOUNT:

            if (noProf)
                return (AG_PROFINFO_NP); // Disable profiling for now

            pI->count = 0;
            if (pI->nStart & 0x00000001)
                return (AG_PROFINFO_BAD);

            //---TODO: Set execution count for requested address pI->nStart
            DEVELOP_MSG("Todo: \nSet execution count for requested address");
            // pI->count = ...
            break;

        default:
            return (AG_PROFINFO_BAD);
    }

    return (AG_PROFINFO_OK);
}
#endif // DBGCM_FEATURE_PAPROF


#if (DBGCM_FEATURE_COVERAGE || DBGCM_FEATURE_PAPROF)
/*
 * Clear Code Coverage and Execution Profile
 */

static void ClearCoverage(void)
{
    struct EMM(*s)[256];
    struct EMM acc;
    int        i, j, k;

    for (i = 0; i < 256; i++) {
        s = slots[i]; // a24-a31
        if (s) {
            for (j = 0; j < 256; j++) {
                acc = (*s)[j]; // slot available?
                if (acc.atr) {
                    for (k = 0; k < 0x4000; k++) {
                        acc.atr[k] &= ~(ATRX_EXECD | ATRX_ITAKEN | ATRX_EXECDO | ATRX_ITAKENO);
                    }
                }
            }
        }
    }

    //---TODO: Clear any other Code Coverage and Execution Profiling related information
    DEVELOP_MSG("Todo: \nClear remaining Code Coverage \nand Execution Profiling information");
}
#endif // (DBGCM_FEATURE_COVERAGE || DBGCM_FEATURE_PAPROF)

/*
 * Output a message line into uVision2's command window in debug mode
 * and to build window in Flash download mode
 */

void txtout(char *fmt, ...)
{
    va_list marker;
    char    txtbuf[2048];

    va_start(marker, fmt);
    vsprintf(&txtbuf[0], fmt, marker);
    SendMessage(hMfrm, // for Flash Download send message to Build window
                Uv2Msg, pio->FlashLoad ? MSG_UV2_TRNLINE : MSG_UV2_CMDLINE,
                (LPARAM)&txtbuf[0]);
}


/*
 * Output an error message line into uVision2's command window in debug mode
 * and to build window in Flash download mode
 */

void errtxtout(int status)
{
    char *errTxt = StatusText(status);
    if (errTxt) {
        txtout("Error: %s\n", errTxt);
    }
}


/*
 * Output a warning message line into uVision2's command window in debug mode
 * and to build window in Flash download mode
 */

extern void warntxtout(int status)
{
    char *warnTxt = StatusText(status);
    if (warnTxt) {
        txtout("Warning: %s\n", warnTxt);
    }
}


/*
 * Output a message to uVision Status Bar Pane (RTA)
 */

void OutMsg(char *txt)
{
    if ((pio == NULL) || (pio->hwnd == NULL) || (pio->hmsg == NULL))
        return;
    pio->Notify(UV_RTAH_ERRMSG, (void *)txt);
}


/*
* Display Trace Status in Status Bar Pane (RTA)
*/
void OutTraceMsg(BYTE msg)
{
    char buf[128];
    int  n = 0;
    char clr;
    int  clrLen = 0;

    if ((pio == NULL) || (pio->hwnd == NULL) || (pio->hmsg == NULL))
        return;

    if (TraceOpt & TRACE_ENABLE) {
        if (pio->bSecureExt && msg != 0) {
            // 05.11.2018: Consider color preamble
            if (sscanf_s(TraceMsg[msg], "\\<!clr%c>", &clr, 1) == 1) {
                clrLen = strlen("\\<!clrx>");
                strncpy_s(&buf[n], sizeof(buf) - n, TraceMsg[msg], clrLen);
                n += clrLen;
            }
            if (pio->bSTrcEna) {
                n += sprintf(&buf[n], "Secure ");
                T_Secure = TRUE;
            } else {
                n += sprintf(&buf[n], "Non-Secure ");
                T_Secure = FALSE;
            }
        }
    }

    n += sprintf(&buf[n], TraceMsg[msg] + clrLen);
    pio->Notify(UV_RTAH_ERRMSG, (void *)buf);
}


/*
 * Create the 'key' for project-registry access
 */
static char szVals[4096];         // config/setup values
static char szKey[128] = "DBGCM"; // plain name of dll, e.g. 'DBGCM'
static QDLL qdll;

static int UpdateRegVals(void)
{
    int n;

    WriteParms(&szVals[0]); // create args/parms from MonConf/TraceConf
    memset(&qdll, 0, sizeof(qdll));
    qdll.key   = &szKey[0];  // Project-Registry key in current target
    qdll.value = &szVals[0]; // new setup values

    if (pio && pio->Notify) {
        n = pio->Notify(UV_NEW_TARG_SETTINGS, (void *)&qdll); // register new config settings
        if (n != 0) {
            return (-1); // failed to register new config values...
        }
    }

    return (0);
}



/*
 * This function is required for Remote-Setup in uVision Debug mode
 */

static void ConfDisp(DYMENU *pM)
{
    struct MONCONF   mCfg;
    struct TRACECONF tCfg;
    int              i;

    pM;
    mCfg = MonConf;      // save current setup
    tCfg = TraceConf;    // save current setup
    i    = DoDlgSetup(); // start setup dialog

    if (i == IDOK) {
#if DBGCM_DBG_DESCRIPTION
        if ((memcmp(&MonConf, &mCfg, sizeof(mCfg)) != 0) || (memcmp(&TraceConf, &tCfg, sizeof(tCfg)) != 0) || (PDSCDebug_IsSupported() && PDSCDebug_SetupChanged())) {
#else  // DBGCM_DBG_DESCRIPTION
        if ((memcmp(&MonConf, &mCfg, sizeof(mCfg)) != 0) || (memcmp(&TraceConf, &tCfg, sizeof(tCfg)) != 0)) {
#endif // DBGCM_DBG_DESCRIPTION

            // configuration has changed
            if (iRun) {
                i = AGDIMsgBox(hMfrm, "Configuration has been changed, Stop Target and take new values ?",
                               "Target Monitor Notification",
                               MB_OKCANCEL | MB_ICONWARNING, IDOK);
            } else {
                i = AGDIMsgBox(hMfrm, "Configuration has been changed, take new values ?",
                               "Target Monitor Notification",
                               MB_OKCANCEL | MB_ICONWARNING, IDOK);
            }
            if (i == IDCANCEL) {
                MonConf   = mCfg; // restore previous configuration
                TraceConf = tCfg; // restore previous configuration
            }
            if (i == IDOK) {     // take new configuration
                UpdateRegVals(); // write changes to project-registry
                ReInit = 1;
                if (iRun) {
                    StopRun = 1;
                } else {
#if DBGCM_DBG_DESCRIPTION
                    // Pass new configuration to PDSC Setup (not really part of ReInitTarget(), so do it here)
                    if (PDSCDebug_IsEnabled()) {
                        PDSCDebug_Reinit(); // Safe, only debug access vars are updated in non-Setup Mode
                    }
#endif // DBGCM_DBG_DESCRIPTION

                    if (ReInitTarget()) { // failed...
                        // Keep the PlayDead handling as it is here; we are not in a path that ends in PlayDead handling
                        if (!PlayDead) {
                            OutErrorDelayed(); // Print last AGDI error message to cmd window

#if DBGCM_V8M
                            if (NonSecureCantStop()) {
                                AGDIMsgBox(hMfrm, &NonSecureReInitErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                            } else {
                                AGDIMsgBox(hMfrm, &FatalReInitErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                            }
#else  // DBGCM_V8M
                            AGDIMsgBox(hMfrm, &FatalReInitErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
#endif // DBGCM_V8M
                        }
                        // Don't set PlayDead here to avoid firing a second MSG_UV2_TERMINATE message
                        StopTarget(); // final shutdown
                        PostMessage(hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
                    }
                    UpdateAllDlg();
                }
            }
        }
    } else { // cancelled: restore previous config
        MonConf   = mCfg;
        TraceConf = tCfg;
    }
}



/*
 * Extension Menues and Dialogs
 */


static DYMENU MenuUV3[] = {
    { 1, "Debug Settings...", ConfDisp, 0, 0, NULL }, // Settings (Modal Dlg)
    { 2, "Trace", NULL, 0, 0, NULL },                 // Trace Group Start
#if !DBGCM_FEATURE_TRCDATA_WIN
    { 1, "&Records", TR_Disp, 0, IDD_TRACE_REC, &TR_Dlg }, // Trace Records
#endif                                                     // !DBGCM_FEATURE_TRCDATA_WIN
    { -2, NULL, NULL, 0, 0, NULL },                        // Trace Group End
    { -1, /* End of menu list */ },
};

static DYMENU MenuUV4[] = {
    { 1, "Debug Settings...", ConfDisp, 0, 0, NULL }, // Settings (Modal Dlg)
    { 3, "_Trace", NULL, 0, 0, NULL },                // Trace Group Start
#if !DBGCM_FEATURE_TRCDATA_WIN
    { 1, "&Records", TR_Disp, 0, IDD_TRACE_REC, &TR_Dlg }, // Trace Records
#endif                                                     // !DBGCM_FEATURE_TRCDATA_WIN
    { -3, NULL, NULL, 0, 0, NULL },                        // Trace Group End
    { -1, /* End of menu list */ },
};




/*
 * uVision2 want's to update all modeless dialogs.
 */

void UpdateAllDlg(void)
{ // Update all modeless extension dialogs
    DYMENU *pM;

    pM = pDyMenu;
    while (pM->nDelim != -1) {              // while not end of Menu-list
        if (pM->pDlg && pM->pDlg->Update) { // if dialog has been created
            pM->pDlg->Update();             // then call it's update function
        }
        ++pM; // next menu entry.
    }
}


DWORD SetTargKey(const char *pKey, const char *pString)
{
    void *p[2];
    DWORD nR;

    p[0] = (void *)pKey;    // example: "__IntrPos"
    p[1] = (void *)pString; // example: "0A55AE44019F"
    nR   = pio->Notify(47, (void *)&p[0]);
    return (nR);
}

const char *GetTargKey(const char *pKey)
{
    const char *pVal;

    pVal = (const char *)pio->Notify(46, (void *)pKey);
    return (pVal);
}


/*
 * Close all currently open modeless dialogs and store window position
 */

void CloseAllDlg(void)
{
    DYMENU *pM;
    char    s[128];
    int     i;

    s[0] = 0;
    pM   = pDyMenu;
    while (pM->nDelim != -1) {
        if (pM->pDlg) { // Dialog is still open
                        //    pM->pDlg->iOpen = 0;              // RR: 07.04.2008
            if (pM->pDlg->hw) {
                pM->pDlg->Kill(pM->pDlg); // so close it
                pM->pDlg->iOpen = 1;      // auto reopen next time
            }
        }
        ++pM;
    }

    // Store Dialog Position and Status in Target Setup
    i  = 0;
    pM = pDyMenu;
    while (pM->nDelim != -1) {
        if (pM->pDlg) { // Dialog is present
            i += sprintf(&s[i], "(%d=%d,%d,%d,%d,%d)", pM->nDlgId,
                         pM->pDlg->rc.left,
                         pM->pDlg->rc.top,
                         pM->pDlg->rc.right,
                         pM->pDlg->rc.bottom,
                         pM->pDlg->iOpen);
        }
        ++pM;
    }
    SetTargKey("DLGUARM", s);
}


#define SM_XVIRTUALSCREEN    76
#define SM_YVIRTUALSCREEN    77
#define SM_CXVIRTUALSCREEN   78
#define SM_CYVIRTUALSCREEN   79
#define SM_CMONITORS         80
#define SM_SAMEDISPLAYFORMAT 81


void GetScreenRc(RECT *Rc)
{
    if (GetSystemMetrics(SM_CMONITORS) <= 1) {
        GetWindowRect(GetDesktopWindow(), Rc);
        return;
    }
    Rc->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    Rc->top  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    // R.K. 15.1.2008: when primary screen is RIGHT, Rc->left is negativ
    Rc->right  = Rc->left + GetSystemMetrics(SM_CXVIRTUALSCREEN); // R.K. 15.1.2008
    Rc->bottom = Rc->top + GetSystemMetrics(SM_CYVIRTUALSCREEN);  // R.K. 15.1.2008
}


/*
 * Restore dialog window position and open all previously open dialogs
 */

void OpenAllDlg(void)
{
    DYMENU     *pM;
    char const *s;
    DWORD       n, dlgid, open;
    LONG        left, top, right, bottom;
    RECT        r;

    s = GetTargKey("DLGUARM");
    if (s == NULL)
        return;
    GetScreenRc(&r);

    // Restore Dialog Position and Status from Target Setup
    while (1) {
        while ((*s != '(') && (*s != 0))
            s++;
        if (*s == 0)
            break;
        n = sscanf(s, "(%d=%d,%d,%d,%d,%d)", &dlgid, &left, &top, &right, &bottom, &open);
        if (n != 6)
            break; // Error: abort further analysis
        /* check if position of dialog is on screen */
        if (left != -1) {
            if (left < r.left) {
                right = (right - left) + r.left;
                left  = r.left;
            }
            if (top < r.top) {
                bottom = (bottom - top) + r.top;
                top    = r.top;
            }
            if (right > r.right) {
                left  = r.right - (right - left);
                right = r.right;
            }
            if (bottom > r.bottom) {
                top    = r.bottom - (bottom - top);
                bottom = r.bottom;
            }
        }
        pM = pDyMenu;
        while (pM->nDelim != -1) {     // search corresponding dialogs
            if (pM->nDlgId == dlgid) { // Dialog found ?
                pM->pDlg->iOpen     = open;
                pM->pDlg->rc.left   = left;
                pM->pDlg->rc.top    = top;
                pM->pDlg->rc.right  = right;
                pM->pDlg->rc.bottom = bottom;
                break;
            }
            ++pM;
        }
        s++;
    }

    // Open all previously open dialogs
    pM = pDyMenu;
    while (pM->nDelim != -1) {          // run until end of dialog list
        if (pM->pDlg) {                 // Dialog is present
            if (pM->pDlg->iOpen == 1) { // Auto reopen
                pM->fp(pM);             // so open it
            }
        }
        ++pM;
    }
}



//--- Interface functions between AGDI and target follow
//------------------------------------------------------


// Get ErrorText for status information
//char *StatusText (int status)  {
char *_StatusText(int status)
{
    switch (status) {
        case 0: return ("No Error");
        case EU01: return ("Internal DLL Error");
        case EU02: return ("No Debug Unit Found");
        case EU03: return ("No JTAG Devices Found");
        case EU04: return ("Too Many JTAG Devices in Chain");
        case EU05: return ("JTAG Communication Failure");
        case EU06: return ("JTAG Device Chain Error");
        case EU07: return ("JTAG RTCK Failure");
        case EU08: return ("SWD Communication Failure");
        case EU09: return ("No Cortex-M Device found in JTAG chain.\nPlease check the JTAG cable and the connected devices.");
        case EU10: return ("No Cortex-M SW Device Found");
        case EU11: return ("Device could not be powered up");
        case EU12: return ("Invalid ROM Table");
        case EU13: return ("Cannot enter Debug Mode");
        case EU14: return ("Cannot access Memory");
        case EU15: return ("Trace HW not present");
        case EU16: return ("Trace available only on SW!\nPlease select SW Port.");
        case EU17: return ("Selected SWO Clock not supported!");
        case EU18: return ("Not possible while target is executing");
        case EU19: return ("Selected Trace Port is not supported!");
        case EU20: return ("Internal DLL Error: Unsupported Debug Protocol");
        case EU21: return ("Unsupported Memory Access Size");
        case EU22: return ("PDSC: Debug Description not available");
        case EU23: return ("PDSC: Cannot stop target");
        case EU24: return ("PDSC: Cannot recover from reset");
        case EU25: return ("PDSC: Unknown Debug Port ID.\nCannot switch to Debug Port.");
        case EU26: return ("PDSC: Sequence not implemented");
        case EU27: return ("PDSC: Sequence Execution failed");
        case EU28: return ("PDSC: Data Patch failed");
        case EU29: return ("PDSC: Multiple SW Debug Port definitions.\nMulti-Drop SWD not supported.");
        case EU30: return ("PDSC: Debug Port name redifinition");
        case EU31: return ("PDSC: JTAG Debug Port ID Code redifinition");
        case EU32: return ("PDSC: JTAG Debug Port IR Length redifinition");
        case EU33: return ("PDSC: JTAG TAP Index out of bounds");
        case EU34: return ("PDSC: Multiple Implementations valid for same Sequence Context");
        case EU35: return ("PDSC: Unknown Sequence ID.\nCannot execute Sequence.");
        case EU36: return ("PDSC: Debug Description disabled");
        case EU37: return ("PDSC: Sequence disabled");
        case EU38: return ("PDSC: No data patch available");
        case EU39: return ("Parameter Error");
        case EU40: return ("Cannot recover Debug Connection");
        case EU41: return ("Atomic sequences not supported");
        case EU42: return ("Atomic sequence assembly error");
        case EU43: return ("Atomic sequence execution error");
        case EU44: return ("Debug Server lost");
        case EU45: return ("CPU locked up, stopping target");
        case EU46: return ("Unknown CPU");
        case EU47: return ("Cannot change security view while target executes");
        case EU48: return ("Could not reset target with non-secure debugger");
        case EU49: return ("TRACE_CLK is read-only for option 'Use Core Clock'. Write CORE_CLK instead."); /* 02.04.2019 */
        case EU50: return ("Invalid Access Port selected");
        case EU51: return ("Unsupported Access Port");
        case EU52: return ("Connection refused due to device mismatch!\n\nDevice connected to Debug Unit is different\nfrom device selected for project target");
        case EU53: return ("ETB Trace Error");
        case EU54: return ("Trace flush failed");
        case EU55: return ("No supported CPU behind selected debug access port");
        default: return ("Unknown Error");
    }
}


/* Display Error Text */
void OutError(int status)
{
    if (PlayDead)
        return;
    AGDIError     = TRUE;
    AGDIErrorCode = status;
}

/* Display Error Text (after CheckCom is released!) */
void OutErrorDelayed(void)
{
    char *text;

    if (AGDIError) {
        text = StatusText(AGDIErrorCode);
        txtout("%s\n", text);
    }
}

void OutErrorMessage(int status)
{ // during the initialization, messages in the command window are not visible
    char *text;

    if (PlayDead)
        return;
    text = StatusText(status);
    AGDIMsgBox(hMfrm, text, &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
}


/*
 * Sets PlayDead and PlayDeadMsg, MsgTxt can be NULL
 */
void SetPlayDead(const char *MsgTxt)
{
    if (PlayDead)
        return; // PlayDead already set, use the previous PlayDeadMsg
    PlayDead = 1;

#if DBGCM_RECOVERY
    if (dbgAccLevel == DA_NORMAL) {
        PlayDeadMsg = MsgTxt;
    } else {
        // Debugger GUI shows message, no further message boxes required
        PlayDeadMsg = NULL;
    }
#else  // DBGCM_RECOVERY
    PlayDeadMsg = MsgTxt;
#endif // DBGCM_RECOVERY
}


/*
 * Do what's required to play dead
 */
void ExecPlayDead()
{
    BOOL        bShutdown = FALSE;
    BOOL        bDelay    = FALSE;
    const char *pMsg      = PlayDeadMsg; // Store message locally and clear it, AGDIMsgBox causes recursion
    if (!PlayDead)
        return; // Nothing to do

    if (PlayDeadShut) {
        // Don't enter the thread-safe part if it is clear we won't init the shutdown
        return;
    }

    if (WaitForSingleObject(PlayDeadShut_mtx, INFINITE) != WAIT_OBJECT_0) {
        // Abort, obtaining mutex seriously failed
        return;
    }
    if (!PlayDeadShut) {
        // bShutdown    = TRUE;
        // PlayDeadShut = 1;
        // 10.12.2018: Immediate shutdown only from GUI thread
        if (GetWindowThreadProcessId(hMfrm, NULL) == GetCurrentThreadId()) { // Is GUI thread?
            bShutdown    = TRUE;
            PlayDeadShut = 1;
        } else if (!PlayDeadDelayedMsg) {
            bDelay             = TRUE;
            PlayDeadDelayedMsg = 1;
        }
    } //else keep init value FALSE
    if (!ReleaseMutex(PlayDeadShut_mtx)) {
        // Abort, releasing mutex seriously failed
        if (bShutdown)
            PlayDeadShut = 0; // maybe we recover later on...
    }

    if (bShutdown) {
        PlayDeadMsg = NULL;
        if (pMsg != NULL) {
            OutErrorDelayed(); // Should be safe here, we shouldn't be in a "CheckCom" section when calling this
            AGDIMsgBox(hMfrm, pMsg, &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
        }

        // if (bShutdown) {
        // This is the first call, do the shutdown
        StopTarget(); // shut down driver
        if (!pio->FlashLoad)
            PostMessage(hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0); // unload this driver
    } else if (bDelay) {
        PlayDeadNotifyMsg.n1 = UV_UPDATE_GUTTERS;
        ::PostMessage(pio->hwnd, pio->hmsg, MSG_UV2_NOTIFY, (LPARAM)&PlayDeadNotifyMsg); // Post gutter update message to trigger memory reads from the GUI thread kicking off the real PlayDead
    }
}


// PlayDead but not a regular shutdown
BOOL HardPlayDead(void)
{
    if (PlayDead && !PlayDeadRegular) {
        return TRUE;
    }
    return FALSE;
}


#if DBGCM_WITHOUT_STOP
static BOOL SkipTargetStop(void)
{
#if DBGCM_V8M
    if (DbgExitStarted && ((MonConf.Opt & CONN_NO_STOP) || NonSecureCantStop())) {
        return (TRUE);
    }
#else  // DBGCM_V8M
    if (DbgExitStarted && (MonConf.Opt & CONN_NO_STOP)) {
        return (TRUE);
    }
#endif // DBGCM_V8M

    return (FALSE);
}
#endif // DBGCM_WITHOUT_STOP


/*
 * CheckCom available (prevent from multi-threading recursions)
 */

extern HANDLE Com_mtx;

static void CheckCom(int stat)
{
    DWORD Res;

    if (stat) {
        // Wait forever to get mutex
        Res = WaitForSingleObject(Com_mtx, INFINITE);
        if (Res != WAIT_OBJECT_0) {
            // FAILED!
        } else {
            AGDIError = FALSE;
        }
    } else {
        if (!ReleaseMutex(Com_mtx)) {
            // FAILED!
        } else {
            OutErrorDelayed();
        }
    }
}


__inline void UpdatePBStart(void)
{
    if (pBStart == &TBp) {
        pBStart->next = *pBhead;
    } else {
        pBStart = *pBhead;
    }
    // 12.07.2016: Orignal update of pBStart, changed to fix lockups
    // when setting breakpoints during long StepOver operations.
    // pBStart->next = *pBhead;
}


#if DBGCM_DS_MONITOR
void RunningCB(U32 info)
{
    RUNNINGPARM rparm;
    memset(&rparm, 0, sizeof(rparm));
    rparm.info = info;
    pCbFunc(AG_CB_RUNNING, (void *)(&rparm));
}
#endif // DBGCM_DS_MONITOR


#if DBGCM_WITHOUT_STOP
// Disables all internal and external interrupts including Hard Faults
static void DisableAllExceptions(void)
{
    // Update SYS Registers
    GetRegs(1ULL << nSYS);

    // Set FAULTMASK Bit
    if (RegARM.SYS.R.FAULTMASK == 0) {
        RegARM.SYS.R.FAULTMASK = 1;

#if DBGCM_V8M
        SetRegs(&RegARM, NULL, NULL, (1ULL << nSYS));
#else  // DBGCM_V8M
        SetRegs(&RegARM, NULL, (1ULL << nSYS));
#endif // DBGCM_V8M
    }
}


static int InitRunningTarget(void)
{
    int   status;
    int   res;
    DWORD val;

    res = DSM_WaitForState(0, DSM_STATE_CPU_HALTED, TRUE, 5);
    if (res >= DSM_WAIT_ERR) {
        OutErrorMessage(EU12);
        return (1);
    }

    // 11.04.2017: Stop CPU before Flash Download without Stop connection
    if (pio->FlashLoad) {
        if (!(DSMonitorState & DSM_STATE_CPU_HALTED)) {
            // Stop Target
#if DBGCM_V8M
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#else  // DBGCM_V8M
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#endif // DBGCM_V8M

            // Check if Target is stopped
            res = DSM_WaitForState(DSM_STATE_CPU_HALTED, DSM_STATE_CPU_HALTED, TRUE, 100);
            if (res >= DSM_WAIT_ERR) {
                OutErrorMessage(EU12);
                return (1);
            }
        }

        if (!(DSMonitorState & DSM_STATE_CPU_HALTED)) {
            // Couldn't stop target for flash download
            AGDIMsgBox(hMfrm, &StopErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
            return (1);
        }

        // Prepare target for flash download
        DisableAllExceptions();

        return (0);
    }

    if (DSMonitorState & DSM_STATE_CPU_HALTED)
        return (0); // Target stopped

    iRun = 1; // Update run status

    // Update PC from DWT PC Sample Register (if available)
    // Otherwise set to 0.

#if DBGCM_V8M
    status = ReadD32(DWT_PCSR, &RegDWT.PCSR, BLOCK_SECTYPE_ANY);
    if (status) {
        OutErrorMessage(status);
        return (1);
    }

    if (RegDWT.PCSR == 0xFFFFFFFF)
        RegDWT.PCSR = 0x00000000; // PCSR == 0xFFFFFFFF - target stopped or debug not allowed
    *pCURPC = RegARM.PC = RegDWT.PCSR;

    // Enable Breakpoint Unit if not done already.
    // Scripting and Restore Breakpoints could require this.

    status = ReadD32(FPB_CTRL, &val, BLOCK_SECTYPE_ANY);
    if (status) {
        OutErrorMessage(status);
        return (1);
    }

    if (!(val & FPB_ENABLE)) {
        status = WriteD32(FPB_CTRL, val | FPB_KEY | FPB_ENABLE, BLOCK_SECTYPE_ANY);
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
    }
#else  // DBGCM_V8M
    status = ReadD32(DWT_PCSR, &RegDWT.PCSR);
    if (status) {
        OutErrorMessage(status);
        return (1);
    }

    if (RegDWT.PCSR == 0xFFFFFFFF)
        RegDWT.PCSR = 0x00000000; // PCSR == 0xFFFFFFFF - target stopped or debug not allowed
    *pCURPC = RegARM.PC = RegDWT.PCSR;

    // Enable Breakpoint Unit if not done already.
    // Scripting and Restore Breakpoints could require this.

    status = ReadD32(FPB_CTRL, &val);
    if (status) {
        OutErrorMessage(status);
        return (1);
    }

    if (!(val & FPB_ENABLE)) {
        status = WriteD32(FPB_CTRL, val | FPB_KEY | FPB_ENABLE);
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
    }
#endif // DBGCM_V8M

    RunningCB(RUNNING_AFTER_CONNECT); // Notify SARMCM3 about having detected a running target after connection

    return (0);
}
#endif // DBGCM_WITHOUT_STOP


/*
 * Initialize your target communication
 * Return 0 if Ok, 1 if failed.
 */

U32 InitTarget(void)
{
    int   status;
    DWORD val, n;

#if DBGCM_DBG_DESCRIPTION
    status = PDSCDebug_Init();
    if (status == 0) {
        return PDSCDebug_InitTarget();                 // Initialize your target..., function does the error output
    } else if (status != (EU22) && status != (EU36)) { // PDSC: Debug Description not available/PDSC: Debug Description disabled
        OutErrorMessage(status);
        return (1);
    }
    status = 0; // Use old way
#endif          // DBGCM_DBG_DESCRIPTION

    OutMsg("");

    //---TODO:
    // Init Target and configure according MonConf (Debug Port & Clock ...)
    //if (bAnyUnit) {                       // Select "Any" (first) Unit
    //  strcpy(MonConf.UnitSerNo, "Any");
    //  active unit = 0
    //}
    DEVELOP_MSG("Todo: \nInit Target and configure according MonConf (Debug Port & Clock ...)\nreturn(1); if init fails.");

    if (MonConf.Opt & INIT_RST_PULSE) {
        //---TODO:
        // HW Chip Reset (pulse)
        DEVELOP_MSG("Todo: \nHW Chip Reset (pulse)");
    }
    if (MonConf.Opt & INIT_RST_HOLD) {
        //---TODO:
        // Assert HW Chip Reset
        DEVELOP_MSG("Todo: \nAssert HW Chip Reset");
    }

    if (MonConf.Opt & PORT_SW) {
        DP_Type = SW_DP;
        if (MonConf.Opt & USE_SWJ) {
            // SWJ Switch to SWD
            status = SWJ_Reset();
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
            status = SWJ_Switch(0xE79E);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
        }
        status = SWJ_Reset();
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
        status = SWD_ReadID();
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
        MonConf.JtagCpuIndex = 0;
    } else {
        DP_Type = JTAG_DP;
        if (MonConf.Opt & USE_SWJ) {
            // SWJ Switch to JTAG
            status = SWJ_Reset();
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
            status = SWJ_Switch(0xE73C);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
            status = SWJ_Reset();
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
        } else {
            status = JTAG_Reset();
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
        }
        if ((MonConf.Opt & JTAG_MANUAL) == 0) {
            // Auto Detect Devices
            status = JTAG_DetectDevices();
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
            if (MonConf.JtagCpuIndex > (JTAG_devs.cnt - 1)) {
                MonConf.JtagCpuIndex = JTAG_devs.cnt - 1;
            }
        }
        //---TODO:
        // Select active device in chain according to MonConf.JtagCpuIndex if ID JTAG is valid
        DEVELOP_MSG("Todo: \nSelect active device in chain according to MonConf.JtagCpuIndex if ID JTAG is valid");
        status = JTAG_ReadID();
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
    }

    JTAG_devs.com_no = nCPU = MonConf.JtagCpuIndex;

    if (nCPU == -1) {
        AGDIMsgBox(hMfrm, &JTAGErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
        return (1);
    } else {
        status = Debug_Init();
        if (status) {
            OutErrorMessage(status);
            return (1);
        }

#if DBGCM_WITHOUT_STOP
        if (MonConf.Opt & CONN_NO_STOP) {
#if DBGCM_V8M
            // Enable Debug
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#else  // DBGCM_V8M \
       // Enable Debug
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#endif // DBGCM_V8M

        } else {
#endif // DBGCM_WITHOUT_STOP

#if DBGCM_V8M
            // Enable Reset Vector Catch
            status = WriteD32(DBG_EMCR, VC_CORERESET, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }

            // Enable Debug & Stop Target (except when Reset is active)
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#else  // DBGCM_V8M \
       // Enable Reset Vector Catch
        status = WriteD32(DBG_EMCR, VC_CORERESET);
        if (status) {
            OutErrorMessage(status);
            return (1);
        }

        // Enable Debug & Stop Target (except when Reset is active)
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
#endif // DBGCM_V8M

#if DBGCM_WITHOUT_STOP
        }
#endif // DBGCM_WITHOUT_STOP

        if (MonConf.Opt & INIT_RST_HOLD) {
            //---TODO:
            // Deassert HW Chip Reset
            DEVELOP_MSG("Todo: \nDeassert HW Chip Reset");

            // 02.10.2019: Device check if connection under reset. Not all devices are fully accessible under reset.
            if (pCbFunc(AG_CB_CHECKDEVICE, NULL)) { // Genuine device check, called here after DAP is ready for accesses
                status = EU52;                      // Connection refused due to device mismatch!
                OutErrorMessage(status);
                return (1);
            }
        }

#if DBGCM_WITHOUT_STOP
        if (!(MonConf.Opt & CONN_NO_STOP) && !pio->FlashLoad) {
#endif // #if DBGCM_WITHOUT_STOP

            // Check if Target is stopped
            n = GetTickCount();
            do {
#if DBGCM_V8M
                status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
                if (val & S_HALT)
                    break;
#else  // DBGCM_V8M
            status = ReadD32(DBG_HCSR, &val);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
            if (val & S_HALT)
                break;
#endif // DBGCM_V8M

            } while ((GetTickCount() - n) < 500);

            if ((val & S_HALT) == 0) {
#if DBGCM_V8M
                if (NonSecureCantStop()) {
                    txtout("Warning: %s\n", &NonSecureStopErr[0]);
                } else {
                    AGDIMsgBox(hMfrm, &StopErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                    return (1);
                }
#else  // DBGCM_V8M
            AGDIMsgBox(hMfrm, &StopErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
            return (1);
#endif // DBGCM_V8M
            }

#if DBGCM_V8M
            if (!NonSecureCantStop()) {
                // Ensure that MASKINTS is cleared while target is stopped (if not required to be set)
                status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
            }
#else  // DBGCM_V8M \
       // Ensure that MASKINTS is cleared while target is stopped (if not required to be set)
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
#endif // DBGCM_V8M

#if DBGCM_WITHOUT_STOP
        }
#endif // DBGCM_WITHOUT_STOP

#if DBGCM_V8M
        status = WriteD32(DBG_EMCR, TRCENA | MONCONF_RST_VECT_CATCH, BLOCK_SECTYPE_ANY); // Enable Trace
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
#else  // DBGCM_V8M
        status = WriteD32(DBG_EMCR, TRCENA | MONCONF_RST_VECT_CATCH); // Enable Trace
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
#endif // DBGCM_V8M

        RegARM.nCycles = 0;

        // Detect number of HW Breakpoint & Watchpoints
#if 0
    if (FPB_Addr) {
      status = ReadD32 (FPB_CTRL, &val);
      if (status) { OutErrorMessage (status); return (1); }
      NumBP = (BYTE)((val & FPB_NUM_CODE) >> FPB_NUM_CODE_P);
      FPB_Ver = BYTE((val & FPB_REV) >> FPB_REV_P);
      if (FPB_Ver > 0) {  // FPB version 1 (Cortex-M7)
        FPB_CompMask = FPB_V1_COMP_M;
      }
    } else {
      NumBP = 0;
      FPB_Ver = 0;
    }
    if (DWT_Addr) {
      status = ReadD32 (DWT_CTRL, &val);
      if (status) { OutErrorMessage (status); return (1); }
      NumWP = (BYTE)((val & DWT_NUMCOMP) >> DWT_NUMCOMP_P);
    } else {
      NumWP = 0;
    }
#endif

        if ((NumBP = DetectBreakResources(&FPB_Ver)) == -1) {
            // Error, error message already shown in DetectBreakResources().
            return (1);
        }
        if (FPB_Ver > 0) { // FPB version 1 (Cortex-M7)
            FPB_CompMask = FPB_V1_COMP_M;
        }
        if ((NumWP = DetectDwtResources()) == -1) {
            // Error, error message already shown in DetectDwtResources().
            return (1);
        }

        // Init rest of break resources
        InitBreakResources();

        NTrWP = 0;
        MTrWP = 0;
        MDtWP = 0;

        // 27.01.2020: Tracepoint extensions
        NTrRP = 0;
        NTrSP = 0;
        NTrHP = 0;
        NTrDP = 0;
        MTrRP = 0;
        MTrSP = 0;
        MTrHP = 0;
        MTrDP = 0;

        CntBP = 0;
        ClearUsedBreakResources(); // Clear used breakpoint resources in resource manager
        CntWP = 0;

        memset(&RegFPB, 0, sizeof(RegFPB));
        memset(&RegDWT, 0, sizeof(RegDWT));
        memset(&UseWP, 0, sizeof(UseWP));

        // Disable Breakpoints
        if (NumBP) {
#if DBGCM_V8M
            status = WriteD32(FPB_CTRL, FPB_KEY, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#else  // DBGCM_V8M
            status = WriteD32(FPB_CTRL, FPB_KEY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
#endif // DBGCM_V8M
        }

#if DBGCM_V8M
        // Disable Watchpoints
        for (n = 0; n < NumWP; n++) {
            status = WriteD32(DWT_FUNC + (n << 4), 0, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
            if (IsV8M()) {
                // Read Comparator Function IDs
                status = ReadD32(DWT_FUNC + (n << 4), &RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
            }
        }
#else  // DBGCM_V8M \
       // Disable Watchpoints
        for (n = 0; n < NumWP; n++) {
            status = WriteD32(DWT_FUNC + (n << 4), 0);
            if (status) {
                OutErrorMessage(status);
                return (1);
            }
        }
#endif // DBGCM_V8M

        switch (xxCPU) {
            case ARM_CM0:
            case ARM_CM0P:
            case ARM_CM1:
            case ARM_SC000:
                NumIRQ = 32; // Fixed according to ARM (NVIC_ICT does not exist)
                break;
            case ARM_CM3:
            case ARM_CM4:
            case ARM_CM7:
            case ARM_SC300:
            case ARM_CM33:
            case ARM_CM35P: {
#if DBGCM_V8M
                // Number of External IRQ
                status = ReadD32(NVIC_ICT, &val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
#else  // DBGCM_V8M \
       // Number of External IRQ
                status = ReadD32(NVIC_ICT, &val);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
#endif // DBGCM_V8M

                NumIRQ = ((val & INTLINESNUM) + 1) << 5;
                if (NumIRQ > 240)
                    NumIRQ = 240;

#if DBGCM_V8M
                // Enable CYCCNT and clear all counters
                if (IsV8M()) {
                    status = ReadBlock(DWT_CTRL, (BYTE *)&RegDWT.CTRL, 4, BLOCK_SECTYPE_ANY); // [TdB: 03.02.2017] (SDMDK-6636) preserve DWT_CTRL.CYCDISS Bit
                    if (status) {
                        OutErrorMessage(status);
                        return (1);
                    }
                    RegDWT.CTRL &= DWT_CYCDISS;
                    RegDWT.CTRL |= DWT_CYCCNTEN;
                } else {
                    RegDWT.CTRL = DWT_CYCCNTEN;
                }
                status = WriteBlock(DWT_CTRL, (BYTE *)&RegDWT, 0x1C, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
#else  // DBGCM_V8M \
       // Enable CYCCNT and clear all counters
                RegDWT.CTRL = DWT_CYCCNTEN;
                status = WriteBlock(DWT_CTRL, (BYTE *)&RegDWT, 0x1C, 0 /*attrib*/);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
#endif // DBGCM_V8M
            } break;
            case ARM_CM23:

#if DBGCM_V8M
                // Number of External IRQ (Configurable for Baseline opposed to v6-M)
                status = ReadD32(NVIC_ICT, &val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
#else  // DBGCM_V8M \
       // Number of External IRQ (Configurable for Baseline opposed to v6-M)
                status = ReadD32(NVIC_ICT, &val);
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }
#endif // DBGCM_V8M

                NumIRQ = ((val & INTLINESNUM) + 1) << 5;
                if (NumIRQ > 240)
                    NumIRQ = 240;
                break;
        }

        Invalidate(); // Registers, Memory cache

#if DBGCM_DS_MONITOR
        status = DSM_StartMonitor();
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
#endif // DBGCM_DS_MONITOR

        if (!pio->FlashLoad) {
#if DBGCM_WITHOUT_STOP

#if DBGCM_V8M
            // if (!(MonConf.Opt & CONN_NO_STOP) || !NonSecureCantStop()) {
            if (!(MonConf.Opt & CONN_NO_STOP) && !NonSecureCantStop()) { // 05.02.2019: Fixed condition, used to be '||' which is wrong
                GetRegs((1ULL << nR15) | (1ULL << nPSR));                // Read PC & xPSR
            }
#else  // DBGCM_V8M
            if (!(MonConf.Opt & CONN_NO_STOP)) {
                GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR
            }
#endif // DBGCM_V8M

#else  // DBGCM_WITHOUT_STOP
            GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR
#endif // DBGCM_WITHOUT_STOP

            if (TraceConf.Opt & TRACE_ENABLE) {
                //---TODO: Check supported trace types (see "TPIU Pin Protocol" in Debug.h)
                DEVELOP_MSG("Todo: \nCheck if debugger supports selected trace protocol");
                //---OutErrorMessage (EU19); return (1);  // Selected Trace Port is not supported
                if (DP_Type != SW_DP) {
                    if (TraceConf.Protocol == TPIU_SWO_MANCHESTER || TraceConf.Protocol == TPIU_SWO_UART) {
                        // SWO Trace only available in Serial Wire Debug mode
                        OutErrorMessage(EU16);
                        return (1);
                    }
                }
                status = Trace_Init();
                if (status) {
                    OutErrorMessage(status);
                    return (1);
                }

                if (TraceConf.Protocol == TPIU_ETB) {
                    status = ETB_Init(); // Check if ETB selected and available
                    if (status) {
                        OutErrorMessage(status);
                        return (1);
                    }

                    // setup the trace funnel if available
                    status = CSTF_Setup(); // Also returns success if CSTF not available
                    if (status)
                        return status;

                    status = ETB_Setup();
                    if (status) {
                        OutErrorMessage(status);
                        return (1);
                    }

                    status = Trace_Setup();
                    if (status) {
                        OutErrorMessage(status);
                        return (1);
                    }

                    status = ETB_Flush(); // Flush to ensure no leftovers on ATB
                    if (status) {
                        OutErrorMessage(status);
                        return (1);
                    }
                } else {
                    if (TPIU_Type == TPIU_TYPE_CS || TPIU_Type == TPIU_TYPE_SWO) {
                        // setup the trace funnel if available
                        status = CSTF_Setup(); // Also returns success if CSTF not available
                        if (status)
                            return status;
                    }

                    status = Trace_Setup();
                    if (status) {
                        OutErrorMessage(status);
                        return (1);
                    }
                }
            }
        }
    }

    return (0); // say 'Ok.'
}


/*
 * Target settings have changed (Baudrate or ComPort for example)
 * Return 0 if Ok, 1 if failed.
 */

U32 ReInitTarget(void)
{
    int     status;
    union v v;
    DWORD   vp, vn;
    DWORD   n, val;

    // 02.04.2019: Moved up here
#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        return PDSCDebug_ReInitTarget();
    }
#endif

    // 02.04.2019: Added
    if (!ReInit || ReInitInProgress) {
        // Reinitialization already executed or still in progress,
        // nothing to do for us.
        return 0;
    }

    //---TODO: shutdown target
    //---      reinit communication with current 'MonConf' values
    DEVELOP_MSG("Todo: \nshutdown target, reinit communication with current 'MonConf' values");
    ReInit           = 0;
    ReInitInProgress = 1; // 02.04.2019

#if 0 // 02.04.2019: Was in wrong place (see above)
#if DBGCM_DBG_DESCRIPTION
  if (PDSCDebug_IsEnabled()) {
    return PDSCDebug_ReInitTarget();
  }
#endif // DBGCM_DBG_DESCRIPTION
#endif

    if (pCoreClk) {
        v.ul = TraceConf.Clk;
        pio->p_putval(pCoreClk, &v);
    }

    // 02.04.2019: Separate Trace Clock setting - TRACE_CLK VTREG
    if (pTraceClk) {
        v.ul = (TraceConf.Opt & TRACE_USE_CORECLK) ? TraceConf.Clk : TraceConf.TPIU_Clk;
        pio->p_putval(pTraceClk, &v);
    }

    // Update Reset Vector Catch Setting
#if DBGCM_V8M
    status = ReadD32(DBG_EMCR, &val, BLOCK_SECTYPE_ANY);
    // if (status) { OutError(status); return (status); }
    if (status) {
        OutError(status);
        ReInitInProgress = 0;
        return (status);
    } // 02.04.2019
    if ((val & VC_CORERESET) != MONCONF_RST_VECT_CATCH) {
        val ^= VC_CORERESET;
        status = WriteD32(DBG_EMCR, val, BLOCK_SECTYPE_ANY);
        // if (status) { OutError(status); return (status); }
        if (status) {
            OutError(status);
            ReInitInProgress = 0;
            return (status);
        } // 02.04.2019
    }
#else  // DBGCM_V8M
    status = ReadD32(DBG_EMCR, &val);
    // if (status) { OutError(status); return (status); }
    if (status) {
        OutError(status);
        ReInitInProgress = 0;
        return (status);
    } // 02.04.2019
    if ((val & VC_CORERESET) != MONCONF_RST_VECT_CATCH) {
        val ^= VC_CORERESET;
        status = WriteD32(DBG_EMCR, val);
        // if (status) { OutError(status); return (status); }
        if (status) {
            OutError(status);
            ReInitInProgress = 0;
            return (status);
        } // 02.04.2019
    }
#endif // DBGCM_V8M

    if (TraceConf.Opt & TRACE_ENABLE) {
        status = 0; // init for next block
        if (TraceConf.Protocol == TPIU_ETB) {
            status = ETB_Setup();
            if (!status) {
                status = Trace_Setup();
            }
            if (!status) {
                status = ETB_Flush(); // Flush to ensure no leftovers on ATB
            }
        } else {
            status = Trace_Setup();
        }
        if (status) {
            OutError(status);
            ReInitInProgress = 0; // 02.04.2019
            return (status);
        }
#if DBGCM_V8M
        if (IsV8M()) {
            if (TraceConf.Opt & TRACE_PC_DATA) {
                vp = DWTv8_DADDR_VAL_W | DWTv8_TRACE0;
                vn = DWTv8_DADDR_VAL_W | DWTv8_TRACE1;
            } else {
                vp = DWTv8_DADDR_VAL_W | DWTv8_TRACE1;
                vn = DWTv8_DADDR_VAL_W | DWTv8_TRACE0;
            }
            for (n = 0; n < NumWP; n++) {
                if ((RegDWT.CMP[n].FUNC & DWTv8_FUNCTION_CFG) == vp) {
                    RegDWT.CMP[n].FUNC = (RegDWT.CMP[n].FUNC & ~DWTv8_FUNCTION_CFG) | vn;
                    status             = WriteD32(DWT_FUNC + (n << 4), RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
                    if (status) {
                        OutError(status);
                        ReInitInProgress = 0; // 02.04.2019
                        return (status);
                    }
                }
            }
        } else {
            if (TraceConf.Opt & TRACE_PC_DATA) {
                vp = DWT_ITM_F_R_W ? DWT_ITM_DW : DWT_ITM_DRW;
                vn = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
            } else {
                vp = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
                vn = DWT_ITM_F_R_W ? DWT_ITM_DW : DWT_ITM_DRW;
            }
            for (n = 0; n < NumWP; n++) {
                if (RegDWT.CMP[n].FUNC == vp) {
                    RegDWT.CMP[n].FUNC = vn;
                    status             = WriteD32(DWT_FUNC + (n << 4), RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
                    if (status) {
                        OutError(status);
                        ReInitInProgress = 0; // 02.04.2019
                        return (status);
                    }
                }
            }
        }
#else  // DBGCM_V8M
        if (TraceConf.Opt & TRACE_PC_DATA) {
            vp = DWT_ITM_F_R_W ? DWT_ITM_DW : DWT_ITM_DRW;
            vn = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
        } else {
            vp = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
            vn = DWT_ITM_F_R_W ? DWT_ITM_DW : DWT_ITM_DRW;
        }
        for (n = 0; n < NumWP; n++) {
            if (RegDWT.CMP[n].FUNC == vp) {
                RegDWT.CMP[n].FUNC = vn;
                status = WriteD32(DWT_FUNC + (n << 4), RegDWT.CMP[n].FUNC);
                if (status) {
                    ReInitInProgress = 0; // 02.04.2019
                    OutError(status);
                    return (status);
                }
            }
        }
#endif // DBGCM_V8M
    }

    ReInitInProgress = 0; // 02.04.2019

    return (0); // say 'Ok.'
}


#if DBGCM_WITHOUT_STOP
int UnInitRunningTarget(void)
{
    int  status;
    BYTE n;

    if (!WriteBlock || !WriteD32) { // Did we make it far enough to init WriteBlock? (if aborting connection)
        return (0);
    }

    // Disable FPB to avoid Hard Fault after disconnect
    memset(&RegFPB, 0, sizeof(RegFPB));
    RegFPB.CTRL = FPB_KEY;

#if DBGCM_V8M
    status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
#else  // DBGCM_V8M
    status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), 0 /*attrib*/);
    if (status)
        return (status);
#endif // DBGCM_V8M

    // Disable DWT COMPS
    for (n = 0; n < NumWP; n++) {
        // if ((1 << n) & (MTrWP | MDtWP)) {
        if ((1 << n) & (MTrRP | MTrSP | MTrHP | MTrDP | MTrWP | MDtWP)) { // 27.01.2020: Tracepoint extensions
            RegDWT.CMP[n].FUNC = DWT_DISABLED;

#if DBGCM_V8M
            status = WriteD32(DWT_FUNC + (n * 16), RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
            if (status)
                return (status);
#else  // DBGCM_V8M
            status = WriteD32(DWT_FUNC + (n * 16), RegDWT.CMP[n].FUNC);
            if (status)
                return (status);
#endif // DBGCM_V8M
        }
    }
    MDtWP = 0;

    return (0);
}
#endif // DBGCM_WITHOUT_STOP


/*
 * Stop your target communication
 * Free all resources (dynamic memory, ...)
 */

void StopTarget(void)
{
    int status;

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        PDSCDebug_StopTarget(); // final shutdown
        PDSCDebug_UnInit();     // Cleanup
        return;
    }
    PDSCDebug_UnInit(); // Cleanup if initialized but not enabled
#endif                  // DBGCM_DBG_DESCRIPTION

    OutMsg("");

    FreeCache(); // free allocated cache memory.

#if DBGCM_WITHOUT_STOP
    status = UnInitRunningTarget();
    if (status)
        OutError(status);
#endif // DBGCM_WITHOUT_STOP

    status = Trace_UnInit();
    if (status)
        OutError(status);

    // 23.03.2020: Call ExitDebug here so that trace uninitialization can finish before clearing DEMCR.TRCENA
    if (pio && !pio->FlashLoad) { // Only for normal debug connections. Flash Download calls earlier because of "Reset & Run".
        ExitDebug();
    }

#if DBGCM_DS_MONITOR
    status = DSM_StopMonitor();
    if (status)
        OutError(status);
#endif // DBGCM_DS_MONITOR

    status = Debug_UnInit();
    if (status)
        OutError(status);

    //---TODO:
    // UnInit Target
    DEVELOP_MSG("Todo: \nUnInit Target");
}


/*
 * Reset your target
 */

DWORD ResetTarget(void)
{
    int   status;
    DWORD val;

#if DBGCM_DS_MONITOR
    int  res;
    BYTE suppressed  = 0;
    BYTE traceActive = 0;
#else  // DBGCM_DS_MONITOR
    DWORD n;
#endif // DBGCM_DS_MONITOR

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        status = PDSCDebug_ResetTarget();
        if (status) {
            OutError(status);
            return (1);
        }
        return (0);
    }
#endif // DBGCM_DBG_DESCRIPTION

    if (!iRun) {
        // Enable Reset Vector Catch
#if DBGCM_V8M
        status = WriteD32(DBG_EMCR, VC_CORERESET, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            goto end;
        }
        status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY); // Clear S_RESET_ST
        if (status) {
            OutError(status);
            goto end;
        }
#else  // DBGCM_V8M
        status = WriteD32(DBG_EMCR, VC_CORERESET);
        if (status) {
            OutError(status);
            goto end;
        }
        status = ReadD32(DBG_HCSR, &val); // Clear S_RESET_ST
        if (status) {
            OutError(status);
            goto end;
        }
#endif // DBGCM_V8M
    }

#if DBGCM_DS_MONITOR
    traceActive = DSMonitorThread.traceActive;
    status      = DSM_ConfigureMonitor(DSM_CFG_RESET_MASK,
                                       DSM_CFG_BEFORE_RESET,
                                       0, TRUE);
    if (status)
        goto end;
    suppressed = 1;

    DSM_ClearState(DSM_STATE_RST_DONE); // Clear Reset Done Flag to catch unexecuted resets (e.g. SYSRESETREQS '1' for non-secure debugger)
#endif                                  // DBGCM_DS_MONITOR

    switch (MonConf.Opt & RESET_TYPE) {
        case RESET_HW:
            //---TODO:
            // HW Chip Reset
            DEVELOP_MSG("Todo: \nHW Chip Reset");
            if (status)
                goto end;
            break;
        case RESET_SW_SYS:
            val = SYSRESETREQ;

#if DBGCM_V8M
            if (pio->bSecureExt) {
                status = WriteD32(NVIC_AIRCR, VECTKEY | val, pio->bSDbgEna ? BLOCK_SECTYPE_SECURE : BLOCK_SECTYPE_NSECURE);
            } else {
                status = WriteD32(NVIC_AIRCR, VECTKEY | val, BLOCK_SECTYPE_ANY);
            }
            if ((status == EU05) || (status == EU08) || (status == EU14))
                status = 0;
            if (status) {
                OutError(status);
                goto end;
            }
#else  // DBGCM_V8M
            status = WriteD32(NVIC_AIRCR, VECTKEY | val);
            if ((status == EU05) || (status == EU08) || (status == EU14))
                status = 0;
            if (status) {
                OutError(status);
                goto end;
            }
#endif // DBGCM_V8M
            break;
        case RESET_SW_VECT:

#if DBGCM_V8M
            if (!IsV8M()) {
                val    = VECTRESET;
                status = WriteD32(NVIC_AIRCR, VECTKEY | val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutError(status);
                    goto end;
                }
            }
#else  // DBGCM_V8M
            val = VECTRESET;
            status = WriteD32(NVIC_AIRCR, VECTKEY | val);
            if (status) {
                OutError(status);
                goto end;
            }
#endif // DBGCM_V8M

            break;
        default:
            //---TODO:
            // Autodetect Reset Type (according to selected device) and execute it
            DEVELOP_MSG("Todo: \nAutodetect Reset Type (according to selected device) and execute it");
            break;
    }

        //if (MonConf.Opt & BOOT_RUN) {
        //---TODO:
        // Run Bootloader and stop
        //DEVELOP_MSG("Todo: \nRun Bootloader and stop");
        //}

        // Reset Recovery (Reset Pulse can be extended by External Circuits)
#if DBGCM_DS_MONITOR
    res = DSM_WaitForState(DSM_STATE_RST_DONE, DSM_STATE_RST_DONE, TRUE, 300);
    if (res >= DSM_WAIT_ERR) {
        status = EU01;
        goto end;
    }

    if (DSMonitorState & DSM_STATE_RST_DONE) {
        if (DSMonitorState & DSM_STATE_CPU_HALTED) {
            if (MonConf.Opt & RST_VECT_CATCH) { // Execute based on Debug Setup. Reset without "Stop After Reset" will have
                                                // DEMCR.VC_CORERESET set (and some special handlings won't).
                pCbFunc(AG_CB_EXECHOOK, "OnStopAfterReset");
            }
        }
    }
#if DBGCM_V8M
    else {
        // Could not detect a completed reset
        if (pio->bSecureExt && !pio->bSDbgEna) {
            // Possibly failed because of not having required access rights
            warntxtout(EU48); // Could not reset target with non-secure debugger
        }
    }
#endif // DBGCM_V8M

#endif // DBGCM_DS_MONITOR

    if (iRun)
        goto end;

#if DBGCM_DS_MONITOR
    // Check if Target is stopped
    res = DSM_WaitForState(DSM_STATE_CPU_HALTED, DSM_STATE_CPU_HALTED, TRUE, 250); // 50);  Incremented timeout, 50ms can be a bit short due to monitor thread overhead
    if (res >= DSM_WAIT_ERR) {
        status = EU01;
        goto end;
    }

    if (!(DSMonitorState & DSM_STATE_CPU_HALTED)) {
#if DBGCM_V8M
        SevereTargetError(SEV_ERR_STOP);
        goto end;
#else  // DBGCM_V8M
        SetPlayDead(&StopErr[0]);
        goto end;
#endif // DBGCM_V8M
    }

#else // DBGCM_DS_MONITOR
    n = GetTickCount();
    do {

#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            return (1);
        }
        if ((val & S_RESET_ST) == 0)
            break;
#else  // DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val);
        if (status) {
            OutError(status);
            return (1);
        }
        if ((val & S_RESET_ST) == 0)
            break;
#endif // DBGCM_V8M

    } while ((GetTickCount() - n) < 300);

    // Check if Target is stopped
    n = GetTickCount();
    do {

#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            return (1);
        }
        if (val & S_HALT)
            break;
#else  // DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val);
        if (status) {
            OutError(status);
            return (1);
        }
        if (val & S_HALT)
            break;
#endif // DBGCM_V8M

    } while ((GetTickCount() - n) < 50);

    if ((val & S_HALT) == 0) {

#if DBGCM_V8M
        SevereTargetError(SEV_ERR_STOP);
        return (1);
#else  // DBGCM_V8M
        SetPlayDead(&StopErr[0]);
        return (1);
#endif // DBGCM_V8M
    }
#endif // DBGCM_DS_MONITOR

#if DBGCM_V8M
    status = WriteD32(DBG_EMCR, TRCENA | MONCONF_RST_VECT_CATCH, BLOCK_SECTYPE_ANY); // Enable Trace
    if (status) {
        OutError(status);
        goto end;
    }
#else  // DBGCM_V8M
    status = WriteD32(DBG_EMCR, TRCENA | MONCONF_RST_VECT_CATCH); // Enable Trace
    if (status) {
        OutError(status);
        goto end;
    }
#endif // DBGCM_V8M

    Invalidate(); // Registers, Memory cache

    if (!pio->FlashLoad) {
        GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR
    }

end:
#if DBGCM_DS_MONITOR
    if (suppressed) {
        if (status) {
            DSM_ConfigureMonitor(DSM_CFG_RESET_MASK,
                                 DSM_CFG_AFTER_RESET | (traceActive ? DSM_CFG_TRACE_ACTIVE : 0),
                                 0, FALSE);
        } else {
            status = DSM_ConfigureMonitor(DSM_CFG_RESET_MASK,
                                          DSM_CFG_AFTER_RESET | (traceActive ? DSM_CFG_TRACE_ACTIVE : 0),
                                          0, FALSE);
        }
        suppressed = 0;
    }
#endif // DBGCM_DS_MONITOR
    if (status) {
        OutError(status);
        return (1);
    }


    return (0); // 0 = OK
}


/*
 * Stop execution of user program
 */

U32 StopExec(void)
{
    int status;

#if DBGCM_DS_MONITOR
    int res;
#else  // DBGCM_DS_MONITOR
    DWORD val, n;
#endif // DBGCM_DS_MONITOR

#if DBGCM_WITHOUT_STOP
    if (iRun && !SkipTargetStop()) { // if currently executing and not disconnect without stop
#else                                // DBGCM_WITHOUT_STOP
    if (iRun) { // if currently executing
#endif                               // DBGCM_WITHOUT_STOP

#if DBGCM_V8M
        // Stop Target
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            return (0);
        }
#else  // DBGCM_V8M \
       // Stop Target
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);
        if (status) {
            OutError(status);
            return (0);
        }
#endif // DBGCM_V8M

#if DBGCM_DS_MONITOR
        res = DSM_WaitForState(DSM_STATE_CPU_HALTED, DSM_STATE_CPU_HALTED, TRUE, 250); // 50);  Incremented timeout, 50ms can be a bit short due to monitor thread overhead
        if (res >= DSM_WAIT_ERR) {
            status = EU01;
            OutError(status);
            return (0);
        }

        if (!(DSMonitorState & DSM_STATE_CPU_HALTED)) {
#if DBGCM_V8M
            SevereTargetError(SEV_ERR_STOP);
#else  // DBGCM_V8M
            SetPlayDead(&StopErr[0]);
#endif // DBGCM_V8M

            return (0);
        }

#else // DBGCM_DS_MONITOR \
      // Check if Target is stopped
        n = GetTickCount();
        do {

#if DBGCM_V8M
            status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                return (0);
            }
            if (val & S_HALT)
                break;
#else  // DBGCM_V8M
            status = ReadD32(DBG_HCSR, &val);
            if (status) {
                OutError(status);
                return (0);
            }
            if (val & S_HALT)
                break;
#endif // DBGCM_V8M

        } while ((GetTickCount() - n) < 50);

        if ((val & S_HALT) == 0) {

#if DBGCM_V8M
            SevereTargetError(SEV_ERR_STOP);
#else  // DBGCM_V8M
            SetPlayDead(&StopErr[0]);
#endif // DBGCM_V8M

            return (0);
        }
#endif // DBGCM_DS_MONITOR

        RegUpToDate = 0; // Invalidate Registers

        GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR

        if (TraceOpt & PC_SAMPLE) {
            status = Trace_Read(0, FALSE);
            if (status) {
                OutError(status);
                return (0);
            }
        }
    }
    return (1); // 'Stopped'
}


/*
 * Execute a single instruction, return 1 if ok, else 0
 */

UL32 Step(void)
{
    int status;

#if DBGCM_DS_MONITOR
    int res;
#else  // DBGCM_DS_MONITOR
    DWORD val, n;
#endif // DBGCM_DS_MONITOR

#if DBGCM_V8M
    // Update authentication and security state in pio to indicate this to uVision.
    // Authentication signals may have been updated manually and some debug functionality may
    // need this information before attempting the step.
    status = UpdateDebugAuthentication();
    if (status) {
        OutError(status);
        return (0);
    }

    status = UpdateSecurityState();
    if (status) {
        OutError(status);
        return (0);
    }

    if (!DHCSR_MaskIntsStop) {
        // Set MASKINTS (Mask Interrupts) - also stops runaway target
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | C_MASKINTS, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            return (0);
        }
    }
#else  // DBGCM_V8M

    if (!DHCSR_MaskIntsStop) {
        // Set MASKINTS (Mask Interrupts) - also stops runaway target
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | C_MASKINTS);
        if (status) {
            OutError(status);
            return (0);
        }
    }
#endif // DBGCM_V8M

    // Execute Cross Trigger Interface Handshake for possibly asserted EDBGRQ
    status = CTI_RunStepProcessor();
    if (status) {
        OutError(status);
        return (0);
    }

    // Flush Trace Data
    if (TraceOpt & TRACE_ENABLE) {
        // ETB Activate
        if (ETB_Configured) {
            status = ETB_Activate();
        } else {
            status = Trace_Flush(TRUE);
        }
        if (status) {
            OutError(status);
            return (0);
        }

        SetTraceRunSynch(true, true);
    }

#if DBGCM_V8M
    // Step Target
    status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_STEP | C_MASKINTS, BLOCK_SECTYPE_ANY);
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, false);
        return (0);
    }
#else  // DBGCM_V8M
    // Step Target
    status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_STEP | C_MASKINTS);
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, false);
        return (0);
    }
#endif // DBGCM_V8M

    // Check if Target is stopped
#if DBGCM_DS_MONITOR
    res = DSM_WaitForState(DSM_STATE_CPU_HALTED, DSM_STATE_CPU_HALTED, TRUE, 250); // 50);  Incremented timeout, 50ms can be a bit short due to monitor thread overhead
    if (res >= DSM_WAIT_ERR) {
        OutError(EU01);
        SetTraceRunSynch(false, false);
        return (0);
    }

    if (!(DSMonitorState & DSM_STATE_CPU_HALTED)) {
#if DBGCM_V8M
        if (SevereTargetError(SEV_ERR_STOP)) {
            RunningCB(RUNNING_AFTER_STEP); // Extended v8-M step (non-secure debugger)
        }
#else  // DBGCM_V8M
        SetPlayDead(&StopErr[0]);
#endif // DBGCM_V8M

        SetTraceRunSynch(false, false); // No need to trigger window update, playing dead
        return (0);
    }
#else // DBGCM_DS_MONITOR
    n = GetTickCount();
    do {

#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return (0);
        }
        if (val & S_HALT)
            break;
#else  // DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return (0);
        }
        if (val & S_HALT)
            break;
#endif // DBGCM_V8M

    } while ((GetTickCount() - n) < 50);

    if ((val & S_HALT) == 0) {

#if DBGCM_V8M
        SevereTargetError(SEV_ERR_STOP);
#else  // DBGCM_V8M
        SetPlayDead(&StopErr[0]);
#endif // DBGCM_V8M

        SetTraceRunSynch(false, false); // No need to trigger window update, playing dead
        return (0);
    }
#endif // DBGCM_DS_MONITOR

#if DBGCM_V8M
    if (!DHCSR_MaskIntsStop) {
        // Clear MASKINTS (Mask Interrupts)
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return (0);
        }
    }

    status = UpdateDebugAuthentication();
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, true);
        return (0);
    }

    status = UpdateSecurityState();
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, true);
        return (0);
    }
#else  // DBGCM_V8M
    if (!DHCSR_MaskIntsStop) {
        // Clear MASKINTS (Mask Interrupts)
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return (0);
        }
    }
#endif // DBGCM_V8M

    // Read Trace Data
    if (TraceOpt & TRACE_ENABLE) {
        if (TraceOpt & (TRACE_TIMESTAMP | TRACE_PCSAMPLE)) {
            status = UpdateCycles();
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, true);
                return (0);
            }
        }
        if (ETB_Active) {
            status = ETB_Process();
        } else {
            status = Trace_Read(1, FALSE);
        }
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return (0);
        }
    }

    RegUpToDate = 0; // Invalidate Registers

    GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR

    if (TraceOpt & PC_SAMPLE) {
        status = Trace_Read(0, FALSE);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return (0);
        }
    }

    SetTraceRunSynch(false, true);

    return (1);
}



#if !DBGCM_DS_MONITOR     // Periodic update moved into DSMonitor module
#define PERIODIC_TIME 500 // 500 mSec

static void DoPeriodicUpdate(void)
{
    DWORD updates = VW_MEMORY | VW_WATCHWINDOW | VW_LOGIC_ANALYZER | VW_RTXVIEW | VW_PERIDIALOGS | VW_AGDIDIALOGS | VW_COMPONENTS;
#if DBGCM_FEATURE_PAPROF
    updates |= VW_PERF_ANALYZER;
#endif // DBGCM_FEATURE_PAPROF

    if (GoMode && pio->periodic) { // if executing and periodic active
                                   // Update memory & watch window if activated...
#if DBGCM_V8M
        UpdateSecurityState();       // Read current security state from target
        UpdateDebugAuthentication(); // Read debug authentication status from target
#endif                               // DBGCM_V8M
        UpdateCycles();
        UpdateAllDlg();
        pio->Notify(UV_UPDATE_SELECTIVE, (void *)updates);
    }
}
#endif // !DBGCM_DS_MONITOR


/*
 * Start execution.  Stop when a Bp is reached
 */

void GoCmd(BOOL setRun)
{
    int   status;
    DWORD val;

#if DBGCM_WITHOUT_STOP
    DWORD dhcsr, dfsr;
#endif // DBGCM_WITHOUT_STOP

#if !DBGCM_DS_MONITOR
    DWORD cnt;
    DWORD dwFireTick, dwTicks;
#endif // !DBGCM_DS_MONITOR

#if DBGCM_V8M
    // Update authentication and security state in pio to indicate this to uVision.
    // Authentication signals may have been updated manually and some debug functionality may
    // need this information before attempting the step.
    status = UpdateDebugAuthentication();
    if (status) {
        OutError(status);
        return;
    }

    status = UpdateSecurityState();
    if (status) {
        OutError(status);
        return;
    }

    // Clear Debug Event (Debug Fault Status Register)
    status = WriteD32(NVIC_DFSR, HALTED | BKPT | DWTTRAP | VCATCH | EXTERNAL, BLOCK_SECTYPE_ANY);
    if (status) {
        OutError(status);
        if (!pio->DbgRecovery)
            return;
    }
#else  // DBGCM_V8M

    // Clear Debug Event (Debug Fault Status Register)
    status = WriteD32(NVIC_DFSR, HALTED | BKPT | DWTTRAP | VCATCH | EXTERNAL);
    if (status) {
        OutError(status);
        if (!pio->DbgRecovery)
            return;
    }
#endif // DBGCM_V8M

    // Execute Cross Trigger Interface Handshake for possibly asserted EDBGRQ
    status = CTI_RunStepProcessor();
    if (status)
        OutError(status);

    // Trace Start
    if (TraceOpt & TRACE_ENABLE) {
        status = Trace_Flush(setRun);
        if (status) {
            OutError(status);
            if (!pio->DbgRecovery)
                return;
        }
        if (TraceOpt & (TRACE_TIMESTAMP | TRACE_PCSAMPLE)) {
            status = Trace_TSync(TRUE, setRun);
            if (status) {
                OutError(status);
                if (!pio->DbgRecovery)
                    return;
            }
        }
        if (TraceOpt & TRACE_TIMESTAMP) {
            status = Trace_Cycles(TRUE);
            if (status) {
                OutError(status);
                if (!pio->DbgRecovery)
                    return;
            }
        }

        if (ETB_Configured) {
            // ETB Activate
            status = ETB_Activate();
            if (status) {
                OutError(status);
                return;
            }
        }

        SetTraceRunSynch(true, true);
    }

    if (setRun) {
#if DBGCM_V8M
        if (DHCSR_MaskIntsStop) {
            // Clear MASKINTS (Mask Interrupts)
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                if (!pio->DbgRecovery)
                    return;
            }

            // Clear Debug Event again (Debug Fault Status Register)
            status = WriteD32(NVIC_DFSR, HALTED | BKPT | DWTTRAP | VCATCH | EXTERNAL, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                if (!pio->DbgRecovery)
                    return;
            }
        }

        // Run Target
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN, BLOCK_SECTYPE_ANY);
        if (!pio->DbgRecovery && status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        } // Target may go into low power mode
          // before sending acknowledgement
#else     // DBGCM_V8M

        if (DHCSR_MaskIntsStop) {
            // Clear MASKINTS (Mask Interrupts)
            status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                if (!pio->DbgRecovery)
                    return;
            }

            // Clear Debug Event again (Debug Fault Status Register)
            status = WriteD32(NVIC_DFSR, HALTED | BKPT | DWTTRAP | VCATCH | EXTERNAL);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                if (!pio->DbgRecovery)
                    return;
            }
        }

        // Run Target
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN);
        if (!pio->DbgRecovery && status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        } // Target may go into low power mode
          // before sending acknowledgement
#endif    // DBGCM_V8M
    }

#if DBGCM_DS_MONITOR
    // Set Interval and trigger read of DHCSR
    status = DSM_ConfigureMonitor((DSM_CFG_WAKEUP | DSM_CFG_UPDATE_INTERVAL), (DSM_CFG_WAKEUP | DSM_CFG_UPDATE_INTERVAL), 50, TRUE); // Sync for initial update of DSMonitorState
    if (status) {
        OutError(status);
        return;
    }

    if (TraceOpt & TRACE_ENABLE) {
        if (TraceOpt & (TRACE_TIMESTAMP | TRACE_PCSAMPLE)) {
            status = Trace_TSync(FALSE, setRun);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, true);
                if (!pio->DbgRecovery)
                    return;
            }
        }

        // Activate Trace Read in Device State Monitor
        status = DSM_ConfigureMonitor(DSM_CFG_TRACE_ACTIVE, DSM_CFG_TRACE_ACTIVE, 0, FALSE);
        if (status) {
            OutError(status);
            return;
        }
    }
#else  // DBGCM_DS_MONITOR
    if (TraceOpt & TRACE_ENABLE) {
        if (TraceOpt & (TRACE_TIMESTAMP | TRACE_PCSAMPLE)) {
            status = Trace_TSync(FALSE, setRun);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, true);
                if (!pio->DbgRecovery)
                    return;
            }
        }
    }
#endif // DBGCM_DS_MONITOR

#if 1

#if DBGCM_DS_MONITOR

#if DBGCM_RECOVERY
    // Wait until Target running (and Read Trace Data)
    do {
        if (StopRun)
            break;
        if (dbgAccLevel == DA_NORMAL) {
            Sleep(50);
        } else {
            // Sleep less during recovery phase to recover as soon as possible
            Sleep(5);
        }
    } while (!(DSMonitorState & DSM_STATE_CPU_HALTED));
#else  // DBGCM_RECOVERY
    // Wait until Target running (and Read Trace Data)
    do {
        if (StopRun)
            break;
        Sleep(50);
    } while (!(DSMonitorState & DSM_STATE_CPU_HALTED));
#endif // DBGCM_RECOVERY

#else // DBGCM_DS_MONITOR
    // Wait until Target running (and Read Trace Data)
    cnt        = 0;
    dwFireTick = GetTickCount() + PERIODIC_TIME;

    do {

#if DBGCM_V8M
        if (StopRun) {
            if (StopExec() || !NonSecureCantStop())
                break;   // Keep running mode if not able to stop and Non-Secure Debugger with secure CPU
            StopRun = 0; // Clear StopRun flag until next attempt
        }
#else  // DBGCM_V8M
        if (StopRun)
            break;
#endif // DBGCM_V8M

        if (TraceOpt & (TRACE_ENABLE || PC_SAMPLE)) {
            status = Trace_Read(50, TRUE);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, true);
                return;
            }
        } else {
            Sleep(50);
        }

#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return;
        }
#else  // DBGCM_V8M
        status = ReadD32(DBG_HCSR, &val);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return;
        }
#endif // DBGCM_V8M

        // Periodic update is called approximatly every PERIODIC_TIME ms
        dwTicks = GetTickCount();
        if ((dwFireTick - dwTicks) >= PERIODIC_TIME) { // Every PERIODIC_TIME ms (accounting for tick count looping)
            dwFireTick = dwTicks + PERIODIC_TIME;      // Set the next fire time
            DoPeriodicUpdate();
        }

    } while ((val & S_HALT) == 0);
#endif // DBGCM_DS_MONITOR


    if (StopRun)
        StopExec();

#if DBGCM_DS_MONITOR
    if (TraceOpt & TRACE_ENABLE) {
        // Deactivate Trace Read in Device State Monitor
        status = DSM_ConfigureMonitor(DSM_CFG_TRACE_ACTIVE, 0, 0, FALSE);
        if (status) {
            OutError(status);
        }
    }
#endif // DBGCM_DS_MONITOR

#if DBGCM_V8M
    status = UpdateDebugAuthentication();
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, false);
        return;
    }

    status = UpdateSecurityState();
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, false);
        return;
    }
#endif // DBGCM_V8M

#if DBGCM_WITHOUT_STOP
    if (!setRun) {
        // WaitUntil(), check if extended single step and remove C_STEP and C_MASKINTS if so

        // Read DHCSR to find out about C_STEP and C_MASKINTS
#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &dhcsr, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        }

        ExtendedStep = (dhcsr & C_STEP) ? 1 : 0;

        if (ExtendedStep || ((dhcsr & C_MASKINTS) && (!DHCSR_MaskIntsStop))) {
            // Read Debug Events (Debug Fault Status Register)
            status = ReadD32(NVIC_DFSR, &dfsr, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                return;
            }

            // C_STEP still set, or C_MASKINTS set and not required to be set while stopped (most CPUs except early Cortex-M7 revs)
            dhcsr &= ~(0xFFFF0000 | C_STEP | (DHCSR_MaskIntsStop ? 0 : C_MASKINTS));
            dhcsr |= DBGKEY;

            status = WriteD32(DBG_HCSR, dhcsr, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                return;
            } // No new data processed, no trace win update necessary

            // Clear HALTED Debug Event if not set before (Debug Fault Status Register)
            if ((dfsr & HALTED) == 0) {
                status = WriteD32(NVIC_DFSR, HALTED, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutError(status);
                    SetTraceRunSynch(false, false);
                    return;
                }
            }
        }
#else  // DBGCM_V8M
        status = ReadD32(DBG_HCSR, &dhcsr);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        }

        ExtendedStep = (dhcsr & C_STEP) ? 1 : 0;

        if (ExtendedStep || ((dhcsr & C_MASKINTS) && (!DHCSR_MaskIntsStop))) {
            // Read Debug Events (Debug Fault Status Register)
            status = ReadD32(NVIC_DFSR, &dfsr);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                return;
            }

            // C_STEP still set, or C_MASKINTS set and not required to be set while stopped (most CPUs except early Cortex-M7 revs)
            dhcsr &= ~(0xFFFF0000 | C_STEP | (DHCSR_MaskIntsStop ? 0 : C_MASKINTS));
            dhcsr |= DBGKEY;

            status = WriteD32(DBG_HCSR, dhcsr);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                return;
            } // No new data processed, no trace win update necessary

            // Clear HALTED Debug Event if not set before (Debug Fault Status Register)
            if ((dfsr & HALTED) == 0) {
                status = WriteD32(NVIC_DFSR, HALTED);
                if (status) {
                    OutError(status);
                    SetTraceRunSynch(false, false);
                    return;
                }
            }
        }
#endif // DBGCM_V8M

    } else if (DHCSR_MaskIntsStop && !SkipTargetStop()) {
#else  // DBGCM_WITHOUT_STOP
    if (DHCSR_MaskIntsStop) {
#endif // DBGCM_WITHOUT_STOP

#if DBGCM_V8M
        // Read Debug Events (Debug Fault Status Register)
        status = ReadD32(NVIC_DFSR, &val, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        }

        // Set MASKINTS (Mask Interrupts)
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | C_MASKINTS, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        }

        // Clear HALTED Debug Event if not set before (Debug Fault Status Register)
        if ((val & HALTED) == 0) {
            status = WriteD32(NVIC_DFSR, HALTED, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                return;
            }
        }
#else  // DBGCM_V8M \
       // Read Debug Events (Debug Fault Status Register)
        status = ReadD32(NVIC_DFSR, &val);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        }

        // Set MASKINTS (Mask Interrupts)
        status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | C_MASKINTS);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, false);
            return;
        }

        // Clear HALTED Debug Event if not set before (Debug Fault Status Register)
        if ((val & HALTED) == 0) {
            status = WriteD32(NVIC_DFSR, HALTED);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, false);
                return;
            }
        }
#endif // DBGCM_V8M
    }

    // Trace End
    if (TraceOpt & TRACE_ENABLE) {
        if (TraceOpt & TRACE_TIMESTAMP) {
            status = Trace_Cycles(FALSE);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, true);
                return;
            }
        }

        if (ETB_Active) {
            // ETB Process
            status = ETB_Process();
            if (status)
                OutError(status);
        } else {
            // Read remaining Trace Data
            status = Trace_Read(50, TRUE);
            if (status) {
                OutError(status);
                SetTraceRunSynch(false, true);
                return;
            }
        }
    }

#else

#if DBGCM_V8M
    do {
        if (StopRun) {
            StopExec();
            return;
        }
        Sleep(50);

        status = ReadD32(DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            return;
        }
    } while ((val & S_HALT) == 0);
#else  // DBGCM_V8M
    do {
        if (StopRun) {
            StopExec();
            return;
        }
        Sleep(50);
        status = ReadD32(DBG_HCSR, &val);
        if (status) {
            OutError(status);
            return;
        }
    } while ((val & S_HALT) == 0);
#endif // DBGCM_V8M

#endif

#if DBGCM_DS_MONITOR
    status = DSM_ConfigureMonitor(DSM_CFG_UPDATE_INTERVAL, DSM_CFG_UPDATE_INTERVAL, 250, FALSE);
    if (status) {
        OutError(status);
        SetTraceRunSynch(false, true);
        return;
    }
#endif // DBGCM_DS_MONITOR

    if (StopRun) {
        SetTraceRunSynch(false, true);
        return;
    }

    RegUpToDate = 0; // Invalidate Registers

#if DBGCM_WITHOUT_STOP
    if (!SkipTargetStop()) {
        GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR
    }
#else  // DBGCM_WITHOUT_STOP
    GetRegs((1ULL << nR15) | (1ULL << nPSR)); // Read PC & xPSR
#endif // DBGCM_WITHOUT_STOP

    if (TraceOpt & PC_SAMPLE) {
        status = Trace_Read(0, FALSE);
        if (status) {
            OutError(status);
            SetTraceRunSynch(false, true);
            return;
        }
    }

    if (TraceOpt & TRACE_ENABLE) {
        SetTraceRunSynch(false, true);
    }
}


/*
 * Exit Debug Mode
 */

void ExitDebug(void)
{
    int status;

    if (WriteD32 == NULL) { // 23.03.2020: We didn't make to initialize WriteD32 function pointer (early exit during connect)
        return;
    }

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        PDSCDebug_DebugContext = DBGCON_DISCONNECT;

#if DBGCM_RECOVERY
        status = PDSCDebug_RecoverySupportStop();
        if (status) {
            OutError(status);
        }
#endif // DBGCM_RECOVERY

        PDSCDebug_DebugCoreStop();
        return;
    }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_V8M
    status = WriteD32(DBG_HCSR, DBGKEY, BLOCK_SECTYPE_ANY);
    if (status) {
        OutError(status);
        return;
    }
    status = WriteD32(DBG_EMCR, 0, BLOCK_SECTYPE_ANY);
    if (status) {
        OutError(status);
        return;
    }
#else  // DBGCM_V8M
    status = WriteD32(DBG_HCSR, DBGKEY);
    if (status) {
        OutError(status);
        return;
    }
    status = WriteD32(DBG_EMCR, 0);
    if (status) {
        OutError(status);
        return;
    }
#endif // DBGCM_V8M
}


/*
 * Update Instruction Cycles
 */

int UpdateCycles(void)
{
    int   status;
    DWORD val;

#if DBGCM_V8M
    if (TraceCycDwt || (xxCPU == ARM_CM3) || (xxCPU == ARM_CM4) || (xxCPU == ARM_CM7) || (xxCPU == ARM_SC300)) {
        status = ReadD32(DWT_CYCCNT, &val, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
        RegARM.nCycles += val - RegDWT.CYCCNT;
        RegDWT.CYCCNT = val;
        pio->cycles   = RegARM.nCycles;
    } else {
        pio->cycles = RegARM.nCycles;
    }
#else  // DBGCM_V8M
    if ((xxCPU == ARM_CM3) || (xxCPU == ARM_CM4) || (xxCPU == ARM_CM7) || (xxCPU == ARM_SC300)) {
        status = ReadD32(DWT_CYCCNT, &val);
        if (status)
            return (status);
        RegARM.nCycles += val - RegDWT.CYCCNT;
        RegDWT.CYCCNT = val;
        pio->cycles = RegARM.nCycles;
    }
#endif // DBGCM_V8M

    return (0);
}


/*
 * Read registers out of target and fill the RegARM structure
 */

void GetRegs(U64 mask)
{
    int status;

    if ((RegUpToDate & mask) == mask)
        return; // already up to date

#if DBGCM_V8M
    status = GetARMRegs(&RegARM, &RegFPU, &RegV8MSE, mask);
    if (status) {
        OutError(status);
        SevereTargetError(SEV_ERR_ACCESS);
        return;
    }
#else  // DBGCM_V8M
    status = GetARMRegs(&RegARM, &RegFPU, mask);
    if (status) {
        OutError(status);
        SetPlayDead(&FatalAccErr[0]);
        return;
    }
#endif // DBGCM_V8M

    *pCURPC    = RegARM.PC; // let uVision2 know about PC...
    pio->Thumb = (RegARM.xPSR & T_Bit) ? 1 : 0;

    RegUpToDate |= mask; // mark regs as 'up to date'
}


/*
 * Write registers to target
 */

#if DBGCM_V8M
void SetRegs(RgARMCM *pR, RgARMFPU *pF, RgARMV8MSE *pS, U64 mask)
{
#else  // DBGCM_V8M
void SetRegs(RgARMCM *pR, RgARMFPU *pF, U64 mask)
{
#endif // DBGCM_V8M
    int status;

#if DBGCM_V8M
    status = SetARMRegs(pR, pF, pS, mask);
    if (status) {
        OutError(status);
        SevereTargetError(SEV_ERR_ACCESS);
        return;
    }
#else  // DBGCM_V8M
    status = SetARMRegs(pR, pF, mask);
    if (status) {
        OutError(status);
        SetPlayDead(&FatalAccErr[0]);
        return;
    }
#endif // DBGCM_V8M

    if (pR)
        RegARM = *pR;
    if (pF)
        RegFPU = *pF;
#if DBGCM_V8M
    if (pS)
        RegV8MSE = *pS;
#endif // DBGCM_V8M

    *pCURPC    = RegARM.PC; // let uVision2 know about PC...
    pio->Thumb = (RegARM.xPSR & T_Bit) ? 1 : 0;

    RegUpToDate = 0; // not all values written are valid
}



// Get Range Mask
DWORD GetRangeMask(DWORD n)
{
    DWORD i;

    for (i = 0; i < 32; i++) {
        if (n == (DWORD)(1 << i))
            break;
    }
    return (i);
}



/*
 * Logic-Analyzer Functions
 */


/*
 * Query if LA-Signal is acceptable
 */

#if DBGCM_V8M
U32 QueryLaSig_v8M(AGDI_LA *pLA)
{
    U32  nAdr;
    U32  nSize;
    U32  n, m, cmp;
    U32  numTrWP;
    bool skip;
    int  status;

    if (!(TraceConf.Opt & TRACE_ENABLE)) {
        strcpy(pLA->szError, "Trace is disabled!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    if (pio->FindVTR(pLA->szDname)) {
        strcpy(pLA->szError, "VTREGs not supported!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    nAdr  = (U32)pLA->nAdr;  // Memory-Address
    nSize = (U32)pLA->nSize; // Size in Bytes

    // Support native data types <= 32 bit only
    switch (nSize) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            strcpy(pLA->szError, "Requested size not supported!");
            return (AGDI_LA_NOTSUPPORTED);
    }

    if (nAdr & (nSize - 1)) {
        strcpy(pLA->szError, "Invalid alignment!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    // Transform into DATAVSIZE value
    nSize >>= 1;

    // Data Address With Value is supported for the first four comparators only
    numTrWP = (NumWP > 4) ? 4 : NumWP;

    // Search for existing watchpoint with same address
    for (n = 0; n < numTrWP; n++) {
        if (MTrWP & (1 << n)) {
            if ((RegDWT.CMP[n].COMP == nAdr)
                && ((RegDWT.CMP[n].FUNC & DWTv8_DATAVSIZE) == (nSize << DWTv8_DATAVSIZE_P))) {
                UseWP[n]++;
                return (AGDI_LA_OK);
            }
        }
    }

    // Search for free watchpoint - first pass:
    //   From higher to lower watchpoint index sparing linkable comparator pairs.
    skip = false;
    for (n = 0, m = 1 << (numTrWP - 1); n < numTrWP; n++, m >>= 1) {
        cmp = numTrWP - 1 - n;
        if (RegDWT.CMP[cmp].FUNC & (DWTv8_ID_LIMIT | DWTv8_ID_DVAL)) {
            // Linkable comparator
            skip = true;
        } else {
            if (!skip) {
                // if (((MTrWP | MDtWP /*| MTrRP | MTrSP | MTrHP | MTrDP*/) & m) == 0) {
                if (((MTrWP | MDtWP | MTrRP | MTrSP | MTrHP | MTrDP) & m) == 0) { // 27.01.2020: Tracepoint extensions
                    n = cmp;
                    break;
                }
            }
            skip = false;
        }
    }

    if (n >= numTrWP) {
        // Search for free watchpoint - second pass:
        //   From lower to higher watchpoint index, more likely that linkable
        //   comparator pairs or at the higher indexes; still should try to
        //   spare them (at least for Cortex-M23 and Cortex-M33).
        for (n = 0, m = 1; n < numTrWP; n++, m <<= 1) {
            // if (((MTrWP | MDtWP /*| MTrRP | MTrSP | MTrHP | MTrDP*/) & m) == 0) {
            if (((MTrWP | MDtWP | MTrRP | MTrSP | MTrHP | MTrDP) & m) == 0) { // 27.01.2020: Tracepoint extensions
                break;
            }
        }
    }

    if (n >= numTrWP) {
        strcpy(pLA->szError, "Too many watchpoints defined!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    RegDWT.CMP[n].COMP = nAdr;
    RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
    RegDWT.CMP[n].FUNC |= (TraceOpt & TRACE_PC_DATA) ? (DWTv8_DADDR_VAL_W | DWTv8_TRACE1 | (nSize << DWTv8_DATAVSIZE_P)) : // PC Trace Packet
                              (DWTv8_DADDR_VAL_W | DWTv8_TRACE0 | (nSize << DWTv8_DATAVSIZE_P));                           // Data Trace Packet

    status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, BLOCK_SECTYPE_ANY);
    if (status) {
        strcpy(pLA->szError, StatusText(status));
        return (AGDI_LA_NOTSUPPORTED);
    }

    MTrWP |= 1 << n;
    NTrWP++;

    return (AGDI_LA_OK); // LA-Signal accepted
}
#endif // DBGCM_V8M

U32 QueryLaSig(AGDI_LA *pLA)
{
    U32 nAdr;
    U32 nSize;
    U32 mask;
    U32 n, m;
    int status;

#if DBGCM_V8M
    if (IsV8M()) {
        return QueryLaSig_v8M(pLA);
    }
#endif // DBGCM_V8M

    if (!(TraceConf.Opt & TRACE_ENABLE)) {
        strcpy(pLA->szError, "Trace is disabled!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    if (pio->FindVTR(pLA->szDname)) {
        strcpy(pLA->szError, "VTREGs not supported!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    nAdr  = (U32)pLA->nAdr;  // Memory-Address
    nSize = (U32)pLA->nSize; // Size in Bytes
    mask  = GetRangeMask(nSize);

    if (mask > 2 /*AM_WP*/) {
        strcpy(pLA->szError, "Requested size not supported!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    if (nAdr & (nSize - 1)) {
        strcpy(pLA->szError, "Invalid alignment!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    // Search for existing watchpoint with same address
    for (n = 0; n < NumWP; n++) {
        if (MTrWP & (1 << n)) {
            if ((RegDWT.CMP[n].COMP == nAdr) && (RegDWT.CMP[n].MASK == mask)) {
                UseWP[n]++;
                return (AGDI_LA_OK);
            }
        }
    }

    // Search for free watchpoint (from higher to lower watchpoint index)
    for (n = 0, m = 1 << (NumWP - 1); n < NumWP; n++, m >>= 1) {
        // if (((MTrWP | MDtWP) & m) == 0) {
        if (((MTrWP | MDtWP | MTrRP | MTrSP | MTrHP | MTrDP) & m) == 0) { // 27.01.2020: Tracepoint extensions
            n = NumWP - 1 - n;
            break;
        }
    }

    if (m == 0) {
        strcpy(pLA->szError, "Too many watchpoints defined!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    RegDWT.CMP[n].COMP = nAdr;
    RegDWT.CMP[n].MASK = mask;
    RegDWT.CMP[n].FUNC = (TraceOpt & TRACE_PC_DATA) ? (DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC) : (DWT_ITM_F_R_W ? DWT_ITM_DW : DWT_ITM_DRW);

#if DBGCM_V8M
    status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, BLOCK_SECTYPE_ANY);
    if (status) {
        strcpy(pLA->szError, StatusText(status));
        return (AGDI_LA_NOTSUPPORTED);
    }
#else  // DBGCM_V8M
    status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, 0 /*attrib*/);
    if (status) {
        strcpy(pLA->szError, StatusText(status));
        return (AGDI_LA_NOTSUPPORTED);
    }
#endif // DBGCM_V8M

    MTrWP |= 1 << n;
    NTrWP++;

    return (AGDI_LA_OK); // LA-Signal accepted
}


/*
 * Given LA-Signal has been killed.
 */

#if DBGCM_V8M
U32 KilledLaSig_v8M(AGDI_LA *pLA)
{
    U32 nAdr;
    U32 nSize;
    U32 n;
    U32 numTrWP;
    int status;

    nAdr  = (U32)pLA->nAdr;  // Memory-Address
    nSize = (U32)pLA->nSize; // Size in Bytes

    // Data Address With Value is supported for the first four comparators only
    numTrWP = (NumWP > 4) ? 4 : NumWP;

    // Transform into DATAVSIZE value
    nSize >>= 1;

    for (n = 0; n < numTrWP; n++) {
        if (MTrWP & (1 << n)) {
            if ((RegDWT.CMP[n].COMP == nAdr)                                                // Address
                && ((RegDWT.CMP[n].FUNC & DWTv8_DATAVSIZE) == (nSize << DWTv8_DATAVSIZE_P)) // Data Size
                && ((RegDWT.CMP[n].FUNC & DWTv8_TRACE0) != 0))                              // DWTv8_TRACE0 or DWTv8_TRACE1 (ACTION 0b1x)
            {
                if (UseWP[n]) {
                    UseWP[n]--;
                    return (AGDI_LA_OK);
                }
                break;
            }
        }
    }

    if (n == NumWP) {
        strcpy(pLA->szError, "Watchpoint not found!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;

    status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
    if (status) {
        strcpy(pLA->szError, StatusText(status));
        return (AGDI_LA_NOTSUPPORTED);
    }

    MTrWP &= ~(1 << n);
    NTrWP--;

    return (AGDI_LA_OK);
}
#endif // DBGCM_V8M

U32 KilledLaSig(AGDI_LA *pLA)
{
    U32 nAdr;
    U32 nSize;
    U32 mask;
    U32 n;
    int status;

#if DBGCM_V8M
    if (IsV8M()) {
        return KilledLaSig_v8M(pLA);
    }
#endif // DBGCM_V8M

    nAdr  = (U32)pLA->nAdr;  // Memory-Address
    nSize = (U32)pLA->nSize; // Size in Bytes
    mask  = GetRangeMask(nSize);

    for (n = 0; n < NumWP; n++) {
        if (MTrWP & (1 << n)) {
            if ((RegDWT.CMP[n].COMP == nAdr) && (RegDWT.CMP[n].MASK == mask)) {
                if (UseWP[n]) {
                    UseWP[n]--;
                    return (AGDI_LA_OK);
                }
                break;
            }
        }
    }

    if (n == NumWP) {
        strcpy(pLA->szError, "Watchpoint not found!");
        return (AGDI_LA_NOTSUPPORTED);
    }

    RegDWT.CMP[n].FUNC = DWT_DISABLED;

#if DBGCM_V8M
    status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
    if (status) {
        strcpy(pLA->szError, StatusText(status));
        return (AGDI_LA_NOTSUPPORTED);
    }
#else  // DBGCM_V8M
    status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC);
    if (status) {
        strcpy(pLA->szError, StatusText(status));
        return (AGDI_LA_NOTSUPPORTED);
    }
#endif // DBGCM_V8M

    MTrWP &= ~(1 << n);
    NTrWP--;

    return (AGDI_LA_OK);
}


/*
 * Send a Data-Record to uVision
 *  uVision will look for the matching LA-Signal based on the write-address
 *  and assign the data record to that La-signal.
 */

U32 SendLaDataRecord(AGDI_LAREC *pLA)
{
    U32 nE;

    nE = pCbFunc(AG_CB_LAREC_DATA, (void *)pLA); // send data to uVision
    switch (nE) {
        default:
        case 0: // could not find a matching Signal (pLA->nAdr)
            break;
        case AGDI_LA_OK:
            break;
    }
    return (nE);
}


/*
 *  Handle SW Breakpoint Configuration Request
 *
 */
U32 ConfigureSWBreaks(AG_SWBREAKCONF_REQ *pSBC)
{
    U32 nE = AG_SWBC_ERR_OK;

    if (pSBC == NULL) {
        return (AG_SWBC_ERR_INTERNAL); // Internal Error
    }

    switch (pSBC->nCmd) {
        case AG_SWBREAKCONF_LIST:
            nE = SwBreakConfGetHead(&(pSBC->pConf));
            break;
        case AG_SWBREAKCONF_ADD:
            nE = SwBreakConfAdd(pSBC->pConf);
            break;
        case AG_SWBREAKCONF_REM:
            nE = SwBreakConfRem(pSBC->pConf); // Unknown Command
            break;
        default:
            return (AG_SWBC_ERR_CMD);
    }

    return (nE);
}


/*
 *  Handle announcement of upcoming debug exit
 *
 */
U32 StartDbgExit(void)
{
    U32 nE = 0;

    if (DbgExitStarted) {
        return (0);
    }

    DbgExitStarted = 1;

#if DBGCM_DS_MONITOR
    DSM_ConfigureMonitor(DSM_CFG_EXIT_MASK,
                         DSM_CFG_EXIT,
                         0, TRUE);
#endif // DBGCM_DS_MONITOR

    // 11.12.2018: Ensure that we do not execute PlayDead with a
    // message box now that we got notification from SARMxxx to
    // shut down.
    PlayDeadMsg        = NULL;
    PlayDeadDelayedMsg = 1;

    return (nE);
}


// 02.04.2019: Used by wrCoreClk and wrTraceClk
static void UpdateClockSWO(void)
{
    DWORD val;

    if (TraceConf.Opt & TRACE_ENABLE) {
        if (TraceConf.Protocol != TPIU_ETB) {
            if (TraceConf.Protocol != TPIU_TRACE_PORT) {
                // Autodetect SWO Prescaler
                if (TraceConf.SWV_Pre & 0x8000) {
                    for (val = 1; val <= 8192; val++) {
                        if (SWV_Check(TPIU_Clock / val) == 0) {
                            TraceConf.SWV_Pre = 0x8000 | (WORD)(val - 1);
                            break;
                        }
                    }
                }
            }
            Trace_ClkSync();
        }
    }
}

/*
 * Core Clock Watch
 */

void wrCoreClk(void)
{
    // DWORD val;

    if (iRun) {
        pCoreClk->v.ul = TraceConf.Clk;
        return;
    }
    TraceConf.Clk = pCoreClk->v.ul;
    if (TraceConf.Clk == 0)
        TraceConf.Clk = 1;

#if 0 // 02.04.2019: Refactored for reuse in wrTraceClk
  if (TraceConf.Opt & TRACE_ENABLE) {
    // Autodetect SWO Prescaler
    if (TraceConf.SWV_Pre & 0x8000) {
      for (val = 1; val <= 8192; val++) {
        if (SWV_Check(TraceConf.Clk / val) == 0) {
          TraceConf.SWV_Pre = 0x8000 | (WORD)(val - 1);
          break;
        }
      }
    }
    Trace_ClkSync();
  }
#endif

    if (TraceConf.Opt & TRACE_USE_CORECLK) {
        pTraceClk->v.ul = TraceConf.Clk; // Don't use putval, otherwise TraceConf.TPIU_Clk gets overwritten
        TPIU_Clock      = TraceConf.Clk;
        UpdateClockSWO();
    }
}

static WATCH wCoreClk = {
    NULL, NULL, wrCoreClk, (DWORD)&pCoreClk, ATRX_WRITE, 0xFFFF
};


/*
 * Register Clock to uVision (via CORE_CLK alias for CLOCK VTReg)
 */

void RegisterClock(void)
{
    DWORD   n;
    VTR    *pV;
    union v v;

    // Clear Clock Flag for any previously defined CLOCK VTR (from Target DLL)
    pV = pio->pVTA;
    for (n = 0; n < pio->nVT; ++n, ++pV) {
        if (pV->Clock)
            pV->Clock = 0;
    }

    // Register a new CLOCK VTReg and renamed it to CORE_CLK
    v.ul     = TraceConf.Clk;
    pCoreClk = pio->p_create("CLOCK", bt_ulong);
    if (pCoreClk) {
        pCoreClk->name = "CORE_CLK";
        pio->p_putval(pCoreClk, &v);
        pio->w_create(&wCoreClk);
    }
}


#if DBGCM_V8M
static int UpdateRegisterGroup(RGROUP *group)
{
    if (group == NULL) {
        return (1); // Invalid parameter (do error codes later)
    }

    if (group->desc != 0) {
        return (1); // Not a register group (do error codes later)
    }

    if (!pio->bSecureExt) {
        return (1); // Don't care if no security extensions
    }

    if (strncmp(group->name, "Secure", 6) == 0) {
        group->ShEx = pio->bCPUSecure ? (group->ShEx | 0x2) : (group->ShEx & (~0x2));
    } else if (strncmp(group->name, "Non-Secure", 10) == 0) {
        group->ShEx = pio->bCPUSecure ? (group->ShEx & (~0x2)) : (group->ShEx | 0x2);
    }

    return (1);
}
#endif // DBGCM_V8M



/*
 * 02.04.2019: Trace Clock Watch
 */

void wrTraceClk(void)
{
    if (iRun) {
        pTraceClk->v.ul = TPIU_Clock; // Use effective TPIU Clock for display
        return;
    }

    if (TraceConf.Opt & TRACE_USE_CORECLK) {
        pTraceClk->v.ul = TPIU_Clock; // Use effective TPIU Clock for display
        if (!ReInitInProgress) {
            warntxtout(EU49); // Read-only from command line in this case
        }
        return;
    }

    TraceConf.TPIU_Clk = pTraceClk->v.ul;
    if (TraceConf.TPIU_Clk == 0)
        TraceConf.TPIU_Clk = 1;
    TPIU_Clock = TraceConf.TPIU_Clk;
    UpdateClockSWO();
}

static WATCH wTraceClk = {
    NULL, NULL, wrTraceClk, (DWORD)&pTraceClk, ATRX_WRITE, 0xFFFF
};


/*
 * 02.04.2019: Register TPIU Clock to uVision (via TRACE_CLK)
 */

void RegisterTraceClock(void)
{
    union v v;

    // Register TRACE_CLK
    v.ul      = (TraceConf.Opt & TRACE_USE_CORECLK) ? TraceConf.Clk : TraceConf.TPIU_Clk;
    pTraceClk = pio->p_create("TRACE_CLK", bt_ulong);
    if (pTraceClk) {
        pTraceClk->name = "TRACE_CLK";
        pio->p_putval(pTraceClk, &v);
        pio->w_create(&wTraceClk);
    }
}



/*
 *----------- AGDI Basic Functions -----------------
 */


/*
 * AGDI-Init Function
 */

static BOOL bFirstCall = 1;

U32 _EXPO_ AG_Init(U16 nCode, void *vp)
{
    U32 nE;

    CheckCom(1);

    if (bFirstCall) {
        // pDyMenu = MenuUV3;
        pDyMenu    = MenuUV4;
        bFirstCall = 0;
    }

    nE = 0;
    switch (nCode & 0xFF00) {
        case AG_INITFEATURES:    // Initialize & start the target
            PlayDead        = 0; // clear some variables...
            PlayDeadMsg     = NULL;
            PlayDeadShut    = 0;
            PlayDeadRegular = 0;

            iBpCmd     = 0;
            GoUntilAdr = 0xFFFFFFFF;

            //---We don't have special features here, so all variables are cleared.
            //---uVision will query these features lateron.
            //---Note: the 'supp' structure is defined in 'AGDI.H'

            supp.MemAccR = 1; // memory-access while running
            supp.RegAccR = 0; // register-access while running
            supp.hTrace  = 0; // trace support

#if DBGCM_FEATURE_COVERAGE
            supp.hCover = 1; // code coverage support
#else                        // DBGCM_FEATURE_COVERAGE
            supp.hCover = 0;         // code coverage support
#endif                       // DBGCM_FEATURE_COVERAGE

#if DBGCM_FEATURE_PAPROF
            supp.hPaLyze = 1; // Performance-Analyzer support
#else                         // DBGCM_FEATURE_PAPROF
            supp.hPaLyze = 0;        // Performance-Analyzer support
#endif                        // DBGCM_FEATURE_PAPROF

            supp.hMemMap    = 0; // Memory-Map support
            supp.ResetR     = 1; // Reset while running support
            supp.LaSupp     = 1; // Logic-Analyzer support
            supp.ExtBrkSupp = 1; // BpSupport while executing
            supp.ExcTrcSupp = 1; // Support for Exception Trace Window
            // Initialize this feature to '1' only when requested by AG_GETFEATURE
            supp.DbgRecoverySupp = 0; // Target DLL supports debug recovery
            supp.SwBreakConfSupp = 1; // Target DLL supports SW Breakpoint Configuration
            // 27.01.2020: Tracepoint extensions
            supp.TpData = 1; // Tracepoint support: Trace Data/Access Point (TraceDataPoint/TraceAccessPoint commands)
#if DBGCM_FEATURE_ETM
            supp.TpInstrRunSusp = 1; // Tracepoint support: Instruction Trace Run/Suspend (TraceRun/TraceSuspend commands)
            supp.TpInstrHalt    = 1; // Tracepoint support: Instruction Trace Halt (TraceHalt command)
#else                                // DBGCM_FEATURE_ETM
            supp.TpInstrRunSusp = 0; // Tracepoint support: Instruction Trace Run/Suspend (TraceRun/TraceSuspend commands)
            supp.TpInstrHalt = 0;    // Tracepoint support: Instruction Trace Halt (TraceHalt command)
#endif                               // DBGCM_FEATURE_ETM

            //---Note: if InitTarget() fails, then set nE=1, else nE=0.
            //         if 1 is returned, then uVision will cancel using this driver

            FirstReset = 1;

            nCPU = -1;

            bootflag = 1;
            nE       = InitTarget(); // Initialize your target...
            bootflag = 0;
            if (nE == 0) { // initialization was Ok.
#if DBGCM_WITHOUT_STOP
                if ((MonConf.Opt & BOOT_RESET) || ((pio->FlashLoad) && !(MonConf.Opt & CONN_NO_STOP))) { // 11.04.2017: No reset if Flash Download without Stop connection
#else                                                                                                    // DBGCM_WITHOUT_STOP
                if ((MonConf.Opt & BOOT_RESET) || (pio->FlashLoad)) {
#endif                                                                                                   // DBGCM_WITHOUT_STOP \
    // Perform Reset
                    nE = ResetTarget();                                                                  // First Reset

#if DBGCM_V8M
                    if (nE) {
                        SevereTargetError(SEV_ERR_RESET);
                        break;
                    }
#else  // DBGCM_V8M
                    if (nE) {
                        SetPlayDead(&FatalRstErr[0]);
                        break;
                    }
#endif // DBGCM_V8M
                }
                FirstReset = 0;
                if (!pio->FlashLoad) {
                    InitRegs(); // define register layout for RegWindow
                }

#if DBGCM_WITHOUT_STOP

#if DBGCM_V8M
                if (NonSecureCantStop() || (MonConf.Opt & CONN_NO_STOP)) {
                    nE = InitRunningTarget(); // Initialization of items skipped in InitTarget() + notification to SARMCM3
                }
#else  // DBGCM_V8M
                if (MonConf.Opt & CONN_NO_STOP) {
                    nE = InitRunningTarget(); // Initialization of items skipped in InitTarget() + notification to SARMCM3
                }
#endif // DBGCM_V8M

#endif // DBGCM_WITHOUT_STOP

            } else {
                StopTarget(); // shutdown target & communication
            }

            if (nE == 0) {
                if (!pio->FlashLoad) {
                    RegisterClock();
                    RegisterTraceClock(); // 02.04.2019: Separate Trace Clock setting
                }
                OpenAllDlg();
            }
            break;

        case AG_INITITEM: // init item
            switch (nCode & 0x00FF) {
                case AG_INITMENU: // init extension menu
                    *((DYMENU **)vp) = (DYMENU *)pDyMenu;
                    break;
                case AG_INITEXTDLGUPD: // init modeless extesion dlg update function
                    *((UC8 **)vp) = (UC8 *)UpdateAllDlg;
                    break;
                case AG_INITMHANDLEP: // setup ptr to HWND of active modeless dlg
                    pHwnd = (HWND *)vp;
                    break;
                case AG_INITPHANDLEP: // pointer to parent handle (MainFrame)
                    hMfrm = (HWND)vp;
                    break;
                case AG_INITINSTHANDLE: // pointer to Agdi-instance handle
                    hInst = (HMODULE)vp;
                    break;
                case AG_INITBPHEAD: // pointer to head of Bp-List
                    pBhead = (AG_BP **)vp;
                    break;
                case AG_INITCURPC: // pointer to program counter
                    pCURPC = (UL32 *)vp;
                    break;
                case AG_INITDOEVENTS: // DoEvents function pointer
                                      //--- this is no longer relevant due to uVision's threading.
                    break;
                case AG_INITUSRMSG:     // Registered Message for SendMessage
                    Uv2Msg = (DWORD)vp; // (Serial-Window, TextOut messages)
                    break;
                case AG_INITCALLBACK:   // pointer to callback function
                    pCbFunc = (pCBF)vp; // call-back function of sarm
                    if (!pio)
                        pio = (struct bom *)pCbFunc(AG_CB_GETBOMPTR, NULL); // get ioc
                    break;
                case AG_INITFLASHLOAD:   // Prepare for Flash Download
                    nE = InitDevFlash(); // Initialize Flash, Unlock
                    break;
                case AG_STARTFLASHLOAD: // SARM says 'Ready for Flash DownLoad'
                    nE = FlashLoad();   // Flash-Download: Erase, Write, Verify
                    break;
                case AG_INITSTARTLOAD:
                    if (vp != NULL) {
                        lParms = *((LOADPARMS *)vp);
                        // lParms.szFile[]    : full path name of App to load
                        // lParms.Incremental : 1:= incremental load
                        // lParms.NoCode      : 1:= load debug info only, no code
                    }

#if DBGCM_DBG_DESCRIPTION
                    if (PDSCDebug_IsEnabled()) {
                        PDSCDebug_DebugContext = DBGCON_VERIFY_CODE;

                        nE = PDSCDebug_DebugCodeMemRemap();
                        if (nE)
                            OutErrorMessage(nE);

#if DBGCM_RECOVERY
                        nE = DebugAccessEnsure();
                        if (nE)
                            OutErrorMessage(nE);
#endif // DBGCM_RECOVERY

                        break;
                    }
#endif // DBGCM_DBG_DESCRIPTION

                    break;
                case AG_INITENDLOAD: // Load is now completed.
                    break;
            }
            break;

        case AG_GETFEATURE: // uVision2 want's details about features...
            switch (nCode & 0x00FF) {
                case AG_F_MEMACCR: nE = supp.MemAccR; break;
                case AG_F_REGACCR: nE = supp.RegAccR; break;
                case AG_F_TRACE: nE = supp.hTrace; break;
                case AG_F_COVERAGE: nE = supp.hCover; break;
                case AG_F_PALYZE: nE = supp.hPaLyze; break;
                case AG_F_MEMMAP: nE = supp.hMemMap; break;
                case AG_F_RESETR: nE = supp.ResetR; break;
                case AG_F_LANALYZER: nE = supp.LaSupp; break;
                case AG_F_EXTBRKSUPP: nE = supp.ExtBrkSupp; break;
                case AG_F_EXCTRC: nE = supp.ExcTrcSupp; break;

#if DBGCM_RECOVERY
                // Initialize supp.DbgRecoverySupp only when feature is requested, we need a handshake to determine support in debugger
                case AG_F_DBGRECOVERY: nE = supp.DbgRecoverySupp = 1; break; // Target DLL supports debug recovery
#else                                                                        // DBGCM_RECOVERY
                case AG_F_DBGRECOVERY: nE = supp.DbgRecoverySupp; break; // Target DLL does not support debug recovery
#endif                                                                       // DBGCM_RECOVERY

                case AG_F_SWBREAKCONF: nE = supp.SwBreakConfSupp; break;

                // 27.01.2020: Tracepoint extensions
                case AG_F_TP_DATA: nE = supp.TpData; break;
                case AG_F_TP_INSTR_RUNSUSP: nE = supp.TpInstrRunSusp; break;
                case AG_F_TP_INSTR_HALT: nE = supp.TpInstrHalt; break;
            }
            break;

        case AG_EXECITEM: // execute various commands
            switch (nCode & 0x00FF) {
                case AG_UNINIT: // Clean up target system settings
                                //        if (PlayDead || hMfrm == NULL) break;  // target is disconnected
                                //        if (!pio->FlashLoad && !PlayDead) ExitDebug();  // 23.03.2020: Call ExitDebug() later as part of StopTarget() to ensure trace is disabled before core debug.
                    if (hMfrm)
                        CloseAllDlg();          // close all open dialogs
                                                // this is done below       StopTarget();  // shutdown target & communication
                    WriteParms(pdbg->TargArgs); // update argument string
                    PlayDeadRegular = 1;        // mark following PlayDead as regular shutdown
                    PlayDead        = 1;        // mark target as disconnected.
                    PlayDeadMsg     = NULL;     // don't show a message box
                    break;

                case AG_RESET: // perform a reset on the target system
                    if (PlayDead || hMfrm == NULL)
                        break;     // target is disconnected
                    ResetTarget(); // reset the target

                    //--removed:  /6.2.2004/
                    //        pCbFunc (AG_CB_EXECCMD, "U $\n");      // dasm $
                    break;

                case AG_RUNSTART: // Go or Step Starts
                    MultipleGoStep     = FALSE;
                    BkpInstructionSkip = TRUE;
                    break;

                case AG_RUNSTOP: // Go or Step Stops
                    Invalidate();
                    bNoCache = 0; // JR, 10.11.2016: Clear extended cache bypass after AG_GoStep exit
                    break;

                case AG_QUERY_LASIG: // LA-Signal acceptable ?
                    nE = QueryLaSig((AGDI_LA *)vp);
                    break;

                case AG_KILLED_LASIG: // LA-Signal was killed
                    nE = KilledLaSig((AGDI_LA *)vp);
                    break;

                case AG_EXCTRC:
                    nE = ExcTrc((excTrcData_t *)vp);
                    break;

#if DBGCM_FEATURE_PAPROF
                case AG_PROFILEINFO: // Get Profile Info
                    nE = PAProfInfo((AG_PROFINFO *)vp);
                    break;
#endif // DBGCM_FEATURE_PAPROF

#if DBGCM_RECOVERY
                case AG_RECOVERY_CONF: // Configure Debug Access Recovery
                    nE = ConfigureDebugAccessRecovery((AG_RECOVERY *)vp);
                    break;
#endif // DBGCM_RECOVERY

                case AG_SWBREAKCONF: // Configure SW Breakpoint Support for Memory Ranges
                    nE = ConfigureSWBreaks((AG_SWBREAKCONF_REQ *)vp);
                    break;

                case AG_OVERSTEP_START: // __breakpoint instruction: Debugger single step improvement
                case AG_STEPLINE_START:
                case AG_STEPINTO_START:
                    MultipleGoStep     = TRUE;
                    BkpInstructionSkip = TRUE;
                    break;

                case AG_START_DBGEXIT:   // Announce debug exit to give DLLs a chance to react accordingly
                    nE = StartDbgExit(); // to certain interface calls
                    break;

#if DBGCM_V8M
                case AG_REGTREE_UPDT_GRP: // Update properties of struct RGROUP
                    nE = UpdateRegisterGroup((RGROUP *)vp);
                    break;
#endif // DBGCM_V8M
            }
            break;
    }

    CheckCom(0);

    if (PlayDead) {     // driver is disconnected
        ExecPlayDead(); // show error message box and disconnect the target
    }

    return (nE);
}



/*
 * Memory Interface functions
 */

static DWORD ReadMem(DWORD nAdr, BYTE *pB, DWORD nMany)
{
    DWORD adr, n;
    BYTE  buf[RWBlock];
    int   status;

    MemErr = 0;

    if (ReadCache(nAdr, pB, nMany)) { // Use Cache if possible
        return (0);
    }

    if (!iRun && Opcode && (MonConf.Opt & CACHE_CODE)) {
        // Speed-up Opcode Read by requesting a Block and Write to Cache
        if ((nAdr & 0x00000003) == 0) {
            adr = nAdr;

#if DBGCM_V8M
            status = ReadARMMem(&adr, buf, RWBlock, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                MemErr = 1;
                return (adr);
            }
#else  // DBGCM_V8M
            status = ReadARMMem(&adr, buf, RWBlock);
            if (status) {
                OutError(status);
                MemErr = 1;
                return (adr);
            }
#endif // DBGCM_V8M

            WriteCache(nAdr, buf, RWBlock); // Write to Cache
        }
    }

    while (nMany) {
        n = (nMany > RWBlock) ? RWBlock : nMany;
        if (!ReadCache(nAdr, pB, n)) { // Check Cache Again
            adr = nAdr;

#if DBGCM_V8M
            status = ReadARMMem(&adr, pB, n, BLOCK_SECTYPE_ANY);
            if (status) {
                OutError(status);
                MemErr = 1;
                return (adr);
            }
#else  // DBGCM_V8M
            status = ReadARMMem(&adr, pB, n);
            if (status) {
                OutError(status);
                MemErr = 1;
                return (adr);
            }
#endif // DBGCM_V8M

            WriteCache(nAdr, pB, n); // Write to Cache
        }
        nAdr += n;
        pB += n;
        nMany -= n;
    }

    return (0);
}


static DWORD WriteMem(DWORD nAdr, BYTE *pB, DWORD nMany)
{
    DWORD adr, n;
    int   status;

    MemErr = 0;

    while (nMany) {
        n   = (nMany > RWBlock) ? RWBlock : nMany;
        adr = nAdr;

#if DBGCM_V8M
        status = WriteARMMem(&adr, pB, n, BLOCK_SECTYPE_ANY);
        if (status) {
            OutError(status);
            MemErr = 1;
            return (adr);
        }
#else  // DBGCM_V8M
        status = WriteARMMem(&adr, pB, n);
        if (status) {
            OutError(status);
            MemErr = 1;
            return (adr);
        }
#endif // DBGCM_V8M

        WriteCache(nAdr, pB, n); // Write to Cache
        nAdr += n;
        pB += n;
        nMany -= n;
    }
    return (0);
}


static DWORD VerifyMem(DWORD nAdr, BYTE *pB, DWORD nMany)
{
    BYTE  buf[RWBlock];
    char  msg[100];
    DWORD adr, n, i;
    int   status;

    MemErr = 0;

    while (nMany) {
        n   = (nMany > RWBlock) ? RWBlock : nMany;
        adr = nAdr;

#if DBGCM_V8M
        if (DP_Min) {
            status = ReadARMMem(&adr, buf, n, BLOCK_SECTYPE_ANY);
        } else {
            // Pushed Verify (Faster)
            memcpy(buf, pB, n);
            status = VerifyARMMem(&adr, buf, n, BLOCK_SECTYPE_ANY);
        }
        if (status) {
            OutError(status);
            MemErr = 1;
            return (adr);
        }
#else  // DBGCM_V8M
        if (DP_Min) {
            status = ReadARMMem(&adr, buf, n);
        } else {
            // Pushed Verify (Faster)
            memcpy(buf, pB, n);
            status = VerifyARMMem(&adr, buf, n);
        }
        if (status) {
            OutError(status);
            MemErr = 1;
            return (adr);
        }
#endif // DBGCM_V8M

        for (i = 0; i < n; i++) {
            if (buf[i] != pB[i]) {
                sprintf(msg, VerErr, nAdr + i, buf[i], pB[i]);
                AGDIMsgBox(hMfrm, msg, &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                MemErr = 1;
                return (nAdr + i);
            }
        }
        nAdr += n;
        pB += n;
        nMany -= n;
    }

    return (0);
}



/*
 * Access target memory
 */

U32 _EXPO_ AG_MemAcc(U16 nCode, UC8 *pB, GADR *pA, UL32 nMany)
{
    U16  nErr;
    UL32 n, m, o, a, s, e, x;
    int  i;

    if (iRun && !supp.MemAccR) { // currently running, can't access.
        return (AG_NOACCESS);    // error: can't access register
    }
    if (PlayDead) {     // driver disconnected.
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (AG_NOACCESS);
    }

    nErr = 0;
    if (hMfrm == NULL)
        return (AG_NOACCESS); // 05.11.2014: Return error code (early window updates in UV4 + opcode cache in SARMCM3)

    MemErr = 0; // clear previous error  /13.11.2009/
    CheckCom(1);

    switch (nCode) {
        case AG_READ: // need 'read' permission
            pA->ErrAdr = ReadMem(pA->Adr, pB, nMany);
            if (MemErr)
                nErr = AG_RDFAILED;
            break;

        case AG_WRITE: // need 'write' permission
            pA->ErrAdr = WriteMem(pA->Adr, pB, nMany);
            if (MemErr)
                nErr = AG_WRFAILED;
            break;
            // used for Program download
        case AG_WROPC: // need 'Read/Execute' permissions
            Opcode = 1;
            if (pio->Loading) {
                if ((MonConf.Opt & FLASH_LOAD) == 0) {
                    // Skip Download to Flash
                    o = 0;
                    a = pA->Adr;
                    n = nMany;
                    x = nMany;
                    while (n) {
                        for (i = 0; i < FlashConf.Nitems; i++) {
                            s = FlashConf.Dev[i].Start;
                            e = FlashConf.Dev[i].Size + s;
                            if ((a >= s) && (a < e)) {
                                m = e - a;
                                if (m > n)
                                    m = n;
                                o += m;
                                a += m;
                                n -= m;
                                x = n;
                                if (n == 0)
                                    break;
                                i = -1; // force another scan
                            } else if (a < s) {
                                m = s - a;
                                if (m > n)
                                    m = n;
                                if (m < x)
                                    x = m;
                            }
                        }
                        if (x) {
                            pA->ErrAdr = WriteMem(a, pB + o, x);
                            if (MemErr) {
                                nErr = AG_WRFAILED;
                                break;
                            }
                            o += x;
                            a += x;
                            n -= x;
                            x = n;
                        }
                    }
                    if (!MemErr && (MonConf.Opt & CODE_VERIFY)) {
                        pA->ErrAdr = VerifyMem(pA->Adr, pB, nMany);
                        if (MemErr)
                            nErr = AG_WRFAILED;
                    }

#if DBGCM_V8M
                    if (MemErr) {
                        SevereTargetError(SEV_ERR_ACCESS);
                    }
#else  // DBGCM_V8M
                    if (MemErr) {
                        SetPlayDead(&FatalAccErr[0]);
                    }
#endif // DBGCM_V8M

                } else {
                    pA->ErrAdr = WriteMem(pA->Adr, pB, nMany);
                    if (MemErr) {
                        nErr = AG_WRFAILED;
                    } else {
                        if (MonConf.Opt & CODE_VERIFY) {
                            pA->ErrAdr = VerifyMem(pA->Adr, pB, nMany);
                            if (MemErr)
                                nErr = AG_WRFAILED;
                        }
                    }
                }
            } else {
                pA->ErrAdr = WriteMem(pA->Adr, pB, nMany);
                if (MemErr)
                    nErr = AG_WRFAILED;
            }
            Opcode = 0;
            break;
            // used for disassembly etc.
        case AG_RDOPC: // need 'Read/Execute' permissions
            Opcode     = 1;
            pA->ErrAdr = ReadMem(pA->Adr, pB, nMany);
            if (MemErr)
                nErr = AG_RDFAILED;
            Opcode = 0;
            break;

        case AG_F_ERASE: // Erase Flash
            if (pA == NULL) {
                i = EraseFlash();
            }
            break;

        default:
            nErr = AG_INVALOP; // invalid Operation
            break;
    }

    CheckCom(0);

    if (PlayDead == 1) { // disconnected...
        ExecPlayDead();  // show error message box and disconnect the target
    }
    return (nErr);
}



/*
 * Read/Write Registers
 */

U32 _EXPO_ AG_AllReg(U16 nCode, void *vp)
{
    U16 nErr;

    if (iRun && !supp.RegAccR) { // currently running, can't access.
        return (AG_NOACCESS);    // error: can't access register
    }
    // if (PlayDead) return (AG_NOACCESS);
    if (PlayDead) {
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (AG_NOACCESS);
    }


    CheckCom(1);

    nErr = 0;
    switch (nCode) {  // which Opcode
        case AG_READ: // read register
            GetRegs(mREGS);
            memcpy(vp, &RegARM, sizeof(RgARMCM));
            break;

        case AG_WRITE:

#if DBGCM_V8M
            memcpy(&RegARM, vp, sizeof(RgARMCM));
            SetRegs(&RegARM, NULL, NULL, mREGS);
#else  // DBGCM_V8M
            memcpy(&RegARM, vp, sizeof(RgARMCM));
            SetRegs(&RegARM, NULL, mREGS);
#endif // DBGCM_V8M

            break;

        case AG_READFPU: // read FPU registers
            GetRegs(mRFPU);
            memcpy(vp, &RegFPU, sizeof(RgARMFPU));
            break;

        case AG_WRITEFPU: // write FPU registers

#if DBGCM_V8M
            memcpy(&RegFPU, vp, sizeof(RgARMFPU));
            SetRegs(NULL, &RegFPU, NULL, mRFPU);
#else  // DBGCM_V8M
            memcpy(&RegFPU, vp, sizeof(RgARMFPU));
            SetRegs(NULL, &RegFPU, mRFPU);
#endif // DBGCM_V8M

            break;

#if DBGCM_V8M
        case AG_READSEC: // read Security Extension registers
            GetRegs(mRSEC);
            memcpy(vp, &RegV8MSE, sizeof(RegV8MSE));
            break;

        case AG_WRITESEC: // write Security Extension registers
            memcpy(&RegV8MSE, vp, sizeof(RegV8MSE));
            SetRegs(NULL, NULL, &RegV8MSE, mRSEC);
            break;
#endif // DBGCM_V8M

        default:
            nErr = AG_INVALOP; // invalid Operation
            break;
    }
    CheckCom(0);

    if (PlayDead == 1) {
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (nErr);
}


/*
 * Read/Write a single Register
 */

U32 _EXPO_ AG_RegAcc(U16 nCode, U32 nReg, GVAL *pV)
{
    U16 nErr;
    U32 n;

    if (iRun && !supp.RegAccR) { // currently running, can't access.
        return (AG_NOACCESS);    // error: can't access register
    }
    // if (PlayDead) return (AG_NOACCESS);  // driver is disconnected.
    if (PlayDead) {
        ExecPlayDead();       // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (AG_NOACCESS); // driver is disconnected.
    }

    CheckCom(1);

    nErr = 0;
    switch (nCode) {  // which Opcode
        case AG_READ: // read register
            switch (nReg) {
                case nR0:
                case nR1:
                case nR2:
                case nR3:
                case nR4:
                case nR5:
                case nR6:
                case nR7:
                case nR8:
                case nR9:
                case nR10:
                case nR11:
                case nR12:
                case nR13:
                case nR14:
                case nR15:
                case nPSR:
                case nMSP:
                case nPSP:
                case nDSP:
                case nSYS:
                    GetRegs(1ULL << nReg);
                    pV->u32 = *((DWORD *)&RegARM + nReg);
                    break;
                case mPC: // current PC
                    GetRegs(1ULL << nR15);
                    pV->u32 = RegARM.Rn[15];
                    break;
                case nS0:
                case nS1:
                case nS2:
                case nS3:
                case nS4:
                case nS5:
                case nS6:
                case nS7:
                case nS8:
                case nS9:
                case nS10:
                case nS11:
                case nS12:
                case nS13:
                case nS14:
                case nS15:
                case nS16:
                case nS17:
                case nS18:
                case nS19:
                case nS20:
                case nS21:
                case nS22:
                case nS23:
                case nS24:
                case nS25:
                case nS26:
                case nS27:
                case nS28:
                case nS29:
                case nS30:
                case nS31:
                    n = nReg - nS0;
                    GetRegs(1ULL << (n + nFPUSx));
                    pV->u32 = RegFPU.Sn[n];
                    break;
                case nFPSCR:
                    GetRegs(1ULL << nFPSCR);
                    pV->u32 = RegFPU.FPSCR;
                    break;

#if DBGCM_V8M
                    // v8-M Security Extensions
                case nMSP_NS:
                case nPSP_NS:
                case nMSP_S:
                case nPSP_S:
                case nMSPLIM_S:
                case nPSPLIM_S:
                case nMSPLIM_NS:
                case nPSPLIM_NS:
                case nSYS_S:
                case nSYS_NS:
                    GetRegs(1ULL << nReg);
                    pV->u32 = *((DWORD *)&RegV8MSE + (nReg - nMSP_NS));
                    break;
#endif // DBGCM_V8M
            }
            break;

        case AG_WRITE:
            switch (nReg) {
                case nR0:
                case nR1:
                case nR2:
                case nR3:
                case nR4:
                case nR5:
                case nR6:
                case nR7:
                case nR8:
                case nR9:
                case nR10:
                case nR11:
                case nR12:
                case nR13:
                case nR14:
                case nR15:
                case nPSR:
                case nMSP:
                case nPSP:
                case nDSP:
                case nSYS:

#if DBGCM_V8M
                    *((DWORD *)&RegARM + nReg) = pV->u32;
                    SetRegs(&RegARM, NULL, NULL, 1ULL << nReg);
                    break;
#else  // DBGCM_V8M
                    *((DWORD *)&RegARM + nReg) = pV->u32;
                    SetRegs(&RegARM, NULL, 1ULL << nReg);
                    break;
#endif // DBGCM_V8M

                case mPC: // current PC
#if DBGCM_V8M
                    RegARM.Rn[15] = pV->u32;
                    SetRegs(&RegARM, NULL, NULL, 1ULL << nR15);
                    break;
#else  // DBGCM_V8M
                    RegARM.Rn[15] = pV->u32;
                    SetRegs(&RegARM, NULL, 1ULL << nR15);
                    break;
#endif // DBGCM_V8M

                case nS0:
                case nS1:
                case nS2:
                case nS3:
                case nS4:
                case nS5:
                case nS6:
                case nS7:
                case nS8:
                case nS9:
                case nS10:
                case nS11:
                case nS12:
                case nS13:
                case nS14:
                case nS15:
                case nS16:
                case nS17:
                case nS18:
                case nS19:
                case nS20:
                case nS21:
                case nS22:
                case nS23:
                case nS24:
                case nS25:
                case nS26:
                case nS27:
                case nS28:
                case nS29:
                case nS30:
                case nS31:

#if DBGCM_V8M
                    n            = nReg - nS0;
                    RegFPU.Sn[n] = pV->u32;
                    SetRegs(NULL, &RegFPU, NULL, 1ULL << (n + nFPUSx));
                    break;
#else  // DBGCM_V8M
                    n = nReg - nS0;
                    RegFPU.Sn[n] = pV->u32;
                    SetRegs(NULL, &RegFPU, 1ULL << (n + nFPUSx));
                    break;
#endif // DBGCM_V8M

                case nFPSCR:
#if DBGCM_V8M
                    RegFPU.FPSCR = pV->u32;
                    SetRegs(NULL, &RegFPU, NULL, 1ULL << nFPSCR);
                    break;
#else  // DBGCM_V8M

                    RegFPU.FPSCR = pV->u32;
                    SetRegs(NULL, &RegFPU, 1ULL << nFPSCR);
                    break;
#endif // DBGCM_V8M

#if DBGCM_V8M
                // v8-M Security Extensions
                case nMSP_NS:
                case nPSP_NS:
                case nMSP_S:
                case nPSP_S:
                case nMSPLIM_S:
                case nPSPLIM_S:
                case nMSPLIM_NS:
                case nPSPLIM_NS:
                case nSYS_S:
                case nSYS_NS:
                    *((DWORD *)&RegV8MSE + (nReg - nMSP_NS)) = pV->u32;
                    SetRegs(NULL, NULL, &RegV8MSE, 1ULL << nReg);
                    break;
#endif // DBGCM_V8M
            }
            break;

        default:
            nErr = AG_INVALOP; // invalid Operation
            break;
    }
    CheckCom(0);

    if (PlayDead == 1) {
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (nErr);
}



/*
 * Memory Attribute access
 */

U32 _EXPO_ AG_MemAtt(U16 nCode, UL32 nAttr, GADR *pA)
{
    U16 nErr;
#if DBGCM_FEATURE_COVERAGE
    U32 nAdr;
#endif // DBGCM_FEATURE_COVERAGE

    if (iRun && !supp.MemAccR) { // currently running, can't access.
        return (AG_NOACCESS);    // error: can't access memory
    }
    // if (PlayDead) return (AG_NOACCESS);
    if (PlayDead) {
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (AG_NOACCESS);
    }

    nErr = 0;
    switch (nCode) {
        case AG_MEMMAP: // map memory address range
            pA->ErrAdr = MMapMem(nAttr, pA->Adr, pA->nLen);
            break;

        case AG_GETMEMATT:                     // Attribute descriptor is requested
            pA->Adr = (UL32)MGetAttr(pA->Adr); // so return it in pA->Adr.
            break;

        case AG_SETMEMATT: // not used by uVision.
            break;

#if DBGCM_FEATURE_COVERAGE
        // Attribute segment required for coverage load/save:
        case AG_GETATTRSEG:                 // get attribute segment
            nAdr    = pA->Adr & 0xFFFF0000; // mask to segment
            pA->Adr = (UL32)MGetAttr(nAdr); // so return it in pA->Adr.
            break;
#endif // DBGCM_FEATURE_COVERAGE

#if (DBGCM_FEATURE_COVERAGE || DBGCM_FEATURE_PAPROF)
        case AG_CLEARCOVERAGE: // clear current coverage info
            ClearCoverage();
            break;
#endif // (DBGCM_FEATURE_COVERAGE || DBGCM_FEATURE_PAPROF)
    }
    if (PlayDead) {
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (nErr);
}



// Help functions for Breakpoints and Watchpoints
// - Watchpoint (R/W Access Breakpoint) with optional value match
//   BS [READ|WRITE] [address|varialbe] {==const}


// Extended Breakpoint Info
#define BP_INFO      ExtInf0
#define BP_ADDR      ExtInf1
#define BP_DATA      ExtInf2

#define BP_INFO_WVAL 0x00000001 // Watchpoint with value match


#if 0 // Replaced by CanValueWatch() in BreakResources module
/*
 *  CanValueWatch(): Returns TRUE if DWT has a value matcher, else FALSE
 */
__inline BOOL CanValueWatch() {
  switch (xxCPU) {
  case ARM_CM0:
  case ARM_CM0P:
  case ARM_CM1:
  case ARM_SC000:
    return FALSE;
  case ARM_CM3:
  case ARM_CM4:
  case ARM_CM7:
  case ARM_SC300:
  default:
    return TRUE;
  }
}
#endif


/*
 * check for some address
 */

int etree_addr(EXP *ep, DWORD *pA)
{
    int retVal = 0;

    switch (ep->etyp) {
        case e_addr: // SDMDK-4873: & (address of)
        case e_cast: // Recurse for address (e_usrcon / x_const)
        case e_ref:
            retVal = etree_addr(ep->L, pA);
            break;
        case x_const: // some constant address
        case e_usrcon:
        case e_indx:
            *pA    = ep->v.ul;
            retVal = 1;
            break;
        default:
            break;
    }
    return (retVal);
}


/*
 * check for some constant data
 */

int etree_data(EXP *ep, DWORD *pA)
{
    switch (ep->etyp) {
        case x_const:       // some constant data
            *pA = ep->v.ul; // funcname, label, constant etc.
            return (1);
        default:
            break;
    }
    return (0);
}


/*
 * Process L part of a 'expr_L &&/|| expr_R' expression  [TdB 13.03.2012] /PH: 3.3.2012/
 */
int etree_land_lor(AG_BP *pB, EXP *ep)
{
    int retVal;

    retVal = 0;
    if (ep)
        switch (ep->etyp) {
            case e_addr:    // SDMDK-4873: & (address of)
            case x_const:   // constant - an address
            case e_ref:     // <variable>
                retVal = 1; //
                break;
            case e_equ: // ==
                if (etree_addr(ep->L, &pB->BP_ADDR) && etree_data(ep->R, &pB->BP_DATA)) {
                    if (CanValueWatch()) {
                        retVal = 2;
                    } else {
                        retVal = 3;
                    }
                }
                break;
            case e_lequ: // <=
            case e_lt:   // <
            case e_gequ: // >=
            case e_gt:   // >
            case e_nequ: // !=      // added, 3.3.2012
                if (etree_addr(ep->L, &pB->BP_ADDR)) {
                    retVal = 3; //
                }
                break;
        }
    return (retVal);
}



int etree_data_watch(AG_BP *pB)
{
    int  retVal;
    EXP *ep;

    retVal = 0;
    ep     = (EXP *)pB->ep;
    if (ep)
        switch (ep->etyp) {
            case e_addr:  // SDMDK-4873: & (address of)
            case x_const: // constant - an address
            case e_ref:   // <variable>
                retVal = 1;
                break;
            case e_equ: // ==
                if (etree_addr(ep->L, &pB->BP_ADDR) && etree_data(ep->R, &pB->BP_DATA)) {
                    if (CanValueWatch()) {
                        retVal = 2;
                    } else {
                        retVal = 3;
                    }
                }
                break;
            case e_lequ: // <=
            case e_lt:   // <
            case e_gequ: // >=
            case e_gt:   // >
            case e_nequ: // != - added, 3.3.2012
                if (etree_addr(ep->L, &pB->BP_ADDR)) {
                    retVal = 3; //
                }
                break;
            case e_land:        // && - added 3.3.2012
                ep     = ep->L; // 'expr_L && expr_R'
                retVal = etree_land_lor(pB, ep);
                break;
            default: // others are invalid here
                     //  case e_lor:                     // ||
                break;
        }
    return (retVal);
}


// Setup Temporary BreakPoint and BreakPoint list
static void SetupTmpBrk(DWORD nA)
{
    TBp.type   = AG_ABREAK;
    TBp.rcount = 1;
    TBp.ocount = 1;
    if (nA != 0xFFFFFFFF) {
        TBp.enabled = 1;
        TBp.Adr     = nA;
    } else {
        TBp.enabled = 0;
    }
    TBp.next = *pBhead;
    pBStart  = &TBp;
    if (*pBhead) {
        (*pBhead)->prev = &TBp;
    }
}



/*******************************************************************************
  Function:      AnalyzeBreaks

  Parameter:     nA: Address of temporary BreakPoint (-1 for None)

  Return Value:  0 = OK
                 1 = Conditional Breakpoints not supported
                 2 = Too many Watchpoints defined
                 3 = Invalid Watchpoint Range
--------------------------------------------------------------------------------
  Description:   This function checks if all BrekPoints can be set
*******************************************************************************/

static int AnalyzeBreaks(DWORD nA)
{
    AG_BP *pB;
    DWORD  n;
    AG_BP *pB2; // 27.01.2020: For tracepoint extensions (complex condition)

    abreaks = 0; // Number of Code-Address BPs
    cbreaks = 0; // Number of Conditional BPs
    hbreaks = 0; // Number of HW Breakpoints
    wbreaks = 0; // Number of Watchpoint BPs
    vbreaks = 0; // Number of Watchpoint BPs with value match

#if DBGCM_V8M
    lbreaks   = 0; // Number of Watchpoint BPs with link capabilities
    limbreaks = 0; // Number of Watchpoint BPs with address limit capabilities
#endif             // DBGCM_V8M

    // Setup Temporary BreakPoint
    SetupTmpBrk(nA);
#if 0
  TBp.type   = AG_ABREAK;
  TBp.rcount = 1;
  TBp.ocount = 1;
  if (nA != 0xFFFFFFFF) {
    TBp.enabled = 1;
    TBp.Adr     = nA;
  } else {
    TBp.enabled = 0;
  }
  TBp.next = *pBhead;
  pBStart  = &TBp;
  if (*pBhead) {
    (*pBhead)->prev = &TBp;
  }
#endif

    // Scan all BreakPoints
    for (pB = pBStart; pB; pB = pB->next) {
        if (!pB->enabled)
            continue;
        switch (pB->type) {
            case AG_ABREAK:                            // Address BPs
                if ((pB->Adr == nA) && (pB != &TBp)) { // Equal to Temp BP ?
                    TBp.enabled = 0;                   // Disable Temp BP
                } else {
                    abreaks++; // Increase Address BPs Nr.
                    if ((FPB_Ver > 0) || pB->Adr < 0x20000000)
                        hbreaks++; // Increase HW Breakpoints Nr.
                }
                break;
            case AG_CBREAK: // Conditional BPs
                cbreaks++;  // Increase Cond. BPs Nr.
                break;
            case AG_WBREAK: // Watchpoint BPs

#if DBGCM_V8M
                if (IsV8M()) {
                    n = pB->tsize * pB->many;
                    switch (etree_data_watch(pB)) {
                        case 0: // Invalid Watchpoint
                            return (3);
                        case 1: // Address Watchpoint
                            break;
                        case 2: // Watchpoint with value match
                            if (n == 3 || n > 4)
                                return (3); // Can only handle 1, 2, and 4 byte value comparison
                            pB->BP_INFO |= BP_INFO_WVAL;
                            vbreaks++;
                            lbreaks++; // Value match comparator is always a link comparator
                            wbreaks++; // Additional Watchpoint used
                            break;
                        case 3: // Watchpoint without calced match  /3.3.2012/
                            if (n == 3 || n > 4)
                                return (3);               // Can only handle 1, 2, and 4 byte value comparison (even if realized in software)
                            pB->BP_INFO &= ~BP_INFO_WVAL; // no value
                                                          //wbreaks++;                         // Additional Watchpoint used, JR, 20.07.2012: This is not the case!
                            break;
                    }
                    switch (n) {
                        case 0:
                        case 1:
                        case 2:
                        case 4:
                            break;
                        default:         // Address range, unsupported sizes for value matches filtered out above
                            limbreaks++; // Requires address limit watchpoint
                            lbreaks++;
                            wbreaks++;
                            break;
                    }
                    wbreaks++; // Increase Watch BPs Nr.
                } else {
                    n = GetRangeMask(pB->tsize);
                    if ((n > AM_WP) || (pB->Adr & ((1 << n) - 1))) {
                        return (3);
                    }
                    switch (etree_data_watch(pB)) {
                        case 0: // Invalid Watchpoint
                            return (3);
                        case 1: // Address Watchpoint
                            break;
                        case 2: // Watchpoint with value match
                            if (n > 2)
                                return (3);
                            pB->BP_INFO |= BP_INFO_WVAL;
                            vbreaks++;
                            wbreaks++; // Additional Watchpoint used
                            break;
                        case 3: // Watchpoint without calced match  [TdB 13.03.2012] /PH: 3.3.2012/
                            if (n > 2)
                                return (3);
                            pB->BP_INFO &= ~BP_INFO_WVAL; // no value
                                                          //wbreaks++;                         // Additional Watchpoint used, 20.07.2012: Actually no additional Watchpoint used
                            break;
                    }
                    wbreaks++; // Increase Watch BPs Nr.
                }
                break;
#else  // DBGCM_V8M
                n = GetRangeMask(pB->tsize);
                if ((n > AM_WP) || (pB->Adr & ((1 << n) - 1))) {
                    return (3);
                }
                switch (etree_data_watch(pB)) {
                    case 0: // Invalid Watchpoint
                        return (3);
                    case 1: // Address Watchpoint
                        break;
                    case 2: // Watchpoint with value match
                        if (n > 2)
                            return (3);
                        pB->BP_INFO |= BP_INFO_WVAL;
                        vbreaks++;
                        wbreaks++; // Additional Watchpoint used
                        break;
                    case 3: // Watchpoint without calced match  [TdB 13.03.2012] /PH: 3.3.2012/
                        if (n > 2)
                            return (3);
                        pB->BP_INFO &= ~BP_INFO_WVAL; // no value
                        //wbreaks++;                         // Additional Watchpoint used, 20.07.2012: Actually no additional Watchpoint used
                        break;
                }
                wbreaks++; // Increase Watch BPs Nr.
                break;
#endif // DBGCM_V8M

            // 27.01.2020: Tracepoint extensions
            case AG_TBREAK:
                pB2 = pB;

                if ((TraceConf.Opt & TRACE_ENABLE) == 0) {
                    // Trace not enabled
                    return (8);
                }

                if (ETM_Addr == 0) {
                    if (pB->TP_TYPE != TP_TYPE_DATA && pB->TP_TYPE != TP_TYPE_ACC) {
                        // Requested tracepoint only supported with ETM, but ETM not present
                        return (10);
                    }
                }

                if ((TraceConf.Opt & ETM_TRACE) == 0) {
                    if (pB->TP_TYPE != TP_TYPE_DATA && pB->TP_TYPE != TP_TYPE_ACC) {
                        // Requested tracepoint only supported with ETM, but ETM disabled
                        return (9);
                    }
                }

                while (pB2) { // Loop to catch complex trigger condition
                    if (pB->BytObj) {
                        n = GetRangeMask(pB->tsize * pB->many);
                    } else {
                        n = GetRangeMask(pB->many); // needs to be many, this is the number of bytes
                    }

                    if ((n > AM_WP) || (pB->Adr & ((1 << n) - 1))) {
                        return (7);
                    }

                    wbreaks++;
                    if (pB->TP_INFO & TP_INFO_WVAL) {
                        vbreaks++;
                    }

                    pB2 = (AG_BP *)pB2->TP_EXT_PTR;
                }
                break;
        }
    }

    if ((hbreaks <= NumBP) && (hbreaks == abreaks)) {
        UseSWBP = 0;
    } else {
        UseSWBP = 1;
    }

    if (cbreaks) {
        return (1); // Conditional BPs not supported
    }

#if DBGCM_V8M
    if (IsV8M()) {
        if ((wbreaks > (UL32)(NumWP - NTrWP)) || (vbreaks > GetDWTValueComps()) || (lbreaks > GetDWTLinkComps())
            || (limbreaks > GetDWTLimitComps())) {
            return (2); // Too many WatchPoints defined
        }
    } else {
        if ((wbreaks > (UL32)(NumWP - NTrWP)) || (vbreaks > 1)) {
            return (2); // Too many WatchPoints defined
        }
    }
#else  // DBGCM_V8M
    if ((wbreaks > (UL32)(NumWP - NTrWP)) || (vbreaks > 1)) {
        return (2); // Too many WatchPoints defined
    }
#endif // DBGCM_V8M

    return (0); // OK
}


// 27.01.2020: Tracepoint extensions

/*
 *  Set Tracepoint usage flag
 */
static void SetUseFlagTP(DWORD nNum, DWORD nType)
{
    CntWP++;
    switch (nType) {
        case TP_TYPE_RUN: // TraceRun
            MTrRP |= 1 << nNum;
            NTrRP++;
            break;
        case TP_TYPE_SUSP: // TraceSuspend
            MTrSP |= 1 << nNum;
            NTrSP++;
            break;
        case TP_TYPE_HALT: // TraceHalt
            MTrHP |= 1 << nNum;
            NTrHP++;
            break;
        case TP_TYPE_DATA: // Trace Data   Point
        case TP_TYPE_ACC:  // Trace Access Point
            MTrDP |= 1 << nNum;
            NTrDP++;
            break;
    }
}

/*
 *  Clear Tracepoint usage flag
 */
static void ClearUseFlagTP(DWORD nNum, DWORD nType)
{
    CntWP--;
    switch (nType) {
        case TP_TYPE_RUN: // TraceRun
            if (NTrRP > 0) {
                MTrRP &= ~(1 << nNum);
                NTrRP--;
            }
            break;
        case TP_TYPE_SUSP: // TraceSuspend
            if (NTrSP > 0) {
                MTrSP &= ~(1 << nNum);
                NTrSP--;
            }
            break;
        case TP_TYPE_HALT: // TraceHalt
            if (NTrHP > 0) {
                MTrHP &= ~(1 << nNum);
                NTrHP--;
            }
            break;
        case TP_TYPE_DATA: // Trace Data   Point
        case TP_TYPE_ACC:  // Trace Access Point
            if (NTrDP > 0) {
                MTrDP &= ~(1 << nNum);
                NTrDP--;
            }
            break;
    }
}


#if DBGCM_V8M
/*
 *  Get required DWT function code based on Tracepoint type and
 *  its access type.
 */
static DWORD GetDWTFunc_v8M(DWORD nTPType, WORD nAccType)
{
    DWORD nFunc = DWTv8_DISABLED;

    if (nTPType == TP_TYPE_DATA) {
        // Trace Data Point, generate value + memory address
        // TODO: This resolves to two comparators now, review way of determining DWT function and comparator allocation
        //       Resolve to data value only for the time being
        switch (nAccType) {
            case TP_ACC_TYPE_RD: nFunc = (DWTv8_DADDR_VAL_R | DWTv8_TRACE0); break;  // Data + Addr Offset (Read)
            case TP_ACC_TYPE_WR: nFunc = (DWTv8_DADDR_VAL_W | DWTv8_TRACE0); break;  // Data + Addr Offset (Write)
            case TP_ACC_TYPE_RW: nFunc = (DWTv8_DADDR_VAL_RW | DWTv8_TRACE0); break; // Data + Addr Offset (Read/Write)
        }
    } else if (nTPType == TP_TYPE_ACC) {
        // Trace Access Point, generate value + PC which caused the access
        switch (nAccType) {
            case TP_ACC_TYPE_RD: nFunc = (DWTv8_DADDR_VAL_R | DWTv8_TRACE1); break;  // Data + PC (Read)
            case TP_ACC_TYPE_WR: nFunc = (DWTv8_DADDR_VAL_W | DWTv8_TRACE1); break;  // Data + PC (Write)
            case TP_ACC_TYPE_RW: nFunc = (DWTv8_DADDR_VAL_RW | DWTv8_TRACE1); break; // Data + PC (Read/Write)
        }
    } else {
        // Control of ETM logic, generate CMPMATCH signal
        switch (nAccType) {
            case TP_ACC_TYPE_CODE: nFunc = (DWTv8_IADDR | DWTv8_TRIGGER); break;  // CMPMATCH on PC
            case TP_ACC_TYPE_RD: nFunc = (DWTv8_DADDR_R | DWTv8_TRIGGER); break;  // CMPMATCH on Read
            case TP_ACC_TYPE_WR: nFunc = (DWTv8_DADDR_W | DWTv8_TRIGGER); break;  // CMPMATCH on Write
            case TP_ACC_TYPE_RW: nFunc = (DWTv8_DADDR_RW | DWTv8_TRIGGER); break; // CMPMATCH on Read/Write
        }
    }

    return nFunc;
}
#endif // DBGCM_V8M


/*
 *  Get required DWT function code based on Tracepoint type and
 *  its access type.
 */
static DWORD GetDWTFunc(DWORD nTPType, WORD nAccType)
{
    DWORD nFunc = DWT_DISABLED;

#if DBGCM_V8M
    if (IsV8M()) {
        return GetDWTFunc_v8M(nTPType, nAccType);
    }
#endif // DBGCM_V8M

    if (nTPType == TP_TYPE_DATA) {
        if (DWT_ITM_F_R_W) {
            // Trace Data Point, generate value + memory address
            switch (nAccType) {
                case TP_ACC_TYPE_RD: nFunc = DWT_EMITRANGE | DWT_ITM_DR_PC; break;  // Data + Addr Offset (Read)
                case TP_ACC_TYPE_WR: nFunc = DWT_EMITRANGE | DWT_ITM_DW_PC; break;  // Data + Addr Offset (Write)
                case TP_ACC_TYPE_RW: nFunc = DWT_EMITRANGE | DWT_ITM_DRW_PC; break; // Data + Addr Offset (Read/Write)
            }
        } else {
            // Target hardware cannot distinguish between reads and writes for trace packets
            nFunc = DWT_EMITRANGE | DWT_ITM_DRW_PC; // Data + Addr Offset (Read/Write)
        }
    } else if (nTPType == TP_TYPE_ACC) {
        if (DWT_ITM_F_R_W) {
            // Trace Access Point, generate value + PC which caused the access
            switch (nAccType) {
                case TP_ACC_TYPE_RD: nFunc = DWT_ITM_DR_PC; break;  // Data + PC (Read)
                case TP_ACC_TYPE_WR: nFunc = DWT_ITM_DW_PC; break;  // Data + PC (Write)
                case TP_ACC_TYPE_RW: nFunc = DWT_ITM_DRW_PC; break; // Data + PC (Read/Write)
            }
        } else {
            // Target hardware cannot distinguish between reads and writes for trace packets
            nFunc = DWT_ITM_DRW_PC; // Data + PC (Read/Write)
        }
    } else {
        // Control of ETM logic, generate CMPMATCH signal, all targets should be able to
        // distinguish between memory reads and writes for CMPMATCH regardless of DWT_ITM_F_R_W
        switch (nAccType) {
            case TP_ACC_TYPE_CODE: nFunc = DWT_ETM_PC; break; // CMPMATCH on PC
            case TP_ACC_TYPE_RD: nFunc = DWT_ETM_R; break;    // CMPMATCH on Read
            case TP_ACC_TYPE_WR: nFunc = DWT_ETM_W; break;    // CMPMATCH on Write
            case TP_ACC_TYPE_RW: nFunc = DWT_ETM_RW; break;   // CMPMATCH on Read/Write
        }
    }

    return nFunc;
}


#if DBGCM_V8M

/*******************************************************************************
  Function:      SetClrHwTP_v8M

  Parameter:     set:     1 - Set, 0 - Clear
                 pB:      Pointer to BreakPoint (NULL for All)
                 update : Update Target Flag

  Return Value:  0 = OK, Other = Error
--------------------------------------------------------------------------------
  Description:   This function sets or clears a Tracepoint (v8-M). It needs to be
                 separated from the SetClrHwBp function to allow a "clear
                 all" when passing NULL as pB.
                 Breakpoint's Opc[7] = HW WP Index, if using a data value
                                       match, the index belongs to the comparator
                                       used in addition to DWT.COMP[1].
*******************************************************************************/

static int SetClrHwTP_v8M(DWORD set, AG_BP *pB, DWORD update)
{
    DWORD n, /*m,*/ i, t, bytes;
    DWORD nTPType;
    BOOL  bHaltSet = (NTrHP > 0);
    int   status   = 0;

    // Set/clear all
    if (pB == NULL) {
        if (!set) { // set == 1 not supported
            // clear all
            for (n = 0; n < NumWP; n++) {
                if ((1 << n) & (MTrRP | MTrSP | MTrHP | MTrDP)) {
                    RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
                }
            }

            CntWP -= (NTrRP + NTrSP + NTrHP + NTrDP);
            NTrRP = NTrSP = NTrHP = NTrDP = 0;
            MTrRP = MTrSP = MTrHP = MTrDP = 0;
        }

        if (update) {
            if (NumWP) {
                status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), BLOCK_SECTYPE_ANY);
                if (status)
                    return (status);
            }
        }
        return (0);
    }

    // Check enabled trace resources
    if ((TraceConf.Opt & TRACE_ENABLE) == 0) {
        // Trace not enabled
        return (-1);
    }

    nTPType = pB->TP_TYPE;

    if ((TraceConf.Opt & ETM_TRACE) == 0 || ETM_Addr == 0) {
        if (nTPType != TP_TYPE_DATA && nTPType != TP_TYPE_ACC) {
            // Requested tracepoint only supported with ETM
            return (-1);
        }
    }

    if (set) {
        if (pB->BytObj) {
            bytes = pB->tsize * pB->many;
        } else {
            bytes = pB->many;
        }
        switch (bytes) {
            case 1:
            case 2:
            case 4:
                // single comparator
                if ((pB->Adr & (bytes - 1)) == 0) {
                    break;
                }
            default:
                // address range
                return (-1); // TODO: add support for address range tracepoint in v8-M, return error for the time being
                break;
        }

        // Specify DWT function by TracePoint type and general settings
        t = GetDWTFunc_v8M(nTPType, pB->acc);

        for (i = 0; i < NumWP; i++) {
            // TODO: Rework allocation strategy not optimized
            n = i;

            if ((RegDWT.CMP[n].FUNC & DWTv8_FUNCTION_CFG) == DWTv8_DISABLED) {
                // TODO: Data value matching not implemented yet for v8-M targets, programmer's model changed

                // Set the "main" DWT comparator
                RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
                RegDWT.CMP[n].FUNC |= t;
                RegDWT.CMP[n].COMP = pB->Adr;
                pB->Opc[7]         = (BYTE)n; // DWT number

                SetUseFlagTP(n, nTPType);
                if (pB->TP_INFO & TP_INFO_WVAL) {
                    SetUseFlagTP(1, nTPType);
                }

                if (update) {
                    status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, BLOCK_SECTYPE_ANY);
                    if (status)
                        return (status);
                }

                // Set/Clear the second condition for trace halt
                if (nTPType == TP_TYPE_HALT && pB->TP_EXT_PTR != NULL) {
                    status = SetClrHwTP_v8M(set, (AG_BP *)pB->TP_EXT_PTR, update);
                }

                return (status); // Done
            }
        }
    } else {
        if (CntWP == 0)
            return (-1); // Error
        n = pB->Opc[7];
        RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;

        ClearUseFlagTP(n, nTPType);

        if (update) {
            status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
            if (status)
                return (status);
        }

        // TODO: Add support for complex conditions (TraceHalt for ETM, (pB->TP_EXT_PTR != NULL))
        return 0;
    }
    return (-1); // Error
}
#endif // DBGCM_V8M


/*******************************************************************************
  Function:      SetClrHwTP

  Parameter:     set:     1 - Set, 0 - Clear
                 pB:      Pointer to BreakPoint (NULL for All)
                 update : Update Target Flag

  Return Value:  0 = OK, Other = Error
--------------------------------------------------------------------------------
  Description:   This function sets or clears a Tracepoint. It needs to be
                 separated from the SetClrHwBp function to allow a "clear
                 all" when passing NULL as pB.
                 Breakpoint's Opc[7] = HW WP Index, if using a data value
                                       match, the index belongs to the comparator
                                       used in addition to DWT.COMP[1].
*******************************************************************************/

static int SetClrHwTP(DWORD set, AG_BP *pB, DWORD update)
{
    DWORD n, m, i, t;
    DWORD nTPType;
    BOOL  bHaltSet = (NTrHP > 0);
    int   status   = 0;
    BYTE  mLinkWP; // Mask of linked address comparators

#if DBGCM_V8M
    if (IsV8M()) {
        return SetClrHwTP_v8M(set, pB, update);
    }
#endif // DBGCM_V8M

    // Set/clear all
    if (pB == NULL) {
        if (!set) { // set == 1 not supported
            // clear all
            for (n = 0; n < NumWP; n++) {
                if ((1 << n) & (MTrRP | MTrSP | MTrHP | MTrDP)) {
                    RegDWT.CMP[n].FUNC = DWT_DISABLED;
                }
            }

            CntWP -= (NTrRP + NTrSP + NTrHP + NTrDP);
            NTrRP = NTrSP = NTrHP = NTrDP = 0;
            MTrRP = MTrSP = MTrHP = MTrDP = 0;
        }

        if (update) {
            if (NumWP) {
                status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), 0 /*attrib*/);
                if (status)
                    return (status);
            }
        }
        return (0);
    }

    // Check enabled trace resources
    if ((TraceConf.Opt & TRACE_ENABLE) == 0) {
        // Trace not enabled
        return (-1);
    }

    nTPType = pB->TP_TYPE;

    if ((TraceConf.Opt & ETM_TRACE) == 0 || ETM_Addr == 0) {
        if (nTPType != TP_TYPE_DATA && nTPType != TP_TYPE_ACC) {
            // Requested tracepoint only supported with ETM
            return (-1);
        }
    }

    if (set) {
        // Get range mask
        if (pB->BytObj) {
            m = GetRangeMask(pB->tsize * pB->many); // range support
        } else {
            m = GetRangeMask(pB->many); // range support
        }
        if ((m > AM_WP) || (pB->Adr & ((1 << m) - 1))) {
            return (-1); // Range Error
        }

        // Specify DWT function by TracePoint type and general settings
        t = GetDWTFunc(nTPType, pB->acc);

        // Detect Linked Address Comparators
        mLinkWP = 0;
        for (i = 0; i < NumWP; i++) {
            if ((RegDWT.CMP[i].FUNC & DWT_DATAVMATCH)
                && (RegDWT.CMP[i].FUNC & DWT_FUNCTION) != DWT_DISABLED) {
                // Active Data Value Comparator
                n = (RegDWT.CMP[i].FUNC >> DWT_DATAVADDR0) & 0xF;
                if (n != i)
                    (mLinkWP = (1 << n));
                n = (RegDWT.CMP[i].FUNC >> DWT_DATAVADDR1) & 0xF; // Should be programmed to i or n anyway (even if not supported)
                if (n != i)
                    (mLinkWP = (1 << n));
            }
        }
        for (i = 0; i < NumWP; i++) {
            if (NumWP > 2) { // Use DWT CMP0,CMP1 as last
                n = i + 2;
                if (n >= NumWP)
                    n -= NumWP;
            } else {
                n = i;
            }
            if ((RegDWT.CMP[n].FUNC & DWT_FUNCTION) == DWT_DISABLED // Disabled and not used as linked comparator
                && !(mLinkWP & (1 << n))) {
                // Check if Data Value Comparator required & free
                if (pB->TP_INFO & TP_INFO_WVAL) {
                    if ((n == 1) || ((RegDWT.CMP[1].FUNC & DWT_FUNCTION) != DWT_DISABLED)) {
                        return (-1);
                    }
                }
                // Program Data Address Comparator
                RegDWT.CMP[n].COMP = pB->Adr;
                RegDWT.CMP[n].MASK = m;
                RegDWT.CMP[n].FUNC = (pB->TP_INFO & TP_INFO_WVAL) ? 0 : t;
                pB->Opc[7]         = (BYTE)n; // DWT number
                SetUseFlagTP(n, nTPType);
                if (pB->TP_INFO & TP_INFO_WVAL) {
                    SetUseFlagTP(1, nTPType);
                }
                if (update) {
                    status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, 0 /*attrib*/);
                    if (status)
                        return (status);
                }
                // Program Data Value Comparator if required
                if (pB->TP_INFO & TP_INFO_WVAL) {
                    // Set DWT.COMP[1] for data value matching
                    RegDWT.CMP[1].FUNC = t | (n << DWT_DATAVADDR0) | (n << DWT_DATAVADDR1);
                    switch (pB->tsize) {
                        default:
                        case 1:
                            RegDWT.CMP[1].COMP = (BYTE)(pB->TP_DATA & 0xFF);
                            RegDWT.CMP[1].FUNC |= DWT_DATAVMATCH | DWT_DATAVSIZEB;
                            break;
                        case 2:
                            RegDWT.CMP[1].COMP = (WORD)(pB->BP_DATA & 0xFFFF);
                            RegDWT.CMP[1].FUNC |= DWT_DATAVMATCH | DWT_DATAVSIZEH;
                            break;
                        case 4:
                            RegDWT.CMP[1].COMP = pB->BP_DATA;
                            RegDWT.CMP[1].FUNC |= DWT_DATAVMATCH | DWT_DATAVSIZEW;
                            break;
                    }
                    RegDWT.CMP[1].MASK = 0;
                    if (update) {
                        status = WriteBlock(DWT_CMP + 16 * 1, (BYTE *)&RegDWT.CMP[1], 3 * 4, 0 /*attrib*/);
                        if (status)
                            return (status);
                    }
                }

                // Set/Clear the second condition for trace halt
                if (nTPType == TP_TYPE_HALT && pB->TP_EXT_PTR != NULL) {
                    status = SetClrHwTP(set, (AG_BP *)pB->TP_EXT_PTR, update);
                }

                return (status); // Done
            }
        }
    } else {
        if (CntWP == 0)
            return (-1);                  // Error
        if (pB->TP_INFO & TP_INFO_WVAL) { // 17.09.2015, deactivate data value comparator first;
                                          // is the only "enabled" comparator when linking to
                                          // address comparator (see v7-M ARM).
            RegDWT.CMP[1].FUNC = DWT_DISABLED;
            ClearUseFlagTP(1, nTPType);
            if (update) {
#if DBGCM_V8M
                status = WriteD32(DWT_FUNC + 16 * 1, RegDWT.CMP[1].FUNC, BLOCK_SECTYPE_ANY);
                if (status)
                    return (status);
#else  // DBGCM_V8M
                status = WriteD32(DWT_FUNC + 16 * 1, RegDWT.CMP[1].FUNC);
                if (status)
                    return (status);
#endif // DBGCM_V8M
            }
        }
        n                  = pB->Opc[7];
        RegDWT.CMP[n].FUNC = DWT_DISABLED;
        ClearUseFlagTP(n, nTPType);
        if (update) {
#if DBGCM_V8M
            status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
            if (status)
                return (status);
#else  // DBGCM_V8M
            status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC);
            if (status)
                return (status);
#endif // DBGCM_V8M
        }

        if (pB->TP_EXT_PTR != NULL) {
            status = SetClrHwTP(set, (AG_BP *)(pB->TP_EXT_PTR), update);
            if (status)
                return (status);
        }

        return 0;
    }
    return (-1); // Error
}


#if DBGCM_V8M
static int ReadSwBPMem(DWORD *nAdr, UC8 *buf, DWORD nBytes, DWORD nAccSz)
{
    switch (nAccSz) {
        case 0:
            return ReadARMMem(nAdr, buf, nBytes, BLOCK_SECTYPE_ANY);
        case 1:
            return ReadARMMemD8(nAdr, buf, nBytes, BLOCK_SECTYPE_ANY);
        case 2:
            return ReadARMMemD16(nAdr, (U16 *)buf, nBytes >> 1, BLOCK_SECTYPE_ANY);
        case 4:
            return ReadARMMemD32(nAdr, (U32 *)buf, nBytes >> 2, BLOCK_SECTYPE_ANY);
    }
    return (EU21); // Unsupported Memory Access Size
}

static int WriteSwBPMem(DWORD *nAdr, UC8 *buf, DWORD nBytes, DWORD nAccSz)
{
    switch (nAccSz) {
        case 0:
            return WriteARMMem(nAdr, buf, nBytes, BLOCK_SECTYPE_ANY);
        case 1:
            return WriteARMMemD8(nAdr, buf, nBytes, BLOCK_SECTYPE_ANY);
        case 2:
            return WriteARMMemD16(nAdr, (U16 *)buf, nBytes >> 1, BLOCK_SECTYPE_ANY);
        case 4:
            return WriteARMMemD32(nAdr, (U32 *)buf, nBytes >> 2, BLOCK_SECTYPE_ANY);
    }
    return (EU21); // Unsupported Memory Access Size
}
#else  // DBGCM_V8M
static int ReadSwBPMem(DWORD *nAdr, UC8 *buf, DWORD nBytes, DWORD nAccSz)
{
    switch (nAccSz) {
        case 0:
            return ReadARMMem(nAdr, buf, nBytes);
        case 1:
            return ReadARMMemD8(nAdr, buf, nBytes, 0);
        case 2:
            return ReadARMMemD16(nAdr, (U16 *)buf, nBytes >> 1, 0);
        case 4:
            return ReadARMMemD32(nAdr, (U32 *)buf, nBytes >> 2, 0);
    }
    return (EU21); // Unsupported Memory Access Size
}

static int WriteSwBPMem(DWORD *nAdr, UC8 *buf, DWORD nBytes, DWORD nAccSz)
{
    switch (nAccSz) {
        case 0:
            return WriteARMMem(nAdr, buf, nBytes);
        case 1:
            return WriteARMMemD8(nAdr, buf, nBytes, 0);
        case 2:
            return WriteARMMemD16(nAdr, (U16 *)buf, nBytes >> 1, 0);
        case 4:
            return WriteARMMemD32(nAdr, (U32 *)buf, nBytes >> 2, 0);
    }
    return (EU21); // Unsupported Memory Access Size
}
#endif // DBGCM_V8M




int SetClrSwBP(DWORD set, AG_BP *pB, const AG_SWBREAKCONF_ITEM *conf)
{
    DWORD  adr, a;
    DWORD  m, n, l;
    UC8    rwbuf[1024];
    UC8    vbuf[1024];
    DWORD *repl;
    int    status;

    if (conf == NULL) {
        return (-1);
    }

    if (set) {
        if (pB->Adr >= 0xE0000000)
            return (-1); // Error
        if (!conf->bEnableSWBreaks)
            return (-1); // SW Breaks disabled for address
#if 0                    /* Cortex-M has only Thumb Instructions */
    pA = MGetAttr(pB->Adr);
    if (pA && (*pA & ATRX_ARM)) {
      // ARM Instruction
      if ((pB->Adr & 0x00000003) != 0) return (-1);     // Error
      m = 4;                                            // Breakpoint Size
      n = 4;                                            // 32-bit Access
    } else {
      // Thumb Instruction
      m = 2;                                            // Breakpoint Size
//    n = 2;                                            // 16-bit Access
      n = 4;                                            // 32-bit Access!
    }
#else
        m = 2;                                   // Breakpoint Size
                                                 //  n = 2;                                              // 16-bit Access
        n = 4;                                   // 32-bit Access!
#endif
        if (conf->nAccAlign >= 4 || conf->nAccSz >= 4) {
            l = (conf->nAccAlign > conf->nAccSz) ? conf->nAccAlign : conf->nAccSz;
        } else {
            l = 4;
        }
        adr = pB->Adr & ~(l - 1); // Aligned address
        a   = adr;

        // Read Original Memory (at least 4 bytes to keep breakpoint struct info consistent)
        status = ReadSwBPMem(&a, rwbuf, l, conf->nAccSz);
        if (status) {
            OutError(status);
            return (1);
        }

        // Store opcode to replace
        repl                      = (DWORD *)(&rwbuf[(pB->Adr & ~(4 - 1)) - adr]);
        *((DWORD *)(&pB->Opc[0])) = *repl;

        // Insert BKPT instruction
        if (m == 4) {
            *repl = SWBKPT32;
        } else {
            if (n == 2) { // TODO: check if to use l...
                *repl = SWBKPT16;
            } else {
                if (pB->Adr & 0x00000002) {
                    *repl = (*((WORD *)&pB->Opc[0])) | (SWBKPT16 << 16);
                } else {
                    *repl = ((*((WORD *)&pB->Opc[2])) << 16) | SWBKPT16;
                }
            }
        }

        // Write memory
        a      = adr;
        status = WriteSwBPMem(&a, rwbuf, l, conf->nAccSz);
        if (status) {
            OutError(status);
            return (-1);
        } // Error: Write failed
        // Read back and verify written memory
        a      = adr;
        status = ReadSwBPMem(&a, vbuf, l, conf->nAccSz);
        if (status) {
            OutError(status);
            return (1);
        }
        if (memcmp(rwbuf, vbuf, l)) { // Verify Written Memory
            return (-1);              // SW BP Failed
        }
        pB->Opc[4] = (BYTE)m;
        pB->Opc[5] = (BYTE)n;
    } else {
        if ((m = pB->Opc[4]) && (n = pB->Opc[5])) {
            l = 4;
            if (conf->nAccAlign >= 4 || conf->nAccSz >= 4) {
                l = (conf->nAccAlign > conf->nAccSz) ? conf->nAccAlign : conf->nAccSz;
            }
            adr = pB->Adr & ~(l - 1); // Aligned address

            if (l > m) {
                a      = adr;
                status = ReadSwBPMem(&a, rwbuf, l, conf->nAccSz); // Read back 32-bit
                if (status) {
                    OutError(status);
                    return (1);
                }

                repl = (DWORD *)(&rwbuf[(pB->Adr & ~(4 - 1)) - adr]);
                if (m == 2) {
                    // Update breakpoint struct
                    if (pB->Adr & 0x00000002) {
                        *((WORD *)&pB->Opc[0]) = (WORD)(*repl);
                        if ((WORD)(*repl >> 16) != SWBKPT16) { // Code modified from outside and no longer holding BKPT?
                            *((WORD *)&pB->Opc[2]) = (WORD)(*repl >> 16);
                            txtout((char *)SwBpRestoreWarn, pB->Adr);
                        }
                    } else {
                        *((WORD *)&pB->Opc[2]) = (WORD)(*repl >> 16);
                        if ((WORD)(*repl) != SWBKPT16) { // Code modified from outside and no longer holding BKPT?
                            *((WORD *)&pB->Opc[0]) = (WORD)(*repl);
                            txtout((char *)SwBpRestoreWarn, pB->Adr);
                        }
                    }
                }
                // Now restore the opcode in target memory
                *repl = *((DWORD *)&pB->Opc[0]);
            } else { // l == m (l < m doesn't make sense...)
                repl  = (DWORD *)(rwbuf);
                *repl = *((DWORD *)&pB->Opc[0]);
            }

            // Write memory // Restore Original
            a      = adr;
            status = WriteSwBPMem(&a, rwbuf, l, conf->nAccSz);
            if (status) {
                OutError(status);
                return (-1);
            } // Error: Write failed
            pB->Opc[4] = 0;
            pB->Opc[5] = 0;
        }
    }
    return (0);
}



/*******************************************************************************
  Function:      SetClrSwBP

  Parameter:     set: 1 - Set, 0 - Clear
                 pB:  Pointer to BreakPoint

  Return Value:  0 = OK, Other = Error
--------------------------------------------------------------------------------
  Description:   This function sets or clears a Software BreakPoint.
                 BreakPoint is set by writing a DEBUG Opcode at the
                 BreakPoint's Address. The original Memory contents is saved
                 and restored when BreakPoint is cleared.
                 Opc[0..3] = Original Opcode
                 Opc[4]    = Opcode Size
                 Opc[5]    = Access Size
  Note:          Only 32-bit R/W Access is used!
                 Required by some emulators (NXP,ST: SC300)
*******************************************************************************/

int SetClrSwBP(DWORD set, AG_BP *pB)
{
    DWORD                      adr, a;
    DWORD                      opc, o;
    DWORD                      m, n;
    const AG_SWBREAKCONF_ITEM *conf;
    //DWORD  *pA;
    int status;

    // Get SW Breakpoint Config
    conf = SwBreakGetConf(pB->Adr);
    if (conf && (conf->nAccSz || conf->nAccAlign)) { // Specific requirements for setting SW break
        return SetClrSwBP(set, pB, conf);
    }

    if (set) {
        if (pB->Adr >= 0xE0000000)
            return (-1); // Error
        if (conf && !conf->bEnableSWBreaks)
            return (-1); // SW Breaks disabled for address
#if 0                    /* Cortex-M has only Thumb Instructions */
    pA = MGetAttr(pB->Adr);
    if (pA && (*pA & ATRX_ARM)) {
      // ARM Instruction
      if ((pB->Adr & 0x00000003) != 0) return (-1);     // Error
      m = 4;                                            // Breakpoint Size
      n = 4;                                            // 32-bit Access
    } else {
      // Thumb Instruction
      m = 2;                                            // Breakpoint Size
//    n = 2;                                            // 16-bit Access
      n = 4;                                            // 32-bit Access!
    }
#else
        m = 2;                                   // Breakpoint Size
                                                 //  n = 2;                                              // 16-bit Access
        n = 4;                                   // 32-bit Access!
#endif
        adr = pB->Adr & ~(n - 1);
        a   = adr;

#if DBGCM_V8M
        status = ReadARMMem(&a, &pB->Opc[0], n, BLOCK_SECTYPE_ANY); // Read Original Opc
        if (status) {
            OutError(status);
            return (1);
        }
#else  // DBGCM_V8M
        status = ReadARMMem(&a, &pB->Opc[0], n); // Read Original Opc
        if (status) {
            OutError(status);
            return (1);
        }
#endif // DBGCM_V8M

        if (m == 4) {
            opc = SWBKPT32;
        } else {
            if (n == 2) {
                opc = SWBKPT16;
            } else {
                if (pB->Adr & 0x00000002) {
                    opc = (*((WORD *)&pB->Opc[0])) | (SWBKPT16 << 16);
                } else {
                    opc = ((*((WORD *)&pB->Opc[2])) << 16) | SWBKPT16;
                }
            }
        }

#if DBGCM_V8M
        a      = adr;
        status = WriteARMMem(&a, (BYTE *)&opc, n, BLOCK_SECTYPE_ANY); // Write Breakpoint
        if (status) {
            OutError(status);
            return (-1);
        } // Error: Write failed
        a      = adr;
        status = ReadARMMem(&a, (BYTE *)&o, n, BLOCK_SECTYPE_ANY); // Read back Opc
        if (status) {
            OutError(status);
            return (1);
        }
#else  // DBGCM_V8M
        a = adr;
        status = WriteARMMem(&a, (BYTE *)&opc, n); // Write Breakpoint
        if (status) {
            OutError(status);
            return (-1);
        } // Error: Write failed
        a = adr;
        status = ReadARMMem(&a, (BYTE *)&o, n); // Read back Opc
        if (status) {
            OutError(status);
            return (1);
        }
#endif // DBGCM_V8M

        if (memcmp(&opc, &o, n)) { // Verify Opc
            return (-1);           // SW BP Failed
        }
        pB->Opc[4] = (BYTE)m;
        pB->Opc[5] = (BYTE)n;
    } else {
        if ((m = pB->Opc[4]) && (n = pB->Opc[5])) {
            adr = pB->Adr & ~(n - 1);
            if ((m == 2) && (n == 4)) {
#if DBGCM_V8M
                a      = adr;
                status = ReadARMMem(&a, (BYTE *)&opc, n, BLOCK_SECTYPE_ANY); // Read back 32-bit
                if (status) {
                    OutError(status);
                    return (1);
                }
#else  // DBGCM_V8M
                a = adr;
                status = ReadARMMem(&a, (BYTE *)&opc, n); // Read back 32-bit
                if (status) {
                    OutError(status);
                    return (1);
                }
#endif // DBGCM_V8M

                if (pB->Adr & 0x00000002) {
                    *((WORD *)&pB->Opc[0]) = (WORD)(opc);
                    if ((WORD)(opc >> 16) != SWBKPT16) { // Code modified from outside and no longer holding BKPT?
                        *((WORD *)&pB->Opc[2]) = (WORD)(opc >> 16);
                        txtout((char *)SwBpRestoreWarn, pB->Adr);
                    }
                } else {
                    *((WORD *)&pB->Opc[2]) = (WORD)(opc >> 16);
                    if ((WORD)(opc) != SWBKPT16) { // Code modified from outside and no longer holding BKPT?
                        *((WORD *)&pB->Opc[0]) = (WORD)(opc);
                        txtout((char *)SwBpRestoreWarn, pB->Adr);
                    }
                }
            }

#if DBGCM_V8M
            status = WriteARMMem(&adr, &pB->Opc[0], n, BLOCK_SECTYPE_ANY); // Restore Original
            if (status) {
                OutError(status);
                return (1);
            }
#else  // DBGCM_V8M
            status = WriteARMMem(&adr, &pB->Opc[0], n); // Restore Original
            if (status) {
                OutError(status);
                return (1);
            }
#endif // DBGCM_V8M

            pB->Opc[4] = 0;
            pB->Opc[5] = 0;
        }
    }
    return (0);
}


/*******************************************************************************
  Function:      SetClrHwBP

  Parameter:     set:     1 - Set, 0 - Clear
                 pB:      Pointer to BreakPoint (NULL for All)
                 update : Update Target Flag

  Return Value:  0 = OK, Other = Error
--------------------------------------------------------------------------------
  Description:   This function sets or clears a Hardware BreakPoint.
                 Breakpoint's Opc[7] = HW BP/WP Index
*******************************************************************************/

#if DBGCM_V8M
static int SetClrHwBP_v8M(DWORD set, AG_BP *pB, DWORD update)
{
    DWORD n, i, s, t, f;
    bool  linked;
    int   status, fallback;

    if (pB == NULL) {
        if (set) {
            RegFPB.CTRL = FPB_KEY | FPB_ENABLE;
        } else {
            CntBP = 0;
            ClearUsedBreakResources(); // Clear used breakpoint resources in resource manager
            CntWP = 0;
            memset(&RegFPB, 0, sizeof(RegFPB));
            //    memset(&RegDWT, 0, sizeof(RegDWT));
            RegFPB.CTRL = FPB_KEY;
            for (n = 0; n < NumWP; n++) {
                if (MDtWP & (1 << n)) {
                    RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID; // Keep Function ID in cached value, clears MATCH and ACTION
                }
            }
            MDtWP = 0;
        }
        if (update) {
            if (NumBP) {
                status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), BLOCK_SECTYPE_ANY);
                if (status)
                    return (status);
            }
            if (NumWP) {
                status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), BLOCK_SECTYPE_ANY);
                if (status)
                    return (status);
            }
        }
        return (0);
    }

    if (set) {
        switch (pB->type) {
            case AG_ABREAK: // Address BPs
                if ((FPB_Ver == 0) && pB->Adr >= 0x20000000)
                    return (-1); // Error
                for (n = 0; n < NumBP; n++) {
                    if (RegFPB.COMP[n] & FPB_ENABLE) {
                        if (((RegFPB.COMP[n] ^ pB->Adr) & FPB_CompMask) == 0) {
                            if (FPB_Ver == 0) {
                                RegFPB.COMP[n] |= (pB->Adr & 0x02) ? FPB_BKPT_H : FPB_BKPT_L;
                            }
                            pB->Opc[7] = (BYTE)n;
                            if (update) {
                                status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n], BLOCK_SECTYPE_ANY);
                                if (status)
                                    return (status);
                            }
                            return (0); // Done
                        }
                    }
                }
                for (n = 0; n < NumBP; n++) {
                    if ((RegFPB.COMP[n] & FPB_ENABLE) == 0) {
                        if (FPB_Ver > 0) {
                            RegFPB.COMP[n] = (pB->Adr & FPB_V1_COMP_M) | FPB_ENABLE;
                        } else {
                            RegFPB.COMP[n] = ((pB->Adr & 0x02) ? FPB_BKPT_H : FPB_BKPT_L) | (pB->Adr & FPB_COMP_M) | FPB_ENABLE;
                        }
                        pB->Opc[7] = (BYTE)n;
                        CntBP++;
                        IncUsedBreakResources();
                        if (update) {
                            status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n], BLOCK_SECTYPE_ANY);
                            if (status)
                                return (status);
                        }
                        return (0); // Done
                    }
                }
                return (-1); // Error
            case AG_CBREAK:  // Conditional BPs
                return (-1); // Not Supported
            case AG_WBREAK:  // Watchpoint BPs
                switch (pB->acc) {
                    case 1: t = (DWTv8_DADDR_R | DWTv8_DBGEVT); break;  // Read
                    case 2: t = (DWTv8_DADDR_W | DWTv8_DBGEVT); break;  // Write
                    case 3: t = (DWTv8_DADDR_RW | DWTv8_DBGEVT); break; // Read/Write
                    default: t = 0;
                }

                s = pB->tsize * pB->many;
                f = 0;

                switch (s) {
                    case 1:
                    case 2:
                    case 4:
                        if (!(pB->Adr & (s - 1))) { // Aligned start address
                            t |= ((s >> 1) << DWTv8_DATAVSIZE_P) & DWTv8_DATAVSIZE;
                            break;
                        }
                    default: // Size other than 1, 2, 4,
                             // OR unaligned start address
                             // DWTv8_DATAVSIZE = 0 (1 Byte)
                        if (pB->BP_INFO & BP_INFO_WVAL)
                            return (-1); // Cannot link data value AND address limit
                        f |= DWTv8_ID_LIMIT;
                        break;
                }

                if (pB->BP_INFO & BP_INFO_WVAL)
                    f |= (DWTv8_ID_DVAL | DWTv8_ID_LIMIT);

                fallback = -1;
                for (i = 0; i < NumWP; i++) {
                    n = f ? NumWP - 1 - i : i;
                    if (f && n < 1)
                        break;                                                  // Cannot find suitable comparator for range
                    if ((RegDWT.CMP[n].FUNC & DWTv8_MATCH) == DWTv8_DISABLED) { // Unused comparator?
                        if (f) {                                                // Link to second comparator?
                            if (((RegDWT.CMP[n].FUNC & f) == f)
                                && ((RegDWT.CMP[n - 1].FUNC & DWTv8_MATCH) == DWTv8_DISABLED)) {
                                // Suitable comparator <n> for linkage and comparator <n-1> is free
                                if (!(f & DWTv8_ID_DVAL) && (RegDWT.CMP[n].FUNC & DWTv8_ID_DVAL)) {
                                    // Continue looking out for less expensive comparator pair
                                    if (fallback == -1)
                                        fallback = n;
                                    continue;
                                }

                            set_dwtwatch:
                                // Program Linked Comparator
                                if (f & DWTv8_ID_DVAL) {
                                    // DAddr + DVal
                                    switch (s) { // Size (not mask like for v6-M/v7-M!)
                                        case 1:
                                            RegDWT.CMP[n].COMP = ((BYTE)(pB->BP_DATA) << 24) | ((BYTE)(pB->BP_DATA) << 16) | ((BYTE)(pB->BP_DATA) << 8) | ((BYTE)(pB->BP_DATA) << 0);
                                            break;
                                        case 2:
                                            RegDWT.CMP[n].COMP = ((WORD)(pB->BP_DATA) << 16) | ((WORD)(pB->BP_DATA) << 0);
                                            break;
                                        case 4:
                                            RegDWT.CMP[n].COMP = pB->BP_DATA;
                                            break;
                                    }
                                    // Fist comparator (n-1): ACTION 0b00 - otherwise debug event on every address match
                                    // Second comparator (n): ACTION DBGEVT
                                    t &= ~DWTv8_ACTION;
                                    RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
                                    RegDWT.CMP[n].FUNC |= ((s >> 1) << DWTv8_DATAVSIZE_P) | DWTv8_DVALUE_LINK | DWTv8_DBGEVT /*|DWTv8_TRIGGER*/;
                                } else {
                                    // DAddr + DAddr Limit (Both inclusive)
                                    RegDWT.CMP[n].COMP = pB->Adr + s - 1;
                                    RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
                                    RegDWT.CMP[n].FUNC |= DWTv8_DADDR_LIMIT /*|DWTv8_TRIGGER*/;
                                }
                                MDtWP |= 1 << n;
                                CntWP++;
                                n--;
                            } else {
                                continue;
                            }
                        }
                        RegDWT.CMP[n].COMP = pB->Adr;
                        RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
                        RegDWT.CMP[n].FUNC |= t;
                        pB->Opc[7] = (BYTE)n;
                        MDtWP |= 1 << n;
                        CntWP++;
                        if (update) {
                            status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, BLOCK_SECTYPE_ANY);
                            if (status)
                                return (status);
                        }
                        return (0); // Done
                    }
                }

                if (fallback >= 0) {
                    // Couldn't find less expensive comparator, use first found fallback
                    n = fallback;
                    goto set_dwtwatch;
                }

                return (-1);
        }
    } else {
        switch (pB->type) {
            case AG_ABREAK: // Address BPs
                if (CntBP == 0)
                    return (-1); // Error
                n = pB->Opc[7];
                if (FPB_Ver > 0) {
                    RegFPB.COMP[n] &= ~(FPB_V1_FE | FPB_ENABLE); // 22.06.15: FPBv1 needs different handling
                    CntBP--;
                    DecUsedBreakResources();
                } else {
                    RegFPB.COMP[n] &= (pB->Adr & 0x02) ? ~FPB_BKPT_H : ~FPB_BKPT_L;
                    if ((RegFPB.COMP[n] & FPB_BKPT_LH) == 0) {
                        RegFPB.COMP[n] &= ~FPB_ENABLE;
                        CntBP--;
                        DecUsedBreakResources();
                    }
                }
                if (update) {
                    status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n], BLOCK_SECTYPE_ANY);
                    if (status)
                        return (status);
                }
                break;
            case AG_CBREAK:  // Conditional BPs
                return (-1); // Not Supported
            case AG_WBREAK:  // Watchpoint BPs
                if (CntWP == 0)
                    return (-1); // Error
                n      = pB->Opc[7];
                linked = false;
                if (n + 1 < NumWP && (RegDWT.CMP[n + 1].FUNC)) {
                    // Linked to next comparator
                    RegDWT.CMP[n + 1].FUNC &= DWTv8_FUNCTION_ID;
                    MDtWP &= ~(1 << (n + 1));
                    linked = true;
                }
                RegDWT.CMP[n].FUNC &= DWTv8_FUNCTION_ID;
                MDtWP &= ~(1 << n);
                CntWP--;
                if (update) {
                    status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
                    if (status)
                        return (status);
                    if (linked) {
                        status = WriteD32(DWT_FUNC + 16 * (n + 1), RegDWT.CMP[n + 1].FUNC, BLOCK_SECTYPE_ANY);
                        if (status)
                            return (status);
                    }
                }
                break;
        }
    }
    return (0);
}
#endif // DBGCM_V8M


static int SetClrHwBP(DWORD set, AG_BP *pB, DWORD update)
{
    DWORD n, m, i, t;
    int   status;
    BYTE  mLinkWP; // Mask of linked address comparators

#if DBGCM_V8M
    if (IsV8M()) {
        return SetClrHwBP_v8M(set, pB, update);
    }
#endif // DBGCM_V8M

    if (pB == NULL) {
        if (set) {
            RegFPB.CTRL = FPB_KEY | FPB_ENABLE;
        } else {
            CntBP = 0;
            ClearUsedBreakResources(); // Clear used breakpoint resources in resource manager
            CntWP = 0;
            memset(&RegFPB, 0, sizeof(RegFPB));
            //    memset(&RegDWT, 0, sizeof(RegDWT));
            RegFPB.CTRL = FPB_KEY;
            for (n = 0; n < NumWP; n++) {
                if (MDtWP & (1 << n))
                    RegDWT.CMP[n].FUNC = DWT_DISABLED;
            }
            MDtWP = 0;
        }

#if DBGCM_V8M
        if (update) {
            if (NumBP) {
                status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), BLOCK_SECTYPE_ANY);
                if (status)
                    return (status);
            }
            if (NumWP) {
                status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), BLOCK_SECTYPE_ANY);
                if (status)
                    return (status);
            }
        }
#else  // DBGCM_V8M
        if (update) {
            if (NumBP) {
                status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), 0 /*attrib*/);
                if (status)
                    return (status);
            }
            if (NumWP) {
                status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), 0 /*attrib*/);
                if (status)
                    return (status);
            }
        }
#endif // DBGCM_V8M

        return (0);
    }

    if (set) {
        switch (pB->type) {
            case AG_ABREAK: // Address BPs
                if ((FPB_Ver == 0) && pB->Adr >= 0x20000000)
                    return (-1); // Error
                for (n = 0; n < NumBP; n++) {
                    if (RegFPB.COMP[n] & FPB_ENABLE) {
                        if (((RegFPB.COMP[n] ^ pB->Adr) & FPB_CompMask) == 0) {
                            if (FPB_Ver == 0) {
                                RegFPB.COMP[n] |= (pB->Adr & 0x02) ? FPB_BKPT_H : FPB_BKPT_L;
                            }
                            pB->Opc[7] = (BYTE)n;

#if DBGCM_V8M
                            if (update) {
                                status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n], BLOCK_SECTYPE_ANY);
                                if (status)
                                    return (status);
                            }
#else  // DBGCM_V8M
                            if (update) {
                                status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n]);
                                if (status)
                                    return (status);
                            }
#endif // DBGCM_V8M

                            return (0); // Done
                        }
                    }
                }
                for (n = 0; n < NumBP; n++) {
                    if ((RegFPB.COMP[n] & FPB_ENABLE) == 0) {
                        if (FPB_Ver > 0) {
                            RegFPB.COMP[n] = (pB->Adr & FPB_V1_COMP_M) | FPB_ENABLE;
                        } else {
                            RegFPB.COMP[n] = ((pB->Adr & 0x02) ? FPB_BKPT_H : FPB_BKPT_L) | (pB->Adr & FPB_COMP_M) | FPB_ENABLE;
                        }
                        pB->Opc[7] = (BYTE)n;
                        CntBP++;
                        IncUsedBreakResources();

#if DBGCM_V8M
                        if (update) {
                            status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n], BLOCK_SECTYPE_ANY);
                            if (status)
                                return (status);
                        }
#else  // DBGCM_V8M
                        if (update) {
                            status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n]);
                            if (status)
                                return (status);
                        }
#endif // DBGCM_V8M

                        return (0); // Done
                    }
                }
                return (-1); // Error
            case AG_CBREAK:  // Conditional BPs
                return (-1); // Not Supported
            case AG_WBREAK:  // Watchpoint BPs
                m = GetRangeMask(pB->tsize);
                if ((m > AM_WP) || (pB->Adr & ((1 << m) - 1))) {
                    return (-1); // Range Error
                }
                switch (pB->acc) {
                    case 1: t = DWT_WP_R; break;  // Read
                    case 2: t = DWT_WP_W; break;  // Write
                    case 3: t = DWT_WP_RW; break; // Read/Write
                }
                // Detect Linked Address Comparators
                mLinkWP = 0;
                for (i = 0; i < NumWP; i++) {
                    if ((RegDWT.CMP[i].FUNC & DWT_DATAVMATCH)
                        && (RegDWT.CMP[i].FUNC & DWT_FUNCTION) != DWT_DISABLED) {
                        // Active Data Value Comparator
                        n = (RegDWT.CMP[i].FUNC >> DWT_DATAVADDR0) & 0xF;
                        if (n != i)
                            (mLinkWP = (1 << n));
                        n = (RegDWT.CMP[i].FUNC >> DWT_DATAVADDR1) & 0xF; // Should be programmed to i or n anyway (even if not supported)
                        if (n != i)
                            (mLinkWP = (1 << n));
                    }
                }
                for (i = 0; i < NumWP; i++) {
                    if (NumWP > 2) { // Use DWT CMP0,CMP1 as last
                        n = i + 2;
                        if (n >= NumWP)
                            n -= NumWP;
                    } else {
                        n = i;
                    }
                    if ((RegDWT.CMP[n].FUNC & DWT_FUNCTION) == DWT_DISABLED // Disabled and not used as linked comparator
                        && !(mLinkWP & (1 << n))) {
                        // Check if Data Value Comparator required & free
                        if (pB->BP_INFO & BP_INFO_WVAL) {
                            if ((n == 1) || (m > 2) || ((RegDWT.CMP[1].FUNC & DWT_FUNCTION) != DWT_DISABLED)) {
                                return (-1);
                            }
                        }
                        // Program Data Address Comparator
                        RegDWT.CMP[n].COMP = pB->Adr;
                        RegDWT.CMP[n].MASK = m;
                        RegDWT.CMP[n].FUNC = (pB->BP_INFO & BP_INFO_WVAL) ? 0 : t;
                        pB->Opc[7]         = (BYTE)n;
                        MDtWP |= 1 << n;
                        CntWP++;

#if DBGCM_V8M
                        if (update) {
                            status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, BLOCK_SECTYPE_ANY);
                            if (status)
                                return (status);
                        }
#else  // DBGCM_V8M
                        if (update) {
                            status = WriteBlock(DWT_CMP + 16 * n, (BYTE *)&RegDWT.CMP[n], 3 * 4, 0 /*attrib*/);
                            if (status)
                                return (status);
                        }
#endif // DBGCM_V8M

                        // Program Data Value Comparator if required
                        if (pB->BP_INFO & BP_INFO_WVAL) {
                            RegDWT.CMP[1].FUNC = t | (n << DWT_DATAVADDR0) | (n << DWT_DATAVADDR1);
                            switch (pB->tsize) {
                                case 1:
                                    RegDWT.CMP[1].COMP = ((BYTE)(pB->BP_DATA) << 24) | ((BYTE)(pB->BP_DATA) << 16) | ((BYTE)(pB->BP_DATA) << 8) | ((BYTE)(pB->BP_DATA) << 0);
                                    RegDWT.CMP[1].FUNC |= DWT_DATAVMATCH | DWT_DATAVSIZEB;
                                    break;
                                case 2:
                                    RegDWT.CMP[1].COMP = ((WORD)(pB->BP_DATA) << 16) | ((WORD)(pB->BP_DATA) << 0);
                                    RegDWT.CMP[1].FUNC |= DWT_DATAVMATCH | DWT_DATAVSIZEH;
                                    break;
                                case 4:
                                    RegDWT.CMP[1].COMP = pB->BP_DATA;
                                    RegDWT.CMP[1].FUNC |= DWT_DATAVMATCH | DWT_DATAVSIZEW;
                                    break;
                            }
                            RegDWT.CMP[1].MASK = 0;
                            MDtWP |= 1 << 1;

#if DBGCM_V8M
                            if (update) {
                                status = WriteBlock(DWT_CMP + 16 * 1, (BYTE *)&RegDWT.CMP[1], 3 * 4, BLOCK_SECTYPE_ANY);
                                if (status)
                                    return (status);
                            }
#else  // DBGCM_V8M
                            if (update) {
                                status = WriteBlock(DWT_CMP + 16 * 1, (BYTE *)&RegDWT.CMP[1], 3 * 4, 0 /*attrib*/);
                                if (status)
                                    return (status);
                            }
#endif // DBGCM_V8M
                        }
                        return (0); // Done
                    }
                }
                return (-1); // Error
        }
    } else {
        switch (pB->type) {
            case AG_ABREAK: // Address BPs
                if (CntBP == 0)
                    return (-1); // Error
                n = pB->Opc[7];
                if (FPB_Ver > 0) {
                    RegFPB.COMP[n] &= ~(FPB_V1_FE | FPB_ENABLE); // 22.06.15: FPBv1 needs different handling
                    CntBP--;
                    DecUsedBreakResources();
                } else {
                    RegFPB.COMP[n] &= (pB->Adr & 0x02) ? ~FPB_BKPT_H : ~FPB_BKPT_L;
                    if ((RegFPB.COMP[n] & FPB_BKPT_LH) == 0) {
                        RegFPB.COMP[n] &= ~FPB_ENABLE;
                        CntBP--;
                        DecUsedBreakResources();
                    }
                }

#if DBGCM_V8M
                if (update) {
                    status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n], BLOCK_SECTYPE_ANY);
                    if (status)
                        return (status);
                }
#else  // DBGCM_V8M
                if (update) {
                    status = WriteD32(FPB_COMP + 4 * n, RegFPB.COMP[n]);
                    if (status)
                        return (status);
                }
#endif // DBGCM_V8M

                break;
            case AG_CBREAK:  // Conditional BPs
                return (-1); // Not Supported
            case AG_WBREAK:  // Watchpoint BPs
                if (CntWP == 0)
                    return (-1);                  // Error
                if (pB->BP_INFO & BP_INFO_WVAL) { // 17.09.2015, deactivate data value comparator first;
                                                  // is the only "enabled" comparator when linking to
                                                  // address comparator (see v7-M ARM).
                    RegDWT.CMP[1].FUNC = DWT_DISABLED;
                    MDtWP &= ~(1 << 1);

#if DBGCM_V8M
                    if (update) {
                        status = WriteD32(DWT_FUNC + 16 * 1, RegDWT.CMP[1].FUNC, BLOCK_SECTYPE_ANY);
                        if (status)
                            return (status);
                    }
#else  // DBGCM_V8M
                    if (update) {
                        status = WriteD32(DWT_FUNC + 16 * 1, RegDWT.CMP[1].FUNC);
                        if (status)
                            return (status);
                    }
#endif // DBGCM_V8M
                }
                n                  = pB->Opc[7];
                RegDWT.CMP[n].FUNC = DWT_DISABLED;
                MDtWP &= ~(1 << n);
                CntWP--;

#if DBGCM_V8M
                if (update) {
                    status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
                    if (status)
                        return (status);
                }
#else  // DBGCM_V8M
                if (update) {
                    status = WriteD32(DWT_FUNC + 16 * n, RegDWT.CMP[n].FUNC);
                    if (status)
                        return (status);
                }
#endif // DBGCM_V8M

                break;
        }
    }
    return (0);
}


/*******************************************************************************
  Function:      SaCBreaks

  Parameter:     set: 1 - Set, 0 - Clear

  Return Value:  0 = OK, Other = Error
--------------------------------------------------------------------------------
  Description:   This function sets or clears all BrekPoints depending on 'set'
*******************************************************************************/

static int SaCBreaks(DWORD set)
{
    AG_BP *pB;
    int    ret;

    ret = 0;
    UpdatePBStart();                        // 23.06.2015: Ensure breakpoint list is in sync (bpkill only
                                            // unlinks bp structs on SARMCM3 level while running).
                                            // 12.07.2016: Refactored and extended to fix lockups because of
                                            // breakpoint operations during long StepOver operations.
    for (pB = pBStart; pB; pB = pB->next) { // go through break table
        if (!pB->enabled)
            continue; // breakpoint is disabled
        if (pB->type == AG_ABREAK) {
            if (UseSWBP) {
                ret = SetClrSwBP(set, pB); // SW Breakpoint
                // if (set && (ret == -1)) {                // SW BreakPoint can't be set ?
                if (set && (ret != 0)) {        // SW BreakPoint can't be set ?  (02.07.2019, fixed check of return code)
                    ret = SetClrHwBP(1, pB, 0); // HW Breakpoint
                    if (ret != 0) {
                        // BreakPoint can't be set: Clear alredy defined BreakPoints
                        AGDIMsgBox(hMfrm, BPErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                        ret = SetClrHwBP(0, NULL, 0);
                        if (ret != 0)
                            break;
                        // 27.01.2020: Tracepoint extensions
                        ret = SetClrHwTP(0, NULL, 0);
                        if (ret != 0)
                            break;

                        for (pB = pB->prev; pB; pB = pB->prev) {
                            if (!pB->enabled)
                                continue;
                            if (pB->type != AG_ABREAK)
                                continue;
                            ret = SetClrSwBP(0, pB);
                            if (ret != 0)
                                break;
                        }
                        ret = -1;
                        break;
                    }
                }
            } else {
                ret = SetClrHwBP(set, pB, 0); // HW Breakpoint
            }
        } else if (pB->type == AG_TBREAK) { // 27.01.2020: Tracepoint extensions
            ret = SetClrHwTP(set, pB, 0);
        } else {
            ret = SetClrHwBP(set, pB, 0); // HW Breakpoint/Watchpoint
        }
        if (ret != 0)
            break;
    }
    if (ret == 0)
        ret = SetClrHwBP(set, NULL, 1);
    if (ret == 0)
        ret = SetClrHwTP(set, NULL, 1);
    if (ret == 0 && set) {
        if (NTrRP == 0 && NTrSP > 0) {
            if (AGDIMsgBox(hMfrm, TrRunWarn, &InfoTitle[0], MB_YESNO | MB_ICONQUESTION, IDYES) != IDYES) {
                ret = 1;
            }
        }
    }

#if 0 // 27.01.2020: Call function to program ETM tracepoint logic (TODO: port full implementation!!)

#if DBGCM_FEATURE_ETM
  if (supp.TpInstrRunSusp || supp.TpInstrHalt) {
    //---TODO: Implement logic to programm ETM tracepoints
    DEVELOP_MSG ("Todo: \nImplement logic to programm ETM tracepoints.");
    // if (ret == 0) ret = ETM_ProgTPLogic();
  }
#endif // DBGCM_FEATURE_ETM

#endif

    if (ret && pio)
        pio->st.inf.StopBut |= 1; // Stop a HLL step when an error occurs

    return (ret);
}


/*
 * Setup Hit-Breakpoint info
 */

void SetHitBp(AG_Bps *Bp, DWORD nAdr, DWORD nAcc, DWORD nPC)
{
    pio->hitbp.nAdr      = nAdr;
    pio->hitbp.nAcc      = (WORD)nAcc;
    pio->hitbp.nPC       = nPC;
    pio->hitbp.nBpNum    = Bp ? (WORD)Bp->number : 0xFFFF; // Bp-number or invalid
    pio->hitbp.nTickMark = Bp ? ((BKS *)Bp)->nTickMark : 0;
}


/*
 * Get Break Event / Read Debug Fault Status Register
 *   Return value: Debug Event Flags
 */

DWORD GetBreakEvent(void)
{
    DWORD val, n;
    int   status;

#if DBGCM_V8M
    status = ReadD32(NVIC_DFSR, &val, BLOCK_SECTYPE_ANY);
    if (status)
        return (0);

    for (n = 0; n < NumWP; n++) {
        if ((MTrWP & (1 << n)) == 0) {
            status = ReadD32(DWT_FUNC + 16 * n, &RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
            if (status)
                return (0);
        }
    }
#else  // DBGCM_V8M
    status = ReadD32(NVIC_DFSR, &val);
    if (status)
        return (0);

    for (n = 0; n < NumWP; n++) {
        if ((MTrWP & (1 << n)) == 0) {
            status = ReadD32(DWT_FUNC + 16 * n, &RegDWT.CMP[n].FUNC);
            if (status)
                return (0);
        }
    }
#endif // DBGCM_V8M

    return (val);
}


/*
 * Is there a Breakpoint at current PC
 */

DWORD IsBreak(void)
{
    AG_BP *pB;

    for (pB = pBStart; pB; pB = pB->next) {
        if (pB->enabled) {
            if (pB->type == AG_ABREAK) {
                if (*pCURPC == pB->Adr)
                    return (1);
            }
        }
    }
    return (0); // there is no breakpoint on the current PC
}


static int ExpOp(EXP *ep)
{ // 4.5.2012
    if (ep)
        switch (ep->etyp) {
            case e_land: // e1 && e2
            case e_lor:  // e1 || e2
                return (1);
            case e_equ:  // ==
            case e_nequ: // !=
            case e_lequ: // <=
            case e_gequ: // >=
            case e_gt:   // >
            case e_lt:   // <
                return (1);
        }
    return (0);
}


/*
 * Search for Triggered Breakpoint
 */

DWORD BrkWalk(BOOL step)
{
    AG_BP *pB;
    DWORD  brk, nRes, nReason, n;
    EXP   *ep;

    if (step) {
        brk = BKPT;
    } else {
        brk = GetBreakEvent();
        if (brk & (EXTERNAL | VCATCH | HALTED))
            return (1);
    }

    nReason = 2; // [TdB 13.03.2012] seto 'no reason' PH: 7.3.2012
    for (pB = pBStart; pB; pB = pB->next) {
        if (!pB->enabled)
            continue; // this Break is disabled.
        switch (pB->type) {
            case AG_ABREAK: // Address Break
                if (pB == &TBp)
                    continue; // skip temporary breakpoint (GoUntil)
                if ((brk & BKPT) && (*pCURPC == pB->Adr)) {
                    SetHitBp(pB, *pCURPC, HIT_EXEC, *pCURPC);
                    if (pB->rcount == 1) {
                        if (pB->cmd) {                       // is command defined
                            bNoCache = 1;                    // turn off memory cache in case user reads from  /10.4.2013/
                                                             // 10.11.2016: Switched order of bNoCache and iRun to avoid accidental reads from cache by other threads
                            iRun   = 0;                      // HS 20.12.2002 line added to enable memory access during breakpoint commands
                            iBpCmd = 1;                      // SDMDK-4306, 13.01.2015: executing breakpoint command
                            pCbFunc(AG_CB_EXECCMD, pB->cmd); // execute command
                            iBpCmd   = 0;                    // SDMDK-4306, 13.01.2015: finished executing breakpoint command
                            iRun     = 1;                    // HS 20.12.2002 line added to disable memory access again
                            bNoCache = 0;                    // turn on memory cache                           /10.4.2013/
                                                             // 10.11.2016: Switched order of bNoCache and iRun to avoid accidental reads from cache by other threads
                            if (GoUntilAdr != 0xFFFFFFFF && pB->Adr == GoUntilAdr) {
                                return (1); // 09.01.2015: Always indicate break when reaching RunTil() destination address
                            } else {
                                return (0); // continue with application when command is defined
                            }
                        }
                        return (1); // don't continue with application
                    }
                    --pB->rcount; // decrement break counter
                    return (0);   // continue with application
                }
                break;

            case AG_CBREAK: // Conditional Break
                break;

            case AG_WBREAK: // Watchpoint Break
                if (brk & DWTTRAP) {
                    ep = (EXP *)pB->ep; // [TdB 13.03.2012] PH: 3.3.2012 - check for expression recalculation
                    //if (!(pB->BP_INFO & BP_INFO_WVAL) || (ep && (ep->etyp == e_land || ep->etyp == e_lor)))  {
                    if (!(pB->BP_INFO & BP_INFO_WVAL) || ExpOp(ep)) { // 4.5.2012: Extended results for expression operator
#if DBGCM_V8M
                        if (IsV8M()) {
                            n = pB->Opc[7]; // find matching access-bp
                            if (!(RegDWT.CMP[n].FUNC & DWTv8_MATCHED)) {
                                continue; // another access-break
                            }             // ---------------------------------
                        } else {
                            if (pB->BP_INFO & BP_INFO_WVAL) {
                                n = 1; // 17.09.2015, made DWT programming architecure-compliant
                                       // data value comparator indicates the match (not the linked
                                       // but "disabled" address comparator).
                            } else {
                                n = pB->Opc[7]; // 4.5.2012, find matching access-bp
                            }
                            if (!(RegDWT.CMP[n].FUNC & DWT_MATCHED)) {
                                continue; // another access-break
                            }             // ---------------------------------
                        }
#else  // DBGCM_V8M
                        if (pB->BP_INFO & BP_INFO_WVAL) {
                            n = 1; // 17.09.2015, made DWT programming architecure-compliant
                                   // data value comparator indicates the match (not the linked
                                   // but "disabled" address comparator).
                        } else {
                            n = pB->Opc[7]; // 4.5.2012, find matching access-bp
                        }
                        if (!(RegDWT.CMP[n].FUNC & DWT_MATCHED)) {
                            continue; // another access-break
                        }             // ---------------------------------
#endif // DBGCM_V8M

                        if (pB->ReCalc && ep) {
                            bNoCache = 1; // 10.11.2016: Switched order of bNoCache and iRun to avoid accidental reads from cache by other threads
                            iRun     = 0;
                            nRes     = pCbFunc(AG_CB_TRUEXPR, ep); // calc expression - true value ?
                            iRun     = 1;
                            bNoCache = 0;    // 10.11.2016: Switched order of bNoCache and iRun to avoid accidental reads from cache by other threads
                            if (!nRes) {     // expression yields false...
                                nReason = 0; // ignore false watch-breakpoint 7.3.2012
                                continue;    // check other breakpoints...  /5.3.2012/
                            }
                        }
                    } // --------

                    // hitting ReadWrite access BP reports write access on read access
                    switch (pB->acc) {
                        case 1:
                            SetHitBp(pB, *pCURPC, HIT_READ, *pCURPC);
                            break;
                        case 3:
                            SetHitBp(pB, *pCURPC, (HIT_READ | HIT_WRITE), *pCURPC);
                            break;
                        default:
                            SetHitBp(pB, *pCURPC, HIT_WRITE, *pCURPC);
                            break;
                    }

                    if (pB->rcount == 1) {
                        if (pB->cmd) {                       // is command defined
                            bNoCache = 1;                    // turn off memory cache in case user reads from  /10.4.2013/
                                                             // 10.11.2016: Switched order of bNoCache and iRun to avoid accidental reads from cache by other threads
                            iRun   = 0;                      // HS 20.12.2002 line added to enable memory access during breakpoint commands
                            iBpCmd = 1;                      // SDMDK-4306, 13.01.2015: executing breakpoint command
                            pCbFunc(AG_CB_EXECCMD, pB->cmd); // execute command
                            iBpCmd   = 0;                    // SDMDK-4306, 13.01.2015: finished executing breakpoint command
                            iRun     = 1;                    // HS 20.12.2002 line added to disable memory access again
                            bNoCache = 0;                    // turn on memory cache                           /10.4.2013/
                                                             // 10.11.2016: Switched order of bNoCache and iRun to avoid accidental reads from cache by other threads
                            return (0);                      // continue with application when command is defined
                        }
                        return (1); // don't continue with application
                    }
                    --pB->rcount; // decrement break counter
                    return (0);   // continue with application
                }
                break;
        }
    }
    SetHitBp(NULL, *pCURPC, StopRun ? HIT_ESC : HIT_EXEC, *pCURPC);

    return (nReason); // [TdB 13.03.2012] PH: 7.3.2012
}


/*
 * Breakpoint Management
 */

#define ATRX_BPQ  (ATRX_BREAK | ATRX_BPDIS | ATRX_EXECD | ATRX_ITAKEN)
#define ATRX_CCOV (ATRX_EXECD | ATRX_ITAKEN)

U32 _EXPO_ AG_BpInfo(U16 nCode, void *vp)
{
    U32    nA, nAdr;
    U16   *pB, nB;
    AG_BP *pK;
    int    s, err;
    // Disable code coverage attribs until having proper story for ETB
    U16 attrMask = (TraceConf.Protocol == TPIU_ETB) ? (~ATRX_CCOV) : 0xFFFF;

    // if (PlayDead) return (0);
    if (PlayDead) {
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (0);
    }
    switch (nCode) {
        case AG_BPDISALL:  // disable all Exec-Bp's
        case AG_BPKILLALL: // kill all Exec-Bp's
            err = 0;
            break;
        // RR 16.05.2008 [Start]: added support for breakpoints in unmapped memory
        case AG_BPSET:                // set breakpoint
            nAdr = ((GADR *)vp)->Adr; // the referred address
            pB   = MGetAttr16(nAdr);  // get attribute location for 'nAdr'
            if (!pB) {                // check if not mapped yet
                MMapMem(ATRX_EXEC | ATRX_READ | ATRX_WRITE, nAdr, (nAdr & 0x00000002) ? 2 : 4);
                pB = MGetAttr16(nAdr); // get attribute location
            }
            if (!pB)
                return (0x80000000); // failed: unmapped address
            nB = *pB;
            break;
        // RR 16.05>2008: [End]
        default:                      // need attribute pointer anyway, set it up here.
            nAdr = ((GADR *)vp)->Adr; // the referred address
            pB   = MGetAttr16(nAdr);  // get attribute location for 'nAdr'
            if (!pB)
                return (0x80000000); // failed: unmapped address
            nB = *pB;
            break;
    }

    nA = 0;
    switch (nCode) {                     // Function code
        case AG_BPQUERY:                 // Query Break
        case AG_BPEXQUERY:               // Query for 'executed' attribute
            nA = (UL32)(*pB & ATRX_BPQ); // Attributes are similar to uVision
            nA &= attrMask;              // Filter out code coverage attrs if ETB
            break;

        case AG_CANHANDLEBP:        // Can handle breakpoint while running ?
            if (*pB & ATRX_BREAK) { // Breakpoint Active ?
                nA = 1;             // Can Remove BP (HW or SW)
            } else {
                if (CntBP < NumBP)
                    nA = 1; // Can Add HW BP ?
            }
            break;

        case AG_BPENABLE:                               // enable breakpoint
            if (*pB & ATRX_BPDIS) {                     // is Bp disabled ?
                *pB = (*pB & ~ATRX_BPDIS) | ATRX_BREAK; // enable it
                goto bps;
            }
            break;

        case AG_BPDISABLE:                              // disable breakpoint
            if (*pB & ATRX_BREAK) {                     // is Bp enabled ?
                *pB = (*pB & ~ATRX_BREAK) | ATRX_BPDIS; // disable it
                goto bpc;
            }
            break;

        case AG_BPKILL:                             // kill breakpoint
            *pB = *pB & ~(ATRX_BREAK | ATRX_BPDIS); // kill it
        bpc:
            if (iRun || iBpCmd) { // if in BP command, only update pBStart
                for (pK = *pBhead; pK; pK = pK->next) {
                    if ((pK->type == AG_ABREAK) && (pK->Adr == nAdr)) {
                        UpdatePBStart();      // 12.07.2016: Refactored and extended to fix lockups because of
                                              // breakpoint operations during long StepOver operations.
                        if (iRun && GoMode) { // full remove only when running, not in BP command
                            if (pK->enabled == 0)
                                break;
                            if (pK->Opc[4]) {
                                s = SetClrSwBP(0, pK);
                            } else {
                                s = SetClrHwBP(0, pK, 1);
                            }
                            if (s) {      // error ?
                                *pB = nB; // restore BP
                                AGDIMsgBox(hMfrm, BPErr[4], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                            }
                        }
                        break;
                    }
                }
            }
            nA = *pB;       // Extended BP return value
            nA &= attrMask; // Filter out code coverage attrs if ETB
            break;

        case AG_BPSET:                              // set breakpoint
            *pB = (*pB & ~ATRX_BPDIS) | ATRX_BREAK; // set it
        bps:
            if (iRun || iBpCmd) { // if in BP command, only update pBStart
                for (pK = *pBhead; pK; pK = pK->next) {
                    if ((pK->type == AG_ABREAK) && (pK->Adr == nAdr)) {
                        UpdatePBStart();      // 12.07.2016: Refactored and extended to fix lockups because of
                                              // breakpoint operations during long StepOver operations.
                        if (iRun && GoMode) { // full remove only when running, not in BP command
                            s = SetClrHwBP(1, pK, 1);
                            if (s) {      // error ?
                                *pB = nB; // restore BP
                                AGDIMsgBox(hMfrm, BPErr[4], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
                            }
                        }
                        break;
                    }
                }
            }
            nA = *pB;       // Extended BP return value
            nA &= attrMask; // Filter out code coverage attrs if ETB
            break;

        case AG_BPDISALL: // disable all Bp's
            for (pK = *pBhead; pK; pK = pK->next) {
                if (pK->type == TBREAK) { // 27.01.2020: Tracepoint extensions
                    // Don't handle tracepoints in this call
                    continue;
                }
                if (pK->type != AG_ABREAK) { // 21.3.2012
                    pK->enabled = 0;         // disable Bp
                    ++nA;
                    continue;
                }
                if (pK->type == AG_ABREAK && pK->enabled) {
                    pB = MGetAttr16(pK->Adr);       // get attribute location
                    if (pB && (*pB & ATRX_BREAK)) { // is Bp enabled ?
                        nB  = *pB;
                        *pB = (*pB & ~ATRX_BREAK) | ATRX_BPDIS; // disable it
                        if (iRun) {
                            if (pK->Opc[4]) {
                                s = SetClrSwBP(0, pK);
                            } else {
                                s = SetClrHwBP(0, pK, 1);
                            }
                            if (s) {      // error ?
                                *pB = nB; // restore
                                err++;
                                continue;
                            }
                        }
                        pK->enabled = 0; // Preset BP as disabled
                    }
                    nA++; // count number of changed Bp's
                }
            }
            if (err)
                AGDIMsgBox(hMfrm, BPErr[5], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
            break;

        case AG_BPKILLALL: // kill all Bp's
            for (pK = *pBhead; pK; pK = pK->next) {
                if (pK->type == AG_ABREAK) {
                    pB = MGetAttr16(pK->Adr); // get attribute location
                    if (pB) {
                        nB  = *pB;
                        *pB = *pB & ~(ATRX_BREAK | ATRX_BPDIS); // kill it
                        if (iRun && pK->enabled) {
                            if (pK->Opc[4]) {
                                s = SetClrSwBP(0, pK);
                            } else {
                                s = SetClrHwBP(0, pK, 1);
                            }
                            if (s) {      // error ?
                                *pB = nB; // restore
                                err++;
                                continue;
                            }
                        }
                        pK->enabled = 0; // Preset BP as disabled
                    }
                    nA++; // count number of changed Bp's
                }
            }
            if (err)
                AGDIMsgBox(hMfrm, BPErr[5], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
            break;
    }

    if (PlayDead == 1) {
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (nA);
}


_EXPO_ AG_BP *AG_BreakFunc(U16 nCode, U16 n1, GADR *pA, AG_BP *pBp)
{
    U16 *pB, nB;
    int  s;

    // if (PlayDead) return (NULL);       // driver is disconnected
    if (PlayDead) {
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (NULL);  // driver is disconnected
    }
    if (pA == NULL || pBp == NULL) { // valid input parameters
        if (nCode == AG_BF_CLEARATTR || nCode == AG_BF_SETATTR)
            return (pBp); // return "success", handled in calling DLL
        if (nCode != AG_BF_ACCEPT)
            return (NULL); // invalid, cancel
    }

    if (nCode != AG_BF_ACCEPT) {  // not BpAccept function
        pB = MGetAttr16(pA->Adr); // get attribute location
        if (!pB) {                // check if not mapped yet
            MMapMem(ATRX_EXEC | ATRX_READ | ATRX_WRITE,
                    pA->Adr, (pA->Adr & 0x00000002) ? 2 : 4);
            pB = MGetAttr16(pA->Adr); // get attribute location
        }
        if (!pB)
            return (NULL); // invalid, cancel
        nB = *pB;
    }

    switch (nCode) {
        case AG_BF_UNLINK:             // Notification: 'pB' will be unlinked
            if (pBp->type == TBREAK) { // 27.01.2020: Tracepoint extensions
                // Don't modify breakpoint attributes for tracepoints
                break;
            }
            *pB &= ~(ATRX_BREAK | ATRX_BPDIS);
            if ((iRun || iBpCmd) && (nB & ATRX_BREAK)) { // if in BP command, only update pBStart
                UpdatePBStart();                         // 12.07.2016: Refactored and extended to fix lockups because of
                                                         // breakpoint operations during long StepOver operations.
                if (iRun && GoMode) {                    // full remove only when running, not in BP command
                    if (pBp->Opc[4]) {
                        s = SetClrSwBP(0, pBp);
                    } else {
                        s = SetClrHwBP(0, pBp, 1);
                    }
                    if (s) {      // error ?
                        *pB = nB; // restore
                        OutError(EU18);
                    }
                }
            }
            break;
        case AG_BF_QUERY:     // not used.
        case AG_BF_CLEARATTR: // not used.
        case AG_BF_SETATTR:   // not used.
        case AG_BF_GETHEAD:   // not used.
            break;
        case AG_BF_LINK:   // Notification: 'pB' will be linked
        case AG_BF_CHANGE: // 'pB->enabled' may have changed
        // xbrk: if (pBp->enabled)  {
        xbrk:
            if (pBp->type == TBREAK) { // 27.01.2020: Tracepoint extensions
                // Don't modify breakpoint attributes for tracepoints
                break;
            }

            if (pBp->enabled) {
                *pB = (*pB & ~ATRX_BPDIS) | ATRX_BREAK; // enable it
                if (iRun || iBpCmd) {                   // if in BP command, only update pBStart
                    UpdatePBStart();                    // 12.07.2016: Refactored and extended to fix lockups because of
                                                        // breakpoint operations during long StepOver operations.
                    if (iRun && GoMode) {               // full remove only when running, not in BP command
                        s = SetClrHwBP(1, pBp, 1);
                        if (s) {      // error ?
                            *pB = nB; // restore
                            OutError(EU18);
                        }
                    }
                }
            } else {
                *pB = (*pB & ~ATRX_BREAK) | ATRX_BPDIS;      // disable it
                if ((iRun || iBpCmd) && (nB & ATRX_BREAK)) { // if in BP command, only update pBStart
                    UpdatePBStart();                         // 12.07.2016: Refactored and extended to fix lockups because of
                                                             // breakpoint operations during long StepOver operations.
                    if (iRun && GoMode) {                    // full remove only when running, not in BP command
                        if (pBp->Opc[4]) {
                            s = SetClrSwBP(0, pBp);
                        } else {
                            s = SetClrHwBP(0, pBp, 1);
                        }
                        if (s) {      // error ?
                            *pB = nB; // restore
                            OutError(EU18);
                        }
                    }
                }
            }
            break;

        case AG_BF_ACCEPT: // Bp-accept function
            /* We accept all BP types (then check later if we can really set them),
         unless we are running in which case we only accept address BPs (via BpInfo) */
            if (iRun && (pBp->type == TBREAK || pBp->type == CBREAK || pBp->type == RBREAK)) {
                pBp = (AG_Bps *)0xFFFFFFFF;
                OutError(EU18);
            }
            break;

        case AG_BF_TARGET: // Install/Uninstall break on target
            if (pBp->type == ABREAK)
                goto xbrk; // Same as AG_BF_LINK/AG_BF_UNLINK
            if (pBp->type != WBREAK)
                break; // Don't modify breakpoints other than access breakpoints
            if (!iRun && !iBpCmd)
                break; // Update breakpoint head if target running or in breakpoint command execution

            UpdatePBStart(); // 12.07.2016: Refactored and extended to fix lockups because of
                             // breakpoint operations during long StepOver operations.
            if (!(iRun && GoMode))
                break; // Set breakpoint to hardware if target running

            s = SetClrHwBP(pBp->enabled, pBp, 1);
            if (s)
                OutError(EU18); // Error
            break;
    }

    if (PlayDead == 1) {
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (pBp);
}



/*
 * Execute until 'nAdr' is reached or some other Bp fires.
 */

void GoUntil(DWORD nAdr)
{ // 0xFFFFFFFF := go forever
    int   brk, sacRes;
    BYTE  val[2];
    DWORD addr;
    BOOL  IsBpInstructionSkipped = false;
    DWORD breakReason;

    if (StopRun) { // Stop-Button pressed
        StopRun = 0;
        return;
    }

    brk = AnalyzeBreaks(nAdr); // Analyze BreakPoints
    if (brk) {                 // BreakPoints cannot be handled
        AGDIMsgBox(hMfrm, BPErr[brk], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
        if (pio) {
            pio->st.inf.StopBut |= 1; // Stop a HLL step when an error occurs
        }
    } else {
        if (BkpInstructionSkip) { // Skip SW breakpoint instruction

#if DBGCM_V8M
            addr = *pCURPC;
            ReadARMMem(&addr, val, 2, BLOCK_SECTYPE_ANY); // Read value at address of PC
            if (val[1] == (SWBKPT16 >> 8)) {              // If value at address is a __breakpoint instruction PC increment one step further
                RegARM.PC += 2;
                SetRegs(&RegARM, NULL, NULL, (1ULL << nR15)); // Set PC in target device
                IsBpInstructionSkipped = true;
                breakReason            = BrkWalk(1);   // Get the reason
                pio->hitbp.nAcc        = HIT_NOREASON; // Clear bp reason from BrkWalk
            }
#else  // DBGCM_V8M
            addr = *pCURPC;
            ReadARMMem(&addr, val, 2);       // Read value at address of PC
            if (val[1] == (SWBKPT16 >> 8)) { // If value at address is a __breakpoint instruction PC increment one step further
                RegARM.PC += 2;
                SetRegs(&RegARM, NULL, (1ULL << nR15)); // Set PC in target device
                IsBpInstructionSkipped = true;
                breakReason = BrkWalk(1);       // Get the reason
                pio->hitbp.nAcc = HIT_NOREASON; // Clear bp reason from BrkWalk
            }
#endif // DBGCM_V8M

            if (MultipleGoStep)
                BkpInstructionSkip = FALSE; // This happens when we are in HLL multistepping and ASM overstepping
        }

        SetTraceRunSynch(true, true); // Set trace running (internal check for availability)

        GoUntilAdr = nAdr;
        while (!StopRun) {
            if (IsBreak() && !IsBpInstructionSkipped) { // Bp at current PC, step over it.
                Step();
                //      Invalidate();               // registers, memory cache  // when AG_RUNSTOP
                if (PlayDead) {
                    SetTraceRunSynch(false, false); // Set trace stopped (internal check for availability), skip window update, playing dead
                    goto end;
                }
                if (BrkWalk(1) == 1)
                    break;
                UpdatePBStart(); // 12.07.2016: Update pBStart after single-step breakwalk
                                 // in case update came in
                continue;
            }

            ClearRunBreaks();
            if (SaCBreaks(1)) {                 // Set BreakPoints
                SetTraceRunSynch(false, false); // Set trace stopped (internal check for availability), no window update, target not really started yet
                goto end;
            }
            GoMode++; // Changed to semaphore like usage, also used in other places
            GoCmd(TRUE);
            GoMode--; // Changed to semaphore like usage, also used in other places
                      //    Invalidate();                 // registers, memory cache  // when AG_RUNSTOP
            sacRes = SaCBreaks(0);
            MergeRunBreaks();
            if (sacRes) {                      // Clear BreakPoints
                SetTraceRunSynch(false, true); // Set trace stopped (internal check for availability)
                goto end;
            }

            if (PlayDead) {
                SetTraceRunSynch(false, false); // Set trace stopped (internal check for availability), no need to even try updating
                goto end;
            }
            //    if (BrkWalk(0) == 1) break;
            if (BrkWalk(0))
                break; // Check breakpoints (also user BPs)
        }

        SetTraceRunSynch(false, true); // Set trace stopped (internal check for availability)
    }
end:
    GoUntilAdr = 0xFFFFFFFF;
}


#if DBGCM_WITHOUT_STOP
/*
 * Execute until 'nAdr' is reached or some other Bp fires.
 */

void WaitUntil(void)
{
    int sacRes;

    if (StopRun) { // Stop-Button pressed
        StopRun = 0;
        return;
    }

    if (GoMode) {
        return;
    }

    SetTraceRunSynch(true, true); // Set trace running (internal check for availability)

    GoUntilAdr = 0xFFFFFFFF; // 0xFFFFFFFF := wait forever
    while (!StopRun) {
        ExtendedStep = 0; // Clear extended step flag

        GoMode++;
        GoCmd(FALSE);
        GoMode--;

        sacRes = SaCBreaks(0);
        MergeRunBreaks();
#if 0 // 03.11.2016: Ignore SaCBreaks() result, at the moment no breakpoints \
      //             are installed before WaitUntil().
    if (sacRes) {
      SetTraceRunSynch(false, true); // Set trace stopped (internal check for availability)
      goto end;                      // Clear BreakPoints
    }
#endif

        if (PlayDead) {
            SetTraceRunSynch(false, true); // Set trace stopped (internal check for availability)
            goto end;
        }
        //if (BrkWalk(0)) break;           // Check breakpoints (also user BPs)
        if (BrkWalk(ExtendedStep))
            break; // Check breakpoints (also user BPs)
    }

    // GoUntil stopped

    SetTraceRunSynch(false, true); // Set trace stopped (internal check for availability)

end:
    GoUntilAdr = 0xFFFFFFFF;
}
#endif // DBGCM_WITHOUT_STOP


/*
 * Go/Step/Stop commands
 */

_EXPO_ U32 AG_GoStep(U16 nCode, U32 nSteps, GADR *pA)
{
    U32   nE;
    BYTE  val[2];
    DWORD addr;
    DWORD breakReason;

    if (PlayDead) { // driver is disconnected.
        if (nCode == AG_STOPRUN)
            StopRun = 1; // ensure GoCmd() exits
        ExecPlayDead();  // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (0);
    }


    nE              = 0;            // clear error code
    pio->hitbp.nAcc = HIT_NOREASON; // clear bp reason
    switch (nCode) {
        case AG_STOPRUN: // Stop Go/Step.
            StopRun = 1;
            break;

        case AG_NSTEP:                // execute 'nSteps' instruction steps
            if (BkpInstructionSkip) { // Skip SW breakpoint instruction

#if DBGCM_V8M
                addr = *pCURPC;
                ReadARMMem(&addr, val, 2, BLOCK_SECTYPE_ANY); // Read value at address of PC
                if (val[1] == (SWBKPT16 >> 8)) {              // If value at address is a __breakpoint instruction PC increment one step further
                    RegARM.PC += 2;
                    SetRegs(&RegARM, NULL, NULL, (1ULL << nR15)); // Set PC in target device
                    nSteps--;                                     // Step number decrement
                    breakReason     = BrkWalk(1);                 // Get the reason
                    pio->hitbp.nAcc = HIT_NOREASON;               // Clear bp reason from BrkWalk
                }
#else  // DBGCM_V8M
                addr = *pCURPC;
                ReadARMMem(&addr, val, 2);       // Read value at address of PC
                if (val[1] == (SWBKPT16 >> 8)) { // If value at address is a __breakpoint instruction PC increment one step further
                    RegARM.PC += 2;
                    SetRegs(&RegARM, NULL, (1ULL << nR15)); // Set PC in target device
                    nSteps--;                               // Step number decrement
                    breakReason = BrkWalk(1);               // Get the reason
                    pio->hitbp.nAcc = HIT_NOREASON;         // Clear bp reason from BrkWalk
                }
#endif // DBGCM_V8M

                if (MultipleGoStep)
                    BkpInstructionSkip = FALSE; // This happens when we are in HLL multistepping and ASM overstepping
            }

            pBStart = *pBhead; // 02.02.2015: Update pBStart before stepping (no call to AnalyzeBreaks() here)
            iRun    = 1;       // 'executing'
            //if  (TraceOpt & TRACE_ENABLE) OutMsg(TraceMsg[T_Msg = T_MSG_RUN]);
            if (TraceOpt & TRACE_ENABLE)
                OutTraceMsg(T_Msg = T_MSG_RUN);
            for (; nSteps != 0; --nSteps) {
                if (StopRun)
                    break; // Stop-Button was pressed
                if (Step() != 0x01)
                    break; // 0x01 means 'Ok.'
                if (pio->hitbp.nAcc != HIT_NOREASON)
                    break; // Stop signal during Step()
                if (BrkWalk(1) == 1)
                    break;                      // BrkWalk and stop if hitting a break without continue
                pio->hitbp.nAcc = HIT_NOREASON; // clear bp reason from BrkWalk
                UpdatePBStart();                // 12.07.2016: Update pBStart after single-step breakwalk
                                                // in case update came in during many single steps
            }

            // Set breakpoint reason for stepping
            // Stop reason is either BP from BrkWalk, stop button or end of stepping (indicated by HIT_EXEC)
            if ((pio->hitbp.nAcc == HIT_NOREASON) || StopRun) {
                DWORD bpReason = HIT_EXEC;
                if (StopRun)
                    bpReason = HIT_ESC;
                SetHitBp(NULL, *pCURPC, bpReason, *pCURPC);
            }

            StopRun = 0;  // clear Stop-Button flag.
                          //    Invalidate();                   // registers, caches, etc.  // when AG_RUNSTOP
            bNoCache = 1; // JR, 10.11.2016: Extend cache bypass until AG_RUNSTOP invalidated caches
            iRun     = 0; // clear 'executing' flag
            NumRecs  = 0; // clear 'number of trace records'
            if ((TraceOpt & TRACE_ENABLE) && (T_Msg == T_MSG_RUN))
                OutMsg("");
            break;

        case AG_GOTILADR: // run til 'pA->Adr' or some Bp,
            iRun = 1;     // whichever comes first
            //if  (TraceOpt & TRACE_ENABLE) OutMsg(TraceMsg[T_Msg = T_MSG_RUN]);
            if (TraceOpt & TRACE_ENABLE)
                OutTraceMsg(T_Msg = T_MSG_RUN);
            GoUntil(pA->Adr);
            StopRun = 0;
            //    Invalidate();                   // registers, caches, etc. // in GoUntil
            bNoCache = 1; // JR, 10.11.2016: Extend cache bypass until AG_RUNSTOP invalidated caches
            iRun     = 0; // clear 'executing' flag
            NumRecs  = 0; // clear 'number of trace records'
            if ((TraceOpt & TRACE_ENABLE) && (T_Msg == T_MSG_RUN))
                OutMsg("");
            break;

        case AG_GOFORBRK: // run forever or till some Bp reached.
            iRun = 1;
            //if  (TraceOpt & TRACE_ENABLE) OutMsg(TraceMsg[T_Msg = T_MSG_RUN]);
            if (TraceOpt & TRACE_ENABLE)
                OutTraceMsg(T_Msg = T_MSG_RUN);
            GoUntil(0xFFFFFFFF); // means 'go forever'
            StopRun = 0;
            //    Invalidate();                   // registers, caches, etc. // in GoUntil
            bNoCache = 1; // JR, 10.11.2016: Extend cache bypass until AG_RUNSTOP invalidated caches
            iRun     = 0; // clear 'executing' flag
            NumRecs  = 0; // clear 'number of trace records'
            if ((TraceOpt & TRACE_ENABLE) && (T_Msg == T_MSG_RUN))
                OutMsg("");
            break;

#if DBGCM_WITHOUT_STOP
        case AG_WAITFORBRK:          // wait in run mode forever or till some Bp reached. Target already running.
            SetupTmpBrk(0xFFFFFFFF); // Setup Temporary BreakPoint and BreakPoint list
            iRun = 1;
            RunningCB(RUNNING_DLL_READY);
            WaitUntil(); // wait forever
            StopRun = 0;
            //    Invalidate();                   // registers, caches, etc. // in GoUntil
            bNoCache = 1; // JR, 10.11.2016: Extend cache bypass until AG_RUNSTOP invalidated caches
            iRun     = 0; // clear 'executing' flag
            NumRecs  = 0; // clear 'number of trace records'
            break;
#endif // DBGCM_WITHOUT_STOP
    }

    if (ReInit) {
#if DBGCM_DBG_DESCRIPTION
        // Pass new configuration to PDSC Setup, delayed version if target was running (not really part of ReInitTarget(), so do it here)
        if (PDSCDebug_IsEnabled()) {
            PDSCDebug_Reinit(); // Safe, only debug access vars are updated in non-Setup Mode
        }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_V8M
        if (ReInitTarget()) { // failed...
            SevereTargetError(SEV_ERR_REINIT);
        }
#else  // DBGCM_V8M
        if (ReInitTarget()) { // failed...
            SetPlayDead(&FatalReInitErr[0]);
        }
#endif // DBGCM_V8M
    }

    if (PlayDead) {     // target not connected
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (nE);
}



/*
 * Serial Window read/write
 */

_EXPO_ U32 AG_Serial(U16 nCode, U32 nSerNo, U32 nMany, void *vp)
{
    struct SerAS va;
    U32          status;

    if (PlayDead)
        return (0); // driver is disconnected.
    va.n1 = nSerNo; // 0:=Serial #1,  1:=Serial #2
    va.n2 = nMany;  // number of items to write

    switch (nCode) {
        case AG_SERBOUT:         // write nMany bytes to Serial #1 Window of uVision2
            va.v.pS = (SC8 *)vp; // content
            SendMessage(hMfrm, Uv2Msg, MSG_UV2_SERBOUT, (LPARAM)&va);
            break;

        case AG_SERWOUT:         // write nMany words to Serial #1 Window
            va.v.pW = (U16 *)vp; // content
            SendMessage(hMfrm, Uv2Msg, MSG_UV2_SERWOUT, (LPARAM)&va);
            break;

        case AG_SERXIN: // Key was pressed in Serial Window #1 or #2
                        //    WriteToSerPort (nSerNo, (char *) vp, nMany); // Output to serial interface
                        //    nSerno := Window number (#0 or #1)
                        //    vp     := &character(s) to send
                        //    nMany  := number of characters to send
            if (nSerNo == 3) {
                // ...
            } else {
                MessageBeep(MB_OK); // play sound (only Serial window RTA is used)
            }
            break;

        case AG_ITMBOUT:         // ITM-Channel byte data for logging  /5.1.2011/
            va.n3   = 1;         // 1:=Byte data
            va.v.pU = (UC8 *)vp; // content
            if (pCbFunc) {
                status = pCbFunc(AG_CB_ITMDATA, (void *)&va); // send to SarmCM3
                return (status);                              // 1:=ITM-ch#n logging was successful
            }                                                 // 0:=ITM-logging disabled / invalid channel parameter
            break;
    }
    return (0);
}


#if 0 // Example on how to output a string to uVision's serial window #1:
  char szSerTxt[] = "this should appear in serial window #1";
  AG_Serial (AG_SERBOUT, 0, sizeof (szSerTxt) - 1, (void *) szSerTxt);
#endif



/*
 * Calculate Trace History
 */

#define MAXTRC 256 // assume 256 (use a power of two value)

#if 0
static DWORD TracePC [MAXTRC];      // trace code-address buffer


static int CalcNumRecords (void)  {
  if (NumRecs == 0)  {              // seems to be invalidated

//---just a test vector:
    TracePC [0] = 0x0000000;        // Slot[0] not used.
    TracePC [1] = 0x0000047;
    TracePC [2] = 0x0000049;
    TracePC [3] = 0x000004B;
    TracePC [4] = 0x000004C;
    TracePC [5] = 0x0000050;
    TracePC [6] = 0x0000050;
    TracePC [7] = 0x0000050;
    TracePC [8] = 0x0000050;
    TracePC [9] = 0x0000050;
    TracePC[10] = 0x0000050;
    NumRecs = 10;                   // assume 10 entries

//  Calc:
//  TracePC [0] := unused
//  TracePC [1] := newest code address
//    ..
//  TracePC [n] := oldest code address  ( n >= 0, n < MAXTRC )
  }
  return (NumRecs);
}
#endif


/*
 * Trace-History access function:
 *   - the first call is always nCode 3 for 'query number of records'
 *   - then nCode 0 / 1 are activated.
 * Note: Emulator hardware must support tracing
 */

_EXPO_ U32 AG_HistFunc(U32 nCode, I32 indx, I32 dir, void *vp)
{
    U32 nR = 0;

    if (PlayDead)
        return (0);

#if 0
  switch (nCode)  {
    case 0:                   // Function #0: get Hist index
      if (NumRecs == 0) break;
      if (dir)  {             // dir, 1:=towards older entries
        if (indx < NumRecs)  {
          nR = indx + 1;
        }
      }
      else  {                 // dir, 0:=towards newer entries
        if (indx > 0) nR = indx - 1;
        else          nR = 0;
      }
      break;

    case 1:                   // Function #1: GetTracePC
      if (indx >= 0 && indx < MAXTRC)  {
        nR  = TracePC [indx]; // get PC of trace record 'indx'
      }
      break;

    case 2:                   // Function #2: GetHistRegs
// this function should be ignored since regs are not recorded anyway.
      break;                  // get recorded regs of record 'indx'

    case 3:                   // Function #3: get Nr. of entries
      if (iRun) return (0);   // block view-trace while running

//   - Find out the number of instructions based on the TRACE-Sfrs up to,
//     but not including the current PC
//***NOTE: make sure that CalcNumRecords() is as fast as possible since
//         it is called many times due to Billy's OnUpdate() handling.
      NumRecs = CalcNumRecords();
      nR      = NumRecs;      // return-value.
      break;
  }
#endif

    if (PlayDead == 1) {
        ExecPlayDead(); // show error message box and disconnect the target
    }
    return (nR);
}

/*
 * Breakpoint Resource Allocation function:
 *  - Optional, if not implemented, breakpoint resources are allocated
 *    when running target.
 *  - Returns 0 on success, otherwise error code.
 *
 */
U32 /*_EXPO_*/ AG_BreakResAlloc(U32 nAlloc, AG_BP *pBp)
{
    U32    nR        = 0;
    DWORD *pB        = NULL;
    DWORD  nExpected = 0;

    // DLL Specific Stuff, WBREAK expression needs to be analyzed first
    if (pBp->type == WBREAK) {
        // Calculate the watch expression and update the breakpoint
        switch (etree_data_watch(pBp)) {
            case 1: // address only
                // do nothing
                break;
            case 2: // address + value matching in hardware
                pBp->BP_INFO |= BP_INFO_WVAL;
                break;
            case 3:                              // address with condition check in software
                pBp->BP_INFO &= ~(BP_INFO_WVAL); // clear in case someone set it
                break;
            case 0:
            default:
                // no need to continue the check/allocation
                return RES_ERR_INVALID_EXPR;
        }
    }

    return AllocBreakResources(nAlloc, pBp);
}

/*
 * Register View Management
 */

struct rGroup rGroups[] = {
    {
        0x00,
        0x03,
        "Core",
    }, // Group 0 (Core), show expanded and Bold
    {
        0x00,
        0x00,
        "Banked",
    }, // Group 1 (Banked), collapsed
    {
        0x00,
        0x00,
        "System",
    }, // Group 2 (System), collapsed
    {
        0x00,
        0x01,
        "Internal",
    }, // Group 3 (Internal), show expanded
};

struct rGroup rGroupsF[] = {
    {
        0x00,
        0x03,
        "Core",
    }, // Group 0 (Core), show expanded and Bold
    {
        0x00,
        0x00,
        "Banked",
    }, // Group 1 (Banked), collapsed
    {
        0x00,
        0x00,
        "System",
    }, // Group 2 (System), collapsed
    {
        0x00,
        0x01,
        "Internal",
    }, // Group 3 (Internal), show expanded
    {
        0x00,
        0x00,
        "FPU",
    }, // Group 4 (FPU), collapsed
};

struct rItem rItems_CM4F_CM7F[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x106,
        "GE",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        2,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x34,
        "Sec",
        0,
        0,
        0,
    },

    {
        0x01,
        4,
        0x400,
        "S<n>",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x40,
        "S0",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x41,
        "S1",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x42,
        "S2",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x43,
        "S3",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x44,
        "S4",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x45,
        "S5",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x46,
        "S6",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x47,
        "S7",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x48,
        "S8",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x49,
        "S9",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x4A,
        "S10",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x4B,
        "S11",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x4C,
        "S12",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x4D,
        "S13",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x4E,
        "S14",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x4F,
        "S15",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x50,
        "S16",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x51,
        "S17",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x52,
        "S18",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x53,
        "S19",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x54,
        "S20",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x55,
        "S21",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x56,
        "S22",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x57,
        "S23",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x58,
        "S24",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x59,
        "S25",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x5A,
        "S26",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x5B,
        "S27",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x5C,
        "S28",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x5D,
        "S29",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x5E,
        "S30",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x5F,
        "S31",
        0,
        1,
        0,
    },

    {
        0x01,
        4,
        0x600,
        "D<n>",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x60,
        "D0",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x61,
        "D1",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x62,
        "D2",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x63,
        "D3",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x64,
        "D4",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x65,
        "D5",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x66,
        "D6",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x67,
        "D7",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x68,
        "D8",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x69,
        "D9",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x6A,
        "D10",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x6B,
        "D11",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x6C,
        "D12",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x6D,
        "D13",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x6E,
        "D14",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x6F,
        "D15",
        0,
        1,
        0,
    },


    {
        0x01,
        4,
        0x700,
        "Float",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x70,
        "S0",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x71,
        "S1",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x72,
        "S2",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x73,
        "S3",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x74,
        "S4",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x75,
        "S5",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x76,
        "S6",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x77,
        "S7",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x78,
        "S8",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x79,
        "S9",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x7A,
        "S10",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x7B,
        "S11",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x7C,
        "S12",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x7D,
        "S13",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x7E,
        "S14",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x7F,
        "S15",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x80,
        "S16",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x81,
        "S17",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x82,
        "S18",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x83,
        "S19",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x84,
        "S20",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x85,
        "S21",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x86,
        "S22",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x87,
        "S23",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x88,
        "S24",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x89,
        "S25",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x8A,
        "S26",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x8B,
        "S27",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x8C,
        "S28",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x8D,
        "S29",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x8E,
        "S30",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x8F,
        "S31",
        0,
        1,
        0,
    },

    {
        0x01,
        4,
        0x900,
        "Double",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x90,
        "D0",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x91,
        "D1",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x92,
        "D2",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x93,
        "D3",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x94,
        "D4",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x95,
        "D5",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x96,
        "D6",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x97,
        "D7",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x98,
        "D8",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x99,
        "D9",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x9A,
        "D10",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x9B,
        "D11",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x9C,
        "D12",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x9D,
        "D13",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x9E,
        "D14",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0x9F,
        "D15",
        0,
        1,
        0,
    },

    {
        0x01,
        4,
        0xA00,
        "FPSCR",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA01,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA02,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA03,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA04,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA05,
        "AHP",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA06,
        "DN",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA07,
        "FZ",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA08,
        "RMode",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA09,
        "IDC",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA0A,
        "IXC",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA0B,
        "UFC",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA0C,
        "OFC",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA0D,
        "DZC",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA0E,
        "IOC",
        0,
        1,
        0,
    },
    {
        0x01,
        4,
        0xA0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

};

struct rItem rItems_CM4_CM7[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x106,
        "GE",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        2,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x34,
        "Sec",
        0,
        0,
        0,
    },

};

struct rItem rItems_CM3[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        2,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x34,
        "Sec",
        0,
        0,
        0,
    },

};

struct rItem rItems_CM1[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        2,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x32,
        "Stack",
        0,
        0,
        0,
    },

};


struct rItem rItems_SC000[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        2,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x32,
        "Stack",
        0,
        0,
        0,
    },

};

#if DBGCM_V8M
/*
* v8-M Register View Management
*/

struct rGroup rGroups_v8M[] = {
    {
        0x00,
        0x03,
        "Core",
    }, // Group 0 (Core), show expanded and Bold
    {
        0x00,
        0x00,
        "Banked",
    }, // Group 1 (Banked), collapsed
    {
        0x00,
        0x01,
        "Internal",
    }, // Group 2 (Internal), show expanded
};

struct rGroup rGroups_v8MS[] = {
    {
        0x00,
        0x01,
        "Core",
    }, // Group 0 (Core), show expanded and Bold
    {
        0x00,
        0x00,
        "Banked",
    }, // Group 1 (Banked), collapsed
    {
        0x00,
        0x00,
        "Secure",
    }, // Group 2 (Secure), collapsed
    {
        0x00,
        0x00,
        "Non-Secure",
    }, // Group 3 (Non-Secure), collapsed
    {
        0x00,
        0x01,
        "Internal",
    }, // Group 4 (Internal), show expanded
};

struct rGroup rGroupsF_v8M[] = {
    {
        0x00,
        0x03,
        "Core",
    }, // Group 0 (Core), show expanded and Bold
    {
        0x00,
        0x00,
        "Banked",
    }, // Group 1 (Banked), collapsed
    {
        0x00,
        0x01,
        "Internal",
    }, // Group 2 (Internal), show expanded
    {
        0x00,
        0x00,
        "FPU",
    }, // Group 3 (FPU), collapsed
};

struct rGroup rGroupsF_v8MS[] = {
    {
        0x00,
        0x01,
        "Core",
    }, // Group 0 (Core), show expanded and Bold
    {
        0x00,
        0x00,
        "Banked",
    }, // Group 1 (Banked), collapsed
    {
        0x00,
        0x00,
        "Secure",
    }, // Group 3 (Secure), collapsed
    {
        0x00,
        0x00,
        "Non-Secure",
    }, // Group 4 (Non-Secure), collapsed
    {
        0x00,
        0x01,
        "Internal",
    }, // Group 2 (Internal), show expanded
    {
        0x00,
        0x00,
        "FPU",
    }, // Group 5 (FPU), collapsed
};

struct rItem rItems_v8M_B[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        1,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x20F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        2,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
};

struct rItem rItems_v8M_M[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x106,
        "GE",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0xC2,
        "MSPLIM",
        0,
        1,
        0,
    }, // 04.04.2019
    {
        0x01,
        1,
        0xC3,
        "PSPLIM",
        0,
        1,
        0,
    }, // 04.04.2019

    {
        0x01,
        1,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x20F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        2,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x34,
        "Sec",
        0,
        0,
        0,
    },
};


struct rItem rItems_v8M_MF[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x106,
        "GE",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0xC2,
        "MSPLIM",
        0,
        1,
        0,
    }, // 04.04.2019
    {
        0x01,
        1,
        0xC3,
        "PSPLIM",
        0,
        1,
        0,
    }, // 04.04.2019

    {
        0x01,
        1,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x20F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        2,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        2,
        0x34,
        "Sec",
        0,
        0,
        0,
    },

    {
        0x01,
        3,
        0x400,
        "S<n>",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x40,
        "S0",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x41,
        "S1",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x42,
        "S2",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x43,
        "S3",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x44,
        "S4",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x45,
        "S5",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x46,
        "S6",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x47,
        "S7",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x48,
        "S8",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x49,
        "S9",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x4A,
        "S10",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x4B,
        "S11",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x4C,
        "S12",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x4D,
        "S13",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x4E,
        "S14",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x4F,
        "S15",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x50,
        "S16",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x51,
        "S17",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x52,
        "S18",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x53,
        "S19",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x54,
        "S20",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x55,
        "S21",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x56,
        "S22",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x57,
        "S23",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x58,
        "S24",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x59,
        "S25",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x5A,
        "S26",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x5B,
        "S27",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x5C,
        "S28",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x5D,
        "S29",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x5E,
        "S30",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x5F,
        "S31",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x600,
        "D<n>",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x60,
        "D0",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x61,
        "D1",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x62,
        "D2",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x63,
        "D3",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x64,
        "D4",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x65,
        "D5",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x66,
        "D6",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x67,
        "D7",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x68,
        "D8",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x69,
        "D9",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x6A,
        "D10",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x6B,
        "D11",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x6C,
        "D12",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x6D,
        "D13",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x6E,
        "D14",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x6F,
        "D15",
        0,
        1,
        0,
    },


    {
        0x01,
        3,
        0x700,
        "Float",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x70,
        "S0",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x71,
        "S1",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x72,
        "S2",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x73,
        "S3",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x74,
        "S4",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x75,
        "S5",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x76,
        "S6",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x77,
        "S7",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x78,
        "S8",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x79,
        "S9",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x7A,
        "S10",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x7B,
        "S11",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x7C,
        "S12",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x7D,
        "S13",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x7E,
        "S14",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x7F,
        "S15",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x80,
        "S16",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x81,
        "S17",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x82,
        "S18",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x83,
        "S19",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x84,
        "S20",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x85,
        "S21",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x86,
        "S22",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x87,
        "S23",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x88,
        "S24",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x89,
        "S25",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x8A,
        "S26",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x8B,
        "S27",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x8C,
        "S28",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x8D,
        "S29",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x8E,
        "S30",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x8F,
        "S31",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0x900,
        "Double",
        0,
        0,
        0,
    },
    {
        0x01,
        3,
        0x90,
        "D0",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x91,
        "D1",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x92,
        "D2",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x93,
        "D3",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x94,
        "D4",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x95,
        "D5",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x96,
        "D6",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x97,
        "D7",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x98,
        "D8",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x99,
        "D9",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x9A,
        "D10",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x9B,
        "D11",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x9C,
        "D12",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x9D,
        "D13",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x9E,
        "D14",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0x9F,
        "D15",
        0,
        1,
        0,
    },

    {
        0x01,
        3,
        0xA00,
        "FPSCR",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA01,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA02,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA03,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA04,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA05,
        "AHP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA06,
        "DN",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA07,
        "FZ",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA08,
        "RMode",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA09,
        "IDC",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA0A,
        "IXC",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA0B,
        "UFC",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA0C,
        "OFC",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA0D,
        "DZC",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA0E,
        "IOC",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xA0F,
        "",
        0,
        0,
        0,
    }, // Delimiter
};


struct rItem rItems_v8M_BS[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        1,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x20F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        2,
        0xB0,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB1,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB2,
        "MSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB3,
        "PSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB5,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB7,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        3,
        0xC0,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC1,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC5,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC7,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        4,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
};

struct rItem rItems_v8M_MS[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x106,
        "GE",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        1,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x20F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        2,
        0xB0,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB1,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB2,
        "MSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB3,
        "PSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB4,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB5,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB6,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB7,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        3,
        0xC0,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC1,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC2,
        "MSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC3,
        "PSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC4,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC5,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC6,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC7,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        4,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x34,
        "Sec",
        0,
        0,
        0,
    },
};

struct rItem rItems_v8M_MSF[] = {
    //--desc-nGi-nItem-szReg[]---isPC-cc-iHig--
    {
        0x01,
        0,
        0x00,
        "R0",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x01,
        "R1",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x02,
        "R2",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x03,
        "R3",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x04,
        "R4",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x05,
        "R5",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x06,
        "R6",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x07,
        "R7",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x08,
        "R8",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x09,
        "R9",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0A,
        "R10",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0B,
        "R11",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0C,
        "R12",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0D,
        "R13 (SP)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0E,
        "R14 (LR)",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x0F,
        "R15 (PC)",
        1,
        1,
        0,
    },
    {
        0x01,
        0,
        0x100,
        "xPSR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x101,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x102,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x103,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x104,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x105,
        "Q",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x106,
        "GE",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x107,
        "T",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x108,
        "IT",
        0,
        0,
        0,
    },
    {
        0x01,
        0,
        0x109,
        "ISR",
        0,
        1,
        0,
    },
    {
        0x01,
        0,
        0x10F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        1,
        0x11,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x12,
        "PSP",
        0,
        1,
        0,
    },

    {
        0x01,
        1,
        0x20,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x21,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x22,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x23,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        1,
        0x20F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        2,
        0xB0,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB1,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB2,
        "MSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB3,
        "PSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB4,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB5,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB6,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB7,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        2,
        0xB0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        3,
        0xC0,
        "MSP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC1,
        "PSP",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC2,
        "MSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC3,
        "PSPLIM",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC4,
        "BASEPRI",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC5,
        "PRIMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC6,
        "FAULTMASK",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC7,
        "CONTROL",
        0,
        1,
        0,
    },
    {
        0x01,
        3,
        0xC0F,
        "",
        0,
        0,
        0,
    }, // Delimiter

    {
        0x01,
        4,
        0x30,
        "Mode",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x31,
        "Privilege",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x32,
        "Stack",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x33,
        "States",
        0,
        0,
        0,
    },
    {
        0x01,
        4,
        0x34,
        "Sec",
        0,
        0,
        0,
    },


    {
        0x01,
        5,
        0x400,
        "S<n>",
        0,
        0,
        0,
    },
    {
        0x01,
        5,
        0x40,
        "S0",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x41,
        "S1",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x42,
        "S2",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x43,
        "S3",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x44,
        "S4",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x45,
        "S5",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x46,
        "S6",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x47,
        "S7",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x48,
        "S8",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x49,
        "S9",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x4A,
        "S10",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x4B,
        "S11",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x4C,
        "S12",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x4D,
        "S13",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x4E,
        "S14",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x4F,
        "S15",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x50,
        "S16",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x51,
        "S17",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x52,
        "S18",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x53,
        "S19",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x54,
        "S20",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x55,
        "S21",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x56,
        "S22",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x57,
        "S23",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x58,
        "S24",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x59,
        "S25",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x5A,
        "S26",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x5B,
        "S27",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x5C,
        "S28",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x5D,
        "S29",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x5E,
        "S30",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x5F,
        "S31",
        0,
        1,
        0,
    },

    {
        0x01,
        5,
        0x600,
        "D<n>",
        0,
        0,
        0,
    },
    {
        0x01,
        5,
        0x60,
        "D0",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x61,
        "D1",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x62,
        "D2",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x63,
        "D3",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x64,
        "D4",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x65,
        "D5",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x66,
        "D6",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x67,
        "D7",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x68,
        "D8",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x69,
        "D9",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x6A,
        "D10",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x6B,
        "D11",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x6C,
        "D12",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x6D,
        "D13",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x6E,
        "D14",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x6F,
        "D15",
        0,
        1,
        0,
    },


    {
        0x01,
        5,
        0x700,
        "Float",
        0,
        0,
        0,
    },
    {
        0x01,
        5,
        0x70,
        "S0",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x71,
        "S1",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x72,
        "S2",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x73,
        "S3",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x74,
        "S4",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x75,
        "S5",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x76,
        "S6",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x77,
        "S7",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x78,
        "S8",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x79,
        "S9",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x7A,
        "S10",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x7B,
        "S11",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x7C,
        "S12",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x7D,
        "S13",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x7E,
        "S14",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x7F,
        "S15",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x80,
        "S16",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x81,
        "S17",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x82,
        "S18",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x83,
        "S19",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x84,
        "S20",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x85,
        "S21",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x86,
        "S22",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x87,
        "S23",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x88,
        "S24",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x89,
        "S25",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x8A,
        "S26",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x8B,
        "S27",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x8C,
        "S28",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x8D,
        "S29",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x8E,
        "S30",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x8F,
        "S31",
        0,
        1,
        0,
    },

    {
        0x01,
        5,
        0x900,
        "Double",
        0,
        0,
        0,
    },
    {
        0x01,
        5,
        0x90,
        "D0",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x91,
        "D1",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x92,
        "D2",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x93,
        "D3",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x94,
        "D4",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x95,
        "D5",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x96,
        "D6",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x97,
        "D7",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x98,
        "D8",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x99,
        "D9",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x9A,
        "D10",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x9B,
        "D11",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x9C,
        "D12",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x9D,
        "D13",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x9E,
        "D14",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0x9F,
        "D15",
        0,
        1,
        0,
    },

    {
        0x01,
        5,
        0xA00,
        "FPSCR",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA01,
        "N",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA02,
        "Z",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA03,
        "C",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA04,
        "V",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA05,
        "AHP",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA06,
        "DN",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA07,
        "FZ",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA08,
        "RMode",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA09,
        "IDC",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA0A,
        "IXC",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA0B,
        "UFC",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA0C,
        "OFC",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA0D,
        "DZC",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA0E,
        "IOC",
        0,
        1,
        0,
    },
    {
        0x01,
        5,
        0xA0F,
        "",
        0,
        0,
        0,
    }, // Delimiter
};
#endif // DBGCM_V8M


struct bmsk {
    U32 mask;
    I32 shft;
};

static const struct bmsk bPSR[] = {
    // PSR
    {
        0x80000000,
        31,
    }, // N
    {
        0x40000000,
        30,
    }, // Z
    {
        0x20000000,
        29,
    }, // C
    {
        0x10000000,
        28,
    }, // V
    {
        0x08000000,
        27,
    }, // Q
    {
        0x000F0000,
        16,
    }, // GE
    {
        0x01000000,
        24,
    }, // T
    {
        0x0600FC00,
        10,
    }, // IT
    {
        0x000001FF,
        0,
    }, // ISR
};

static const struct bmsk bFPSCR[] = {
    // FPSCR
    {
        0x80000000,
        31,
    }, // N
    {
        0x40000000,
        30,
    }, // Z
    {
        0x20000000,
        29,
    }, // C
    {
        0x10000000,
        28,
    }, // V
    {
        0x04000000,
        26,
    }, // AHP
    {
        0x02000000,
        25,
    }, // DN
    {
        0x01000000,
        24,
    }, // FZ
    {
        0x00C00000,
        22,
    }, // RMode
    {
        0x00000080,
        7,
    }, // IDC
    {
        0x00000010,
        4,
    }, // IXC
    {
        0x00000008,
        3,
    }, // UFC
    {
        0x00000004,
        2,
    }, // OFC
    {
        0x00000002,
        1,
    }, // DZC
    {
        0x00000001,
        0,
    }, // IOC
};


static BYTE       nRmode;
static RgARMCM    uRg;      // shadow registers
static RgARMFPU   uRgF;     // shadow registers (FPU)
static RgARMV8MSE uRgS;     // shadow registers (v8-M Security Extensions)
static char       szV[128]; // temporary buffer


/*
 * uVision want's to display some registers value...
 */

static void RegGet(RITEM *vp, int nR)
{
    GVAL   v;
    U32    n, z;
    double dr;
    char  *p1;

    // if (PlayDead) return;                    // target is disconnected...
    if (PlayDead) {
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return;         // target is disconnected...
    }

    switch (nR & 0xF0000000) {
        case UPR_NORMAL: // Setup Normal Regs

#if DBGCM_V8M
            if (xFPU) {
                if (pio->bSecureExt) {
                    GetRegs(mREGS | mRFPU | mRSEC); // update our local regs
                } else {
                    GetRegs(mREGS | mRFPU | xMainline ? (nMSPLIM_NS | nPSPLIM_NS) : 0); // update our local regs, 04.04.2019
                }
            } else {
                if (pio->bSecureExt) {
                    GetRegs(mREGS | mRSEC); // update our local regs
                } else {
                    GetRegs(mREGS | xMainline ? (nMSPLIM_NS | nPSPLIM_NS) : 0); // update our local regs, 04.04.2019
                }
            }
            UpdateSecurityState();       // Read current security state from target
            UpdateDebugAuthentication(); // Read debug authentication status from target
            UpdateCycles();
            uRg    = RegARM;   // capture current regs
            uRgF   = RegFPU;   // capture current regs (FPU)
            uRgS   = RegV8MSE; // capture current regs (v8-M Security Extensions)
            nRmode = 0;        // normal mode

            if (PlayDead) {     // target died...
                ExecPlayDead(); // show error message box and disconnect the target
            }
#else  // DBGCM_V8M
            if (xFPU) {
                GetRegs(mREGS | mRFPU); // update our local regs
            } else {
                GetRegs(mREGS); // update our local regs
            }
            UpdateCycles();
            uRg = RegARM;  // capture current regs
            uRgF = RegFPU; // capture current regs (FPU)
            nRmode = 0;    // normal mode

            if (PlayDead) {     // target died...
                ExecPlayDead(); // show error message box and disconnect the target
            }
#endif // DBGCM_V8M

            return;

        case UPR_HIST: // Setup History Regs
                       //--- currently not supported !
                       //    v.i32 = nR & 0xFFFF;                 // History index (max. 64K entries)
                       //    if (ioc.numtra != MAXTRC) v.i32 = (ioc.numtra - v.i32) & (MAXTRC - 1);
                       //    else                      v.i32 = (ioc.curtra - v.i32) & (MAXTRC - 1);
                       //    uRg    = *(((struct RgARMCM *) ioc.trap) + v.i32);  // HistRegs
                       //    nRmode = 1;                          // history mode
            return;
    }

    v.u64     = 0;
    vp->iDraw = 0;

    switch (vp->nItem) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x11:
        case 0x12:
            v.u32 = uRg.Rn[vp->nItem]; // R0..R15, MSP, PSP
        x32:
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) { // value changed
                sprintf(vp->szVal, "0x%08X", v.u32);
            }
            break;

#if 0
    case 0x0F:                                             // R15 (PC)
      v.u32 = uRg.Rn[15];
      if (v.u32 != vp->v.u32 || vp->szVal[0] == 0)  {
        sprintf (vp->szVal, "0x%08X", v.u32);
        vp->iDraw = 1;
      }
      vp->v.u32 = v.u32;
      vp->iHigh = 0;                                       // never highlighted
      return;
#endif

        case 0x100:
            v.u32 = uRg.xPSR; // xPSR
            goto x32;
        case 0x101:                    // N
        case 0x102:                    // Z
        case 0x103:                    // C
        case 0x104:                    // V
        case 0x105:                    // Q
        case 0x107:                    // T
            n     = vp->nItem - 0x101; // use item-number as index
            z     = uRg.xPSR;
            v.u32 = (z & bPSR[n].mask) >> bPSR[n].shft; // masked normalized value
        xd:
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) { // value changed
                sprintf(vp->szVal, "%d       ", v.u32);    // spaces to ease mouse hit.
            }
            break;
        case 0x106: // GE
            z     = uRg.xPSR;
            v.u32 = (z >> 16) & 0x0F;
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                sprintf(vp->szVal, "0x%X", v.u32);
            }
            break;
        case 0x108: // IT
            z     = uRg.xPSR;
            v.u32 = ((z >> 8) & 0xFC) | ((z >> 25) & 0x03);
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                if (v.u32 == 0) {
                    sprintf(vp->szVal, "Disabled");
                } else if ((v.u32 & 0x0F) == 0) {
                    sprintf(vp->szVal, "???");
                } else {
                    for (z = 0; z < 4; z++) {
                        if (v.u32 & (1 << z))
                            break;
                    }
                    z            = 3 - z;
                    vp->szVal[0] = 'I';
                    vp->szVal[1] = 'T';
                    for (n = 0; n < z; n++) {
                        vp->szVal[2 + n] = (((v.u32 << (n + 1)) ^ v.u32) & 0x10) ? 'E' : 'T';
                    }
                    vp->szVal[2 + n] = ' ';
                    z                = 3 + n;
                    switch ((v.u32 >> 4) & 0x0F) {
                        case 0x00: sprintf(&vp->szVal[z], "EQ"); break;
                        case 0x01: sprintf(&vp->szVal[z], "NE"); break;
                        case 0x02: sprintf(&vp->szVal[z], "CS"); break;
                        case 0x03: sprintf(&vp->szVal[z], "CC"); break;
                        case 0x04: sprintf(&vp->szVal[z], "MI"); break;
                        case 0x05: sprintf(&vp->szVal[z], "PL"); break;
                        case 0x06: sprintf(&vp->szVal[z], "VS"); break;
                        case 0x07: sprintf(&vp->szVal[z], "VC"); break;
                        case 0x08: sprintf(&vp->szVal[z], "HI"); break;
                        case 0x09: sprintf(&vp->szVal[z], "LS"); break;
                        case 0x0A: sprintf(&vp->szVal[z], "GE"); break;
                        case 0x0B: sprintf(&vp->szVal[z], "LT"); break;
                        case 0x0C: sprintf(&vp->szVal[z], "GT"); break;
                        case 0x0D: sprintf(&vp->szVal[z], "LE"); break;
                        case 0x0E: sprintf(&vp->szVal[z], "AL"); break;
                        case 0x0F: sprintf(&vp->szVal[z], "NV"); break;
                    }
                }
            }
            break;
        case 0x109: // ISR
            z     = uRg.xPSR;
            v.u32 = z & 0x1FF;
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                sprintf(vp->szVal, "%d   ", v.u32);
            }
            break;

        case 0x20:
            v.u32 = uRg.SYS.R.BASEPRI; // BASEPRI
        x8:
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                sprintf(vp->szVal, "0x%02X", v.u32);
            }
            break;
        case 0x21:
            v.u32 = uRg.SYS.R.PRIMASK; // PRIMASK
            goto xd;
        case 0x22:
            v.u32 = uRg.SYS.R.FAULTMASK; // FAULTMASK
            goto xd;
        case 0x23:
            v.u32 = uRg.SYS.R.CONTROL; // CONTROL
            goto x8;

        case 0x30: // Mode

#if DBGCM_V8M
            v.u32 = (uRg.xPSR & 0x1FF) ? 1 : 0; // Mode Bits Change
            if (pio->bSecureExt) {
                v.u32 |= uRgS.SECURITY.STATE.isSecure ? 0x2 : 0; // Security State Change
            }
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                n = 0;
                if (pio->bSecureExt) {
                    switch (v.u32) {
                        case 0: sprintf(&(vp->szVal[n]), "Non-Secure Thread"); break;
                        case 1: sprintf(&(vp->szVal[n]), "Non-Secure Handler"); break;
                        case 2: sprintf(&(vp->szVal[n]), "Secure Thread"); break;
                        case 3: sprintf(&(vp->szVal[n]), "Secure Handler"); break;
                    }
                } else {
                    switch (v.u32) {
                        case 0: sprintf(&(vp->szVal[n]), "Thread"); break;
                        case 1: sprintf(&(vp->szVal[n]), "Handler"); break;
                    }
                }
            }
#else  // DBGCM_V8M
            v.u32 = (uRg.xPSR & 0x1FF) ? 1 : 0;
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                switch (v.u32) {
                    case 0: sprintf(vp->szVal, "Thread"); break;
                    case 1: sprintf(vp->szVal, "Handler"); break;
                }
            }
#endif // DBGCM_V8M

            break;

        case 0x31: // Privilege
            v.u32 = (((uRg.xPSR & 0x1FF) == 0) && (uRg.SYS.R.CONTROL & 0x01)) ? 1 : 0;
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                switch (v.u32) {
                    case 0: sprintf(vp->szVal, "Privileged"); break;
                    case 1: sprintf(vp->szVal, "Unprivileged"); break;
                }
            }
            break;

        case 0x32: // Stack
            v.u32 = (((uRg.xPSR & 0x1FF) == 0) && (uRg.SYS.R.CONTROL & 0x02)) ? 1 : 0;
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                switch (v.u32) {
                    case 0: sprintf(vp->szVal, "MSP"); break;
                    case 1: sprintf(vp->szVal, "PSP"); break;
                }
            }
            break;

        case 0x33: // States
            v.u64 = uRg.nCycles;
            if ((U64)uRg.nCycles != vp->v.u64 || vp->szVal[0] == 0) {
                sprintf(vp->szVal, "%I64u", uRg.nCycles);
                vp->iDraw = 1;
            }
            vp->v.u64 = uRg.nCycles; // Cycles as reference value
            vp->iHigh = 0;           // never highlighted
            return;

        case 0x34: // Seconds
            if ((U64)uRg.nCycles != vp->v.u64 || vp->szVal[0] == 0) {
                if (pio->OscFrq != 0) {
                    vp->iDraw = 1;
                    dr        = pio->pCalcTime(uRg.nCycles);
                    p1        = &szV[0];
                    if (v.u64 < 10000)
                        sprintf(p1, "%10.8f", dr);
                    else
                        sprintf(p1, "%10.8g", dr);
                    while (*p1 == ' ')
                        ++p1;
                    strcpy(vp->szVal, p1);
                } else
                    vp->szVal[0] = 0; // no OscFrq, no time !
            }
            vp->v.u64 = uRg.nCycles; // Cycles as reference value
            vp->iHigh = 0;           // never highlighted
            return;

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            v.u32 = uRgF.Sn[vp->nItem - 0x40];             // S0..S31 (Hex)
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) { // value changed
                sprintf(vp->szVal, "0x%08X", v.u32);
            }
            break;

        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
            uRgF.Sn[(vp->nItem - 0x60) * 2];
            v.u64 = *(UINT64 *)&uRgF.Sn[(vp->nItem - 0x60) * 2]; // D0..D15 (Hex)
            if (v.u64 != vp->v.u64 || vp->szVal[0] == 0) {       // value changed
                sprintf(vp->szVal, "0x%016llX", v.u64);
            }
            break;

        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
            v.u32 = uRgF.Sn[vp->nItem - 0x70];             // S0..S31 (Float)
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) { // value changed
                sprintf(vp->szVal, "%g", v.f32);
            }
            break;

        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            v.u64 = *(UINT64 *)&uRgF.Sn[(vp->nItem - 0x90) * 2]; // D0..D15 (Double)
            if (v.u64 != vp->v.u64 || vp->szVal[0] == 0) {       // value changed
                sprintf(vp->szVal, "%g", v.f64);
            }
            break;

        case 0xA00:
            v.u32 = uRgF.FPSCR; // FPSCR
            goto x32;
        case 0xA01:                    // N
        case 0xA02:                    // Z
        case 0xA03:                    // C
        case 0xA04:                    // V
        case 0xA05:                    // AHP
        case 0xA06:                    // DN
        case 0xA07:                    // FZ
        case 0xA09:                    // IDC
        case 0xA0A:                    // IXC
        case 0xA0B:                    // UFC
        case 0xA0C:                    // OFC
        case 0xA0D:                    // DZC
        case 0xA0E:                    // IOC
            n     = vp->nItem - 0xA01; // use item-number as index
            z     = uRgF.FPSCR;
            v.u32 = (z & bFPSCR[n].mask) >> bFPSCR[n].shft; // masked normalized value
            goto xd;
        case 0xA08: // RMode
            n     = vp->nItem - 0xA01;
            z     = uRgF.FPSCR;
            v.u32 = (z & bFPSCR[n].mask) >> bFPSCR[n].shft; // masked normalized value
            if (v.u32 != vp->v.u32 || vp->szVal[0] == 0) {
                switch (v.u32) {
                    case 0: sprintf(vp->szVal, "0: RN"); break;
                    case 1: sprintf(vp->szVal, "1: RP"); break;
                    case 2: sprintf(vp->szVal, "2: RM"); break;
                    case 3: sprintf(vp->szVal, "3: RZ"); break;
                }
            }
            break;

#if DBGCM_V8M
            // Banked Secure Registers
        case 0xB00: // Secure "Group Item"
            if (RegV8MSE.SECURITY.STATE.isSecure && !vp->iHigh) {
                vp->iHigh = 0x81; // Highlight with bold font
                vp->iDraw = 1;
            } else if (!RegV8MSE.SECURITY.STATE.isSecure && vp->iHigh) {
                vp->iHigh = 0;
                vp->iDraw = 1;
            }
            return; // Don't go through general update algorithm after switch-stmt
        case 0xB0:
            v.u32 = uRgS.MSP_S;
            goto x32;
        case 0xB1:
            v.u32 = uRgS.PSP_S;
            goto x32;
        case 0xB2:
            v.u32 = uRgS.MSPLIM_S;
            goto x32;
        case 0xB3:
            v.u32 = uRgS.PSPLIM_S;
            goto x32;
        case 0xB4:
            v.u32 = uRgS.SYS_S.R.BASEPRI;
            goto x8;
        case 0xB5:
            v.u32 = uRgS.SYS_S.R.PRIMASK;
            goto xd;
        case 0xB6:
            v.u32 = uRgS.SYS_S.R.FAULTMASK;
            goto xd;
        case 0xB7:
            v.u32 = uRgS.SYS_S.R.CONTROL;
            goto x8;

            // Banked Non-Secure Registers
        case 0xC00: // Non-Secure "Group Item"
            if (RegV8MSE.SECURITY.STATE.isSecure && vp->iHigh) {
                vp->iHigh = 0;
                vp->iDraw = 1;
            } else if (!RegV8MSE.SECURITY.STATE.isSecure && !vp->iHigh) {
                vp->iHigh = 0x81; // Highlight with bold font
                vp->iDraw = 1;
            }
            return; // Don't go through general update algorithm after switch-stmt
        case 0xC0:
            v.u32 = uRgS.MSP_NS;
            goto x32;
        case 0xC1:
            v.u32 = uRgS.PSP_NS;
            goto x32;
        case 0xC2:
            v.u32 = uRgS.MSPLIM_NS;
            goto x32;
        case 0xC3:
            v.u32 = uRgS.PSPLIM_NS;
            goto x32;
        case 0xC4:
            v.u32 = uRgS.SYS_NS.R.BASEPRI;
            goto x8;
        case 0xC5:
            v.u32 = uRgS.SYS_NS.R.PRIMASK;
            goto xd;
        case 0xC6:
            v.u32 = uRgS.SYS_NS.R.FAULTMASK;
            goto xd;
        case 0xC7:
            v.u32 = uRgS.SYS_NS.R.CONTROL;
            goto x8;
#endif // DBGCM_V8M
    }

    if (v.u32 == vp->v.u32) { // value unchanged
        if (vp->iHigh) {      // item is currently highlighted
            vp->iHigh = 0;    // clear highlight color
            vp->iDraw = 1;    // need repaint.
        }
    } else {           // value changed
        vp->iHigh = 1; // highlight
        vp->iDraw = 1; // need repaint
    }
    vp->v.u64 = v.u64; // update 'On Screen' value
}


/*
 * uVision want's to change some registers value...
 */

static I32 RegSet(RITEM *vp, GVAL *pV)
{
    I32   n;
    U32   val;
    float f;

    if (nRmode == 1)
        return (0); // Error: can't change HistRegs !
    // if (PlayDead)    return (0);       // cancel: driver is disconnected...
    if (PlayDead) {
        ExecPlayDead(); // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (0);     // cancel: driver is disconnected...
    }
    if (iRun)
        return (0); // cancel: currently running...

    switch (vp->nItem) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x11:
        case 0x12:

#if DBGCM_V8M
            uRg.Rn[vp->nItem] = pV->u32;
            SetRegs(&uRg, NULL, NULL, 1ULL << vp->nItem);
#else  // DBGCM_V8M
            uRg.Rn[vp->nItem] = pV->u32;
            SetRegs(&uRg, NULL, 1ULL << vp->nItem);
#endif // DBGCM_V8M

            break;

        case 0x100: // xPSR

#if DBGCM_V8M
            uRg.xPSR = pV->u32;
        sx:
            SetRegs(&uRg, NULL, NULL, 1ULL << nPSR);
#else  // DBGCM_V8M
            uRg.xPSR = pV->u32;
        sx:
            SetRegs(&uRg, NULL, 1ULL << nPSR);
#endif // DBGCM_V8M

            break;
        case 0x101:                  // N
        case 0x102:                  // Z
        case 0x103:                  // C
        case 0x104:                  // V
        case 0x105:                  // Q
        case 0x106:                  // GE
        case 0x107:                  // T
        case 0x109:                  // ISR
            n   = vp->nItem - 0x101; // use Item-number for index
            val = pV->u32;
            val = (val << bPSR[n].shft) & bPSR[n].mask;
            uRg.xPSR &= ~bPSR[n].mask;
            uRg.xPSR |= val;
            goto sx;
        case 0x108: // IT: can't change
            return (0);

        case 0x20: // BASEPRI

#if DBGCM_V8M
            uRg.SYS.R.BASEPRI = pV->uc;
        ss:
            SetRegs(&uRg, NULL, NULL, 1ULL << nSYS);
#else  // DBGCM_V8M
            uRg.SYS.R.BASEPRI = pV->uc;
        ss:
            SetRegs(&uRg, NULL, 1ULL << nSYS);
#endif // DBGCM_V8M

            break;
        case 0x21: // PRIMASK
            uRg.SYS.R.PRIMASK = pV->uc;
            goto ss;
        case 0x22: // FAULTMASK
            uRg.SYS.R.FAULTMASK = pV->uc;
            goto ss;
        case 0x23: // CONTROL
            uRg.SYS.R.CONTROL = pV->uc;
            goto ss;

        case 0x30: // Mode, can't change
        case 0x31: // Privilege, can't change
        case 0x32: // Stack, can't change
        case 0x33: // States, can't change
        case 0x34: // Sec, can't change
            return (0);

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:

#if DBGCM_V8M
            n          = vp->nItem - 0x40;
            uRgF.Sn[n] = pV->u32;
            SetRegs(NULL, &uRgF, NULL, 1ULL << (n + nFPUSx));
#else  // DBGCM_V8M
            n = vp->nItem - 0x40;
            uRgF.Sn[n] = pV->u32;
            SetRegs(NULL, &uRgF, 1ULL << (n + nFPUSx));
#endif // DBGCM_V8M

            break;

        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
#if DBGCM_V8M
        {
            n          = (vp->nItem - 0x60) * 2;
            uRgF.Sn[n] = (DWORD)pV->u64;
            SetRegs(NULL, &uRgF, NULL, 1ULL << (n + nFPUSx));
            uRgF.Sn[n + 1] = (DWORD)(pV->u64 >> 32);
            SetRegs(NULL, &uRgF, NULL, 1ULL << ((n + 1) + nFPUSx));
        }
#else  // DBGCM_V8M
        {
            n = (vp->nItem - 0x60) * 2;
            uRgF.Sn[n] = (DWORD)pV->u64;
            SetRegs(NULL, &uRgF, 1ULL << (n + nFPUSx));
            uRgF.Sn[n + 1] = (DWORD)(pV->u64 >> 32);
            SetRegs(NULL, &uRgF, 1ULL << ((n + 1) + nFPUSx));
        }
#endif // DBGCM_V8M

        break;

        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
            n = vp->nItem - 0x70;
            if (fabs(pV->f64) < 1e-50) {
                f = (float)pV->i32;
            } else {
                f = (float)pV->f64;
            }

#if DBGCM_V8M
            uRgF.Sn[n] = *((DWORD *)&f);
            SetRegs(NULL, &uRgF, NULL, 1ULL << (n + nFPUSx));
#else  // DBGCM_V8M
            uRgF.Sn[n] = *((DWORD *)&f);
            SetRegs(NULL, &uRgF, 1ULL << (n + nFPUSx));
#endif // DBGCM_V8M

            break;

        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F: {
            double d;
            d = (double)pV->f64;
            n = (vp->nItem - 0x90) * 2;

#if DBGCM_V8M
            uRgF.Sn[n] = (DWORD)(*((UINT64 *)&d));
            SetRegs(NULL, &uRgF, NULL, 1ULL << (n + nFPUSx));
            uRgF.Sn[n + 1] = (DWORD)((*((UINT64 *)&d)) >> 32);
            SetRegs(NULL, &uRgF, NULL, 1ULL << ((n + 1) + nFPUSx));
#else  // DBGCM_V8M
            uRgF.Sn[n] = (DWORD)(*((UINT64 *)&d));
            SetRegs(NULL, &uRgF, 1ULL << (n + nFPUSx));
            uRgF.Sn[n + 1] = (DWORD)((*((UINT64 *)&d)) >> 32);
            SetRegs(NULL, &uRgF, 1ULL << ((n + 1) + nFPUSx));
#endif // DBGCM_V8M

        } break;

        case 0xA00: // FPCSR

#if DBGCM_V8M
            uRgF.FPSCR = pV->u32;
        sf:
            SetRegs(NULL, &uRgF, NULL, 1ULL << nFPSCR);
#else  // DBGCM_V8M
            uRgF.FPSCR = pV->u32;
        sf:
            SetRegs(NULL, &uRgF, 1ULL << nFPSCR);
#endif // DBGCM_V8M

            break;
        case 0xA01:                  // N
        case 0xA02:                  // Z
        case 0xA03:                  // C
        case 0xA04:                  // V
        case 0xA05:                  // AHP
        case 0xA06:                  // DN
        case 0xA07:                  // FZ
        case 0xA08:                  // RMode
        case 0xA09:                  // IDC
        case 0xA0A:                  // IXC
        case 0xA0B:                  // UFC
        case 0xA0C:                  // OFC
        case 0xA0D:                  // DZC
        case 0xA0E:                  // IOC
            n   = vp->nItem - 0xA01; // use Item-number for index
            val = pV->u32;
            val = (val << bFPSCR[n].shft) & bFPSCR[n].mask;
            uRgF.FPSCR &= ~bFPSCR[n].mask;
            uRgF.FPSCR |= val;
            goto sf;

#if DBGCM_V8M
        // Banked Secure Registers
        case 0xB0:
            uRgS.MSP_S = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nMSP_S);
            break;
        case 0xB1:
            uRgS.PSP_S = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nPSP_S);
            break;
        case 0xB2:
            uRgS.MSPLIM_S = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nMSPLIM_S);
            break;
        case 0xB3:
            uRgS.PSPLIM_S = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nPSPLIM_S);
            break;
        case 0xB4:
            uRgS.SYS_S.R.BASEPRI = pV->uc;
        sbss:
            SetRegs(NULL, NULL, &uRgS, 1ULL << nSYS_S);
            break;
        case 0xB5:
            uRgS.SYS_S.R.PRIMASK = pV->uc;
            goto sbss;
        case 0xB6:
            uRgS.SYS_S.R.FAULTMASK = pV->uc;
            goto sbss;
        case 0xB7:
            uRgS.SYS_S.R.CONTROL = pV->uc;
            goto sbss;

        // Banked Non-Secure Registers
        case 0xC0:
            uRgS.MSP_NS = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nMSP_NS);
            break;
        case 0xC1:
            uRgS.PSP_NS = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nPSP_NS);
            break;
        case 0xC2:
            uRgS.MSPLIM_NS = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nMSPLIM_NS);
            break;
        case 0xC3:
            uRgS.PSPLIM_NS = pV->u32;
            SetRegs(NULL, NULL, &uRgS, 1ULL << nPSPLIM_NS);
            break;
        case 0xC4:
            uRgS.SYS_NS.R.BASEPRI = pV->uc;
        sbnss:
            SetRegs(NULL, NULL, &uRgS, 1ULL << nSYS_NS);
            break;
        case 0xC5:
            uRgS.SYS_NS.R.PRIMASK = pV->uc;
            goto sbnss;
        case 0xC6:
            uRgS.SYS_NS.R.FAULTMASK = pV->uc;
            goto sbnss;
        case 0xC7:
            uRgS.SYS_NS.R.CONTROL = pV->uc;
            goto sbnss;
#endif // DBGCM_V8M

        default:
            return (0);
    }

    if (PlayDead == 1) { // target died for some reason...
        ExecPlayDead();  // show error message box and disconnect the target
    }
    return (1); // Ok.
}


/*
 * Initialize Register View
 */

static void InitRegs(void)
{
    REGDSC dsc;

#if DBGCM_V8M
    if (IsV8M()) {
        if (xFPU) {
            // Groups with FPU
            if (pio->bSecureExt) {
                dsc.nGitems = sizeof(rGroupsF_v8MS) / sizeof(rGroupsF_v8MS[0]);
                dsc.GrpArr  = rGroupsF_v8MS;
            } else {
                dsc.nGitems = sizeof(rGroupsF_v8M) / sizeof(rGroupsF_v8M[0]);
                dsc.GrpArr  = rGroupsF_v8M;
            }
        } else {
            // Groups without FPU
            if (pio->bSecureExt) {
                // Groups without FPU
                dsc.nGitems = sizeof(rGroups_v8MS) / sizeof(rGroups_v8MS[0]);
                dsc.GrpArr  = rGroups_v8MS;
            } else {
                dsc.nGitems = sizeof(rGroups_v8M) / sizeof(rGroups_v8M[0]);
                dsc.GrpArr  = rGroups_v8M;
            }
        }

        if (xMainline) {
            if (pio->bSecureExt) {
                if (xFPU) {
                    dsc.nRitems = sizeof(rItems_v8M_MSF) / sizeof(rItems_v8M_MSF[0]);
                    dsc.RegArr  = rItems_v8M_MSF;
                } else {
                    dsc.nRitems = sizeof(rItems_v8M_MS) / sizeof(rItems_v8M_MS[0]);
                    dsc.RegArr  = rItems_v8M_MS;
                }
            } else {
                if (xFPU) {
                    dsc.nRitems = sizeof(rItems_v8M_MF) / sizeof(rItems_v8M_MF[0]);
                    dsc.RegArr  = rItems_v8M_MF;
                } else {
                    dsc.nRitems = sizeof(rItems_v8M_M) / sizeof(rItems_v8M_M[0]);
                    dsc.RegArr  = rItems_v8M_M;
                }
            }
        } else {
            if (pio->bSecureExt) {
                dsc.nRitems = sizeof(rItems_v8M_BS) / sizeof(rItems_v8M_BS[0]);
                dsc.RegArr  = rItems_v8M_BS;
            } else {
                dsc.nRitems = sizeof(rItems_v8M_B) / sizeof(rItems_v8M_B[0]);
                dsc.RegArr  = rItems_v8M_B;
            }
        }
    } else {
        if (xFPU) {
            dsc.nGitems = sizeof(rGroupsF) / sizeof(rGroupsF[0]);
            dsc.GrpArr  = rGroupsF;
        } else {
            dsc.nGitems = sizeof(rGroups) / sizeof(rGroups[0]);
            dsc.GrpArr  = rGroups;
        }
        switch (xxCPU) {
            case ARM_CM0P:
            case ARM_SC000:
                dsc.nRitems = sizeof(rItems_SC000) / sizeof(rItems_SC000[0]);
                dsc.RegArr  = rItems_SC000;
                break;
            case ARM_CM0:
            case ARM_CM1:
                dsc.nRitems = sizeof(rItems_CM1) / sizeof(rItems_CM1[0]);
                dsc.RegArr  = rItems_CM1;
                break;
            case ARM_CM3:
            case ARM_SC300:
                dsc.nRitems = sizeof(rItems_CM3) / sizeof(rItems_CM3[0]);
                dsc.RegArr  = rItems_CM3;
                break;
            case ARM_CM4:
            case ARM_CM7:
                if (xFPU) {
                    dsc.nRitems = sizeof(rItems_CM4F_CM7F) / sizeof(rItems_CM4F_CM7F[0]);
                    dsc.RegArr  = rItems_CM4F_CM7F;
                } else {
                    dsc.nRitems = sizeof(rItems_CM4_CM7) / sizeof(rItems_CM4_CM7[0]);
                    dsc.RegArr  = rItems_CM4_CM7;
                }
                break;
        }
    }
#else  // DBGCM_V8M
    if (xFPU) {
        dsc.nGitems = sizeof(rGroupsF) / sizeof(rGroupsF[0]);
        dsc.GrpArr = rGroupsF;
    } else {
        dsc.nGitems = sizeof(rGroups) / sizeof(rGroups[0]);
        dsc.GrpArr = rGroups;
    }
    switch (xxCPU) {
        case ARM_CM0P:
        case ARM_SC000:
            dsc.nRitems = sizeof(rItems_SC000) / sizeof(rItems_SC000[0]);
            dsc.RegArr = rItems_SC000;
            break;
        case ARM_CM0:
        case ARM_CM1:
            dsc.nRitems = sizeof(rItems_CM1) / sizeof(rItems_CM1[0]);
            dsc.RegArr = rItems_CM1;
            break;
        case ARM_CM3:
        case ARM_SC300:
            dsc.nRitems = sizeof(rItems_CM3) / sizeof(rItems_CM3[0]);
            dsc.RegArr = rItems_CM3;
            break;
        case ARM_CM4:
        case ARM_CM7:
            if (xFPU) {
                dsc.nRitems = sizeof(rItems_CM4F_CM7F) / sizeof(rItems_CM4F_CM7F[0]);
                dsc.RegArr = rItems_CM4F_CM7F;
            } else {
                dsc.nRitems = sizeof(rItems_CM4_CM7) / sizeof(rItems_CM4_CM7[0]);
                dsc.RegArr = rItems_CM4_CM7;
            }
            break;
    }
#endif // DBGCM_V8M

    dsc.RegGet = RegGet;           // get function
    dsc.RegSet = RegSet;           // set function
    pCbFunc(AG_CB_INITREGV, &dsc); // Install RegView in uVision2
}


UINT AGDIMsgBox(HWND        hWnd,
                const char *pMsg,
                const char *pCaption,
                UINT        uiType,
                UINT        uiDefAnswer)
{
    UINT      retVal;
    AG_MSGBOX mb;

    if (pCbFunc) {
        mb.hWnd        = hWnd;
        mb.pMsg        = pMsg;
        mb.pCaption    = pCaption;
        mb.uiType      = uiType;
        mb.uiDefAnswer = uiDefAnswer;

        retVal = ((UINT)pCbFunc(AG_CB_MSGBOX, &mb)); // Call AGDI messagebox function
    } else {
        // AGDI callback not set, so display a regular message box
        retVal = (UINT)MessageBox(hWnd, pMsg, pCaption, uiType);
    }

    return (retVal);
}


#if DBGCM_MEMACCX
/******** AG_MemAccX Extensions ********/

// 04.03.2019: Filter target access related errors if requested.
static DWORD SkipTargetAccErr(DWORD status)
{
    switch (status) {
        // Clear error codes that
        //   - can happen during memory accesses
        //   - indicate potentially non-persistant issues like temporary connection losses
        case EU05: // JTAG Communication Error
        case EU08: // SWD Communication Error
        case EU14: // Cannot access Memory
            return (0);
    }

    return (status);
}


static __inline U32 MemAccX_SetErr(ACCMEMX *pIn, U64 addr, U32 err)
{
    pIn->bErr     = 1;
    pIn->nErrAddr = addr;
    pIn->nErr     = err;
    if (pIn->bDebugPort) {
        pIn->nErrDP = pIn->debugPort;
    } else {
#if DBGCM_DBG_DESCRIPTION
        if (PDSCDebug_IsEnabled()) {
            pIn->nErrDP = PDSCDebug_GetActiveDP();
        } else {
            pIn->nErrDP = (U32)(-1);
        }
#else  // DBGCM_DBG_DESCRIPTION
        pIn->nErrDP = (U32)(-1);
#endif // DBGCM_DBG_DESCRIPTION
    }
    pIn->nErrAP = pIn->bAccessPort ? pIn->accessPort : ((AP_Sel & APSEL) >> 24);
    return (ACCMX_FAILED);
}


#if 0 // 28.06.2019: Reworked code for better maintenance, latest changes only in the reworked implementation at the end of the file.
static U32 MemAccX_MemAcc(ACCMEMX *pIn) {
  DWORD nAddr;
  int   status    = 0;
  U32   nE       = ACCMX_DONE;
  U32   _dpID    = 0;               // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
  DWORD oDP      = JTAG_devs.com_no;
  DWORD oAP_Sel  = AP_Sel;
  DWORD oCSW_Val = CSW_Val;
  DWORD sCSW_Val = 0;               // CSW Register Value of AP switched to
                                    //BYTE  accAttr  = pIn->bNoAddrIncr ? BLOCK_NADDRINC : 0;
  BYTE  accAttr;

  if (pIn->nMany == 0) {
    return (ACCMX_BADPARM);                                       // invalid input parameters
  }

  if (pIn->nAddr & 0xFFFFFFFF00000000ULL) {
    return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
  }
  nAddr = (DWORD)(pIn->nAddr & 0x00000000FFFFFFFFULL);

  switch (pIn->accSize) {
  case ACCMX_U8:
    if (!(AP_AccSizes & AP_ACCSZ_BYTE)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);      // Access size not supported by target
    }
    if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany - 1) & 0xFFFFFFFF00000000ULL)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS);    // Access exceeds supported bounds of requested resource
    }
    break;
  case ACCMX_U16:
    if (!(AP_AccSizes & AP_ACCSZ_HWORD)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);      // Access size not supported by target
    }
    if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany*2 - 1) & 0xFFFFFFFF00000000ULL)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS);    // Access exceeds supported bounds of requested resource
    }
    if (pIn->bNoAddrIncr && (pIn->nAddr & 0x1)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN);         // Unsupported unaligned resource access
    }
    break;
  case ACCMX_U32:
    if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany*4 - 1) & 0xFFFFFFFF00000000ULL)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS);    // Access exceeds supported bounds of requested resource
    }
    if (pIn->bNoAddrIncr && (pIn->nAddr & 0x3)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN);         // Unsupported unaligned resource access
    }
    break;

#if DBGCM_V8M
  case ACCMX_ANYSZ:
    if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany - 1) & 0xFFFFFFFF00000000ULL)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS);    // Access exceeds supported bounds of requested resource
    }
    if (pIn->bNoAddrIncr) {                                       // 29.01.2019: Only allow 32-bit aligned accesses w/o address increment for ACCMX_ANYSZ
      if (pIn->nAddr & 0x3 || pIn->nMany & 0x03) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN);       // Unsupported unaligned resource access
      }
    }
    break;
#endif // DBGCM_V8M

  default:
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);        // Unsupported access size for this resource
  }

  if (iRun && !supp.MemAccR)  {                                   // currently running, can't access.
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_RUN);             // Cannot access resource while target is running
  }


#if DBGCM_DBG_DESCRIPTION
  if (pIn->bDebugPort) {
    if (!PDSCDebug_IsEnabled()) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);     // DP access not supported (also if PDSC Debug disabled)
    }
    _dpID   = PDSCDebug_GetInternalDeviceId(pIn->debugPort);
    if (_dpID == (U32)(-1)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_DP);    // Invalid DP ID selected
    }
    status  = SwitchDP(_dpID, false);
    if (status) return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_SWITCH_DP);  // Switching DP failed
  }
#else  // DBGCM_DBG_DESCRIPTION
  if (pIn->bDebugPort) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);       // DP access not supported (also if PDSC Debug disabled)
  }
#endif // DBGCM_DBG_DESCRIPTION

  // Don't return until having restored port selection

  if (pIn->bAccessPort) {
    if (pIn->accessPort & 0xFFFFFF00) {
      SwitchDP(oDP, false);
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_AP);    // Invalid AP index selected
    }
    AP_Sel = (pIn->accessPort << 24);
  }

  // DP or AP switch? Init CSW accordingly.
  if ( (pIn->bDebugPort  && (oDP != _dpID))
    || (pIn->bAccessPort && ((oAP_Sel & APSEL) != (AP_Sel & APSEL)))) {
    status = ReadAP(AP_CSW, &sCSW_Val);
    if (status) {
      AP_Sel = oAP_Sel;
      SwitchDP(oDP, false);
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL);        // Target access failed
    }
    CSW_Val = (sCSW_Val | CSW_RESERVED|CSW_MSTRDBG|CSW_HPROT_PRIV|CSW_DBGSTAT);  // Add standard settings
    if ((sCSW_Val & (CSW_RESERVED|CSW_MSTRDBG|CSW_HPROT_PRIV|CSW_DBGSTAT)) != (CSW_RESERVED|CSW_MSTRDBG|CSW_HPROT_PRIV|CSW_DBGSTAT)) {
      status = WriteAP(AP_CSW, CSW_Val);
      if (status) {
        CSW_Val = oCSW_Val;
        AP_Sel  = oAP_Sel;
        SwitchDP(oDP, false);
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL);        // Target access failed
      }
    }
  }


  // Don't return until having restored port selection
  accAttr = 0;
  if (pIn->bNoAddrIncr) accAttr |= BLOCK_NADDRINC;
  accAttr |= (pIn->secureType & ACCMX_SECTYPE_CPU) << BLOCK_SECTYPE_P;

  switch (pIn->accCmd) {
  case ACCMX_READ:                   // read access

#if DBGCM_DBG_DESCRIPTION
    // if (!PDSCDebug_IsExecutingSeq() && !pIn->bNoAddrIncr) {  // always read from target if executing sequence or no address increment
    if (!PDSCDebug_IsExecutingSeq() && !pIn->bNoAddrIncr                // always read from target if executing sequence or no address increment
      && (pIn->secureType == BLOCK_SECTYPE_ANY)) {                      // 29.01.2019: Skip caching if explicitly chosen security attribute
      if (ReadCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany*pIn->accSize))) {
        // Cache Read, we are done here
        break;
      }
    }
#else  // DBGCM_DBG_DESCRIPTION
    // if (!pIn->bNoAddrIncr) {  // always read from target if executing sequence or no address increment
    if (!pIn->bNoAddrIncr                                               // always read from target if executing sequence or no address increment
      && (pIn->secureType == BLOCK_SECTYPE_ANY)) {                      // 29.01.2019: Skip caching if explicitly chosen security attribute
      if (ReadCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany*pIn->accSize))) {
        // Cache Read, we are done here
        break;
      }
    }
#endif // DBGCM_DBG_DESCRIPTION

    switch (pIn->accSize)  {
    case ACCMX_U8:                 // read 'nMany' bytes into pIn->pB8
      status   = ReadARMMemD8(&nAddr, pIn->pB8, pIn->nMany, accAttr);
      break;
    case ACCMX_U16:                // read 'nMany' uint16 into pIn->pB16
      status   = ReadARMMemD16(&nAddr, pIn->pB16, pIn->nMany, accAttr);
      break;
    case ACCMX_U32:                // read 'nMany' uint32 into pIn->pB32
      status   = ReadARMMemD32(&nAddr, pIn->pB32, pIn->nMany, accAttr);
      break;

#if DBGCM_V8M
    case ACCMX_ANYSZ:              // read 'nMany' bytes into pIn->pB8, AGDI DLL decides which access size(s) to use
      if (accAttr & BLOCK_NADDRINC) {
        // 29.01.2019: Only 32-bit aligned accesses w/o address increment should reach this point
        status = ReadARMMemD32(&nAddr, pIn->pB32, pIn->nMany >> 2, accAttr);
      } else {
        status = ReadARMMem   (&nAddr, pIn->pB8, pIn->nMany, accAttr);
      }
      break;
#endif // DBGCM_V8M

    }
    // if (!status && (_dpID == nCPU) && (((AP_Sel & APSEL) >> 24) == MonConf.AP)) {
    if (!status && (_dpID == nCPU) && (((AP_Sel & APSEL) >> 24) == MonConf.AP)
      && !pIn->bNoAddrIncr                                              // 30.01.2019: Fixed, was missing
      && (pIn->secureType == BLOCK_SECTYPE_ANY)) {                      // 30.01.2019: Skip caching if explicitly chosen security attribute
      // Default DP and AP => Write Cache
      WriteCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany*pIn->accSize));
    }
    break;
  case ACCMX_WRITE:                  // write access
    switch (pIn->accSize)  {
    case ACCMX_U8:                   // write 'nMany' bytes from pIn->pB8
      status   = WriteARMMemD8(&nAddr, pIn->pB8, pIn->nMany, accAttr);
      break;
    case ACCMX_U16:                  // write 'nMany' uint16 from pIn->pB16
      status   = WriteARMMemD16(&nAddr, pIn->pB16, pIn->nMany, accAttr);
      break;
    case ACCMX_U32:                  // write 'nMany' uint32 from pIn->pB32
      status   = WriteARMMemD32(&nAddr, pIn->pB32, pIn->nMany, accAttr);
      break;

#if DBGCM_V8M
    case ACCMX_ANYSZ:                // write 'nMany' bytes into pIn->pB8, AGDI DLL decides which access size(s) to use
      if (accAttr & BLOCK_NADDRINC) {
        // 29.01.2019: Only 32-bit aligned accesses w/o address increment should reach this point
        status = WriteARMMemD32(&nAddr, pIn->pB32, pIn->nMany, accAttr);
      } else {
        status = WriteARMMem   (&nAddr, pIn->pB8, pIn->nMany, accAttr);
      }
      break;
#endif // DBGCM_V8M

    }
    // if (!status && (_dpID == nCPU) && (((AP_Sel & APSEL) >> 24) == MonConf.AP)) {
    if (!status && (_dpID == nCPU) && (((AP_Sel & APSEL) >> 24) == MonConf.AP)
      && !pIn->bNoAddrIncr                                              // 29.01.2019: Fixed, was missing
      && (pIn->secureType == BLOCK_SECTYPE_ANY)) {                      // 29.01.2019: Skip caching if explicitly chosen security attribute
      // Default DP and AP => Write Cache
      WriteCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany*pIn->accSize));
    }
    break;
  default:
    nE = ACCMX_BADCMD;                                            // invalid input command
    break;
  }

  // DP or AP switch? Restore CSW values.
  if ( (pIn->bDebugPort  && (oDP != _dpID))
    || (pIn->bAccessPort && ((oAP_Sel & APSEL) != (AP_Sel & APSEL)))) {
    if ((CSW_Val & ~(CSW_DBGSTAT|CSW_TINPROG)) != (sCSW_Val & ~(CSW_DBGSTAT|CSW_TINPROG))) {
      if (status) {
        WriteAP(AP_CSW, sCSW_Val);
      } else {
        status = WriteAP(AP_CSW, sCSW_Val);
      }
    }
    CSW_Val = oCSW_Val;
  }

  // Restore Port Selection
  if (pIn->bDebugPort) {
    // SwitchDP now only sets internal variable, port already powered-up
    if (status) {
      SwitchDP(oDP, false);
    } else {
      status = SwitchDP(oDP, false);
    }
  }
  if (pIn->bAccessPort) {
    AP_Sel  = oAP_Sel;
  }

  if (pIn->bSkipAccErr) {                                         // 04.03.2019: Filter access error if requested
    status = SkipTargetAccErr(status);
  }

  if (status != 0) {
    OutError (status);        // [TdB: 11.05.2015] Report Error to Cmd Window also for specific memory accesses (8, 16, 32 Bit)
  }

  if (status != 0 && nE == ACCMX_DONE) {
    nE = MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL);           // Target access failed
  }

  // States restored, returning allowed again

  return nE;
}


static U32 MemAccX_APAcc(ACCMEMX *pIn) {
  DWORD nAddr;
  int   status = 0;
  U32   nE     = ACCMX_DONE;
  U32   _dpID  = 0;          // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
  DWORD oDP     = JTAG_devs.com_no;
  DWORD oAP_Sel = AP_Sel;

  if (pIn->nMany == 0) {
    return (ACCMX_BADPARM);                                        // invalid input parameters
  }

  if (pIn->nAddr & 0xFFFFFFFFFFFFFF00ULL) {                        // 0x00 - 0xFC, 4-byte aligned
    return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_ACC_BOUNDS);  // Access exceeds supported bounds of requested resource
  }
  nAddr = (DWORD)(pIn->nAddr & 0x00000000000000FFULL);

  if (nAddr & 0x00000003) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN);            // Unsupported unaligned resource access
  }

  if (pIn->nMany > 1) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOBLOCK);          // Block access not allowed for this resource
  }

  // if (pIn->accSize != ACCMX_U32) {
  if (pIn->accSize != ACCMX_U32 && pIn->accSize != ACCMX_ANYSZ) {  // 29.01.2019: ACCMX_ANYSZ shall also pass
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);         // Unsupported access size for this resource
  }

#if DBGCM_DBG_DESCRIPTION
  if (pIn->bDebugPort) {
    if (!PDSCDebug_IsEnabled()) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);      // DP access not supported (also if PDSC Debug disabled)
    }
    _dpID  = PDSCDebug_GetInternalDeviceId(pIn->debugPort);
    if (_dpID == (U32)(-1)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_DP);     // Invalid DP ID selected
    }
    status = SwitchDP(_dpID, false);
    if (status) return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_SWITCH_DP);  // Switching DP failed
  }
#else  // DBGCM_DBG_DESCRIPTION
  if (pIn->bDebugPort) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);        // DP access not supported (also if PDSC Debug disabled)
  }
#endif // DBGCM_DBG_DESCRIPTION

  // Don't return until having restored port selection

  if (pIn->bAccessPort) {
    if (pIn->accessPort & 0xFFFFFF00) {
      SwitchDP(oDP, false);
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_AP);     // Invalid AP index selected
    }
    AP_Sel     = (pIn->accessPort << 24);
  }

  // Don't return until having restored port selection

  switch (pIn->accCmd) {
  case ACCMX_READ:
    status = ReadAP((BYTE)(pIn->nAddr), (DWORD*)pIn->pB32);
    break;
  case ACCMX_WRITE:
    status = WriteAP((BYTE)(pIn->nAddr), *((DWORD*)pIn->pB32));
    if (status == 0 && (DWORD)pIn->nAddr == AP_CSW) {
      // Update CSW_Val as potentially modified by sequence
      CSW_Val = *((DWORD*)pIn->pB32);
    }
    break;
  default:
    nE = ACCMX_BADCMD;                                             // invalid input command
    break;
  }

  // Restore Port Selection
  if (pIn->bDebugPort) {
    // SwitchDP now only sets internal variable, port already powered-up
    if (status) {
      SwitchDP(oDP, false);
    } else {
      status = SwitchDP(oDP, false);
    }
  }
  if (pIn->bAccessPort) {
    AP_Sel = oAP_Sel;
  }

  if (pIn->bSkipAccErr) {                                          // 04.03.2019: Filter access error if requested
    status = SkipTargetAccErr(status);
  }

  if (status != 0 && nE == ACCMX_DONE) {
    nE = MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL);            // Target access failed
  }

  // States restored, returning allowed again

  return nE;
}


static U32 MemAccX_DPAcc(ACCMEMX *pIn) {
  DWORD nAddr;
  int   status = 0;
  U32   nE     = ACCMX_DONE;
  U32   _dpID  = 0;          // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
  DWORD oDP    = JTAG_devs.com_no;

  if (pIn->bAccessPort || pIn->nMany == 0) {
    // debug port register access does not know about an access port
    return (ACCMX_BADPARM);                                        // invalid input parameters
  }

  if (pIn->nAddr & 0xFFFFFFFFFFFFFFF0ULL) {                        // 0x0 - 0xC, 4-byte aligned
    return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_ACC_BOUNDS);  // Access exceeds supported bounds of requested resource
  }
  nAddr = (DWORD)(pIn->nAddr & 0x000000000000000FULL);

  if (nAddr & 0x00000003) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN);            // Unsupported unaligned resource access
  }

  if (pIn->nMany > 1) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOBLOCK);          // Block access not allowed for this resource
  }

  // if (pIn->accSize != ACCMX_U32) {
  if (pIn->accSize != ACCMX_U32 && pIn->accSize != ACCMX_ANYSZ) {  // 29.01.2019: ACCMX_ANYSZ shall also pass
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);         // Unsupported access size for this resource
  }

#if DBGCM_DBG_DESCRIPTION
  if (pIn->bDebugPort) {
    if (!PDSCDebug_IsEnabled()) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);      // DP access not supported (also if PDSC Debug disabled)
    }
    _dpID  = PDSCDebug_GetInternalDeviceId(pIn->debugPort);
    if (_dpID == (U32)(-1)) {
      return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_DP);     // Invalid DP ID selected
    }
    status = SwitchDP(_dpID, false);
    if (status) return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_SWITCH_DP);  // Switching DP failed
  }
#else  // DBGCM_DBG_DESCRIPTION
  if (pIn->bDebugPort) {
    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);        // DP access not supported (also if PDSC Debug disabled)
  }
#endif // DBGCM_DBG_DESCRIPTION

  // Don't return until having restored port selection

  switch (pIn->accCmd) {
  case ACCMX_READ:
    status = ReadDP((BYTE)(pIn->nAddr), (DWORD*)pIn->pB32);
    break;
  case ACCMX_WRITE:
    status = WriteDP((BYTE)(pIn->nAddr), *((DWORD*)pIn->pB32));
#if 0  // 20.11.2018: This automatism causes more trouble than good. Behaviour to be clarified as part of the CMSIS specs.
    if (status == 0 && (DWORD)pIn->nAddr == DP_SELECT) {
      // Update AP_Sel
      AP_Sel = *((DWORD*)pIn->pB32) & APSEL;
    }
#endif
    break;
  default:
    nE = ACCMX_BADCMD;                                             // invalid input command
    break;
  }

  // Restore Port Selection
  if (pIn->bDebugPort) {
    // SwitchDP now only sets internal variable, port already powered-up
    if (status) {
      SwitchDP(oDP, false);
    } else {
      status = SwitchDP(oDP, false);
    }
  }

  if (pIn->bSkipAccErr) {                                          // 04.03.2019: Filter access error if requested
    status = SkipTargetAccErr(status);
  }

  if (status != 0 && nE == ACCMX_DONE) {
    nE = MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL);            // Target access failed
  }

  // States restored, returning allowed again

  return nE;
}


/*
* Extended Memory-Access Read/Write with access size specifier  /24.5.2010/
*/
U32 _EXPO_ AG_MemAccX (ACCMEMX *pIn) { // SarmCM3 will query this interface...
  DWORD nE         = 0;
  int   lockStatus = 0;

#if DBGCM_DBG_DESCRIPTION
  // Continue target accesses if executing a PDSC sequence,
  // StopTarget currently called after setting PlayDead
  if (PlayDead && !PDSCDebug_IsExecutingSeq())  {   // driver disconnected.
    ExecPlayDead();                                 // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
    return (ACCMX_CONNECTION);                      // No target connection
  }
#else  // DBGCM_DBG_DESCRIPTION
  // Continue target accesses if executing a PDSC sequence,
  // StopTarget currently called after setting PlayDead
  if (PlayDead)  {                                  // driver disconnected.
    ExecPlayDead();                                 // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
    return (ACCMX_CONNECTION);                      // No target connection
  }
#endif // DBGCM_DBG_DESCRIPTION

  MemErr = 0;                          // clear previous error  /13.11.2009/

  if (pIn == NULL)  {
    return (ACCMX_BADPARM);            // invalid input parameters
  }

  nE = ACCMX_DONE;

  CheckCom (1);

#if DBGCM_DBG_DESCRIPTION
  if (!PDSCDebug_IsExecutingSeq()) {                // Otherwise keep context of currently executed sequence
    PDSCDebug_DebugContext = DBGCON_TARGET_ACCESS;  // Sequence for example sent if powering up DP
  }
#endif // DBGCM_DBG_DESCRIPTION

  if (pIn->accCmd == ACCMX_QUERY_ACC) {
    if (pIn->pB8 == NULL) {
      nE = ACCMX_BADPARM;
      goto end;
    }

#if DBGCM_V8M
    *(pIn->pB8) = AP_AccSizes | ACCMX_ANYSZ; // Currently same mapping // 29.01.2019: Indicate support for 'any size' accesses.
#else  // DBGCM_V8M
    *(pIn->pB8) = AP_AccSizes;         // Currently same mapping
#endif // DBGCM_V8M

    goto end;
  }

  // Lock target communication. The following must not be interrupted.
  //---TODO: Ensure that the following section has exclusive target access
  DEVELOP_MSG ("Todo: \nLock target communication. The following must not be interrupted.");

  switch (pIn->accType) {
  case ACCMX_TYPE_MEM:
    nE = MemAccX_MemAcc(pIn);
    break;
  case ACCMX_TYPE_AP:
    nE = MemAccX_APAcc(pIn);
    break;
  case ACCMX_TYPE_DP:
    nE = MemAccX_DPAcc(pIn);
    break;
  default:
    nE = ACCMX_BADPARM;
    break;
  }

  // Unlock target communication.
  //---TODO: Ensure that the following section has exclusive target access
  DEVELOP_MSG ("Todo: \nUnlock target communication.");

end:
  CheckCom (0);

  //---TODO: Activate if "Unlock target communication" failed.
  // if (nE == ACCMX_DONE && pIn->bErr == 0 && lockStatus) {
  //   nE = ACCMX_FAILED;
  // }

  if (PlayDead == 1) {                 // disconnected...
    ExecPlayDead();
  }

  return (nE);
}
#endif // #if 0
#endif // DBGCM_MEMACCX


#if DBGCM_DBG_DESCRIPTION
static __inline U32 DapAcc_SetErr(ACCDAP *pIn, U32 err)
{
    pIn->bErr = 1;
    pIn->nErr = err;
    return (ACCDAP_FAILED);
}


/*
*  DAP Access: Read/Write Debug Pins
*    - pIn->pB8  : pin values
*    - pIn->mask : pin select
*    - pIn->wait : pin wait
*/
static U32 DapAcc_Pins(ACCDAP *pIn)
{
    int  status = 0;
    U32  nE     = ACCDAP_DONE;
    bool bForce = false;

    if (pIn == NULL) {
        return (ACCDAP_BADPARM);
    }

    if (pIn->pOut8 == NULL || pIn->pIn8 == NULL) {
        nE = ACCDAP_BADPARM;
        goto end;
    }

    // Check mask (Debug Unit may not be able to drive all pins
    //---TODO: Change mask if debug unit cannot drive all relevant pins
    switch (DP_Type) {
        case JTAG_DP:
            if (pIn->nMask & ~(ACCDAP_PIN_TCK_SWCLK | ACCDAP_PIN_TMS_SWDIO | ACCDAP_PIN_TDI | ACCDAP_PIN_TDO_SWO | ACCDAP_PIN_nTRST | ACCDAP_PIN_nRESET)) { // Pins used in JTAG mode
                nE = DapAcc_SetErr(pIn, ACCDAP_ERR_PIN_INVALID);
                goto end;
            }
            break;
        case SW_DP:
            if (pIn->nMask & ~(ACCDAP_PIN_TCK_SWCLK | ACCDAP_PIN_TMS_SWDIO | ACCDAP_PIN_nRESET)) { // Pins used in SWD mode
                nE = DapAcc_SetErr(pIn, ACCDAP_ERR_PIN_INVALID);
                goto end;
            }
            break;
    }

    //---TODO: Write data to JTAG/SWD pins and wait for pins to stabilize
    DEVELOP_MSG("Todo: \nWrite data to JTAG/SWD pins and wait for pins to stabilize");
    // Members of pIn to use:
    //  - pIn->pOut8 : Value to Write
    //  - pIn->nMask : Pin mask to select pins for value change
    //  - pIn->nDelay: Time to wait for pins to stabilize (in microseconds)
    //
    // Bit layout in pIn->pOut8/nMask (see also AGDI.H):
    //  - Bit 0: TCK/SWCLK (ACCDAP_PIN_TCK_SWCLK)
    //  - Bit 1: TMS/SWDIO (ACCDAP_PIN_TMS_SWDIO)
    //  - Bit 2: TDI       (ACCDAP_PIN_TDI)
    //  - Bit 3: TDO/SWO   (ACCDAP_PIN_TDO_SWO)
    //  - Bit 4: nTRST     (ACCDAP_PIN_nTRST)
    //  - Bit 5: nRESET    (ACCDAP_PIN_nRESET)

end:

    if (status != 0 && nE == ACCDAP_DONE) {
        nE = DapAcc_SetErr(pIn, ACCDAP_ERR_GENERAL); // Target access failed
    }

    return (nE);
}


/*
*  DAP Access: Delay execution
*    - pIn->value : time to wait in microseconds
*/
static U32 DapAcc_Delay(ACCDAP *pIn)
{
    DWORD mac[2];
    DWORD delay; //, d;

    memset(mac, 0, sizeof(mac));

    if (pIn == NULL) {
        return (ACCDAP_BADPARM);
    }

    delay = pIn->nDelay;

    // Non-atomic, do it on driver level (USB communication will already make this inaccurate)
    if (delay == 0) {
        // Zero-delay
        return (ACCDAP_DONE);
    }


    if (pIn->nDelay % 1000) {
        Sleep((pIn->nDelay / 1000) + 1);
    } else {
        Sleep(pIn->nDelay / 1000);
    }

    return (ACCDAP_DONE);
}


/*
*  DAP Access: Write SDWIO_TMS pin for n cycles
*    - pIn->nBits : number of bits to write
*    - pIn->pB64 : bits to write, LSB first
*/
static U32 DapAcc_SeqSWJ(ACCDAP *pIn)
{
    int status;
    U32 nE = ACCDAP_DONE;

    if (pIn == NULL) {
        return (ACCDAP_BADPARM);
    }

    if (pIn->nManyOut * 8 < pIn->nCnt) {
        return (ACCDAP_BADPARM);
    }

    status = SWJ_Sequence(pIn->nCnt, *pIn->pOut64);
    switch (status) {
        case 0:
            break;
        case EU41: // Atomic sequences not supported
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_NOTSUPP);
            break;
        case EU42: // Atomic sequence assembly error
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_ASM);
            break;
        case EU43: // Atomic sequence execution error
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_EXEC);
            break;
        default:
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_GENERAL);
            break;
    }

    return (nE);
}

/*
*  DAP Access: Write JTAG Sequence
*    - pIn->nBits : number of bits to write
*    - pIn->tms   : fixed TMS value for the sequence
*    - pIn->pB64  : TDI values
*/
static U32 DapAcc_SeqJTAG(ACCDAP *pIn)
{
    int status = 0;
    U32 nE     = ACCDAP_DONE;

    if (pIn == NULL || pIn->pOut64 == NULL || pIn->pIn64 == NULL) {
        return ACCDAP_BADPARM;
    }

    if (pIn->nManyOut * 8 < pIn->nCnt) {
        return (ACCDAP_BADPARM);
    }

    switch (DP_Type) {
        case JTAG_DP:
            break;
        case SW_DP:
        default:
            // Only supported for JTAG debugging!!
            return DapAcc_SetErr(pIn, ACCDAP_ERR_PROT_CMDNOTSUPP);
    }

    status = JTAG_Sequence(pIn->nCnt, pIn->tms, *(pIn->pOut64), pIn->pIn64);
    switch (status) {
        case 0:
            break;
        case EU41: // Atomic sequences not supported
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_NOTSUPP);
            break;
        case EU42: // Atomic sequence assembly error
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_ASM);
            break;
        case EU43: // Atomic sequence execution error
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_EXEC);
            break;
        default:
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_GENERAL);
            break;
    }

    return (nE);
}

/*
*  DAP Access: Write ABORT
*    - pIn->pB32  : Value to write to ABORT register
*/
static U32 DapAcc_ABORT(ACCDAP *pIn)
{
    int   status = 0;
    U32   nE     = ACCDAP_DONE;
    U32   _dpID  = 0; // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
    DWORD oDP    = JTAG_devs.com_no;

    if (pIn == NULL || pIn->pOut32 == NULL) {
        return (ACCDAP_BADPARM);
    }

    if (pIn->nManyOut != 4) {
        return (ACCDAP_BADPARM);
    }

    if (pIn->bDebugPort) {
        if (!PDSCDebug_IsEnabled()) {
            return DapAcc_SetErr(pIn, ACCDAP_ERR_NOSUPP_DP); // DP access not supported (also if PDSC Debug disabled)
        }
        _dpID = PDSCDebug_GetInternalDeviceId(pIn->debugPort);
        if (_dpID == (U32)(-1)) {
            return DapAcc_SetErr(pIn, ACCDAP_ERR_INVALID_DP); // Invalid DP ID selected
        }
        status = SwitchDP(_dpID, false);
        if (status)
            return DapAcc_SetErr(pIn, ACCDAP_ERR_SWITCH_DP); // Switching DP failed
    }

    //DAPAbort with value required
    status = DAPAbortVal(*(pIn->pOut32));
    if (status)
        goto end;

end:
    // Restore Port Selection
    if (pIn->bDebugPort) {
        // SwitchDP now only sets internal variable, port already powered-up
        if (status) {
            SwitchDP(oDP, false);
        } else {
            status = SwitchDP(oDP, false);
        }
    }

    if (pIn->bSkipAccErr) { // 04.03.2019: Filter access error if requested
        status = SkipTargetAccErr(status);
    }

    if (nE == ACCDAP_DONE) {
        switch (status) {
            case 0:
                break;
            case EU41: // Atomic sequences not supported
                nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_NOTSUPP);
                break;
            case EU42: // Atomic sequence assembly error
                nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_ASM);
                break;
            case EU43: // Atomic sequence execution error
                nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_EXEC);
                break;
            default:
                nE = DapAcc_SetErr(pIn, ACCDAP_ERR_GENERAL);
                break;
        }
    }

    return (nE);
}

/*
*  DAP Access: Change debug clock
*    - pIn->pB32  : Clock value to set
*/
static U32 DapAcc_Clock(ACCDAP *pIn)
{
    int status = 0;
    U32 cid;
    U32 nE = ACCDAP_DONE;

    if (pIn == NULL || pIn->pOut32 == NULL) {
        return (ACCDAP_BADPARM);
    }

    if (pIn->nManyOut != 4) {
        return (ACCDAP_BADPARM);
    }

    // Non-Atomic
    cid = PDSCDebug_DebugClockId(*(pIn->pOut32));

    status = SWJ_Clock(cid, FALSE);
    if (status)
        goto end;

end:
    switch (status) {
        case 0:
            break;
        case EU41: // Atomic sequences not supported
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_NOTSUPP);
            break;
        case EU42: // Atomic sequence assembly error
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_ASM);
            break;
        case EU43: // Atomic sequence execution error
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_EXEC);
            break;
        default:
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_GENERAL);
            break;
    }

    return (nE);
}


/*
* DAP and debugger access commands /23.06.2014/
*/
U32 _EXPO_ AG_DapAcc(ACCDAP *pIn)
{ // SarmCM3 will query this interface...
    DWORD nE         = 0;
    int   lockStatus = 0;

    // Continue target accesses if executing a PDSC sequence,
    // StopTarget currently called after setting PlayDead
    if (PlayDead && !PDSCDebug_IsExecutingSeq()) { // driver disconnected.
        ExecPlayDead();                            // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (ACCDAP_CONNECTION);                // No target connection
    }
    //if (hMfrm == NULL) return (0);

    MemErr = 0; // clear previous error

    if (pIn == NULL) {
        return (ACCDAP_BADPARM); // invalid input parameters
    }

    nE = ACCDAP_DONE;

    if (!PDSCDebug_IsExecutingSeq()) {                 // Otherwise keep context of currently executed sequence
        PDSCDebug_DebugContext = DBGCON_TARGET_ACCESS; // Sequence for example sent if powering up DP
    }

    CheckCom(1);

    // Lock target communication. The following must not be interrupted.
    lockStatus = LinkCom(1);
    if (lockStatus)
        return ACCMX_FAILED;

    // Do the command
    switch (pIn->accCmd) {
        case ACCDAP_PINS: // Direct Debug Pin Access
            nE = DapAcc_Pins(pIn);
            break;
        case ACCDAP_DELAY: // Delay in debugger firmware
            nE = DapAcc_Delay(pIn);
            break;
        case ACCDAP_SEQ_SWJ: // SWJ Sequence (Value sequence on SWDIO/TMS line)
            nE = DapAcc_SeqSWJ(pIn);
            break;
        case ACCDAP_SEQ_JTAG: // JTAG Sequence (Fixed TMS, Value sequence on TDI line)
            nE = DapAcc_SeqJTAG(pIn);
            break;
        case ACCDAP_ABORT: // Write 32-bit value to DAP ABORT (SWD-DP Register access, JTAG-DP ABORT instruction)
            nE = DapAcc_ABORT(pIn);
            break;
        case ACCDAP_CLOCK: // Set debug clock
            nE = DapAcc_Clock(pIn);
            break;
        case ACCDAP_ATOMIC_START: // Atomic sequences not supported yet!
        case ACCDAP_ATOMIC_EXEC:
        case ACCDAP_ATOMIC_RESULT:
            nE = DapAcc_SetErr(pIn, ACCDAP_ERR_ATOM_NOTSUPP);
            break;
        default:
            nE = ACCDAP_BADCMD;
            break;
    }

    // Unlock target communication.
    lockStatus = LinkCom(0);
    if (lockStatus)
        goto end;

end:
    CheckCom(0);

    if (nE == ACCMX_DONE && pIn->bErr == 0) {
        if (lockStatus) {
            nE = ACCMX_FAILED;
        }
    }

    if (PlayDead == 1) { // disconnected...
        ExecPlayDead();
    }

    return (nE);
}

#endif // DBGCM_DBG_DESCRIPTION


/*
 * Connected CPU is v8-M based
 */
BOOL IsV8M(void)
{
    switch (xxCPU) {
        case ARM_CM0:   // ARM Cortex-M0
        case ARM_CM0P:  // ARM Cortex-M0+
        case ARM_CM1:   // ARM Cortex-M1
        case ARM_CM3:   // ARM Cortex-M3
        case ARM_CM4:   // ARM Cortex-M4
        case ARM_SC000: // ARM SC000 (Secure Cortex-M0)
        case ARM_SC300: // ARM SC300 (Secure Cortex-M3)
        case ARM_CM7:   // ARM Cortex-M7
            return FALSE;
        case ARM_CM23:  // ARM Cortex-M23 (v8-M Baseline)
        case ARM_CM33:  // ARM Cortex-M33 (v8-M Mainline)
        case ARM_CM35P: // ARM Cortex-M35P (v8-M Mainline with Physical Security)
            return TRUE;
    }
    return FALSE;
}


#if DBGCM_V8M
/*
 * Detect Security Extensions
 */
int DetectSecurityExtensions(void)
{
    int   status;
    DWORD val;

    if (!IsV8M()) {
        // Nothing to detect
        pio->bSecureExt = 0;
        return (0);
    }

    if (xMainline) {
        //  Processor Feature Register 1 holds explicit information (Mainline only)
        status = ReadD32(NVIC_PFR1, &val, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);

        pio->bSecureExt = ((val & NVIC_PF_SEC) == NVIC_PF_SEC_IMP) ? 1 : 0;
    } else {
        // Secure Invasive or Secure Non-Invasive Debug implemented? => Security Extensions
        status = ReadD32(DBG_AUTHSTATUS, &val, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);

        pio->bSecureExt = (val & (DAUTH_SI | DAUTH_SNI)) ? 1 : 0;
    }

    if (pio->bSecureExt == 0)
        return (0);

    status = UpdateDebugAuthentication();
    if (status)
        return (status);

    status = UpdateSecurityState();
    if (status)
        return (status);

    return (0);
}


/*
 * Update Processor Security State
 */
int UpdateSecurityState(void)
{
    int   status;
    DWORD val;

    if (!IsV8M()) {
        // Nothing to update
        return (0);
    }

    if (pio->bSecureExt) {
        status = ReadD32(DBG_SCSR, &val, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
        pio->bCPUSecure  = (val & CDS) ? 1 : 0;
        DSCSR_Has_CDSKEY = (val & CDSKEY) ? TRUE : FALSE;
    } else {
        pio->bCPUSecure  = 0;
        DSCSR_Has_CDSKEY = FALSE;
    }
    RegV8MSE.SECURITY.STATE.isSecure = pio->bCPUSecure;

    return (0);
}


#define BITS_SET(val, bits) ((val & bits) == bits)

/*
 * Update Debug Authentication Status
 */
int UpdateDebugAuthentication(void)
{
    int   status;
    DWORD val1, val2;

    if (!IsV8M()) {
        // Nothing to update
        return (0);
    }

    if (pio->bSecureExt) {
        status = ReadD32(DBG_AUTHSTATUS, &val1, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
        status = ReadD32(DBG_HCSR, &val2, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
        pio->bNSDbgEna = BITS_SET(val1, (DAUTH_NSI | DAUTH_NSE)) ? 1 : 0;
        pio->bNSTrcEna = BITS_SET(val1, (DAUTH_NSNI | DAUTH_NSNE)) ? 1 : 0;
        pio->bSDbgEna  = BITS_SET(val1, (DAUTH_SI | DAUTH_SE)) || (val2 & S_SDE) ? 1 : 0; // Check S_SDE, Secure Debug enabled until next instruction
        pio->bSTrcEna  = BITS_SET(val1, (DAUTH_SNI | DAUTH_SNE)) ? 1 : 0;
    }

    return (0);
}


/* Non-Secure Debugger, CPU in Secure Mode */
__inline BOOL NonSecureCantStop(void)
{
    int status;

    if (!IsV8M()) {
        // No Non-Secure vs Secure debug
        return (0);
    }

    status = UpdateDebugAuthentication(); // Get Debug Authentication Status up-to-date
    if (status) {
        OutError(status);
        return (0);
    }

    if (pio->bSecureExt && !pio->bSDbgEna) {
        return (1);
    }
    return (0);
}


// Report severe error, show warning if Non-Secure Debugger + Secure CPU
// return value: 0 - Abort Connection (SetPlayDead called), 1 - Severe Error degraded to warning
int SevereTargetError(unsigned int err_type)
{
    const char *msg    = NULL;
    int         result = 0;

    if (NonSecureCantStop()) {
        switch (err_type) {
            case SEV_ERR_STOP:
                msg = NonSecureStopErr;
                break;
            case SEV_ERR_RESET:
                msg = NonSecureRstErr;
                break;
            case SEV_ERR_ACCESS:
                msg = NonSecureAccErr;
                break;
            case SEV_ERR_REINIT:
                msg = NonSecureReInitErr;
                break;
            default:
                break;
        }
    } else {
        switch (err_type) {
            case SEV_ERR_STOP:
                msg = StopErr;
                break;
            case SEV_ERR_RESET:
                msg = FatalRstErr;
                break;
            case SEV_ERR_ACCESS:
                msg = FatalAccErr;
                break;
            case SEV_ERR_REINIT:
                msg = FatalReInitErr;
                break;
            default:
                break;
        }
    }

    if (NonSecureCantStop() && msg) {
        txtout("Warning: %s\n", msg);
        result = 1;
    } else {
        SetPlayDead(msg);
    }

    return result;
}
#endif // DBGCM_V8M



#if DBGCM_MEMACCX
static U32 MemAccx_SelectDP(ACCMEMX *pIn, U32 *internalDP)
{
    DWORD nAddr = (DWORD)(pIn->nAddr & 0x00000000FFFFFFFFULL);

#if DBGCM_DBG_DESCRIPTION
    int status;

    if (!PDSCDebug_IsEnabled()) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP); // DP access not supported (also if PDSC Debug disabled)
    }
    *internalDP = PDSCDebug_GetInternalDeviceId(pIn->debugPort);
    if (*internalDP == (U32)(-1)) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_DP); // Invalid DP ID selected
    }
    status = SwitchDP(*internalDP, false);
    if (status)
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_SWITCH_DP); // Switching DP failed

    return (ACCMX_DONE);

#else // DBGCM_DBG_DESCRIPTION

    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOSUPP_DP);

#endif // DBGCM_DBG_DESCRIPTION
}


static U32 MemAccx_MemAcc_Validate(ACCMEMX *pIn)
{
    DWORD       nAddr;
    AP_CONTEXT *apCtx;
    DWORD       ap, dp;

    if (pIn->nMany == 0) {
        return (ACCMX_BADPARM); // invalid input parameters
    }

#if !DBGCM_DBG_DESCRIPTION
    if (pIn->bDebugPort) {
        return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_NOSUPP_DP); // DP access not supported (also if PDSC Debug disabled)
    }
#endif // DBGCM_DBG_DESCRIPTION

    if (pIn->nAddr & 0xFFFFFFFF00000000ULL) {                         // Supported address range?
        return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
    }
    nAddr = (DWORD)(pIn->nAddr & 0x00000000FFFFFFFFULL);

    if (pIn->bAccessPort && pIn->accessPort & 0xFFFFFF00) {      // Valid access port index?
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_AP); // Invalid AP index selected
    }

    dp = pIn->bDebugPort ? pIn->debugPort : JTAG_devs.com_no;
    ap = pIn->bAccessPort ? pIn->accessPort : AP_SEL2IDX(AP_Sel);
    if (dp >= DP_NUM) {
        return MemAccX_SetErr(pIn, dp, ACCMX_ERR_INVALID_DP); // Invalid DP index selected
    }
    if (ap >= AP_NUM) {
        return MemAccX_SetErr(pIn, ap, ACCMX_ERR_INVALID_AP); // Invalid AP index selected
    }

    apCtx = &AP_Context[dp][ap];

    switch (pIn->accSize) {
        case ACCMX_U8:
            if (!(apCtx->AccSizes & AP_ACCSZ_BYTE)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE); // Access size not supported by target
            }
            if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany - 1) & 0xFFFFFFFF00000000ULL)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
            }
            break;
        case ACCMX_U16:
            if (!(apCtx->AccSizes & AP_ACCSZ_HWORD)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE); // Access size not supported by target
            }
            if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany * 2 - 1) & 0xFFFFFFFF00000000ULL)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
            }
            if (pIn->bNoAddrIncr && (pIn->nAddr & 0x1)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN); // Unsupported unaligned resource access
            }
            break;
        case ACCMX_U32:
            if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany * 4 - 1) & 0xFFFFFFFF00000000ULL)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
            }
            if (pIn->bNoAddrIncr && (pIn->nAddr & 0x3)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN); // Unsupported unaligned resource access
            }
            break;

#if DBGCM_V8M
        case ACCMX_ANYSZ:
            if (!pIn->bNoAddrIncr && ((pIn->nAddr + pIn->nMany - 1) & 0xFFFFFFFF00000000ULL)) {
                return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
            }
            if (pIn->bNoAddrIncr) { // 29.01.2019: Only allow 32-bit aligned accesses w/o address increment for ACCMX_ANYSZ
                if (pIn->nAddr & 0x3 || pIn->nMany & 0x03) {
                    return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN); // Unsupported unaligned resource access
                }
            }
            break;
#endif // DBGCM_V8M

        default:
            return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE); // Unsupported access size for this resource
    }

    if (iRun && !supp.MemAccR) {                          // currently running, can't access.
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_RUN); // Cannot access resource while target is running
    }

    return (ACCMX_DONE);
}


static BYTE MemAccx_MemAcc_AssembleAccAttr(ACCMEMX *pIn)
{
    BYTE accAttr = 0;
    if (pIn->bNoAddrIncr)
        accAttr |= BLOCK_NADDRINC;
    accAttr |= (pIn->secureType & ACCMX_SECTYPE_CPU) << BLOCK_SECTYPE_P;
    return accAttr;
}


static int MemAccx_MemAcc_DoRead(ACCMEMX *pIn, U32 internalDP)
{
    int   status  = 0;
    BYTE  accAttr = MemAccx_MemAcc_AssembleAccAttr(pIn);         // Assemble access attributes
    DWORD nAddr   = (DWORD)(pIn->nAddr & 0x00000000FFFFFFFFULL); // Convert to 32-Bit address

#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsExecutingSeq() && !pIn->bNoAddrIncr // always read from target if executing sequence or no address increment
        && (pIn->secureType == BLOCK_SECTYPE_ANY)) {     // 29.01.2019: Skip caching if explicitly chosen security attribute
        if (ReadCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany * pIn->accSize))) {
            // Cache Read, we are done here
            return (0);
        }
    }
#else  // DBGCM_DBG_DESCRIPTION
    if (!pIn->bNoAddrIncr                            // always read from target if executing sequence or no address increment
        && (pIn->secureType == BLOCK_SECTYPE_ANY)) { // 29.01.2019: Skip caching if explicitly chosen security attribute
        if (ReadCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany * pIn->accSize))) {
            // Cache Read, we are done here
            return (0);
        }
    }
#endif // DBGCM_DBG_DESCRIPTION

    switch (pIn->accSize) {
        case ACCMX_U8: // read 'nMany' bytes into pIn->pB8
            status = ReadARMMemD8(&nAddr, pIn->pB8, pIn->nMany, accAttr);
            break;
        case ACCMX_U16: // read 'nMany' uint16 into pIn->pB16
            status = ReadARMMemD16(&nAddr, pIn->pB16, pIn->nMany, accAttr);
            break;
        case ACCMX_U32: // read 'nMany' uint32 into pIn->pB32
            status = ReadARMMemD32(&nAddr, pIn->pB32, pIn->nMany, accAttr);
            break;

#if DBGCM_V8M
        case ACCMX_ANYSZ: // read 'nMany' bytes into pIn->pB8, AGDI DLL decides which access size(s) to use
            if (accAttr & BLOCK_NADDRINC) {
                // 29.01.2019: Only 32-bit aligned accesses w/o address increment should reach this point
                status = ReadARMMemD32(&nAddr, pIn->pB32, pIn->nMany >> 2, accAttr);
            } else {
                status = ReadARMMem(&nAddr, pIn->pB8, pIn->nMany, accAttr);
            }
            break;
#endif // DBGCM_V8M
    }
    if (status)
        return (status);

    if (!status && (internalDP == nCPU) && (((AP_Sel & APSEL) >> 24) == MonConf.AP)
        && !pIn->bNoAddrIncr                         // 29.01.2019: Fixed, was missing
        && (pIn->secureType == BLOCK_SECTYPE_ANY)) { // 29.01.2019: Skip caching if explicitly chosen security attribute
        // Default DP and AP => Write Cache
        WriteCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany * pIn->accSize));
    }

    return (0);
}


static int MemAccx_MemAcc_DoWrite(ACCMEMX *pIn, U32 internalDP)
{
    int   status  = 0;
    BYTE  accAttr = MemAccx_MemAcc_AssembleAccAttr(pIn);         // Assemble access attributes
    DWORD nAddr   = (DWORD)(pIn->nAddr & 0x00000000FFFFFFFFULL); // Convert to 32-Bit address

    switch (pIn->accSize) {
        case ACCMX_U8: // write 'nMany' bytes from pIn->pB8
            status = WriteARMMemD8(&nAddr, pIn->pB8, pIn->nMany, accAttr);
            break;
        case ACCMX_U16: // write 'nMany' uint16 from pIn->pB16
            status = WriteARMMemD16(&nAddr, pIn->pB16, pIn->nMany, accAttr);
            break;
        case ACCMX_U32: // write 'nMany' uint32 from pIn->pB32
            status = WriteARMMemD32(&nAddr, pIn->pB32, pIn->nMany, accAttr);
            break;

#if DBGCM_V8M
        case ACCMX_ANYSZ: // write 'nMany' bytes into pIn->pB8, AGDI DLL decides which access size(s) to use
            if (accAttr & BLOCK_NADDRINC) {
                // 29.01.2019: Only 32-bit aligned accesses w/o address increment should reach this point
                status = WriteARMMemD32(&nAddr, pIn->pB32, pIn->nMany, accAttr);
            } else {
                status = WriteARMMem(&nAddr, pIn->pB8, pIn->nMany, accAttr);
            }
            break;
#endif // DBGCM_V8M
    }
    if (status)
        return (status);

    if (!status && (internalDP == nCPU) && (((AP_Sel & APSEL) >> 24) == MonConf.AP)
        && !pIn->bNoAddrIncr                         // 29.01.2019: Fixed, was missing
        && (pIn->secureType == BLOCK_SECTYPE_ANY)) { // 29.01.2019: Skip caching if explicitly chosen security attribute
        // Default DP and AP => Write Cache
        WriteCache((DWORD)(pIn->nAddr), pIn->pB8, (pIn->nMany * pIn->accSize));
    }

    return (0);
}


static U32 MemAccX_MemAcc(ACCMEMX *pIn)
{
    DWORD nAddr;
    int   status        = 0;
    U32   nE            = ACCMX_DONE;
    U32   _dpID         = 0; // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
    BOOL  MemAccX_APOrg = MemAccX_AP;
    DWORD oDP           = JTAG_devs.com_no;
    DWORD oAP_Sel       = AP_Sel; // 08.11.2018: AP_Sel protected by caller function


    nE = MemAccx_MemAcc_Validate(pIn); // Check if memory access can be fulfilled
    if (nE != ACCMX_DONE) {
        return nE;
    }

    nAddr = (DWORD)(pIn->nAddr & 0x00000000FFFFFFFFULL); // Convert to 32-Bit address, address range validated above

    if (pIn->bDebugPort) {                  // Explicit DP selection
        nE = MemAccx_SelectDP(pIn, &_dpID); // Select DP

        if (nE != ACCMX_DONE) { // DP selection failed
            return nE;
        }
    }

    if (pIn->bAccessPort) {                   // Explicit AP selection
        AP_Sel     = (pIn->accessPort << 24); // Select access port(all the AP handling is done later in the access function)
        MemAccX_AP = TRUE;
    }

    // Don't return until having restored port selections and MemAccX_AP

    switch (pIn->accCmd) {
        case ACCMX_READ: // read access
            status = MemAccx_MemAcc_DoRead(pIn, _dpID);
            break;
        case ACCMX_WRITE: // write access
            status = MemAccx_MemAcc_DoWrite(pIn, _dpID);
            break;
        default:
            nE = ACCMX_BADCMD; // invalid input command
            break;
    }

    if (pIn->bDebugPort) { // Restore Debug Port Selection
        // SwitchDP now only sets internal variable, port already powered-up
        if (status) {
            SwitchDP(oDP, false);
        } else {
            status = SwitchDP(oDP, false);
        }
    }

    AP_Sel = oAP_Sel; // Restore Access Port Selection

    if (pIn->bSkipAccErr) { // Filter access error if requested
        status = SkipTargetAccErr(status);
    }

    if (status != 0) {
        OutError(status); // 11.05.2015: Report Error to Cmd Window also for specific memory accesses (8, 16, 32 Bit)
    }

    if (status != 0 && nE == ACCMX_DONE) {
        nE = MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL); // Target access failed
    }

    MemAccX_AP = MemAccX_APOrg;

    // States restored, returning allowed again

    return nE;
}


static U32 MemAccx_APAcc_Validate(ACCMEMX *pIn)
{
    DWORD nAddr;

    if (pIn->nMany == 0) {
        return (ACCMX_BADPARM); // invalid input parameters
    }

#if !DBGCM_DBG_DESCRIPTION
    if (pIn->bDebugPort) {
        return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_NOSUPP_DP); // DP access not supported (also if PDSC Debug disabled)
    }
#endif // DBGCM_DBG_DESCRIPTION

    if (pIn->nAddr & 0xFFFFFFFFFFFFFF00ULL) {                         // 0x00 - 0xFC, 4-byte aligned
        return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
    }
    nAddr = (DWORD)(pIn->nAddr & 0x00000000000000FFULL);

    if (pIn->bAccessPort && pIn->accessPort & 0xFFFFFF00) {      // Valid access port index?
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_INVALID_AP); // Invalid AP index selected
    }

    if (nAddr & 0x00000003) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN); // Unsupported unaligned resource access
    }

    if (pIn->nMany > 1) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOBLOCK); // Block access not allowed for this resource
    }

    if (pIn->accSize != ACCMX_U32 && pIn->accSize != ACCMX_ANYSZ) { // 29.01.2019: ACCMX_ANYSZ shall also pass
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);      // Unsupported access size for this resource
    }

    return (ACCMX_DONE);
}


static U32 MemAccX_APAcc(ACCMEMX *pIn)
{
    DWORD nAddr;
    int   status        = 0;
    U32   nE            = ACCMX_DONE;
    U32   _dpID         = 0; // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
    BOOL  MemAccX_APOrg = MemAccX_AP;
    DWORD oDP           = JTAG_devs.com_no;
    DWORD oAP_Sel       = AP_Sel; // 08.11.2018: AP_Sel protected by caller function

    nE = MemAccx_APAcc_Validate(pIn); // Check if AP access can be fulfilled
    if (nE != ACCMX_DONE) {
        return nE;
    }

    nAddr = (DWORD)(pIn->nAddr & 0x00000000000000FFULL); // Convert to 8-Bit address, address range validated above

    if (pIn->bDebugPort) {                  // Explicit DP selection
        nE = MemAccx_SelectDP(pIn, &_dpID); // Select DP
        if (nE != ACCMX_DONE) {             // DP selection failed
            return nE;
        }
    }

    // Don't return until having restored port selection

    if (pIn->bAccessPort) {                   // Explicit AP selection
        AP_Sel     = (pIn->accessPort << 24); // Select AP only, no need to ID and determine CSW
        MemAccX_AP = TRUE;
    }

    // Don't return until having restored port selection and MemAccX_AP

    switch (pIn->accCmd) {
        case ACCMX_READ:
            status = ReadAP((BYTE)(pIn->nAddr), (DWORD *)pIn->pB32);
            break;
        case ACCMX_WRITE:
            status = WriteAP((BYTE)(pIn->nAddr), *((DWORD *)pIn->pB32));
            break;
        default:
            nE = ACCMX_BADCMD; // invalid input command
            break;
    }

    // Restore Port Selection
    if (pIn->bDebugPort) {
        // SwitchDP now only sets internal variable, port already powered-up
        if (status) {
            SwitchDP(oDP, false);
        } else {
            status = SwitchDP(oDP, false);
        }
    }

    AP_Sel = oAP_Sel; // Restore access port selection

    if (pIn->bSkipAccErr) { // Filter access error if requested
        status = SkipTargetAccErr(status);
    }

    if (status != 0 && nE == ACCMX_DONE) {
        nE = MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL); // Target access failed
    }

    MemAccX_AP = MemAccX_APOrg;

    // States restored, returning allowed again

    return nE;
}


static U32 MemAccx_DPAcc_Validate(ACCMEMX *pIn)
{
    DWORD nAddr;

    if (pIn->bAccessPort || pIn->nMany == 0) {
        // debug port register access does not know about an access port
        return (ACCMX_BADPARM); // invalid input parameters
    }

#if !DBGCM_DBG_DESCRIPTION
    if (pIn->bDebugPort) {
        return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_NOSUPP_DP); // DP access not supported (also if PDSC Debug disabled)
    }
#endif // DBGCM_DBG_DESCRIPTION

    if (pIn->nAddr & 0xFFFFFFFFFFFFFFF0ULL) {                         // 0x0 - 0xC, 4-byte aligned
        return MemAccX_SetErr(pIn, pIn->nAddr, ACCMX_ERR_ACC_BOUNDS); // Access exceeds supported bounds of requested resource
    }
    nAddr = (DWORD)(pIn->nAddr & 0x000000000000000FULL);

    if (nAddr & 0x00000003) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ALIGN); // Unsupported unaligned resource access
    }

    if (pIn->nMany > 1) {
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_NOBLOCK); // Block access not allowed for this resource
    }

    if (pIn->accSize != ACCMX_U32 && pIn->accSize != ACCMX_ANYSZ) { // 29.01.2019: ACCMX_ANYSZ shall also pass
        return MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_ACC_SIZE);      // Unsupported access size for this resource
    }

    return (ACCMX_DONE);
}


static U32 MemAccX_DPAcc(ACCMEMX *pIn)
{
    DWORD nAddr;
    int   status = 0;
    U32   nE     = ACCMX_DONE;
    U32   _dpID  = 0; // internal ID of the DP, i.e. 0 for SWD, tapindex for JTAG
    DWORD oDP    = JTAG_devs.com_no;

    nE = MemAccx_DPAcc_Validate(pIn); // Check if DP access can be fulfilled
    if (nE != ACCMX_DONE) {
        return nE;
    }

    nAddr = (DWORD)(pIn->nAddr & 0x000000000000000FULL); // Convert to 8-Bit address, address range validated above

    if (pIn->bDebugPort) {                  // Explicit DP selection
        nE = MemAccx_SelectDP(pIn, &_dpID); // Select DP
        if (nE != ACCMX_DONE) {             // DP selection failed
            return nE;
        }
    }

    // Don't return until having restored port selection

    switch (pIn->accCmd) {
        case ACCMX_READ:
            status = ReadDP((BYTE)(pIn->nAddr), (DWORD *)pIn->pB32);
            break;
        case ACCMX_WRITE:
            status = WriteDP((BYTE)(pIn->nAddr), *((DWORD *)pIn->pB32));
#if 0 // This automatism causes more trouble than good. Behaviour to be clarified as part of the CMSIS specs.
    if (status == 0 && (DWORD)pIn->nAddr == DP_SELECT) {
      // Update AP_Sel
      AP_Sel = *((DWORD*)pIn->pB32) & APSEL;
    }
#endif
            break;
        default:
            nE = ACCMX_BADCMD; // invalid input command
            break;
    }

    // Restore Port Selection
    if (pIn->bDebugPort) {
        // SwitchDP now only sets internal variable, port already powered-up
        if (status) {
            SwitchDP(oDP, false);
        } else {
            status = SwitchDP(oDP, false);
        }
    }

    if (pIn->bSkipAccErr) { // Filter access error if requested
        status = SkipTargetAccErr(status);
    }

    if (status != 0 && nE == ACCMX_DONE) {
        nE = MemAccX_SetErr(pIn, nAddr, ACCMX_ERR_GENERAL); // Target access failed
    }

    // States restored, returning allowed again

    return nE;
}


/*
 * Extended Memory-Access Read/Write with access size specifier  /24.5.2010/
 */
U32 _EXPO_ AG_MemAccX(ACCMEMX *pIn)
{ // SarmCM3 will query this interface...
    DWORD       nE         = 0;
    int         lockStatus = 0;
    AP_CONTEXT *apCtx;

#if DBGCM_DBG_DESCRIPTION
    // Continue target accesses if executing a PDSC sequence,
    // StopTarget currently called after setting PlayDead
    if (PlayDead && !PDSCDebug_IsExecutingSeq()) { // driver disconnected.
        ExecPlayDead();                            // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (ACCMX_CONNECTION);                 // No target connection
    }
#else  // DBGCM_DBG_DESCRIPTION
    // Continue target accesses if executing a PDSC sequence,
    // StopTarget currently called after setting PlayDead
    if (PlayDead) {                // driver disconnected.
        ExecPlayDead();            // 10.12.2018: Execute in case PlayDead was triggered by other than GUI thread
        return (ACCMX_CONNECTION); // No target connection
    }
#endif // DBGCM_DBG_DESCRIPTION

    //if (hMfrm == NULL) return (0);

    MemErr = 0; // clear previous error  /13.11.2009/

    if (pIn == NULL) {
        return (ACCMX_BADPARM); // invalid input parameters
    }

    //if (SetupMode) {// && pIn->accType != ACCMX_TYPE_PINS) {
    // Only allow pin accesses in Setup Mode
    //  return (ACCMX_NOTSUPP);
    //}

    nE = ACCMX_DONE;

    CheckCom(1);

#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsExecutingSeq()) {                 // Otherwise keep context of currently executed sequence
        PDSCDebug_DebugContext = DBGCON_TARGET_ACCESS; // Sequence for example sent if powering up DP
    }
#endif // DBGCM_DBG_DESCRIPTION

    if (pIn->accCmd == ACCMX_QUERY_ACC) {
        if (pIn->pB8 == NULL) {
            nE = ACCMX_BADPARM;
            goto end;
        }
    }

    // 08.11.2018: Lock target communication. The following must not be interrupted, particularly AP_Sel in AP_CurrentCtx().
    lockStatus = LinkCom(1);
    if (lockStatus)
        goto end;

    if (pIn->accCmd == ACCMX_QUERY_ACC) {
        if (AP_CpuCtx(&apCtx) != 0) {
            nE = MemAccX_SetErr(pIn, MonConf.AP, ACCMX_ERR_INVALID_AP); // Invalid AP index selected
        } else {
#if DBGCM_V8M
            *(pIn->pB8) = apCtx->AccSizes | ACCMX_ANYSZ; // Currently same mapping // 29.01.2019: Indicate support for 'any size' accesses.
#else                                                    // DBGCM_V8M
            *(pIn->pB8) = apCtx->AccSizes; // Currently same mapping // 29.01.2019: Indicate support for 'any size' accesses.
#endif                                                   // DBGCM_V8M
        }
    } else {
        switch (pIn->accType) {
            case ACCMX_TYPE_MEM:
                nE = MemAccX_MemAcc(pIn);
                break;
            case ACCMX_TYPE_AP:
                nE = MemAccX_APAcc(pIn);
                break;
            case ACCMX_TYPE_DP:
                nE = MemAccX_DPAcc(pIn);
                break;
            default:
                nE = ACCMX_BADPARM;
                break;
        }
    }

    // Unlock target communication.
    lockStatus = LinkCom(0);
    if (lockStatus)
        goto end;

end:
    CheckCom(0);

    if (nE == ACCMX_DONE && pIn->bErr == 0) {
        if (lockStatus) {
            nE = ACCMX_FAILED;
        }
    }

    if (PlayDead == 1) { // disconnected...
        ExecPlayDead();
    }

    return (nE);
}
#endif // DBGCM_MEMACCX


/*
 *  Extended "Get Error Text" for Status Information
 *  to add further Info to "Cannot access Memory"
 */
statusInfoMem_t statusInfoMem   = { 0 };
char            statusBuf[1024] = { 0 };

char *statusAccType_str[] = {
    "",      // 0
    "Read",  // 1  STATUS_MEMREAD
    "Write", // 2  STATUS_MEMWRITE
};

char *GetStatusAccTypeStr(int idx)
{
    int max = sizeof(statusAccType_str) / sizeof(char *);

    if (idx < max)
        return statusAccType_str[idx];
    else
        return "-";
}

/*
 *  Extended "Get Error Text" for Status Information
 *  to add @addr to "Cannot access Memory"
 *    Parameter:      status  : Status Code
 *                    nAddr   : Target Address which was accessed
 *                    nSize   : Size of Memory access
 *                    accTYpe : Type of memory access (R, W)
 *    Return Value:   status  : Status Code
 */
int SetStatusMem(int status, DWORD nAddr, DWORD type /*=0*/, int nSize /*=0*/)
{
    statusInfoMem.nAddr = nAddr;
    statusInfoMem.nSize = nSize;
    statusInfoMem.type  = type;
    statusInfoMem.valid = 1;

    return status;
}


/*
 *  Extended "Get Error Text" for Status Information
 *  to for example add @addr to "Cannot access Memory"
 *    Parameter:      status  : Status Code
 *    Return Value:   status  : Status Code
 */
int ClearStatusMem(int status)
{
    statusInfoMem.valid = 0;

    return status;
}


/*
 *  Extended "Get Error Text" for Status Information
 *  to for example add @addr to "Cannot access Memory"
 *    Parameter:      status: Status Code
 *    Return Value:   Pointer to Text
 */
char *StatusText(int status)
{
    switch (status) {
        case EU52: // Not a genuine device
            _snprintf(statusBuf, sizeof(statusBuf) - 1, "%s:\n\n%s %s", _StatusText(status), pdbg->Vendor, pdbg->Device);
            return (statusBuf);
    }

    if (!statusInfoMem.valid) {
        return _StatusText(status);
    }

    char *pText = statusBuf;
    pText += _snprintf(pText, sizeof(statusBuf), "%s", _StatusText(status));

    // Add Information to Message, i.e. Address location to "Cannot access Memory"
    switch (status) {
        case EU14: { // Cannot access Memory
            pText += _snprintf(pText, 32, " (");
            pText += _snprintf(pText, 32, "@ 0x%08x", statusInfoMem.nAddr);

            if (statusInfoMem.type) {
                pText += _snprintf(pText, 32, ", %s", GetStatusAccTypeStr(statusInfoMem.type));
            }
            if (statusInfoMem.nSize) {
                pText += _snprintf(pText, 64, ", Acc Size: %i Byte", statusInfoMem.nSize);
            }
            pText += _snprintf(pText, 32, ")");
            pText = statusBuf;
        } break;

        default:
            pText = _StatusText(status);
    }

    statusInfoMem.valid = 0; // not valid anymore after Message has been reported

    return pText;
}


void InitAGDI()
{
    memset(&RegARM, 0, sizeof(RegARM)); // ARM Cortex-M Registers
    memset(&RegFPU, 0, sizeof(RegFPU)); // FPU Cortex-M Registers

    RegUpToDate  = 0;    // Specifies whether register structure is up to date
    bootflag     = 0;    // Specifies whether first boot loader is in progress or not
    PlayDead     = 0;    // Disables the driver after the communication breaks down.
    PlayDeadMsg  = NULL; // Message to send if processing PlayDead.
    PlayDeadShut = 0;    // Shutting down because of PlayDead
    FirstReset   = 0;    // Is set during initialization and reset during the first reset

    xxCPU = 0; // CPU Type (ARM Cortex-M0/M1/M3/M4 ARM SC300)
    xFPU  = 0; // FPU Flag

    pDyMenu = NULL; // Dynamic menu pointer

    pio = NULL;

    memset(&supp, 0, sizeof(supp)); // supported features
    iRun    = 0;                    // target is currently executing
    StopRun = 0;                    // Flag: stop execution
    GoMode  = 0;
    ReInit  = 0;
    pBhead  = NULL;
    pCURPC  = NULL;
    memset(&lParms, 0, sizeof(lParms)); // LOAD-Parameters

    abreaks = 0; // Number of Code-Address BPs
    cbreaks = 0; // Number of Conditional BPs
    hbreaks = 0; // Number of HW Breakpoints
    wbreaks = 0; // Number of Watchpoint BPs
    vbreaks = 0; // Number of Watchpoint BPs with value match

    pBStart = NULL;                  // pointer to breakpoint chain with temporary break
    memset(&TBp, 0, sizeof(TBp));    // Temporary BreakPoint
    UseSWBP = 0;                     // Use SW Breakpoints
    CntBP   = 0;                     // HW Breakpoint Count
    CntWP   = 0;                     // HW Watchpoint Count
    memset(UseWP, 0, sizeof(UseWP)); // HW Watchpoint Multi-usage Count
    bNoCache = 0;                    // [TdB 13.03.2012] PH: 3.3.2012, 1:=cache temporarily turned off

    Opcode    = 0;     // 1 - Opcode Access, 0 - Data Access
    MemErr    = 0;     // Memory Error (Read/Write or Verify)
    AGDIError = 0;     // Indicates an AGDI error occurred during this AGDI function
                       // ... for delayed error message sending (outside the CheckCom() mutex)
    AGDIErrorCode = 0; // ADGI error code set is AGDIError is true

    NumRecs = 0; // number of trace records

    pCoreClk = NULL; // Core Clock VTR


    Uv2Msg  = 0;    // Uv2-registered Message token
    pCbFunc = NULL; // call-back function of SARM

    pHwnd = NULL;
    hMfrm = NULL; // parent handle (CMainFrame)
    hInst = NULL; // this DLL's instance handle

    iBpCmd     = 0;                    // 13.01.2015: currently executing breakpoint command
    GoUntilAdr = 0;                    // 0xFFFFFFFF - until break/stop, set in GoUntil()
    memset(szVals, 0, sizeof(szVals)); // config/setup values
    strcpy(szKey, "DBGCM");            // plain name of dll, e.g. 'DBGCM'
    memset(&qdll, 0, sizeof(qdll));
    bFirstCall = 1;
    nRmode     = 0;
    memset(&uRg, 0, sizeof(uRg));   // shadow registers
    memset(&uRgF, 0, sizeof(uRgF)); // shadow registers (FPU)
    memset(szV, 0, sizeof(szV));    // temporary buffer
    DbgExitStarted     = 0;
    MultipleGoStep     = FALSE;
    BkpInstructionSkip = TRUE;
    nCPU               = 0;
    lbreaks            = 0; // Number of Watchpoint BPs with link capabilities
    limbreaks          = 0; // Number of Watchpoint BPs with address limit capabilities
    memset(&RegV8MSE, 0, sizeof(RegV8MSE));
    xMainline          = FALSE;
    xSecure            = FALSE;
    ExtendedStep       = 0;
    PlayDeadRegular    = 0; // PlayDead set because of a regular shutdown
    PlayDeadDelayedMsg = 0; // Error message shall be shown during secon call to ExecPlayDead
    memset(&PlayDeadNotifyMsg, 0, sizeof(PlayDeadNotifyMsg));
    pTraceClk        = NULL; // Trace Clock VTR  /* 02.04.2019 */
    ReInitInProgress = 0;
    memset(&statusInfoMem, 0, sizeof(statusInfoMem));
    memset(statusBuf, 0, sizeof(statusBuf));
}
