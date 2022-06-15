/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.6
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016-2018 ARM Limited. All rights reserved.
 *
 * @brief     Device State Monitor
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

#if DBGCM_DS_MONITOR

#include "resource.h"
#include "..\BOM.H"
#include "Debug.h"

#if DBGCM_RECOVERY
#include "DebugAccess.h"
#endif // DBGCM_RECOVERY

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION

#include "DSMonitor.h"
#include "Trace.h"

DSM_THREAD DSMonitorThread;
BYTE       DSMonitorState;
struct lPa DSMonitorResetMsg;


#define PERIODIC_TIME 500 // 500 mSec

static __inline void DSMonitor_DoPeriodicUpdate(void)
{
    DWORD updates = VW_COMPONENTS | // 08.07.2016
                    VW_MEMORY | VW_WATCHWINDOW | VW_LOGIC_ANALYZER | VW_PERF_ANALYZER | VW_RTXVIEW | VW_PERIDIALOGS | VW_AGDIDIALOGS;

#if DBGCM_RECOVERY
    if (GoMode && pio->periodic && dbgAccLevel == DA_NORMAL) { // if executing and periodic active
#else                                                          // DBGCM_RECOVERY
    if (GoMode && pio->periodic) { // if executing and periodic active
#endif                                                         // DBGCM_RECOVERY
        // Update memory & watch window if activated...
#if DBGCM_V8M
        UpdateSecurityState();       // Read current security state from target
        UpdateDebugAuthentication(); // Read debug authentication status from target
#endif                               // DBGCM_V8M
        UpdateCycles();
        UpdateAllDlg();
        pio->Notify(UV_UPDATE_SELECTIVE, (void *)updates);
    }
}


static __inline bool DSMonitor_MatchState(void)
{
    if (DSMonitorThread.matchAll) { // All States must match
        if ((DSMonitorState & DSMonitorThread.matchMask) == (DSMonitorThread.matchValue & DSMonitorThread.matchMask)) {
            return true;
        }
    } else { // Only one state must match
        for (unsigned int i = 0; i < 8; i++) {
            if ((1 << i) & DSMonitorThread.matchMask) {
                if ((DSMonitorState & (1 << i)) == (DSMonitorThread.matchValue & (1 << i))) {
                    return true;
                }
            }
        }
    }
    return false;
}


/*
 * Update Device State Monitor State based on given DHCSR value.
 */
static __inline void DSMonitor_UpdateCPUState(DWORD dhcsr)
{
    BYTE  prevState;
    DWORD ticks, res, status;

    prevState = DSMonitorState;

    // Determine CPU Halted State
    if (dhcsr & S_HALT) {
        DSMonitorState |= DSM_STATE_CPU_HALTED;
    } else {
        DSMonitorState &= ~DSM_STATE_CPU_HALTED;
        if (!DSMonitorThread.kill) {
            if (prevState & DSM_STATE_CPU_HALTED) {
#if DBGCM_WITHOUT_STOP
                if (DSMonitorThread.suppressRunningCB) {
                    DSMonitorThread.missedRunningCB = 1;
                } else {
                    DSM_SuspendMonitor();
                    RunningCB(RUNNING_DETECTED);
                    DSM_ResumeMonitor();
                }
#endif // DBGCM_WITHOUT_STOP
                if (!DSMonitorThread.suppressPeriodic) {
                    DSMonitorThread.lastPeriodicUpdate = GetTickCount();
                }
            } else if (!DSMonitorThread.suppressPeriodic) {
                // Periodic update is called approximatly every PERIODIC_TIME ms
                ticks = GetTickCount();
                if ((ticks - DSMonitorThread.lastPeriodicUpdate) > DSMonitorThread.periodicTime) {
                    DSMonitorThread.lastPeriodicUpdate = ticks;
                    DSM_SuspendMonitor();
                    DSMonitor_DoPeriodicUpdate(); // uVision blocks target accesses if in recovery mode
                    DSM_ResumeMonitor();
                }
            }
            if (DSMonitorThread.traceActive) {
                // Read from Debug Unit Trace Buffer
                if (TraceOpt & (TRACE_ENABLE || PC_SAMPLE)) {
                    status = Trace_Read(50, TRUE);
                    if (status) {
                        OutError(status);
                        DSMonitorThread.traceActive = 0;
                    }
                }
            }
        }
    }

    // Handle Change in Reset State
    if (DSMonitorState & DSM_STATE_RST_ACTIVE) {
        if (dhcsr & S_RESET_ST) { // Target still in reset
            DSMonitorState &= ~DSM_STATE_RST_DONE;
        } else { // Target finished reset since last update
            DSMonitorState |= DSM_STATE_RST_DONE;
            if ((prevState & DSM_STATE_RST_DONE) == 0) {     // Execute hook on detection of finished reset
                if (DSMonitorState & DSM_STATE_CPU_HALTED) { // CPU stopped after reset because of Reset Vector Catch?
                    if (MonConf.Opt & RST_VECT_CATCH) {      // Execute based on Debug Setup. Reset without "Stop After Reset" will have
                                                             // DEMCR.VC_CORERESET set (and some special handlings will never have it set).
                        if (!DSMonitorThread.suppressHooks) {
                            // Now give pio->iRun up to 1 second to go low
                            if (pio->iRun) {
                                ticks = GetTickCount();
                                do {
                                    res = WaitForSingleObject(DSMonitorThread.h, 2);
                                    if (res != WAIT_TIMEOUT)
                                        return; // Thread handle signalled end or error
                                    if (pio->iRun == 0)
                                        break; // Stop procedure ended, should be able to safely execute hook now
                                } while (GetTickCount() - ticks < 1000);
                            }
                            DSM_SuspendMonitor();
                            pCbFunc(AG_CB_EXECHOOK, "OnStopAfterReset");
                            DSM_ResumeMonitor();
                        }
                    }
                    if (!pio->FlashLoad && !(DSMonitorState & DSM_STATE_RUN_BEFORE_RST)) { // Otherwise done by the normal stop procedure;
                                                                                           // there is also a race condition between the following UV_DBG_RESET_GIVEN
                                                                                           // and UV_RUN_COMPLETED which would cause inconsistent window update
                                                                                           // behavior (Call Stack window)
                        Invalidate();                                                      // Invalidate all cached target resources, target is stopped after detected reset
                        GetRegs((1ULL << nR15) | (1ULL << nPSR));                          // Read PC & xPSR
                        if (pio->hwnd && pio->hmsg) {
                            ::PostMessage(pio->hwnd, pio->hmsg, MSG_UV2_NOTIFY, (LPARAM)&DSMonitorResetMsg); // Send UV_DBG_RESET_GIVEN to trigger full post-reset update
                        }
                    }
                }
                DSMonitorState &= ~DSM_STATE_RUN_BEFORE_RST;
            }
        }
    }

    // Determine CPU Reset State
    if (dhcsr & S_RESET_ST) {
        DSMonitorState &= ~DSM_STATE_RST_DONE;
        DSMonitorState |= DSM_STATE_RST_ACTIVE;
        if ((prevState & DSM_STATE_RST_ACTIVE) == 0) { // Execute hook on detection of reset activation
            if ((prevState & DSM_STATE_CPU_HALTED) == 0) {
                DSMonitorState |= DSM_STATE_RUN_BEFORE_RST;
            }
        }
    } else {
        DSMonitorState &= ~DSM_STATE_RST_ACTIVE;
    }
}


/*
 * Device State Monitor Thread
 */
static void DSMonitor_Thread(void *vp)
{
    DWORD  res, dhcsr, matchTimerStart, matchTimeout;
    int    status;
    bool   update, wokenup;
    HANDLE events[] = { DSMonitorThread.wakeup };
#if DBGCM_RECOVERY
    int daLevel;
#endif // DBGCM_RECOVERY

    // Init Monitor Thread Structure
    DSMonitorThread.threadID           = GetCurrentThreadId();
    DSMonitorThread.interval           = 250; // Poll frequently at start
    DSMonitorThread.recovery           = pio->DbgRecovery;
    DSMonitorThread.up                 = 1;
    DSMonitorThread.periodicTime       = PERIODIC_TIME;
    DSMonitorThread.lastPeriodicUpdate = 0;
    DSMonitorThread.configSynced       = 1;
    DSMonitorThread.matchActive        = 0;

#if DBGCM_RECOVERY
    // Initialize Debug Access Level variable
    dbgAccLevel = DA_NORMAL;
#endif // DBGCM_RECOVERY

    // Signal thread creation
    if (!SetEvent(DSMonitorThread.awake))
        return;

    // Init States
    status       = 0;
    matchTimeout = 0;
    wokenup      = false;

    for (;;) {
        if (wokenup) {
            // Signal wake-up
            SetEvent(DSMonitorThread.awake);
            wokenup = false;
        }

        if (!DSMonitorThread.configSynced) {
            SetEvent(DSMonitorThread.config_evt);
            DSMonitorThread.configSynced = 1;
        }

        if (DSMonitorThread.suspendCount) { // Monitor Thread Suspended, JR 10.03.16: always suspend the thread, depended on other conditions
            DSMonitorThread.inSuspension = 1;
            if (SetEvent(DSMonitorThread.suspended)) {
                res = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE);
            }
            DSMonitorThread.inSuspension = 0;
            if (matchTimeout)
                DSMonitorThread.matchTimeout = matchTimeout; // Re-arm possibly interrupted match
        }

        if (DSMonitorThread.matchTimeout) {
            // Arm timeout
            matchTimeout                 = DSMonitorThread.matchTimeout;
            matchTimerStart              = GetTickCount();
            DSMonitorThread.matchActive  = 1;
            DSMonitorThread.matchTimeout = 0;
        } else if (matchTimeout) {
            if (DSMonitor_MatchState()) {
                matchTimeout                = 0;
                DSMonitorThread.matchActive = 0;
                SetEvent(DSMonitorThread.match);
            }

            // Check time if no match
            if (matchTimeout && GetTickCount() - matchTimerStart > matchTimeout) {
                matchTimeout                = 0;
                DSMonitorThread.matchActive = 0;
                SetEvent(DSMonitorThread.match);
            }

        } else if ((DSMonitorState & DSM_STATE_RST_ACTIVE) == 0) { // Increase polling frequency if in reset
            if (DSMonitorThread.suspendCount) {                    // Monitor Thread Suspended
                DSMonitorThread.inSuspension = 1;
                if (SetEvent(DSMonitorThread.suspended)) {
                    res = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE);
                }
                DSMonitorThread.inSuspension = 0;
            } else { // Normal Monitor Mode
                res = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, DSMonitorThread.interval);
            }
            switch (res) {
                case WAIT_TIMEOUT: break;                  // Do nothing
                case WAIT_OBJECT_0: wokenup = true; break; // DSMonitorThread.wakeup
                case WAIT_FAILED: break;                   // TODO: Handle errors
                default: break;                            // TODO: Handle errors
            }
        }

        if (PlayDead)
            break; // Exit Thread loop
        if (DSMonitorThread.kill)
            break; // Exit Thread loop
        if (DSMonitorThread.suspendCount)
            continue; // Skip iteration and go to sleep (above)

