/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.2
 * @date     $Date: 2017-09-20 19:31:21 +0200 (Wed, 20 Sep 2017) $
 *
 * @note
 * Copyright (C) 2009-2015 ARM Limited. All rights reserved.
 *
 * @brief     Flash Download Setup Dialog
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
#include "Flash.h"
#include "Setup.h"
#include "SetupFD.h"
#include "..\BOM.h"   // 7.11.2012
#include "..\ALLOC.H" // 7.11.2012, wg. ENV_PRJ


/******************************************************************************/

static const char INPUT_ERR_TITLE[] = "Invalid number";
static const char INPUT_ERRZERO[] =
    "Number must not be zero !\n"
    "The previous value will be restored.\n";
static const char INPUT_ERRALGN[] =
    "Number must be DWORD aligned !\n"
    "The previous value will be restored.\n";

static void SizeToString(char *str, DWORD size)
{
    if ((size >= (1 << 30)) && ((size & ((1 << 30) - 1)) == 0)) {
        sprintf(str, "%uG", size >> 30);
        return;
    }
    if ((size >= (1 << 20)) && ((size & ((1 << 20) - 1)) == 0)) {
        sprintf(str, "%uM", size >> 20);
        return;
    }
    if ((size >= (1 << 10)) && ((size & ((1 << 10) - 1)) == 0)) {
        sprintf(str, "%uk", size >> 10);
        return;
    }
    sprintf(str, "%uB", size); // 12.4.2013
}

/******************************************************************************/


// CSetupFD dialog

IMPLEMENT_DYNCREATE(CSetupFD, CPropertyPage)

CSetupFD::CSetupFD()
    : CPropertyPage(CSetupFD::IDD)
{
}

CSetupFD::~CSetupFD()
{
}

BEGIN_MESSAGE_MAP(CSetupFD, CPropertyPage)
ON_BN_CLICKED(IDC_FLASH_ERASEALL, OnFlashErase)
ON_BN_CLICKED(IDC_FLASH_ERASESECT, OnFlashErase)
ON_BN_CLICKED(IDC_FLASH_ERASENONE, OnFlashErase)
ON_BN_CLICKED(IDC_FLASH_PROGRAM, OnFlashProgram)
ON_BN_CLICKED(IDC_FLASH_VERIFY, OnFlashVerify)
ON_BN_CLICKED(IDC_FLASH_RESETRUN, OnFlashResetRun)
ON_EN_KILLFOCUS(IDC_FLASH_START, OnKillfocusFlashStart)
ON_EN_KILLFOCUS(IDC_FLASH_SIZE, OnKillfocusFlashSize)
ON_EN_KILLFOCUS(IDC_FLASH_RAMSTART, OnKillfocusRAMStart)
ON_EN_KILLFOCUS(IDC_FLASH_RAMSIZE, OnKillfocusRAMSize)
ON_NOTIFY(LVN_ITEMCHANGING, IDC_FLASH_ALGLIST, OnItemchangingAlgList)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_FLASH_ALGLIST, OnItemchangedAlgList)
ON_BN_CLICKED(ID_ADD, OnFlashAdd)
ON_BN_CLICKED(ID_REMOVE, OnFlashRemove)
END_MESSAGE_MAP()


// CSetupFD message handlers

//static const char *algdlgheader[] = { "Description", "Device Type", "Device Size", "Address Range" };
static const char *algdlgheader[] = { "Description", "Device Size", "Device Type", "Address Range" }; // 18.4.2013
static const int   algdlgfmt[]    = { LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER };


static int   SelAlg;   // Selected Flash Algorithm
static int   SelFile;  // Selected Flash File
static char *pSelFile; // selected file, full path
static BYTE  bRteAlgo; // 1:= selected algo via RTE

/*
 * Add an Item to the Flash List
 */

