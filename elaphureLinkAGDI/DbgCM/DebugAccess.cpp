/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.8
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016-2020 ARM Limited. All rights reserved.
 *
 * @brief     Debug Access Detection and Recovery
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

#include "COLLECT.H"

#if DBGCM_RECOVERY

#include "resource.h"

#include "..\BOM.H"
#include "DebugAccess.h"
#include "Debug.h"
#include "JTAG.h"
#include "SWD.h"
#include "Trace.h"
#include "PDSCDebug.h"
#include "DSMonitor.h"
#include "CSTF.h"
#include "ETB.h"


#define DP_POWERED_MASK (CDBGPWRUPACK | CSYSPWRUPACK)

bool recoverUnderReset = false;
int  dbgAccLevel;

extern HANDLE kDebugAccessMutex;

int DebugAccessDetection(void)
{
    int   status;
    DWORD value;

    // Not available for all debug units.
#if 0
  if (UsbPipeServerDead()) {
    dbgAccLevel = DA_DBGSERVER;
    return (0);
  }
#endif


    switch (dbgAccLevel) {
        case DA_NORMAL:

#if DBGCM_V8M
            // Check memory access
            status = ReadD32(DBG_HCSR, &value, BLOCK_SECTYPE_ANY);
            if (status == 0) {
                if (value & C_DEBUGEN) {
                    // Could read AP and debug is enabled
                    break;
                }

                // Debug no longer enabled
            }
#else  // DBGCM_V8M \
       // Check memory access
            status = ReadD32(DBG_HCSR, &value);
            if (status == 0) {
                if (value & C_DEBUGEN) {
                    // Could read AP and debug is enabled
                    break;
                }

                // Debug no longer enabled
            }
#endif // DBGCM_V8M

            // Memory and AP access failed, change access level
            dbgAccLevel = DA_ACCESSPORT;
        case DA_ACCESSPORT:
            // Memory access failed, check access to AP
            status = ReadAP(AP_CSW, &value);
            if (status == 0) {
                if (value & CSW_DBGSTAT) {
                    // Could read AP and device access is enabled
                    break;
                }

                // Cannot access device
            }

            // Could not read AP, next access level would be the debug port
            dbgAccLevel = DA_DEBUGPORT;
        case DA_DEBUGPORT:
        case DA_DEBUGPOWER:
            // Check if DP accessible and if system is powered up
            status = ReadDP(DP_CTRL_STAT, &value);
            if (status == 0) {
                if ((value & DP_POWERED_MASK) == DP_POWERED_MASK) {
                    dbgAccLevel = DA_DEBUGPOWER;
                }
                break;
            }

            // Could not access DP which means a protocol error
            dbgAccLevel = DA_INACTIVE;
        case DA_INACTIVE:
        case DA_DBGSERVER:
            // nothing more to test here
            break;
    }

    return (0);
}

