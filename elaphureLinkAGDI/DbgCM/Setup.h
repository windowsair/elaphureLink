/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.3
 * @date     $Date: 2016-11-16 18:18:17 +0100 (Wed, 16 Nov 2016) $
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

#include "COLLECT.H"

#include "SetupDbg.h"
#include "SetupTrc.h"
#include "SetupFD.h"
#include "SetupPdsc.h"
#include "CCBPropertySheet.h"

// CSetupPS

class CSetupPS : public CCBPropertySheet
{
    DECLARE_DYNAMIC(CSetupPS)

    public:
    //CSetupPS(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    //CSetupPS(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CSetupPS(int iInitalPage = 0);
    virtual ~CSetupPS();

    CSetupDbg pageDbg;
    CSetupTrc pageTrc;
    CSetupFD  pageFD;
#if DBGCM_DBG_DESCRIPTION
    CSetupPdsc pagePdsc;
#endif // DBGCM_DBG_DESCRIPTION

    protected:
    DECLARE_MESSAGE_MAP()
};


// CSetup dialog

class CSetup : public CDialog
{
    DECLARE_DYNAMIC(CSetup)

    public:
    CSetup(CWnd *pParent = NULL); // standard constructor
    virtual ~CSetup();

    // Dialog Data
    enum { IDD = IDD_SETUP };
    CSetupPS ps;
    DWORD    page;

    protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
};


extern void   StringHex2(CWnd *pCWnd, BYTE val);
extern void   StringHex4(CWnd *pCWnd, WORD val);
extern void   StringHex8(CWnd *pCWnd, DWORD val);
extern void   StringDec(CWnd *pCWnd, DWORD val);
extern void   StringDouble(CWnd *pCWnd, double val, int precision);
extern BOOL   GetDlgHex2(CWnd *pCWnd, BYTE oldval, BYTE *newval);
extern BOOL   GetDlgHex4(CWnd *pCWnd, WORD oldval, WORD *newval);
extern BOOL   GetDlgHex8(CWnd *pCWnd, DWORD oldval, DWORD *newval);
extern DWORD  GetDlgDec(CWnd *pCWnd, DWORD oldval, DWORD min, DWORD max);
extern double GetDlgDouble(CWnd *pCWnd, double oldval, double min, double max, int precision);
