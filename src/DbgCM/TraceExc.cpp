/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.6
 * @date     $Date: 2017-11-17 17:57:54 +0100 (Fri, 17 Nov 2017) $
 *
 * @note
 * Copyright (C) 2009-2017 ARM Limited. All rights reserved.
 *
 * @brief     Display of Trace Exceptions
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
#include "Debug.h"
#include "Trace.h"
#include "TraceExc.h"





#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define GETBOOL(x) ((x)? true : false)



int ExcTrc(excTrcData_t *excTrcData)
{
  if(PlayDead) return(0);

  // must be executed also when Trace is disabled
  switch(excTrcData->excTrcCmd) {
    case EXCTRC_GETIRQLINES:
      ExcTrcGetIRQLines (excTrcData);
      break;

    case EVTCNT_DELROW:
      EvtCntResetData (excTrcData);
      break;

    case EXCTRC_GET_CFG:
      ExcTrcGetConfig(excTrcData);
      break;

    case EVTCNT_INIT:
      break;

    case EXCTRC_INIT:
      break;

    case EXCTRC_DELROW:
      ExcTrcResetData (excTrcData);
      break;

    case EXCTRC_RESET:
      excTrcData->row = -1;
      ExcTrcResetData (excTrcData);
      break;

    case EVTCNT_RESET:
      excTrcData->row = -1;
      EvtCntResetData (excTrcData);
      break;

    case EXCTRC_SET_CFG:
      ExcTrcSetConfig(excTrcData);
      break;

    default:
      break;
  }

  if (!(TraceConf.Opt & TRACE_ENABLE)) {
    excTrcData->bTraceConfigValid = true;
    excTrcData->bTraceEn = false;
    return(0);
  }

  // must be executed when Trace is enabled
  switch(excTrcData->excTrcCmd) {
    case EXCTRC_UPDATE:
      ExcTrcGetData   (excTrcData);
      break;

    case EVTCNT_UPDATE:
      EvtCntGetData   (excTrcData);
      break;

    case EXCTRC_NONE:
    default:
      break;
  }

  return(0);
}



void ExcTrcGetIRQLines(excTrcData_t *excTrcData)
{
  excTrcData->extIRQLines = NumIRQ;
}


int ExcTrcGetData(excTrcData_t *excTrcData)
{
  unsigned int i, update=0;

  for (i=0; i<=excTrcData->numOfLastIrq; i++) {

#if 0
    // ---  This generates demo Data ! ---
    TraceExcData[i].count    = 42 * i;
    TraceExcData[i].ttotal   = 42 * i;
    TraceExcData[i].tinmin   = 42 * i;
    TraceExcData[i].tinmax   = 42 * i;
    TraceExcData[i].toutmin  = 42 * i;
    TraceExcData[i].toutmax  = 42 * i;
    TraceExcData[i].tfirst   = 42 * i;
    // -----------------------------------
#endif

    if (excTrcData->TraceExcDisp[i].count   != TraceExcData[i].count)     { excTrcData->TraceExcDisp[i].count   = excTrcData->bTraceEn? TraceExcData[i].count   : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].ttotal  != TraceExcData[i].ttotal)    { excTrcData->TraceExcDisp[i].ttotal  = excTrcData->bTraceEn? TraceExcData[i].ttotal  : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].tinmin  != TraceExcData[i].tinmin)    { excTrcData->TraceExcDisp[i].tinmin  = excTrcData->bTraceEn? TraceExcData[i].tinmin  : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].tinmax  != TraceExcData[i].tinmax)    { excTrcData->TraceExcDisp[i].tinmax  = excTrcData->bTraceEn? TraceExcData[i].tinmax  : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].toutmin != TraceExcData[i].toutmin)   { excTrcData->TraceExcDisp[i].toutmin = excTrcData->bTraceEn? TraceExcData[i].toutmin : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].toutmax != TraceExcData[i].toutmax)   { excTrcData->TraceExcDisp[i].toutmax = excTrcData->bTraceEn? TraceExcData[i].toutmax : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].tfirst  != TraceExcData[i].tfirst)    { excTrcData->TraceExcDisp[i].tfirst  = excTrcData->bTraceEn? TraceExcData[i].tfirst  : 0; update = 1;  }
    if (excTrcData->TraceExcDisp[i].tenter  != TraceExcData[i].tenter)    { excTrcData->TraceExcDisp[i].tenter  = excTrcData->bTraceEn? TraceExcData[i].tenter  : 0; update = 1;  }
  }

  excTrcData->bUpdatedExcTrc = update;

  return(0);
}