static int _DebugProtocolRecovery(void)
{
    int   status = 0;
    DWORD dummy;

#if DBGCM_DBG_DESCRIPTION
    // Use debug description for recovery
    if (PDSCDebug_IsEnabled()) {
        status = PDSCDebug_DebugPortSetup();
        if (status)
            return (status);

        if (DP_Type == JTAG_DP) {
            // Read DP CTRL/STAT Register to test that JTAG connection is up.
            // (Replace by reading IDCODE via IDCODE IR later; but CTRL/STAT
            // should do the job as well)
            status = JTAG_ReadDP(DP_CTRL_STAT, &dummy);
            if (status)
                return (status);
        }

    } else {
#endif // DBGCM_DBG_DESCRIPTION
        switch (DP_Type) {
            case JTAG_DP:
                if (MonConf.Opt & USE_SWJ) {
                    // Select JTAG protocol using switch sequence from connection
                    // Use SWJ Sequence Functions instead of what's done in GetDeviceList()
                    //  => no loss of reset state which could make the whole
                    //     sequence obsolete (target could run into disconnect again)
                    // SWJ Line Reset ( > 51*TMS)
                    status = JTAG_SWJ_Sequence(51, 0xFFFFFFFFFFFFFFFFULL);
                    if (status)
                        return (status);
                    status = JTAG_SWJ_Sequence(16, SWJ_SwitchSeq);
                    if (status)
                        return (status);
                    // SWJ Line Reset ( > 51*TMS)
                    status = JTAG_SWJ_Sequence(51, 0xFFFFFFFFFFFFFFFFULL);
                    if (status)
                        return (status);
                }

                // Reset JTAG Chain and put to Run-Test/Idle
                status = JTAG_Reset();
                if (status)
                    return (status);

                // Read DP CTRL/STAT Register to test that JTAG connection is up.
                // (Replace by reading IDCODE via IDCODE IR later; but CTRL/STAT
                // should do the job as well)
                status = JTAG_ReadDP(DP_CTRL_STAT, &dummy);
                if (status)
                    return (status);

                break;
            case SW_DP:
                if (MonConf.Opt & USE_SWJ) {
                    // Select SW protocol using switch sequence from connection
                    status = SWJ_Reset();
                    if (status)
                        return (status);
                    status = SWJ_Switch(SWJ_SwitchSeq);
                    if (status)
                        return (status);
                    status = SWJ_Reset();
                    if (status)
                        return (status);
                } else {
                    status = SWJ_Reset();
                    if (status)
                        return (status);
                }

                // Enable Serial Wire Debug via DPIDR read
                status = SWD_ReadID();
                if (status)
                    return (status);
                break;
            default:
                return (EU20); // Internal DLL Error: Unsupported Debug Protocol
        }
#if DBGCM_DBG_DESCRIPTION
    }
#endif // DBGCM_DBG_DESCRIPTION

    return (0);
}

static int _DebugPortRecovery(void)
{
    int   status;
    DWORD value;
    DWORD tick;

#if DBGCM_DBG_DESCRIPTION
    // Use debug description for recovery
    if (PDSCDebug_IsEnabled()) {
        return PDSCDebug_DebugPortStart();
    }
#endif // DBGCM_DBG_DESCRIPTION

    // Power debug port and system
    status = WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ);
    if (status)
        return (status);

    // Wait for debug port and system power up acknowledge
    tick = GetTickCount();
    do {
        status = ReadDP(DP_CTRL_STAT, &value);
        if (status)
            return (status);
        if ((value & (CDBGPWRUPACK | CSYSPWRUPACK)) == (CDBGPWRUPACK | CSYSPWRUPACK))
            break;
    } while ((GetTickCount() - tick) < 1000);

    if ((value & (CDBGPWRUPACK | CSYSPWRUPACK)) != (CDBGPWRUPACK | CSYSPWRUPACK)) {
        return EU11; // Device could not be powered up
    }

// Removed usage of DP CTRL/STAT CDBGRSTREQ bit. It is a last resort solution to recover from a locked up device
// and should not be used as part of each connection.
#if 0
  // Reset debug
  status = WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ|CSYSPWRUPREQ|CDBGRSTREQ);
  if (status) return (status);
#endif

    // Clear sticky bits
    switch (DP_Type) {
        case JTAG_DP:
            status = WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ | STICKYERR | STICKYCMP | STICKYORUN | TRNNORMAL | MASKLANE);
            if (status)
                return (status);
            break;
        case SW_DP:
            status = WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ | TRNNORMAL | MASKLANE);
            if (status)
                return (status);

            status = WriteDP(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR);
            if (status)
                return (status);
            break;
        default:
            return (EU20); // Internal DLL Error: Unsupported Debug Protocol
    }

    return 0;
}

