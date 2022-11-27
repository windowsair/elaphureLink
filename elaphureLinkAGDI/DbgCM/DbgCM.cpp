/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.9
 * @date     $Date: 2020-08-10 16:35:34 +0200 (Mon, 10 Aug 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
 *
 * @brief     DLL main file, loads/stores ..Conf parameters
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
#include "DbgCM.h"

#include "Setup.h" // Target Setup Dialog

#include "..\BOM.h"
#include "..\AGDI.h"
#include "Collect.h"
#include "Debug.h"
#include "Trace.h"
#include "Flash.h"
#include "JTAG.h"

#include "rddi_dll.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct dbgblk   *pdbg;             // startup values
struct MONCONF   MonConf;          // holds target-setup values
struct FLASHCONF FlashConf;        // holds Flash-setup values
BYTE             SetupMode;        // 1:=Remote Setup Mode via Options-Debug
HANDLE           Com_mtx;          // Communication mutex
HANDLE           PlayDeadShut_mtx; // PlayDeadShut mutex
bool             bAnyUnit;         // Flag for selection of Unit entry "Any"

static int CreateLinkCom();

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CDbgCMApp

BEGIN_MESSAGE_MAP(CDbgCMApp, CWinApp)
END_MESSAGE_MAP()


// CDbgCMApp construction

CDbgCMApp::CDbgCMApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
    char name[MAX_PATH];

    sprintf(name, "DBGCM:Com_mtx#%d", GetCurrentProcessId());
    Com_mtx = CreateMutex(NULL, FALSE, name);
    sprintf(name, "DBGCM:PlayDeadShut_mtx#%d", GetCurrentProcessId());
    PlayDeadShut_mtx = CreateMutex(NULL, FALSE, name);
}

CDbgCMApp::~CDbgCMApp()
{
    if (rddi::rddi_Close) {
        rddi::rddi_Close(rddi::k_rddi_handle);
    }

    CloseHandle(Com_mtx);
    CloseHandle(PlayDeadShut_mtx);
}


// The one and only CDbgCMApp object

CDbgCMApp theApp;


// CDbgCMApp initialization

