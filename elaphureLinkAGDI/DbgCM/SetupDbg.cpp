/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.5
 * @date     $Date: 2016-11-17 14:34:25 +0100 (Thu, 17 Nov 2016) $
 *
 * @note
 * Copyright (C) 2009-2016 ARM Limited. All rights reserved.
 *
 * @brief     Debug Setup Dialog
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
#include "JTAG.h"
#include "SWD.h"
#include "Setup.h"
#include "SetupDbg.h"

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION

#include "rddi_dll.hpp"

static int k_if_selected = 0;

// CSetupDbg dialog

IMPLEMENT_DYNCREATE(CSetupDbg, CPropertyPage)

CSetupDbg::CSetupDbg()
    : CPropertyPage(CSetupDbg::IDD)
{
    m_sernum          = _T("");
    m_hversion        = _T("");
    m_fversion        = _T("");
    m_pImageListState = NULL;
}

CSetupDbg::~CSetupDbg()
{
    if (m_pImageListState != NULL)
        delete m_pImageListState;
}

BEGIN_MESSAGE_MAP(CSetupDbg, CPropertyPage)
ON_CBN_SELCHANGE(IDC_CONFIG_UNIT, OnSelchangeConfigUnit)
ON_CBN_SELCHANGE(IDC_CONFIG_PORT, OnSelchangeConfigPort)
ON_CBN_SELCHANGE(IDC_CONFIG_CLK, OnSelchangeConfigClk)
ON_BN_CLICKED(IDC_CONFIG_SWJ, OnConfigSwj)
ON_BN_CLICKED(IDC_CACHE_CODE, OnCacheCode)
ON_BN_CLICKED(IDC_CACHE_MEM, OnCacheMem)
ON_BN_CLICKED(IDC_CODE_VERIFY, OnCodeVerify)
ON_BN_CLICKED(IDC_FLASH_LOAD, OnFlashLoad)
ON_BN_CLICKED(IDC_BOOT_RESET, OnBootReset)
ON_BN_CLICKED(IDC_BOOT_RUN, OnBootRun)
ON_BN_CLICKED(IDC_RST_VECT_CATCH, OnRstVectCatch)
ON_CBN_SELCHANGE(IDC_RST_TYPE, OnSelchangeRstType)
ON_CBN_SELCHANGE(IDC_INIT_RST, OnSelchangeInitRst)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_CONFIG_DEVICE, OnItemchangedConfigDevice)
ON_BN_CLICKED(IDC_JTAG_AUTO, OnJtagAuto)
ON_BN_CLICKED(IDC_JTAG_MANUAL, OnJtagManual)
ON_BN_CLICKED(IDC_JTAG_UPDATE, OnJtagUpdate)
ON_BN_CLICKED(IDC_JTAG_ADD, OnJtagAdd)
ON_BN_CLICKED(IDC_JTAG_DELETE, OnJtagDelete)
ON_BN_CLICKED(IDC_JTAG_UP, OnJtagUp)
ON_BN_CLICKED(IDC_JTAG_DOWN, OnJtagDown)
ON_EN_KILLFOCUS(IDC_CONFIG_AP, OnKillfocusConfigAP)
END_MESSAGE_MAP()


// CSetupDbg message handlers

static const char INPUT_ERR_TITLE[] = "Invalid entry";
static const char INPUT_ERRMSG[] =
    "At least one JTAG device information is not correct!\n"
    "Example:\n"
    "ID CODE (32bit hex number): 0x0BA00477\n"
    "Device Name (max. 30 char): ARM CoreSight JTAG-DP\n"
    "IR len (value range 1-20):  4\n";

static const char *jtagdlgheader[] = { "IDCODE", "Device Name", "IR len" };
static const int   jtagdlgfmt[]    = { LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_CENTER };

static const int jtagmanctrlh[] = { IDC_JTAG_UPDATE, IDC_JTAG_DELETE, IDC_JTAG_UP, IDC_JTAG_DOWN }; // controls to hide on update

static const int jtagmanctrls[] = { IDC_JTAG_ADD, IDC_JTAG_ID, IDC_JTAG_DEVNAME, IDC_JTAG_IRLEN, // conrtols to show or hide on update
                                    IDC_STATIC_ID, IDC_STATIC_DEVNAME, IDC_STATIC_IRLEN, IDC_STATIC_MOVE };

static const char *swjclk[] = {
    "10MHz", "5MHz", "2MHz", "1MHz", "500kHz", "200kHz", "100kHz", "50kHz", "20kHz", "10kHz", "5kHz"
};

const char *clkspeed[] = {
    "10000000", "5000000", "2000000", "1000000",
    "500000", "200000", "100000", "50000", "20000", "10000", "5000"
};

static int SelectedJTAGItem;