static int _TraceRecovery(void)
{
    int status;

    if (TraceConf.Protocol == TPIU_ETB) {
        status = CSTF_Recovery();
        if (status)
            return (status);

        status = ETB_Recovery();
        if (status)
            return (status);

        status = Trace_Recovery();
        if (status)
            return (status);
    } else {
        if (TPIU_Type == TPIU_TYPE_CS || TPIU_Type == TPIU_TYPE_SWO) {
            status = CSTF_Recovery();
            if (status)
                return (status);
        }

        status = Trace_Recovery();
        if (status)
            return (status);
    }


#if DBGCM_DBG_DESCRIPTION
    // 21.04.2016: Use debug description for recovery
    if (PDSCDebug_IsEnabled()) {
        status = PDSCDebug_TraceStart();
        if (status)
            return (status);
    }
#endif // DBGCM_DBG_DESCRIPTION

    if (TraceConf.Opt & TRACE_TIMESTAMP) {
        status = Trace_Cycles(TRUE);
        if (status)
            return (status);
    }

    return 0;
}

static int _BreakpointUnitRecovery(void)
{
    int status;

    // Recover breakpoint configuration (control/remap/instruction comparators)
#if DBGCM_V8M
    status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
    status = WriteBlock(FPB_Addr, (BYTE *)&RegFPB, 4 * (2 + NumBP), 0 /*attrib*/);
#endif // DBGCM_V8M
    if (status)
        return (status);
    return 0;
}

static int _WatchpointUnitRecovery(void)
{
    int status;

    // Recover control register
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
#else  // DBGCM_V8M
    status = WriteD32(DWT_CTRL, RegDWT.CTRL);
#endif // DBGCM_V8M
    if (status)
        return (status);

        // Restore the profiling counter registers to continue counting after recovery (need an indicator in trace!)
        // skipping restoring of CYCCNT, handled differently since caching the value (opposed to the other counters)
#if DBGCM_V8M
    status = WriteBlock(DWT_CPICNT, (BYTE *)&RegDWT.CPICNT, 4 * 5, BLOCK_SECTYPE_ANY); // 5 profiling counters
#else                                                                                  // DBGCM_V8M
    status = WriteBlock(DWT_CPICNT, (BYTE *)&RegDWT.CPICNT, 4 * 5, 0 /*attrib*/); // 5 profiling counters
#endif                                                                                 // DBGCM_V8M
    if (status)
        return (status);

        // Recover DWT comparators
#if DBGCM_V8M
    status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
    status = WriteBlock(DWT_CMP, (BYTE *)&RegDWT.CMP, 4 * (4 * NumWP), 0 /*attrib*/);
#endif // DBGCM_V8M
    if (status)
        return (status);

    return 0;
}

