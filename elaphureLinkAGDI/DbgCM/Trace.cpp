/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.18
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2009-2020 ARM Limited. All rights reserved.
 *
 * @brief     Trace Decoder
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
#define _IN_TARG_
#include "..\AGDI.h"
#include "..\BOM.h"
#include "Collect.h"
#include "Debug.h"
#include "Trace.h"
#include "SWV.h"
#include "ETB.h"
#include "TraceWinConnect.h"
#include "..\TraceView\TraceDataTypes.h"


// Trace Messages (Priority High -> Low)
// 05.11.2018: Show errors in status bar in red
char *TraceMsg[] = {
    "",
    "\\<!clrr>Trace: Communication Error",
    "\\<!clrr>Trace: HW Buffer Overrun",
    "\\<!clrr>Trace: SW Buffer Overrun",
    "\\<!clrr>Trace: No Synchronization",
    "\\<!clrr>Trace: Data Stream Error",
    "\\<!clrr>Trace: Data Overflow",
    "Trace: Running ...",
};

BYTE T_Msg;     // Trace Message
int  T_Err;     // Trace Error
int  T_Recover; // Trace Error Recover Time

// Trace Data
BYTE  TD_Buf[TD_CNT]; // Buffer
DWORD TD_Head;        // Head Pointer
DWORD TD_Tail;        // Tail Pointer
BYTE  TD_Byte;        // Byte

// Trace Packets
TP_ITEM TP_Buf[TP_CNT]; // Buffer
DWORD   TP_Head;        // Head Pointer
DWORD   TP_Tail;        // Tail Pointer
BYTE    TP_Idx;         // Index

// Trace Records
TR_ITEM TR_Buf[TR_CNT]; // Buffer
DWORD   TR_Head;        // Head Pointer
DWORD   TR_Tail;        // Tail Pointer
DWORD   TR_NoTS;        // No Timestamp Pointer


// Trace Buffer (Compressed)
BYTE  *TraceBuffer = NULL;
DWORD  TraceHead;
DWORD  TraceHeadClock;
I64    TraceHeadCycles;
double TraceHeadTime;
DWORD  TraceTail;
DWORD  TraceTailClock;
I64    TraceTailCycles;
double TraceTailTime;
DWORD  TraceDisp;

TR_EXC TraceExcData[EXC_NUM]; // Exception Trace Data

DWORD  TraceExcNum;  // Current Exception Number
double TraceExcTime; // Latched Exception Time

/* Event Counters (Count Overflows of 256) */
DWORD TraceCntCPI;   // CPI Overflow Counter
DWORD TraceCntExc;   // EXC Overflow Counter
DWORD TraceCntSleep; // Sleep Overflow Counter
DWORD TraceCntLSU;   // LSU Overflow Counter
DWORD TraceCntFold;  // Fold Overflow Counter


BOOL TraceDispFlg = FALSE; // Trace Display Flag
BOOL TraceInit    = FALSE; // Trace Init Flag

CRITICAL_SECTION TraceCS;


// Trace Configuration
struct TRACECONF TraceConf;

// Trace Options (Active TraceConf.Opt)
DWORD TraceOpt = 0;

BOOL T_Secure;    // Trace Security State (only if pio->bSecureExt)
BOOL TraceCycDwt; // DWT is Ref Count for Trace Cycles

// Trace Registers
static DWORD ITM_TraceControl; // ITM Trace Control

// Trace Variables
static DWORD TracePacketSync; // Trace Packet Synchronization
static BYTE  TracePacketID;   // Trace Packet ID
static BYTE  TracePacket[16]; // Trace Packet Data

static BOOL  TS_Active; // Timestamps Active
static BOOL  TS_Sync;   // Timestamps Synchronization
static BYTE  TS_Presc;  // Timestamps Prescaler
static INT64 TS_Cycles; // Timestamps Cycles

static int SyncPeriod;  // Synchronization Period in ms
static int SyncTimeout; // Synchronization Timeout in ms

static DWORD SampleCycles; // Sample Cycles (PC Sample or CYCCNT)

static DWORD TaskAddress;  // Task Address
static DWORD TaskLUT[256]; // Task Address Look-Up Table

static AGDI_LAREC lar; // Logic Analyzer Record

static DWORD MsgData[4]; // Message Data

static DCCMSG Message = {
    // DCC Message
    0x0004 /*RTAH_ROUTE_RTX*/ //  nTarg
};

// 02.04.2019: Separate Trace Clock setting - TPIU Clock (Active TraceConf.Clk/TraceConf.TPIUClk)
DWORD TPIU_Clock = 1;


// Trace Data: Get Byte
//   return : 1 - OK, 0 - No Data
static int TD_GetByte(void)
{
    if (TD_Tail != TD_Head) {
        TD_Byte = TD_Buf[TD_Tail++];
        if (TD_Tail == TD_CNT)
            TD_Tail = 0;
        return (1);
    }

    return (0);
}


// Trace Data: Set Byte
//   return : 1 - OK, 0 - Error
static int TD_SetByte(void)
{
    TD_Buf[TD_Head++] = TD_Byte;
    if (TD_Head == TD_CNT)
        TD_Head = 0;
    if (TD_Head == TD_Tail) {
        T_Err |= T_ERR_TD_BUF;
        return (0);
    }

    return (1);
}


// Trace Data: Read Trace Data
static void TD_Read(void)
{
    BYTE id, data, bit;
    int  cnt, n, m, i, j;

    if (TraceOpt & TPIU_FORMAT) {
        //---TODO: Add ETM decode path starting with separating ETM and ITM data here
        //   DEVELOP_MSG("..."); // Called at ETM setup of ETM enabled

        // TPIU Formatter
        cnt = SWV_DataHead - SWV_DataTail;
        if (cnt < 0)
            cnt += SWV_DATACNT;
        n = cnt / 16;
        m = cnt % 16;
        for (i = 0; i < n; i++) {
            // Process 16-byte packets
            for (j = 0; j < 16; j++) {
                data = SWV_DataBuf[SWV_DataTail++];
                if (SWV_DataTail == SWV_DATACNT)
                    SWV_DataTail = 0;
                TracePacketSync >>= 8;
                TracePacketSync |= data << 24;
                if (TracePacketSync == 0x7FFFFFFF) {
                    // Trace Packet Synchronization
                    m -= j + 1;
                    if (m < 0) {
                        m += 16;
                        n--;
                        if (i == n)
                            return;
                    }
                    j = -1;
                    continue;
                }
                TracePacket[j] = data;
            }
            for (j = 0; j < 16; j += 2) {
                bit = (TracePacket[15] >> (j >> 1)) & 1;
                if (TracePacket[j] & 1) {
                    // Even Byte is ID
                    id            = TracePacketID;
                    TracePacketID = TracePacket[j] >> 1;
                    if (j == 14)
                        break;
                    if (bit == 0) {
                        id = TracePacketID;
                    }
                    if (id == ITM_ATBID) {
                        TD_Byte = TracePacket[j + 1];
                        TD_SetByte();
                    }
                } else {
                    // Even Byte is Data
                    if (TracePacketID == ITM_ATBID) {
                        TD_Byte = (TracePacket[j] & 0xFE) | bit;
                        TD_SetByte();
                        if (j == 14)
                            break;
                        TD_Byte = TracePacket[j + 1];
                        TD_SetByte();
                    }
                }
            }
        }
    } else {
        while (SWV_DataTail != SWV_DataHead) {
            TD_Byte = SWV_DataBuf[SWV_DataTail++];
            if (SWV_DataTail == SWV_DATACNT)
                SWV_DataTail = 0;
            TD_SetByte();
        }
    }
}


// Trace Packet: Get Item
//   return : Pointer to Item
static TP_ITEM_PTR TP_GetItem(void)
{
    TP_ITEM *tp;

    if (TP_Tail != TP_Head) {
        tp = &TP_Buf[TP_Tail];
        TP_Tail++;
        if (TP_Tail == TP_CNT)
            TP_Tail = 0;
        return (tp);
    }

    return (NULL);
}

// Trace Packet: Save Item
//   return : Pointer to next Item
static TP_ITEM_PTR TP_SaveItem(void)
{
    TP_ITEM *tp;

    TP_Head++;
    if (TP_Head == TP_CNT)
        TP_Head = 0;
    if (TP_Head == TP_Tail)
        T_Err |= T_ERR_TP_BUF;
    tp       = &TP_Buf[TP_Head];
    tp->type = TP_IDLE;
    return (tp);
}

// Trace Packet: Reject Item
static void TP_RejectItem(void)
{
    TP_ITEM *tp;

    T_Err |= T_ERR_TD_ERR;
    tp       = &TP_Buf[TP_Head];
    tp->type = TP_IDLE;
}


