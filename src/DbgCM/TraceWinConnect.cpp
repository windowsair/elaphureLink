/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.5
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2015-2016, 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM Cortex-M Trace Data Window Connection Module
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

#include "..\BOM.h"
#include "..\TraceView\TraceDataIf.h"
#include "..\TraceView\TraceDataTypes.h"
#include "TraceTypes.h"

#include "Collect.h"
#include "Debug.h"
#include "Trace.h"
#include "ETB.h"

// Function to save item into trace buffer in Trace.cpp
extern void  TR_Save (TR_ITEM *tr);

/* Global Variables */
bool                    TraceIf_Initialized = false;
TRACE_INTERFACE_INIT_64 TraceIfInit64;


#define TRACE_REC_TYPE_MAX 8  // there are 8 supported types
static LPCTSTR s_TraceRecTypeNames[TRACE_REC_TYPE_MAX] = {
  _T("ETM"),           // TR_ETM
  _T("ITM"),           // TR_ITM
  _T("Counter Event"), // TR_EVT
  _T("Exception"),     // TR_EXCTRC
  _T("PC Sample"),     // TR_PC_SAMPLE
  _T("Data Read"),     // TR_DATA_READ
  _T("Data Write"),    // TR_DATA_WRITE
  _T("ETM"),           // TR_ETM_HLL
};

static LPCTSTR s_TraceColNames[TRACE_COL_MAX] = {
  _T(/*Unused*/""),              // TRACE_COL_INDEX
  _T("Time"),                    // TRACE_COL_TIME
  _T(/*Unused*/""),              // TRACE_COL_CYCLES
  _T("Address / Port"),          // TRACE_COL_ADDR_PORT
  _T(/*Unused*/""),              // TRACE_COL_OPCODE
  _T("Instruction / Data"),      // TRACE_COL_INSTR_DATA
  _T("Src Code / Trigger Addr"), // TRACE_COL_SRC_TRGADDR
  _T("Function"),                // TRACE_COL_FUNCTION
};

static INT32 s_TraceColWidthDef[TRACE_COL_MAX] = {
  75,  // Index
  135, // Time
  130, // Cycles
  95,  // Address
  70,  // Opcode
  230, // Instruction
  200, // src code
  150, // function
};


#define DISPLAY_FILTERS_MAX 11
#define TRACE_COL_BITS_SUPPORTED (TRACE_COL_BIT_TIME | TRACE_COL_BIT_ADDR_PORT | TRACE_COL_BIT_INSTR_DATA | TRACE_COL_BIT_SRC_TRGADDR | TRACE_COL_BIT_FUNCTION)

