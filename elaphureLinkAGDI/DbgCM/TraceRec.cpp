/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.2
 * @date     $Date: 2016-03-24 09:07:53 +0100 (Thu, 24 Mar 2016) $
 *
 * @note
 * Copyright (C) 2009-2015 ARM Limited. All rights reserved.
 *
 * @brief     Display of Trace Records in the Trace Records Window
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
#include "..\AGDI.h"
#include "..\BOM.h"
#include "..\ComTyp.h"
#include "Collect.h"
#include "DbgCM.h"
#include "Trace.h"
#include "TraceRec.h"
#include "..\TraceView\TraceDataTypes.h"


// CTraceRec dialog
static CTraceRec *pCTR;

//             iOpen  Hwnd   Dlg Proc.  Rect: -1 := default   Update     Kill
DIAD TR_Dlg = { 0, NULL, NULL, {
                                   -1,
                                   -1,
                                   -1,
                                   -1,
                               },
                TR_Update,
                TR_Kill };


static void TR_Update(void)
{ // Update Function
    if (pCTR && pCTR->pMen && pCTR->pMen->pDlg && pCTR->pMen->pDlg->hw != NULL) {
        pCTR->Update();
    }
}

static void TR_Kill(DIAD *pM)
{ // Kill Function
    if (pCTR == NULL)
        return;
    pCTR->SendMessage(WM_CLOSE);
    pCTR      = NULL;
    pM->iOpen = 0;
    pM->hw    = NULL;
}


void TR_Disp(DYMENU *pM)
{
    if (pM->pDlg->hw != NULL) { // created
        TR_Kill(pM->pDlg);      // close
    } else {
        pCTR = new CTraceRec(pM, PWND);  // modeless construction
        if (pCTR != NULL) {              // construction was Ok.
            pM->pDlg->hw = pCTR->m_hWnd; // Dialog handle
        }
    }
}


IMPLEMENT_DYNAMIC(CTraceRec, CDialog)

CTraceRec::CTraceRec(DYMENU *pM, CWnd *pParent /*=NULL*/)
    : CDialog(CTraceRec::IDD, pParent)
{
    pMen = pM;
    Create(IDD, pParent);
}

CTraceRec::~CTraceRec()
{
}


#define USE_FAST_CODE

#ifdef USE_FAST_CODE

// Packet Type according to Packet 4-bit Flags (not all combinations are valid)
static const BYTE PacketType[16] = {
    /* EVENT   */ TRD_PC_SAMPLE,
    TRD_CNT_EVT,
    TRD_EXC_EVT,
    TRD_UNKNOWN,
    /* ITM     */ TRD_SW_ITM,
    TRD_SW_ITM,
    TRD_SW_ITM,
    TRD_SW_ITM,
    /* DATA_RD */ TRD_DATA_READ,
    TRD_DATA_READ,
    TRD_DATA_READ,
    TRD_DATA_READ,
    /* DATA_WR */ TRD_DATA_WRITE,
    TRD_DATA_WRITE,
    TRD_DATA_WRITE,
    TRD_DATA_WRITE,
};