// Trace Packet: Read Data from Trace Data
static void TP_Read(void)
{
    TP_ITEM *tp;

    tp = &TP_Buf[TP_Head];

s:
    while (TD_GetByte()) {
        switch (tp->type) {
            case TP_IDLE:
                tp->ovfl = 0;
            case TP_OVFL:
                if (TD_Byte == 0x70) {
                    tp->type = TP_OVFL;
                    tp->ovfl = 1;
                    T_Err |= T_ERR_DATA_OVF;
                    goto s;
                }
                TP_Idx = 1;
                if (TD_Byte == 0x00) {
                    tp->type = TP_SYNC;
                    tp->size = 6;
                    goto s;
                }
                if ((TD_Byte & 0x03) == 0x00) {
                    switch ((TD_Byte >> 2) & 0x03) {
                        case 0x00:
                            tp->type = TP_TIME;
                            tp->ctrl = (TD_Byte >> 4) & 0x07;
                            tp->data = 0;
                            break;
                        case 0x01:
                            tp->type = TP_RSRVD;
                            tp->data = (TD_Byte >> 4) & 0x07;
                            break;
                        case 0x02:
                            tp->type = TP_SWEXT;
                            tp->data = (TD_Byte >> 4) & 0x07;
                            break;
                        case 0x03:
                            tp->type = TP_HWEXT;
                            tp->data = (TD_Byte >> 4) & 0x07;
                            break;
                    }
                    if (TD_Byte & 0x80) {
                        tp->size = 2;
                    } else {
                        tp->size = 1;
                        tp       = TP_SaveItem();
                    }
                    goto s;
                }
                tp->size = 1 << ((TD_Byte & 0x03) - 1);
                tp->addr = (TD_Byte >> 3) & 0x1F;
                tp->data = 0;
                if (TD_Byte & 0x04) {
                    tp->type = TP_HWIT;
                    switch ((TD_Byte >> 6) & 0x03) {
                        case 0x00:
                            switch ((TD_Byte >> 3) & 0x07) {
                                case 0x00:
                                    tp->hwid = TP_EVT;
                                    break;
                                case 0x01:
                                    tp->hwid = TP_EVT_EXTTRC;
                                    break;
                                case 0x02:
                                    tp->hwid = TP_EVT_PC;
                                    break;
                                case 0x03: // PMU Event (8-bit, v8.1-M and later, no PMU support in uVision)
                                default:
                                    tp->hwid = TP_NOHW;
                                    break;
                            }
                            break;
                        case 0x01:
                            tp->hwid = (TD_Byte & 0x08) ? TP_ADDR16 : TP_PC32; // v8-M: Data Address (only for Data Address Range) vs. PC or Match Packet
                                                                               //   v7-M enum types match according v8-M enum types
                                                                               //   (TP_ADDR16 == TP_DATA_ADDR)
                                                                               //   (TP_PC32   == TP_PC_OR_MATCH)
                            tp->ctrl = (TD_Byte >> 4) & 0x03;
                            break;
                        case 0x02:
                            tp->hwid = (TD_Byte & 0x08) ? TP_DATA_WRITE : TP_DATA_READ;
                            tp->ctrl = (TD_Byte >> 4) & 0x03;
                            break;
                        case 0x03:
                            tp->hwid = TP_NOHW;
                            break;
                    }
                } else {
                    tp->type = TP_SWIT;
                }
                break;
            case TP_SYNC:
                TP_Idx++;
                if (TP_Idx < 6) {
                    if (TD_Byte != 0x00) {
                        TP_RejectItem();
                    }
                } else {
                    if (TD_Byte != 0x80) {
                        TP_RejectItem();
                    } else {
                        tp = TP_SaveItem();
                    }
                }
                break;
            case TP_TIME:
                TP_Idx++;
                if (TD_Byte & 0x80) {
                    tp->size++;
                    if (tp->size > 5) {
                        TP_RejectItem();
                        break;
                    }
                }
                tp->data |= (TD_Byte & 0x7F) << (7 * (TP_Idx - 2));
                if (TP_Idx == tp->size) {
                    tp = TP_SaveItem();
                }
                break;
            case TP_SWIT:
            case TP_HWIT:
                TP_Idx++;
                tp->data |= TD_Byte << (8 * (TP_Idx - 2));
                if (TP_Idx > tp->size) {
                    tp = TP_SaveItem();
                }
                break;
            case TP_SWEXT:
            case TP_HWEXT:
            case TP_RSRVD:
                TP_Idx++;
                if (TD_Byte & 0x80) {
                    tp->size++;
                    if (tp->size > 5) {
                        TP_RejectItem();
                        break;
                    }
                }
                tp->data |= (TD_Byte & 0x7F) << (3 + 7 * (TP_Idx - 2));
                if (TP_Idx == tp->size) {
                    tp = TP_SaveItem();
                }
                break;
        }
    }
}


// Trace Packet: Debug Print
static void TP_DbgPrint(void)
{
    TP_ITEM *tp;
    int      i;
    char     buf[256];

    while (tp = TP_GetItem()) {
        switch (tp->type) {
            case TP_SYNC:
                i = sprintf(buf, "SYNC:  S=6");
                break;
            case TP_TIME:
                i = sprintf(buf, "TIME:  ");
                i += sprintf(&buf[i], "S=%d T=%08X C=%d", tp->size, tp->data, tp->ctrl);
                break;
            case TP_SWIT:
                i = sprintf(buf, "SWIT:  ");
                i += sprintf(&buf[i], "S=%d D=%08X A=%02X", tp->size, tp->data, tp->addr);
                break;
            case TP_HWIT:
                i = sprintf(buf, "HWIT:  ");
                i += sprintf(&buf[i], "S=%d D=%08X", tp->size, tp->data);
                switch (tp->hwid) {
                    case TP_NOHW:
                        i += sprintf(&buf[i], " RSRVD");
                        break;
                    case TP_EVT:
                        i += sprintf(&buf[i], " EVT");
                        break;
                    case TP_EVT_EXTTRC:
                        i += sprintf(&buf[i], " EVT EXTTRC");
                        break;
                    case TP_EVT_PC:
                        i += sprintf(&buf[i], " EVT PC");
                        break;
                    case TP_PC32: // TP_PC_OR_MATCH for v8-M
                        if (IsV8M()) {
                            if (tp->data & DWTv8_MATCH_N_PC) {
                                i += sprintf(&buf[i], " MATCH%d", tp->ctrl);
                            } else {
                                i += sprintf(&buf[i], " PC%d", tp->ctrl);
                            }
                        } else {
                            i += sprintf(&buf[i], " PC");
                        }
                        break;
                    case TP_ADDR16: // TP_DATA_ADDR for v8-M
                        if (IsV8M()) {
                            i += sprintf(&buf[i], " DADDR%d", tp->ctrl);
                        } else {
                            i += sprintf(&buf[i], " A16");
                        }
                        break;
                    case TP_DATA_READ:
                        i += sprintf(&buf[i], " RD%d", tp->ctrl);
                        break;
                    case TP_DATA_WRITE:
                        i += sprintf(&buf[i], " WR%d", tp->ctrl);
                        break;
                }
                break;
            case TP_SWEXT:
                i = sprintf(buf, "SWEXT: ");
                i += sprintf(&buf[i], "S=%d D=%08X", tp->size, tp->data);
                break;
            case TP_HWEXT:
                i = sprintf(buf, "HWEXT: ");
                i += sprintf(&buf[i], "S=%d D=%08X", tp->size, tp->data);
                break;
            case TP_RSRVD:
                i = sprintf(buf, "RSRVD: ");
                i += sprintf(&buf[i], "S=%d D=%08X", tp->size, tp->data);
                break;
            default:
                i = sprintf(buf, "ERR");
                break;
        }
        if (tp->ovfl) {
            i += sprintf(&buf[i], " OVFL");
        }
        i += sprintf(&buf[i], "\n");
        AG_Serial(AG_SERBOUT, 3, i, buf);
    }
}


// Trace Record: Get Item
//   return : Pointer to Item
static TR_ITEM_PTR TR_GetItem(void)
{
    TR_ITEM *tr;

    if ((TR_Tail != TR_Head) && (TR_Tail != TR_NoTS)) {
        tr = &TR_Buf[TR_Tail];
        TR_Tail++;
        if (TR_Tail == TR_CNT)
            TR_Tail = 0;
        return (tr);
    }

    return (NULL);
}

// Trace Record: Save Item
//   return : Pointer to next Item
static TR_ITEM_PTR TR_SaveItem(void)
{
    TR_ITEM *tr;

    if (TS_Active) {
        if (TraceOpt & TRACE_TIMESTAMP) {
            if (TR_NoTS == -1) {
                TR_NoTS = TR_Head;
            } else if (((TR_Head - TR_NoTS) & (TR_CNT - 1)) > 100) {
                // No Timestap for more than 100 records (Data Overflow)
                TR_NoTS = -1;
            }
        }
    } else {
        tr       = &TR_Buf[TR_Head];
        tr->tcyc = RegARM.nCycles;
        tr->flag |= TR_TS_VALID | TR_DP_DELAY;
    }

    TR_Head++;
    if (TR_Head == TR_CNT)
        TR_Head = 0;
    if (TR_Head == TR_Tail)
        T_Err |= T_ERR_TR_BUF;
    tr       = &TR_Buf[TR_Head];
    tr->flag = 0;
    return (tr);
}

// Trace Record: Process Timestamp
//   diff   : cycle difference
//   delay  : delayed timestamp
static void TR_Timestamp(DWORD diff, BYTE delay)
{
    TR_ITEM *tr;
    BYTE     flag;

    if (TS_Sync) {
        diff    = 0;
        TS_Sync = FALSE;
    }

    diff *= TS_Presc;

    TS_Cycles += diff + 1;

    if (TR_NoTS == -1)
        return;

    flag = TR_TS_VALID;
    if (delay & 0x01)
        flag |= TR_TS_DELAY;
    if (delay & 0x02)
        flag |= TR_DP_DELAY;

    while (TR_NoTS != TR_Head) {
        tr       = &TR_Buf[TR_NoTS];
        tr->tcyc = TS_Cycles;
        tr->flag |= flag;
        TR_NoTS++;
        if (TR_NoTS == TR_CNT)
            TR_NoTS = 0;
    }
    TR_NoTS = -1;
}