void CSetupDbg::Update(void)
{
    char       tbuf[512];
    char      *pchar;
    int        status = 0, tmp;
    int        ShowControl;
    CListCtrl *pLC;
    LVCOLUMN   col;
    LVITEM     item, subitem;
    DWORD      itemindex;
    DWORD      i, j, k;
    CComboBox *pC;
    int cur_sel = 0;
    int numOfIFs = 0;
#if DBGCM_DBG_DESCRIPTION
    bool pdscerr = false;
#endif // DBGCM_DBG_DESCRIPTION


    if (SetupMode) {
        // Detect available Unit

        // Add Units to list
        pC = (CComboBox *)GetDlgItem(IDC_CONFIG_UNIT);
        pC->ResetContent();   // reset list of UNITs
        pC->AddString("Any"); // Select "Any" (first) Unit

        rddi::rddi_Open(&rddi::k_rddi_handle, NULL);
        rddi::CMSIS_DAP_Detect(rddi::k_rddi_handle, &numOfIFs);
        for (int i = 0; i < numOfIFs; i++) {
            rddi::CMSIS_DAP_Identify(rddi::k_rddi_handle, i, 2, tbuf, sizeof(tbuf));
            pC->AddString(tbuf);
        }
    }
    GetDlgItem(IDC_CONFIG_UNIT)->EnableWindow(SetupMode ? TRUE : FALSE);

    // Select Unit
    if (bAnyUnit) { // Select "Any" (first) Unit
        ((CComboBox *)GetDlgItem(IDC_CONFIG_UNIT))->SetCurSel(0);
        // Select first Unit found
    } else {
        if (k_if_selected == 0) {
            if (numOfIFs == 0) {
                cur_sel = 0;
            } else {
                cur_sel = 1;
            }
        } else {
            cur_sel = k_if_selected;
        }
        // if more available than one decide based on project settings MonConf.UnitSerNo[]
        ((CComboBox *)GetDlgItem(IDC_CONFIG_UNIT))->SetCurSel(cur_sel); // Unit number + "Any" entry
        status = 0;
    }
    //status = EU02;                           // No Debug Unit found


    // Display Unit information: Serial Number, HW Ver, SW Ver
    if (cur_sel != 0) {
        rddi::k_rddi_if_index = cur_sel - 1;

        rddi::CMSIS_DAP_Identify(rddi::k_rddi_handle, rddi::k_rddi_if_index, 2, tbuf, sizeof(tbuf));
        for (char *p = tbuf; *p != '\0'; p++) {
            if (*p == ' ' || *p == '-') {
                *p = '_';
            }
        }
        strncpy(MonConf.UnitSerNo, tbuf, sizeof(MonConf.UnitSerNo));
        MonConf.UnitSerNo[sizeof(MonConf.UnitSerNo) - 1] = '\0';

        // set info item
        rddi::CMSIS_DAP_Identify(rddi::k_rddi_handle, rddi::k_rddi_if_index, 3, tbuf, sizeof(tbuf));
        SetDlgItemText(IDC_CONFIG_SERNUM, tbuf);

        SetDlgItemText(IDC_CONFIG_HVERSION, "");

        rddi::CMSIS_DAP_Identify(rddi::k_rddi_handle, rddi::k_rddi_if_index, 4, tbuf, sizeof(tbuf));
        SetDlgItemText(IDC_CONFIG_FVERSION, tbuf);
    } else {
        rddi::k_rddi_if_index = -1;

        strcpy(MonConf.UnitSerNo, "Any");

        SetDlgItemText(IDC_CONFIG_SERNUM, "");
        SetDlgItemText(IDC_CONFIG_HVERSION, "");
        SetDlgItemText(IDC_CONFIG_FVERSION, "");

        status = EU02; // No Debug Unit found
    }


    GetDlgItem(IDC_CONFIG_SERNUM)->EnableWindow((status == 0) ? TRUE : FALSE);
    //GetDlgItem(IDC_CONFIG_HVERSION)->EnableWindow((status == 0) ? TRUE : FALSE);
    GetDlgItem(IDC_CONFIG_FVERSION)->EnableWindow((status == 0) ? TRUE : FALSE);

    if (SetupMode && (status == 0)) {
        // Init Target and configure according MonConf (Debug Port & Clock ...)
        sprintf(tbuf, "Master=Y;Port=%s;SWJ=Y;Clock=%s;",
                (MonConf.Opt & PORT_SW) ? "SW" : "JTAG",
                clkspeed[MonConf.SWJ_Clock]);

        rddi::CMSIS_DAP_ConfigureInterface(rddi::k_rddi_handle, cur_sel - 1, tbuf);


#if DBGCM_DBG_DESCRIPTION
        if (PDSCDebug_IsEnabled()) {
            // May have to split this into an Init and a Reinit
            // TODO: Is there a possible critical error during reinit??
            PDSCDebug_Reinit();
            status  = PdscTargetConnect();
            pdscerr = (status >= EU22 && status <= EU38);
        } else {
#endif // DBGCM_DBG_DESCRIPTION

            if ((MonConf.Opt & PORT_SW) || ((MonConf.Opt & JTAG_MANUAL) == 0)) {
                // Update the device list when in automatic mode
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
                    if (MonConf.Opt & USE_SWJ) {
                        // SWJ Switch to SWD
                        SWJ_Reset();
                        SWJ_Switch(0xE79E);
                    }
                    SWJ_Reset();
                    status = SWD_ReadID();
                } else {
                    if (MonConf.Opt & USE_SWJ) {
                        // SWJ Switch to JTAG
                        SWJ_Reset();
                        SWJ_Switch(0xE73C);
                        SWJ_Reset();
                    } else {
                        JTAG_Reset();
                    }
                    status = JTAG_DetectDevices();
                    if (status == EU03) {
                        status = 0; // ignore error: No JTAG Devices Found
                    }
                    if (status == EU04) {
                        status        = 0; // ignore error: Too Many JTAG Devices in Chain
                        JTAG_devs.cnt = 0;
                    }
                }
                if (MonConf.Opt & INIT_RST_HOLD) {
                    //---TODO:
                    // Deassert HW Chip Reset
                    DEVELOP_MSG("Todo: \nDeassert HW Chip Reset");
                }
            }
            //---TODO:
            // UnInit Target

#if DBGCM_DBG_DESCRIPTION
        }
#endif // DBGCM_DBG_DESCRIPTION
    }

    // Debug Port & SWJ Switching and Clock