// Packet Size according to Packet 8-bit Flags (not all combinations are valid)
static const BYTE PacketSize[256] = {
    // 0x00..0x7F: PADDING (0x00) + Packets without Overflow
    /* 0x00..0x03: EVENT (with PADDING)    */ 0,
    1,
    2,
    4,
    /* 0x04..0x07: ITM                     */ 0,
    1,
    2,
    4,
    /* 0x08..0x0B: DATA_RD                 */ 0,
    1,
    2,
    4,
    /* 0x0C..0x0F: DATA_WR                 */ 0,
    1,
    2,
    4,
    /* 0x10..0x13: EVENT   + PC            */ 4,
    5,
    6,
    8,
    /* 0x14..0x17: ITM     + PC            */ 4,
    5,
    6,
    8,
    /* 0x18..0x1B: DATA_RD + PC            */ 4,
    5,
    6,
    8,
    /* 0x1C..0x1F: DATA_WR + PC            */ 4,
    5,
    6,
    8,
    /* 0x20..0x33: EVENT        + ADR      */ 4,
    5,
    6,
    8,
    /* 0x24..0x37: ITM          + ADR      */ 1,
    2,
    3,
    5,
    /* 0x28..0x3B: DATA_RD      + ADR      */ 4,
    5,
    6,
    8,
    /* 0x2C..0x3F: DATA_WR      + ADR      */ 4,
    5,
    6,
    8,
    /* 0x30..0x33: EVENT   + PC + ADR      */ 8,
    9,
    10,
    12,
    /* 0x34..0x37: ITM     + PC + ADR      */ 5,
    6,
    7,
    9,
    /* 0x38..0x3B: DATA_RD + PC + ADR      */ 8,
    9,
    10,
    12,
    /* 0x3C..0x3F: DATA_WR + PC + ADR      */ 8,
    9,
    10,
    12,
    /* 0x40..0x43: EVENT              + TS */ 4,
    5,
    6,
    8,
    /* 0x44..0x47: ITM                + TS */ 4,
    5,
    6,
    8,
    /* 0x48..0x4B: DATA_RD            + TS */ 4,
    5,
    6,
    8,
    /* 0x4C..0x4F: DATA_WR            + TS */ 4,
    5,
    6,
    8,
    /* 0x50..0x53: EVENT   + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0x54..0x57: ITM     + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0x58..0x5B: DATA_RD + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0x5C..0x5F: DATA_WR + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0x60..0x63: EVENT        + ADR + TS */ 8,
    9,
    10,
    12,
    /* 0x64..0x67: ITM          + ADR + TS */ 5,
    6,
    7,
    9,
    /* 0x68..0x6B: DATA_RD      + ADR + TS */ 8,
    9,
    10,
    12,
    /* 0x6C..0x6F: DATA_WR      + ADR + TS */ 8,
    9,
    10,
    12,
    /* 0x70..0x73: EVENT   + PC + ADR + TS */ 12,
    13,
    14,
    16,
    /* 0x74..0x77: ITM     + PC + ADR + TS */ 9,
    10,
    11,
    13,
    /* 0x78..0x7B: DATA_RD + PC + ADR + TS */ 12,
    13,
    14,
    16,
    /* 0x7C..0x7F: DATA_WR + PC + ADR + TS */ 12,
    13,
    14,
    16,
    // 0x80..0xFF: TIMEINFO (0x80) + Packets with Overflow
    /* 0x80..0x83: EVENT (with TIMEINFO)   */ 20,
    1,
    2,
    4,
    /* 0x84..0x87: ITM                     */ 0,
    1,
    2,
    4,
    /* 0x88..0x8B: DATA_RD                 */ 0,
    1,
    2,
    4,
    /* 0x8C..0x8F: DATA_WR                 */ 0,
    1,
    2,
    4,
    /* 0x90..0x93: EVENT   + PC            */ 4,
    5,
    6,
    8,
    /* 0x94..0x97: ITM     + PC            */ 4,
    5,
    6,
    8,
    /* 0x98..0x9B: DATA_RD + PC            */ 4,
    5,
    6,
    8,
    /* 0x9C..0x9F: DATA_WR + PC            */ 4,
    5,
    6,
    8,
    /* 0xA0..0xA3: EVENT        + ADR      */ 4,
    5,
    6,
    8,
    /* 0xA4..0xA7: ITM          + ADR      */ 1,
    2,
    3,
    5,
    /* 0xA8..0xAB: DATA_RD      + ADR      */ 4,
    5,
    6,
    8,
    /* 0xAC..0xAF: DATA_WR      + ADR      */ 4,
    5,
    6,
    8,
    /* 0xB0..0xB3: EVENT   + PC + ADR      */ 8,
    9,
    10,
    12,
    /* 0xB4..0xB7: ITM     + PC + ADR      */ 5,
    6,
    7,
    9,
    /* 0xB8..0xBB: DATA_RD + PC + ADR      */ 8,
    9,
    10,
    12,
    /* 0xBC..0xBF: DATA_WR + PC + ADR      */ 8,
    9,
    10,
    12,
    /* 0xC0..0xC3: EVENT              + TS */ 4,
    5,
    6,
    8,
    /* 0xC4..0xC7: ITM                + TS */ 4,
    5,
    6,
    8,
    /* 0xC8..0xCB: DATA_RD            + TS */ 4,
    5,
    6,
    8,
    /* 0xCC..0xCF: DATA_WR            + TS */ 4,
    5,
    6,
    8,
    /* 0xD0..0xD3: EVENT   + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0xD4..0xD7: ITM     + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0xD8..0xDB: DATA_RD + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0xDC..0xDF: DATA_WR + PC       + TS */ 8,
    9,
    10,
    12,
    /* 0xE0..0xE3: EVENT        + ADR + TS */ 8,
    9,
    10,
    12,
    /* 0xE4..0xE7: ITM          + ADR + TS */ 5,
    6,
    7,
    9,
    /* 0xE8..0xEB: DATA_RD      + ADR + TS */ 8,
    9,
    10,
    12,
    /* 0xEC..0xEF: DATA_WR      + ADR + TS */ 8,
    9,
    10,
    12,
    /* 0xF0..0xF3: EVENT   + PC + ADR + TS */ 12,
    13,
    14,
    16,
    /* 0xF4..0xF7: ITM     + PC + ADR + TS */ 9,
    10,
    11,
    13,
    /* 0xF8..0xFB: DATA_RD + PC + ADR + TS */ 12,
    13,
    14,
    16,
    /* 0xFC..0xFF: DATA_WR + PC + ADR + TS */ 12,
    13,
    14,
    16,
};

#else