// Trace Record: Read Data from Trace Packets
static void TR_Read(void)
{
    TP_ITEM *tp;
    TR_ITEM *tr;

    tr = &TR_Buf[TR_Head];

    while (tp = TP_GetItem()) {
        switch (tp->type) {
            case TP_SYNC:
                SyncTimeout = SyncPeriod;
                break;
            case TP_TIME:
                if (tp->size == 1) {
                    TR_Timestamp(tp->ctrl, 0);
                } else {
                    TR_Timestamp(tp->data, tp->ctrl & 0x03);
                }
                break;
            case TP_SWIT:
                tr->type = TR_ITM;
                tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                tr->size = tp->size;
                tr->addr = tp->addr;
                tr->data = tp->data;
                tr       = TR_SaveItem();
                break;
            case TP_HWIT:
                switch (tp->hwid) {
                    case TP_NOHW:
                        break;
                    case TP_EVT:
                        tr->type = TR_EVT;
                        tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        tr->size = tp->size;
                        tr->data = tp->data & ~0x20; /* Mask out CYCEVT */
                        tr       = TR_SaveItem();
                        break;
                    case TP_EVT_EXTTRC:
                        tr->type = TR_EVT_EXTTRC;
                        tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        tr->size = tp->size;
                        tr->data = tp->data;
                        tr       = TR_SaveItem();
                        break;
                    case TP_EVT_PC:
                        tr->type = TR_EVT_PC;
                        tr->flag = tp->ovfl ? TR_OVERFLOW | TR_PC_VALID : TR_PC_VALID;
                        tr->nPC  = tp->data;
                        if (!(TraceOpt & TRACE_TIMESTAMP)) {
                            TS_Cycles += SampleCycles;
                            tr->flag |= TR_TS_VALID;
                            tr->tcyc = TS_Cycles;
                        }
                        tr = TR_SaveItem();
                        break;
                    case TP_PC32:
                        tr->flag = tp->ovfl ? TR_OVERFLOW | TR_PC_VALID : TR_PC_VALID;
                        tr->nPC  = tp->data;
                        break;
                    case TP_ADDR16:
                        tr->flag = tp->ovfl ? TR_OVERFLOW | TR_ADR_VALID : TR_ADR_VALID;
                        tr->addr = (RegDWT.CMP[tp->ctrl].COMP & 0xFFFF0000) | (tp->data & 0xFFFF);
                        break;
                    case TP_DATA_READ:
                        tr->type = TR_DATA_READ;
                        if (tr->flag) {
                            tr->flag |= tp->ovfl ? TR_OVERFLOW : 0;
                        } else {
                            tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        }
                        if (!(tr->flag & TR_ADR_VALID)) {
                            tr->flag |= TR_ADR_VALID;
                            tr->addr = RegDWT.CMP[tp->ctrl].COMP;
                        }
                        tr->size = tp->size;
                        tr->data = tp->data;
                        tr       = TR_SaveItem();
                        break;
                    case TP_DATA_WRITE:
                        tr->type = TR_DATA_WRITE;
                        if (tr->flag) {
                            tr->flag |= tp->ovfl ? TR_OVERFLOW : 0;
                        } else {
                            tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        }
                        if (!(tr->flag & TR_ADR_VALID)) {
                            tr->flag |= TR_ADR_VALID;
                            tr->addr = RegDWT.CMP[tp->ctrl].COMP;
                        }
                        tr->size = tp->size;
                        tr->data = tp->data;
                        tr       = TR_SaveItem();
                        break;
                }
                break;
            case TP_SWEXT:
            case TP_HWEXT:
            case TP_RSRVD:
            default:
                break;
        }
    }
}


// Trace Record: Read Data from Trace Packets (v8-M version)
static void TR_Read_v8M(void)
{
    TP_ITEM *tp;
    TR_ITEM *tr;
    DWORD    matchval;

    tr = &TR_Buf[TR_Head];

    while (tp = TP_GetItem()) {
        switch (tp->type) {
            case TP_SYNC:
                SyncTimeout = SyncPeriod;
                break;
            case TP_TIME:
                if (tp->size == 1) {
                    TR_Timestamp(tp->ctrl, 0);
                } else {
                    TR_Timestamp(tp->data, tp->ctrl & 0x03);
                }
                break;
            case TP_SWIT:
                tr->type = TR_ITM;
                tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                tr->size = tp->size;
                tr->addr = tp->addr;
                tr->data = tp->data;
                tr       = TR_SaveItem();
                break;
            case TP_HWIT:
                switch (tp->hwid) {
                    case TP_NOHW:
                        break;
                    case TP_EVT:
                        tr->type = TR_EVT;
                        tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        tr->size = tp->size;
                        tr->data = tp->data & ~0x20; /* Mask out CYCEVT */
                        tr       = TR_SaveItem();
                        break;
                    case TP_EVT_EXTTRC:
                        tr->type = TR_EVT_EXTTRC;
                        tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        tr->size = tp->size;
                        tr->data = tp->data;
                        tr       = TR_SaveItem();
                        break;
                    case TP_EVT_PC:
                        tr->type = TR_EVT_PC;
                        tr->flag = tp->ovfl ? TR_OVERFLOW | TR_PC_VALID : TR_PC_VALID;
                        tr->nPC  = tp->data;
                        if (!(TraceOpt & TRACE_TIMESTAMP)) {
                            TS_Cycles += SampleCycles;
                            tr->flag |= TR_TS_VALID;
                            tr->tcyc = TS_Cycles;
                        }
                        tr = TR_SaveItem();
                        break;
                    case TP_PC_OR_MATCH:
                        if (tp->data & DWTv8_MATCH_N_PC) { // Match Packet
                            tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                            switch (RegDWT.CMP[tp->ctrl].FUNC & DWTv8_MATCH) {
                                case DWTv8_CYCLE:
                                    // Not supported
                                    break;
                                case DWTv8_IADDR:
                                    tr->flag |= TR_PC_VALID;
                                    tr->nPC = RegDWT.CMP[tp->ctrl].COMP;
                                    break;
                                case DWTv8_DADDR_RW:
                                case DWTv8_DADDR_R:
                                case DWTv8_DADDR_W:
                                    tr->flag |= TR_ADR_VALID;
                                    tr->addr = RegDWT.CMP[tp->ctrl].COMP;
                                    tr->type = ((RegDWT.CMP[tp->ctrl].FUNC & DWTv8_MATCH) == DWTv8_DADDR_R) ? TR_DATA_READ : TR_DATA_WRITE;
                                    tr->size = 2 << ((RegDWT.CMP[tp->ctrl].FUNC & DWTv8_DATAVSIZE) >> DWTv8_DATAVSIZE_P);
                                    if (tp->ctrl < NumWP - 1) {
                                        if ((RegDWT.CMP[tp->ctrl + 1].FUNC & DWTv8_MATCH) == DWTv8_DVALUE_LINK) {
                                            switch ((RegDWT.CMP[tp->ctrl + 1].FUNC & DWTv8_DATAVSIZE) >> DWTv8_DATAVSIZE_P) {
                                                case DWTv8_DATAVSIZEB:
                                                    tr->data = RegDWT.CMP[tp->ctrl + 1].COMP & 0x00FF;
                                                    break;
                                                case DWTv8_DATAVSIZEH:
                                                    tr->data = RegDWT.CMP[tp->ctrl + 1].COMP & 0xFFFF;
                                                    break;
                                                case DWTv8_DATAVSIZEW:
                                                    tr->data = RegDWT.CMP[tp->ctrl + 1].COMP;
                                                    break;
                                            }
                                        }
                                    }
                                    break;
                                case DWTv8_DVALUE_RW:
                                case DWTv8_DVALUE_W:
                                case DWTv8_DVALUE_R:
                                    // Not supported at the moment (no ITM_TR_XXX_VALID flag defined for data only)
                                    tr->type = ((RegDWT.CMP[tp->ctrl].FUNC & DWTv8_MATCH) == DWTv8_DVALUE_R) ? TR_DATA_READ : TR_DATA_WRITE;
                                    tr->size = 2 << ((RegDWT.CMP[tp->ctrl].FUNC & DWTv8_DATAVSIZE) >> DWTv8_DATAVSIZE_P);
                                    switch (tr->size) {
                                        case 1:
                                            tr->data = RegDWT.CMP[tp->ctrl].COMP & 0x00FF;
                                            break;
                                        case 2:
                                            tr->data = RegDWT.CMP[tp->ctrl].COMP & 0xFFFF;
                                            break;
                                        case 4:
                                            tr->data = RegDWT.CMP[tp->ctrl].COMP;
                                            break;
                                    }
                                    break;
                            }
                            // tr->flag = tp->ovfl ? TR_OVERFLOW | TR_PC_VALID : TR_PC_VALID;
                            // tr->nPC  = tp->data;
                        } else {                                   // PC Value
                                                                   // MATCH - Cycle Counter: Not supported
                                                                   // MATCH - Instruction Addr: All information in trace packet
                                                                   // MATCH - Data Addr: Read data address from this comparator
                                                                   // MATCH - Data Addr + Value: Read data address from this comparator
                            tr->flag = tp->ovfl ? TR_OVERFLOW : 0; // | TR_PC_VALID : TR_PC_VALID;
                            switch (RegDWT.CMP[tp->ctrl].FUNC & DWTv8_MATCH) {
                                case DWTv8_IADDR:
                                    tr->flag |= TR_PC_VALID;
                                    switch (tp->size) {
                                        case 1:
                                            tr->nPC = (RegDWT.CMP[tp->ctrl].COMP & 0xFFFFFF00) | (tp->data & 0x00FF);
                                            break;
                                        case 2:
                                            tr->nPC = (RegDWT.CMP[tp->ctrl].COMP & 0xFFFF0000) | (tp->data & 0xFFFF);
                                            break;
                                        case 4:
                                            tr->nPC = tp->data;
                                            break;
                                    }
                                    break;
                                case DWTv8_DADDR_RW:
                                case DWTv8_DADDR_W:
                                case DWTv8_DADDR_R:
                                case DWTv8_DADDR_VAL_RW:
                                case DWTv8_DADDR_VAL_W:
                                case DWTv8_DADDR_VAL_R:
                                    tr->flag |= TR_PC_VALID | TR_ADR_VALID;
                                    tr->addr = RegDWT.CMP[tp->ctrl].COMP;
                                    tr->nPC  = tp->data;
                                    break;
                            }
                            tr->nPC = tp->data;
                        }
                        break;
                    case TP_DATA_ADDR:
                        tr->flag = tp->ovfl ? TR_OVERFLOW | TR_ADR_VALID : TR_ADR_VALID;
                        switch (tp->size) {
                            case 1:
                                tr->addr = (RegDWT.CMP[tp->ctrl].COMP & 0xFFFFFF00) | (tp->data & 0x00FF);
                                break;
                            case 2:
                                tr->addr = (RegDWT.CMP[tp->ctrl].COMP & 0xFFFF0000) | (tp->data & 0xFFFF);
                                break;
                            case 4:
                                tr->addr = tp->data;
                                break;
                        }
                        break;
                    case TP_DATA_READ:
                    case TP_DATA_WRITE:
                        tr->type = (tp->hwid == TR_DATA_READ) ? TR_DATA_READ : TR_DATA_WRITE;
                        if (tr->flag) {
                            tr->flag |= tp->ovfl ? TR_OVERFLOW : 0;
                        } else {
                            tr->flag = tp->ovfl ? TR_OVERFLOW : 0;
                        }
                        if (!(tr->flag & TR_ADR_VALID)) {
                            switch (RegDWT.CMP[tp->ctrl].FUNC & DWTv8_MATCH) {
                                case DWTv8_DVALUE_LINK:
                                    if (tp->ctrl > 0) {
                                        matchval = RegDWT.CMP[tp->ctrl - 1].FUNC & DWTv8_MATCH;
                                        if ((matchval & 0x0C) == DWTv8_DADDR_RW && matchval != DWTv8_DADDR_LIMIT) { // Data Address (R, W, R/W), RW variant has lower two bits 0?
                                            tr->flag |= TR_ADR_VALID;
                                            tr->addr = RegDWT.CMP[tp->ctrl - 1].COMP;
                                        }
                                    }
                                    break;
                                case DWTv8_DADDR_VAL_RW:
                                case DWTv8_DADDR_VAL_W:
                                case DWTv8_DADDR_VAL_R:
                                    tr->flag |= TR_ADR_VALID;
                                    tr->addr = RegDWT.CMP[tp->ctrl].COMP;
                                    break;
                            }
                        }
                        tr->size = tp->size;
                        tr->data = tp->data;
                        tr       = TR_SaveItem();
                        break;
                }
                break;
            case TP_SWEXT:
            case TP_HWEXT:
            case TP_RSRVD:
            default:
                break;
        }
    }
}


