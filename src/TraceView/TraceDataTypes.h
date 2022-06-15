/**************************************************************************//**
 *           TraceDataTypes.h: Data types and other definitions for
 *                             Trace Data Interface.
 *
 * @version  V2.0.3
 * @date     $Date: 2016-04-26 14:33:20 +0200 (Tue, 26 Apr 2016) $
 *
 * @note
 * Copyright (C) 2011-2015 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision.
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

#ifndef __TRACEDATATYPES_H__
#define __TRACEDATATYPES_H__

#define TDI_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#pragma pack(push,1)

/* Trace Record Type */
#define TR_TYPE_FLOW         0x00    // Trace Flow (synch, etc.), for TRACE_RECORD and TRACE_RECORD_64
#define TR_TYPE_ETM          0x01    // ETM (Instruction or Data)
#define TR_TYPE_ITM          0x02    // ITM (SW Stimulus)
#define TR_TYPE_EVT          0x04    // Event (Counters)
#define TR_TYPE_EXCTRC       0x08    // Exception Trace
#define TR_TYPE_PC_SAMPLE    0x10    // PC Sample
#define TR_TYPE_DATA_READ    0x20    // Data Read
#define TR_TYPE_DATA_WRITE   0x40    // Data Write
#define TR_TYPE_ETM_HLL      0x80    // ETM HLL

/* Extended Record Type Mask */
#define TR_TYPE_EXT_FLOW     0x01    // Type for extended display filter mask

/* Trace Flow Record Type */
#define TR_TYPE_FLOW_OVF     0x000000001   // Trace recovered from overflow
#define TR_TYPE_FLOW_TRCENA  0x000000002   // Trace is enabled (start/stop, trace event)
#define TR_TYPE_FLOW_DBG     0x000000004   // Trace capture leaves DEBUG state
#define TR_TYPE_FLOW_TRIG    0x000000008   // Trace capture received a trigger packet
#define TR_TYPE_FLOW_GAP     0x000000010   // Trace capture has a gap, e.g. due to a buffer wraparound


/* TR_TYPE_EVT Sub-Types (used in TRACE_RECORD_64::nData) */
#define TR_EVT_CPI   0x01            // Event CPI   (CPICNT,   8-bit)
#define TR_EVT_EXC   0x02            // Event EXC   (EXCCNT,   8-bit)
#define TR_EVT_SLEEP 0x04            // Event SLEEP (SLEEPCNT, 8-bit)
#define TR_EVT_LSU   0x08            // Event LSU   (LSUCNT,   8-bit)
#define TR_EVT_FOLD  0x10            // Event FOLD  (FOLDCNT,  8-bit)
#define TR_EVT_CYC   0x20            // Event Cycle (POSTCNT,  32-bit)


/* Exception Trace Types */
typedef enum {
  TR_EXC_INVALID = 0,                // Exception Type Invalid
  TR_EXC_ENTRY,                      // Exception Entry
  TR_EXC_EXIT,                       // Exception Exit
  TR_EXC_RETURN,                     // Exception Return
} TR_EXC_TYPE;


#define TR_EXC_TYP_POS         12    // Exception Type Bit Position (Cortex-M)
#define TR_EXC_TYP_MSK       0x03    // Exception Type Bit Mask (Cortex-M)
#define TR_EXC_NUM_MSK      0x1FF    // Exception Number Mask (Cortex-M)


#define TR_ETM_PKGCNT_MSK  0xFFFF    // ETM Package Count Mask (TRACE_RECORD::nData/TRACE_RECORD_64::nData)