__inline static BYTE PacketType(BYTE flag)
{
    BYTE type;

    switch (flag & TB_MASK) {
        case TB_EVENT:
            switch (flag & TB_DATA_SZ) {
                case 0:
                    type = TRD_PC_SAMPLE;
                    break;
                case TB_DATA_SZ8:
                    type = TRD_CNT_EVT;
                    break;
                case TB_DATA_SZ16:
                    type = TRD_EXC_EVT;
                    break;
                case TB_DATA_SZ32:
                    type = TRD_UNKNOWN;
                    break;
            }
            break;
        case TB_ITM:
            type = TRD_SW_ITM;
            break;
        case TB_DATA_RD:
            type = TRD_DATA_READ;
            break;
        case TB_DATA_WR:
            type = TRD_DATA_WRITE;
            break;
    }
    return (type);
}

__inline static BYTE PacketSize(BYTE flag)
{
    BYTE size = 0;

    if (flag & TB_PC_VALID) {
        size += sizeof(DWORD);
    }
    if (flag & TB_ADR_VALID) {
        if ((flag & TB_MASK) == TB_ITM) {
            size += sizeof(BYTE);
        } else {
            size += sizeof(DWORD);
        }
    }
    switch (flag & TB_DATA_SZ) {
        case TB_DATA_SZ8:
            size += sizeof(BYTE);
            break;
        case TB_DATA_SZ16:
            size += sizeof(WORD);
            break;
        case TB_DATA_SZ32:
            size += sizeof(DWORD);
            break;
    }
    if (flag & TB_TS_VALID) {
        size += sizeof(DWORD);
    }
    return (size);
}

#endif


#define TRD_CNT 20

static DWORD TraceDispMask;  // Display Mask
static int   TraceDispOfs;   // Item Offset
static int   TraceDispCnt;   // Total Item Count
static int   TraceDispItems; // Item Count (display area)

static TRD_ITEM TRecData[TRD_CNT];
static TRD_ITEM TRecDisp[TRD_CNT];