void ExcTrcResetData(excTrcData_t *excTrcData)
{
  unsigned int i, row;

  row = excTrcData->row;

  if(row != -1) {
    i = row;

    //EnterCriticalSection(&TraceCS);  // critical section begins
    if(!TryEnterCriticalSection(&TraceCS)) return;
    TraceExcData[i].count   =  0;
    TraceExcData[i].tenter  = -1.0;
    TraceExcData[i].texit   = -1.0;
    TraceExcData[i].tin     =  0.0;
    TraceExcData[i].ttotal  =  0.0;
    TraceExcData[i].tinmin  = -1.0;
    TraceExcData[i].tinmax  = -1.0;
    TraceExcData[i].toutmin = -1.0;
    TraceExcData[i].toutmax = -1.0;
    TraceExcData[i].tfirst  = -1.0;
    LeaveCriticalSection(&TraceCS);  // critical section ends
  }
  else {
    //EnterCriticalSection(&TraceCS);  // critical section begins
    if(!TryEnterCriticalSection(&TraceCS)) return;
    for (i = 0; i < EXC_NUM; i++) {
      TraceExcData[i].count   =  0;
      TraceExcData[i].tenter  = -1.0;
      TraceExcData[i].texit   = -1.0;
      TraceExcData[i].tin     =  0.0;
      TraceExcData[i].ttotal  =  0.0;
      TraceExcData[i].tinmin  = -1.0;
      TraceExcData[i].tinmax  = -1.0;
      TraceExcData[i].toutmin = -1.0;
      TraceExcData[i].toutmax = -1.0;
      TraceExcData[i].tfirst  = -1.0;
    }
    LeaveCriticalSection(&TraceCS);  // critical section ends
  }
}




static I64 CntCPI;
static I64 CntExc;
static I64 CntSleep;
static I64 CntLSU;
static I64 CntFold;

static int init = 1;