/* Generic Trace Record used for both ITM and ETM records */
typedef struct TraceRecord_t {
  UINT8  nType;         /* Trace Record Type */

  /* Flags */
  UINT8  bTS       : 1; /* Indicates if nCycle shall be displayed as the record's timestamp. */
  UINT8  bPC       : 1; /* ETM: 0 - Data Trace Record (ETMv4)
                                1 - Instruction Trace Record, nPC holds valid PC value.
                           ITM - PC Sample: nPC has valid PC value.
                           ITM - Data Read/Write: nPC has valid PC value. Ignored if bExec is also set.
                           Other: Unused.
                        */
  UINT8  bAddr     : 1; /* ETM - Data Trace: nAddr holds data access address.
                           ITM - SW Stimulus: nAddr holds ITM channel number.
                           ITM - Data Read/Write: nAddr holds the start address of the monitored range.
                           Other: Unused.
                        */
  UINT8  nDataS    : 2; /* ETM - Data Trace: Data Access Size (0 : None, 1 : 1 Byte, 2 : 2 Bytes, 3 : 4 Bytes).
                           ITM - SW Stimulus,
                           ITM - Data Read/Write: Data Access Size (0 : None, 1 : 1 Byte, 2 : 2 Bytes, 3 : 4 Bytes).
                                 Data is stored in nData, LSB first.
                           ITM - Event Counter: Always '1'.
                           ITM - Exception Trace: Always '2'.
                           ITM - PC Sample: Always '0'.
                           Other: Unused.
                        */
  UINT8  bExec     : 1; /* ETM - Instruction Trace: (Conditional) Instruction executed.
                           ETM - Data Trace: nPC holds data access value. See also nDataS.
                           ITM - Data Read/Write: nPC holds exact data access address (comparator value + address offset).
                           Other: Unused.
                        */
  UINT8  bTD       : 1; /* Timestamp is delayed */
  UINT8  bOvf      : 1; /* First record after recovery from trace data overflow */

  UINT32 nPC;           /* ETM - Instruction Trace: Instruction Address.
                           ETM - Data Trace: Data Access Value.
                           ITM - PC Sample: Instruction Address.
                           ITM - Data Read/Write:
                                 bPC == 1 => nPC holds PC which caused data access
                                 bExec == 1 => nPC holds exact data access address address (comparator value + address offset).
                                 bPC wins over bExec.
                           Other: Unused.
                        */
  UINT32 nAddr;         /* ETM - Data Trace: Data Access Address.
                           ITM - SW Stimulus: ITM Channel/Port Number.
                           ITM - Data Read/Write: Start address of monitored range (DWT comparator value)
                           Other: Unused.
                        */
  UINT32 nData;         /* Flow Record: Flow record sub-type.
                           ITM - SW Stimulus,
                           ITM - Data Read/Write: Data value.
                           ITM - Event Counter: Event Counter ID.
                           ITM - Exception Trace: Exception Type and Number.
                           ETM - Data Trace (bPC == 0):
                                   Bit  [0..15]: Offset into ETM Package.
                                   Bit [16..23]: Transfer Index if multiple data trace packets for one instruction trace packet.
                                   Bit [24..30]: Reserved
                                   Bit     [31]: Endianess, 0 - Little Endian, 1 - Big Endian
                           ETM - Instruction Trace (bPC == 1):
                                   Bit  [0..15]: Offset into ETM Package.
                                   Bit     [16]: bNS  - Record has valid Security State Info
                                   Bit     [17]: nNS  - Security State if bNS == 1 (0 - Secure State, 1 - Non-secure State)
                                   Bit     [18]: bEL  - Record has valid Exception Level Info
                                   Bit [19..20]: nEL  - Exception Level Info (v7-M: 0b00 - Thread Mode, 0b11 - Handler Mode)
                                   Bit [21..22]: nCRT - Conditional Result Type for non-branch instructions (ETMv4).
                                                        0 - No result available.
                                                        1 - Pass/Fail result (see Bit [23]).
                                                        2 - APSR condition flags (see Bits [23..26]).
                                   Bit     [23]: nCRT == 1 => 0 - Condition failed, 1 - condition passed
                                                 nCRT == 2 => V Flag
                                   Bit     [24]: nCRT == 2 => C Flag
                                   Bit     [25]: nCRT == 2 => Z Flag
                                   Bit     [26]: nCRT == 2 => N Flag
                                   Bit [27..31]: Reserved
                           Other: Unused.
                        */
  UINT32 nCycle;        /* Cycle count relative to the start cycle of a GetTraceRecords()/GetFiltTraceRecords()
                           request. Set bTS if this value shall be displayed.
                        */
} TRACE_RECORD;
// 18 bytes


