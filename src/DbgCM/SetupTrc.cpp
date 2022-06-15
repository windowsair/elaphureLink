/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.11
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
 *
 * @brief     Trace Setup Dialog
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
#include "DbgCM.h"
#include "Debug.h"
#include "Trace.h"
#include "SWV.h"
#include "Setup.h"
#include "SetupTrc.h"

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION


// CSetupTrc dialog

IMPLEMENT_DYNAMIC(CSetupTrc, CPropertyPage)

CSetupTrc::CSetupTrc()
  : CPropertyPage(CSetupTrc::IDD)
{
}

CSetupTrc::~CSetupTrc()
{
}

BEGIN_MESSAGE_MAP(CSetupTrc, CPropertyPage)
  ON_EN_KILLFOCUS(IDC_TRACE_CLOCK, &CSetupTrc::OnKillfocusTraceClock)
  ON_BN_CLICKED(IDC_TRACE_ENABLE, &CSetupTrc::OnTraceEnable)
  ON_CBN_SELCHANGE(IDC_TRACE_PORT, &CSetupTrc::OnSelchangeTracePort)
  ON_EN_KILLFOCUS(IDC_TRACE_SWO_PRE, &CSetupTrc::OnKillfocusTraceSwoPre)
  ON_BN_CLICKED(IDC_TRACE_SWO_AP, &CSetupTrc::OnClickedSwoAp)
  ON_BN_CLICKED(IDC_TRACE_TIMESTAMP, &CSetupTrc::OnClickedTraceTimestamp)
  ON_CBN_SELCHANGE(IDC_TRACE_TS_PRE, &CSetupTrc::OnSelchangeTraceTsPre)
  ON_CBN_SELCHANGE(IDC_TRACE_PC_PRE, &CSetupTrc::OnSelchangeTracePcPre)
  ON_BN_CLICKED(IDC_TRACE_PCSAMPLE, &CSetupTrc::OnClickedTracePcsample)
  ON_BN_CLICKED(IDC_TRACE_PC_DATA, &CSetupTrc::OnClickedTracePcData)
  ON_BN_CLICKED(IDC_TRACE_CPI, &CSetupTrc::OnClickedTraceCpi)
  ON_BN_CLICKED(IDC_TRACE_EXC, &CSetupTrc::OnClickedTraceExc)
  ON_BN_CLICKED(IDC_TRACE_SLEEP, &CSetupTrc::OnClickedTraceSleep)
  ON_BN_CLICKED(IDC_TRACE_LSU, &CSetupTrc::OnClickedTraceLsu)
  ON_BN_CLICKED(IDC_TRACE_FOLD, &CSetupTrc::OnClickedTraceFold)
  ON_BN_CLICKED(IDC_TRACE_EXCTRC, &CSetupTrc::OnClickedTraceExctrc)
  ON_EN_KILLFOCUS(IDC_ITM_TE, &CSetupTrc::OnKillfocusItmTe)
  ON_BN_CLICKED(IDC_ITM_TE0, &CSetupTrc::OnClickedItmTe0)
  ON_BN_CLICKED(IDC_ITM_TE1, &CSetupTrc::OnClickedItmTe1)
  ON_BN_CLICKED(IDC_ITM_TE2, &CSetupTrc::OnClickedItmTe2)
  ON_BN_CLICKED(IDC_ITM_TE3, &CSetupTrc::OnClickedItmTe3)
  ON_BN_CLICKED(IDC_ITM_TE4, &CSetupTrc::OnClickedItmTe4)
  ON_BN_CLICKED(IDC_ITM_TE5, &CSetupTrc::OnClickedItmTe5)
  ON_BN_CLICKED(IDC_ITM_TE6, &CSetupTrc::OnClickedItmTe6)
  ON_BN_CLICKED(IDC_ITM_TE7, &CSetupTrc::OnClickedItmTe7)
  ON_BN_CLICKED(IDC_ITM_TE8, &CSetupTrc::OnClickedItmTe8)
  ON_BN_CLICKED(IDC_ITM_TE9, &CSetupTrc::OnClickedItmTe9)
  ON_BN_CLICKED(IDC_ITM_TE10, &CSetupTrc::OnClickedItmTe10)
  ON_BN_CLICKED(IDC_ITM_TE11, &CSetupTrc::OnClickedItmTe11)
  ON_BN_CLICKED(IDC_ITM_TE12, &CSetupTrc::OnClickedItmTe12)
  ON_BN_CLICKED(IDC_ITM_TE13, &CSetupTrc::OnClickedItmTe13)
  ON_BN_CLICKED(IDC_ITM_TE14, &CSetupTrc::OnClickedItmTe14)
  ON_BN_CLICKED(IDC_ITM_TE15, &CSetupTrc::OnClickedItmTe15)
  ON_BN_CLICKED(IDC_ITM_TE16, &CSetupTrc::OnClickedItmTe16)
  ON_BN_CLICKED(IDC_ITM_TE17, &CSetupTrc::OnClickedItmTe17)
  ON_BN_CLICKED(IDC_ITM_TE18, &CSetupTrc::OnClickedItmTe18)
  ON_BN_CLICKED(IDC_ITM_TE19, &CSetupTrc::OnClickedItmTe19)
  ON_BN_CLICKED(IDC_ITM_TE20, &CSetupTrc::OnClickedItmTe20)
  ON_BN_CLICKED(IDC_ITM_TE21, &CSetupTrc::OnClickedItmTe21)
  ON_BN_CLICKED(IDC_ITM_TE22, &CSetupTrc::OnClickedItmTe22)
  ON_BN_CLICKED(IDC_ITM_TE23, &CSetupTrc::OnClickedItmTe23)
  ON_BN_CLICKED(IDC_ITM_TE24, &CSetupTrc::OnClickedItmTe24)
  ON_BN_CLICKED(IDC_ITM_TE25, &CSetupTrc::OnClickedItmTe25)
  ON_BN_CLICKED(IDC_ITM_TE26, &CSetupTrc::OnClickedItmTe26)
  ON_BN_CLICKED(IDC_ITM_TE27, &CSetupTrc::OnClickedItmTe27)
  ON_BN_CLICKED(IDC_ITM_TE28, &CSetupTrc::OnClickedItmTe28)
  ON_BN_CLICKED(IDC_ITM_TE29, &CSetupTrc::OnClickedItmTe29)
  ON_BN_CLICKED(IDC_ITM_TE30, &CSetupTrc::OnClickedItmTe30)
  ON_BN_CLICKED(IDC_ITM_TE31, &CSetupTrc::OnClickedItmTe31)
  ON_EN_KILLFOCUS(IDC_ITM_TP, &CSetupTrc::OnKillfocusItmTp)
  ON_BN_CLICKED(IDC_ITM_TP0, &CSetupTrc::OnClickedItmTp0)
  ON_BN_CLICKED(IDC_ITM_TP1, &CSetupTrc::OnClickedItmTp1)
  ON_BN_CLICKED(IDC_ITM_TP2, &CSetupTrc::OnClickedItmTp2)
  ON_BN_CLICKED(IDC_ITM_TP3, &CSetupTrc::OnClickedItmTp3)
  ON_BN_CLICKED(IDC_ETM_ENABLE, &CSetupTrc::OnClickedEtmEnable)
  ON_EN_KILLFOCUS(IDC_TPIU_CLOCK, &CSetupTrc::OnKillfocusTPIUClock)
  ON_BN_CLICKED(IDC_TRACE_USE_CORECLK_ENABLE, &CSetupTrc::OnUseCoreClk)
