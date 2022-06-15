/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.8
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


#ifndef __TRACE_H__
#define __TRACE_H__


/* Trace Packet Type */
typedef enum {
  TP_IDLE,                      // Idle Packet
  TP_SYNC,                      // Synchronization Packet
  TP_OVFL,                      // Overflow Packet
  TP_TIME,                      // Timestamp Packet
  TP_SWIT,                      // SWIT Packet
  TP_HWIT,                      // HWIT Packet
  TP_SWEXT,                     // SW Extension Packet
  TP_HWEXT,                     // HW Extension Packet
  TP_RSRVD,                     // Reserved Packet
} TP_TYPE;

/* Trace Packet HW ID */
typedef enum {
  TP_NOHW,                      // No HW ID
  TP_EVT,                       // Event (8-bit)
  TP_EVT_EXTTRC,                // Event EXTTRC
  TP_EVT_PC,                    // Event PC Sample
  TP_PC32,                      // v7-M: PC Value
  TP_PC_OR_MATCH = TP_PC32,     // v8-M: PC Value (8, 16, or 32 bits depends on packet size) or Match Packet (depends on payload)
  TP_ADDR16,                    // v7-M: Address Offset
  TP_DATA_ADDR = TP_ADDR16,     // v8-M: Data Address (8, 16, or 32 bits depends on packet size)
  TP_DATA_READ,                 // Data Read
  TP_DATA_WRITE,                // Data Write
} TP_HWID;

/* Trace Packet Item */
typedef struct {
  BYTE  type;                   // Type
  BYTE  hwid;                   // HW ID
  BYTE  ovfl;                   // Overflow
  BYTE  ctrl;                   // Control
  BYTE  size;                   // Size
  BYTE  addr;                   // IT Address
  DWORD data;                   // Payload Data
} TP_ITEM, *TP_ITEM_PTR;


/* Trace Record Type */
typedef enum {
  TR_NONE,
  TR_ITM,                       // ITM (SW)
  TR_EVT,                       // Event (except EXTTRC)
  TR_EVT_EXTTRC,                // Event EXTTRC
  TR_EVT_PC,                    // Event PC Sample
  TR_DATA_READ,                 // Data Read
  TR_DATA_WRITE,                // Data Write
} TR_TYPE;

/* Trace Record Events */
#define TR_EVT_CPI      0x01    // Event CPI
#define TR_EVT_EXC      0x02    // Event EXC
#define TR_EVT_SLEEP    0x04    // Event Sleep
#define TR_EVT_LSU      0x08    // Event LSU
#define TR_EVT_FOLD     0x10    // Event Fold
#define TR_EVT_CYC      0x20    // Event Cycle

#if 0  // See TraceDataTypes.h
/* Trace Record Exceptions */
typedef enum {
  TR_EXC_INVALID = 0,
  TR_EXC_ENTRY,                 // Excpetion Entry
  TR_EXC_EXIT,                  // Excpetion Exit
  TR_EXC_RETURN,                // Excpetion Return
} TR_EXC_TYPE;

#define TR_EXC_TYP_POS    12    // Exception Type Bit Position
#define TR_EXC_TYP_MSK  0x03    // Exception Type Bit Mask
#define TR_EXC_NUM_MSK 0x1FF    // Exception Number Mask
#endif

/* Trace Record Flags */
#define TR_TS_VALID     0x01    // Timestamp exists
#define TR_TS_DELAY     0x02    // Timestamp delayed
#define TR_DP_DELAY     0x04    // Data Packet delayed
#define TR_PC_VALID     0x10    // PC exists
#define TR_ADR_VALID    0x20    // Address exists
#define TR_OVERFLOW     0x80    // Overflow

/* Trace Record Item */
typedef struct {
  BYTE  type;                   // Type
  BYTE  flag;                   // Flags
  BYTE  size;                   // Size
  DWORD addr;                   // Address
  DWORD data;                   // Data
  DWORD nPC;                    // PC Value
  I64   tcyc;                   // Timestamp Cycles
} TR_ITEM, *TR_ITEM_PTR;


/* Trace Messages */
extern  char *TraceMsg[];

#define T_MSG_COM_ERR      1    // Trace: Communication Error
#define T_MSG_HW_BUF       2    // Trace: HW Buffer Overrun
#define T_MSG_SW_BUF       3    // Trace: SW Buffer Overrun
#define T_MSG_NO_SYNC      4    // Trace: No Synchronization
#define T_MSG_DATA_ERR     5    // Trace: Data Stream Error
#define T_MSG_DATA_OVF     6    // Trace: Data Overflow
#define T_MSG_RUN          7    // Trace: Running ...