// 64-bit cycle counter version of the above
typedef struct TraceRecord64_t {
  UINT8  nType;         /* Trace Record Type (64-Bit cycle count) */

  /* Flags */
  UINT8  bTS       : 1; /* Indicates if nCycle shall be displayed as the record's timestamp. */
  UINT8  bPC       : 1; /* ETM: 0 - Data Trace Record (ETMv4)
                                1 - Instruction Trace Record, nPC holds valid PC value.
                           ITM - PC Sample: nPC has valid PC value.
                           ITM - Data Read/Write: nPC has valid PC value. Ignored if bExec is also set.
                           Other: Unused.
                        */
  UINT8  bAddr     : 1; /* ETM - Data Trace: nAddr holds data access address.
                           ITM - SW Stimulus: nAddr holds ITM channel number.
                           ITM - Data Read/Write: nAddr holds the start address of the monitored range.
                           Other: Unused.
                        */
  UINT8  nDataS    : 2; /* ETM - Data Trace: Data Access Size (0 : None, 1 : 1 Byte, 2 : 2 Bytes, 3 : 4 Bytes).
                           ITM - SW Stimulus,
                           ITM - Data Read/Write: Data Access Size (0 : None, 1 : 1 Byte, 2 : 2 Bytes, 3 : 4 Bytes).
                                 Data is stored in nData, LSB first.
                           ITM - Event Counter: Always '1'.
                           ITM - Exception Trace: Always '2'.
                           ITM - PC Sample: Always '0'.
                           Other: Unused.
                        */
  UINT8  bExec     : 1; /* ETM - Instruction Trace: (Conditional) Instruction executed.
                           ETM - Data Trace: nPC holds data access value. See also nDataS.
                           ITM - Data Read/Write: nPC holds exact data access address (comparator value + address offset).
                           Other: Unused.
                        */
  UINT8  bTD       : 1; /* Timestamp is delayed */
  UINT8  bOvf      : 1; /* First record after recovery from trace data overflow */

  UINT32 nPC;           /* ETM - Instruction Trace: Instruction Address.
                           ETM - Data Trace: Data Access Value.
                           ITM - PC Sample: Instruction Address.
                           ITM - Data Read/Write:
                                 bPC == 1 => nPC holds PC which caused data access
                                 bExec == 1 => nPC holds exact data access address address (comparator value + address offset).
                                 bPC wins over bExec.
                           Other: Unused.
                        */
  UINT32 nAddr;         /* ETM - Data Trace: Data Access Address.
                           ITM - SW Stimulus: ITM Channel/Port Number.
                           ITM - Data Read/Write: Start address of monitored range (DWT comparator value)
                           Other: Unused.
                        */
  UINT32 nData;         /* Flow Record: Flow record sub-type.
                           ITM - SW Stimulus,
                           ITM - Data Read/Write: Data value.
                           ITM - Event Counter: Event Counter ID.
                           ITM - Exception Trace: Exception Type and Number.
                           ETM - Data Trace (bPC == 0):
                                   Bit  [0..15]: Offset into ETM Package.
                                   Bit [16..23]: Transfer Index if multiple data trace packets for one instruction trace packet.
                                   Bit [24..30]: Reserved
                                   Bit     [31]: Endianess, 0 - Little Endian, 1 - Big Endian
                           ETM - Instruction Trace (bPC == 1):
                                   Bit  [0..15]: Offset into ETM Package.
                                   Bit     [16]: bNS  - Record has valid Security State Info
                                   Bit     [17]: nNS  - Security State if bNS == 1 (0 - Secure State, 1 - Non-secure State)
                                   Bit     [18]: bEL  - Record has valid Exception Level Info
                                   Bit [19..20]: nEL  - Exception Level Info (v7-M: 0b00 - Thread Mode, 0b11 - Handler Mode)
                                   Bit [21..22]: nCRT - Conditional Result Type for non-branch instructions (ETMv4).
                                                        0 - No result available.
                                                        1 - Pass/Fail result (see Bit [23]).
                                                        2 - APSR condition flags (see Bits [23..26]).
                                   Bit     [23]: nCRT == 1 => 0 - Condition failed, 1 - condition passed
                                                 nCRT == 2 => V Flag
                                   Bit     [24]: nCRT == 2 => C Flag
                                   Bit     [25]: nCRT == 2 => Z Flag
                                   Bit     [26]: nCRT == 2 => N Flag
                                   Bit [27..31]: Reserved
                           Other: Unused.
                        */
  UINT64 nCycle;       /* Cycle count relative to the start cycle of a GetTraceRecords64()/GetFiltTraceRecords64()
                           request. Set bTS if this value shall be displayed.
                       */
} TRACE_RECORD_64;
// 22 bytes



