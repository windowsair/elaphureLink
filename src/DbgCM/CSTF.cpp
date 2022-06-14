/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.1
 * @date     $Date: 2020-07-30 14:15:04 +0200 (Thu, 30 Jul 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM CoreSight Trace Funnel Module
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
#include "CSTF.h"
#include "..\BOM.h"
#include "..\AGDI.H"
#include "..\Alloc.h"
#include "Debug.h"
#include "Collect.h"
#include "PDSCDebug.h"

BOOL  CSTF_Single     = TRUE; // TRUE if <= 1 Trace Funnels in system

DWORD CSTF_Addr       = 0;
DWORD CSTF_Control    = 0;
BYTE  CSTF_ConnSlaves = 0x00; // Bit-field holding the ports which are connected (topology detection)
BYTE  CSTF_SlaveITM   = 0x00; // Bit-field indicating the slave 
BYTE  CSTF_SlaveETM   = 0x00; // Bit-field indicating the slave

static CSTF_Instance *CSTF_Instances = NULL;

static int CSTF_TopologyDetectStart() {
  int status;

  // Read CSTF control register and buffer it
#if DBGCM_V8M
  status = ReadD32(CSTF_CONTROL(CSTF_Addr), &CSTF_Control, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(CSTF_CONTROL(CSTF_Addr), &CSTF_Control);
#endif // DBGCM_V8M
  if (status) return status;

  // Enable integration mode for CSTF
#if DBGCM_V8M
  status = WriteD32(CSTF_ITATBMODE(CSTF_Addr), CSTF_ITATBMODE_EN, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CSTF_ITATBMODE(CSTF_Addr), CSTF_ITATBMODE_EN);
#endif // DBGCM_V8M
  if (status) return status;  // last time to immediately leave the function

  return (0);
}


static int CSTF_TopologyDetectStop() {
  int status, result = 0;

#if DBGCM_V8M
  status = WriteD32(CSTF_ITATBMODE(CSTF_Addr), 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CSTF_ITATBMODE(CSTF_Addr), 0);
#endif // DBGCM_V8M
  if (status && !result) result = status;

  // restore trace funnel control
#if DBGCM_V8M
  status = WriteD32(CSTF_CONTROL(CSTF_Addr), CSTF_Control, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CSTF_CONTROL(CSTF_Addr), CSTF_Control);
#endif // DBGCM_V8M
  if (status && !result) result = status;

  return (result);
}


// Uses the topology detection registers to determine
// the funnels slave port connected to the ETM v3
int CSTF_TopologyDetectETMv3() {
//DWORD val;
//int n;
  int /*status = 0,*/ result = 0;
//BOOL bDetected = FALSE;
#if 0
  if (ETM_Addr == 0) {
    return EU32; // not available
  }

  // Unlock ETM
  status = WriteD32(ETMv3_LOCKACCESS, ETM_UNLOCK);
  if (status) return (status);

  // Power-Up ETM
  status = ReadD32(ETMv3_CONTROL, &val);
  if (status) return (status);
  if (val & ETMv3_POWERDOWN) {
    val &= ~ETMv3_POWERDOWN;
    status = WriteD32(ETMv3_CONTROL, val);
  }

  // Enable Integration Mode
  status = WriteD32(ETMv3_ITCTRL, ETMv3_ITCTRL_EN);
  if (status) goto etm_detect_end;


  // Init signals
  for (n = 0; n < 8; n++) {

    if (CSTF_ConnSlaves & (1 << n)) {
      // already in use by other trace source
      continue;
    }

    // Set port to test
    status = WriteD32(CSTF_CONTROL(CSTF_Addr), (1UL << n), BLOCK_SECTYPE_ANY);
    if (status) goto etm_detect_end;

    // Init master and slave signals
    status = WriteD32(ETMv3_ITATBCTR0, 0);
    if (status) goto etm_detect_end;

    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), 0, BLOCK_SECTYPE_ANY);
    if (status) goto etm_detect_end;

    // Assert master valid
    status = WriteD32(ETMv3_ITATBCTR0, ETMv3_ITATBCTR0_ATVALID);
    if (status) goto etm_detect_end;

    // Check slave valid asserted
    status = ReadD32(CSTF_ITATBCTR0(CSTF_Addr), &val, BLOCK_SECTYPE_ANY);
    if (status) goto etm_detect_end;

    bDetected = (val & CSTF_ITATBCTR0_ATVALID);

    // Assert slave ready
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), CSTF_ITATBCTR2_ATREADY, BLOCK_SECTYPE_ANY);
    if (status) goto etm_detect_end;

    // Deassert master valid
    status = WriteD32(ETMv3_ITATBCTR0, 0);
    if (status) goto etm_detect_end;

    // Check slave valid deasserted
    status = ReadD32(CSTF_ITATBCTR0(CSTF_Addr), &val, BLOCK_SECTYPE_ANY);
    if (status) goto etm_detect_end;

    bDetected = bDetected && ((val & CSTF_ITATBCTR0_ATVALID) == 0);

    // Deassert slave ready
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), CSTF_ITATBCTR2_ATREADY, BLOCK_SECTYPE_ANY);
    if (status) goto etm_detect_end;

    if (bDetected) {
      CSTF_SlaveETM   |= (1 << n);
      CSTF_ConnSlaves |= CSTF_SlaveETM;
      break;
    }
  }