void CTraceRec::Update()
{
    CListCtrl * pLC;
    CScrollBar *pSB;
    LVITEM      item;
    DWORD       idx;
    DWORD       clock;
    I64         cycles;
    double      time;
    BYTE        flag;
    BYTE        type;
    int         ofs;
    int         items;
    int         diff;
    int         i;
    char        buf[32];

    if (!TraceDispFlg)
        return;

    if (!(TraceConf.Opt & TRACE_ENABLE))
        return;

    EnterCriticalSection(&TraceCS);

    if (initflag || (TraceDisp == 0xFFFFFFFF)) {
        TraceDisp    = TraceHead;
        TraceDispOfs = 0;
    }

    // Collect information about records before display area
    idx    = TraceHead;
    clock  = TraceHeadClock;
    cycles = TraceHeadCycles;
    time   = TraceHeadTime;
    ofs    = 0;
    while ((idx != TraceTail) && (idx != TraceDisp) && (ofs != TraceDispOfs)) {
        flag = *((BYTE *)(TraceBuffer + idx));
        idx += sizeof(BYTE);
        if (flag == TB_TIMEINFO) {
            clock = *((DWORD *)(TraceBuffer + idx));
            idx += sizeof(DWORD);
            cycles = *((I64 *)(TraceBuffer + idx));
            idx += sizeof(I64);
            time = *((double *)(TraceBuffer + idx));
            idx += sizeof(double);
        } else if (flag != TB_PADDING) {
#ifdef USE_FAST_CODE
            type = PacketType[flag & 0x0F];
            idx += PacketSize[flag];
#else
            type = PacketType(flag & 0x0F);
            idx += PacketSize(flag);
#endif
            if (TraceDispMask & (1 << type))
                ofs++;
        }
        if (idx == TB_SIZE)
            idx = 0;
    }
    TraceDisp    = idx;
    TraceDispOfs = ofs;
    TraceDispCnt = ofs;

    // Collect information about records for display area
    i = 0;
    while ((idx != TraceTail) && (i < TRD_CNT)) {
        flag = *((BYTE *)(TraceBuffer + idx));
        idx += sizeof(BYTE);
        if (flag == TB_TIMEINFO) {
            clock = *((DWORD *)(TraceBuffer + idx));
            idx += sizeof(DWORD);
            cycles = *((I64 *)(TraceBuffer + idx));
            idx += sizeof(I64);
            time = *((double *)(TraceBuffer + idx));
            idx += sizeof(double);
        } else if (flag != TB_PADDING) {
            TRecData[i].ovf = (flag & TB_OVERFLOW) ? 1 : 0;
            if (flag & TB_PC_VALID) {
                TRecData[i].nPC = *((DWORD *)(TraceBuffer + idx));
                idx += sizeof(DWORD);
                TRecData[i]._nPC = 1;
            } else {
                TRecData[i]._nPC = 0;
            }
            TRecData[i]._num  = 0;
            TRecData[i]._addr = 0;
            if (flag & TB_ADR_VALID) {
                if ((flag & TB_MASK) == TB_ITM) {
                    TRecData[i].num = *((BYTE *)(TraceBuffer + idx));
                    idx += sizeof(BYTE);
                    TRecData[i]._num = 1;
                } else {
                    TRecData[i].addr = *((DWORD *)(TraceBuffer + idx));
                    idx += sizeof(DWORD);
                    TRecData[i]._addr = 1;
                }
            }
            switch (flag & TB_DATA_SZ) {
                case TB_DATA_SZ8:
                    TRecData[i].data = *((BYTE *)(TraceBuffer + idx));
                    idx += sizeof(BYTE);
                    TRecData[i]._data = 1;
                    break;
                case TB_DATA_SZ16:
                    TRecData[i].data = *((WORD *)(TraceBuffer + idx));
                    idx += sizeof(WORD);
                    TRecData[i]._data = 2;
                    break;
                case TB_DATA_SZ32:
                    TRecData[i].data = *((DWORD *)(TraceBuffer + idx));
                    idx += sizeof(DWORD);
                    TRecData[i]._data = 3;
                    break;
                default:
                    TRecData[i]._data = 0;
                    break;
            }
            if (flag & TB_TS_VALID) {
                diff = *((DWORD *)(TraceBuffer + idx));
                idx += sizeof(DWORD);
                if (diff < 0) {
                    diff            = -diff;
                    TRecData[i].dts = 1;
                } else {
                    TRecData[i].dts = 0;
                }
                TRecData[i].tcyc = cycles + diff;
                TRecData[i].time = time + (double)diff / clock;
                TRecData[i]._ts  = 1;
            } else {
                TRecData[i]._ts = 0;
            }
            switch (flag & TB_MASK) {
                case TB_EVENT:
                    switch (flag & TB_DATA_SZ) {
                        case 0:
                            TRecData[i].type = type = TRD_PC_SAMPLE;
                            break;
                        case TB_DATA_SZ8:
                            TRecData[i].type = type = TRD_CNT_EVT;
                            break;
                        case TB_DATA_SZ16:
                            type = TRD_EXC_EVT;
                            switch ((TRecData[i].data >> TR_EXC_TYP_POS) & TR_EXC_TYP_MSK) {
                                case TR_EXC_INVALID:
                                    TRecData[i].type  = TRD_EXC_INVALID;
                                    TRecData[i].num   = (WORD)(TRecData[i].data & TR_EXC_NUM_MSK);
                                    TRecData[i]._num  = 1;
                                    TRecData[i]._data = 0;
                                    break;
                                case TR_EXC_ENTRY:
                                    TRecData[i].type  = TRD_EXC_ENTRY;
                                    TRecData[i].num   = (WORD)(TRecData[i].data & TR_EXC_NUM_MSK);
                                    TRecData[i]._num  = 1;
                                    TRecData[i]._data = 0;
                                    break;
                                case TR_EXC_EXIT:
                                    TRecData[i].type  = TRD_EXC_EXIT;
                                    TRecData[i].num   = (WORD)(TRecData[i].data & TR_EXC_NUM_MSK);
                                    TRecData[i]._num  = 1;
                                    TRecData[i]._data = 0;
                                    break;
                                case TR_EXC_RETURN:
                                    TRecData[i].type  = TRD_EXC_RETURN;
                                    TRecData[i].num   = (WORD)(TRecData[i].data & TR_EXC_NUM_MSK);
                                    TRecData[i]._num  = 1;
                                    TRecData[i]._data = 0;
                                    break;
                            }
                            break;
                        case TB_DATA_SZ32:
                            TRecData[i].type = type = TRD_UNKNOWN;
                            break;
                    }
                    break;
                case TB_ITM:
                    TRecData[i].type = type = TRD_SW_ITM;
                    break;
                case TB_DATA_RD:
                    TRecData[i].type = type = TRD_DATA_READ;
                    break;
                case TB_DATA_WR:
                    TRecData[i].type = type = TRD_DATA_WRITE;
                    break;
            }
            if (TraceDispMask & (1 << type))
                i++;
        }
        if (idx == TB_SIZE)
            idx = 0;
    }
    TraceDispCnt += i;
    items = i;

    // Collect information about records after display area
    while (idx != TraceTail) {
        flag = *((BYTE *)(TraceBuffer + idx));
        idx += sizeof(BYTE);
        if (flag == TB_TIMEINFO) {
            idx += sizeof(DWORD) + sizeof(I64) + sizeof(double);
        } else if (flag != TB_PADDING) {
#ifdef USE_FAST_CODE
            idx += PacketSize[flag];
            type = PacketType[flag & 0x0F];
#else
            type = PacketType(flag & 0x0F);
            idx += PacketSize(flag);
#endif
            if (TraceDispMask & (1 << type))
                TraceDispCnt++;
        }
        if (idx == TB_SIZE)
            idx = 0;
    }

    LeaveCriticalSection(&TraceCS);

    if (initflag) {
        initflag       = 0;
        TraceDispItems = 0;
    }

    for (i = TraceDispItems; i < items; i++) {
        *((BYTE *)&TRecDisp[i]) = ~(*((BYTE *)&TRecData[i]));
        TRecDisp[i].type        = ~TRecData[i].type;
        TRecDisp[i].num         = ~TRecData[i].num;
        TRecDisp[i].addr        = ~TRecData[i].addr;
        TRecDisp[i].data        = ~TRecData[i].data;
        TRecDisp[i].nPC         = ~TRecData[i].nPC;
        TRecDisp[i].tcyc        = ~TRecData[i].tcyc;
        TRecDisp[i].time        = TRecData[i].time + 1.0;
    }

    pLC = (CListCtrl *)GetDlgItem(IDC_TRACE_RECLIST);

    pLC->SetRedraw(FALSE);

    while (TraceDispItems > items) {
        TraceDispItems--;
        pLC->DeleteItem(items);
    }

    // Update ListCtrl Data
    memset(&item, 0, sizeof(item));
    item.mask = LVIF_TEXT;
    for (i = 0; i < items; i++) {
        item.iItem = i;
        if (TRecDisp[i].type != TRecData[i].type) {
            TRecDisp[i].type = TRecData[i].type;
            item.iSubItem    = 0;
            switch (TRecData[i].type) {
                case TRD_UNKNOWN:
                    item.pszText = "Unknown Type";
                    break;
                case TRD_CNT_EVT:
                    item.pszText = "Counter Event";
                    break;
                case TRD_EXC_INVALID:
                    item.pszText = "Exception ???";
                    break;
                case TRD_EXC_ENTRY:
                    item.pszText = "Exception Entry";
                    break;
                case TRD_EXC_EXIT:
                    item.pszText = "Exception Exit";
                    break;
                case TRD_EXC_RETURN:
                    item.pszText = "Exception Return";
                    break;
                case TRD_PC_SAMPLE:
                    item.pszText = "PC Sample";
                    break;
                case TRD_DATA_READ:
                    item.pszText = "Data Read";
                    break;
                case TRD_DATA_WRITE:
                    item.pszText = "Data Write";
                    break;
                case TRD_SW_ITM:
                    item.pszText = "ITM";
                    break;
            }
            if (i >= TraceDispItems) {
                TraceDispItems++;
                pLC->InsertItem(&item);
            } else {
                pLC->SetItem(&item);
            }
        }
        if (TRecDisp[i].ovf != TRecData[i].ovf) {
            TRecDisp[i].ovf = TRecData[i].ovf;
            item.iSubItem   = 1;
            item.pszText    = TRecData[i].ovf ? "X" : "";
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._num != TRecData[i]._num) || (TRecDisp[i].num != TRecData[i].num)) {
            TRecDisp[i]._num = TRecData[i]._num;
            TRecDisp[i].num  = TRecData[i].num;
            item.iSubItem    = 2;
            if (TRecData[i]._num) {
                sprintf(buf, "%d", TRecData[i].num);
                item.pszText = buf;
            } else {
                item.pszText = "";
            }
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._addr != TRecData[i]._addr) || (TRecDisp[i].addr != TRecData[i].addr)) {
            TRecDisp[i]._addr = TRecData[i]._addr;
            TRecDisp[i].addr  = TRecData[i].addr;
            item.iSubItem     = 3;
            if (TRecData[i]._addr) {
                sprintf(buf, "%08XH", TRecData[i].addr);
                item.pszText = buf;
            } else {
                item.pszText = "";
            }
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._data != TRecData[i]._data) || (TRecDisp[i].data != TRecData[i].data)) {
            TRecDisp[i]._data = TRecData[i]._data;
            TRecDisp[i].data  = TRecData[i].data;
            item.iSubItem     = 4;
            switch (TRecData[i]._data) {
                case 0:
                    item.pszText = "";
                    break;
                case 1:
                    sprintf(buf, "%02XH", (BYTE)TRecData[i].data);
                    item.pszText = buf;
                    break;
                case 2:
                    sprintf(buf, "%04XH", (WORD)TRecData[i].data);
                    item.pszText = buf;
                    break;
                case 3:
                    sprintf(buf, "%08XH", (DWORD)TRecData[i].data);
                    item.pszText = buf;
                    break;
            }
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._nPC != TRecData[i]._nPC) || (TRecDisp[i].nPC != TRecData[i].nPC)) {
            TRecDisp[i]._nPC = TRecData[i]._nPC;
            TRecDisp[i].nPC  = TRecData[i].nPC;
            item.iSubItem    = 5;
            if (TRecData[i]._nPC) {
                sprintf(buf, "%08XH", TRecData[i].nPC);
                item.pszText = buf;
            } else {
                item.pszText = "";
            }
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._ts != TRecData[i]._ts) || (TRecDisp[i].dts != TRecData[i].dts)) {
            TRecDisp[i].dts = TRecData[i].dts;
            item.iSubItem   = 6;
            item.pszText    = (TRecData[i]._ts && TRecData[i].dts) ? "X" : "";
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._ts != TRecData[i]._ts) || (TRecDisp[i].tcyc != TRecData[i].tcyc)) {
            TRecDisp[i].tcyc = TRecData[i].tcyc;
            item.iSubItem    = 7;
            if (TRecData[i]._ts) {
                sprintf(buf, "%I64d", TRecData[i].tcyc);
                item.pszText = buf;
            } else {
                item.pszText = "";
            }
            pLC->SetItem(&item);
        }
        if ((TRecDisp[i]._ts != TRecData[i]._ts) || (TRecDisp[i].time != TRecData[i].time)) {
            TRecDisp[i].time = TRecData[i].time;
            item.iSubItem    = 8;
            if (TRecData[i]._ts) {
                sprintf(buf, "%0.8f", TRecData[i].time);
                item.pszText = buf;
            } else {
                item.pszText = "";
            }
            pLC->SetItem(&item);
        }
        TRecDisp[i]._ts = TRecData[i]._ts;
    }

    if (TraceDispItems) {
        i = pLC->GetSelectionMark();
        if (i == -1) {
            i = 0;
        }
        if (i >= TraceDispItems) {
            i = TraceDispItems - 1;
        }
        pLC->SetSelectionMark(i);
    }

    pLC->SetRedraw(TRUE);

    pSB = (CScrollBar *)GetDlgItem(IDC_TRACE_RECSCROLL);
    if (TraceDispCnt > TRD_CNT) {
        if (TraceDispCnt > 1000) {
            items = 1000;
            ofs   = 1000 * TraceDispOfs / (TraceDispCnt - TRD_CNT);
        } else {
            items = TraceDispCnt - TRD_CNT;
            ofs   = TraceDispOfs;
        }
        pSB->SetScrollRange(0, items, FALSE);
        pSB->SetScrollPos(ofs);
        pSB->EnableScrollBar(ESB_ENABLE_BOTH);
    } else {
        pSB->EnableScrollBar(ESB_DISABLE_BOTH);
    }
}