/************** Enums used by Trace Data View *************/
typedef enum TraceColumnNameIdx {
  TRACE_COL_INDEX = 0,    // Index in currently loaded debug driver DLL buffer
	TRACE_COL_TIME,         // Time in seconds
	TRACE_COL_CYCLES,       // Cycles
	TRACE_COL_ADDR_PORT,    // Memory Address/Port Number (ITM)
	TRACE_COL_OPCODE,       // Opcode
	TRACE_COL_INSTR_DATA,   // Disassembled Instruction/Data Value
	TRACE_COL_SRC_TRGADDR,  // Source Code/Trigger Address (DWT Data Trace)
  TRACE_COL_FUNCTION,     // Function Name
	TRACE_COL_MAX,          // Max value (not a valid column index)
} TRACE_COLUMN_NAME_IDX;


// Masks for trace column bits
#define TRACE_COL_BIT_INDEX       (1 << TRACE_COL_INDEX)
#define TRACE_COL_BIT_TIME        (1 << TRACE_COL_TIME)
#define TRACE_COL_BIT_CYCLES      (1 << TRACE_COL_CYCLES)
#define TRACE_COL_BIT_ADDR_PORT   (1 << TRACE_COL_ADDR_PORT)
#define TRACE_COL_BIT_OPCODE      (1 << TRACE_COL_OPCODE)
#define TRACE_COL_BIT_INSTR_DATA  (1 << TRACE_COL_INSTR_DATA)
#define TRACE_COL_BIT_SRC_TRGADDR (1 << TRACE_COL_SRC_TRGADDR)
#define TRACE_COL_BIT_FUNCTION    (1 << TRACE_COL_FUNCTION)


/************** Trace Data View Column Settings ***********/

typedef struct TraceColumnInfo_t {
  UINT32   m_nColumns;        // Number of m_pColumnNames entries (should be TRACE_COL_MAX)
  LPCTSTR *m_pColumnNames;    // Column names to display in table header/find type
                              // box. Empty strings disable columns.
  INT32   *m_pColumnWidthDef; // Default column width, overridden by layout in
                              // uvgui/uvguix file.
} TRACE_COLUMN_INFO;


/************** Trace Display Filters **************/

// Trace record type names
typedef struct TraceRecTypeNames_t {
  UINT32   m_nSize;        // Size of record type name array
  LPCTSTR *m_pNames;       // Array of record type names, indexing refers to bit index of the
                           // corresponding type (see defines above). Empty strings indicate unsupported types.
  UINT32   m_nReserved[6]; // Reserved
} TRACE_REC_TYPE_NAMES;

// Single display filter
typedef struct TraceDisplayFilter_t {
  LPCTSTR m_pName;         // Display Filter Name
  UINT8   m_nTypeMask;     // Display Filter Type Mask (see defines above)
  UINT32  m_nColumnMask;   // Columns to display for this filter (bit mask using TRACE_COLUMN_NAME_IDX for bit position)
  UINT8   m_nExtTypeMask;  // Extended Display Filter Type Mask
  UINT8   m_nReserved8[2]; // Reserved
  UINT32  m_nReserved[13]; // Reserved
} TRACE_DISPLAY_FILTER;


// Display filter configuration, i.e. the set of available filters
typedef struct TraceDisplayFilterConfig_t {
  UINT32                m_nFilters; // Number of filter configurations
  TRACE_DISPLAY_FILTER *m_pFilters; // Filter configuration array, filter at index 0 is selected by default
} TRACE_DISPLAY_FILTER_CONFIG;