void EvtCntGetData(excTrcData_t *excTrcData)
{

  I64  CPI;
  I64  Exc;
  I64  Sleep;
  I64  LSU;
  I64  Fold;

  if(init) {
    init = 0;
    CPI   = -1LL;
    Exc   = -1LL;
    Sleep = -1LL;
    LSU   = -1LL;
    Fold  = -1LL;

    memset(excTrcData->EventCountDisp, 0, EVTCNT_NUM * sizeof(EVTCNT_ENTRY));
  }

  if (TraceConf.Protocol != TPIU_ETB) { // JR, 14.09.2012: Deactivate for ETB, accumulation not possible here

#if DBGCM_V8M
    if (TraceConf.Opt & TRACE_CPI) {
      ReadD32(DWT_CPICNT, &RegDWT.CPICNT, BLOCK_SECTYPE_ANY);
      CPI = (INT64)(RegDWT.CPICNT) + ((INT64)TraceCntCPI << 8);
    } else {
      CPI = -1LL;
    }
    if (TraceConf.Opt & TRACE_EXC) {
      ReadD32(DWT_EXCCNT, &RegDWT.EXCCNT, BLOCK_SECTYPE_ANY);
      Exc = (INT64)(RegDWT.EXCCNT) + ((INT64)TraceCntExc << 8);
    } else {
      Exc = -1LL;
    }
    if (TraceConf.Opt & TRACE_SLEEP) {
      ReadD32(DWT_SLEEPCNT, &RegDWT.SLEEPCNT, BLOCK_SECTYPE_ANY);
      Sleep = (INT64)(RegDWT.SLEEPCNT) + ((INT64)TraceCntSleep << 8);
    } else {
      Sleep = -1LL;
    }
    if (TraceConf.Opt & TRACE_LSU) {
      ReadD32(DWT_LSUCNT, &RegDWT.LSUCNT, BLOCK_SECTYPE_ANY);
      LSU = (INT64)(RegDWT.LSUCNT) + ((INT64)TraceCntLSU << 8);
    } else {
      LSU = -1LL;
    }
    if (TraceConf.Opt & TRACE_FOLD) {
      ReadD32(DWT_FOLDCNT, &RegDWT.FOLDCNT, BLOCK_SECTYPE_ANY);
      Fold = (INT64)(RegDWT.FOLDCNT) + ((INT64)TraceCntFold << 8);
    } else {
      Fold = -1LL;
    }
#else // DBGCM_V8M
    if (TraceConf.Opt & TRACE_CPI) {
      ReadD32(DWT_CPICNT, &RegDWT.CPICNT);
      CPI = (INT64)(RegDWT.CPICNT) + ((INT64)TraceCntCPI << 8);
    } else {
      CPI = -1LL;
    }
    if (TraceConf.Opt & TRACE_EXC) {
      ReadD32(DWT_EXCCNT, &RegDWT.EXCCNT);
      Exc = (INT64)(RegDWT.EXCCNT) + ((INT64)TraceCntExc << 8);
    } else {
      Exc = -1LL;
    }
    if (TraceConf.Opt & TRACE_SLEEP) {
      ReadD32(DWT_SLEEPCNT, &RegDWT.SLEEPCNT);
      Sleep = (INT64)(RegDWT.SLEEPCNT) + ((INT64)TraceCntSleep << 8);
    } else {
      Sleep = -1LL;
    }
    if (TraceConf.Opt & TRACE_LSU) {
      ReadD32(DWT_LSUCNT, &RegDWT.LSUCNT);
      LSU = (INT64)(RegDWT.LSUCNT) + ((INT64)TraceCntLSU << 8);
    } else {
      LSU = -1LL;
    }
    if (TraceConf.Opt & TRACE_FOLD) {
      ReadD32(DWT_FOLDCNT, &RegDWT.FOLDCNT);
      Fold = (INT64)(RegDWT.FOLDCNT) + ((INT64)TraceCntFold << 8);
    } else {
      Fold = -1LL;
    }
#endif // DBGCM_V8M

  } else {
    CPI   = -1LL;
    Exc   = -1LL;
    Sleep = -1LL;
    LSU   = -1LL;
    Fold  = -1LL;
  }

  if (CntCPI != CPI) {
    CntCPI = CPI;
    excTrcData->bUpdatedEvtCnt = true;
    excTrcData->EventCountDisp[EVTCNT_CPI].value = CPI;
  }
  if (CntExc != Exc) {
    CntExc = Exc;
    excTrcData->bUpdatedEvtCnt = true;
    excTrcData->EventCountDisp[EVTCNT_Exc].value = Exc;
  }
  if (CntSleep != Sleep) {
    CntSleep = Sleep;
    excTrcData->bUpdatedEvtCnt = true;
    excTrcData->EventCountDisp[EVTCNT_Sleep].value = Sleep;
  }
  if (CntLSU != LSU) {
    CntLSU = LSU;
    excTrcData->bUpdatedEvtCnt = true;
    excTrcData->EventCountDisp[EVTCNT_LSU].value = LSU;
  }
  if (CntFold != Fold) {
    CntFold = Fold;
    excTrcData->bUpdatedEvtCnt = true;
    excTrcData->EventCountDisp[EVTCNT_Fold].value = Fold;
  }
}



void EvtCntResetData(excTrcData_t *excTrcData)
{
  unsigned int row;

  row = excTrcData->row;

  if(row != -1) {
    switch(row) {
      case EVTCNT_CPI:
        CpiClr();
        break;

      case EVTCNT_Exc:
        ExcClr();
        break;

      case EVTCNT_Sleep:
        SleepClr();
        break;

      case EVTCNT_LSU:
        LsuClr();
        break;

      case EVTCNT_Fold:
        FoldClr();
        break;

      default:
        break;
    }
  }
  else {
    CpiClr();
    ExcClr();
    SleepClr();
    LsuClr();
    FoldClr();
  }

  EvtCntGetData(excTrcData);
}


void ExcTrcGetConfig   (excTrcData_t *excTrcData)
{
  excTrcData->bTraceEn    = GETBOOL(TraceConf.Opt & TRACE_ENABLE    );
  excTrcData->bEnExcTrc   = GETBOOL(TraceConf.Opt & TRACE_EXCTRC    );
  excTrcData->bEnTS       = GETBOOL(TraceConf.Opt & TRACE_TIMESTAMP );

  excTrcData->bEnCPI      = GETBOOL(TraceConf.Opt & TRACE_CPI       );
  excTrcData->bEnEXC      = GETBOOL(TraceConf.Opt & TRACE_EXC       );
  excTrcData->bEnSLEEP    = GETBOOL(TraceConf.Opt & TRACE_SLEEP     );
  excTrcData->bEnLSU      = GETBOOL(TraceConf.Opt & TRACE_LSU       );
  excTrcData->bEnFOLD     = GETBOOL(TraceConf.Opt & TRACE_FOLD      );

  excTrcData->bTraceConfigValid = true;
}