// Trace Record: Debug Print
static void TR_DbgPrint(void)
{
    TR_ITEM *tr;
    int      i;
    char     buf[256];

    while (tr = TR_GetItem()) {
        switch (tr->type) {
            case TR_ITM:
                i = sprintf(buf, "ITM: ");
                i += sprintf(&buf[i], "S=%d A=%02X D=%0*X", tr->size, tr->addr,
                             tr->size * 2, tr->data);
                break;
            case TR_EVT:
                i = sprintf(buf, "EVT: ");
                i += sprintf(&buf[i], "D=%02X", tr->data);
                break;
            case TR_EVT_EXTTRC:
                i = sprintf(buf, "EVT: EXTTRC");
                i += sprintf(&buf[i], "D=%04X", tr->data);
                break;
            case TR_EVT_PC:
                i = sprintf(buf, "EVT: ");
                break;
            case TR_DATA_READ:
                i = sprintf(buf, "RD:  ");
                i += sprintf(&buf[i], "S=%d A=%08X D=%08X", tr->size, tr->addr, tr->data);
                break;
            case TR_DATA_WRITE:
                i = sprintf(buf, "WR:  ");
                i += sprintf(&buf[i], "S=%d A=%08X D=%08X", tr->size, tr->addr, tr->data);
                break;
            default:
                i = sprintf(buf, "ERR: ");
                break;
        }
        if (tr->flag & TR_PC_VALID) {
            i += sprintf(&buf[i], " PC=%08X", tr->nPC);
        }
        if (tr->flag & TR_TS_VALID) {
            i += sprintf(&buf[i], " T=%I64u", tr->tcyc);
            if (tr->flag & TR_TS_DELAY) {
                i += sprintf(&buf[i], " DLY");
            }
        }
        if (tr->flag & TR_OVERFLOW) {
            i += sprintf(&buf[i], " OVF");
        }
        i += sprintf(&buf[i], "\n");
        AG_Serial(AG_SERBOUT, 3, i, buf);
    }
}


// Trace Record: Save to Trace Buffer (Compressed)
/*__inline static*/ void TR_Save(TR_ITEM *tr)
{
    BYTE flag;
    int  diff;

    // Ensure enough Memory (delete oldest records if necessary)
    // Twice the size (possible padding on buffer rollover) of max. data (< 48)
    diff = TraceTail - TraceHead;
    if (diff < 0)
        diff += TB_SIZE;
    while (diff > (TB_SIZE - 96)) {
        if (TraceDisp == TraceHead)
            TraceDisp = 0xFFFFFFFF;
        flag = *((BYTE *)(TraceBuffer + TraceHead));
        TraceHead += sizeof(BYTE);
        if (flag == TB_TIMEINFO) {
            TraceHeadClock = *((DWORD *)(TraceBuffer + TraceHead));
            TraceHead += sizeof(DWORD);
            TraceHeadCycles = *((I64 *)(TraceBuffer + TraceHead));
            TraceHead += sizeof(I64);
            TraceHeadTime = *((double *)(TraceBuffer + TraceHead));
            TraceHead += sizeof(double);
        } else if (flag != TB_PADDING) {
            if (flag & TB_PC_VALID)
                TraceHead += sizeof(DWORD);
            if (flag & TB_ADR_VALID) {
                if ((flag & TB_MASK) == TB_ITM) {
                    TraceHead += sizeof(BYTE);
                } else {
                    TraceHead += sizeof(DWORD);
                }
            }
            switch (flag & TB_DATA_SZ) {
                case TB_DATA_SZ8:
                    TraceHead += sizeof(BYTE);
                    break;
                case TB_DATA_SZ16:
                    TraceHead += sizeof(WORD);
                    break;
                case TB_DATA_SZ32:
                    TraceHead += sizeof(DWORD);
                    break;
            }
            if (flag & TB_TS_VALID)
                TraceHead += sizeof(DWORD);
        }
        if (TraceHead == TB_SIZE)
            TraceHead = 0;
        diff = TraceTail - TraceHead;
        if (diff < 0)
            diff += TB_SIZE;
    }

    if (tr == NULL) {
        // No Record Data - check if Clock has changed
        if (TraceTailClock != TraceConf.Clk) {
            TraceTailTime += (double)(RegARM.nCycles - TraceTailCycles) / TraceTailClock;
            TraceTailCycles                      = RegARM.nCycles;
            TraceTailClock                       = TraceConf.Clk;
            *((BYTE *)(TraceBuffer + TraceTail)) = TB_TIMEINFO;
            TraceTail += sizeof(BYTE);
            *((DWORD *)(TraceBuffer + TraceTail)) = TraceTailClock;
            TraceTail += sizeof(DWORD);
            *((I64 *)(TraceBuffer + TraceTail)) = TraceTailCycles;
            TraceTail += sizeof(I64);
            *((double *)(TraceBuffer + TraceTail)) = TraceTailTime;
            TraceTail += sizeof(double);
        }
        goto padding;
    }

    // Save Time Info when cycle difference is out of range
    if (tr->flag & TR_TS_VALID) {
        if ((tr->tcyc - TraceTailCycles) > 0x7FFFFFFF) {
            TraceTailTime += (double)(tr->tcyc - TraceTailCycles) / TraceTailClock;
            TraceTailCycles                      = tr->tcyc;
            *((BYTE *)(TraceBuffer + TraceTail)) = TB_TIMEINFO;
            TraceTail += sizeof(BYTE);
            *((DWORD *)(TraceBuffer + TraceTail)) = TraceTailClock;
            TraceTail += sizeof(DWORD);
            *((I64 *)(TraceBuffer + TraceTail)) = TraceTailCycles;
            TraceTail += sizeof(I64);
            *((double *)(TraceBuffer + TraceTail)) = TraceTailTime;
            TraceTail += sizeof(double);
        }
    }

    // Setup Record Info
    flag = 0;
    if (tr->flag & TR_TS_VALID)
        flag |= TB_TS_VALID;
    if (tr->flag & TR_OVERFLOW)
        flag |= TB_OVERFLOW;
    switch (tr->type) {
        case TR_ITM:
            flag |= TB_ITM | TB_ADR_VALID;
            switch (tr->size) {
                case 1: flag |= TB_DATA_SZ8; break;
                case 2: flag |= TB_DATA_SZ16; break;
                case 4: flag |= TB_DATA_SZ32; break;
            }
            break;
        case TR_EVT:
            if (tr->data == 0)
                goto padding;
        case TR_EVT_EXTTRC:
            flag |= TB_EVENT;
            switch (tr->size) {
                case 1: flag |= TB_DATA_SZ8; break;
                case 2: flag |= TB_DATA_SZ16; break;
                case 4: flag |= TB_DATA_SZ32; break;
            }
            break;
        case TR_EVT_PC:
            flag |= TB_EVENT | TB_PC_VALID;
            break;
        case TR_DATA_READ:
            flag |= TB_DATA_RD;
            if (tr->flag & TR_ADR_VALID)
                flag |= TB_ADR_VALID;
            if (tr->flag & TR_PC_VALID)
                flag |= TB_PC_VALID;
            switch (tr->size) {
                case 1: flag |= TB_DATA_SZ8; break;
                case 2: flag |= TB_DATA_SZ16; break;
                case 4: flag |= TB_DATA_SZ32; break;
            }
            break;
        case TR_DATA_WRITE:
            flag |= TB_DATA_WR;
            if (tr->flag & TR_ADR_VALID)
                flag |= TB_ADR_VALID;
            if (tr->flag & TR_PC_VALID)
                flag |= TB_PC_VALID;
            switch (tr->size) {
                case 1: flag |= TB_DATA_SZ8; break;
                case 2: flag |= TB_DATA_SZ16; break;
                case 4: flag |= TB_DATA_SZ32; break;
            }
            break;
    }

    // Save Record to Trace Buffer
    *((BYTE *)(TraceBuffer + TraceTail)) = flag;
    TraceTail += sizeof(BYTE);
    if (flag & TB_PC_VALID) {
        *((DWORD *)(TraceBuffer + TraceTail)) = tr->nPC;
        TraceTail += sizeof(DWORD);
    }
    if (flag & TB_ADR_VALID) {
        if ((flag & TB_MASK) == TB_ITM) {
            *((DWORD *)(TraceBuffer + TraceTail)) = (BYTE)tr->addr;
            TraceTail += sizeof(BYTE);
        } else {
            *((DWORD *)(TraceBuffer + TraceTail)) = tr->addr;
            TraceTail += sizeof(DWORD);
        }
    }
    switch (flag & TB_DATA_SZ) {
        case TB_DATA_SZ8:
            *((BYTE *)(TraceBuffer + TraceTail)) = (BYTE)tr->data;
            TraceTail += sizeof(BYTE);
            break;
        case TB_DATA_SZ16:
            *((WORD *)(TraceBuffer + TraceTail)) = (WORD)tr->data;
            TraceTail += sizeof(WORD);
            break;
        case TB_DATA_SZ32:
            *((DWORD *)(TraceBuffer + TraceTail)) = tr->data;
            TraceTail += sizeof(DWORD);
            break;
    }
    if (flag & TB_TS_VALID) {
        diff = (int)(tr->tcyc - TraceTailCycles);
        if (tr->flag & (TR_TS_DELAY | TR_DP_DELAY))
            diff = -diff;
        *((DWORD *)(TraceBuffer + TraceTail)) = diff;
        TraceTail += sizeof(DWORD);
    }

padding:
    // Add padding before buffer rollover - max. data (< 48)
    diff = TB_SIZE - TraceTail;
    if (diff < 48) {
        while (diff--) {
            *((BYTE *)(TraceBuffer + TraceTail)) = TB_PADDING;
            TraceTail += sizeof(BYTE);
        }
        TraceTail = 0;
    }
}