// Exception type and number masks
typedef struct TraceExceptionMasks_t {
  UINT8  m_nExcNumPos;     // Bit position of exception number information
  UINT32 m_nExcNumMask;    // Mask for exception number information
  UINT8  m_nExcTypePos;    // Bit position of exception type information
  UINT32 m_nExcTypeMask;   // Mask for exception type information
  UINT8  m_nReserved8[2];  // Reserved
  UINT32 m_nReserved[5];   // Reserved
} TRACE_EXCEPTION_MASKS;


// Exception configuration
typedef struct TraceExceptionConfig_t {
  UINT32                m_nFixedVectors;               // Number of fixed vectors
  INT32                 m_nFixedVectorOffset;          // Offset to fixed vectors
  LPCTSTR              *m_pFixedVectorNames;           // String array to contain the fixed vector names
  INT32                 m_nGenericVectorOffset;        // Offset to be subtracted from an exception vector ID, not supported if < 0
  LPCTSTR               m_pGenericeVectorPrefix;       // String to prepend to the ID of a generic exception vector
  TRACE_EXCEPTION_MASKS m_ExceptionMasks;              // Masks to extract exception number and type from a 32 bit value.
  UINT32                m_nReserved[10];               // Reserved
} TRACE_EXCEPTION_CONFIG;


/* Initialization Object for CTraceWin object in uVision (container for the new trace views),
   use with UV_TRACE_WIN_CONFIG-Notification. */
typedef struct TraceWinConfig_t {
  TRACE_REC_TYPE_NAMES        *m_pTraceRecTypeNames;      // Record Type Display Configuration
  TRACE_COLUMN_INFO           *m_pTraceColumnInfo;        // Display Column Configuration
  TRACE_DISPLAY_FILTER_CONFIG *m_pDisplayFiltConf;        // Display Filter Configuration
  TRACE_EXCEPTION_CONFIG      *m_pTraceExceptionConfig;   // Trace Exception Condfiguration
  UINT32                       m_bSuppressAddrType :  1;  // Suppress Address Qualifier in display
  UINT32                       m_nReservedBits     : 31;  // Reserved
  UINT32                       m_nReserved[27];           // Reserved
} TRACE_WIN_CONFIG;


/* Trace Win synchronization types. */
#define TWS_TYPE_NONE  0x00  // None, invalid
#define TWS_TYPE_CYCLE 0x01  // Synchronize to a cycle
#define TWS_TYPE_ADDR  0x02  // Synchronize to the next occurrence of the address (use in combination with TWS_TYPE_CYCLE)


/* Trace Win synchronization targets, i.e. the views to synchronize. */
#define TWS_VIEW_NONE  0x00  // None, invalid
#define TWS_VIEW_DATA  0x01  // Trace Data Window
#define TWS_VIEW_NAV   0x02  // Trace Navigation Window
#define TWS_VIEW_HLL   0x04  // Source Code Editor
#define TWS_VIEW_DASM  0x08  // Disassembly Window


/* Synchronization object for CTraceWin instance in uVision. Describes which views to synchronize
   and how. Use with UV_TRACE_WIN_SYNC-notification. */
typedef struct TraceWinSync_t {
  UINT32 m_nTypes;        // Synchronization type (see TWS_TYPE_xxx)
  UINT32 m_nViews;        // Synchronization targets (see TWS_VIEW_xxx)
  INT64  m_nCycle;        // Cycle value to synchronize to
  UINT32 m_nAddr;         // Address to synchronize to (if TWS_TYPE_ADDR)
  UINT32 m_nReserved[12]; // Reserved
} TRACE_WIN_SYNC;


// Trace Configuration sent by target
typedef struct TraceConfig_t {
  UINT32 nClock;
  UINT32 nReserved[23];
} TRACE_CONFIG;