#if DBGCM_RECOVERY
        daLevel = dbgAccLevel; // Store target access mode
#endif                         // DBGCM_RECOVERY
        update = false;

        if (DSMonitorThread.extDHCSR) { // Data from external DHCSR read?
            DSMonitorState &= ~DSM_STATE_ACC_ERR;
            dhcsr                    = DSMonitorThread.dhcsr;
            DSMonitorThread.extDHCSR = 0;
            update                   = true;
        }
#if DBGCM_RECOVERY
        else if (DSMonitorThread.recovery) { // Monitor Thread with recovery mechanism enabled
            DSM_SuspendMonitor();
            status = DebugAccessRecovery(); // Only recovering if (targAccMode != TA_NORMAL)
            DSM_ResumeMonitor();
            if (status == 0) { // Fully recovered target access, read status

#if DBGCM_V8M
                status = ReadD32(DBG_HCSR, &dhcsr, BLOCK_SECTYPE_ANY);
                if (status) {
                    DSMonitorState |= DSM_STATE_ACC_ERR;
                } else {
                    DSMonitorState &= ~DSM_STATE_ACC_ERR;
                }
                update = true;
#else  // DBGCM_V8M
                status = ReadD32(DBG_HCSR, &dhcsr);
                if (status) {
                    DSMonitorState |= DSM_STATE_ACC_ERR;
                } else {
                    DSMonitorState &= ~DSM_STATE_ACC_ERR;
                }
                update = true;
#endif // DBGCM_V8M
            }

            if (PlayDead)
                break; // Exit Thread loop
            if (DSMonitorThread.kill)
                break; // Exit Thread loop
            if (DSMonitorThread.suspendCount)
                continue; // Skip iteration and go to sleep (above)

            if (status) {
                // Reading DBG_HCSR failed, check to what extent the target is inaccessible
                status = DebugAccessDetection();
#if DBGCM_DBG_DESCRIPTION
                PDSCDebug_DebugContext = DBGCON_RECOVERY;
#endif // DBGCM_DBG_DESCRIPTION
            }

            if (dbgAccLevel == DA_DBGSERVER)
                break; // No chance to recover, exit
            if (PlayDead)
                break; // Abort if PlayDead to avoid further delays in disconnect
            if (DSMonitorThread.kill)
                break; // Abort if kill request for thread to avoid further delays in disconnect
            if (DSMonitorThread.suspendCount)
                continue; // Skip iteration and go to sleep (above)

            if (daLevel != dbgAccLevel) {
                // Change in target access mode, notify uVision
                DSM_SuspendMonitor();
                pio->Notify(UV_DBG_ACCESS_LEVEL, (void *)dbgAccLevel);
                if (dbgAccLevel == DA_NORMAL && ((DSMonitorState & DSM_STATE_ACC_ERR) == 0) && (dhcsr & S_HALT)) { // Target stopped after recovery
                    Invalidate();
                    UpdateAllDlg();
                    pio->Notify(UV_UPDATE_SELECTIVE, (void *)(VW_MEMORY | VW_WATCHWINDOW | VW_LOGIC_ANALYZER | VW_PERF_ANALYZER | VW_RTXVIEW | VW_PERIDIALOGS | VW_AGDIDIALOGS));
                }
                DSM_ResumeMonitor();
            }
        }