void CSetupFD::AddItem(int index, BOOL select)
{
    CListCtrl *pLC;
    LVITEM     item;
    char       buf[512];

    pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);

    memset(&item, 0, sizeof(item));
    item.mask  = LVIF_TEXT | LVIF_PARAM;
    item.iItem = index;

    item.iSubItem = 0;
    item.pszText  = FlashConf.Dev[index].DevName;
    item.lParam   = index;
    pLC->InsertItem((const LPLVITEM)&item);
    item.mask = LVIF_TEXT;

    switch (FlashConf.Dev[index].DevType) {
        case UNKNOWN: strcpy(buf, "Unknown Device"); break;
        case ONCHIP: strcpy(buf, "On-chip Flash"); break;
        case EXT8BIT: strcpy(buf, "Ext. Flash 8-bit"); break;
        case EXT16BIT: strcpy(buf, "Ext. Flash 16-bit"); break;
        case EXT32BIT: strcpy(buf, "Ext. Flash 32-bit"); break;
        case EXTSPI: strcpy(buf, "Ext. Flash SPI"); break;
        default: strcpy(buf, "Invalid Device");
    }
    item.iSubItem = 2; // 1;
    item.pszText  = buf;
    pLC->SetItem(&item);

    SizeToString(buf, FlashDev.szDev);
    item.iSubItem = 1; // 2;
    item.pszText  = buf;
    pLC->SetItem(&item);

    sprintf(buf, "%08XH - %08XH",
            FlashConf.Dev[index].Start,
            FlashConf.Dev[index].Start + FlashConf.Dev[index].Size - 1);
    item.iSubItem = 3;
    item.pszText  = buf;
    pLC->SetItem(&item);

    if (select) {
        // Select the New Item
        item.mask      = LVIF_STATE;
        item.iSubItem  = 0;
        item.state     = LVIS_SELECTED | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        pLC->SetItem(&item);
    }
}


/*
 * Update an Item in the Flash List
 */

void CSetupFD::UpdateItem(int index)
{
    CListCtrl *pLC;
    LVITEM     item;
    char       buf[512];

    pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);

    memset(&item, 0, sizeof(item));
    item.mask  = LVIF_TEXT;
    item.iItem = index;

    sprintf(buf, "%08XH - %08XH",
            FlashConf.Dev[SelAlg].Start,
            FlashConf.Dev[SelAlg].Start + FlashConf.Dev[SelAlg].Size - 1);
    item.iSubItem = 3;
    item.pszText  = buf;
    pLC->SetItem(&item);
}


BOOL CSetupFD::OnInitDialog()
{
    LVCOLUMN   col;
    CListCtrl *pLC;
    char       buf[512], szCP[MAX_PATH + 32];
    int        i, j;

    CPropertyPage::OnInitDialog();

    pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);
    pLC->SetExtendedStyle(LVS_EX_FULLROWSELECT);

    // insert columns
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    for (i = 0; i < 4; i++) {
        col.iSubItem = i;
        col.pszText  = (LPSTR)algdlgheader[i]; // Value Column
        col.fmt      = algdlgfmt[i];           // align columns
        col.cx       = pLC->GetStringWidth(algdlgheader[i]);
        switch (i) {
            case 0: col.cx += 100; break;
            case 1: col.cx += 24; break; // col.cx += 36  /18.4.2013/
            case 2: col.cx += 36; break; // col.cx += 24
            case 3: col.cx += 96; break;
        }
        pLC->InsertColumn(i, &col);
    }

    if (FlashConf.Opt & FLASH_ERASE) {
        if (FlashConf.Opt & FLASH_ERASEALL) {
            CheckDlgButton(IDC_FLASH_ERASEALL, 1);
        } else {
            CheckDlgButton(IDC_FLASH_ERASESECT, 1);
        }
    } else {
        CheckDlgButton(IDC_FLASH_ERASENONE, 1);
    }

    CheckDlgButton(IDC_FLASH_PROGRAM, (FlashConf.Opt & FLASH_PROGRAM) ? 1 : 0);
    CheckDlgButton(IDC_FLASH_VERIFY, (FlashConf.Opt & FLASH_VERIFY) ? 1 : 0);
    CheckDlgButton(IDC_FLASH_RESETRUN, (FlashConf.Opt & FLASH_RESETRUN) ? 1 : 0);

    StringHex8(GetDlgItem(IDC_FLASH_RAMSTART), FlashConf.RAMStart);
    //StringHex4(GetDlgItem(IDC_FLASH_RAMSIZE),  FlashConf.RAMSize);
    StringHex8(GetDlgItem(IDC_FLASH_RAMSIZE), FlashConf.RAMSize); // 11.9.2017

    RTEFLASH *pRF;                       // 7.11.2012
    pRF = (RTEFLASH *)pio->SXX_RTEFLASH; // exposed Rte-Flash Info

    // insert items
    FlashConf.Nitems = 0;
    for (i = 0; i < NFlash; i++) {
        if (FlashConf.Dev[i].FileName[0] == 0)
            break;

        if (pRF == NULL || pRF->nItems == 0) { // no Rte-Flash algos are exposed  /7.11.2012/
            FlashConf.Dev[i].fPath[0] = 0;     // clear .flm RTE package path
        }
        if (FlashConf.Dev[i].fPath[0]) {
            strcpy(buf, FlashConf.Dev[i].fPath); // use absolute RTE-package path
        } else {
            strcpy(szCP, MonConf.DriverPath);
            strcat(szCP, "..\\flash\\");
            strcat(szCP, FlashConf.Dev[i].FileName);
            strcat(szCP, ".flm");
            PathCanonicalize(buf, szCP);
        } // -------------------------------------------

        if (LoadFlashDevice(buf)) {
            strcpy(&FlashConf.Dev[i].DevName[0], FlashDev.DevName);
            FlashConf.Dev[i].DevType = FlashDev.DevType;
            AddItem(i, FALSE);
            FlashConf.Nitems++;
        } else {
            for (j = i; j < (NFlash - 1); j++) {
                FlashConf.Dev[j] = FlashConf.Dev[j + 1];
            }
            memset(&FlashConf.Dev[j], 0, sizeof(FlashConf.Dev[0]));
            i--;
        }
    }

    GetDlgItem(ID_ADD)->EnableWindow((FlashConf.Nitems < NFlash) ? TRUE : FALSE);

    GetDlgItem(ID_REMOVE)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLASH_START)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLASH_SIZE)->EnableWindow(FALSE);

    return (TRUE);
}