#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        if (PDSCDebug_ProtocolSupported(PROTOCOL_JTAG) && PDSCDebug_ProtocolSupported(PROTOCOL_SWD)) {
            GetDlgItem(IDC_CONFIG_PORT)->EnableWindow(SetupMode ? TRUE : FALSE);
        } else {
            GetDlgItem(IDC_CONFIG_PORT)->EnableWindow(FALSE);
        }
    } else {
        GetDlgItem(IDC_CONFIG_PORT)->EnableWindow(SetupMode ? TRUE : FALSE);
        GetDlgItem(IDC_CONFIG_SWJ)->EnableWindow(SetupMode ? TRUE : FALSE);
    }
    GetDlgItem(IDC_CONFIG_CLK)->EnableWindow(SetupMode ? TRUE : FALSE);
    // PDSC: PORT_SW bit setting adjusted accordingly by PDSCDebug_Reinit()
#else  // DBGCM_DBG_DESCRIPTION
    GetDlgItem(IDC_CONFIG_PORT)->EnableWindow(SetupMode ? TRUE : FALSE);
    GetDlgItem(IDC_CONFIG_SWJ)->EnableWindow(SetupMode ? TRUE : FALSE);
    GetDlgItem(IDC_CONFIG_CLK)->EnableWindow(SetupMode ? TRUE : FALSE);
#endif // DBGCM_DBG_DESCRIPTION
    ((CComboBox *)GetDlgItem(IDC_CONFIG_PORT))->SetCurSel((MonConf.Opt & PORT_SW) ? 1 : 0);
    CheckDlgButton(IDC_CONFIG_SWJ, (MonConf.Opt & USE_SWJ) ? 1 : 0);

    // SWJ Clock
    pC = (CComboBox *)GetDlgItem(IDC_CONFIG_CLK);
    pC->ResetContent(); // reset Max SWJ Clock
    j = 0;
    if ((MonConf.Opt & PORT_SW) == 0) {
        pC->AddString("RTCK");
        pC->SetItemData(0, 0x80);
        j++;
    }
    for (i = 0; i < (sizeof(swjclk) / sizeof(swjclk[0])); i++, j++) {
        pC->AddString(swjclk[i]);
        pC->SetItemData(j, i);
    }
    if (MonConf.SWJ_Clock & 0x80) {
        i = 0;
    } else {
        i = (MonConf.SWJ_Clock & 0x7F) + ((MonConf.Opt & PORT_SW) ? 0 : 1);
    }
    pC->SetCurSel(i);

    SelectedJTAGItem = -1; // no JTAG device is selected after an dialog update

    if (MonConf.Opt & PORT_SW) {
        SetDlgItemText(IDC_STATIC_DEVICE, "SW Device");
        SetDlgItemText(IDC_STATIC_TDO, "SWDIO");
        //  GetDlgItem(IDC_STATIC_TDO)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_TDI)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_IRLEN)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_JTAG_IRLEN)->ShowWindow(SW_HIDE);
        MonConf.Opt &= ~JTAG_MANUAL;
    } else {
        SetDlgItemText(IDC_STATIC_DEVICE, "JTAG Device Chain");
        SetDlgItemText(IDC_STATIC_TDO, "TDO");
        //  GetDlgItem(IDC_STATIC_TDO)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_TDI)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_IRLEN)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_JTAG_IRLEN)->ShowWindow(SW_SHOW);
    }
#if DBGCM_DBG_DESCRIPTION
    CheckRadioButton(IDC_JTAG_AUTO, IDC_JTAG_MANUAL, ((MonConf.Opt & JTAG_MANUAL) && !PDSCDebug_IsEnabled()) ? IDC_JTAG_MANUAL : IDC_JTAG_AUTO);
#else  // DBGCM_DBG_DESCRIPTION
    CheckRadioButton(IDC_JTAG_AUTO, IDC_JTAG_MANUAL, (MonConf.Opt & JTAG_MANUAL) ? IDC_JTAG_MANUAL : IDC_JTAG_AUTO);
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        GetDlgItem(IDC_BOOT_RUN)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RST_VECT_CATCH)->EnableWindow(TRUE);
    } else
#endif           // DBGCM_DBG_DESCRIPTION
        if (1) { // TODO: Stop after Bootloader is supported
            GetDlgItem(IDC_BOOT_RUN)->ShowWindow(SW_SHOW);
        } else {
            GetDlgItem(IDC_BOOT_RUN)->ShowWindow(SW_HIDE);
        }

    if (MonConf.Opt & BOOT_RUN) {
        MonConf.Opt &= ~RST_VECT_CATCH;
    }

#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsEnabled()) {
        GetDlgItem(IDC_RST_VECT_CATCH)->EnableWindow((MonConf.Opt & BOOT_RUN) ? FALSE : TRUE);
    }
#else  // DBGCM_DBG_DESCRIPTION
    GetDlgItem(IDC_RST_VECT_CATCH)->EnableWindow((MonConf.Opt & BOOT_RUN) ? FALSE : TRUE);
