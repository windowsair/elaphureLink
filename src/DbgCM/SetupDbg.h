/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.2
 * @date     $Date: 2016-10-14 18:09:40 +0200 (Fri, 14 Oct 2016) $
 *
 * @note
 * Copyright (C) 2009-2016 ARM Limited. All rights reserved.
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

#pragma once

#include "Collect.h"

// CSetupDbg dialog

class CSetupDbg : public CPropertyPage
{
  DECLARE_DYNCREATE(CSetupDbg)

public:
  CSetupDbg();
  virtual ~CSetupDbg();

  void Update(void);

// Dialog Data
  enum { IDD = IDD_SETUP_DBG };
  CString	m_sernum;
  CString	m_hversion;
  CString	m_fversion;

  CImageList * m_pImageListState;

  CToolTipCtrl ToolTip;

protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnSetActive();
  afx_msg void OnSelchangeConfigUnit();
  afx_msg void OnSelchangeConfigPort();
  afx_msg void OnSelchangeConfigClk();
  afx_msg void OnConfigSwj();
  afx_msg void OnCacheCode();
  afx_msg void OnCacheMem();
  afx_msg void OnCodeVerify();
  afx_msg void OnFlashLoad();
  afx_msg void OnBootReset();
  afx_msg void OnBootRun();
  afx_msg void OnSelchangeInitRst();
  afx_msg void OnSelchangeRstType();
  afx_msg void OnRstVectCatch();
  afx_msg void OnItemchangedConfigDevice(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnJtagAuto();
  afx_msg void OnJtagManual();
  afx_msg void OnJtagUpdate();
  afx_msg void OnJtagAdd();
  afx_msg void OnJtagDelete();
  afx_msg void OnJtagUp();
  afx_msg void OnJtagDown();
  afx_msg void OnKillfocusConfigAP();

  DECLARE_MESSAGE_MAP()

#if DBGCM_DBG_DESCRIPTION
  int PdscTargetConnect(void);
#endif // DBGCM_DBG_DESCRIPTION
};