#define RDDILL_GetProcAddress(func)                                       \
    do {                                                                  \
        rddi::func = (decltype(rddi::func))GetProcAddress(handle, #func); \
        if (rddi::func == nullptr)                                        \
            return FALSE;                                                 \
    } while (0)

inline BOOL LoadRddiDllFunction()
{
    auto handle = LoadLibrary("elaphureRddi.dll");
    if (handle == nullptr) {
        return FALSE;
    }

    rddi::rddi_Open  = (decltype(rddi::rddi_Open))GetProcAddress(handle, "RDDI_Open");
    rddi::rddi_Close = (decltype(rddi::rddi_Close))GetProcAddress(handle, "RDDI_Close");
    if (!rddi::rddi_Open || !rddi::rddi_Close) {
        return FALSE;
    }

    RDDILL_GetProcAddress(DAP_ReadReg);
    RDDILL_GetProcAddress(DAP_WriteReg);
    RDDILL_GetProcAddress(DAP_RegAccessBlock);
    RDDILL_GetProcAddress(DAP_RegWriteRepeat);
    RDDILL_GetProcAddress(DAP_RegReadRepeat);
    RDDILL_GetProcAddress(CMSIS_DAP_Detect);
    RDDILL_GetProcAddress(CMSIS_DAP_Identify);
    RDDILL_GetProcAddress(CMSIS_DAP_ConfigureInterface);
    RDDILL_GetProcAddress(CMSIS_DAP_ConfigureDAP);
    RDDILL_GetProcAddress(CMSIS_DAP_Capabilities);
    RDDILL_GetProcAddress(CMSIS_DAP_DetectNumberOfDAPs);
    RDDILL_GetProcAddress(CMSIS_DAP_DetectDAPIDList);
    RDDILL_GetProcAddress(CMSIS_DAP_Commands);

    if (rddi::rddi_Open(&rddi::k_rddi_handle, NULL)) {
        return FALSE;
    }


    return TRUE;
}

BOOL CDbgCMApp::InitInstance()
{
    if (!LoadRddiDllFunction()) {
        return FALSE;
    }

    CWinApp::InitInstance();

    return TRUE;
}



/*
 * Analyze the command arguments 'pArg' and initialize 'MonConf'
 */

void AnalyzeParms(char *pPath, char *pArgs)
{
    char  serno[sizeof(MonConf.UnitSerNo)];
    char  buf[MAX_PATH], szFn[MAX_PATH + 32];
    int   len, i, j, tmp;
    char *cpustring  = 0;
    BOOL  hasTPIUClk = FALSE; // 02.04.2019

    memset(&MonConf, 0, sizeof(MonConf));
    memset(&FlashConf, 0, sizeof(FlashConf));

    JTAG_devs.cnt = 0; // Init JTAG Device Count (Manual JTAG Setup)

    // Initialize default Monitor settings
    strcpy(MonConf.DriverPath, pPath);
    len = strlen(MonConf.DriverPath) - 1;
    while (len && MonConf.DriverPath[len] != '\\')
        len--;
    MonConf.DriverPath[len + 1] = 0;

    MonConf.Opt = CACHE_CODE | CACHE_MEM | BOOT_RESET | USE_SWJ | PORT_SW; // 10.02.2015: Select SWD by default

    MonConf.SWJ_Clock    = 3; // Default 1MHz (without RTCK)
    MonConf.JtagCpuIndex = -1;
    MonConf.AP           = 0;

    FlashConf.RAMStart = 0x00000000;
    FlashConf.RAMSize  = 0x0800; // Defualt 2kB Memory for Algorithms
    MonConf.SFRStart   = 0x40000000;
    MonConf.SFREnd     = 0x5FFFFFFF;

    // Initialize default Trace settings
    //TraceConf.Opt = /*TRACE_ENABLE |*/ TRACE_TIMESTAMP | TRACE_EXCTRC;
    TraceConf.Opt      = /*TRACE_ENABLE |*/ TRACE_TIMESTAMP | TRACE_EXCTRC | TRACE_USE_CORECLK; // 02.04.2019
    TraceConf.Clk      = 10000000;
    TraceConf.PortSize = 1;
    TraceConf.Protocol = TPIU_SWO_UART;
    TraceConf.SWV_Pre  = 0x8007;
    TraceConf.TS_Pre   = 0;
    TraceConf.CYC_Pre  = 0x1F;
    TraceConf.ITM_Ena  = 0xFFFFFFFF;
    TraceConf.ITM_Priv = 0x08;
    TraceConf.TPIU_Clk = 10000000; // 02.04.2019

    if (pdbg) {
        if (pdbg->Iram.nSize) {
            FlashConf.RAMStart = pdbg->Iram.nStart;
            if (pdbg->Iram.nSize < FlashConf.RAMSize) {
                FlashConf.RAMSize = (WORD)pdbg->Iram.nSize;
            }
        }
    }

    // Initialize default Flash settings
    FlashConf.Nitems = 0;
    FlashConf.Opt    = FLASH_ERASE | FLASH_PROGRAM | FLASH_VERIFY;

    // analyze arguments
    if (pArgs) {
        for (i = 0; pArgs[i];) {
            while ((pArgs[i] != '-') && pArgs[i])
                i++; // skip character until next '-'
            //if (pArgs[i] == 0)  return;
            if (pArgs[i] == 0)
                break;
            i++;
            switch (pArgs[i]) {
                case 'U':
                    if (sscanf(&pArgs[i + 1], "%s", &serno) == 1) {
                        tmp = strlen(serno);
                        strcpy(MonConf.UnitSerNo, serno);      // copy serial number
                        if (tmp == 3 && !strcmp(serno, "Any")) // Select "Any" (first) Unit
                            bAnyUnit = true;
                        else
                            bAnyUnit = false;
                    }
                    break;
                case 'O':
                    if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                        MonConf.Opt = tmp;
                    }
                    break;
                case 'S':
                    if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                        MonConf.SWJ_Clock = tmp;
                    }
                    break;
                case 'C':
                    if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                        MonConf.JtagCpuIndex = tmp;
                    }
                    break;
                case 'N': // JTAG device name
                    if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                        if ((tmp >= 0) && (tmp < NJDEVS)) {
                            i += 5;
                            j = 0;
                            while (pArgs[i] && pArgs[i] != '"' && (j < 30)) { // copy string until '"' character
                                JTAG_devs.icname[tmp][j++] = pArgs[i++];      // copy device name
                            }
                            JTAG_devs.icname[tmp][j] = 0; // terminate string
                            JTAG_devs.cnt++;
                        }
                    }
                    break;
                case 'D': // JTAG device ID
                    if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                        if ((tmp >= 0) && (tmp < NJDEVS)) {
                            if (sscanf(&pArgs[i + 4], "%X", &j) == 1) {
                                JTAG_devs.ic[tmp].id = j;
                            }
                        }
                    }
                    break;
                case 'L': // JTAG device IR length
                    if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                        if ((tmp >= 0) && (tmp < NJDEVS)) {
                            if (sscanf(&pArgs[i + 4], "%d", &j) == 1) {
                                JTAG_devs.ic[tmp].ir_len = j;
                            }
                        }
                    }
                    break;
                case 'T':
                    i++;
                    switch (pArgs[i]) {
                        case 'O':
                            if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                                TraceConf.Opt = tmp;
                            }
                            break;
                        case 'C':
                            if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                                TraceConf.Clk = tmp;
                            }
                            break;
                        case 'P':
                            if (sscanf(&pArgs[i + 1], "%X", &tmp) == 1) {
                                TraceConf.Protocol = (BYTE)(tmp >> 4);
                                TraceConf.PortSize = (BYTE)(tmp & 0x0F);
                            }
                            break;
                        case 'D':
                            if (sscanf(&pArgs[i + 2], "%X", &tmp) == 1) {
                                switch (pArgs[i + 1]) {
                                    case 'S':
                                        TraceConf.SWV_Pre = (WORD)tmp;
                                        break;
                                    case 'T':
                                        TraceConf.TS_Pre = (BYTE)tmp;
                                        break;
                                    case 'C':
                                        TraceConf.CYC_Pre = (BYTE)tmp;
                                        break;
                                }
                            }
                            break;
                        case 'I':
                            if (sscanf(&pArgs[i + 2], "%X", &tmp) == 1) {
                                switch (pArgs[i + 1]) {
                                    case 'E':
                                        TraceConf.ITM_Ena = tmp;
                                        break;
                                    case 'P':
                                        TraceConf.ITM_Priv = tmp;
                                        break;
                                }
                            }
                            break;
                        case 'T': // 02.04.2019: Separate Trace Clock setting
                            hasTPIUClk = TRUE;
                            if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                                TraceConf.TPIU_Clk = tmp;
                            }
                            break;
                    }
                    break;
                case 'F':
                    i++;
                    switch (pArgs[i]) {
                        case 'O':
                            if (sscanf(&pArgs[i + 1], "%d", &tmp) == 1) {
                                FlashConf.Opt = tmp;
                            }
                            break;
                        case 'D':
                            if (sscanf(&pArgs[i + 1], "%X", &tmp) == 1) {
                                FlashConf.RAMStart = tmp;
                            }
                            break;
                        case 'C':
                            if (sscanf(&pArgs[i + 1], "%X", &tmp) == 1) {
                                FlashConf.RAMSize = tmp;
                            }
                            break;
                        case 'N':
                            if (sscanf(&pArgs[i + 1], "%X", &tmp) == 1) {
                                FlashConf.Nitems = tmp;
                            }
                            break;
                        case 'F':
                            if (pArgs[i + 2] == ' ')
                                break;
                            if (sscanf(&pArgs[i + 2], "%s", buf) == 1) {
                                strcpy(&FlashConf.Dev[pArgs[i + 1] - '0'].FileName[0], buf);
                            }
                            break;
                        case 'S':
                            if (sscanf(&pArgs[i + 2], "%X", &tmp) == 1) {
                                FlashConf.Dev[pArgs[i + 1] - '0'].Start = tmp;
                            }
                            break;
                        case 'L':
                            if (sscanf(&pArgs[i + 2], "%X", &tmp) == 1) {
                                FlashConf.Dev[pArgs[i + 1] - '0'].Size = tmp;
                            }
                            break;
                            //---7.11.2012: fullpath of algo file:
                        case 'P': {
                            int  i1 = pArgs[i + 1] - '0'; // -FP<n>  n:=0...9
                            int  n;
                            char c1;
                            if (pArgs[i + 2] != '(') { // -FP0(full_path)
                                break;
                            }
                            i += 3; //
                            n = 0;
                            while (pArgs[i] && pArgs[i] != ')') {
                                c1                         = pArgs[i];
                                FlashConf.Dev[i1].fPath[n] = c1;
                                ++n;
                                ++i;
                            }
                            FlashConf.Dev[i1].fPath[n] = 0;

                            //---supply FileName from path:  /7.4.3013/
                            strcpy(szFn, FlashConf.Dev[i1].fPath);
                            PathStripPath(szFn);
                            strcpy(FlashConf.Dev[i1].FileName, szFn);
                            PathRemoveExtension(szFn);
                            strcpy(FlashConf.Dev[i1].DevName, szFn);
                            //---
                            if (pArgs[i] == ')') {
                                ++i;
                            }
                        } break;
                            //------------------------------------
                    }
                    break;
            }
        }
    }

    // 02.04.2019: Initialize TPIU_Clk if it old config string
    if (!hasTPIUClk) {
        TraceConf.TPIU_Clk = TraceConf.Clk;
        TraceConf.Opt |= TRACE_USE_CORECLK;
    }
    TPIU_Clock = (TraceConf.Opt & TRACE_USE_CORECLK) ? TraceConf.Clk : TraceConf.TPIU_Clk;

    // 28.11.2013: Disable CPU specific features in Setup Dialogs (Cortex-M7 does not support ITM TS Prescaler)
    if (SetupMode) {
        xxCPU = 0; // Unknown core
        if (pdbg && pdbg->pDbgX) {
            cpustring = pdbg->pDbgX->szCpu;
            tmp       = (cpustring ? strlen(cpustring) : 0);
            if (strnicmp(cpustring, "\"Cortex-M0\"", tmp) == 0) {
                xxCPU = ARM_CM0;
            } else if (strnicmp(cpustring, "\"Cortex-M0+\"", tmp) == 0) {
                xxCPU = ARM_CM0P;
            } else if (strnicmp(cpustring, "\"Cortex-M1\"", tmp) == 0) {
                xxCPU = ARM_CM1;
            } else if (strnicmp(cpustring, "\"Cortex-M3\"", tmp) == 0) {
                xxCPU = ARM_CM3;
            } else if (strnicmp(cpustring, "\"Cortex-M4\"", tmp) == 0) {
                xxCPU = ARM_CM4;
            } else if (strnicmp(cpustring, "\"Cortex-M4F\"", tmp) == 0) {
                xxCPU = ARM_CM4;
            } else if (strnicmp(cpustring, "\"SC000\"", tmp) == 0) {
                xxCPU = ARM_SC000;
            } else if (strnicmp(cpustring, "\"SC300\"", tmp) == 0) {
                xxCPU = ARM_SC300;
            } else if (strnicmp(cpustring, "\"Cortex-M7\"", tmp) == 0) {
                xxCPU = ARM_CM7;
            } else if ((strnicmp(cpustring, "\"ARMV8MBL\"", tmp) == 0)
                       || (strnicmp(cpustring, "\"Cortex-M23\"", tmp) == 0)) {
                xxCPU = ARM_CM23;
            } else if ((strnicmp(cpustring, "\"ARMV8MML\"", tmp) == 0)
                       || (strnicmp(cpustring, "\"Cortex-M33\"", tmp) == 0)) {
                xxCPU = ARM_CM33;
            } else if (strnicmp(cpustring, "\"Cortex-M35P\"", tmp) == 0) {
                xxCPU = ARM_CM35P;
            }
        }
    }

    switch (xxCPU) {
        case ARM_CM23:
            TraceConf.Opt &= (TRACE_BASELINE_SUPP);            // Remove unsupported trace features from options
            if ((MonConf.Opt & RESET_TYPE) == RESET_SW_VECT) { // No VECTRESET in v8-M
                MonConf.Opt &= ~RESET_TYPE;                    // Force to AutoDetect if project carries VECTRESET option.
            }
            break;
        case ARM_CM33:
        case ARM_CM35P:
            if ((MonConf.Opt & RESET_TYPE) == RESET_SW_VECT) { // No VECTRESET in v8-M
                MonConf.Opt &= ~RESET_TYPE;                    // Force to AutoDetect if project carries VECTRESET option.
            }
            break;
    }
}