void ExcTrcSetConfig   (excTrcData_t *excTrcData)
{
  if(excTrcData->bEnTS != GETBOOL(TraceConf.Opt & TRACE_TIMESTAMP)) {
    if(excTrcData->bEnTS) { TraceConf.Opt |= TRACE_TIMESTAMP;    TraceOpt |= TRACE_TIMESTAMP; }
    else                  { TraceConf.Opt &= ~TRACE_TIMESTAMP;   TraceOpt &= ~TRACE_TIMESTAMP; }
  }

  if(excTrcData->bEnExcTrc)   { TraceConf.Opt |= TRACE_EXCTRC; } else {  TraceConf.Opt &= ~TRACE_EXCTRC;  }

  if(excTrcData->bEnCPI   )   { TraceConf.Opt |= TRACE_CPI  ;  } else {  TraceConf.Opt &= ~TRACE_CPI  ;   }
  if(excTrcData->bEnEXC   )   { TraceConf.Opt |= TRACE_EXC  ;  } else {  TraceConf.Opt &= ~TRACE_EXC  ;   }
  if(excTrcData->bEnSLEEP )   { TraceConf.Opt |= TRACE_SLEEP;  } else {  TraceConf.Opt &= ~TRACE_SLEEP;   }
  if(excTrcData->bEnLSU   )   { TraceConf.Opt |= TRACE_LSU  ;  } else {  TraceConf.Opt &= ~TRACE_LSU  ;   }
  if(excTrcData->bEnFOLD  )   { TraceConf.Opt |= TRACE_FOLD ;  } else {  TraceConf.Opt &= ~TRACE_FOLD ;   }

  ITM_Reconfig();
}





void CpiClr(void) {
  RegDWT.CPICNT = 0;

#if DBGCM_V8M
  WriteD32(DWT_CPICNT, RegDWT.CPICNT, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  WriteD32(DWT_CPICNT, RegDWT.CPICNT);
#endif // DBGCM_V8M

  if(!TryEnterCriticalSection(&TraceCS)) return;
  TraceCntCPI = 0;
  LeaveCriticalSection(&TraceCS);
}

void ExcClr(void) {
  RegDWT.EXCCNT = 0;

#if DBGCM_V8M
  WriteD32(DWT_EXCCNT, RegDWT.EXCCNT, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  WriteD32(DWT_EXCCNT, RegDWT.EXCCNT);
#endif // DBGCM_V8M

  if(!TryEnterCriticalSection(&TraceCS)) return;
  TraceCntExc = 0;
  LeaveCriticalSection(&TraceCS);
}

void SleepClr(void) {
  RegDWT.SLEEPCNT = 0;

#if DBGCM_V8M
  WriteD32(DWT_SLEEPCNT, RegDWT.SLEEPCNT, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  WriteD32(DWT_SLEEPCNT, RegDWT.SLEEPCNT);
#endif // DBGCM_V8M

  if(!TryEnterCriticalSection(&TraceCS)) return;
  TraceCntSleep = 0;
  LeaveCriticalSection(&TraceCS);
}

void LsuClr(void) {
  RegDWT.LSUCNT = 0;

#if DBGCM_V8M
  WriteD32(DWT_LSUCNT, RegDWT.LSUCNT, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  WriteD32(DWT_LSUCNT, RegDWT.LSUCNT);
#endif // DBGCM_V8M

  if(!TryEnterCriticalSection(&TraceCS)) return;
  TraceCntLSU = 0;
  LeaveCriticalSection(&TraceCS);
}

void FoldClr(void) {
  RegDWT.FOLDCNT = 0;

#if DBGCM_V8M
  WriteD32(DWT_FOLDCNT, RegDWT.FOLDCNT, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  WriteD32(DWT_FOLDCNT, RegDWT.FOLDCNT);
#endif // DBGCM_V8M

  if(!TryEnterCriticalSection(&TraceCS)) return;
  TraceCntFold = 0;
  LeaveCriticalSection(&TraceCS);
}

void InitTraceExc() {
  CntCPI = 0;
  CntExc = 0;
  CntSleep = 0;
  CntLSU = 0;
  CntFold = 0;
  init = 1;
}