extern  BYTE    T_Msg;          // Trace Message

/* Trace Error Mask */
#define T_ERR_TD_ERR    0x0001  // TD Stream Error
#define T_ERR_TD_BUF    0x0002  // TD Buffer Overrun
#define T_ERR_TP_BUF    0x0004  // TP Buffer Overrun
#define T_ERR_TR_BUF    0x0008  // TR Buffer Overrun
#define T_ERR_DATA_OVF  0x0010  // Data Overflow
#define T_ERR_NO_SYNC   0x0020  // No Synchronization
#define T_ERR_SW_BUF    0x000E  // SW Buffer Overrun
#define T_ERR_HW_BUF    0x0040  // HW Buffer Overrun
#define T_ERR_HW_COM    0x0080  // HW Communication Error
#define T_ERR_DATA_ERR  0x0100  // Data Stream Error

#define T_RECOVER       3000    // Trace Error Recover Timeout in ms

//extern  BYTE    T_Err;          // Trace Error
extern  int     T_Err;          // Trace Error
extern  int     T_Recover;      // Trace Error Recover Time

/* Trace Data */
#define TD_CNT  8192            // Max. Items
extern  BYTE    TD_Buf[TD_CNT]; // Buffer
extern  DWORD   TD_Head;        // Head Pointer
extern  DWORD   TD_Tail;        // Tail Pointer

/* Trace Packets */
#define TP_CNT  8192            // Max. Items
extern  TP_ITEM TP_Buf[TP_CNT]; // Buffer
extern  DWORD   TP_Head;        // Head Pointer
extern  DWORD   TP_Tail;        // Tail Pointer

/* Trace Records */
#define TR_CNT  8192            // Max. Items
extern  TR_ITEM TR_Buf[TR_CNT]; // Buffer
extern  DWORD   TR_Head;        // Head Pointer
extern  DWORD   TR_Tail;        // Tail Pointer


/* Trace Buffer (Compressed) */
#define TB_SIZE 0x200000
extern  BYTE   *TraceBuffer;
extern  DWORD   TraceHead;
extern  DWORD   TraceHeadClock;
extern  I64     TraceHeadCycles;
extern  double  TraceHeadTime;
extern  DWORD   TraceTail;
extern  DWORD   TraceTailClock;
extern  I64     TraceTailCycles;
extern  double  TraceTailTime;
extern  DWORD   TraceDisp;

/* Trace Buffer Record Flags */
#define TB_PADDING      0x00    // Padding
#define TB_TIMEINFO     0x80    // Time Info (Clock, Cycles, Time)
#define TB_OVERFLOW     0x80    // Overflow
#define TB_TS_VALID     0x40    // Timestamp exist
#define TB_ADR_VALID    0x20    // Address exists
#define TB_PC_VALID     0x10    // PC exists
#define TB_MASK         0x0C    // Record Mask
#define TB_EVENT        0x00    // Record: Event
#define TB_ITM          0x04    // Record: ITM
#define TB_DATA_RD      0x08    // Record: Data Read
#define TB_DATA_WR      0x0C    // Record: Data Write
#define TB_DATA_SZ      0x03    // Data Size Mask
#define TB_DATA_SZ8     0x01    // Data Size  8-bit
#define TB_DATA_SZ16    0x02    // Data Size 16-bit
#define TB_DATA_SZ32    0x03    // Data Size 32-bit


#define EXC_NUM 512             // Number of Exceptions

/* Exception Trace Data */
typedef struct {
  DWORD  count;                 // Exception Counter
  double tenter;                // Exception Enter Time
  double texit;                 // Exception Exit Time
  double tin;                   // Time in Exception
  double ttotal;                // Total Time in Exception
  double tinmin;                // Min Time in Exception
  double tinmax;                // Max Time in Exception
  double toutmin;               // Min Time out of Exception
  double toutmax;               // Max Time out of Exception
  double tfirst;                // First Time in Exception
} TR_EXC;

extern TR_EXC TraceExcData[EXC_NUM];

extern DWORD  TraceExcNum;      // Current Exception Number
extern double TraceExcTime;     // Latched Exception Time


/* Event Counters (Count Overflows of 256) */
extern DWORD  TraceCntCPI;      // CPI Overflow Counter
extern DWORD  TraceCntExc;      // EXC Overflow Counter
extern DWORD  TraceCntSleep;    // Sleep Overflow Counter
extern DWORD  TraceCntLSU;      // LSU Overflow Counter
extern DWORD  TraceCntFold;     // Fold Overflow Counter


extern BOOL   TraceDispFlg;     // Trace Display Flag