// Trace Record: Process Exception Data
__inline static void TR_Exception(TR_ITEM *tr)
{
    TR_EXC *pE, *nE;
    DWORD   type;
    DWORD   num;
    double  t, d;

    type = (tr->data >> TR_EXC_TYP_POS) & TR_EXC_TYP_MSK;
    num  = tr->data & TR_EXC_NUM_MSK;

    if (!(tr->flag & TR_TS_VALID)) { // Check for Timestamp
        if (type == TR_EXC_ENTRY) {
            TraceExcData[num].count++;
        }
        t = 0;    // R.K. 15.1.2008
        goto end; // No Timestamp
    }

    t = TraceTailTime + ((double)(tr->tcyc - TraceTailCycles) / TraceTailClock);
    d = t - TraceExcTime; // Time difference

    pE = &TraceExcData[TraceExcNum]; // Previous Exception
    nE = &TraceExcData[num];         // New Exception

    switch (type) {
        case TR_EXC_ENTRY:
            // Process previous Exception interrupted by new Exception
            pE->tin += d;    // Adjust Time In
            pE->ttotal += d; // Adjust Total Time
            // Process New Exception
            nE->count++;    // Increment Counter
            nE->tin    = 0; // Reset Time In
            nE->tenter = t; // Save Enter Time
            if (nE->tfirst < 0) {
                nE->tfirst = t; // Save First Time
            }
            if (nE->texit < 0)
                break;
            d = t - nE->texit; // Time Out
            if ((nE->toutmin < 0) || (d < nE->toutmin)) {
                nE->toutmin = d; // Min Time Out
            }
            if ((nE->toutmax < 0) || (d > nE->toutmax)) {
                nE->toutmax = d; // Max Time Out
            }
            break;
        case TR_EXC_RETURN:
            break;
        case TR_EXC_EXIT:
            nE->tin += d;    // Time In
            nE->ttotal += d; // Total Time
            if ((nE->tinmin < 0) || (nE->tin < nE->tinmin)) {
                nE->tinmin = nE->tin; // Min Time In
            }
            if ((nE->tinmax < 0) || (nE->tin > nE->tinmax)) {
                nE->tinmax = nE->tin; // Max Time In
            }
            nE->texit = t; // Save Exit Time
            break;
    }

end:
    TraceExcNum  = num; // Save current Exception Number
    TraceExcTime = t;   // Save current Exception Time
}


// Buffer ITM printf Ch.0 chars before sending them over SendMessage in AG_Serial

#define ITMBUF_SIZE 4096
char         ITMBuf_Buffer[ITMBUF_SIZE];
unsigned int ITMBuf_pos = 0;

static void ITMBuf_SendBuf(void)
{
    if (!ITMBuf_pos)
        return;

    AG_Serial(AG_SERBOUT, 3, ITMBuf_pos, ITMBuf_Buffer);
    ITMBuf_pos = 0;
}

static void ITMBuf_AddChar(int c)
{
    if (ITMBuf_pos > ITMBUF_SIZE - 4) {
        ITMBuf_SendBuf();
    }
    ITMBuf_Buffer[ITMBuf_pos++] = c;
}
// ------------------------------


// Trace Record: Process
static void TR_Process(void)
{
    TR_ITEM *tr;
    double   t;

    while (tr = TR_GetItem()) {
        TR_Save(tr);
        switch (tr->type) {
            case TR_ITM:
                if (tr->addr == 0) {
                    LeaveCriticalSection(&TraceCS);
                    //AG_Serial (AG_SERBOUT, 3, 1, &tr->data);
                    ITMBuf_AddChar(tr->data);               // Buffer ITM printf Ch.0 chars before sending them over SendMessage in AG_Serial
                    AG_Serial(AG_ITMBOUT, 0, 1, &tr->data); // 10.1.2011
                    EnterCriticalSection(&TraceCS);
                } else if (tr->addr == 31) {
                    // RTX Message
                    switch (tr->size) {
                        case 1: // Task Switch: 8-bit Task ID
                            t = TraceTailTime;
                            t += ((double)(tr->tcyc - TraceTailCycles) / TraceTailClock);
                            MsgData[0]               = (BYTE)tr->data;
                            MsgData[1]               = TaskLUT[(BYTE)tr->data];
                            *((double *)&MsgData[2]) = t;
                            Message.pData            = (BYTE *)MsgData;
                            Message.nLen             = 16;
                            pio->Notify(UV_DCC_MESSAGE, (void *)&Message);
                            break;
                        case 2: // Task Notify - Part2: 8-bit Task ID + Create Flag in bit8
                            if (tr->data & 0x0100) {
                                // Create Task
                                TaskLUT[(BYTE)tr->data] = TaskAddress;
                                TaskAddress             = 0; // [TdB: 12.06.2014] On MCU Clk switch events (Task Notify) get lost, and TaskLUT is filled incorrect.
                            } else {
                                // Delete Task
                                TaskLUT[(BYTE)tr->data] = 0;
                            }
                            break;
                        case 4: // Task Notify - Part1: 32-bit Task Address
                            TaskAddress = tr->data;
                            break;
                    }
                }

                switch (tr->addr) { // 10.1.2011
                    case 0:         // printf - #0 - routed
                                    //          AG_Serial (AG_ITMBOUT, 0, 1, &tr->data);  - routed above outside CS.
                        break;
                    default: // #1...#30 - touted
                        AG_Serial(AG_ITMBOUT, tr->addr, tr->size, &tr->data);
                        break;
                    case 31:                                            // RTX - #31 - not routed
                        AG_Serial(AG_ITMBOUT, 31, tr->size, &tr->data); // 07.06.2016: Must be routed to ITM Router for UVSock
                        break;
                }
                break;

            case TR_EVT:
                if (tr->data & TR_EVT_CPI)
                    TraceCntCPI++;
                if (tr->data & TR_EVT_EXC)
                    TraceCntExc++;
                if (tr->data & TR_EVT_SLEEP)
                    TraceCntSleep++;
                if (tr->data & TR_EVT_LSU)
                    TraceCntLSU++;
                if (tr->data & TR_EVT_FOLD)
                    TraceCntFold++;
                break;
            case TR_EVT_EXTTRC:
                TR_Exception(tr);
                break;
            case TR_EVT_PC:
                break;
            case TR_DATA_READ:
                break;
            case TR_DATA_WRITE:
                if (!(tr->flag & TR_TS_VALID))
                    break;
                memset(&lar, 0, sizeof(lar));
                lar.nAdr    = tr->addr;
                lar.nPC     = (tr->flag & TR_PC_VALID) ? tr->nPC : 0xFFFFFFFF;
                lar.wType   = tr->size;
                lar.v.u32   = tr->data;
                lar.tStamp  = tr->tcyc;
                lar.totTime = tr->tcyc;
                SendLaDataRecord(&lar);
                break;
            default:
                break;
        }
    }

    ITMBuf_SendBuf(); // Buffer ITM printf Ch.0 chars before sending them over SendMessage in AG_Serial
}