END_MESSAGE_MAP()


// CSetupTrc message handlers

void CSetupTrc::Update() {
  DWORD  val;
  int    status;
  double pcperiod;
  char   buf[64];
  BOOL   swo, etb;

  etb = (TraceConf.Protocol == TPIU_ETB);
  swo = (TraceConf.Protocol == TPIU_SWO_MANCHESTER) || (TraceConf.Protocol == TPIU_SWO_UART);

  GetDlgItem(IDC_TRACE_SWO_PRE )->ShowWindow(swo ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_TRACE_SWO_PRE_)->ShowWindow(swo ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_TRACE_SWO_AP  )->ShowWindow(swo ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_TRACE_SWO_CLK )->ShowWindow(swo ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_TRACE_SWO_CLK_)->ShowWindow(swo ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_TRACE_SWO_MHZ )->ShowWindow(swo ? SW_SHOW : SW_HIDE);
  GetDlgItem(IDC_TRACE_STATUS  )->ShowWindow(SW_SHOW /*swo ? SW_SHOW : SW_HIDE*/);

  if (swo) {
    // Autodetect SWO Prescaler
    if (TraceConf.SWV_Pre & 0x8000) {
      for (val = 1; val <= 8192; val++) {
        // if (SWV_Check(TraceConf.Clk / val) == 0) {
        if (SWV_Check(TPIU_Clock / val) == 0) {          // 02.04.2019
          TraceConf.SWV_Pre = 0x8000 | (WORD)(val - 1);
          break;
        }
      }
    }

    val = (TraceConf.SWV_Pre & 0x1FFF) + 1;
    if (SWOPresc != val) {
      StringDec(GetDlgItem(IDC_TRACE_SWO_PRE), val);
      SWOPresc = val;
    }
    // val = TraceConf.Clk / val;
    val = TPIU_Clock / val;                              // 02.04.2019
    if (SWOClock != val) {
      StringDouble(GetDlgItem(IDC_TRACE_SWO_CLK), val / 1E6, 6);
      SWOClock = val;
    }

    if (!(MonConf.Opt & PORT_SW)) {
      status = -1;
    } else {
      // status = SWV_Check(TraceConf.Clk / ((TraceConf.SWV_Pre & 0x1FFF) + 1));
      status = SWV_Check(TPIU_Clock / ((TraceConf.SWV_Pre & 0x1FFF) + 1));  // 02.04.2019
    }
    if (TRStatus != status) {
      switch (status) {
        case -1:
          sprintf(buf, "Error: <SW Port not selected>");
          break;
        case EU17:
          sprintf(buf, "Error: <SWO Clock not supported>");
          break;
        case EU19:
          sprintf(buf, "Error: <SWO Port not supported>");
          break;
        default:
          sprintf(buf, "");
      }
      SetDlgItemText(IDC_TRACE_STATUS, buf);
      TRStatus = status;
    }
  }
  else if(etb) {
    *buf = '\0';
    SetDlgItemText(IDC_TRACE_STATUS, buf);
    TRStatus = 0;
  }
  else {
    sprintf(buf, "Error: <Trace Port not supported>");
    SetDlgItemText(IDC_TRACE_STATUS, buf);
    TRStatus = -1;
  }

  if (TraceConf.Opt & TRACE_PCSAMPLE) {
    val = 64*((TraceConf.CYC_Pre & 0x0F) + 1);
    if (TraceConf.CYC_Pre & 0x10) val *= 16;
    pcperiod = (double)val / TraceConf.Clk;              // 02.04.2019: PC Sample Period in Core Clock
  } else {
    pcperiod = 0.0;
  }
  if (PCPeriod != pcperiod) {
    if (pcperiod == 0.0) {
      sprintf(buf, "<Disabled>");
    }
    else if (pcperiod < 1E-6) {
      sprintf(buf, "%1.1f ns", pcperiod * 1E9);
    }
    else if (pcperiod < 1E-3) {
      sprintf(buf, "%1.3f us", pcperiod * 1E6);
    }
    else if (pcperiod < 1.0) {
      sprintf(buf, "%1.3f ms", pcperiod * 1E3);
    }
    else {
      sprintf(buf, "%1.3f s",  pcperiod);
    }
    SetDlgItemText(IDC_TRACE_PC_PER, buf);
    PCPeriod = pcperiod;
  }

#if DBGCM_DBG_DESCRIPTION
  if (PDSCDebug_IsEnabled()) {
    PDSCDebug_Reinit();
  }
#endif // DBGCM_DBG_DESCRIPTION
}

