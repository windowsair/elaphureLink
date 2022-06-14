/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.1
 * @date     $Date: 2020-07-30 14:15:04 +0200 (Thu, 30 Jul 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM Embedded Cross Trigger Interface Module
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
#include "..\BOM.h"
#include "..\Alloc.h"
#include "Collect.h"
#include "Debug.h"
#include "CTI.h"

static CTI_Instance *CTI_Instances = NULL;

// Add CTI Instance
int CTI_AddInstance(DWORD addr, BYTE ap, DWORD dp) {
  DWORD status, val, APSel;
  CTI_Instance* inst = CTI_Instances;
  CTI_Instance* tail = inst;

  for (; inst != NULL; inst = inst->next) {
    if (addr == inst->addr && ap == inst->ap && dp == inst->dp) {
      // CTI instance already exists
      return (0);
    }
    tail = inst;
  }

  status = LinkCom(1);                      // 08.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (status); }

  APSel  = AP_Sel;                          // Save AP_Sel
  AP_Sel = ap << 24;                        // Switch to CTI Access Port
#if DBGCM_V8M
  status = ReadD32(CTI_DEVID(addr), &val, BLOCK_SECTYPE_ANY);  // Read Device Configuration Register to determine
                                                               // number of channels and triggers
#else // DBGCM_V8M
  status = ReadD32(CTI_DEVID(addr), &val);  // Read Device Configuration Register to determine
                                            // number of channels and triggers
#endif // DBGCM_V8M
  AP_Sel = APSel;                           // Restore AP_Sel

  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  // Check Status
  if (status) { OutErrorMessage(status); return (status); }

  // Allocate memory for new instance
  inst = (CTI_Instance*)pio->GetMem(sizeof(CTI_Instance), ENV_DBM);
  memset(inst, 0, sizeof(CTI_Instance));
  inst->addr     = addr;
  inst->ap       = ap;
  inst->dp       = dp;
  inst->channels = ((val & CTI_DEVID_NUMCH)   >> CTI_DEVID_NUMCH_P);
  inst->triggers = ((val & CTI_DEVID_NUMTRIG) >> CTI_DEVID_NUMTRIG_P);

  if (tail == NULL) {
    CTI_Instances = inst;
  } else {
    tail->next    = inst;
  }

  return (0);
}

static bool _Activated(CTI_Instance* cti) {
  DWORD status, val, APSel;
  DWORD ofs = cti->addr;

  status = LinkCom(1);                      // 08.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (false); }
  
  APSel  = AP_Sel;                          // Save AP_Sel
  AP_Sel = cti->ap << 24;                   // Switch to CTI Access Port
#if DBGCM_V8M
  status = ReadD32(CTI_CONTROL(ofs), &val, BLOCK_SECTYPE_ANY); // Check if CTI Instance enabled
#else // DBGCM_V8M
  status = ReadD32(CTI_CONTROL(ofs), &val); // Check if CTI Instance enabled
#endif // DBGCM_V8M
  AP_Sel = APSel;                           // Restore AP_Sel

  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  if (status) { OutErrorMessage(status); return (false); }

  return (val & CTI_CONTROL_GLBEN);
}

// One or more CTIs activated
bool CTI_Activated(void) {
  CTI_Instance *inst = CTI_Instances;

  for (; inst != NULL; inst = inst->next) {
    if (_Activated(inst)) return (true);
  }
  return (false);
}

// Return channel mapping bit mask for CTITRIGIN[trignum]
static DWORD _GetTrigInChannels(CTI_Instance* cti, DWORD trignum) {
  DWORD status, val, APSel;
  DWORD ofs    = cti->addr;

  if (trignum >= cti->triggers) {
    // Out of bounds
    return (0);
  }

  status = LinkCom(1);                      // 08.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (0); }
  
  APSel  = AP_Sel;                                    // Save AP_Sel
  AP_Sel = cti->ap << 24;                             // Switch to CTI Access Port
#if DBGCM_V8M
  status = ReadD32(CTI_INEN0(ofs) + trignum*4, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(CTI_INEN0(ofs) + trignum*4, &val);
#endif // DBGCM_V8M
  AP_Sel = APSel;                                     // Restore AP_Sel

  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  if (status) { OutErrorMessage(status); return (0); }

  return val;
}

// Return channel mapping bit mask for CTITRIGOUT[trignum]
static DWORD _GetTrigOutChannels(CTI_Instance* cti, DWORD trignum) {
  DWORD status, val, APSel;
  DWORD ofs    = cti->addr;

  if (trignum >= cti->triggers) {
    // Out of bounds
    return (0);
  }

  status = LinkCom(1);                      // 08.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (0); }
  
  APSel  = AP_Sel;                                     // Save AP_Sel
  AP_Sel = cti->ap << 24;                              // Switch to CTI Access Port
#if DBGCM_V8M
  status = ReadD32(CTI_OUTEN0(ofs) + trignum*4, &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(CTI_OUTEN0(ofs) + trignum*4, &val);
#endif // DBGCM_V8M
  AP_Sel = APSel;                                      // Restore AP_Sel

  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  if (status) { OutErrorMessage(status); return (0); }

  return val;
}

static int _ClearAppTriggers(CTI_Instance* cti, DWORD mask) {
  DWORD status = 0, val, APSel;
  DWORD ofs    = cti->addr;
  DWORD chmask = ((1 << cti->channels) - 1);

  status = LinkCom(1);                                 // 08.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (0); }
  
  APSel  = AP_Sel;                                     // Save AP_Sel
  AP_Sel = cti->ap << 24;                              // Switch to CTI Access Port

  // Read active Application Triggers
#if DBGCM_V8M
  status = ReadD32(CTI_APPSET(ofs), &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(CTI_APPSET(ofs), &val);
#endif // DBGCM_V8M
  if (status) goto end;

  val &= (chmask & mask);

  if (val == 0) {
    // Nothing to do, no Application Triggers set
    goto end;
  }

  // Clear active Application Triggers
#if DBGCM_V8M
  status = WriteD32(CTI_APPCLEAR(ofs), val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CTI_APPCLEAR(ofs), val);
#endif // DBGCM_V8M
  if (status) goto end;

end:
  AP_Sel = APSel;                                      // Restore AP_Sel
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  return (0);
}

// Clear Application Triggers for all activated CTIs
int CTI_ClearAppTriggers(DWORD mask) {
  int status = 0;
  CTI_Instance *inst = CTI_Instances;

  for (; inst != NULL; inst = inst->next) {

    // Nothing to do if CTI disabled
    if (!_Activated(inst)) continue;

    // Clear Application Triggers for CTI instance
    status = _ClearAppTriggers(inst, mask);
    if (status) return (status);

  }

  return (0);
}

static int _AcknowledgeTriggers(CTI_Instance* cti, DWORD mask, bool setclr) {
  DWORD status = 0, val, APSel;
  DWORD ofs      = cti->addr;
  DWORD trigmask = ((1 << cti->triggers) - 1);

  status = LinkCom(1);                                 // 08.11.2018: Apply lock to protect AP_Sel
  if (status) { OutErrorMessage(status); return (0); }
  
  APSel  = AP_Sel;                                     // Save AP_Sel
  AP_Sel = cti->ap << 24;                              // Switch to CTI Access Port

  // Read active Output Triggers
#if DBGCM_V8M
  status = ReadD32(CTI_TRIGOUTSTATUS(ofs), &val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = ReadD32(CTI_TRIGOUTSTATUS(ofs), &val);
#endif // DBGCM_V8M
  if (status) goto end;
  val &= (trigmask & mask);

  if (val == 0) {
    // Concerned Output Triggers inactive
    goto end;
  }

  // Acknowledge all active Output Triggers
#if DBGCM_V8M
  status = WriteD32(CTI_INTACK(ofs), val, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
  status = WriteD32(CTI_INTACK(ofs), val);
#endif // DBGCM_V8M
  if (status) goto end;

  if (setclr) {
    // Clear acknowledge bits to deactivate possible feedback loops from processor, e.g. HALTED signal (CTRIGIN[0]).
#if DBGCM_V8M
    status = WriteD32(CTI_INTACK(ofs), 0, BLOCK_SECTYPE_ANY);
#else // DBGCM_V8M
    status = WriteD32(CTI_INTACK(ofs), 0);
#endif // DBGCM_V8M
    if (status) goto end;
  }

end:
  AP_Sel = APSel;                                      // Restore AP_Sel
  if (status) {
    LinkCom(0);
  } else {
    status = LinkCom(0);
  }

  return (0);
}


// Acknowledge Output Triggers for all activated CTIs (setclr "pulses" the acknowledge bits)
int CTI_AcknowledgeTriggers(DWORD mask, bool setclr) {
  int status = 0;
  CTI_Instance *inst = CTI_Instances;

  for (; inst != NULL; inst = inst->next) {

    // Nothing to do if CTI disabled
    if (!_Activated(inst)) continue;

    // Acknowledge all active Output Triggers for CTI instance
    status = _AcknowledgeTriggers(inst, mask, setclr);
    if (status) return (status);

  }

  return (0);
}

// Execute handshake mechanisms to release core from CTI EDBGRQ
int CTI_RunStepProcessor() {
  int   status = 0;
  DWORD chmask;
  CTI_Instance *inst = CTI_Instances;

  for (; inst != NULL; inst = inst->next) {

    // Nothing to do if CTI disabled
    if (!_Activated(inst)) continue;

    // Get channels mapped to CTITRIGOUT[0]
    chmask = _GetTrigOutChannels(inst, 0);

    if (chmask != 0) {
      // Clear potentially set application triggers.
      //  Debugger is active, so no DebugMonitor ISR can clear the app trigger.
      status = _ClearAppTriggers(inst, chmask);
    }

    // CTITRIGOUT[0] is heavily recommended by ARM to be used for EDBGRQ
    // Following this recommendation for now.
    status = _AcknowledgeTriggers(inst, 0x00000001, true);
    if (status) return (status);

  }

  return (0);
}


void InitCTI() {
  CTI_Instances = NULL;
}