BEGIN_MESSAGE_MAP(CTraceRec, CDialog)
ON_WM_CLOSE()
ON_WM_ACTIVATE()
ON_NOTIFY(LVN_KEYDOWN, IDC_TRACE_RECLIST, &CTraceRec::OnLvnKeydownTraceReclist)
ON_WM_VSCROLL()
ON_NOTIFY(NM_DBLCLK, IDC_TRACE_RECLIST, &CTraceRec::OnNMDblclkTraceReclist)
ON_WM_CONTEXTMENU()
ON_COMMAND(ID_TRACE_COUNTER, &CTraceRec::OnTraceCounter)
ON_UPDATE_COMMAND_UI(ID_TRACE_COUNTER, &CTraceRec::OnUpdateTraceCounter)
ON_COMMAND(ID_TRACE_DATAREAD, &CTraceRec::OnTraceDataread)
ON_UPDATE_COMMAND_UI(ID_TRACE_DATAREAD, &CTraceRec::OnUpdateTraceDataread)
ON_COMMAND(ID_TRACE_DATAWRITE, &CTraceRec::OnTraceDatawrite)
ON_UPDATE_COMMAND_UI(ID_TRACE_DATAWRITE, &CTraceRec::OnUpdateTraceDatawrite)
ON_COMMAND(ID_TRACE_EXCEPTION, &CTraceRec::OnTraceException)
ON_UPDATE_COMMAND_UI(ID_TRACE_EXCEPTION, &CTraceRec::OnUpdateTraceException)
ON_COMMAND(ID_TRACE_ITM, &CTraceRec::OnTraceItm)
ON_UPDATE_COMMAND_UI(ID_TRACE_ITM, &CTraceRec::OnUpdateTraceItm)
ON_COMMAND(ID_TRACE_PCSAMPLE, &CTraceRec::OnTracePcsample)
ON_UPDATE_COMMAND_UI(ID_TRACE_PCSAMPLE, &CTraceRec::OnUpdateTracePcsample)
END_MESSAGE_MAP()