#endif // DBGCM_DBG_DESCRIPTION
    CheckDlgButton(IDC_RST_VECT_CATCH, (MonConf.Opt & RST_VECT_CATCH) ? BST_CHECKED : BST_UNCHECKED);

#if DBGCM_DBG_DESCRIPTION
    if (status || (SetupMode == 0) || PDSCDebug_IsEnabled()) {
#else  // DBGCM_DBG_DESCRIPTION
    if (status || (SetupMode == 0)) {
#endif // DBGCM_DBG_DESCRIPTION
        ShowControl = FALSE;
    } else {
        ShowControl = (MonConf.Opt & JTAG_MANUAL) ? TRUE : FALSE;
    }

    GetDlgItem(IDC_JTAG_AUTO)->EnableWindow(ShowControl);
    GetDlgItem(IDC_JTAG_MANUAL)->EnableWindow(ShowControl);

    for (i = 0; i < (sizeof(jtagmanctrls) / sizeof(jtagmanctrls[0])); i++) {
        // enable/disable all contols for manual/automatic JTAG chain setup
        GetDlgItem(jtagmanctrls[i])->EnableWindow(ShowControl);
    }
    for (i = 0; i < (sizeof(jtagmanctrlh) / sizeof(jtagmanctrlh[0])); i++) {
        // enable all contols for automatic JTAG chain detection
        GetDlgItem(jtagmanctrlh[i])->EnableWindow(FALSE);
    }
    SetDlgItemText(IDC_JTAG_ID, "");
    SetDlgItemText(IDC_JTAG_DEVNAME, "");
    SetDlgItemText(IDC_JTAG_IRLEN, "");
    SetDlgItemText(IDC_CONFIG_AP, "");

    pLC = (CListCtrl *)GetDlgItem(IDC_CONFIG_DEVICE);
    pLC->SetExtendedStyle(LVS_EX_FULLROWSELECT);
    pLC->DeleteColumn(2); // delete previous defined columns
    pLC->DeleteColumn(1);
    pLC->DeleteColumn(0);
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;

    if (status) { // Check if device is connected
        pchar = StatusText(status);
        //  insert columns
        col.pszText = "         Error"; //(LPSTR) pchar;      // Value Column
        col.fmt     = LVCFMT_LEFT;      // align columns
        col.cx      = pLC->GetStringWidth(pchar) + 17 + 16;
        pLC->InsertColumn(0, &col);

        pLC->DeleteAllItems();
        memset(&item, 0, sizeof(item));
        item.mask     = LVIF_TEXT | LVIF_IMAGE;
        item.iSubItem = 0;
        item.pszText  = pchar;
        item.iItem    = 0;
        item.iImage   = 2; // don't show radio button
        pLC->InsertItem((const LPLVITEM)&item);

        GetDlgItem(IDC_CONFIG_DEVICE)->EnableWindow(FALSE); // disable all device fields
        GetDlgItem(IDC_CONFIG_AP)->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_AP)->EnableWindow(FALSE);

    } else {
#if DBGCM_DBG_DESCRIPTION
        if ((SetupMode == 0) || (MonConf.Opt & PORT_SW) || PDSCDebug_IsEnabled()) {
            GetDlgItem(IDC_CONFIG_DEVICE)->EnableWindow(FALSE); // disable all device fields
        } else {
            GetDlgItem(IDC_CONFIG_DEVICE)->EnableWindow(TRUE); // enable all device fields
        }
        GetDlgItem(IDC_CONFIG_AP)->EnableWindow(SetupMode && !PDSCDebug_IsEnabled() ? TRUE : FALSE);
        GetDlgItem(IDC_STATIC_AP)->EnableWindow(SetupMode && !PDSCDebug_IsEnabled() ? TRUE : FALSE);
#else  // DBGCM_DBG_DESCRIPTION
        if ((SetupMode == 0) || (MonConf.Opt & PORT_SW)) {
            GetDlgItem(IDC_CONFIG_DEVICE)->EnableWindow(FALSE); // disable all device fields
        } else {
            GetDlgItem(IDC_CONFIG_DEVICE)->EnableWindow(TRUE); // enable all device fields
        }
        GetDlgItem(IDC_CONFIG_AP)->EnableWindow(SetupMode ? TRUE : FALSE);
        GetDlgItem(IDC_STATIC_AP)->EnableWindow(SetupMode ? TRUE : FALSE);
#endif // DBGCM_DBG_DESCRIPTION

        sprintf(tbuf, "0x%02X", MonConf.AP);
        SetDlgItemText(IDC_CONFIG_AP, tbuf);

        if (MonConf.Opt & PORT_SW) {
            k = 2;
        } else {
            k = 3;
        }

        // insert columns
        for (i = 0; i < k; i++) {
            //    col.iSubItem = i;
            col.pszText = (LPSTR)jtagdlgheader[i]; // Value Column
            col.fmt     = jtagdlgfmt[i];           // align columns
            col.cx      = pLC->GetStringWidth(jtagdlgheader[i]);
            switch (i) {
                case 0:
                    tmp = pLC->GetStringWidth("0xDDDDDDDD"); // JTAG ID: 'D' is the widest character of hex numbers
                    if (tmp > col.cx)
                        col.cx = tmp;
                    break;

                case 1:
                    for (j = 0; j < JTAG_devs.cnt; j++) { // Device Name: go through all devices in JTAG chain and look for the longes name
                        tmp = pLC->GetStringWidth(JTAG_devs.icname[j]);
                        if (tmp > col.cx)
                            col.cx = tmp;
                    }
                    break;

                case 2:
                    tmp = pLC->GetStringWidth("20"); // IR len: two decimal digits are enough for this number
                    if (tmp > col.cx)
                        col.cx = tmp;
                    break;
            }
            col.cx += 17;
            pLC->InsertColumn(i, &col);
        }

        pLC->DeleteAllItems();
        memset(&item, 0, sizeof(item));
        memset(&subitem, 0, sizeof(item));
        item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        subitem.mask  = LVIF_TEXT;
        item.iSubItem = 0;
        item.pszText  = tbuf;
        for (i = 0, itemindex = 0; i < JTAG_devs.cnt; i++, itemindex++) { // go through all devices in JTAG chain
            // insert items
            item.iItem  = itemindex;
            item.lParam = (JTAG_devs.icinfo[i] << 16) | i;
            sprintf(tbuf, "0x%08X", JTAG_devs.ic[i].id);
            if (JTAG_devs.icinfo[i] != 0) {
                if (i == MonConf.JtagCpuIndex) {
                    item.iImage = 1; // show selected radio button
                } else {
                    item.iImage = 0; // show deselected radio button
                }
            } else {
                item.iImage = 2; // don't show radio button
            }

            pLC->InsertItem((const LPLVITEM)&item); // insert line and item 0 with icon

            for (j = 1; j < k; j++) {
                subitem.iSubItem = j;
                subitem.iItem    = itemindex;
                switch (j) {
                    case 1:
                        subitem.pszText = JTAG_devs.icname[i];
                        break;
                    case 2:
                        subitem.pszText = tbuf;
                        sprintf(tbuf, "%d", JTAG_devs.ic[i].ir_len);
                        break;
                }
                // set device name.
                pLC->SetItem(&subitem);
            }
        }
    }
}