/*
 * Create an argument string out of current 'MonConf' and 'FlashConf'.
 * This string will be registered with the current project.
 * Note: the maximum length of the string must not exceed 'ValSize-1'
 *       characters. ValSize is a member of QDLL (in Bom.h). The current
 *       maximum is 1024 characters including the zero terminator.
 */

void WriteParms(char *pArgs)
{
    int i;

    pArgs += sprintf(pArgs, "-U%s -O%d -S%d -C%d",
                     MonConf.UnitSerNo,
                     MonConf.Opt,
                     MonConf.SWJ_Clock,
                     MonConf.JtagCpuIndex);

    for (i = 0; i < (int)JTAG_devs.cnt; i++) {
        pArgs += sprintf(pArgs, " -N%02d(\"%s\") -D%02d(%08X) -L%02d(%d)",
                         i, JTAG_devs.icname[i],
                         i, JTAG_devs.ic[i].id,
                         i, JTAG_devs.ic[i].ir_len);
    }

    // pArgs += sprintf(pArgs, " -TO%d -TC%d -TP%X -TDS%X -TDT%X -TDC%X -TIE%X -TIP%X",
    pArgs += sprintf(pArgs, " -TO%d -TC%d -TT%d -TP%X -TDS%X -TDT%X -TDC%X -TIE%X -TIP%X", // 02.04.2019: Separate Trace Clock setting
                     TraceConf.Opt,
                     TraceConf.Clk,
                     TraceConf.TPIU_Clk, // 02.04.2019: Separate Trace Clock setting
                     (TraceConf.Protocol << 4) | TraceConf.PortSize,
                     TraceConf.SWV_Pre,
                     TraceConf.TS_Pre,
                     TraceConf.CYC_Pre,
                     TraceConf.ITM_Ena,
                     TraceConf.ITM_Priv);

    pArgs += sprintf(pArgs, " -FO%d -FD%X -FC%X -FN%X",
                     FlashConf.Opt,
                     FlashConf.RAMStart,
                     FlashConf.RAMSize,
                     FlashConf.Nitems);

    for (i = 0; i < FlashConf.Nitems; i++) {
        pArgs += sprintf(pArgs, " -FF%c%s", (i + '0'), &FlashConf.Dev[i].FileName[0]);
        pArgs += sprintf(pArgs, " -FS%c%X", (i + '0'), FlashConf.Dev[i].Start);
        pArgs += sprintf(pArgs, " -FL%c%X", (i + '0'), FlashConf.Dev[i].Size);
        //---7.11.2012: full path is defined:
        if (FlashConf.Dev[i].fPath[0] != 0) {
            pArgs += sprintf(pArgs, " -FP%c(%s)", (i + '0'), FlashConf.Dev[i].fPath);
        }
    }
}


