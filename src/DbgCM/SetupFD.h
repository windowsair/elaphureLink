/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.1
 * @date     $Date: 2015-04-28 15:09:20 +0200 (Tue, 28 Apr 2015) $
 *
 * @note
 * Copyright (C) 2009-2015 ARM Limited. All rights reserved.
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


// CSetupFD dialog

class CSetupFD : public CPropertyPage
{
  DECLARE_DYNCREATE(CSetupFD)

public:
  CSetupFD();
  virtual ~CSetupFD();

  void AddItem    (int index, BOOL select);
  void UpdateItem (int index);

// Dialog Data
  enum { IDD = IDD_SETUP_FD };

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnFlashErase();
  afx_msg void OnFlashProgram();
  afx_msg void OnFlashVerify();
  afx_msg void OnFlashResetRun();
  afx_msg void OnKillfocusFlashStart();
  afx_msg void OnKillfocusFlashSize();
  afx_msg void OnKillfocusRAMStart();
  afx_msg void OnKillfocusRAMSize();
  afx_msg void OnItemchangingAlgList(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemchangedAlgList(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnFlashAdd();
  afx_msg void OnFlashRemove();

  DECLARE_MESSAGE_MAP()
};


// AddFD dialog

class CAddFD : public CDialog
{
  DECLARE_DYNAMIC(CAddFD)

public:
  CAddFD(CWnd* pParent = NULL);   // standard constructor
  virtual ~CAddFD();

// Dialog Data
  enum { IDD = IDD_FDADD };

  struct falgs  {                 // 15.11.2012
    char           *AlgPath;      // path of .flm
    unsigned int   bRte : 1;      // 1:=Algo supplied via RTE
    unsigned int        :31;      // unused
  };
#define _NMAXALGS  1500           // should be enough...
  struct falgs       algs[_NMAXALGS];
  int               nAlgs;        // ==========

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnItemchangingAlgList(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemchangedAlgList(NMHDR* pNMHDR, LRESULT* pResult);
  virtual void OnOK();
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()
};