#endif         // DBGCM_RECOVERY
        else { // Monitor Thread with recovery mechanism disabled

#if DBGCM_V8M
            // Feature not requested by debugger, just read DHCSR as before
            status = ReadD32(DBG_HCSR, &dhcsr, BLOCK_SECTYPE_ANY);
            if (status) {
                DSMonitorState |= DSM_STATE_ACC_ERR;
            } else {
                DSMonitorState &= ~DSM_STATE_ACC_ERR;
            }
            update = true;
#else  // DBGCM_V8M \
       // Feature not requested by debugger, just read DHCSR as before
            status = ReadD32(DBG_HCSR, &dhcsr);
            if (status) {
                DSMonitorState |= DSM_STATE_ACC_ERR;
            } else {
                DSMonitorState &= ~DSM_STATE_ACC_ERR;
            }
            update = true;
#endif // DBGCM_V8M
        }
        // JR 18.11.2015: Do not exit in error case, this can cause this thread to finish
        // while someone else waits for it to deliver results.
        if (status && !DSMonitorThread.suppressErrors) {
            OutError(status); /*break;*/
        }

        if (update) { // New DHCSR data available
            if (PlayDead)
                break; // Abort if PlayDead to avoid further delays in disconnect
            if (DSMonitorThread.kill)
                break; // Abort if kill request for thread to avoid further delays in disconnect
            if (DSMonitorThread.suspendCount)
                continue; // Skip iteration and go to sleep (above)
            if (!(DSMonitorState & DSM_STATE_ACC_ERR))
                DSMonitor_UpdateCPUState(dhcsr);
        }
    }

    DSMonitorThread.up = 0;

    if (PlayDead && !PlayDeadRegular) { // 15.12.2016, SDMDK-6325: Kill debug connection if PlayDead happened in this thread.
        // 23.01.2017, SDMDK-6575: PlayDeadRegular set if shutting down the regular way.
        //  Async Terminate message. Will prepare shutdown and trigger the PlayDead error message box during the next GUI thread
        //  driven AGDI access during shutdown. Before, message box from DS Monitor thread became dangling and caused crash if GUI
        //  thread continued shutdown.
        //PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
        ExecPlayDead();
    }
}