void CSetupFD::OnItemchangingAlgList(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
    char         buf[512];

    if ((pNMListView->uNewState != LVIS_SELECTED) && (pNMListView->uOldState == LVIS_SELECTED)) {
        GetDlgItem(ID_REMOVE)->EnableWindow(FALSE);
        GetDlgItem(IDC_FLASH_START)->EnableWindow(FALSE);
        GetDlgItem(IDC_FLASH_SIZE)->EnableWindow(FALSE);
        strcpy(buf, "");
        SetDlgItemText(IDC_FLASH_START, buf);
        SetDlgItemText(IDC_FLASH_SIZE, buf);
    }
}

void CSetupFD::OnItemchangedAlgList(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
    CListCtrl *  pLC;
    LVITEM       item;

    if (pNMListView->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)) {
        pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);
        memset(&item, 0, sizeof(item));
        item.mask  = LVIF_PARAM;
        item.iItem = pNMListView->iItem;
        pLC->GetItem(&item);

        SelAlg = item.iItem;
        GetDlgItem(ID_REMOVE)->EnableWindow(TRUE);
        GetDlgItem(IDC_FLASH_START)->EnableWindow(TRUE);
        GetDlgItem(IDC_FLASH_SIZE)->EnableWindow(TRUE);
        StringHex8(GetDlgItem(IDC_FLASH_START), FlashConf.Dev[SelAlg].Start);
        StringHex8(GetDlgItem(IDC_FLASH_SIZE), FlashConf.Dev[SelAlg].Size);
    }
    *pResult = 0;
}


void CSetupFD::OnFlashErase()
{
    FlashConf.Opt &= ~(FLASH_ERASE | FLASH_ERASEALL);
    if (IsDlgButtonChecked(IDC_FLASH_ERASEALL)) {
        FlashConf.Opt |= FLASH_ERASE | FLASH_ERASEALL;
    }
    if (IsDlgButtonChecked(IDC_FLASH_ERASESECT)) {
        FlashConf.Opt |= FLASH_ERASE;
    }
}

void CSetupFD::OnFlashProgram()
{
    FlashConf.Opt &= ~FLASH_PROGRAM;
    if (IsDlgButtonChecked(IDC_FLASH_PROGRAM)) {
        FlashConf.Opt |= FLASH_PROGRAM;
    }
}

void CSetupFD::OnFlashVerify()
{
    FlashConf.Opt &= ~FLASH_VERIFY;
    if (IsDlgButtonChecked(IDC_FLASH_VERIFY)) {
        FlashConf.Opt |= FLASH_VERIFY;
    }
}