// name, type mask, column mask
static TRACE_DISPLAY_FILTER s_DisplayFilters[DISPLAY_FILTERS_MAX]  = {
  { _T("All"),                   0xFF,                                      TRACE_COL_BITS_SUPPORTED, 0x1}, // TRACE_FILTER_FULL
  { _T("ETM - Code Exec"),       (TR_TYPE_ETM | TR_TYPE_ETM_HLL),           TRACE_COL_BITS_SUPPORTED, 0x1}, // TRACE_FILTER_ETM
  { _T("ETM - Code Exec HLL"),   TR_TYPE_ETM_HLL,                           TRACE_COL_BITS_SUPPORTED, 0x1}, // TRACE_FILTER_ETM_HLL
  { _T("ITM - All"),             (TR_TYPE_ITM | TR_TYPE_EVT | TR_TYPE_EXCTRC | TR_TYPE_PC_SAMPLE | TR_TYPE_DATA_READ | TR_TYPE_DATA_WRITE), TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_ALL
  { _T("ITM - SW Stimuli"),      TR_TYPE_ITM,                               TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_SW
  { _T("ITM - Event Counters"),  TR_TYPE_EVT,                               TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_CNT
  { _T("ITM - Exceptions"),      TR_TYPE_EXCTRC,                            TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_EXC
  { _T("ITM - PC Samples"),      TR_TYPE_PC_SAMPLE,                         TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_PC
  { _T("ITM - Data Read"),       TR_TYPE_DATA_READ,                         TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_READ
  { _T("ITM - Data Write"),      TR_TYPE_DATA_WRITE,                        TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_WRITE
  { _T("ITM - Data Read/Write"), (TR_TYPE_DATA_READ | TR_TYPE_DATA_WRITE),  TRACE_COL_BITS_SUPPORTED, 0x0}, // TRACE_FILTER_ITM_RW
};

#define DISPLAY_FILTERS_UNLIMITED_MAX 2
static TRACE_DISPLAY_FILTER s_DisplayFiltersUnlimited[DISPLAY_FILTERS_UNLIMITED_MAX]  = {
  { _T("Instruction Trace"),       0xFF,            TRACE_COL_BITS_SUPPORTED, 0x1}, // TRACE_FILTER_FULL
  { _T("Instruction Trace HLL"),   TR_TYPE_ETM_HLL, TRACE_COL_BITS_SUPPORTED, 0x1}, // TRACE_FILTER_ETM_HLL
};

#define FIXED_EXC_VEC_MAX 16
static LPCTSTR s_FixedExceptionVecNames[FIXED_EXC_VEC_MAX] = {
  _T(""),
  _T("Reset"),
  _T("NMI"),
  _T("HardFault"),
  _T("MemManage"),
  _T("BusFault"),
  _T("UsageFault"),
  _T(""),
  _T(""),
  _T(""),
  _T(""),
  _T("SVCall"),
  _T("DbgMon"),
  _T(""),
  _T("PendSV"),
  _T("SysTick"),
};

#define GENERIC_EXC_PREFIX _T("ExtIRQ ")

// Trace Display Window Variables

//synchronization objects
bool bEtmRunningSync  = false;
bool bItmRunningSync  = false;
bool bEtmRunningAsync = false;
bool bItmRunningAsync = false;
bool bProcessETM      = false;

__inline static bool TimestampsAvailable() {
  //---TODO: Implement detection of the availability of timestamps
  DEVELOP_MSG("Todo: \nImplement TimestampsAvailable() function");
  // - Return true if there is at least one timestamp in the current buffer.
  // - Return false if there is no timestamp in the buffer. uVision expects
  // "fake timestamps" instead and display of timestamps is disabled.
  // Providing "fake timestamps" keeps window performance up.

  return false;
}


void TraceStatusUpdate(bool bState, bool bUpdateView, bool bExplicitRange) {
  TRACE_STATUS TraceStatus;
  TRACE_RECORD_RANGE RecordRange;

  if (!TraceIf_Initialized) {
    return;
  }

  memset(&TraceStatus, 0, sizeof(TraceStatus));
  memset(&RecordRange, 0, sizeof(RecordRange));

  if (PlayDead) {
    // Debugger is shutting down, do not trigger another trace window update,
    //  this will only cause trouble, especially if we lost the target connection...
    bProcessETM = false;
    return;
  }

  // Change in overall state
  if (!bState) {
    //Target stopped processing
    if (bProcessETM/*TraceConf.Opt & ETM_TRACE*/) {
      bProcessETM = false; // Reset the state
    }

    //---TODO: Implement anything necessary, e.g. synchronize ETM and ITM records based on timestamps
    DEVELOP_MSG("Todo: \nDo anything necessary before updating display \nafter processing trace, \ne.g. synchronize ETM and ITM records by timestamp");
  }

  TraceStatus.bProcessing     = (bState) ? 1 : 0;
  TraceStatus.bHideTimestamps = !TimestampsAvailable();
  TraceStatus.bSkipViewUpdate = (bUpdateView) ? 0 : 1;
  if (bExplicitRange) {
    TraceStatus.bExplicitRange = 1;
    //---TODO: Set trace record cycle range to show after status update
    DEVELOP_MSG("Todo: \nSet trace record cycle range to show after status update");
    // RecordRange.nFirstCycle = ...
    // RecordRange.nLastCycle  = ...
    TraceStatus.m_pRecordRange = &RecordRange;
  }
  pio->Notify(UV_TRACE_IF_STATUS, (void*)(&TraceStatus));
}


bool IsTraceRunning() {
  return (bEtmRunningSync || bItmRunningSync || bEtmRunningAsync || bItmRunningAsync);
}


bool IsEtmRunning() {
  return (bEtmRunningSync || bEtmRunningAsync);
}

bool IsItmRunning() {
  return (bItmRunningSync || bItmRunningAsync);
}

// bState - Processing state, bWinUpdate - update trace win contents
void SetTraceRunSynch(bool bState, bool bWinUpdate) {
  bool bOldOverallRunState = IsTraceRunning();

  if (!TraceIf_Initialized) {
    return;
  }

  if (!(TraceConf.Opt & TRACE_ENABLE)) {
    return;
  }

  if (bState) {
    // Set target running
    if (TraceConf.Opt & ETM_TRACE) {
      bEtmRunningSync = true;
      bProcessETM     = true;
    } else {
      bProcessETM     = false;
    }

    bItmRunningSync = true;

  } else {
    bEtmRunningSync = false;
    bItmRunningSync = false;
  }

  //Send status update if state changed
  if (bOldOverallRunState != IsTraceRunning()) {
    TraceStatusUpdate((bState && !ETB_Configured), bWinUpdate, ((TraceOpt & UNLIMITED_TRACE) != 0));
  }
};


// bState - Processing state, bWinUpdate - update trace win contents
void SetTraceRunAsynch(bool bState, bool bWinUpdate) {
  bool bOldOverallRunState = IsTraceRunning();

  if (!TraceIf_Initialized) {
    return;
  }

  if (!(TraceConf.Opt & TRACE_ENABLE)) {
    return;
  }

  if (bState) {
    if (TraceConf.Opt & ETM_TRACE) {
      bEtmRunningAsync = true;
      bProcessETM      = true;
    } else {
      bProcessETM      = false;
    }

    bItmRunningAsync = true;

  } else {
    bEtmRunningAsync = false;
    bItmRunningAsync = false;
  }

  //Send status update if state changed
  if (bOldOverallRunState != IsTraceRunning()) {
    TraceStatusUpdate((bState && !ETB_Configured), bWinUpdate, ((TraceOpt & UNLIMITED_TRACE) != 0));
  }
};



// bState - Processing state, bWinUpdate - update trace win contents
void SetETMRunningSynch(bool bState, bool bWinUpdate) {
  bool bOldOverallRunState = IsTraceRunning();

  if (!TraceIf_Initialized) {
    return;
  }

  if (!(TraceConf.Opt & ETM_TRACE) && !bEtmRunningSync) { // Also check flag in case ETM was disabled during target run
    return;
  }

  if (bState) {
    bEtmRunningSync = true;
    bProcessETM     = true;
  } else {
    bEtmRunningSync = false;
  }

  //Send status update if state changed
  if (bOldOverallRunState != IsTraceRunning()) {
    TraceStatusUpdate((bState && !ETB_Configured), bWinUpdate, ((TraceOpt & UNLIMITED_TRACE) != 0));
  }
}


// bState - Processing state, bWinUpdate - update trace win contents
void SetITMRunningSynch(bool bState, bool bWinUpdate) {
  bool bOldOverallRunState = IsTraceRunning();

  if (!TraceIf_Initialized) {
    return;
  }

  if (bState) {
    bItmRunningSync = true;
  } else {
    bItmRunningSync = false;
  }

  //Send status update if state changed
  if (bOldOverallRunState != IsTraceRunning()) {
    TraceStatusUpdate((bState && !ETB_Configured), bWinUpdate, ((TraceOpt & UNLIMITED_TRACE) != 0));
  }
}

// bState - Processing state, bWinUpdate - update trace win contents
void SetETMRunningAsynch(bool bState, bool bWinUpdate) {
  bool bOldOverallRunState = IsTraceRunning();

  if (!TraceIf_Initialized) {
    return;
  }

  if (!(TraceConf.Opt & ETM_TRACE) && !bEtmRunningAsync) { // Also check flag in case ETM was disabled during target run
    return;
  }

  if (bState) {
    bEtmRunningAsync = true;
    bProcessETM      = true;
  } else {
    bEtmRunningAsync = false;
  }

  //Send status update if state changed
  if (bOldOverallRunState != IsTraceRunning()) {
    TraceStatusUpdate((bState && !ETB_Configured), bWinUpdate, ((TraceOpt & UNLIMITED_TRACE) != 0));
  }
}


// bState - Processing state, bWinUpdate - update trace win contents
void SetITMRunningAsynch(bool bState, bool bWinUpdate) {
  bool bOldOverallRunState = IsTraceRunning();

  if (!TraceIf_Initialized) {
    return;
  }

  if (bState) {
    bItmRunningAsync = true;
  } else {
    bItmRunningAsync = false;
  }

  //Send status update if state changed
  if (bOldOverallRunState != IsTraceRunning()) {
    TraceStatusUpdate((bState && !ETB_Configured), bWinUpdate, ((TraceOpt & UNLIMITED_TRACE) != 0));
  }
}

// Trace Display Window Initialization
void TraceWinInit (void) {
#if DBGCM_FEATURE_TRCDATA_WIN
  // Trace Data View support not fully ported yet
#endif // DBGCM_FEATURE_TRCDATA_WIN
}

// Trace Display Window Clear
void TraceWinClr (void) {
#if DBGCM_FEATURE_TRCDATA_WIN
  // Trace Data View support not fully ported yet
#endif // DBGCM_FEATURE_TRCDATA_WIN
}


// Record ITM Trace into Trace Display
void ITM_Record (TRC_DATAS *pItem) {

  if ((pItem->nSampTyp == TR_TYPE_EVT) && (pItem->nData == 0)) return;

#if DBGCM_FEATURE_TRCDATA_WIN
  // Trace Data View support not fully ported yet
#else // DBGCM_FEATURE_TRCDATA_WIN
  TR_ITEM tr;
  tr.addr = pItem->nAddr;
  tr.data = pItem->nData;
  tr.size = pItem->nDataS;
  tr.tcyc = pItem->nCycles;
  tr.type = pItem->nSampTyp;

  TR_Save (&tr);
#endif // DBGCM_FEATURE_TRCDATA_WIN

}


// Record Flow information in Trace Display
//   nFlags : The flag is a TR_TYPE_FLOW_xxx flag
//   cyc    : Not supported yet
void Flow_Record (BYTE nFlags, BOOL cyc) {

  if (TraceIfInit64.nVersionUV < 2) {
    // Skip this part for older trace interface versions
    return;
  }

  if ((nFlags & (TR_TYPE_FLOW_OVF|TR_TYPE_FLOW_TRCENA|TR_TYPE_FLOW_DBG|TR_TYPE_FLOW_TRIG|TR_TYPE_FLOW_GAP)) == 0) {
    // unsupported flow type
    return;
  }

#if DBGCM_FEATURE_TRCDATA_WIN
  // Trace Data View support not fully ported yet
#endif // DBGCM_FEATURE_TRCDATA_WIN

  return;
}




/************************************************************/
/*                                                          */
/*    TraceDataIf implementation                            */
/*                                                          */
/************************************************************/

TRACE_FILTER_CONF FilterConf;
TRACE_FILTER_CONF FilterConfAll;

#define TRACE_FILTER_CONF_ALL (TR_TYPE_ETM | TR_TYPE_ITM | TR_TYPE_EVT | TR_TYPE_EXCTRC | TR_TYPE_PC_SAMPLE | TR_TYPE_DATA_READ | TR_TYPE_DATA_WRITE | TR_TYPE_ETM_HLL)

void InitTraceInterface() {
  FilterConf.nTypeMask       = TRACE_FILTER_CONF_ALL;
  FilterConf.nExtTypeMask    = TR_TYPE_EXT_FLOW;
  FilterConfAll.nTypeMask    = TRACE_FILTER_CONF_ALL;
  FilterConfAll.nExtTypeMask = TR_TYPE_EXT_FLOW;

  memset(&TraceIfInit64, 0, sizeof(TRACE_INTERFACE_INIT_64));
  TraceIfInit64.nVersion               = TDI_VERSION;
  // Supported/Required Interface Functions. All other function pointers left to NULL.
  TraceIfInit64.pGetFirstCycle         = &GetFirstCycle;
  TraceIfInit64.pGetLastCycle          = &GetLastCycle;
  TraceIfInit64.pClearTrace            = &ClearTrace;
  TraceIfInit64.pGetTraceRecords64     = &GetTraceRecords64;
  TraceIfInit64.pGetFiltTraceRecords64 = &GetFiltTraceRecords64;

  // TDI_VERSION 2 extensions
  //TraceIfInit64.nVersionUV             = 0;// done by memset, finally set by uVision
  TraceIfInit64.pUpdateTraceBuffer = &UpdateTraceBuffer;
  TraceIfInit64.bUnlimitedTrace    = ((TraceConf.Opt & UNLIMITED_TRACE) && (TraceConf.Protocol == TPIU_TRACE_PORT)) ? 1 : 0;

  pio->Notify(UV_TRACE_IF_INIT, (void*) &TraceIfInit64);

  TraceIf_Initialized = true;
}


void ConfigureTraceWin() {
  TRACE_REC_TYPE_NAMES TraceRecTypeNames;
  memset(&TraceRecTypeNames, 0, sizeof(TRACE_REC_TYPE_NAMES));
  TraceRecTypeNames.m_nSize  = TRACE_REC_TYPE_MAX;
  TraceRecTypeNames.m_pNames = s_TraceRecTypeNames;

  TRACE_DISPLAY_FILTER_CONFIG TraceDispFiltConfig;
  memset (&TraceDispFiltConfig, 0, sizeof(TRACE_DISPLAY_FILTER_CONFIG));

  if (TraceConf.Opt & UNLIMITED_TRACE && TraceConf.Protocol != TPIU_ETB) { // Cannot use TraceOpt, not updated yet
    TraceDispFiltConfig.m_nFilters = DISPLAY_FILTERS_UNLIMITED_MAX;
    TraceDispFiltConfig.m_pFilters = s_DisplayFiltersUnlimited;
  } else {
    TraceDispFiltConfig.m_nFilters = DISPLAY_FILTERS_MAX;
    TraceDispFiltConfig.m_pFilters = s_DisplayFilters;
  }

  TRACE_COLUMN_INFO TraceColumnInfo;
  memset(&TraceColumnInfo, 0, sizeof(TRACE_COLUMN_INFO));
  TraceColumnInfo.m_nColumns        = TRACE_COL_MAX;
  TraceColumnInfo.m_pColumnNames    = s_TraceColNames;
  TraceColumnInfo.m_pColumnWidthDef = s_TraceColWidthDef;

  TRACE_EXCEPTION_CONFIG TraceExceptionConfig;
  memset(&TraceExceptionConfig, 0, sizeof(TRACE_EXCEPTION_CONFIG));
  TraceExceptionConfig.m_ExceptionMasks.m_nExcNumPos   = 0;
  TraceExceptionConfig.m_ExceptionMasks.m_nExcNumMask  = TR_EXC_NUM_MSK;
  TraceExceptionConfig.m_ExceptionMasks.m_nExcTypePos  = TR_EXC_TYP_POS;
  TraceExceptionConfig.m_ExceptionMasks.m_nExcTypeMask = TR_EXC_TYP_MSK;
  TraceExceptionConfig.m_nFixedVectors         = FIXED_EXC_VEC_MAX;
  TraceExceptionConfig.m_nFixedVectorOffset    = 0;
  TraceExceptionConfig.m_pFixedVectorNames     = s_FixedExceptionVecNames;
  TraceExceptionConfig.m_nGenericVectorOffset  = FIXED_EXC_VEC_MAX;
  TraceExceptionConfig.m_pGenericeVectorPrefix = GENERIC_EXC_PREFIX;

  TRACE_WIN_CONFIG TraceWinConfig;
  memset(&TraceWinConfig, 0, sizeof(TRACE_WIN_CONFIG));
  TraceWinConfig.m_pTraceRecTypeNames    = &TraceRecTypeNames;
  TraceWinConfig.m_pDisplayFiltConf      = &TraceDispFiltConfig;
  TraceWinConfig.m_pTraceColumnInfo      = &TraceColumnInfo;
  TraceWinConfig.m_pTraceExceptionConfig = &TraceExceptionConfig;

  //flags
  TraceWinConfig.m_bSuppressAddrType     = 0;

  pio->Notify(UV_TRACE_WIN_CONFIG, (void*) &TraceWinConfig);
}


INT32 GetFirstCycle(UINT64 *nFirstCycle) {
  if (!TraceIf_Initialized) {
    return TDI_ERR_NOT_SUPPORTED;
  }

  if (IsTraceRunning()) {
    return TDI_ERR_PROCESSING;
  }

  if (nFirstCycle == NULL) {
    return TDI_ERR_INVALID_PARAM;
  }

  //---TODO: Implement Trace Interface function GetFirstCycle()
  DEVELOP_MSG("Todo: \nImplement Trace Interface Function GetFirstCycle()");

  // *nFirstCycle = ...
  return TDI_ERR_SUCCESS;
}


INT32 GetLastCycle(UINT64 *nLastCycle) {
  if (!TraceIf_Initialized) {
    return TDI_ERR_NOT_SUPPORTED;
  }

  if (IsTraceRunning()) {
    return TDI_ERR_PROCESSING;
  }

  if (nLastCycle == NULL) {
    return TDI_ERR_INVALID_PARAM;
  }

  //---TODO: Implement Trace Interface function GetLastCycle()
  DEVELOP_MSG("Todo: \nImplement Trace Interface Function GetLastCycle()");

  // *nLastCycle = ...
  return TDI_ERR_SUCCESS;
}


INT32 ClearTrace() {
  if (!TraceIf_Initialized) {
    return TDI_ERR_NOT_SUPPORTED;
  }

  if (IsTraceRunning()) {
    return TDI_ERR_PROCESSING;
  }

  //---TODO: Implement Trace Interface function ClearTrace()
  DEVELOP_MSG("Todo: \nImplement Trace Interface Function ClearTrace()");

  return TDI_ERR_SUCCESS;
}


INT32 GetTraceRecords64(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, TRACE_RECORD_64 *pTrace64) {
  if (!TraceIf_Initialized) {
    return TDI_ERR_NOT_SUPPORTED;
  }

  if (IsTraceRunning()) {
    return TDI_ERR_PROCESSING;
  }

  if (nFirstCycle == NULL || nLastCycle == NULL || nCount == NULL) {
    return TDI_ERR_INVALID_PARAM;
  }

  //---TODO: Implement Trace Interface function GetTraceRecords64()
  DEVELOP_MSG("Todo: \nImplement Trace Interface Function GetTraceRecords64()");

  //--- Is requested cycle range out of captured bounds?
  // *nCount = 0;
  // return TDI_ERR_BOUNDS;

  //--- Does trace data exist for the given cycle range?
  // return TDI_ERR_EMPTY;

  //--- Find start of requested range
  // *nFirstCycle = ...

  //--- Is pTrace64 == NULL: Just count
  // *nCount = ...
  // *nLastCycle = ...
  // return TDI_ERR_SUCCESS;

  //--- Is pTrace64 != NULL: Copy and count
  // Copy records
  // *nCount = ...
  // *nLastCycle = ...
  // return TDI_ERR_SUCCESS;

  return TDI_ERR_SUCCESS;
}


INT32 GetFiltTraceRecords64(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, UINT64 *nRecordOfs, TRACE_FILTER_CONF* pTraceFilter, TRACE_RECORD_64 *pTrace64) {
  if (!TraceIf_Initialized) {
    return TDI_ERR_NOT_SUPPORTED;
  }

  if (IsTraceRunning()) {
    return TDI_ERR_PROCESSING;
  }

  if (nFirstCycle == NULL || nLastCycle == NULL || nCount == NULL || nRecordOfs == NULL) {
    return TDI_ERR_INVALID_PARAM;
  }

  //---TODO: Implement Trace Interface function GetFiltTraceRecords64()
  DEVELOP_MSG("Todo: \nImplement Trace Interface Function GetFiltTraceRecords64()");

  //---TODO: The following is a fallback for uVision before MDK 5.14. For MDK 5.15 and
  //         later, this workaround shall be replaced by the corresponding functionality
  //         in UpdateTraceBuffer().
  //  if (*nRecordOfs == (UINT64)(-1)) {
  //    if (TraceOpt & UNLIMITED_TRACE) {
  //
  //      //---TODO: Load trace window buffer with requested cycle range from unlimited trace storage
  //      DEVELOP_MSG("Todo: \nGetFiltTraceRecords(): Load trace data from unlimited trace storage");
  //
  //      //--- No records loaded?
  //      // return TDI_ERR_EMPTY;
  //   } else {
  //     return TDI_ERR_NOT_SUPPORTED;
  //   }
  // }


  if (pTraceFilter == NULL) {
    //---TODO: Apply FilterConfAll filter
  } else {
    //---TODO: Apply pTraceFilter filter
  }

  //--- Is requested cycle range out of captured bounds?
  // *nCount = 0;
  // return TDI_ERR_BOUNDS;

  //--- Does trace data exist for the given cycle range?
  // return TDI_ERR_EMPTY;

  //--- Find start of requested range
  // *nFirstCycle = ...

  //--- Is pTrace64 == NULL: Just count
  // *nCount = ...
  // *nLastCycle = ...
  // return TDI_ERR_SUCCESS;

  //--- Is pTrace64 != NULL: Copy and count
  // Copy records
  // *nCount = ...
  // *nLastCycle = ...
  // return TDI_ERR_SUCCESS;

  return TDI_ERR_SUCCESS;
}

INT32 UpdateTraceBuffer(UINT64 *nFirstCycle, UINT64 *nLastCycle) {
  if (!TraceIf_Initialized) {
    return TDI_ERR_NOT_SUPPORTED;
  }

  if (PlayDead) {
    return TDI_ERR_GENERAL;
  }

  //---TODO: Implement Trace Interface function UpdateTraceBuffer()
  DEVELOP_MSG("Todo: \nImplement Trace Interface Function UpdateTraceBuffer()");

  //---TODO: Can update trace buffer?
  // return TDI_ERR_PROCESSING;

  //---TODO: Load trace data into window buffer
  //
  // - Don't care about nFirstCycle and nLastCycle in ETB case. In this case
  //   it shall simply read the data from the target buffer.
  //
  // - For "Unlimited Trace", nFirstCycle contains the cycle for the first record
  //   to load into the buffer. nLastCycle normally contains the cycle of the very
  //   last record which can be loaded into the buffer (as reported by GetLastCycle()).
  //   The idea is to have as much data in the buffer as possible. This can reduce the
  //   decoding effort for navigating through subsequent time windows.
  //   Implementations of ULINK drivers normally provide a window buffer of 2^20 records.
  //   In any case it should be large enough to host records for a 10ms time window.
  //
  // return TDI_ERR_EMPTY; // If no data was available for requested range
  //
  // TraceStatusUpdate(false, true, false); // Should not be necessary for Unlimited Trace case.
  //                                           uVision will trigger a window update by itself after this call.

  return TDI_ERR_SUCCESS;
}

void TraceWinConnect() {
  TraceIf_Initialized = false;
  TraceIfInit64 = TRACE_INTERFACE_INIT_64();
  bEtmRunningSync  = false;
  bItmRunningSync  = false;
  bEtmRunningAsync = false;
  bItmRunningAsync = false;
  bProcessETM      = false;
  FilterConf = TRACE_FILTER_CONF();
  FilterConfAll = TRACE_FILTER_CONF();
}