/*
 * Start Device State Monitor
 */
int DSM_StartMonitor()
{
    DWORD ticks, res;
    memset(&DSMonitorResetMsg, 0, sizeof(DSMonitorResetMsg));
    DSMonitorResetMsg.n1 = UV_DBG_RESET_GIVEN;
    memset(&DSMonitorThread, 0, sizeof(DSMonitorThread));
    DSMonitorThread.wakeup     = CreateEvent(NULL, FALSE, FALSE, NULL);
    DSMonitorThread.suspended  = CreateEvent(NULL, FALSE, FALSE, NULL);
    DSMonitorThread.awake      = CreateEvent(NULL, FALSE, FALSE, NULL);
    DSMonitorThread.match      = CreateEvent(NULL, FALSE, FALSE, NULL);
    DSMonitorThread.config_evt = CreateEvent(NULL, FALSE, FALSE, NULL);
    DSMonitorThread.h          = (HANDLE)_beginthread(DSMonitor_Thread, 0, NULL);
    memset(&DSMonitorState, 0, sizeof(DSMonitorState));
    ticks = GetTickCount();
    do {
        res = WaitForSingleObject(DSMonitorThread.awake, 1);
        if (res == WAIT_OBJECT_0)
            break;
        if (res != WAIT_TIMEOUT)
            return (EU01);
        if (DSMonitorThread.up)
            break;
    } while (GetTickCount() - ticks <= 500);
    if (!DSMonitorThread.up)
        return (EU01); // Internal DLL Error
    return (0);
}