// Trace: Initialization
//   return : 0 - OK,  else error code
int Trace_Init(void)
{
    int i;

    TraceBuffer = (BYTE *)calloc(TB_SIZE, 1);
    if (TraceBuffer == NULL)
        return (EU01);

    TraceHead = TraceTail = 0;
    TraceHeadClock = TraceTailClock = TraceConf.Clk;
    TraceHeadCycles = TraceTailCycles = 0;
    TraceHeadTime = TraceTailTime = 0.0;
    TraceDisp                     = 0;

    for (i = 0; i < EXC_NUM; i++) {
        TraceExcData[i].count   = 0;
        TraceExcData[i].tenter  = -1.0;
        TraceExcData[i].texit   = -1.0;
        TraceExcData[i].tin     = 0.0;
        TraceExcData[i].ttotal  = 0.0;
        TraceExcData[i].tinmin  = -1.0;
        TraceExcData[i].tinmax  = -1.0;
        TraceExcData[i].toutmin = -1.0;
        TraceExcData[i].toutmax = -1.0;
        TraceExcData[i].tfirst  = -1.0;
    }
    TraceExcNum  = 0;
    TraceExcTime = 0.0;
    TaskAddress  = 0;

    TraceCntCPI   = 0;
    TraceCntExc   = 0;
    TraceCntSleep = 0;
    TraceCntLSU   = 0;
    TraceCntFold  = 0;

    for (i = 0; i < 256; i++)
        TaskLUT[i] = 0;

    InitializeCriticalSection(&TraceCS);

#if DBGCM_FEATURE_ETM
    if (ETM_Addr) {
        //---TODO: Initialize ETM
        //         - Unlock ETM
        //         - Power-up ETM
        DEVELOP_MSG("Todo: \nUnlock and power up ETM");
    }
#endif // DBGCM_FEATURE_ETM

#if DBGCM_FEATURE_TRCDATA_WIN
    pio->bTrcDisp = 1; // Trace Window Display Support
    InitTraceInterface();
    ConfigureTraceWin();
#endif // DBGCM_FEATURE_TRCDATA_WIN

    TraceDispFlg = TRUE;
    TraceInit    = TRUE;

    return (0);
}


// Trace: Uninitialization
//   return : 0 - OK,  else error code
int Trace_UnInit(void)
{
    if (TraceInit) {
        TraceInit = FALSE;
#if DBGCM_FEATURE_ETM
        if (ETM_Addr) {
            //---TODO: Uninitialize ETM
            //         - Power down ETM
            DEVELOP_MSG("Todo: \nPower down ETM");
        }
#endif // DBGCM_FEATURE_ETM
    }

    if (TraceBuffer) {
        free(TraceBuffer);
        TraceBuffer = NULL;
        EnterCriticalSection(&TraceCS);
        TraceDispFlg = FALSE;
        LeaveCriticalSection(&TraceCS);
        DeleteCriticalSection(&TraceCS);
    }

    return (0);
}