// CTraceRec message handlers

void CTraceRec::PostNcDestroy()
{
    delete this; // delete the new'ed object
    pCTR = NULL; // clear external Object pointer here.
}

void CTraceRec::OnClose()
{
    GetWindowRect(&pMen->pDlg->rc); // save Window position
    pMen->pDlg->hw    = NULL;       // clear m_hWnd
    pMen->pDlg->iOpen = 0;          // flag window as NOT open
    DestroyWindow();                //--- modeless
}

void CTraceRec::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
    CDialog::OnActivate(nState, pWndOther, bMinimized);

    switch (nState) {
        case WA_INACTIVE:
            pio->curDlg = NULL; // Clear Modeless Handle
            break;
        case WA_ACTIVE:
        case WA_CLICKACTIVE:
            pio->curDlg = m_hWnd; // Set Modeless Handle
            break;
    }
}


static const char *trheader[] = {
    "Type", "Ovf", "Num", "Address", "Data", "PC", "Dly", "Cycles", "Time[s]"
};

static const int trdlgfmt[] = {
    LVCFMT_LEFT, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER
};

// These texts specify the maximum column width
static const char *trdummy[] = {
    "Exception Return",
    "Ovf",
    "8888",
    "DDDDDDDDH",
    "DDDDDDDDH",
    "DDDDDDDDH",
    "Dly",
    "8888888888888",
    "88888.88888888"
};