/*
 * Stop Device State Monitor
 */
int DSM_StopMonitor()
{
    DWORD res;

    // Signal end of thread
    DSMonitorThread.kill = 1;
    SetEvent(DSMonitorThread.wakeup);

    // Wait for thread to join
    if (DSMonitorThread.h) {
        while (DSMonitorThread.up) {
            res = WaitForSingleObject(DSMonitorThread.h, 5);
            if (res == WAIT_OBJECT_0)
                break; // Thread joined
            if (res != WAIT_TIMEOUT)
                break; // Something's broken

            if (!HardPlayDead()) {
                // Force GUI thread messages to process.
                // Note that this only has an effect if DSM_StopMonitor is called from the GUI thread which is the
                // only scenario potentially causing a deadlock during PlayDead.
                pio->Notify(UV_DOXEVENTS, (void *)1);
            }
        }
        DSMonitorThread.h = NULL;
    }

    // Free suspended event
    if (DSMonitorThread.suspended) {
        CloseHandle(DSMonitorThread.suspended);
        DSMonitorThread.suspended = NULL;
    }

    // Free wait for value event
    if (DSMonitorThread.match) {
        CloseHandle(DSMonitorThread.match);
        DSMonitorThread.match = NULL;
    }

    // Free wake-up event
    if (DSMonitorThread.wakeup) {
        CloseHandle(DSMonitorThread.wakeup);
        DSMonitorThread.wakeup = NULL;
    }

    // Free configuration synced event
    if (DSMonitorThread.config_evt) {
        CloseHandle(DSMonitorThread.config_evt);
        DSMonitorThread.config_evt = NULL;
    }

    return (0);
}

/*
 * Set Device State Monitor Polling Interval
 *   - interval: Polling Interval in Milliseconds
 */