/*
 * Start the Configuration-Parameter Setup Dialog.
 * Note: this dialog *MUST* be modal, modeless is not allowed !
 */

int DoDlgSetup(void)
{
    int i;
    CSetup     dlg;
    //CSetupPS dlg(0);

    dlg.page = 0;                // Start with Debug Page
    i = dlg.DoModal(); // run the target setup dialog...
    return (i);        // IDOK or IDCANCEL
}

/*
 * Start the Configuration-Parameter Flash Download Setup Dialog.
 * Note: this dialog *MUST* be modal, modeless is not allowed !
 */

int DoFDlgSetup(void)
{
    int i;
    CSetup     dlg;
    //CSetupPS dlg(2);

    dlg.page = 2;                // Start with Flash Download Page
    i = dlg.DoModal(); // run the flash download setup dialog...
    return (i);        // IDOK or IDCANCEL
}


/*
 * pio-bom-pointer now a member of 'struct dbgext' (former nRes8 member)
 */

void InitBomp(struct dbgblk *pD)
{
    pio = NULL;
    if (pD && pD->pDbgX) {
        pio = (struct bom *)pD->pDbgX->pBom; // Bom-pointer
    }
}

void InitDbgCM()
{
    pdbg = NULL;                              // startup values
    memset(&MonConf, 0, sizeof(MonConf));     // holds target-setup values
    memset(&FlashConf, 0, sizeof(FlashConf)); // holds Flash-setup values
    SetupMode = 0;                            // 1:=Remote Setup Mode via Options-Debug
    bAnyUnit  = true;                         // Flag for selection of Unit entry "Any"
}