void CSetupFD::OnFlashResetRun()
{
    FlashConf.Opt &= ~FLASH_RESETRUN;
    if (IsDlgButtonChecked(IDC_FLASH_RESETRUN)) {
        FlashConf.Opt |= FLASH_RESETRUN;
    }
}


void CSetupFD::OnKillfocusFlashStart()
{
    DWORD v32;

    if (GetDlgHex8(GetDlgItem(IDC_FLASH_START), FlashConf.Dev[SelAlg].Start, &v32)) {
        FlashConf.Dev[SelAlg].Start = v32;
        UpdateItem(SelAlg);
    }
}

void CSetupFD::OnKillfocusFlashSize()
{
    DWORD v32;

    if (GetDlgHex8(GetDlgItem(IDC_FLASH_SIZE), FlashConf.Dev[SelAlg].Size, &v32)) {
        FlashConf.Dev[SelAlg].Size = v32;
        UpdateItem(SelAlg);
    }
}

void CSetupFD::OnKillfocusRAMStart()
{
    DWORD v32;
    CWnd *pCWnd;

    pCWnd = GetDlgItem(IDC_FLASH_RAMSTART);
    if (GetDlgHex8(pCWnd, FlashConf.RAMStart, &v32)) {
        if (v32 & 0x00000003) {
            //pCWnd->MessageBox(&INPUT_ERRALGN[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP);
            AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRALGN[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
            StringHex8(pCWnd, FlashConf.RAMStart);
            return;
        }
        FlashConf.RAMStart = v32;
    }
}

#if 0 // 11.9.2017 - orginal 16-Bit RamSize function
void CSetupFD::OnKillfocusRAMSize() {
  WORD  v16;
  CWnd *pCWnd;

  pCWnd = GetDlgItem(IDC_FLASH_RAMSIZE);
  if (GetDlgHex4(pCWnd, FlashConf.RAMSize, &v16)) {
    if (v16 == 0) {
      AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRALGN[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
      StringHex4(pCWnd, FlashConf.RAMSize);
      return;
    }
    if (v16 & 0x0003) {
      AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRALGN[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
      StringHex4(pCWnd, FlashConf.RAMSize);
      return;
    }
    FlashConf.RAMSize = v16;
  }
}

#else // 11.9.2017 - modified 32-Bit RamSize function
void CSetupFD::OnKillfocusRAMSize()
{
    DWORD v32;
    CWnd *pCWnd;

    pCWnd = GetDlgItem(IDC_FLASH_RAMSIZE);
    if (GetDlgHex8(pCWnd, FlashConf.RAMSize, &v32)) {
        if (v32 == 0) {
            AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRALGN[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
            StringHex8(pCWnd, FlashConf.RAMSize);
            return;
        }
        if (v32 & 0x0003) { // DWORD-alignment required since RamSize is used for further calculations...
            AGDIMsgBox(pCWnd->m_hWnd, &INPUT_ERRALGN[0], &INPUT_ERR_TITLE[0], MB_OK | MB_ICONSTOP, IDOK);
            StringHex8(pCWnd, FlashConf.RAMSize);
            return;
        }
        FlashConf.RAMSize = v32;
    }
}
#endif


void CSetupFD::OnFlashAdd()
{
    int    i;
    CAddFD dlg;

    i = dlg.DoModal();
    if (i == 1) {
        AddItem(FlashConf.Nitems, TRUE);
        FlashConf.Nitems++;
        if (FlashConf.Nitems == NFlash)
            GetDlgItem(ID_ADD)->EnableWindow(FALSE);
    }
}


void CSetupFD::OnFlashRemove()
{
    CListCtrl *pLC;
    char       buf[512];
    int        i;

    GetDlgItem(ID_REMOVE)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLASH_START)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLASH_SIZE)->EnableWindow(FALSE);

    strcpy(buf, "");
    SetDlgItemText(IDC_FLASH_START, buf);
    SetDlgItemText(IDC_FLASH_SIZE, buf);
    {
        int nItem = -1;

        pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);

        // Delete all the selected items.
        //...............................
        while ((nItem = pLC->GetNextItem(nItem, LVNI_ALL | LVNI_SELECTED)) > -1) {
            for (i = nItem; i < (FlashConf.Nitems - 1); i++) {
                FlashConf.Dev[i] = FlashConf.Dev[i + 1];
            }
            memset(&FlashConf.Dev[i], 0, sizeof(FlashConf.Dev[0]));
            FlashConf.Nitems--;

            pLC->DeleteItem(nItem);
        }
        // Make sure the icons are arranged to fill
        //  in the gaps left from the delete icons.
        //.........................................
        pLC->Arrange(LVA_DEFAULT);

        if (FlashConf.Nitems < NFlash)
            GetDlgItem(ID_ADD)->EnableWindow(TRUE);
    }
}


// CAddFD dialog

IMPLEMENT_DYNAMIC(CAddFD, CDialog)

CAddFD::CAddFD(CWnd *pParent /*=NULL*/)
    : CDialog(CAddFD::IDD, pParent)
{
    memset(algs, 0, sizeof(algs));
    nAlgs = 0;
}

CAddFD::~CAddFD()
{
}

BEGIN_MESSAGE_MAP(CAddFD, CDialog)
ON_NOTIFY(LVN_ITEMCHANGING, IDC_FLASH_ALGLIST, OnItemchangingAlgList)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_FLASH_ALGLIST, OnItemchangedAlgList)
END_MESSAGE_MAP()


