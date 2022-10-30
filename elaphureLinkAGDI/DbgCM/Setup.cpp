/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.3
 * @date     $Date: 2016-11-16 18:18:17 +0100 (Wed, 16 Nov 2016) $
 *
 * @note
 * Copyright (C) 2009-2016 ARM Limited. All rights reserved.
 *
 * @brief     General Setup Dialog for the Tabbed Dialogs (Debug, Trace, Flash Download)
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
#include "..\ComTyp.h"
#include "Collect.h"
#include "DbgCM.h"
#include "Setup.h"

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION


/******************************************************************************/

static const char INPUT_ERR_TITLE[] = "Invalid number";
static const char INPUT_ERRMSG[] =
    "You have entered an invalid number!\n"
    "The previous value will be restored.\n"
    "Examples:\n"
    "0x1234ABCD   hex number\n"
    "1234ABCDH     hex number\n"
    "27021974        decimal number\n"
    "1234567O        octal number\n"
    "10101010B       binary number\n";

static const char INPUT_ERD_TITLE[] = "Invalid number";
static const char INPUT_ERDMSG[] =
    "You have entered an invalid number!\n"
    "The previous value will be restored.";

static const char INPUT_ERF_TITLE[] = "Invalid float number";
static const char INPUT_ERFMSG[] =
    "You have entered an invalid float number!\n"
    "The previous value will be restored.\n"
    "Example: 1.234\n";

static const char INPUT_OVR_TITLE[] = "Out of range";
static const char INPUT_OVRMSG[] =
    "You have entered a number that is out of range!\n"
    "The previous value will be restored.\n";

void StringHex2(CWnd *pCWnd, BYTE val)
{
    char buf[16];

    sprintf(buf, "0x%02X", val);
    pCWnd->SetWindowText(buf);
}

void StringHex4(CWnd *pCWnd, WORD val)
{
    char buf[16];

    sprintf(buf, "0x%04X", val);
    pCWnd->SetWindowText(buf);
}

void StringHex8(CWnd *pCWnd, DWORD val)
{
    char buf[16];

    sprintf(buf, "0x%08X", val);
    pCWnd->SetWindowText(buf);
}

void StringDec(CWnd *pCWnd, DWORD val)
{
    char buf[16];

    sprintf(buf, "%u", val);
    pCWnd->SetWindowText(buf);
}

void StringDouble(CWnd *pCWnd, double val, int precision)
{
    char buf[32];

    sprintf(buf, "%1.*f", precision, val);
    pCWnd->SetWindowText(buf);
}

BOOL GetDlgHex2(CWnd *pCWnd, BYTE oldval, BYTE *newval)
{
    DWORD val;
    WORD  n;
    char  buf[16];

    n      = pCWnd->GetWindowText(buf, sizeof(buf));
    buf[n] = '\0'; /* terminate string */
    n      = sscanf(buf, "%i", &val);
    if (n != 1) {
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(&INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
        StringHex2(pCWnd, oldval);
        return (FALSE);
    }
    StringHex2(pCWnd, (BYTE)val);
    *newval = (BYTE)val;
    return (TRUE);
}

BOOL GetDlgHex4(CWnd *pCWnd, WORD oldval, WORD *newval)
{
    DWORD val;
    WORD  n;
    char  buf[16];

    n      = pCWnd->GetWindowText(buf, sizeof(buf));
    buf[n] = '\0'; /* terminate string */
    n      = sscanf(buf, "%i", &val);
    if (n != 1) {
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(&INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
        StringHex4(pCWnd, oldval);
        return (FALSE);
    }
    StringHex4(pCWnd, (WORD)val);
    *newval = (WORD)val;
    return (TRUE);
}

BOOL GetDlgHex8(CWnd *pCWnd, DWORD oldval, DWORD *newval)
{
    DWORD val;
    WORD  n;
    char  buf[16];

    n      = pCWnd->GetWindowText(buf, sizeof(buf));
    buf[n] = '\0'; /* terminate string */
    n      = sscanf(buf, "%i", &val);
    if (n != 1) {
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(&INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
        StringHex8(pCWnd, oldval);
        return (FALSE);
    }
    StringHex8(pCWnd, val);
    *newval = val;
    return (TRUE);
}

DWORD GetDlgDec(CWnd *pCWnd, DWORD oldval, DWORD min, DWORD max)
{
    DWORD val;
    WORD  n;
    char  lbuf[48 + 96];
    char  lbuf0[48];

    n       = pCWnd->GetWindowText(lbuf, 48);
    lbuf[n] = '\0'; /* terminate string */
    n       = sscanf(lbuf, "%u", &val);
    if (n != 1) {
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(INPUT_ERDMSG, INPUT_ERD_TITLE, MB_OK|MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, INPUT_ERDMSG, INPUT_ERD_TITLE, MB_OK | MB_ICONSTOP, IDOK);
        StringDec(pCWnd, oldval);
        return (-1);
    }
    if (val < min) {
        sprintf(lbuf0, "Minimum value = %u", min);
        strcpy(lbuf, INPUT_OVRMSG);
        strcat(lbuf, lbuf0);
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP, IDOK);
        StringDec(pCWnd, oldval);
        return (-1);
    }
    if (val > max) {
        sprintf(lbuf0, "Maximum value = %u", max);
        strcpy(lbuf, INPUT_OVRMSG);
        strcat(lbuf, lbuf0);
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP, IDOK);
        StringDec(pCWnd, oldval);
        return (-1);
    }
    StringDec(pCWnd, val);
    return (val);
}

double GetDlgDouble(CWnd *pCWnd, double oldval, double min, double max, int precision)
{
    double val;
    WORD   n;
    char   lbuf[48 + 96];
    char   lbuf0[48];

    n       = pCWnd->GetWindowText(lbuf, 48);
    lbuf[n] = '\0'; /* terminate string */
    n       = sscanf(lbuf, "%lf", &val);
    if (n != 1) {
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(INPUT_ERFMSG, INPUT_ERF_TITLE, MB_OK|MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, INPUT_ERFMSG, INPUT_ERF_TITLE, MB_OK | MB_ICONSTOP, IDOK);
        StringDouble(pCWnd, oldval, precision);
        return (-1.0e-308);
    }
    if (val < min) {
        sprintf(lbuf0, "Minimum value = %1.*f", precision, min);
        strcpy(lbuf, INPUT_OVRMSG);
        strcat(lbuf, lbuf0);
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP, IDOK);
        StringDouble(pCWnd, oldval, precision);
        return (-1.0e-308);
    }
    if (val > max) {
        sprintf(lbuf0, "Maximum value = %1.*f", precision, max);
        strcpy(lbuf, INPUT_OVRMSG);
        strcat(lbuf, lbuf0);
        MessageBeep(MB_ICONEXCLAMATION);
        //pCWnd->MessageBox(lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP);
        AGDIMsgBox(pCWnd->m_hWnd, lbuf, INPUT_OVR_TITLE, MB_OK | MB_ICONSTOP, IDOK);
        StringDouble(pCWnd, oldval, precision);
        return (-1.0e-308);
    }
    StringDouble(pCWnd, val, precision);
    return (val);
}