BOOL CTraceRec::OnInitDialog()
{
    CListCtrl * pLC;
    CScrollBar *pSB;
    LVCOLUMN    col;
    SCROLLINFO  info;
    int         i;

    CDialog::OnInitDialog();

    // Restore Position (Only moving without resizing)
    if (TR_Dlg.rc.left != -1) {
        SetWindowPos(NULL,                       /* placement order - not used */
                     TR_Dlg.rc.left,             /* left */
                     TR_Dlg.rc.top,              /* top  */
                     0,                          /* width - not used */
                     0,                          /* height - not used */
                     SWP_NOSIZE | SWP_NOZORDER); /* flags */
    }

    pLC = (CListCtrl *)GetDlgItem(IDC_TRACE_RECLIST);
    pLC->SetExtendedStyle(LVS_EX_FULLROWSELECT);

    // insert columns
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    for (i = 0; i < sizeof(trheader) / sizeof(char *); i++) {
        col.iSubItem = i;
        col.pszText  = (LPSTR)trheader[i]; // Value Column
        col.fmt      = trdlgfmt[i];        // Align columns
        col.cx       = pLC->GetStringWidth(trdummy[i]) + 13;
        pLC->InsertColumn(i, &col);
    }

    pSB = (CScrollBar *)GetDlgItem(IDC_TRACE_RECSCROLL);

    info.cbSize    = sizeof(SCROLLINFO);
    info.fMask     = SIF_ALL;
    info.nMin      = 0;
    info.nMax      = 0;
    info.nPage     = TRD_CNT - 1;
    info.nPos      = 0;
    info.nTrackPos = 0;
    pSB->SetScrollInfo(&info);
    pSB->EnableScrollBar(ESB_DISABLE_BOTH);

    TraceDispMask = (0 << TRD_UNKNOWN) | (1 << TRD_CNT_EVT) | (1 << TRD_EXC_EVT) | (1 << TRD_PC_SAMPLE) | (1 << TRD_DATA_READ) | (1 << TRD_DATA_WRITE) | (1 << TRD_SW_ITM);

    initflag = 1;
    Update();

    return TRUE; // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}


void CTraceRec::OnLvnKeydownTraceReclist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
    int           i          = ((CListCtrl *)GetDlgItem(IDC_TRACE_RECLIST))->GetSelectionMark();

    switch (pLVKeyDown->wVKey) {
        case VK_UP:
            i--;
            goto up;
        case VK_PRIOR:
            i -= TRD_CNT - 1;
        up:
            if (i < 0) {
                TraceDispOfs += i;
                if (TraceDispOfs < 0) {
                    TraceDispOfs = 0;
                }
                TraceDisp = 0x80000000;
                Update();
            }
            break;
        case VK_HOME:
            TraceDispOfs = 0;
            TraceDisp    = 0x80000000;
            Update();
            break;
        case VK_DOWN:
            i++;
            goto down;
        case VK_NEXT:
            i += TRD_CNT - 1;
        down:
            if (i >= TRD_CNT) {
                TraceDispOfs += i - (TRD_CNT - 1);
                if (TraceDispOfs > (TraceDispCnt - TRD_CNT)) {
                    TraceDispOfs = TraceDispCnt - TRD_CNT;
                    if (TraceDispOfs < 0) {
                        TraceDispOfs = 0;
                    }
                }
                TraceDisp = 0x80000000;
                Update();
            }
            break;
        case VK_END:
            TraceDispOfs = TraceDispCnt - TRD_CNT;
            if (TraceDispOfs < 0) {
                TraceDispOfs = 0;
            }
            TraceDisp = 0x80000000;
            Update();
            break;
    }

    *pResult = 0;
}


