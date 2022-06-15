/**************************************************************************//**
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

#pragma once

#ifndef __SETUPPDSC_H__

#include "Collect.h"

// CSetupPdsc dialog

class CSetupPdsc : public CPropertyPage
{
  DECLARE_DYNCREATE(CSetupPdsc)

public:
  CSetupPdsc();
  virtual ~CSetupPdsc();

  void Update(void);

// Dialog Data
  enum { IDD = IDD_SETUP_PDSC };

  CImageList * m_pImageListState;

  CToolTipCtrl ToolTip;

protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnSetActive();

  afx_msg void OnCheckPdscEnable();
  afx_msg void OnCheckPdscLog();
  afx_msg void OnBnDbgConfEdit();

  DECLARE_MESSAGE_MAP()
};

#endif // __SETUPPDSC_H__