void CSetupTrc::UpdateTrace() {
  BOOL en;
  BOOL etm, etb, hasTsPre;
  BOOL hasItm;
  int  i;

  en = SetupMode;
  GetDlgItem(IDC_TRACE_ENABLE)->EnableWindow(en);

  // 02.04.2019: Separate TPIU Clock Settings
  UpdateTPIUClock();

  en = TraceConf.Opt & TRACE_ENABLE;
  CheckDlgButton(IDC_TRACE_ENABLE, en ? 1 : 0);

  hasTsPre = (xxCPU != ARM_CM7);  // Cortex-M7 does not have an ITM Timestamp Prescaler
  hasItm   = (xxCPU == ARM_CM3 ||  xxCPU == ARM_CM4 || xxCPU == ARM_CM7 || xxCPU == ARM_CM33 || xxCPU == ARM_CM35P);

  etb = (TraceConf.Protocol == TPIU_ETB);
#if DBGCM_FEATURE_ETM
  etm = (TraceConf.Protocol == TPIU_TRACE_PORT) || etb;
#else // DBGCM_FEATURE_ETM
  etm = FALSE;
#endif // DBGCM_FEATURE_ETM

  GetDlgItem(IDC_TRACE_PORT     )->EnableWindow(en && SetupMode);
  GetDlgItem(IDC_TRACE_SWO_PRE  )->EnableWindow(en && ((TraceConf.SWV_Pre & 0x8000) == 0));
  GetDlgItem(IDC_TRACE_SWO_AP   )->EnableWindow(en);
  GetDlgItem(IDC_TRACE_SWO_CLK  )->EnableWindow(en);
  GetDlgItem(IDC_TRACE_SWO_MHZ  )->EnableWindow(en);
  GetDlgItem(IDC_TRACE_STATUS   )->EnableWindow(en);

  GetDlgItem(IDC_TRACE_TIMESTAMP)->EnableWindow(en && !etb && hasItm);
  GetDlgItem(IDC_TRACE_TS_PRE   )->EnableWindow(en && !etb && hasTsPre && hasItm);
  GetDlgItem(IDC_TRACE_PCSAMPLE )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_PC_PRE   )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_PC_PER   )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_PC_DATA  )->EnableWindow(en && hasItm);

  GetDlgItem(IDC_TRACE_CPI      )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_EXC      )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_SLEEP    )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_LSU      )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_FOLD     )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_TRACE_EXCTRC   )->EnableWindow(en && hasItm);

  GetDlgItem(IDC_ITM_TE         )->EnableWindow(en && hasItm);
  GetDlgItem(IDC_ITM_TP         )->EnableWindow(en && hasItm);

  for (i = 0; i < 32; i++) {
    GetDlgItem(IDC_ITM_TE0 + i)->EnableWindow(en && hasItm);
  }
  for (i = 0; i < 4; i++) {
    GetDlgItem(IDC_ITM_TP0 + i)->EnableWindow(en && hasItm);
  }