/******************************************************************************/


// CSetupPS

IMPLEMENT_DYNAMIC(CSetupPS, CPropertySheet)

#if 0
CSetupPS::CSetupPS(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
  :COptionSheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CSetupPS::CSetupPS(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
  :COptionSheet(pszCaption, pParentWnd, iSelectPage)
{
}
#endif

CSetupPS::CSetupPS(int iInitalPage)
    : CPropertySheet(_T("Cortex-M Target Driver Setup"), NULL, iInitalPage)
{
    AddPage(&pageDbg);
    //AddPage(&pageTrc);

    if (SetupMode) {
        AddPage(&pageFD);
    }
#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsInitialized()) {
        PDSCDebug_Init();
    }
    if (!PDSCDebug_HasDebugPropertiesBackup()) {
        PDSCDebug_CreateDebugPropertiesBackup();
    }
    if (PDSCDebug_IsSupported()) {
        AddPage(&pagePdsc);
    }
#endif // DBGCM_DBG_DESCRIPTION
}

CSetupPS::~CSetupPS()
{
#if DBGCM_DBG_DESCRIPTION
    if (SetupMode) {
        PDSCDebug_UnInit(); // Cleanup
    } else {
        PDSCDebug_SendDebugProperties(); // Send update to UV4 engine
    }
#endif // DBGCM_DBG_DESCRIPTION
}

BEGIN_MESSAGE_MAP(CSetupPS, CPropertySheet)
ON_BN_CLICKED(IDHELP, OnHelp)
END_MESSAGE_MAP()


// CSetupPS message handlers


// CSetup dialog

IMPLEMENT_DYNAMIC(CSetup, CDialog)

CSetup::CSetup(CWnd *pParent /*=NULL*/)
    : CDialog(CSetup::IDD, pParent)
{
}

CSetup::~CSetup()
{
}

BEGIN_MESSAGE_MAP(CSetup, CDialog)
ON_BN_CLICKED(IDHELP, OnHelp)
END_MESSAGE_MAP()


// CSetup message handlers

BOOL CSetup::OnInitDialog()
{
    CDialog::OnInitDialog();

    ps.AddPage(&ps.pageDbg);
    //ps.AddPage(&ps.pageTrc);

    if (SetupMode) {
        ps.AddPage(&ps.pageFD);
    }
#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsInitialized()) {
        PDSCDebug_Init();
    }
    if (!PDSCDebug_HasDebugPropertiesBackup()) {
        PDSCDebug_CreateDebugPropertiesBackup();
    }
#endif // DBGCM_DBG_DESCRIPTION

    ps.SetActivePage(page);

    ps.Create(this, WS_CHILD | WS_VISIBLE, WS_EX_CONTROLPARENT);

    ps.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

    return (TRUE);
}


void CSetup::OnOK()
{
    CDialog::OnOK();
}


void CSetup::OnCancel()
{
    CDialog::OnCancel();
}