BOOL CSetupDbg::OnInitDialog()
{
    CListCtrl *pLC;
    CWinApp   *pApp;

    CPropertyPage::OnInitDialog();

    ToolTip.Create(this);

    TOOLINFO ti;
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd   = GetSafeHwnd();
    ti.hinst  = AfxGetInstanceHandle();

    ti.uId      = (UINT)GetDlgItem(IDC_CONFIG_SWJ)->GetSafeHwnd();
    ti.lpszText = "Use SWJ Switching";
    ToolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);

    ToolTip.Activate(TRUE);

    //--- Initialize CheckBox controls:
    CheckDlgButton(IDC_CACHE_CODE, (MonConf.Opt & CACHE_CODE) ? 1 : 0);
    CheckDlgButton(IDC_CACHE_MEM, (MonConf.Opt & CACHE_MEM) ? 1 : 0);
    CheckDlgButton(IDC_CODE_VERIFY, (MonConf.Opt & CODE_VERIFY) ? 1 : 0);
    CheckDlgButton(IDC_FLASH_LOAD, (MonConf.Opt & FLASH_LOAD) ? 1 : 0);
    CheckDlgButton(IDC_BOOT_RESET, (MonConf.Opt & BOOT_RESET) ? 1 : 0);
    CheckDlgButton(IDC_BOOT_RUN, (MonConf.Opt & BOOT_RUN) ? 1 : 0);
    CheckDlgButton(IDC_RST_VECT_CATCH, (MonConf.Opt & RST_VECT_CATCH) ? 1 : 0);

    //GetDlgItem(IDC_BOOT_RUN)->ShowWindow(SW_SHOW);

    //--- Initialize ComboBox controls:
#if DBGCM_V8M
    if (IsV8M()) {
        ((CComboBox *)GetDlgItem(IDC_RST_TYPE))->DeleteString(3);
    }
    ((CComboBox *)GetDlgItem(IDC_RST_TYPE))->SetCurSel((MonConf.Opt >> 8) & 0x03);
#else  // DBGCM_V8M
    ((CComboBox *)GetDlgItem(IDC_RST_TYPE))->SetCurSel((MonConf.Opt >> 8) & 0x03);
#endif // DBGCM_V8M

#if DBGCM_WITHOUT_STOP
    if (MonConf.Opt & CONN_NO_STOP) {
        ((CComboBox *)GetDlgItem(IDC_INIT_RST))->SetCurSel(3);
    } else {
        ((CComboBox *)GetDlgItem(IDC_INIT_RST))->SetCurSel((MonConf.Opt >> 10) & 0x03);
    }
#else  // DBGCM_WITHOUT_STOP
    ((CComboBox *)GetDlgItem(IDC_INIT_RST))->DeleteString(3);
    ((CComboBox *)GetDlgItem(IDC_INIT_RST))->SetCurSel((MonConf.Opt >> 10) & 0x03);
#endif // DBGCM_WITHOUT_STOP

    // fill in image lists
    m_pImageListState = new CImageList();
    m_pImageListState->Create(16, 16, TRUE, 3, 3);
    pApp = AfxGetApp();
    m_pImageListState->Add(pApp->LoadIcon(IDI_ICONLIST1));
    m_pImageListState->Add(pApp->LoadIcon(IDI_ICONLIST2));
    m_pImageListState->Add(pApp->LoadIcon(IDI_ICONLIST3));

    pLC = (CListCtrl *)GetDlgItem(IDC_CONFIG_DEVICE);
    pLC->SetImageList(m_pImageListState, LVSIL_SMALL);

    //Update();  // Executed in OnSetActive