// Trace: Setup
//   return : 0 - OK,  else error code
int Trace_Setup(void)
{
    DWORD val;
    int   status;
    DWORD APSel;

    BOOL hasItm = FALSE;
    switch (xxCPU) {
        case ARM_CM3:
        case ARM_CM4:
        case ARM_CM7:
        case ARM_CM33:
        case ARM_CM35P:
            hasItm = TRUE;
    }

    // Check for Trace HW
    if ((DWT_Addr == 0) || ((ITM_Addr == 0) && hasItm) || (TraceConf.Protocol != TPIU_ETB && TPIU_Location.Addr == 0)) {
        return (EU15); // Trace HW not present
    }

    // Latch Active Trace Options
    TraceOpt = TraceConf.Opt;

    // Preset DWT Control
    RegDWT.CTRL = DWT_CYCCNTEN;
    RegDWT.CTRL |= (TraceConf.CYC_Pre << 1) & DWT_POSTSET;
    RegDWT.CTRL |= (TraceConf.CYC_Pre << 5) & DWT_CYCTAP;
    if (TraceConf.Clk >= (1 << 28)) {
        RegDWT.CTRL |= DWT_SYNCTAP28;
        SyncPeriod = (DWORD)(1000.0 * (1 << 28) / (float)TraceConf.Clk) + 100;
    } else if (TraceConf.Clk >= (1 << 26)) {
        RegDWT.CTRL |= DWT_SYNCTAP26;
        SyncPeriod = (DWORD)(1000.0 * (1 << 26) / (float)TraceConf.Clk) + 100;
    } else {
        RegDWT.CTRL |= DWT_SYNCTAP24;
        SyncPeriod = (DWORD)(1000.0 * (1 << 24) / (float)TraceConf.Clk) + 100;
    }
    if (TraceConf.Opt & TRACE_PCSAMPLE)
        RegDWT.CTRL |= DWT_PCSAMPLEEN;
    if (!ETB_Configured || ETB_ITMConnected) {
        if (TraceConf.Opt & TRACE_EXCTRC)
            RegDWT.CTRL |= DWT_EXCTRCEN;
        if (TraceConf.Opt & TRACE_CPI)
            RegDWT.CTRL |= DWT_CPIEVTEN;
        if (TraceConf.Opt & TRACE_EXC)
            RegDWT.CTRL |= DWT_EXCEVTEN;
        if (TraceConf.Opt & TRACE_SLEEP)
            RegDWT.CTRL |= DWT_SLEEPEVTEN;
        if (TraceConf.Opt & TRACE_LSU)
            RegDWT.CTRL |= DWT_LSUEVTEN;
        if (TraceConf.Opt & TRACE_FOLD)
            RegDWT.CTRL |= DWT_FOLDEVTEN;
    }
    if (TraceConf.Opt & TRACE_CYCCNT)
        RegDWT.CTRL |= DWT_CYCEVTEN;

    // Preset ITM Trace Control
    if (TraceConf.Protocol == TPIU_ETB) {
        ITM_TraceControl = ITM_ITMENA | ITM_DWTENA | (ITM_ATBID << 16); // ITM_DWTENA == ITM_TXENA for v8-M
        TS_Presc         = 1;                                           // Just initialize, not used for ETB
    } else {
        ITM_TraceControl = ITM_ITMENA | ITM_SYNCENA | ITM_DWTENA | (ITM_ATBID << 16); // ITM_DWTENA == ITM_TXENA for v8-M
        ITM_TraceControl |= TraceConf.TS_Pre << 8;
        TS_Presc = 1 << (TraceConf.TS_Pre << 1);
    }

    if (TraceConf.Protocol != TPIU_ETB) {
        status = LinkCom(1); // Apply lock to protect AP_Sel
        if (status)
            return (status);

        APSel  = AP_Sel;                        // Save AP_Sel
        AP_Sel = (TPIU_Location.AP << APSEL_P); // Switch to TPIU Access Port

        if (TraceConf.Protocol != TPIU_TRACE_PORT) {
            // Autodetect SWO Prescaler
            if (TraceConf.SWV_Pre & 0x8000) {
                for (val = 1; val <= 8192; val++) {
                    // if (SWV_Check(TraceConf.Clk / val) == 0) {
                    if (SWV_Check(TPIU_Clock / val) == 0) { // 02.04.2019: Separate Trace Clock setting
                        TraceConf.SWV_Pre = 0x8000 | (WORD)(val - 1);
                        break;
                    }
                }
            }

            // Setup SWV
            //status = SWV_Setup(TraceConf.Clk / ((TraceConf.SWV_Pre & 0x1FFF) + 1));
            status = SWV_Setup(TPIU_Clock / ((TraceConf.SWV_Pre & 0x1FFF) + 1)); // 02.04.2019: Separate Trace Clock setting
            if (status)
                goto end_tpiu;


#if DBGCM_V8M
            // Setup TPIU
            status = WriteD32(TPIU_ASYNCLKPRES, (TraceConf.SWV_Pre & 0x1FFF), BLOCK_SECTYPE_ANY);
            if (status)
                goto end_tpiu;
#else  // DBGCM_V8M \
       // Setup TPIU
            status = WriteD32(TPIU_ASYNCLKPRES, (TraceConf.SWV_Pre & 0x1FFF));
            if (status)
                goto end_tpiu;
#endif // DBGCM_V8M
        }

#if DBGCM_V8M

        // Setup TPIU
        if (TPIU_Type != TPIU_TYPE_SWO) { // CURRSYNPORTSZ read-only for SWO
            status = WriteD32(TPIU_CURRSYNPORTSZ, TraceConf.PortSize, BLOCK_SECTYPE_ANY);
            if (status)
                goto end_tpiu;
        }
        status = WriteD32(TPIU_PINPROTOCOL, TraceConf.Protocol, BLOCK_SECTYPE_ANY);
        if (status)
            goto end_tpiu;
        if (TPIU_Type != TPIU_TYPE_SWO && TPIU_Type != TPIU_TYPE_CS) { // FFCR read-only for SWO
            status = WriteD32(TPIU_FMTFLSHCTRL, TraceConf.Opt & TPIU_FORMAT ? 2 : 0, BLOCK_SECTYPE_ANY);
        }
        if (status)
            goto end_tpiu;

#else // DBGCM_V8M

        // Setup TPIU
        if (TPIU_Type != TPIU_TYPE_SWO) { // CURRSYNPORTSZ read-only for SWO
            status = WriteD32(TPIU_CURRSYNPORTSZ, TraceConf.PortSize);
            if (status)
                goto end_tpiu;
        }
        status = WriteD32(TPIU_PINPROTOCOL, TraceConf.Protocol);
        if (status)
            goto end_tpiu;
        if (TPIU_Type != TPIU_TYPE_SWO && TPIU_Type != TPIU_TYPE_CS) { // FFCR read-only for SWO
            status = WriteD32(TPIU_FMTFLSHCTRL, TraceConf.Opt & TPIU_FORMAT ? 2 : 0);
        }
        if (status)
            goto end_tpiu;

#endif // DBGCM_V8M

    end_tpiu:
        AP_Sel = APSel; // Restore AP_Sel

        if (status) {
            LinkCom(0);
        } else {
            status = LinkCom(0);
        }
        if (status)
            return (status);
    }

#if DBGCM_V8M

    // Setup ITM
    status = WriteD32(ITM_LOCKACCESS, ITM_UNLOCK, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
    status = WriteD32(ITM_TRACECONTROL, ITM_TraceControl, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
    status = WriteD32(ITM_TRACEENABLE, TraceConf.ITM_Ena, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
    status = WriteD32(ITM_TRACEPRIVILEGE, TraceConf.ITM_Priv, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);

    // Setup DWT
    if (IsV8M()) {
        DWORD dummyDwtCtrl = 0;
        status             = ReadBlock(DWT_CTRL, (BYTE *)&dummyDwtCtrl, 4, BLOCK_SECTYPE_ANY); // [TdB: 03.02.2017] (SDMDK-6636) preserve DWT_CTRL.CYCDISS Bit
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
        dummyDwtCtrl &= DWT_CYCDISS;
        if (dummyDwtCtrl)
            RegDWT.CTRL |= DWT_CYCDISS;
        else
            RegDWT.CTRL &= ~DWT_CYCDISS;
    } else {
        status = WriteD32(DWT_CTRL, RegDWT.CTRL, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
    }

#else // DBGCM_V8M

    // Setup ITM
    status = WriteD32(ITM_LOCKACCESS, ITM_UNLOCK);
    if (status)
        return (status);
    status = WriteD32(ITM_TRACECONTROL, ITM_TraceControl);
    if (status)
        return (status);
    status = WriteD32(ITM_TRACEENABLE, TraceConf.ITM_Ena);
    if (status)
        return (status);
    status = WriteD32(ITM_TRACEPRIVILEGE, TraceConf.ITM_Priv);
    if (status)
        return (status);

    // Setup DWT
    status = WriteD32(DWT_CTRL, RegDWT.CTRL);
    if (status)
        return (status);

#endif // DBGCM_V8M


#if DBGCM_FEATURE_ETM
    if (ETM_Addr) {
        //---TODO: Program ETM according to trace setup
        //         (TraceConf.Opt & ETM_TRACE) == 1 => Enable ETM Trace
        //         (TraceConf.Opt & ETM_TRACE) == 0 => Disable ETM Trace
        DEVELOP_MSG("Todo: \nProgram ETM according to trace setup");
        if (TraceConf.Opt & ETM_TRACE) {
            DEVELOP_MSG("Todo: \nImplement ETM decoder path with starts with TPIU formatter decode in TD_Read");
        }
    }
#endif // DBGCM_FEATURE_ETM

    return (status);
}

// ITM Reconfigure Trace Exc & Event Counters
//   return : 0 - OK,  else error code
// [TdB: 13.02.2013] Trace Exceptions & Event Counters Window can modify this settings on the fly
int ITM_Reconfig(void)
{
    int status;

    // Read DWT
#if DBGCM_V8M
    status = ReadD32(DWT_CTRL, &RegDWT.CTRL, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
#else  // DBGCM_V8M
    status = ReadD32(DWT_CTRL, &RegDWT.CTRL);
    if (status)
        return (status);
#endif // DBGCM_V8M

    if (!ETB_Configured || ETB_ITMConnected) {
        if (TraceConf.Opt & TRACE_EXCTRC)
            RegDWT.CTRL |= DWT_EXCTRCEN;
        else
            RegDWT.CTRL &= ~DWT_EXCTRCEN;
        if (TraceConf.Opt & TRACE_CPI)
            RegDWT.CTRL |= DWT_CPIEVTEN;
        else
            RegDWT.CTRL &= ~DWT_CPIEVTEN;
        if (TraceConf.Opt & TRACE_EXC)
            RegDWT.CTRL |= DWT_EXCEVTEN;
        else
            RegDWT.CTRL &= ~DWT_EXCEVTEN;
        if (TraceConf.Opt & TRACE_SLEEP)
            RegDWT.CTRL |= DWT_SLEEPEVTEN;
        else
            RegDWT.CTRL &= ~DWT_SLEEPEVTEN;
        if (TraceConf.Opt & TRACE_LSU)
            RegDWT.CTRL |= DWT_LSUEVTEN;
        else
            RegDWT.CTRL &= ~DWT_LSUEVTEN;
        if (TraceConf.Opt & TRACE_FOLD)
            RegDWT.CTRL |= DWT_FOLDEVTEN;
        else
            RegDWT.CTRL &= ~DWT_FOLDEVTEN;
    }

    // Setup DWT
#if DBGCM_V8M
    if (IsV8M()) {
        DWORD dummyDwtCtrl = 0;
        status             = ReadBlock(DWT_CTRL, (BYTE *)&dummyDwtCtrl, 4, BLOCK_SECTYPE_ANY); // [TdB: 03.02.2017] (SDMDK-6636) preserve DWT_CTRL.CYCDISS Bit
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
        dummyDwtCtrl &= DWT_CYCDISS;
        if (dummyDwtCtrl)
            RegDWT.CTRL |= DWT_CYCDISS;
        else
            RegDWT.CTRL &= ~DWT_CYCDISS;
    }
    status = WriteD32(DWT_CTRL, RegDWT.CTRL, BLOCK_SECTYPE_ANY);
    return (status);
#else  // DBGCM_V8M
    status = WriteD32(DWT_CTRL, RegDWT.CTRL);
    return (status);
#endif // DBGCM_V8M
}


// Trace: Flush Buffers
//   return : 0 - OK,  else error code
int Trace_Flush(BOOL setRun)
{
    int status = 0;

    if (!setRun) {
        if (TraceCycDwt) {
#if DBGCM_V8M
            status = ReadD32(DWT_CYCCNT, &RegDWT.CYCCNT, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
            status = ReadD32(DWT_CYCCNT, &RegDWT.CYCCNT);
#endif // DBGCM_V8M
        }
    }
    if (!status) {
        status = SWV_Flush();
    }

    TD_Head = TD_Tail = 0;
    TP_Head = TP_Tail = 0;
    TP_Buf[0].type    = TP_IDLE;
    TR_Head = TR_Tail = 0;
    TR_NoTS           = -1;

    T_Err     = 0;
    T_Recover = 0;

    TracePacketSync = 0;
    TracePacketID   = 0;

    SyncTimeout = SyncPeriod;
    return (status);
}


// Trace: Timestamp Cycles
//   tcyc   : On/Off
//   return : 0 - OK,  else error code
int Trace_Cycles(BOOL tcyc)
{
    int status;

    if (tcyc && TraceConf.Protocol != TPIU_ETB) {
        ITM_TraceControl |= ITM_TSENA;
    } else {
        ITM_TraceControl &= ~ITM_TSENA;
    }

#if DBGCM_V8M
    status = WriteD32(ITM_TRACECONTROL, ITM_TraceControl, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
#else  // DBGCM_V8M
    status = WriteD32(ITM_TRACECONTROL, ITM_TraceControl);
    if (status)
        return (status);
#endif // DBGCM_V8M

    return (status);
}


// Trace: Timestamp Synchronization
//   sync   : On/Off
//   setRun : Part of setting target running, otherwise starting trace
//            after detecting running target
//   return : 0 - OK,  else error code
int Trace_TSync(BOOL sync, BOOL setRun)
{
    int   status;
    DWORD val;

    if (TraceConf.Protocol == TPIU_ETB) {
        // No need to synchronize, no time info supported for ETB (for now)
        return 0;
    }

    if (!TraceCycDwt) {
        // DWT has no CYCCNT
        if (sync) {
            TS_Cycles = RegARM.nCycles;
            TS_Sync   = TRUE;
        }
        return (0);
    }

    if (!setRun) {
        if (sync) {
            // Sample Cycles (PC Sample or CYCCNT)
            if (RegDWT.CTRL & DWT_CYCTAP) {
                SampleCycles = 1 << 10;
            } else {
                SampleCycles = 1 << 6;
            }
            SampleCycles *= ((RegDWT.CTRL & DWT_POSTSET) >> DWT_POSTSET_P) + 1;
        }
        return (0);
    }

    if (sync) {
#if DBGCM_V8M
        // Update Cycles
        status = ReadD32(DWT_CYCCNT, &val, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
        RegARM.nCycles += val - RegDWT.CYCCNT;
#else  // DBGCM_V8M \
       // Update Cycles
        status = ReadD32(DWT_CYCCNT, &val);
        if (status)
            return (status);
        RegARM.nCycles += val - RegDWT.CYCCNT;
#endif // DBGCM_V8M

        TS_Cycles = RegARM.nCycles;
        TS_Sync   = TRUE;

        // Setup DWT with enabled CYCCNT Event
        val = RegDWT.CTRL & ~DWT_POSTCNT;
        if (!(TraceOpt & (TRACE_PCSAMPLE | TRACE_CYCCNT))) {
            // Enable CYCCNT Event with max period
            val |= DWT_CYCEVTEN | DWT_CYCTAP | DWT_POSTSET;
        }

        // Sample Cycles (PC Sample or CYCCNT)
        if (val & DWT_CYCTAP) {
            SampleCycles = 1 << 10;
        } else {
            SampleCycles = 1 << 6;
        }
        SampleCycles *= ((val & DWT_POSTSET) >> DWT_POSTSET_P) + 1;

        // Force CYCCNT Event on next cycle by adjusting CYCCNT
        RegDWT.CYCCNT |= (val & DWT_CYCTAP) ? 0x03FF : 0x003F;
#if DBGCM_V8M
        status = WriteD32(DWT_CYCCNT, RegDWT.CYCCNT, BLOCK_SECTYPE_ANY);
        if (status)
            return (status);
#else  // DBGCM_V8M
        status = WriteD32(DWT_CYCCNT, RegDWT.CYCCNT);
        if (status)
            return (status);
#endif // DBGCM_V8M

    } else {
        if (TraceOpt & (TRACE_PCSAMPLE | TRACE_CYCCNT))
            return (0);
        val = RegDWT.CTRL;
    }

    // Update DWT Control
#if DBGCM_V8M
    if (IsV8M()) {
        DWORD dummyDwtCtrl = 0;
        status             = ReadBlock(DWT_CTRL, (BYTE *)&dummyDwtCtrl, 4, BLOCK_SECTYPE_ANY); // [TdB: 03.02.2017] (SDMDK-6636) preserve DWT_CTRL.CYCDISS Bit
        if (status) {
            OutErrorMessage(status);
            return (1);
        }
        dummyDwtCtrl &= DWT_CYCDISS;
        if (dummyDwtCtrl)
            val |= DWT_CYCDISS;
        else
            val &= ~DWT_CYCDISS;
    }
    status = WriteD32(DWT_CTRL, val, BLOCK_SECTYPE_ANY);
    if (status)
        return (status);
#else  // DBGCM_V8M
    status = WriteD32(DWT_CTRL, val);
    if (status)
        return (status);
#endif // DBGCM_V8M

    return (status);
}


// Trace: Clock Synchronization
//   return : 0 - OK,  else error code
int Trace_ClkSync(void)
{
    int   status;
    DWORD APSel;

    // Setup SWV
    // status = SWV_Setup(TraceConf.Clk / ((TraceConf.SWV_Pre & 0x1FFF) + 1));
    status = SWV_Setup(TPIU_Clock / ((TraceConf.SWV_Pre & 0x1FFF) + 1)); // 02.04.2019: Separate Trace Clock setting
    if (status)
        return (status);

    status = LinkCom(1); // Apply lock to protect AP_Sel
    if (status)
        return (status);

    APSel  = AP_Sel;                        // Save AP_Sel
    AP_Sel = (TPIU_Location.AP << APSEL_P); // Switch to TPIU Access Port

    // Setup TPIU
#if DBGCM_V8M
    status = WriteD32(TPIU_ASYNCLKPRES, (TraceConf.SWV_Pre & 0x1FFF), BLOCK_SECTYPE_ANY);
    // if (status) return (status);
#else  // DBGCM_V8M
    status = WriteD32(TPIU_ASYNCLKPRES, (TraceConf.SWV_Pre & 0x1FFF));
    // if (status) return (status);
#endif // DBGCM_V8M

    AP_Sel = APSel; // Restore AP_Sel

    if (status) {
        LinkCom(0);
    } else {
        status = LinkCom(0);
    }
    if (status)
        return (status);

    TR_Save(NULL);

    return (0);
}


// Trace: Read Data
//   time   : time in ms
//   ts     : timestamps
//   return : 0 - OK,  else error code
int Trace_Read(DWORD time, BOOL ts)
{
    int   status;
    DWORD tick;

    TS_Active = ts;

    if (TraceOpt & TRACE_ENABLE) {
        if (time) {
            tick = GetTickCount();
            do {
                status = SWV_Read(time);
                if (status)
                    return (status);
                if (T_Err & T_ERR_HW_COM) {
                    T_Err &= ~T_ERR_HW_COM;
                    if (T_Msg >= T_MSG_COM_ERR) {
                        T_Recover = T_RECOVER;
                        if (T_Msg != T_MSG_COM_ERR) {
                            T_Msg = T_MSG_COM_ERR;
                            //OutMsg(TraceMsg[T_Msg]);
                            OutTraceMsg(T_Msg);
                        }
                    }
                }
                if (T_Err & T_ERR_HW_BUF) {
                    T_Err &= ~T_ERR_HW_BUF;
                    if (T_Msg >= T_MSG_HW_BUF) {
                        T_Recover = T_RECOVER;
                        if (T_Msg != T_MSG_HW_BUF) {
                            T_Msg = T_MSG_HW_BUF;
                            //OutMsg(TraceMsg[T_Msg]);
                            OutTraceMsg(T_Msg);
                        }
                    }
                }
            } while ((GetTickCount() - tick) < time);
            if (ts) {
                SyncTimeout -= time;
                if (SyncTimeout <= 0) {
                    SyncTimeout = SyncPeriod;
                    T_Err |= T_ERR_NO_SYNC;
                }
            }
        }

        TD_Read();

        TP_Read();
        //  TP_DbgPrint();

#if DBGCM_V8M
        if (IsV8M()) {
            TR_Read_v8M();
        } else {
            TR_Read();
        }
#else  // DBGCM_V8M
        TR_Read();
#endif // DBGCM_V8M \
    //  TR_DbgPrint();

        if (T_Err & T_ERR_SW_BUF) {
            T_Err &= ~T_ERR_SW_BUF;
            if (T_Msg >= T_MSG_SW_BUF) {
                T_Recover = T_RECOVER;
                if (T_Msg != T_MSG_SW_BUF) {
                    T_Msg = T_MSG_SW_BUF;
                    //OutMsg(TraceMsg[T_Msg]);
                    OutTraceMsg(T_Msg);
                }
            }
        }
        if (T_Err & T_ERR_NO_SYNC) {
            T_Err &= ~T_ERR_NO_SYNC;
            if (T_Msg >= T_MSG_NO_SYNC) {
                T_Recover = T_RECOVER;
                if (T_Msg != T_MSG_NO_SYNC) {
                    T_Msg = T_MSG_NO_SYNC;
                    //OutMsg(TraceMsg[T_Msg]);
                    OutTraceMsg(T_Msg);
                }
            }
        }
        if (T_Err & T_ERR_DATA_OVF) {
            T_Err &= ~T_ERR_DATA_OVF;
            if (T_Msg >= T_MSG_DATA_OVF) {
                T_Recover = T_RECOVER;
                if (T_Msg != T_MSG_DATA_OVF) {
                    T_Msg = T_MSG_DATA_OVF;
                    //OutMsg(TraceMsg[T_Msg]);
                    OutTraceMsg(T_Msg);
                }
            }
        }

        EnterCriticalSection(&TraceCS);
        TR_Process();
        LeaveCriticalSection(&TraceCS);

        if (T_Recover) {
            T_Recover -= time;
            if (T_Recover <= 0) {
                T_Recover = 0;
                T_Msg     = T_MSG_RUN;
                //OutMsg(TraceMsg[T_Msg]);
                OutTraceMsg(T_Msg);
            }
        }
    }

#if DBGCM_V8M
    if (pio->bSecureExt && ((pio->bSTrcEna == 1) != (T_Secure == TRUE))) {
        // Refresh message with correct Trace Authentication
        OutTraceMsg(T_Msg);
    }
#endif // DBGCM_V8M

    if (TraceOpt & PC_SAMPLE) {
        // To Do:
        // Multiple Read of DWT Program Counter Sample @ 0xE000101C
        // Writing the PC Samples to Trace Records (no Timestamp)
        DEVELOP_MSG("Todo: \nTo Do: \nMultiple Read of DWT Program Counter Sample @ 0xE000101C\nWriting the PC Samples to Trace Records (no Timestamp)");
    }

    return (0);
}


#if DBGCM_RECOVERY
// Trace: Recovery, e.g. after low-power mode
//   return : 0 - OK,  else error code
int Trace_Recovery(void)
{
    return Trace_Setup();
}
#endif // DBGCM_RECOVERY


void InitTrace()
{
    T_Msg     = 0; // Trace Message
    T_Err     = 0; // Trace Error
    T_Recover = 0; // Trace Error Recover Time

    // Trace Data
    memset(TD_Buf, 0, sizeof(TD_Buf)); // Buffer
    TD_Head = 0;                       // Head Pointer
    TD_Tail = 0;                       // Tail Pointer
    TD_Byte = 0;                       // Byte

    // Trace Packets
    memset(TP_Buf, 0, sizeof(TP_Buf)); // Buffer
    TP_Head = 0;                       // Head Pointer
    TP_Tail = 0;                       // Tail Pointer
    TP_Idx  = 0;                       // Index

    // Trace Records
    memset(TR_Buf, 0, sizeof(TR_Buf)); // Buffer
    TR_Head = 0;                       // Head Pointer
    TR_Tail = 0;                       // Tail Pointer
    TR_NoTS = 0;                       // No Timestamp Pointer


    // Trace Buffer (Compressed)
    TraceBuffer     = NULL;
    TraceHead       = 0;
    TraceHeadClock  = 0;
    TraceHeadCycles = 0;
    TraceHeadTime   = 0.0;
    TraceTail       = 0;
    TraceTailClock  = 0;
    TraceTailCycles = 0;
    TraceTailTime   = 0.0;
    TraceDisp       = 0;

    memset(TraceExcData, 0, sizeof(TraceExcData)); // Exception Trace Data

    TraceExcNum  = 0;   // Current Exception Number
    TraceExcTime = 0.0; // Latched Exception Time

    /* Event Counters (Count Overflows of 256) */
    TraceCntCPI   = 0; // CPI Overflow Counter
    TraceCntExc   = 0; // EXC Overflow Counter
    TraceCntSleep = 0; // Sleep Overflow Counter
    TraceCntLSU   = 0; // LSU Overflow Counter
    TraceCntFold  = 0; // Fold Overflow Counter


    TraceDispFlg = FALSE; // Trace Display Flag
    TraceInit    = FALSE; // Trace Init Flag

    memset(&TraceCS, 0, sizeof(TraceCS));


    // Trace Configuration
    memset(&TraceConf, 0, sizeof(TraceConf));

    // Trace Options (Active TraceConf.Opt)
    TraceOpt = 0;


    // Trace Registers
    ITM_TraceControl = 0; // ITM Trace Control

    // Trace Variables
    TracePacketSync = 0;                         // Trace Packet Synchronization
    TracePacketID   = 0;                         // Trace Packet ID
    memset(TracePacket, 0, sizeof(TracePacket)); // Trace Packet Data

    TS_Active = 0; // Timestamps Active
    TS_Sync   = 0; // Timestamps Synchronization
    TS_Presc  = 0; // Timestamps Prescaler
    TS_Cycles = 0; // Timestamps Cycles

    SyncPeriod  = 0; // Synchronization Period in ms
    SyncTimeout = 0; // Synchronization Timeout in ms

    SampleCycles = 0; // Sample Cycles (PC Sample or CYCCNT)

    TaskAddress = 0;                     // Task Address
    memset(TaskLUT, 0, sizeof(TaskLUT)); // Task Address Look-Up Table

    memset(&lar, 0, sizeof(lar)); // Logic Analyzer Record

    memset(MsgData, 0, sizeof(MsgData)); // Message Data

    memset(&Message, 0, sizeof(Message));
    Message.nTarg = 0x0004;
    ITMBuf_pos    = 0;

    T_Secure    = 0;
    TraceCycDwt = TRUE;

    TPIU_Clock = 1;
}