#if DBGCM_FEATURE_ETM
  if ((TraceConf.Protocol != TPIU_TRACE_PORT) && (TraceConf.Protocol != TPIU_ETB)) {
    TraceConf.Opt &= ~ ETM_TRACE;
    CheckDlgButton(IDC_ETM_ENABLE, 0);
  }
#else // DBGCM_FEATURE_ETM
  TraceConf.Opt &= ~ ETM_TRACE;
  CheckDlgButton(IDC_ETM_ENABLE, 0);
#endif // DBGCM_FEATURE_ETM
  GetDlgItem(IDC_ETM_ENABLE        )->EnableWindow(en && etm);
  GetDlgItem(IDC_TRACE_TS_PRE_LABEL)->EnableWindow(en && !etb && hasTsPre && hasItm);
}

void CSetupTrc::UpdateITM_Ena() {
  int  i;

  StringHex8(GetDlgItem(IDC_ITM_TE), TraceConf.ITM_Ena);
  for (i = 0; i < 32; i++) {
    CheckDlgButton(IDC_ITM_TE0 + i, (TraceConf.ITM_Ena & (1 << i)) ? 1 : 0);
  }
}

void CSetupTrc::UpdateITM_Priv() {
  int  i;

  StringHex8(GetDlgItem(IDC_ITM_TP), TraceConf.ITM_Priv);
  // for (i = 0; i < 32; i++) {
  for (i = 0; i < 4; i++) {  // 04.02.2019: Condition fixed, used to be i < 32
    CheckDlgButton(IDC_ITM_TP0 + i, (TraceConf.ITM_Priv & (1 << i)) ? 1 : 0);
  }
}