#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsInitialized()) {
        PDSCDebug_Init();
    }
    if (!PDSCDebug_HasDebugPropertiesBackup()) {
        PDSCDebug_CreateDebugPropertiesBackup();
    }
#endif // DBGCM_DBG_DESCRIPTION
    return (TRUE);
}


BOOL CSetupDbg::OnSetActive()
{
    Update();
    return CPropertyPage::OnSetActive();
}


void CSetupDbg::OnSelchangeConfigUnit()
{
    CComboBox *pC;
    int        i;

    pC = (CComboBox *)GetDlgItem(IDC_CONFIG_UNIT);
    i  = pC->GetCurSel();

    k_if_selected = i;
    //Select Unit
    if (!i) { // Select "Any" (first) Unit
        bAnyUnit = true;
        pC->SetCurSel(i);
        // active unit = 0
    } else {
        bAnyUnit = false;
        pC->SetCurSel(i);
        // active unit = i - 1
    }
    Update();
}


void CSetupDbg::OnSelchangeConfigPort()
{
    CComboBox *pC;
    int        i;

    pC = (CComboBox *)GetDlgItem(IDC_CONFIG_PORT);
    i  = pC->GetCurSel();
    if (i) {
        MonConf.Opt |= PORT_SW;
    } else {
        MonConf.Opt &= ~PORT_SW;
    }
    Update();
}


void CSetupDbg::OnSelchangeConfigClk()
{
    CComboBox *pC;

    pC                = (CComboBox *)GetDlgItem(IDC_CONFIG_CLK);
    MonConf.SWJ_Clock = pC->GetItemData(pC->GetCurSel());
    Update();
}


void CSetupDbg::OnConfigSwj()
{
    if (IsDlgButtonChecked(IDC_CONFIG_SWJ)) {
        MonConf.Opt |= USE_SWJ;
    } else {
        MonConf.Opt &= ~USE_SWJ;
    }
    Update();
}


void CSetupDbg::OnCacheCode()
{
    MonConf.Opt &= ~CACHE_CODE;
    if (IsDlgButtonChecked(IDC_CACHE_CODE)) {
        MonConf.Opt |= CACHE_CODE;
    }
}

void CSetupDbg::OnCacheMem()
{
    MonConf.Opt &= ~CACHE_MEM;
    if (IsDlgButtonChecked(IDC_CACHE_MEM)) {
        MonConf.Opt |= CACHE_MEM;
    }
}

void CSetupDbg::OnCodeVerify()
{
    MonConf.Opt &= ~CODE_VERIFY;
    if (IsDlgButtonChecked(IDC_CODE_VERIFY)) {
        MonConf.Opt |= CODE_VERIFY;
    }
}

void CSetupDbg::OnFlashLoad()
{
    MonConf.Opt &= ~FLASH_LOAD;
    if (IsDlgButtonChecked(IDC_FLASH_LOAD)) {
        MonConf.Opt |= FLASH_LOAD;
    }
}

void CSetupDbg::OnBootReset()
{
    MonConf.Opt &= ~BOOT_RESET;
    if (IsDlgButtonChecked(IDC_BOOT_RESET)) {
        MonConf.Opt |= BOOT_RESET;
    }
}

void CSetupDbg::OnBootRun()
{
    MonConf.Opt &= ~BOOT_RUN;
    if (IsDlgButtonChecked(IDC_BOOT_RUN)) {
        MonConf.Opt |= BOOT_RUN;
        MonConf.Opt &= ~RST_VECT_CATCH; // Disallow "Stop after Reset" with "Stop after Bootloader"
    }
    Update(); // Update "Stop after Reset"
}

void CSetupDbg::OnSelchangeRstType()
{
    int i;

    i = ((CComboBox *)GetDlgItem(IDC_RST_TYPE))->GetCurSel();
    MonConf.Opt &= ~(0x03 << 8);
    MonConf.Opt |= i << 8;
}

void CSetupDbg::OnSelchangeInitRst()
{
    int i;

    i = ((CComboBox *)GetDlgItem(IDC_INIT_RST))->GetCurSel();
#if DBGCM_WITHOUT_STOP
    MonConf.Opt &= ~(CONN_NO_STOP | (0x03 << 10));
    if (i == 3) {
        MonConf.Opt |= CONN_NO_STOP;
    } else {
        MonConf.Opt |= i << 10;
    }
#else  // DBGCM_WITHOUT_STOP
    MonConf.Opt &= ~(0x03 << 10);
    MonConf.Opt |= i << 10;
#endif // DBGCM_WITHOUT_STOP
}

void CSetupDbg::OnRstVectCatch()
{
    MonConf.Opt &= ~RST_VECT_CATCH;
    if (IsDlgButtonChecked(IDC_RST_VECT_CATCH)) {
        MonConf.Opt |= RST_VECT_CATCH;
    }
}

void CSetupDbg::OnKillfocusConfigAP()
{
    BYTE val;

    if (GetDlgHex2(GetDlgItem(IDC_CONFIG_AP), MonConf.AP, &val)) {
        MonConf.AP = val;
    }
}


void CSetupDbg::OnJtagAuto()
{
    MonConf.Opt &= ~JTAG_MANUAL;
    Update(); // when this changes, some controls need to be updated in this dialog
}

