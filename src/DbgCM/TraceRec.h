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


/* Trace Record Display Type */
typedef enum {
  TRD_NONE = -1,
  TRD_UNKNOWN,
  TRD_CNT_EVT,          // Counter Event
  TRD_EXC_EVT,          // Exception Event
  TRD_EXC_INVALID,      // Exception Invalid (reserved)
  TRD_EXC_ENTRY,        // Exception Entry
  TRD_EXC_EXIT,         // Exception Exit
  TRD_EXC_RETURN,       // Exception Return
  TRD_PC_SAMPLE,        // PC Sample
  TRD_DATA_READ,        // Data Read
  TRD_DATA_WRITE,       // Data Write
  TRD_SW_ITM,           // SW ITM
} TRD_TYPE;

/* Trace Record Display Item */
typedef struct {
  BYTE   ovf   : 1;     // Overflow Flag
  BYTE   _num  : 1;     // Number exists
  BYTE   _addr : 1;     // Address exists
  BYTE   _data : 2;     // Data (0=None,1=8-bit,2=16-bit,3=32-bit)
  BYTE   _nPC  : 1;     // PC exists
  BYTE   _ts   : 1;     // Timestamp exists
  BYTE   dts   : 1;     // Delayed Timestamp
  BYTE   type;          // Type
  WORD   num;           // Number
  DWORD  addr;          // Address
  DWORD  data;          // Data
  DWORD  nPC;           // PC Value
  I64    tcyc;          // Timestamp Cycles
  double time;          // Timestamp Time
} TRD_ITEM;


// CTraceRec dialog

class CTraceRec : public CDialog
{
  DECLARE_DYNAMIC(CTraceRec)

public:
  CTraceRec(DYMENU *pM, CWnd* pParent = NULL);   // standard constructor
  virtual ~CTraceRec();

  void Update(void);

// Dialog Data
  enum { IDD = IDD_TRACE_REC };

  DYMENU *pMen;

  DWORD   initflag;

protected:
  virtual void PostNcDestroy();

  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnClose();
  afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
  virtual BOOL OnInitDialog();
  afx_msg void OnLvnKeydownTraceReclist(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnNMDblclkTraceReclist(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnTraceCounter();
  afx_msg void OnUpdateTraceCounter(CCmdUI *pCmdUI);
  afx_msg void OnTraceException();
  afx_msg void OnUpdateTraceException(CCmdUI *pCmdUI);
  afx_msg void OnTracePcsample();
  afx_msg void OnUpdateTracePcsample(CCmdUI *pCmdUI);
  afx_msg void OnTraceItm();
  afx_msg void OnUpdateTraceItm(CCmdUI *pCmdUI);
  afx_msg void OnTraceDataread();
  afx_msg void OnUpdateTraceDataread(CCmdUI *pCmdUI);
  afx_msg void OnTraceDatawrite();
  afx_msg void OnUpdateTraceDatawrite(CCmdUI *pCmdUI);
protected:
  virtual void OnOK();
  virtual void OnCancel();
};


extern DIAD TR_Dlg;

extern void TR_Update (void);
extern void TR_Kill   (DIAD   *pM);
extern void TR_Disp   (DYMENU *pM);