void InitDll()
{
    InitDbgCM();
    InitAGDI();
    InitCSTF();
    InitCTI();
    InitDebug();
    InitELF();
    InitETB();
    InitFlash();
    InitJTAG();
    InitSetupDbg();
    InitSetupFD();
    InitSWD();
    InitSWV();
    InitTrace();
    InitTraceExc();
    InitTraceRec();
    TraceWinConnect();
    CrtInitBreakResources();
#if DBGCM_RECOVERY
    InitDebugAccess();
#endif // DBGCM_RECOVERY
#if DBGCM_DS_MONITOR
    InitDSMonitor();
#endif // DBGCM_DS_MONITOR
#if DBGCM_DBG_DESCRIPTION
    InitPDSCDebug();
#endif // DBGCM_DBG_DESCRIPTION
    CreateLinkCom();
}


/*
 * Find option values for
 */
XOPT *FindTargKey(XOPT *pL, char *key)
{
    for (; pL; pL = pL->next) {
        if (stricmp(pL->key, key) == 0) {
            return (pL);
        }
    }
    return (NULL);
}


/*
   <Key>ULP2CM3</Key>
   <Name>-UP0001JAE -O142 -S0 -C-1 -N00("ARM CoreSight SW-DP") -D00(2BA01477) -L00(0) -TO18 -TC10000000 -TP28 -TDX0 -TDD0 -TDS8007 -TDT0 -TDC1F -TIEFFFFFFFF -TIP8 -FO7 -FD20000000 -FC800 -FN1 -FF0STM32F10x_128 -FS08000000 -FL020000</Name>

   <Key>UL2CM3</Key>
   <Name>-UV0760C8E -O14 -S0 -C0 -N00("ARM CoreSight JTAG-DP") -D00(3BA00477) -L00(4) -FO7 -FD20000000 -FC800 -FN1 -FF0STM32F10x_128 -FS08000000 -FL020000</Name>
*/
char *ULINKOptions(int bFlash)
{
    XOPT *Op;
    XOPT *pL;

    if (pdbg == NULL || pdbg->pDbgX == NULL) {
        return NULL;
    }
    if (bFlash) { // Utils/Flash Page - settings
        pL = pdbg->pDbgX->pOpUtil;
    } else { // Debug-Page - target settings
        pL = pdbg->pDbgX->pOpTarg;
    }
    Op = FindTargKey(pL, "UL2CM3"); // search for "UL2CM3" in list
    if (Op != NULL)
        return (Op->name);
    Op = FindTargKey(pL, "UL2V8M"); // search for "UL2V8M" in list
    if (Op != NULL)
        return (Op->name);
    Op = FindTargKey(pL, "ULP2CM3"); // search for "ULP2CM3" in list
    if (Op != NULL)
        return (Op->name);
    Op = FindTargKey(pL, "ULP2V8M"); // search for "ULP2V8M" in list
    if (Op != NULL)
        return (Op->name);
    Op = FindTargKey(pL, "ULPL2CM3"); // search for "ULPL2CM3" in list
    if (Op != NULL)
        return (Op->name);
    Op = FindTargKey(pL, "CMSIS_AGDI"); // search for "CMSIS_AGDI" in list
    if (Op != NULL)
        return (Op->name);
    Op = FindTargKey(pL, "CMSIS_AGDI_V8M"); // search for "CMSIS_AGDI_V8M" in list
    if (Op != NULL)
        return (Op->name);

    return NULL;
}