void CSetupDbg::OnJtagManual()
{
    MonConf.Opt |= JTAG_MANUAL;
    Update(); // when this changes, some controls need to be updated in this dialog
}


void CSetupDbg::OnItemchangedConfigDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
    CListCtrl   *pLC;
    LVITEM       item;
    int          i, num;
    char         tbuf[20];

    if (pNMListView->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)) {
        //  intoldsel = pNMListView->iItem;
        pLC = (CListCtrl *)GetDlgItem(IDC_CONFIG_DEVICE);
        memset(&item, 0, sizeof(item));
        item.mask  = LVIF_PARAM;
        item.iItem = pNMListView->iItem;
        pLC->GetItem(&item);                                 // retrieve lParam value from selected item
        if ((item.lParam >> 16) != 0) {                      // is this device supported by this driver?
            MonConf.JtagCpuIndex = item.lParam & 0x00007FFF; // update device selections for MCU
            num                  = pLC->GetItemCount();      // go through all items and replace the icon
            memset(&item, 0, sizeof(item));
            item.mask = LVIF_IMAGE;
            for (i = 0; i < num; i++) {
                item.iItem = i;
                pLC->GetItem(&item); // retrieve iImage value from selected item
                if (item.iImage == 2)
                    continue; // device has an empty icon (do not change)
                if (item.iItem == pNMListView->iItem) {
                    item.iImage = 1; // change to selected icon (radio button)
                } else {
                    item.iImage = 0; // change to non-selected icon (radio button)
                }
                pLC->SetItem(&item); // set new icon
            }
        }
        if (MonConf.Opt & JTAG_MANUAL) { // copy selected device into edit controls when in manual mode
            sprintf(tbuf, "0x%08X", JTAG_devs.ic[pNMListView->iItem].id);
            SetDlgItemText(IDC_JTAG_ID, tbuf);
            SetDlgItemText(IDC_JTAG_DEVNAME, JTAG_devs.icname[pNMListView->iItem]);
            sprintf(tbuf, "%d", JTAG_devs.ic[pNMListView->iItem].ir_len);
            SetDlgItemText(IDC_JTAG_IRLEN, tbuf);
            SelectedJTAGItem = pNMListView->iItem;
            if (SelectedJTAGItem < 1) {
                GetDlgItem(IDC_JTAG_UP)->EnableWindow(FALSE); // disable UP button when the first item is selected
            } else {
                GetDlgItem(IDC_JTAG_UP)->EnableWindow(TRUE); // enable UP button when not the first item is selected
            }
            if (SelectedJTAGItem > ((int)JTAG_devs.cnt - 2)) {
                GetDlgItem(IDC_JTAG_DOWN)->EnableWindow(FALSE); // disable DOWN button when the last item is selected
            } else {
                GetDlgItem(IDC_JTAG_DOWN)->EnableWindow(TRUE); // enable DOWN button when not the last item is selected
            }
            GetDlgItem(IDC_JTAG_UPDATE)->EnableWindow(TRUE); // enable UPDATE button when not the last item is selected
            GetDlgItem(IDC_JTAG_DELETE)->EnableWindow(TRUE); // enable DELETE button when not the last item is selected
        }
    }
    *pResult = 0;
}


