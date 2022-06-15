/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.1
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016 ARM Limited. All rights reserved.
 *
 * @brief     PDSC Debug Description Setup Dialog
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

#include "Collect.h"

#if DBGCM_DBG_DESCRIPTION

#include "DbgCM.h"
#include "Setup.h"
#include "SetupPdsc.h"

#include "..\BOM.H"

#include "PDSCDebug.h"


// CSetupDbg dialog

IMPLEMENT_DYNCREATE(CSetupPdsc, CPropertyPage)

CSetupPdsc::CSetupPdsc()
    : CPropertyPage(CSetupPdsc::IDD)
{
    m_pImageListState = NULL;
}

CSetupPdsc::~CSetupPdsc()
{
    if (m_pImageListState != NULL)
        delete m_pImageListState;
}

BEGIN_MESSAGE_MAP(CSetupPdsc, CPropertyPage)
ON_BN_CLICKED(IDC_CHECK_PDSC_DEBUG, OnCheckPdscEnable)
ON_BN_CLICKED(IDC_CHECK_PDSC_LOG, OnCheckPdscLog)
ON_BN_CLICKED(IDC_BUTTON_PDSC_DBGCONF_EDIT, OnBnDbgConfEdit)
END_MESSAGE_MAP()


// CSetupPdsc message handlers

void CSetupPdsc::Update(void)
{
    const char *pPackId      = PDSCDebug_GetPackId();
    const char *pLogFile     = PDSCDebug_GetLogFile();
    const char *pDbgConfName = PDSCDebug_GetDbgConfFilePath();
    CString     constrLogStr;

    if (pLogFile && pLogFile[0] != '\0') {
        constrLogStr = pLogFile;
        constrLogStr += "_*.log";
    }

    // Pack ID
    GetDlgItem(IDC_EDIT_PDSC_PACKID)->SetWindowText(pPackId ? pPackId : "<ID Not Found>");

    // Enable
    CheckDlgButton(IDC_CHECK_PDSC_DEBUG, PDSCDebug_IsEnabled() ? 1 : 0);

    // Log
    CheckDlgButton(IDC_CHECK_PDSC_LOG, PDSCDebug_IsLogEnabled() ? 1 : 0);
    GetDlgItem(IDC_CHECK_PDSC_LOG)->EnableWindow(PDSCDebug_IsEnabled() ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT_PDSC_LOGFILE)->EnableWindow((PDSCDebug_IsEnabled() && PDSCDebug_IsLogEnabled()) ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT_PDSC_LOGFILE)->SetWindowText(constrLogStr.IsEmpty() ? "" : constrLogStr.GetString());

    // DBGCONF Options
    CString modpath;

    // At the moment we only use the automatically copied dbgconf files. They are relative to the project folder.
    // Cut it off to avoid the alignment/h-scrolling hazzle for long paths in edit control.
    if (pDbgConfName && pdbg && pdbg->pjPath) {
        if (StrNCmpI(pdbg->pjPath, pDbgConfName, strlen(pdbg->pjPath)) == 0) {
            modpath = ".\\";
            modpath.Append(&(pDbgConfName[strlen(pdbg->pjPath)]));
            pDbgConfName = modpath.GetString();
        }
    }
    GetDlgItem(IDC_STATIC_PDSC_DBGCONF)->EnableWindow((PDSCDebug_IsEnabled() && pDbgConfName) ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT_PDSC_DBGCONF)->EnableWindow((PDSCDebug_IsEnabled() && pDbgConfName) ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT_PDSC_DBGCONF)->SetWindowText(pDbgConfName ? pDbgConfName : "");
    GetDlgItem(IDC_BUTTON_PDSC_DBGCONF_EDIT)->EnableWindow((PDSCDebug_IsEnabled() && pDbgConfName) ? TRUE : FALSE);
}

BOOL CSetupPdsc::OnInitDialog()
{
    //CListCtrl * pLC;
    //CWinApp   * pApp;

    CPropertyPage::OnInitDialog();

#if 0
  ToolTip.Create(this);

  TOOLINFO ti;
  ti.cbSize = sizeof(TOOLINFO);
  ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  ti.hwnd   = GetSafeHwnd();
  ti.hinst  = AfxGetInstanceHandle();

  ti.uId = (UINT)GetDlgItem(IDC_CONFIG_SWJ)->GetSafeHwnd();
  ti.lpszText = "Use SWJ Switching";
  ToolTip.SendMessage (TTM_ADDTOOL, 0, (LPARAM) &ti);

  ToolTip.Activate(TRUE);
#endif

#if 0
  // fill in image lists
  m_pImageListState = new CImageList();
//  ASSERT(m_pImageList != NULL && m_pImageListSmall != NULL);    // serious allocation failure checking
  m_pImageListState->Create(16, 16, TRUE, 3, 3);
  pApp = AfxGetApp();
  m_pImageListState->Add(pApp->LoadIcon(IDI_ICONLIST1));
  m_pImageListState->Add(pApp->LoadIcon(IDI_ICONLIST2));
  m_pImageListState->Add(pApp->LoadIcon(IDI_ICONLIST3));
//m_pImageListSmall->Add(pApp->LoadIcon(IDI_ICONLIST4));
#endif

    if (!PDSCDebug_IsInitialized()) {
        PDSCDebug_Init();
    }
    if (!PDSCDebug_HasDebugPropertiesBackup()) {
        PDSCDebug_CreateDebugPropertiesBackup();
    }

    Update();

    return (TRUE);
}

BOOL CSetupPdsc::OnSetActive()
{
    Update();
    return CPropertyPage::OnSetActive();
}

void CSetupPdsc::OnCheckPdscEnable()
{
    PDSCDebug_Enable(IsDlgButtonChecked(IDC_CHECK_PDSC_DEBUG) ? true : false);
    if (PDSCDebug_IsEnabled()) {
        GetDlgItem(IDC_CHECK_PDSC_LOG)->EnableWindow(TRUE);
        CheckDlgButton(IDC_CHECK_PDSC_LOG, PDSCDebug_IsLogEnabled() ? 1 : 0);
    } else {
        GetDlgItem(IDC_CHECK_PDSC_LOG)->EnableWindow(FALSE);
        // Do not update log state
    }

    Update();
}

void CSetupPdsc::OnCheckPdscLog()
{
    // Maybe even send a notification to UV to enable/disable the log (debugportstart and so forth).
    PDSCDebug_LogEnable(IsDlgButtonChecked(IDC_CHECK_PDSC_LOG) != 0);
    PDSCDebug_SendDebugProperties();

    Update();
}

void CSetupPdsc::OnBnDbgConfEdit()
{
    CString path;
    GetDlgItem(IDC_EDIT_PDSC_DBGCONF)->GetWindowText(path);
    if (path.IsEmpty()) {
        return;
    }

    pio->Notify(UV_EDIT_DOCUMENT, (void *)path.GetString());
}

#endif // DBGCM_DBG_DESCRIPTION