#ifdef __cplusplus
extern "C" { // must avoid C++ mangled names here !
#endif

/*
 * Checker Entry for Target-Dll loader
 *  Note: When the uVision2 debugger is startet in target mode, this
 *        function is called first to check the validity of the DLL.
 *
 * The name of this function depends on architecture:
 *        80167: EnumUv3167()
 *        8051:  EnumUv351()
 *        80251: EnumUv3251()
 *        ARM  : EnumUvARM7()  (Arm7, Arm9, Cortex-M/R)
 */

int _EXPO_ EnumUvARM7(void *p, DWORD nCode)
{
    char *pArgs;

    switch (nCode) {
        case 0:
            break;
        case 1: // not used
            break;
        case 2: // register debug block
            InitDll();
            pdbg  = (struct dbgblk *)p; // defined in ComTyp.h
            pArgs = (pdbg->TargArgs && pdbg->TargArgs[0]) ? pdbg->TargArgs : ULINKOptions(0);
            AnalyzeParms(pdbg->TargDll, pArgs);
            return (7); // Ok.
    }
    return (0); // Ok.
}


/*
 * This function is required for Remote-Setup by Target-Options-Debug
 * and Target-Options-Utilities Sheet.
 */

const char szTFlWar[] = { "Flash Download Warning" };
const char szNoFunc[] = { "Nothing to do. All Download Functions are now disabled." };


int _EXPO_ DllUv3Cap(DWORD nCode, void *p)
{
    QDLL *pQ;
    int   i = 0;
    char *pArgs;

    switch (nCode) {
        case 2:    // match family
            i = 7; // identify as ARM Target driver
            break;

        case 100:       // has Flash-DownLoad Capabilities
            return (1); // say 'Yes'

        case 1:                    // Cpu/Target-DLL Settings
            SetupMode = 1;         // we are just doing remote setup
            pQ        = (QDLL *)p; // refer to Bom.H for 'QDLL'

            // Use ULINK2 Options if ULINKpro Options don't exist
            pArgs = (pQ->value && pQ->value[0]) ? pQ->value : ULINKOptions(0);

            AnalyzeParms(pQ->pathUv3, pArgs);
            i = DoDlgSetup();          // start Target Setup Dialog...
            if (i == IDOK) {           // take the values...
                WriteParms(pQ->value); // create an ASCII string out of MonConf
                i = 1;
            } else
                i = 0; // don't register string in project.
            SetupMode = 0;
            break;

        case 109:                      // Get Debug Block
            pdbg = (struct dbgblk *)p; // defined in ComTyp.h
            InitBomp(pdbg);
            break;

        case 110:          // Cpu/Target/Flash-DLL Settings
            SetupMode = 1; // we are just doing remote setup
            pQ        = (QDLL *)p;

            // Use ULINK2 Options if ULINKpro Options don't exist
            pArgs = (pQ->value && pQ->value[0]) ? pQ->value : ULINKOptions(1);

            AnalyzeParms(pQ->pathUv3, pArgs);
            i = DoFDlgSetup(); // Flash Download Setup Dialog...
            if (i == IDOK) {
                WriteParms(pQ->value);
                if (!(FlashConf.Opt & (FLASH_ERASE | FLASH_PROGRAM | FLASH_VERIFY | FLASH_RESETRUN)))
                    AGDIMsgBox(hMfrm, szNoFunc, szTFlWar, MB_OK | MB_ICONEXCLAMATION, IDOK);
                i = 1;
            } else
                i = 0; // don't register MonConf changes...
            SetupMode = 0;
            break;
    }
    return (i); // return value
}


#ifdef __cplusplus
} // End 'extern "C"'
#endif