BOOL CSetupTrc::OnInitDialog() {
  int  i;

  CPropertyPage::OnInitDialog();

  if (!SetupMode) {
    GetDlgItem(IDC_TRACE_ENABLE)->EnableWindow(FALSE);
  }

  StringDouble(GetDlgItem(IDC_TRACE_CLOCK), TraceConf.Clk / 1E6, 6);

  UpdateTPIUClock();      // 02.04.2019: Separate Trace Clock setting

  SWOClock = 0xFFFFFFFF;
  SWOPresc = 0xFFFFFFFF;
  PCPeriod = -1.0;
  TRStatus = -1;

  if (xxCPU == ARM_CM7) {
    // Cortex-M7 does not have an ITM Timestamp Prescaler
    TraceConf.TS_Pre = 0;
  }

  // Initialize CheckBoxes
  CheckDlgButton(IDC_TRACE_ENABLE,    (TraceConf.Opt & TRACE_ENABLE)    ? 1:0);
  CheckDlgButton(IDC_TRACE_TIMESTAMP, (TraceConf.Opt & TRACE_TIMESTAMP) ? 1:0);
  CheckDlgButton(IDC_TRACE_PCSAMPLE,  (TraceConf.Opt & TRACE_PCSAMPLE)  ? 1:0);
  CheckDlgButton(IDC_TRACE_PC_DATA,   (TraceConf.Opt & TRACE_PC_DATA)   ? 1:0);
  CheckDlgButton(IDC_TRACE_CPI,       (TraceConf.Opt & TRACE_CPI)       ? 1:0);
  CheckDlgButton(IDC_TRACE_EXC,       (TraceConf.Opt & TRACE_EXC)       ? 1:0);
  CheckDlgButton(IDC_TRACE_SLEEP,     (TraceConf.Opt & TRACE_SLEEP)     ? 1:0);
  CheckDlgButton(IDC_TRACE_LSU,       (TraceConf.Opt & TRACE_LSU)       ? 1:0);
  CheckDlgButton(IDC_TRACE_FOLD,      (TraceConf.Opt & TRACE_FOLD)      ? 1:0);
  CheckDlgButton(IDC_TRACE_EXCTRC,    (TraceConf.Opt & TRACE_EXCTRC)    ? 1:0);
  CheckDlgButton(IDC_TRACE_SWO_AP,    (TraceConf.SWV_Pre & 0x8000)      ? 1:0);
#if DBGCM_FEATURE_ETM
  CheckDlgButton(IDC_ETM_ENABLE,      (TraceConf.Opt & ETM_TRACE)       ? 1:0);
#else // DBGCM_FEATURE_ETM
  CheckDlgButton(IDC_ETM_ENABLE, 0);
  GetDlgItem(IDC_ETM_ENABLE)->ShowWindow(SW_HIDE);
#endif // DBGCM_FEATURE_ETM

  CheckDlgButton(IDC_TRACE_USE_CORECLK_ENABLE, (TraceConf.Opt & TRACE_USE_CORECLK) ? 1:0);  // 02.04.2019: Separate Trace Clock setting

  // Initialize ComboBoxes
  InitTracePortCombo();  // Prepare the entry list and item data

  switch (TraceConf.Protocol) {
    case TPIU_TRACE_PORT:
      switch (TraceConf.PortSize) {
        case 0x01: i = 0; break;
        case 0x02: i = 1; break;
        case 0x08: i = 2; break;
        default:   i = 2;
      }
      break;
    case TPIU_SWO_MANCHESTER:
      i = 3;
      break;
    case TPIU_SWO_UART:
      i = 4;
      break;
    case TPIU_ETB:
      i = 5;
      break;
  }
  // ((CComboBox *)GetDlgItem(IDC_TRACE_PORT)  )->SetCurSel(i);
  SetTracePortComboSel(i);
  ((CComboBox *)GetDlgItem(IDC_TRACE_TS_PRE))->SetCurSel(TraceConf.TS_Pre);
  ((CComboBox *)GetDlgItem(IDC_TRACE_PC_PRE))->SetCurSel(TraceConf.CYC_Pre);

  return (TRUE);
}


BOOL CSetupTrc::OnSetActive() {

  Update();
  UpdateTrace();
  UpdateITM_Ena ();
  UpdateITM_Priv();

  return CPropertyPage::OnSetActive();
}


// Initialize data fields and remove unsupported entries
void CSetupTrc::InitTracePortCombo() {
  int i;
  CComboBox *pC = (CComboBox *)GetDlgItem(IDC_TRACE_PORT);

  // Set dwData with trace port selection ID (same as the initial index in the combo)
  for (i = 0; i < pC->GetCount(); i++) {
    pC->SetItemData(i, i);
  }

  //---TODO:
  // Remove unsupported trace ports from selection (reverse order to have predictable indexes).
  // By default the entire range of possible trace ports is available.
  // Comment in the corresponding line to deactivate a trace port selection.
  DEVELOP_MSG("Todo: \nDeactivate unsupported trace ports");

  // pC->DeleteString(5); // Remove Embedded Trace Buffer
  // pC->DeleteString(4); // Remove Serial Wire Output - UART/NRZ
  // pC->DeleteString(3); // Remove Serial Wire Output - Manchester
  // pC->DeleteString(2); // Remove Sync Trace Port with 4-bit Data
  // pC->DeleteString(1); // Remove Sync Trace Port with 2-bit Data
  // pC->DeleteString(0); // Remove Sync Trace Port with 1-bit Data
}


// Get Trace Port Combo Selection ("ID" instead of index)
int CSetupTrc::GetTracePortComboSel() {
  CComboBox *pC = (CComboBox *)GetDlgItem(IDC_TRACE_PORT);
  int    curSel = pC->GetCurSel();
  return (int)pC->GetItemData(curSel);
}