extern CRITICAL_SECTION TraceCS;


// Trace: Initialization
//   return : 0 - OK,  else error code
extern int  Trace_Init (void);

// Trace: Uninitialization
//   return : 0 - OK,  else error code
extern int  Trace_UnInit (void);

// Trace: Setup
//   return : 0 - OK,  else error code
extern int  Trace_Setup (void);

// Trace: Setup
//   return : 0 - OK,  else error code
extern int ITM_Reconfig (void);

// Trace: Flush Buffers
//   setRun : Part of setting target running, otherwise starting trace
//            after detecting running target
//   return : 0 - OK,  else error code
extern int  Trace_Flush (BOOL setRun);

// Trace: Timestamp Cycles
//   tcyc   : On/Off
//   return : 0 - OK,  else error code
extern int  Trace_Cycles (BOOL tcyc);

// Trace: Timestamp Synchronization
//   sync   : On/Off
//   setRun : Part of setting target running, otherwise starting trace
//            after detecting running target
//   return : 0 - OK,  else error code
extern int  Trace_TSync (BOOL sync, BOOL setRun);

// Trace: Clock Synchronization
//   return : 0 - OK,  else error code
extern int  Trace_ClkSync (void);

// Trace: Read Data
//   time   : time in ms
//   ts     : timestamps
//   return : 0 - OK,  else error code
extern int  Trace_Read (DWORD time, BOOL ts);

// Trace: Recovery, e.g. after low-power mode
//   return : 0 - OK,  else error code
extern int  Trace_Recovery (void);

// Trace: Save TR_ITEM from decoder to internal buffers
//   tr : item to save
extern void  TR_Save (TR_ITEM *tr);

// Trace Options
#define TRACE_ENABLE    0x0001  /* Global Trace Enable */
#define TRACE_TIMESTAMP 0x0002  /* Trace Timestamps */
#define TRACE_PCSAMPLE  0x0004  /* Trace PC Samples */
#define TRACE_PC_DATA   0x0008  /* Trace PC with Data Access */
#define TRACE_EXCTRC    0x0010  /* Trace EXTRC Events */
#define TRACE_CPI       0x0020  /* Trace CPI Events */
#define TRACE_EXC       0x0040  /* Trace EXC Events */
#define TRACE_SLEEP     0x0080  /* Trace SLEEP Events */
#define TRACE_LSU       0x0100  /* Trace LSU Events */
#define TRACE_FOLD      0x0200  /* Trace FOLD Events */
#define TRACE_CYCCNT    0x0400  /* Trace CYCCNT Events */
#define PC_SAMPLE       0x0800  /* PC Sample via DWT_PCSR */
#define TPIU_FORMAT     0x1000  /* Use TPIU Formater */
#define ETM_TRACE       0x2000  /* Use ETM Trace */
#define UNLIMITED_TRACE 0x8000  /* Unlimited Trace */
#define TRACE_USE_CORECLK 0x00010000  /* 02.04.2019: Use System Clock instead of TPIU Clock */

// Trace Configuration
struct TRACECONF {
  DWORD Opt;                    // Trace Options
  DWORD Clk;                    // System Clock
  BYTE  Protocol;               // TPIU Pin Protocol
  BYTE  PortSize;               // TPIU Port Size (Sync Trace Port)
  WORD  SWV_Pre;                // SWV Prescaler ([0..12]+1,bit15=Auto)
  BYTE  TS_Pre;                 // Timestamp Prescaler (0=1,1=4,2=16,3=64)
  BYTE  CYC_Pre;                // CYCTAP Prescaler (([3..0]+1)*64,bit4*16)
  DWORD ITM_Ena;                // ITM Trace Enable
  DWORD ITM_Priv;               // ITM Trace Privilege
  DWORD TPIU_Clk;               // TPIU (Trace) Clock  /* 02.04.2019 */
};

extern struct TRACECONF TraceConf;

extern DWORD  TraceOpt;         // Trace Options (Active TraceConf.Opt)

extern  BOOL    T_Secure;       // Trace Security State (only if pio->bSecureExt)
extern  BOOL    TraceCycDwt;    // DWT is Ref Count for Trace Cycles

// #define TRACE_BASELINE_SUPP  (TRACE_ENABLE|TPIU_FORMAT|ETM_TRACE)
#define TRACE_BASELINE_SUPP  (TRACE_ENABLE|TPIU_FORMAT|ETM_TRACE|TRACE_USE_CORECLK)  // 02.04.2019

extern DWORD  TPIU_Clock;       // TPIU Clock (Active TraceConf.Clk/TraceConf.TPIUClk)

#endif