//  Link Communication Mutex (prevent multiple commands), used normally in JTAG.cpp, SWD.cpp and Trace modules.
// This mutex syncs the communication with the Debug unit
//    Parameter:      stat: 1 - Open Communication, 1 - Close Communication
//    Return Value:   0 - OK,  else Error Code
#if 1                        // Example Code
static HANDLE LinkMutex = 0; // Communication Mutex

HANDLE kDebugAccessMutex = 0;

#define EL_MUTEX_AGDI_LINK_COM_NAME     "elaphure.Mutex.agdi.link"
#define EL_MUTEX_AGDI_DEBUG_ACCESS_NAME "elaphure.Mutex.agdi.debugAcc"

static int CreateLinkCom()
{
    // Create mutex (buf holds unique name for the USB device)
    LinkMutex         = CreateMutex(NULL, FALSE, EL_MUTEX_AGDI_LINK_COM_NAME);
    kDebugAccessMutex = CreateMutex(NULL, FALSE, EL_MUTEX_AGDI_DEBUG_ACCESS_NAME);

    return 0;
}


void DeleteLinkCom()
{
    // Delete mutex
    if (LinkMutex) {
        CloseHandle(LinkMutex);
        LinkMutex = 0;
    }
}
#endif


int LinkCom(int stat)
{
    static int lockStatus = 0;

    // if (stat) {
    //     DEVELOP_MSG("Todo: \nLock target communication. The following must not be interrupted.");
    //     if (lockStatus)
    //         return EU12;
    //     lockStatus = 1;
    // } else {
    //     DEVELOP_MSG("Todo: \nUnlock target communication.");
    //     lockStatus = 0;
    // }

#if 1
    DWORD res;

    if (!LinkMutex)
        return (EU12);

    if (stat) {
        res = WaitForSingleObject(LinkMutex, 3000); // Wait 3s to get Mutex
        if (res == WAIT_TIMEOUT)
            return (EU13); // Timeout
        if (res != WAIT_OBJECT_0)
            return (EU12);
    } else {
        if (!ReleaseMutex(LinkMutex))
            return (EU12);
    }
#endif

    return (0);
}