void CSetupDbg::OnJtagUpdate()
{
    char  idbuf[20], devbuf[30];
    int   idlen, idval, devlen;
    BOOL  lentrans;
    DWORD irlen;

    if (SelectedJTAGItem == -1)
        return;
    idlen = GetDlgItemText(IDC_JTAG_ID, idbuf, sizeof(idbuf));
    idval = 0;
    sscanf(idbuf, "%x", &idval);
    devlen   = GetDlgItemText(IDC_JTAG_DEVNAME, devbuf, sizeof(devbuf));
    lentrans = 0;
    irlen    = GetDlgItemInt(IDC_JTAG_IRLEN, &lentrans, FALSE);

    if (idlen && idval && devlen && lentrans && (irlen < 20)) { // all new entries seem to be valid
        JTAG_devs.ic[SelectedJTAGItem].id = idval;
        strcpy(JTAG_devs.icname[SelectedJTAGItem], devbuf);
        JTAG_devs.ic[SelectedJTAGItem].ir_len = (BYTE)irlen;
        Update(); // when this changes, some controls need to be updated in this dialog
    } else {
        AGDIMsgBox(m_hWnd, &INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
    }
}

void CSetupDbg::OnJtagAdd()
{
    char  idbuf[20], devbuf[30];
    int   idlen, idval, devlen;
    BOOL  lentrans;
    DWORD irlen;

    idlen = GetDlgItemText(IDC_JTAG_ID, idbuf, sizeof(idbuf));
    idval = 0;
    sscanf(idbuf, "%x", &idval);
    devlen   = GetDlgItemText(IDC_JTAG_DEVNAME, devbuf, sizeof(devbuf));
    lentrans = 0;
    irlen    = GetDlgItemInt(IDC_JTAG_IRLEN, &lentrans, FALSE);

    if (idlen && idval && devlen && lentrans && (irlen < 20) && (JTAG_devs.cnt < NJDEVS)) { // all new entries seem to be valid
        JTAG_devs.ic[JTAG_devs.cnt].id = idval;
        strcpy(JTAG_devs.icname[JTAG_devs.cnt], devbuf);
        JTAG_devs.ic[JTAG_devs.cnt].ir_len = (BYTE)irlen;
        JTAG_devs.cnt++;
        Update(); // when this changes, some controls need to be updated in this dialog
    } else {
        AGDIMsgBox(m_hWnd, &INPUT_ERRMSG[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
    }
}

void CSetupDbg::OnJtagDelete()
{
    int i;

    if (SelectedJTAGItem < 0)
        return;

    for (i = SelectedJTAGItem; i < ((int)JTAG_devs.cnt - 1); i++) { // move all following entries down
        JTAG_devs.ic[i] = JTAG_devs.ic[i + 1];
        strcpy(JTAG_devs.icname[i], JTAG_devs.icname[i + 1]);
        JTAG_devs.icinfo[i] = JTAG_devs.icinfo[i + 1];
    }
    JTAG_devs.cnt--;

    Update(); // when this changes, some controls need to be updated in this dialog
}


void CSetupDbg::OnJtagUp()
{
    char ticname[30];
    int  ticinfo;
    J_IC tic;

    if (SelectedJTAGItem < 1)
        return;

    // save entry to temporary buffers
    tic = JTAG_devs.ic[SelectedJTAGItem];
    strcpy(ticname, JTAG_devs.icname[SelectedJTAGItem]);
    ticinfo = JTAG_devs.icinfo[SelectedJTAGItem];

    // move upper entry down
    JTAG_devs.ic[SelectedJTAGItem] = JTAG_devs.ic[SelectedJTAGItem - 1];
    strcpy(JTAG_devs.icname[SelectedJTAGItem], JTAG_devs.icname[SelectedJTAGItem - 1]);
    JTAG_devs.icinfo[SelectedJTAGItem] = JTAG_devs.icinfo[SelectedJTAGItem - 1];

    // insert temporary buffers into upper entry
    JTAG_devs.ic[SelectedJTAGItem - 1] = tic;
    strcpy(JTAG_devs.icname[SelectedJTAGItem - 1], ticname);
    JTAG_devs.icinfo[SelectedJTAGItem - 1] = ticinfo;

    Update(); // when this changes, some controls need to be updated in this dialog
}

void CSetupDbg::OnJtagDown()
{
    char ticname[30];
    int  ticinfo;
    J_IC tic;

    if ((SelectedJTAGItem < 0) || (SelectedJTAGItem >= ((int)JTAG_devs.cnt - 1)))
        return;

    // save entry to temporary buffers
    tic = JTAG_devs.ic[SelectedJTAGItem];
    strcpy(ticname, JTAG_devs.icname[SelectedJTAGItem]);
    ticinfo = JTAG_devs.icinfo[SelectedJTAGItem];

    // move upper entry down
    JTAG_devs.ic[SelectedJTAGItem] = JTAG_devs.ic[SelectedJTAGItem + 1];
    strcpy(JTAG_devs.icname[SelectedJTAGItem], JTAG_devs.icname[SelectedJTAGItem + 1]);
    JTAG_devs.icinfo[SelectedJTAGItem] = JTAG_devs.icinfo[SelectedJTAGItem + 1];

    // insert temporary buffers into upper entry
    JTAG_devs.ic[SelectedJTAGItem + 1] = tic;
    strcpy(JTAG_devs.icname[SelectedJTAGItem + 1], ticname);
    JTAG_devs.icinfo[SelectedJTAGItem + 1] = ticinfo;

    Update(); // when this changes, some controls need to be updated in this dialog
}


#if DBGCM_DBG_DESCRIPTION
int CSetupDbg::PdscTargetConnect(void)
{
    int status = 0;

    PDSCDebug_DebugContext = DBGCON_CONNECT;

    status = PDSCDebug_InitDriver();

    if (!status) {
        status = PDSCDebug_InitDebugger();
    }

    // Ensure we do not bypass any TAP in the JTAG chain
    nCPU = 0;
    PDSCDebug_SetActiveDP(0); // Init this (only in SetupMode), otherwise the PDSCDebug_DebugPortSetup() won't work

    if ((MonConf.Opt & JTAG_MANUAL) == 0) { // don't update the device list when in automatic mode
        if (MonConf.Opt & INIT_RST_PULSE) {
            if (!status) {
                status = PDSCDebug_ResetHardware(1 /*bPreReset*/); // HW Chip Reset
            }
        }
        if (MonConf.Opt & INIT_RST_HOLD) {
            if (!status) {
                status = PDSCDebug_ResetHardwareAssert();
            }
        }

        if (!status) {
            status = PDSCDebug_DebugPortSetup();
        }
        if (!status) {
            status = PDSCDebug_DebugGetDeviceList(&JTAG_devs, NJDEVS, true);
        }
        if (MonConf.Opt & INIT_RST_HOLD) {
            if (!status) {
                status = PDSCDebug_ResetHardwareDeassert();
            }
        }
        if ((status == EU04) || (status == EU03) || // Ignore the error message EU03, EU04
            (status == EU10)) {                     // and EU10
            status        = 0;
            JTAG_devs.cnt = 0;
        }
    }
    if (!status) {
        status = PDSCDebug_DebugGetDeviceNames(&JTAG_devs, NJDEVS, true /*merge*/);
    }
    PDSCDebug_UnInitDebugger();

    return (status);
}
#endif // DBGCM_DBG_DESCRIPTION


void InitSetupDbg()
{
    SelectedJTAGItem = 0;
}