/*
    Trace Filter Configuration
*/
typedef struct TraceFilterConf_t {
    UINT8  nTypeMask;              /* Filter Mask, refers to the available trace record types     */
    UINT8  nExtTypeMask;           /* Extended Filter Mask, refers to extended trace record types */
    UINT8  nReserved8[2];
    UINT32 nReserved32[31];        /* Reserved for future filter options */
#ifdef __cplusplus
  TraceFilterConf_t() {
    nTypeMask    = 0;
    nExtTypeMask = 0;
    memset(&nReserved8, 0, sizeof(nReserved8));
    memset(&nReserved32, 0, sizeof(nReserved32));
  }
#endif
} TRACE_FILTER_CONF;


// Clock sample, currently used to receive reference values from the simulator to catch adjusted
// time offsets due to clock changes.
typedef struct TraceClockSample_t {
  UINT32 m_nClock;       // Clock value of the current sample
  INT64  m_nLastCycle;   // Cycle value of the sample
  DOUBLE m_dLastTime;    // Time value of the sample (use as reference)
  UINT32 m_nReserved;    // Reserved
} TRACE_CLOCK_SAMPLE;


// Cycle range, used for explicit record range passed with TRACE_STATUS struct
typedef struct TraceRecordRange_t {
  UINT64 nFirstCycle;    // First cycle in cycle range
  UINT64 nLastCycle;     // Last cycle in cycle range
  UINT32 nReserved[8];   // Reserved
} TRACE_RECORD_RANGE;


// Trace Status Update Structure
typedef struct TraceStatus_t {
  UINT32         bProcessing     : 1; // Target running/driver processing trace data
  UINT32         bHideTimestamps : 1; // Hide timestamps in trace data window
  UINT32         bSkipViewUpdate : 1; // Skip update of related window (e.g. Unlimited Trace buffer update for save operation)
  UINT32         bExplicitRange  : 1; // Show specific cycle range (m_pRecordRange)
  UINT32         nReservedBits   : 4; // Reserved
  BYTE             nReservedBytes[3]; // Reserved
  TRACE_CLOCK_SAMPLE *m_pClockSample; // Clock sample, can be NULL
  TRACE_RECORD_RANGE *m_pRecordRange; // Record range to show, use with bExplicitRange flag
  UINT32                nReserved[7]; // Reserved
} TRACE_STATUS;



// Error codes for Trace Data Interface functions
#define TDI_ERR_SUCCESS         0x00  // Success
#define TDI_ERR_INVALID_PARAM   0x01  // Invalid input parameter
#define TDI_ERR_BOUNDS          0x02  // Requested information out of boundaries
#define TDI_ERR_EMPTY           0x03  // Requested cycle range is empty
#define TDI_ERR_NOT_SUPPORTED   0x04  // Requested action not supported
#define TDI_ERR_GENERAL         0x05  // Not further specified error
#define TDI_ERR_PROCESSING      0x06  // Operation not possible while processing
#define TDI_ERR_TIMESTAMP       0x07  // Detected timestamp issue
#define TDI_ERR_CYC_32BIT       0x08  // Requested cycle range > 32-bit (only for deprecated access functions GetTraceRecords() and GetFiltTraceRecords())
#define TDI_ERR_STORAGE_CORRUPT 0x09  // Trace storage in AGDI DLL corrupted and no longer usable


// Function pointer definitions for Trace Data interface functions.
// See TraceLowLevelIf.h for further descriptions of the interface functions.
typedef INT32 (*SetTraceConfig_f)       (TRACE_CONFIG* pTraceConfig);
typedef INT32 (*GetTraceConfig_f)       (TRACE_CONFIG* pTraceConfig);
typedef INT32 (*GetFirstCycle_f)        (UINT64* nFirstCycle);
typedef INT32 (*GetLastCycle_f)         (UINT64* nLastCycle);
typedef INT32 (*GetTraceRecords_f)      (UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, TRACE_RECORD *pTrace);
typedef INT32 (*GetFiltTraceRecords_f)  (UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, UINT64 *nRecordOfs,
                                         TRACE_FILTER_CONF* pTraceFilter, TRACE_RECORD *pTrace);
typedef INT32 (*GetTraceRecords64_f)    (UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, TRACE_RECORD_64 *pTrace64);
typedef INT32 (*GetFiltTraceRecords64_f)(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, UINT64 *nRecordOfs,
                                         TRACE_FILTER_CONF* pTraceFilter, TRACE_RECORD_64 *pTrace64);