// CAddFD message handlers

//---11.4.2013:
//static const char *algAddHdr[] = { "Description", "Device Type", "Flash Size", "Pack" };  // 12.4.2014
static const char *algAddHdr[] = { "Description", "Flash Size", "Device Type", "Origin" }; // 19.4.2014
static const int   algAddFmt[] = { LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_LEFT };

BOOL CAddFD::OnInitDialog()
{
    WIN32_FIND_DATA fd;
    HANDLE          fh;
    LVCOLUMN        col;
    LVITEM          item;
    CListCtrl *     pLC;
    char            buf[512], szCP[MAX_PATH + 32];
    int             i, y;
    TpPATHEXP       FpP;

    CDialog::OnInitDialog();

    //	CWnd* pHelpButton = GetDlgItem(ID_HELP);
    //	if (pHelpButton != NULL)
    //		pHelpButton->ShowWindow (SW_SHOW);

    GetDlgItem(IDOK)->EnableWindow(FALSE);

    pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);
    pLC->SetExtendedStyle(LVS_EX_FULLROWSELECT);

    // insert columns
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    //---11.4.2013:
    for (i = 0; i < 4; ++i) {
        col.iSubItem = i;
        col.pszText  = (LPSTR)algAddHdr[i]; // Value Column
        col.fmt      = algAddFmt[i];        // align columns
        col.cx       = pLC->GetStringWidth(algAddHdr[i]);
        switch (i) {
            case 0: col.cx += 100; break;
            case 1: col.cx += 24; break;  // col.cx += 36  /18.4.2013/
            case 2: col.cx += 36; break;  // col.cx += 24
            case 3: col.cx += 140; break; // col.cx += 170
        }
        pLC->InsertColumn(i, &col);
    }

    //---7.11.2012 - Rte-Flash-Algo Files
    FpP = (TpPATHEXP)pio->SXX_PATHEXPAND;
    memset(algs, 0, sizeof(algs));
    nAlgs    = 0;
    pSelFile = NULL;
    bRteAlgo = 0;
    i        = 0;
    RTEFLASH *pRF;
    pRF = (RTEFLASH *)pio->SXX_RTEFLASH; // exposed Rte-Flash Info
    if (pRF != NULL) {                   // Rte-Flash info is exposed
        y = 0;
        for (; y < pRF->nItems; ++y) {
            char *pFile = pRF->AlgoFiles[y]; // RTE Flash-Algo file, has absolute path
            if (!pFile || !pFile[0]) {       // bomb-proof
                continue;
            }
            if (!LoadFlashDevice(pFile)) {
                continue;
            }

            if (i < _NMAXALGS) {
                algs[i].AlgPath = pFile; // fullpath .flm file name
                algs[i].bRte    = 1;     // marker: 'algo from RTE package'
            }
            memset(&item, 0, sizeof(item));
            item.mask  = LVIF_TEXT;
            item.iItem = i++;

            item.iSubItem = 0;
            item.pszText  = FlashDev.DevName;
            pLC->InsertItem(&item);

            switch (FlashDev.DevType) {
                case UNKNOWN: item.pszText = "Unknown Device"; break;
                case ONCHIP: item.pszText = "On-chip Flash"; break;
                case EXT8BIT: item.pszText = "Ext. Flash 8-bit"; break;
                case EXT16BIT: item.pszText = "Ext. Flash 16-bit"; break;
                case EXT32BIT: item.pszText = "Ext. Flash 32-bit"; break;
                case EXTSPI: item.pszText = "Ext. Flash SPI"; break;
                default: item.pszText = "Invalid Device"; break;
            }
            item.iSubItem = 2; // 1;
            pLC->SetItem(&item);

            SizeToString(buf, FlashDev.szDev);
            item.iSubItem = 1; // 2;
            item.pszText  = buf;
            pLC->SetItem(&item);

            item.iSubItem = 3;
            item.pszText  = "Device Family Package"; // szRA;   /19.4.2013/
            pLC->SetItem(&item);
        }
    }