// Set Trace Port Combo Selection ("ID" instead of index)
void CSetupTrc::SetTracePortComboSel(int port) {
  CComboBox *pC = (CComboBox *)GetDlgItem(IDC_TRACE_PORT);
  int i;

  for (i = 0; i < pC->GetCount(); i++) {
    if ((int)pC->GetItemData(i) == port) {
      pC->SetCurSel(i);
      return;
    }
  }

  // No hit, use entry 0
  pC->SetCurSel(0);
}


void CSetupTrc::OnKillfocusTraceClock() {
  double result;

  result = GetDlgDouble(GetDlgItem(IDC_TRACE_CLOCK), TraceConf.Clk / 1E6, 1E-6, 999.999999, 6);
  if (result != -1.0e-308) {
    TraceConf.Clk = (DWORD)(result * 1E6);

    // 02.04.2019: Separate Trace Clock setting
    if (TraceConf.Opt & TRACE_USE_CORECLK) {
      TPIU_Clock = TraceConf.Clk;
    }
    UpdateTPIUClock();

    Update();
  }
}

void CSetupTrc::OnTraceEnable() {

  if (IsDlgButtonChecked(IDC_TRACE_ENABLE)) {
    TraceConf.Opt |=  TRACE_ENABLE;
  } else {
    TraceConf.Opt &= ~TRACE_ENABLE;
  }
  UpdateTrace();
}


void CSetupTrc::OnClickedEtmEnable() {
  if (IsDlgButtonChecked(IDC_ETM_ENABLE)) {
    TraceConf.Opt |=  ETM_TRACE;
  } else {
    TraceConf.Opt &= ~ETM_TRACE;
  }
}


void CSetupTrc::OnSelchangeTracePort() {
  int  i;

  // i = ((CComboBox *)GetDlgItem(IDC_TRACE_PORT))->GetCurSel();
  i = GetTracePortComboSel();
  switch (i) {
    case 0:  // 1-bit Sync Trace Port
      TraceConf.Protocol = TPIU_TRACE_PORT;
      TraceConf.PortSize = 0x01;
      break;
    case 1:  // 2-bit Sync Trace Port
      TraceConf.Protocol = TPIU_TRACE_PORT;
      TraceConf.PortSize = 0x02;
      break;
    case 2:  // 4-bit Sync Trace Port
      TraceConf.Protocol = TPIU_TRACE_PORT;
      TraceConf.PortSize = 0x08;
      break;
    case 3:  // SWO Manchester
      TraceConf.Protocol = TPIU_SWO_MANCHESTER;
      break;
    case 4:  // SWO UART
      TraceConf.Protocol = TPIU_SWO_UART;
      break;
    case 5:  // Embedded Trace Buffer
      TraceConf.Protocol = TPIU_ETB;
      break;
  }
  Update();
  UpdateTrace();
}

void CSetupTrc::OnKillfocusTraceSwoPre() {
  DWORD result;

  result = GetDlgDec(GetDlgItem(IDC_TRACE_SWO_PRE), (TraceConf.SWV_Pre & 0x1FFF) + 1, 1, 0x2000);
  if (result != -1) {
    TraceConf.SWV_Pre = (WORD)(result - 1);
    Update();
  }
}

void CSetupTrc::OnClickedSwoAp() {
  BOOL en;

  if (IsDlgButtonChecked(IDC_TRACE_SWO_AP)) {
    TraceConf.SWV_Pre |=  0x8000;
  } else {
    TraceConf.SWV_Pre &= ~0x8000;
  }
  en = (TraceConf.Opt & TRACE_ENABLE) && ((TraceConf.SWV_Pre & 0x8000) == 0);
  GetDlgItem(IDC_TRACE_SWO_PRE)->EnableWindow(en);
  Update();
}


void CSetupTrc::OnClickedTraceTimestamp() {
  if (IsDlgButtonChecked(IDC_TRACE_TIMESTAMP)) {
    TraceConf.Opt |=  TRACE_TIMESTAMP;
  } else {
    TraceConf.Opt &= ~TRACE_TIMESTAMP;
  }
}

void CSetupTrc::OnSelchangeTraceTsPre() {
  TraceConf.TS_Pre = ((CComboBox *)GetDlgItem(IDC_TRACE_TS_PRE))->GetCurSel();
}