static int _DebugSetupRecovery(void)
{
    int   status;
    DWORD value, ctrl;

#if DBGCM_DBG_DESCRIPTION
    // Use debug description for recovery
    if (PDSCDebug_IsEnabled()) {
        status = PDSCDebug_DebugCoreStart();
        if (status)
            return (status);
    } else {
#endif // DBGCM_DBG_DESCRIPTION \
       // Enable debug
#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &value, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
    status = ReadD32(DBG_HCSR, &value);
#endif // DBGCM_V8M
        if (status)
            return (status);
        if ((value & C_DEBUGEN) == 0) {
            // core debug has been disabled, enable it now...
            // ... and preserve settings like being halted by a reset vector catch
            value &= ~0xFFFF0000;        // Clear S_xxx bits
            value |= DBGKEY | C_DEBUGEN; // Ensure debug is enabled
#if DBGCM_V8M
            status = WriteD32(DBG_HCSR, value, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
        status = WriteD32(DBG_HCSR, value);
#endif // DBGCM_V8M
            if (status)
                return (status);
        }
#if DBGCM_DBG_DESCRIPTION
    }
#endif // DBGCM_DBG_DESCRIPTION

    // Enable trace
#if DBGCM_V8M
    status = ReadD32(DBG_EMCR, &value, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
    status = ReadD32(DBG_EMCR, &value);
#endif // DBGCM_V8M
    if (status)
        return (status);

#if DBGCM_V8M
    UpdateSecurityState();       // Read security state after recovery
    UpdateDebugAuthentication(); // Read debug authentication status from target
#endif                           // DBGCM_V8M

    if ((value & (TRCENA | MONCONF_RST_VECT_CATCH)) != (TRCENA | MONCONF_RST_VECT_CATCH)) {
#if DBGCM_V8M
        status = WriteD32(DBG_EMCR, (value | TRCENA | MONCONF_RST_VECT_CATCH), BLOCK_SECTYPE_ANY); // Enable Trace
#else                                                                                              // DBGCM_V8M
        status = WriteD32(DBG_EMCR, (value | TRCENA | MONCONF_RST_VECT_CATCH)); // Enable Trace
#endif                                                                                             // DBGCM_V8M
        if (status)
            return (status);
    }
    if ((value & TRCENA) == 0) {
        if (DWT_Addr != 0) { // Re-Synchronize Cycle Counter as good as possible
#if DBGCM_V8M
            status = ReadD32(DWT_CTRL, &ctrl, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
            status = ReadD32(DWT_CTRL, &ctrl);
#endif // DBGCM_V8M
            if (status)
                return (status);

            if (((ctrl & DWT_CYCCNTEN) == 0) && (RegDWT.CTRL & DWT_CYCCNTEN) && TraceCycDwt) {
                // cycle counter was disabled when losing connection, 'sync' cached value before re-enabling
#if DBGCM_V8M
                status = ReadD32(DWT_CYCCNT, &value, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
                status = ReadD32(DWT_CYCCNT, &value);
#endif // DBGCM_V8M
                if (status)
                    return (status);
                RegDWT.CYCCNT = value;
            } else {
                status = UpdateCycles();
                if (status)
                    return (status);
            }
        }
    }

    // Now recover the "sub-systems"
    if (TraceConf.Opt & TRACE_ENABLE) {
        status = _TraceRecovery();
        if (status)
            return (status);
    }

    // Recover those features as last step to avoid exceptions because of having neither Debug nor DebugMonitor enabled
    status = _BreakpointUnitRecovery();
    if (status)
        return (status);

    status = _WatchpointUnitRecovery();
    if (status)
        return (status);


    return 0;
}

int DebugAccessRecovery(void)
{
    int         status = 0;
    DWORD       value, nR;
    bool        recover  = (dbgAccLevel != DA_NORMAL);
    bool        isLocked = false;
    AP_CONTEXT *apCtx;


    // Do Access Recovery "atomically" to avoid another connection
    // interfering here.
#if DBGCM_DS_MONITOR
    if (dbgAccLevel != DA_NORMAL && dbgAccLevel != DA_DBGSERVER // Otherwise not entering critical path
        && GetCurrentThreadId() != DSMonitorThread.threadID) {  // 05.07.2016: Only lock if not in DSMonitor thread. A call to this function
                                                                // was preceded by ceasing further updates if in DSMonitor thread. Locking this in
                                                                // DSMonitor thread can lead to lockup if calling PDSC sequences.
#else                                                           // DBGCM_DS_MONITOR
    if (dbgAccLevel != DA_NORMAL && dbgAccLevel != DA_DBGSERVER) { // Otherwise not entering critical path
#endif                                                          // DBGCM_DS_MONITOR
        value = WaitForSingleObject(kDebugAccessMutex, INFINITE);
        isLocked = true;
    }

    // Jump into recovery sequence depending on current access level
    switch (dbgAccLevel) {
        case DA_INACTIVE:
            // Recovering debug protocol (JTAG/SW)
            status = _DebugProtocolRecovery();
            if (status)
                goto early_end;

            dbgAccLevel = DA_DEBUGPORT;

            // Fall through to next recovery step
        case DA_DEBUGPORT:
            // Recovering debug port access

            // Check accessibility of DP CTRL/STAT register
            status = ReadDP(DP_CTRL_STAT, &value);
            if (status)
                goto early_end;

            // Check power state
            if ((value & DP_POWERED_MASK) != DP_POWERED_MASK) {
                // Power up debug port and debug system
                status = _DebugPortRecovery();
                if (status)
                    goto early_end;
            }

            // Debug system is powered up, next we need to check the AP
            dbgAccLevel = DA_DEBUGPOWER;

            // Fall through to next recovery step
        case DA_DEBUGPOWER:
            // Recover AP configuration

            // Write DP_SELECT, value unpredictable after POR
            status = WriteDP(DP_SELECT, AP_Sel);
            if (status)
                goto early_end;

            // Check accessibility of CSW register (AP)
            status = ReadAP(AP_CSW, &value);
            if (status)
                goto early_end;

            if ((value & CSW_DBGSTAT) == 0) {
                status = EU13;
                goto early_end;
            }

            status = AP_CurrentCtx(&apCtx);
            if (status)
                goto early_end;

            // Restore cached CSW setting
            // status = WriteAP(AP_CSW, CSW_Val);
            status = WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                goto early_end;

            dbgAccLevel = DA_ACCESSPORT;

        case DA_ACCESSPORT:
            // Check if memory accesses are available again

#if DBGCM_V8M
            status = ReadD32(DBG_HCSR, &value, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
            status = ReadD32(DBG_HCSR, &value);
#endif // DBGCM_V8M
            if (status)
                goto early_end;

            // Recover Debug Setup
            status = _DebugSetupRecovery();
            if (status)
                goto early_end;

            if (recoverUnderReset) {
                // User requested reconnect under reset, deassert the HW Chip Reset
                recoverUnderReset = false;

#if DBGCM_DBG_DESCRIPTION
                // Use debug description for recovery
                if (PDSCDebug_IsEnabled()) {
                    status = PDSCDebug_ResetHardwareDeassert();
                    if (status)
                        goto early_end;
                } else {
#endif // DBGCM_DBG_DESCRIPTION \
    //---TODO:                  \
    // Deassert HW Chip Reset
                    DEVELOP_MSG("Todo: \nDeassert HW Chip Reset");

                    // Reset Recovery (Reset Pulse can be extended by External Circuits)
                    // Wait until nRESET is deasserted (max 1s)

                    //---TODO:
                    // Wait for HW Chip Reset Pin to stabilize (Reset Recovery);
                    // Use 1 second timeout
                    DEVELOP_MSG("Todo: \nWait for HW Chip Reset Pin to stabilize (Reset Recovery)");
                    // n = GetTickCount();
                    // do {
                    //   - Read Reset Pin
                    //   - Reset Deasserted (nRESET HIGH) => break;
                    // } while ((GetTickCount() - n) < 1000);
                }

                Sleep(5);
#if DBGCM_DBG_DESCRIPTION
            }
#endif // DBGCM_DBG_DESCRIPTION

            // Reaching this point we should have re-established a connection.
            // It should be safe to release the LINK lock and the reset won't
            // need atomic execution. (If not releasing we might end in a
            // different thread during the callback call and lock ourselves up).
            if (isLocked) {
                isLocked = false;
                ReleaseMutex(kDebugAccessMutex);
            }

            // Execute functionality in OnRecoveryExec() user function
            nR = pCbFunc(AG_CB_EXECHOOK, "OnRecoveryExec");

            dbgAccLevel = DA_NORMAL;

        case DA_NORMAL:
#if DBGCM_DBG_DESCRIPTION
            if (recover && PDSCDebug_IsEnabled()) {
                status = PDSCDebug_RecoveryAcknowledge();
                if (status)
                    return (status);
            }
#endif // DBGCM_DBG_DESCRIPTION
        case DA_DBGSERVER:
            // normal_end:
            return (status);
    }

early_end: // Only go here if LINK lock hasn't released yet!
    if (isLocked) {
        isLocked = false;
        ReleaseMutex(kDebugAccessMutex);
    }

    return (status);
}

int DebugAccessEnsure()
{
    int   status = 0;
    DWORD value;

    // Detect possible loss of Debug Access
    if (status = DebugAccessDetection()) {
        return status;
    }

    // Debug Access still up?
    if (dbgAccLevel != DA_NORMAL) {
        if (dbgAccLevel == DA_DBGSERVER) {
            return (EU44); // Debug Server lost
        }

        if (status = DebugAccessRecovery()) {
            return (EU40); // Cannot recover Debug Connection
        }

        if (dbgAccLevel != DA_NORMAL) {
            return (EU40); // Cannot recover Debug Connection
        }

#if DBGCM_V8M
        status = ReadD32(DBG_HCSR, &value, BLOCK_SECTYPE_ANY);
#else  // DBGCM_V8M
        status = ReadD32(DBG_HCSR, &value);
#endif // DBGCM_V8M
        if (status)
            return (status);

        if (value & S_HALT) { // Target stopped after recovery
#if DBGCM_DS_MONITOR
            DSM_SuspendMonitor();
#endif // DBGCM_DS_MONITOR
            Invalidate();
            UpdateAllDlg();
            pio->Notify(UV_UPDATE_SELECTIVE, (void *)(VW_MEMORY | VW_WATCHWINDOW | VW_LOGIC_ANALYZER | VW_PERF_ANALYZER | VW_RTXVIEW | VW_PERIDIALOGS | VW_AGDIDIALOGS));
#if DBGCM_DS_MONITOR
            DSM_ResumeMonitor();
#endif // DBGCM_DS_MONITOR
        }
    }

    return (0);
}

int ConfigureDebugAccessRecovery(AG_RECOVERY *pRecovery)
{
#if DBGCM_DBG_DESCRIPTION
    int status;
#endif // DBGCM_DBG_DESCRIPTION

    if (pRecovery == 0 || PlayDead) {
        return (AG_RECOVCONF_ERR_OK);
    }

    if (pRecovery->underReset && !recoverUnderReset) { // Turn on recovery under HW Chip Reset

        // Use debug description for recovery
#if DBGCM_DBG_DESCRIPTION
        if (PDSCDebug_IsEnabled()) {
            status = PDSCDebug_ResetHardwareAssert();
            if (status) {
                return (status);
            }
        } else {
#endif // DBGCM_DBG_DESCRIPTION \
    // Assert HW Chip Reset

            //---TODO:
            // Assert HW Chip Reset
            DEVELOP_MSG("Todo: \nAssert HW Chip Reset");

#if DBGCM_DBG_DESCRIPTION
        }
#endif // DBGCM_DBG_DESCRIPTION

        recoverUnderReset = true;        // Set flag
        dbgAccLevel       = DA_INACTIVE; // Will be in reset => do full recovery

    } else if (!pRecovery->underReset && recoverUnderReset) { // Turn off recovery under HW Chip Reset

        // Use debug description for recovery
#if DBGCM_DBG_DESCRIPTION
        if (PDSCDebug_IsEnabled()) {
            status = PDSCDebug_ResetHardwareDeassert();
            if (status) {
                return (status);
            }
        } else {
#endif // DBGCM_DBG_DESCRIPTION \
    // Deassert HW Chip Reset

            //---TODO:
            // Deassert HW Chip Reset
            DEVELOP_MSG("Todo: \nDeassert HW Chip Reset");
#if DBGCM_DBG_DESCRIPTION
        }
#endif // DBGCM_DBG_DESCRIPTION

        recoverUnderReset = false; // Set flag
                                   // keep dbgAccLevel
    }

    if (pRecovery->exitDebug) {
#if DBGCM_WITHOUT_STOP
        if (iRun) {      // Try the friendly way by using the built-in mechanisms
            StopRun = 1; // Prepare exit of debug by forcing target to stop
        } else {
            SetPlayDead(NULL); // Brute force, let's pray...but this should work if called from GUI thread
        }
#else  // DBGCM_WITHOUT_STOP
        StopRun = 1; // Prepare exit of debug by forcing target to stop
#endif // DBGCM_WITHOUT_STOP
    }

    return (AG_RECOVCONF_ERR_OK);
}

void InitDebugAccess()
{
    recoverUnderReset = false;
    dbgAccLevel       = 0;
}

#endif // DBGCM_RECOVERY