int DSM_ConfigureMonitor(BYTE mask, BYTE control, DWORD interval, BOOL sync)
{
    const int confSyncTimeout = 20; // Wait 20 ms for event signal before checking the sync flag
    int       confSyncRetries = 50; // Wait syncRetries*syncTimeout for synchronization to new config
    DWORD     res;

    if (mask & DSM_CFG_WAKEUP) {
        if (control & DSM_CFG_WAKEUP) {
            // Wake up thread to adopt new polling interval
            if (!SetEvent(DSMonitorThread.wakeup))
                return EU01; // Internal DLL Error
        }
    }
    if (mask & DSM_CFG_UPDATE_INTERVAL) {
        if (control & DSM_CFG_UPDATE_INTERVAL)
            DSMonitorThread.interval = interval;
    }
    if (mask & DSM_CFG_SUPPRESS_ERR) {
        DSMonitorThread.suppressErrors = (control & DSM_CFG_SUPPRESS_ERR) ? 1 : 0;
    }
    if (mask & DSM_CFG_SUPPRESS_HOOK) {
        DSMonitorThread.suppressHooks = (control & DSM_CFG_SUPPRESS_HOOK) ? 1 : 0;
    }
    if (mask & DSM_CFG_TRACE_ACTIVE) {
        DSMonitorThread.traceActive = (control & DSM_CFG_TRACE_ACTIVE) ? 1 : 0;
    }
    if (mask & DSM_CFG_SUPPRESS_PERIODIC) {
        if (control & DSM_CFG_SUPPRESS_PERIODIC) {
            DSMonitorThread.suppressPeriodic = 1;
        } else {
            DSMonitorThread.suppressPeriodic   = 0;
            DSMonitorThread.lastPeriodicUpdate = GetTickCount();
        }
    }
    if (mask & DSM_CFG_SUPPRESS_RUNNINGCB) {
        DSMonitorThread.suppressRunningCB = (control & DSM_CFG_SUPPRESS_RUNNINGCB) ? 1 : 0;
        if (!(control & DSM_CFG_SUPPRESS_RUNNINGCB) && DSMonitorThread.missedRunningCB) {
            DSMonitorThread.missedRunningCB = 0;
#if DBGCM_WITHOUT_STOP
            if (!(DSMonitorState & DSM_STATE_CPU_HALTED)) { // Still running, execute delayed RunningCB
                DSM_SuspendMonitor();
                RunningCB(RUNNING_DETECTED);
                DSM_ResumeMonitor();
            }
#endif // DBGCM_WITHOUT_STOP
        }
    }
#if DBGCM_RECOVERY
    if (mask & DSM_CFG_RECOVERY) {
        DSMonitorThread.recovery = (control & DSM_CFG_RECOVERY) ? pio->DbgRecovery : 0;
    }
#endif // DBGCM_RECOVERY
    if (mask & DSM_CFG_WAKEUP) {
        if (control & DSM_CFG_WAKEUP) {
            WaitForSingleObject(DSMonitorThread.awake, 100);
        }
    }

    DSMonitorThread.configSynced = 0;
    if (sync && DSMonitorThread.config_evt) {
        // Wake up thread in case it went into its periodic downtime
        if (!SetEvent(DSMonitorThread.wakeup))
            return (EU01); // Internal DLL Error
        for (; confSyncRetries > 0; confSyncRetries--) {
            if (HardPlayDead()) {
                return (EU01);
            }
            res = WaitForSingleObject(DSMonitorThread.config_evt, confSyncTimeout);
            if (res != WAIT_OBJECT_0 && res != WAIT_TIMEOUT)
                return (1);
            if (DSMonitorThread.configSynced)
                break;
            pio->Notify(UV_DOXEVENTS, (void *)1); // Process message queue, required if monitor thread is currently waiting for a a notification to finish
        }
    }

    return (0);
}


/*
 * Suspend Device State Monitor, external DHCSR requests still allowed,
 * function waits for current iteration to finish.
 */
int DSM_SuspendMonitor()
{
    const DWORD waitTimeout = 20;
    int         waitRetries = 50;
    DWORD       res;

    // Ignore Suspend Call if coming from monitor thread
    if (DSMonitorThread.threadID == GetCurrentThreadId()) {
        // No need to wait for suspended event, we are in the monitor thread and
        // should be sort of suspended if calling this function.
        DSMonitorThread.suspendCount++;
        return (0);
    }

    if (DSMonitorThread.suspendCount == 0 && DSMonitorThread.h != NULL) { // Suspend Monitor?
        DSMonitorThread.suspendCount++;
        // Wake up thread in case it went into its periodic downtime
        if (!SetEvent(DSMonitorThread.wakeup))
            return (EU01); // Internal DLL Error
        for (; waitRetries > 0; waitRetries--) {
            if (HardPlayDead()) {
                DSMonitorThread.suspendCount--;
                return (EU01);
            }
            res = WaitForSingleObject(DSMonitorThread.suspended, waitTimeout);
            if (res != WAIT_OBJECT_0 && res != WAIT_TIMEOUT) {
                DSMonitorThread.suspendCount--;
                return (EU01); // Internal DLL Error
            }
            if (DSMonitorThread.inSuspension)
                break;                            // Monitor Suspended (fallback if event got lost)
            pio->Notify(UV_DOXEVENTS, (void *)1); // Process message queue, required if monitor thread is currently waiting for a a notification to finish
        }
        if (waitRetries == 0 && !DSMonitorThread.inSuspension) {
            DSMonitorThread.suspendCount--;
            return (EU01); // Timed out
        }
        return (0);
    }

    DSMonitorThread.suspendCount++;
    return (0);
}