#if 0 //---11.4.2013:
  if (i > 0)  {                                  // RTE-flash algos are present
    if (i < _NMAXALGS)  {
      algs[i].AlgPath = NULL;                    // fullpath .flm file name
      algs[i].bRte    = 0;                       // marker: 'algo from RTE package'
    }

    memset (&item, 0, sizeof (item));
    item.mask     = LVIF_TEXT;
    item.iItem    = i++;
    item.pszText  = ">>> Generic-Flash-Algorithms <<<";
    pLC->InsertItem (&item);                         // label
  }
#endif

    //-----------------------------------

    // insert items
    //i = 0;
    strcpy(buf, MonConf.DriverPath);
    strcat(buf, "..\\flash\\");
    strcat(buf, "*.flm");
    fh = FindFirstFile(buf, &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        do {
            strcpy(buf, MonConf.DriverPath);
            strcat(buf, "..\\flash\\");
            strcat(buf, fd.cFileName);
            if (LoadFlashDevice(buf)) {
                //---7.11.2012:
                PathCanonicalize(szCP, buf);
                if (i < _NMAXALGS) {
                    algs[i].AlgPath = (char *)pio->SaveString(szCP, ENV_PRJ); // fullpath .flm file name;                   // fullpath .flm file name
                    algs[i].bRte    = 0;                                      // marker: 'algo not via RTE package'
                }
                memset(&item, 0, sizeof(item));
                item.mask  = LVIF_TEXT;
                item.iItem = i++;

                item.iSubItem = 0;
                item.pszText  = FlashDev.DevName;
                pLC->InsertItem(&item);

                switch (FlashDev.DevType) {
                    case UNKNOWN: strcpy(buf, "Unknown Device"); break;
                    case ONCHIP: strcpy(buf, "On-chip Flash"); break;
                    case EXT8BIT: strcpy(buf, "Ext. Flash 8-bit"); break;
                    case EXT16BIT: strcpy(buf, "Ext. Flash 16-bit"); break;
                    case EXT32BIT: strcpy(buf, "Ext. Flash 32-bit"); break;
                    case EXTSPI: strcpy(buf, "Ext. Flash SPI"); break;
                    default: strcpy(buf, "Invalid Device");
                }
                item.iSubItem = 2; // 1;
                item.pszText  = buf;
                pLC->SetItem(&item);

                SizeToString(buf, FlashDev.szDev);
                item.iSubItem = 1; // 2;
                item.pszText  = buf;
                pLC->SetItem(&item);

                item.iSubItem = 3; // 19.4.2013
                item.pszText  = "MDK Core";
                pLC->SetItem(&item);
            }
        } while (FindNextFile(fh, &fd));
        FindClose(fh);
    }

    nAlgs = i;

    return (TRUE);
}


void CAddFD::OnItemchangingAlgList(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;

    if ((pNMListView->uNewState != LVIS_SELECTED) && (pNMListView->uOldState == LVIS_SELECTED)) {
        GetDlgItem(IDOK)->EnableWindow(FALSE);
        pSelFile = NULL; // 7.11.2012
        bRteAlgo = 0;    // ---------
    }
}