void CSetupTrc::OnSelchangeTracePcPre() {
  TraceConf.CYC_Pre = ((CComboBox *)GetDlgItem(IDC_TRACE_PC_PRE))->GetCurSel();
  Update();
}

void CSetupTrc::OnClickedTracePcsample() {
  if (IsDlgButtonChecked(IDC_TRACE_PCSAMPLE)) {
    TraceConf.Opt |=  TRACE_PCSAMPLE;
  } else {
    TraceConf.Opt &= ~TRACE_PCSAMPLE;
  }
  Update();
}

void CSetupTrc::OnClickedTracePcData() {
  if (IsDlgButtonChecked(IDC_TRACE_PC_DATA)) {
    TraceConf.Opt |=  TRACE_PC_DATA;
  } else {
    TraceConf.Opt &= ~TRACE_PC_DATA;
  }
}


void CSetupTrc::OnClickedTraceCpi() {
  if (IsDlgButtonChecked(IDC_TRACE_CPI)) {
    TraceConf.Opt |=  TRACE_CPI;
  } else {
    TraceConf.Opt &= ~TRACE_CPI;
  }
}

void CSetupTrc::OnClickedTraceExc() {
  if (IsDlgButtonChecked(IDC_TRACE_EXC)) {
    TraceConf.Opt |=  TRACE_EXC;
  } else {
    TraceConf.Opt &= ~TRACE_EXC;
  }
}

void CSetupTrc::OnClickedTraceSleep() {
  if (IsDlgButtonChecked(IDC_TRACE_SLEEP)) {
    TraceConf.Opt |=  TRACE_SLEEP;
  } else {
    TraceConf.Opt &= ~TRACE_SLEEP;
  }
}

void CSetupTrc::OnClickedTraceLsu() {
  if (IsDlgButtonChecked(IDC_TRACE_LSU)) {
    TraceConf.Opt |=  TRACE_LSU;
  } else {
    TraceConf.Opt &= ~TRACE_LSU;
  }
}

void CSetupTrc::OnClickedTraceFold() {
  if (IsDlgButtonChecked(IDC_TRACE_FOLD)) {
    TraceConf.Opt |=  TRACE_FOLD;
  } else {
    TraceConf.Opt &= ~TRACE_FOLD;
  }
}

void CSetupTrc::OnClickedTraceExctrc() {
  if (IsDlgButtonChecked(IDC_TRACE_EXCTRC)) {
    TraceConf.Opt |=  TRACE_EXCTRC;
  } else {
    TraceConf.Opt &= ~TRACE_EXCTRC;
  }
}


void CSetupTrc::OnKillfocusItmTe() {
  DWORD val;

  if (GetDlgHex8(GetDlgItem(IDC_ITM_TE), TraceConf.ITM_Ena, &val)) {
    TraceConf.ITM_Ena = val;
    UpdateITM_Ena();
  }
}

