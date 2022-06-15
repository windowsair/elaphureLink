/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.3
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
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


// CSetupTrc dialog

class CSetupTrc : public CPropertyPage
{
    DECLARE_DYNAMIC(CSetupTrc)

    public:
    CSetupTrc();
    virtual ~CSetupTrc();

    void Update(void);
    void UpdateTrace(void);
    void UpdateITM_Ena(void);
    void UpdateITM_Priv(void);

    void UpdateTPIUClock(void); // 02.04.2019: Separate Trace Clock setting

    // Dialog Data
    enum { IDD = IDD_SETUP_TRC };

    DWORD  SWOClock;
    DWORD  SWOPresc;
    double PCPeriod;
    int    TRStatus;

    protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();

    private:
    void InitTracePortCombo();           // Initialize data fields and remove unsupported entries
    int  GetTracePortComboSel();         // Get Trace Port Combo Selection ("ID" instead of index)
    void SetTracePortComboSel(int port); // Set Trace Port Combo Selection ("ID" instead of index)

    DECLARE_MESSAGE_MAP()
    public:
    afx_msg void OnKillfocusTraceClock();
    afx_msg void OnTraceEnable();
    afx_msg void OnSelchangeTracePort();
    afx_msg void OnKillfocusTraceSwoPre();
    afx_msg void OnClickedSwoAp();
    afx_msg void OnClickedTraceTimestamp();
    afx_msg void OnSelchangeTraceTsPre();
    afx_msg void OnSelchangeTracePcPre();
    afx_msg void OnClickedTracePcsample();
    afx_msg void OnClickedTracePcData();
    afx_msg void OnClickedTraceCpi();
    afx_msg void OnClickedTraceExc();
    afx_msg void OnClickedTraceSleep();
    afx_msg void OnClickedTraceLsu();
    afx_msg void OnClickedTraceFold();
    afx_msg void OnClickedTraceExctrc();
    afx_msg void OnKillfocusItmTe();
    afx_msg void OnClickedItmTe0();
    afx_msg void OnClickedItmTe1();
    afx_msg void OnClickedItmTe2();
    afx_msg void OnClickedItmTe3();
    afx_msg void OnClickedItmTe4();
    afx_msg void OnClickedItmTe5();
    afx_msg void OnClickedItmTe6();
    afx_msg void OnClickedItmTe7();
    afx_msg void OnClickedItmTe8();
    afx_msg void OnClickedItmTe9();
    afx_msg void OnClickedItmTe10();
    afx_msg void OnClickedItmTe11();
    afx_msg void OnClickedItmTe12();
    afx_msg void OnClickedItmTe13();
    afx_msg void OnClickedItmTe14();
    afx_msg void OnClickedItmTe15();
    afx_msg void OnClickedItmTe16();
    afx_msg void OnClickedItmTe17();
    afx_msg void OnClickedItmTe18();
    afx_msg void OnClickedItmTe19();
    afx_msg void OnClickedItmTe20();
    afx_msg void OnClickedItmTe21();
    afx_msg void OnClickedItmTe22();
    afx_msg void OnClickedItmTe23();
    afx_msg void OnClickedItmTe24();
    afx_msg void OnClickedItmTe25();
    afx_msg void OnClickedItmTe26();
    afx_msg void OnClickedItmTe27();
    afx_msg void OnClickedItmTe28();
    afx_msg void OnClickedItmTe29();
    afx_msg void OnClickedItmTe30();
    afx_msg void OnClickedItmTe31();
    afx_msg void OnKillfocusItmTp();
    afx_msg void OnClickedItmTp0();
    afx_msg void OnClickedItmTp1();
    afx_msg void OnClickedItmTp2();
    afx_msg void OnClickedItmTp3();
    afx_msg void OnClickedEtmEnable();
    afx_msg void OnKillfocusTPIUClock();
    afx_msg void OnUseCoreClk();
};