void CTraceRec::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    int pos;

    switch (nSBCode) {
        case SB_LINEUP:
            if (TraceDispOfs > 0) {
                TraceDispOfs--;
                TraceDisp = 0x80000000;
                Update();
            }
            break;
        case SB_LINEDOWN:
            if (TraceDispOfs < (TraceDispCnt - TRD_CNT)) {
                TraceDispOfs++;
                TraceDisp = 0x80000000;
                Update();
            }
            break;
        case SB_PAGEUP:
            TraceDispOfs -= TRD_CNT - 1;
            if (TraceDispOfs < 0) {
                TraceDispOfs = 0;
            }
            TraceDisp = 0x80000000;
            Update();
            break;
        case SB_PAGEDOWN:
            TraceDispOfs += TRD_CNT - 1;
            if (TraceDispOfs > (TraceDispCnt - TRD_CNT)) {
                TraceDispOfs = TraceDispCnt - TRD_CNT;
                if (TraceDispOfs < 0) {
                    TraceDispOfs = 0;
                }
            }
            TraceDisp = 0x80000000;
            Update();
            break;
        case SB_THUMBPOSITION:
            if (GoMode)
                goto pos;
            break;
        case SB_THUMBTRACK:
            if (GoMode)
                break;
        pos:
            if (TraceDispCnt > 1000) {
                pos = (nPos * (TraceDispCnt - TRD_CNT)) / 1000;
            } else {
                pos = nPos;
            }
            if (TraceDispOfs != pos) {
                TraceDispOfs = pos;
                TraceDisp    = 0x80000000;
                Update();
            }
            break;
    }

    //CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CTraceRec::OnNMDblclkTraceReclist(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;

    if (!(TraceConf.Opt & TRACE_ENABLE))
        return;

    EnterCriticalSection(&TraceCS);
    TraceHead = TraceTail = TraceDisp = 0;
    TraceHeadClock                    = TraceTailClock;
    TraceHeadCycles                   = TraceTailCycles;
    TraceHeadTime                     = TraceTailTime;
    LeaveCriticalSection(&TraceCS);
    Update();
}


void CTraceRec::OnContextMenu(CWnd *pWnd, CPoint point)
{
    CMenu menu, *men0;

    menu.LoadMenu(IDR_MENU_TRACE_REC);
    men0 = menu.GetSubMenu(0);
    if ((TraceDispMask >> TRD_CNT_EVT) & 1)
        men0->CheckMenuItem(ID_TRACE_COUNTER, MF_CHECKED);
    if ((TraceDispMask >> TRD_EXC_EVT) & 1)
        men0->CheckMenuItem(ID_TRACE_EXCEPTION, MF_CHECKED);
    if ((TraceDispMask >> TRD_PC_SAMPLE) & 1)
        men0->CheckMenuItem(ID_TRACE_PCSAMPLE, MF_CHECKED);
    if ((TraceDispMask >> TRD_SW_ITM) & 1)
        men0->CheckMenuItem(ID_TRACE_ITM, MF_CHECKED);
    if ((TraceDispMask >> TRD_DATA_READ) & 1)
        men0->CheckMenuItem(ID_TRACE_DATAREAD, MF_CHECKED);
    if ((TraceDispMask >> TRD_DATA_WRITE) & 1)
        men0->CheckMenuItem(ID_TRACE_DATAWRITE, MF_CHECKED);
    men0->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}


void CTraceRec::OnTraceCounter()
{
    TraceDispMask ^= 1 << TRD_CNT_EVT;
    Update();
}

void CTraceRec::OnUpdateTraceCounter(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TraceDispMask >> TRD_CNT_EVT) & 1);
}


void CTraceRec::OnTraceException()
{
    TraceDispMask ^= 1 << TRD_EXC_EVT;
    Update();
}

void CTraceRec::OnUpdateTraceException(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TraceDispMask >> TRD_EXC_EVT) & 1);
}


void CTraceRec::OnTracePcsample()
{
    TraceDispMask ^= 1 << TRD_PC_SAMPLE;
    Update();
}

void CTraceRec::OnUpdateTracePcsample(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TraceDispMask >> TRD_PC_SAMPLE) & 1);
}


void CTraceRec::OnTraceItm()
{
    TraceDispMask ^= 1 << TRD_SW_ITM;
    Update();
}

void CTraceRec::OnUpdateTraceItm(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TraceDispMask >> TRD_SW_ITM) & 1);
}


void CTraceRec::OnTraceDataread()
{
    TraceDispMask ^= 1 << TRD_DATA_READ;
    Update();
}

void CTraceRec::OnUpdateTraceDataread(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TraceDispMask >> TRD_DATA_READ) & 1);
}


void CTraceRec::OnTraceDatawrite()
{
    TraceDispMask ^= 1 << TRD_DATA_WRITE;
    Update();
}

void CTraceRec::OnUpdateTraceDatawrite(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TraceDispMask >> TRD_DATA_WRITE) & 1);
}


void CTraceRec::OnOK()
{
    // do nothing so far
}

void CTraceRec::OnCancel()
{
    OnClose();
}

void InitTraceRec()
{
    pCTR           = NULL;
    TraceDispMask  = 0; // Display Mask
    TraceDispOfs   = 0; // Item Offset
    TraceDispCnt   = 0; // Total Item Count
    TraceDispItems = 0; // Item Count (display area)

    memset(TRecData, 0, sizeof(TRecData));
    memset(TRecDisp, 0, sizeof(TRecDisp));
}