/*
 * Resume Device State Monitor
 */
int DSM_ResumeMonitor()
{
    if (DSMonitorThread.suspendCount == 0) {
        return (EU01); // Internal DLL Error
    }

    DSMonitorThread.suspendCount--;
    if (DSMonitorThread.threadID == GetCurrentThreadId()) {
        // No need to signal wakeup event, we are called from somewhere in the monitor thread and
        // hence are awake.
        return (0);
    }

    if (DSMonitorThread.suspendCount == 0 && DSMonitorThread.h != NULL) { // Wake up?
        if (!SetEvent(DSMonitorThread.wakeup))
            return (EU01); // Internal DLL Error
    }
    return (0);
}


/*
 * Wait for DHCSR to take a certain (maskable) value
 */
int DSM_WaitForState(BYTE state, BYTE mask, BOOL all, DWORD timeout)
{
    DWORD ticks, res;
    int   result  = DSM_WAIT_TIMEOUT; // Timeout
    bool  success = false;

    // Configure match setup
    DSMonitorThread.matchTimeout = timeout;
    DSMonitorThread.matchMask    = mask;
    DSMonitorThread.matchValue   = state;
    DSMonitorThread.matchAll     = all;
    if (!SetEvent(DSMonitorThread.wakeup)) {
        return (DSM_WAIT_ERR); // Error
    }
    ticks = GetTickCount();
    do {
        if (HardPlayDead()) {
            return (DSM_WAIT_ERR);
        }
        res = WaitForSingleObject(DSMonitorThread.match, 50);
        if (res != WAIT_OBJECT_0 && res != WAIT_TIMEOUT)
            return (DSM_WAIT_ERR); // Error
        if (!DSMonitorThread.up)
            return (DSM_WAIT_ERR); // Error
        if (DSMonitor_MatchState()) {
            return (DSM_WAIT_MATCH); // Successful match
        }
        if (DSMonitorThread.matchTimeout == 0 && DSMonitorThread.matchActive == 0)
            break;                             // Done but didn't seem to set match signal
        pio->Notify(UV_DOXEVENTS, (void *)1);  // Process message queue, required if monitor thread is currently waiting for a a notification to finish
    } while (GetTickCount() - ticks <= 20000); // Much larger than normal timeout, probably cut back

    return (DSM_WAIT_TIMEOUT); // Timeout without successful match
}


/*
 * External DHCSR Update Request
 *   - dhcsr: New DHCSR value to update with
 */
int DSM_ExternalDHCSR(DWORD dhcsr)
{
    int status;

    // Ignore these requests from monitor thread, handled in the main loop
    if (GetCurrentThreadId() == DSMonitorThread.threadID) {
        return (0);
    }

    status = DSM_SuspendMonitor(); // Ensure that monitor is stopped
    if (status)
        return (status);

    DSMonitorThread.dhcsr    = dhcsr;
    DSMonitorThread.extDHCSR = 1;

    status = DSM_ResumeMonitor(); // Resume monitor (or at least decrement reference counter)
    if (status)
        return (status);

    return (0);
}


/*
* Clear Device State Monitor State Bits
*   - clear: Bits of Device State Monitor State to clear
*/
int DSM_ClearState(BYTE clear)
{ // Clear Device State Monitor State Bits as specified by 'clear'
    DSMonitorState &= ~clear;
    return (0);
}


void InitDSMonitor(void)
{
    memset(&DSMonitorThread, 0, sizeof(DSMonitorThread));
    memset(&DSMonitorState, 0, sizeof(DSMonitorState));
    memset(&DSMonitorResetMsg, 0, sizeof(DSMonitorResetMsg));
}
#endif // DBGCM_DS_MONITOR