void CAddFD::OnItemchangedAlgList(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
    CListCtrl *  pLC;
    LVITEM       item;
    char *       pFile, szFile[MAX_PATH + 32];
    TpPATHEXP    FpP;

    FpP = (TpPATHEXP)pio->SXX_PATHEXPAND; // 18.4.2013
    if (pNMListView->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)) {
        //---7.11.2012:
        pLC = (CListCtrl *)GetDlgItem(IDC_FLASH_ALGLIST);
        memset(&item, 0, sizeof(item));
        item.mask  = LVIF_PARAM | LVIF_IMAGE;
        item.iItem = pNMListView->iItem;
        pLC->GetItem(&item);

        pFile    = NULL;
        bRteAlgo = 0;
        if (item.iItem < _NMAXALGS) {
            pFile = algs[item.iItem].AlgPath; // fullpath .flm name or NULL
        }
        if (pFile != NULL) {
            pSelFile = pFile;                 // fullpath flm name
            bRteAlgo = algs[item.iItem].bRte; // 1:=from RTE, 0:=not from RTE
            GetDlgItem(IDOK)->EnableWindow(TRUE);
            // 18.4.2013
            if (FpP != NULL) {                    // path-expansion interface is present
                FpP(FPEXP_NORMAL, pFile, szFile); // nCode, pIn, pOut
            } else {
                strcpy(szFile, pFile);
            }
            ((CEdit *)GetDlgItem(IDC_ALGOFILE))->SetWindowText(szFile); // 18.4.2013
            *pResult = 0;
            return;
        }
        // not a flash-algo entry
        bRteAlgo = 0;    // clear
        pSelFile = NULL; // clear
        GetDlgItem(IDOK)->EnableWindow(FALSE);
        *pResult = 0;
        return;
//-------------
#if 0
    GetDlgItem (IDOK)->EnableWindow (TRUE);
    SelFile = pNMListView->iItem;
#endif
    }
    *pResult = 0;
}


void CAddFD::OnOK()
{
#if 1
    char buf[MAX_PATH + 32];
    int  j;

    if (pSelFile != NULL) { // 7.11.2012
        strcpy(buf, pSelFile);
        if (LoadFlashDevice(buf)) {
            PathStripPath(buf);       // IAP_512_xx.flm
            PathRemoveExtension(buf); // filename without path and extension
            j = FlashConf.Nitems;
            strcpy(&FlashConf.Dev[j].FileName[0], buf);
            strcpy(&FlashConf.Dev[j].DevName[0], FlashDev.DevName);
            if (bRteAlgo) {
                strcpy(&FlashConf.Dev[j].fPath[0], pSelFile); // full path of .flm file
            }
            FlashConf.Dev[j].DevType = FlashDev.DevType;
            FlashConf.Dev[j].Start   = FlashDev.DevAdr;
            FlashConf.Dev[j].Size    = FlashDev.szDev;
        }
        CDialog::OnOK();
        return;
    }

#else
    WIN32_FIND_DATA fd;
    HANDLE          fh;
    int             i, j;

    i = 0;
    strcpy(buf, MonConf.DriverPath);
    strcat(buf, "..\\flash\\");
    strcat(buf, "*.flm");
    fh = FindFirstFile(buf, &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        do {
            if (i == SelFile)
                break;
            i++;
        } while (FindNextFile(fh, &fd));
        FindClose(fh);
        if (i == SelFile) {
            strcpy(buf, MonConf.DriverPath);
            strcat(buf, "..\\flash\\");
            strcat(buf, fd.cFileName);
            if (LoadFlashDevice(buf)) {
                j = strlen(fd.cFileName) - 1;
                while (j && fd.cFileName[j] != '.')
                    j--;
                fd.cFileName[j]  = 0; // cut '.flm'
                fd.cFileName[31] = 0; // Truncate String
                j                = FlashConf.Nitems;
                strcpy(&FlashConf.Dev[j].FileName[0], fd.cFileName);
                strcpy(&FlashConf.Dev[j].DevName[0], FlashDev.DevName);
                FlashConf.Dev[j].DevType = FlashDev.DevType;
                FlashConf.Dev[j].Start   = FlashDev.DevAdr;
                FlashConf.Dev[j].Size    = FlashDev.szDev;
                CDialog::OnOK();
                return;
            }
        }
    }
#endif
    CDialog::OnCancel();
}


void CAddFD::OnCancel()
{
    CDialog::OnCancel();
}

void InitSetupFD()
{
    SelAlg   = 0;    // Selected Flash Algorithm
    SelFile  = 0;    // Selected Flash File
    pSelFile = NULL; // selected file, full path
    bRteAlgo = 0;    // 1:= selected algo via RTE
}