etm_detect_end:
  // Bring components back into functional mode
  status = WriteD32(ETMv3_ITCTRL, 0);
  if (status && !result) result = status;
#endif
  return result;
}


// Uses the topology detection registers to determine
// the funnels slave port connected to the ETM v4
int CSTF_TopologyDetectETMv4() {
  //DWORD val;
  //int n;
  int /*status = 0,*/ result = 0;
#if 0
  BOOL bDetected = FALSE;

  if (ETM_Addr == 0) {
    return EU32; // not available
  }

  // TODO: Check to what extent topology detection differs from v3 and implement
#endif
  return result;
}


// Uses the topology detection registers to determine
// the funnels slave port connected to the ETM
int CSTF_TopologyDetectITM() {
  DWORD val, ITM_TraceControl;
  int n;
  int status, result = 0;
  BOOL bDetected = FALSE;

  if (ITM_Addr == 0) {
    return EU32; // not available
  }

  ITM_TraceControl  = ITM_ITMENA | (ITM_ATBID << 16);
  // Setup ITM
#if DBGCM_V8M
  status = WriteD32(ITM_LOCKACCESS,     ITM_UNLOCK, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ITM_LOCKACCESS,     ITM_UNLOCK);
#endif // DBGCM_V8M
  if (status) return (status);
#if DBGCM_V8M
  status = WriteD32(ITM_TRACECONTROL,   ITM_TraceControl, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ITM_TRACECONTROL,   ITM_TraceControl);
#endif // DBGCM_V8M
  if (status) return (status);

  // Enable Integration Mode
#if DBGCM_V8M
  status = WriteD32(ITM_ITMODE, ITM_ITMODE_EN, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ITM_ITMODE, ITM_ITMODE_EN);
#endif // DBGCM_V8M
  if (status) goto itm_detect_end;


  // Init signals
  for (n = 0; n < 8; n++) {

    if (CSTF_ConnSlaves & (1 << n)) {
      // already in use by other trace source
      continue;
    }

    // Set port to test
#if DBGCM_V8M
    status = WriteD32(CSTF_CONTROL(CSTF_Addr), (1UL << n), BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(CSTF_CONTROL(CSTF_Addr), (1UL << n));
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    // Init master and slave signals
#if DBGCM_V8M
    status = WriteD32(ITM_ITWRITE, 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(ITM_ITWRITE, 0);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

#if DBGCM_V8M
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), 0);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    // Assert master valid
#if DBGCM_V8M
    status = WriteD32(ITM_ITWRITE, ITM_ITWRITE_ATVALIDM, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(ITM_ITWRITE, ITM_ITWRITE_ATVALIDM);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    // Check slave valid asserted
#if DBGCM_V8M
    status = ReadD32(CSTF_ITATBCTR0(CSTF_Addr), &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = ReadD32(CSTF_ITATBCTR0(CSTF_Addr), &val);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    bDetected = (val & CSTF_ITATBCTR0_ATVALID);

    // Assert slave ready
#if DBGCM_V8M
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), CSTF_ITATBCTR2_ATREADY, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), CSTF_ITATBCTR2_ATREADY);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    // Deassert master valid
#if DBGCM_V8M
    status = WriteD32(ITM_ITWRITE, 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(ITM_ITWRITE, 0);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    // Check slave valid deasserted
#if DBGCM_V8M
    status = ReadD32(CSTF_ITATBCTR0(CSTF_Addr), &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = ReadD32(CSTF_ITATBCTR0(CSTF_Addr), &val);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    bDetected = bDetected && ((val & CSTF_ITATBCTR0_ATVALID) == 0);

    // Deassert slave ready
#if DBGCM_V8M
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), CSTF_ITATBCTR2_ATREADY, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(CSTF_ITATBCTR2(CSTF_Addr), CSTF_ITATBCTR2_ATREADY);
#endif // DBGCM_V8M
    if (status) goto itm_detect_end;

    if (bDetected) {
      CSTF_SlaveITM   |= (1 << n);
      CSTF_ConnSlaves |= CSTF_SlaveITM;
      break;
    }
  }

itm_detect_end:
  // Bring components back into functional mode
#if DBGCM_V8M
  status = WriteD32(ITM_ITMODE, 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(ITM_ITMODE, 0);
#endif // DBGCM_V8M
  if (status && !result) result = status;

  return result;
}


static int CSTF_ConfigureSingle(DWORD ap, DWORD addr, DWORD connSlaves) {
  DWORD value = 0, ctrl = 0;
  int status  = 0, n;
  DWORD APSel;

  status = LinkCom(1);                     // 16.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (status); }

  APSel  = AP_Sel;                          // Save AP_Sel
  AP_Sel = ap << 24;                        // Switch to CSTF Access Port

  // Read funnel control register to get holdtime after reset
#if DBGCM_V8M
  status = ReadD32(CSTF_CONTROL(addr), &ctrl, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(CSTF_CONTROL(addr), &ctrl);
#endif // DBGCM_V8M
  if (status) goto end;

  // Disable all slave ports
#if DBGCM_V8M
  status = WriteD32(CSTF_CONTROL(addr), 0x00000000, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CSTF_CONTROL(addr), 0x00000000);
#endif // DBGCM_V8M
  if (status) goto end;

  // Set slave port priorities
  value = 0;
  for (n = 0; n < 8; n++) {
    if (connSlaves & (1 << n)) {
      value |= n << (3*n);
    }
  }
#if DBGCM_V8M
  status = WriteD32(CSTF_PRIORITY(addr), value, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CSTF_PRIORITY(addr), value);
#endif // DBGCM_V8M
  if (status) goto end;

  // Preserve Hold Time and enable desired slave ports
  ctrl = (ctrl & CSTF_CONTROL_HT_M) | connSlaves;
#if DBGCM_V8M
  status = WriteD32(CSTF_CONTROL(addr), ctrl, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CSTF_CONTROL(addr), ctrl);
#endif // DBGCM_V8M
  if (status) goto end;

end:
  AP_Sel = APSel;                             // Restore AP_Sel
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }
  return (0);
}


static int CSTF_ConfigureAll() {
  DWORD value = 0, ctrl = 0;
  int status  = 0;
  CSTF_Instance *cstf = CSTF_Instances;

  for (; cstf != NULL; cstf = cstf->next) {
    status = CSTF_ConfigureSingle(cstf->ap, cstf->addr, cstf->slavesConn);// Configure this instance
    if (status) return (status);
  }

  return (0);
}


static int CSTF_Configure() {
  int status  = 0;

  if (CSTF_Instances) {
    status = CSTF_ConfigureAll();             // Configure registered funnels
    if (status) return (status);

    return (0);
  }

  status = CSTF_ConfigureSingle(MonConf.AP, CSTF_Addr, CSTF_ConnSlaves);  // Configure one and only instance
  if (status) return (status);

  return (0);
}


// Add CSTF Port and Instance if required
int CSTF_AddSlavePort(DWORD addr, BYTE ap, DWORD dp, BYTE port) {
  CSTF_Instance* inst;

  inst = CSTF_GetInstance(addr, ap, dp);
  if (inst == NULL) {
    CSTF_AddInstance(addr, ap, dp);
    inst = CSTF_GetInstance(addr, ap, dp);
    if (inst == NULL) return (0);    // TODO: ERROR
  }

  inst->slavesConn |= (BYTE)(1 << port);
  return (0);
}


// Add CSTF Instance
int CSTF_AddInstance(DWORD addr, BYTE ap, DWORD dp) {
  CSTF_Instance* inst = CSTF_Instances;
  CSTF_Instance* tail = inst;

  for (; inst != NULL; inst = inst->next) {
    if (addr == inst->addr && ap == inst->ap && dp == inst->dp) {
      // CSTF instance already exists
      return (0);
    }
    tail = inst;
  }

  // Allocate memory for new instance
  inst = (CSTF_Instance*)pio->GetMem(sizeof(CSTF_Instance), ENV_DBM);
  memset(inst, 0, sizeof(CSTF_Instance));
  inst->addr     = addr;
  inst->ap       = ap;
  inst->dp       = dp;

  if (tail == NULL) {
    CSTF_Instances = inst;
  } else {
    tail->next     = inst;
  }

  return (0);
}


// Get CSTF Instance
CSTF_Instance* CSTF_GetInstance(DWORD addr, BYTE ap, DWORD dp) {
  CSTF_Instance* inst = CSTF_Instances;

  for (; inst != NULL; inst = inst->next) {
    if (addr == inst->addr && ap == inst->ap && dp == inst->dp) {
      return inst;
    }
  }

  return (NULL);
}


int CSTF_Setup() {
  DWORD value = 0;
  int status  = 0;

  if ((CSTF_Addr == 0 && CSTF_Instances == NULL) || (ETM_Addr == 0 && ITM_Addr == 0)) {
    // Trace Funnel not available, exit function with success
    return (0);
  }

#if DBGCM_DBG_DESCRIPTION
  if (PDSCDebug_HasSDF() && CSTF_Instances != NULL) {
    // Configure CSTFs with information from SDF file
    status = CSTF_Configure();
    if (status) return status;

    return (0);
  }
#endif // DBGCM_DBG_DESCRIPTION

  CSTF_ConnSlaves = 0; // Clear connected slaves field
  // Set CTSF into integration mode and save current settings
  status = CSTF_TopologyDetectStart();
  if (status) return status;

  if (ETM_Version == 3) {
    CSTF_TopologyDetectETMv3();
  } else if (ETM_Version == 4) {
    CSTF_TopologyDetectETMv4();
  }
  CSTF_TopologyDetectITM();

  // Set CTSF into functional mode and restore current settings
  status = CSTF_TopologyDetectStop();
  if (status) return status;

  // Configure CSTF with information from topology detection
  status = CSTF_Configure();
  if (status) return status;

  return (0);
}


DWORD CSTF_GetSlavesNum() {
  DWORD nSlaves   = 0;
  BYTE  nPortBits = CSTF_ConnSlaves;

  for (; nPortBits != 0; nPortBits = (nPortBits >> 1)) {
    if (nPortBits & 1) {
      nSlaves++;
    }
  }
  
  return nSlaves;
}


int CSTF_ETMConnected() {
  return ((CSTF_SlaveETM == 0) ? 0 : 1);
}


int CSTF_ITMConnected() {
  return ((CSTF_SlaveITM == 0) ? 0 : 1);
}


int CSTF_Recovery() {
  int status;

  if ((CSTF_Addr == 0 && CSTF_Instances == NULL) || (ETM_Addr == 0 && ITM_Addr == 0)) {
    // Trace Funnel not available, exit function with success
    return (0);
  }

  // Skip the topology detection for recovery (or taking from SDF anyway)
  status = CSTF_Configure();
  if (status) return (status);

  return (0);
}

void InitCSTF() {
  CSTF_Single     = TRUE;

  CSTF_Addr       = 0;
  CSTF_Control    = 0;
  CSTF_ConnSlaves = 0x00;
  CSTF_SlaveITM   = 0x00;
  CSTF_SlaveETM   = 0x00;

  CSTF_Instances  = NULL;
}