typedef INT32 (*ClearTrace_f)           (void);
typedef INT32 (*UpdateTraceBuffer_f)    (UINT64 *nFirstCycle, UINT64 *nLastCycle);


// Trace Interface Init Structure (Deprecated, use TRACE_INTERFACE_INIT_64).
typedef struct TraceInterfaceInit_t {
  INT32 nVersion;       // The interface version, for this struct this is can only be 1.

  SetTraceConfig_f      pSetTraceConfig;
  GetTraceConfig_f      pGetTraceConfig;
  GetFirstCycle_f       pGetFirstCycle;
  GetLastCycle_f        pGetLastCycle;
  GetTraceRecords_f     pGetTraceRecords;
  // Optional, beneficial if filtering is done more efficiently on the lower level
  GetFiltTraceRecords_f pGetFiltTraceRecords;
  ClearTrace_f          pClearTrace;

#ifdef __cplusplus
  TraceInterfaceInit_t () {
    nVersion             = 1;        // Deprecated, TDI_VERSION 1
    pSetTraceConfig      = NULL;
    pGetTraceConfig      = NULL;
    pGetFirstCycle       = NULL;
    pGetLastCycle        = NULL;
    pGetTraceRecords     = NULL;
    pGetFiltTraceRecords = NULL;
    pClearTrace          = NULL;
  }
#endif

} TRACE_INTERFACE_INIT;

// Trace Interface Init Structure
typedef struct TraceInterfaceInit64_t {
  INT32 nVersion;        // The interface version in the driver, set this to TDI_VERSION after initializing struct

  SetTraceConfig_f        pSetTraceConfig;            // Obsolete, do not use
  GetTraceConfig_f        pGetTraceConfig;            // Obsolete, do not use
  GetFirstCycle_f         pGetFirstCycle;             // Get cycle count of oldest available record
  GetLastCycle_f          pGetLastCycle;              // Get cycle count of latest available record
  GetTraceRecords_f       pGetTraceRecords;           // Deprecated, do not use
  GetFiltTraceRecords_f   pGetFiltTraceRecords;       // Deprecated, do not use
  ClearTrace_f            pClearTrace;                // Clear current trace buffer in debug driver DLL
  GetTraceRecords64_f     pGetTraceRecords64;         // Get records in cycle range
  GetFiltTraceRecords64_f pGetFiltTraceRecords64;     // Get records in cycle range with optional offset into range

  // Below are extensions specific to TDI_VERSION 2 and later
  INT32 nVersionUV;       /* Version of the trace interface in UV. Initialize to 0. From TDI_VERSION 2 on, UV sets this
                             during initialization. If left to 0, this means TDI_VERSION 1 is used. */

  UpdateTraceBuffer_f     pUpdateTraceBuffer;         // Update trace buffer in debug driver DLL

  // Interface Capabilities
  DWORD bUnlimitedTrace      :  1;                    // Unlimited Trace Enabled
  DWORD bEmbeddedTraceBuffer :  1;                    // Use Embedded Trace Buffer
  DWORD                      : 30;                    // Reserved

  DWORD        nReserved[19];                         // Reserved (new for TDI_VERSION == 2, no longer compatible with TDI_VERSION 1)

#ifdef __cplusplus
  TraceInterfaceInit64_t () {
    nVersion               = 0;
    pSetTraceConfig        = NULL;
    pGetTraceConfig        = NULL;
    pGetFirstCycle         = NULL;
    pGetLastCycle          = NULL;
    pGetTraceRecords       = NULL;
    pGetFiltTraceRecords   = NULL;
    pClearTrace            = NULL;
    pGetTraceRecords64     = NULL;
    pGetFiltTraceRecords64 = NULL;
    pUpdateTraceBuffer     = NULL;
    bUnlimitedTrace        = 0;
    bEmbeddedTraceBuffer   = 0;
    nVersionUV             = 0;
  }
#endif

} TRACE_INTERFACE_INIT_64;


#pragma pack(pop)


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__TRACEDATATYPES_H__