#define OnClickItmTE(n)                         \
  if (IsDlgButtonChecked(IDC_ITM_TE##n)) {      \
    TraceConf.ITM_Ena |=  (1 << n);             \
  } else {                                      \
    TraceConf.ITM_Ena &= ~(1 << n);             \
  }                                             \
  UpdateITM_Ena();

void CSetupTrc::OnClickedItmTe0 () { OnClickItmTE( 0) }
void CSetupTrc::OnClickedItmTe1 () { OnClickItmTE( 1) }
void CSetupTrc::OnClickedItmTe2 () { OnClickItmTE( 2) }
void CSetupTrc::OnClickedItmTe3 () { OnClickItmTE( 3) }
void CSetupTrc::OnClickedItmTe4 () { OnClickItmTE( 4) }
void CSetupTrc::OnClickedItmTe5 () { OnClickItmTE( 5) }
void CSetupTrc::OnClickedItmTe6 () { OnClickItmTE( 6) }
void CSetupTrc::OnClickedItmTe7 () { OnClickItmTE( 7) }
void CSetupTrc::OnClickedItmTe8 () { OnClickItmTE( 8) }
void CSetupTrc::OnClickedItmTe9 () { OnClickItmTE( 9) }
void CSetupTrc::OnClickedItmTe10() { OnClickItmTE(10) }
void CSetupTrc::OnClickedItmTe11() { OnClickItmTE(11) }
void CSetupTrc::OnClickedItmTe12() { OnClickItmTE(12) }
void CSetupTrc::OnClickedItmTe13() { OnClickItmTE(13) }
void CSetupTrc::OnClickedItmTe14() { OnClickItmTE(14) }
void CSetupTrc::OnClickedItmTe15() { OnClickItmTE(15) }
void CSetupTrc::OnClickedItmTe16() { OnClickItmTE(16) }
void CSetupTrc::OnClickedItmTe17() { OnClickItmTE(17) }
void CSetupTrc::OnClickedItmTe18() { OnClickItmTE(18) }
void CSetupTrc::OnClickedItmTe19() { OnClickItmTE(19) }
void CSetupTrc::OnClickedItmTe20() { OnClickItmTE(20) }
void CSetupTrc::OnClickedItmTe21() { OnClickItmTE(21) }
void CSetupTrc::OnClickedItmTe22() { OnClickItmTE(22) }
void CSetupTrc::OnClickedItmTe23() { OnClickItmTE(23) }
void CSetupTrc::OnClickedItmTe24() { OnClickItmTE(24) }
void CSetupTrc::OnClickedItmTe25() { OnClickItmTE(25) }
void CSetupTrc::OnClickedItmTe26() { OnClickItmTE(26) }
void CSetupTrc::OnClickedItmTe27() { OnClickItmTE(27) }
void CSetupTrc::OnClickedItmTe28() { OnClickItmTE(28) }
void CSetupTrc::OnClickedItmTe29() { OnClickItmTE(29) }
void CSetupTrc::OnClickedItmTe30() { OnClickItmTE(30) }
void CSetupTrc::OnClickedItmTe31() { OnClickItmTE(31) }


void CSetupTrc::OnKillfocusItmTp() {
  DWORD val;

  if (GetDlgHex8(GetDlgItem(IDC_ITM_TP), TraceConf.ITM_Priv, &val)) {
    TraceConf.ITM_Priv = val;
    UpdateITM_Priv();
  }
}

#define OnClickItmTP(n)                         \
  if (IsDlgButtonChecked(IDC_ITM_TP##n)) {      \
    TraceConf.ITM_Priv |=  (1 << n);            \
  } else {                                      \
    TraceConf.ITM_Priv &= ~(1 << n);            \
  }                                             \
  UpdateITM_Priv();

void CSetupTrc::OnClickedItmTp0() { OnClickItmTP(0) }
void CSetupTrc::OnClickedItmTp1() { OnClickItmTP(1) }
void CSetupTrc::OnClickedItmTp2() { OnClickItmTP(2) }
void CSetupTrc::OnClickedItmTp3() { OnClickItmTP(3) }

// 02.04.2019: Separate Trace Clock setting
void CSetupTrc::UpdateTPIUClock(void) {
  BOOL en;

  en = TRUE;   // 02.04.2019: Separate Trace Clock setting - modify here to (conditionally) force "Use Core Clock" and disable selection

  if (TraceConf.Opt & TRACE_USE_CORECLK) {
    CheckDlgButton(IDC_TRACE_USE_CORECLK_ENABLE, 1);
    GetDlgItem(IDC_TPIU_CLOCK)->EnableWindow(FALSE);
  } else {
    CheckDlgButton(IDC_TRACE_USE_CORECLK_ENABLE, 0);
    GetDlgItem(IDC_TPIU_CLOCK)->EnableWindow(en);
  }
  StringDouble(GetDlgItem(IDC_TPIU_CLOCK), TPIU_Clock / 1E6, 6);
  GetDlgItem(IDC_TRACE_USE_CORECLK_ENABLE)->EnableWindow(en);
}

// 02.04.2019: Separate Trace Clock setting
void CSetupTrc::OnKillfocusTPIUClock() {
  double result;

  result = GetDlgDouble(GetDlgItem(IDC_TPIU_CLOCK), TraceConf.TPIU_Clk / 1E6, 1E-6, 999.999999, 6);
  if (result != -1.0e-308) {
    TraceConf.TPIU_Clk = (DWORD)(result * 1E6);
    if (!(TraceConf.Opt & TRACE_USE_CORECLK)) {
      TPIU_Clock = TraceConf.TPIU_Clk;
    }
    Update();
  }
}

// 02.04.2019: Separate Trace Clock setting
void CSetupTrc::OnUseCoreClk() {

  if (IsDlgButtonChecked(IDC_TRACE_USE_CORECLK_ENABLE)) {
    TraceConf.Opt |= TRACE_USE_CORECLK;
    TPIU_Clock     = TraceConf.Clk;
  } else {
    TraceConf.Opt &= ~TRACE_USE_CORECLK;
    TPIU_Clock     = TraceConf.TPIU_Clk;
  }
  Update();
  UpdateTrace();
}
