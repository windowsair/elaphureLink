/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.2
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016 ARM Limited. All rights reserved.
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

#ifndef __DSMONITOR_H__
#define __DSMONITOR_H__

#include "COLLECT.H"

#define DSM_STATE_CPU_HALTED     0x01
#define DSM_STATE_RST_ACTIVE     0x02
#define DSM_STATE_RST_DONE       0x04
#define DSM_STATE_RUN_BEFORE_RST 0x08
#define DSM_STATE_ACC_ERR        0x80

typedef union {
  typedef struct {
    BYTE cpuHalted      : 1;
    BYTE cpuResetActive : 1;
    BYTE cpuResetDone   : 1;
    BYTE accessError    : 1;
  } s;
  BYTE state;
} DSM_STATE;

typedef struct {
  HANDLE h;                     // Thread Handle
  DWORD  threadID;              // Monitor Thread ID
  HANDLE wakeup;                // Wake up thread                   [IN]
  HANDLE suspended;             // Thread suspended signal          [OUT]
  HANDLE awake;                 // Thread started/woken up signal   [OUT]
  HANDLE match;                 // Device State Matches Request     [OUT]
  HANDLE config_evt;            // Configuration settings synced in [OUT]
  BYTE   up               : 1;  // Thread is up
  BYTE   kill             : 1;  // Kill Thread
  BYTE   recovery         : 1;  // Execute Debug Access Recovery (if enabled)
  BYTE   extDHCSR         : 1;  // External DHCSR Update Request
  BYTE   suppressHooks    : 1;  // Suppress hook-calls (to avoid lockups because of CheckCom())
  BYTE   suppressErrors   : 1;  // Suppress error output (e.g. during reset from uVision)
  BYTE   suppressPeriodic : 1;  // Suppress periodic updates
  BYTE   suppressRunningCB: 1;  // Suppress calling RunningCB() from DS Monitor thread
  BYTE   missedRunningCB  : 1;  // RunningCB() was suppressed
  BYTE   traceActive      : 1;  // Trace is active
  BYTE   configSynced     : 1;  // Thread synchronized to configuration
  BYTE   inSuspension     : 1;  // The thread has reached the suspended state
  BYTE   matchActive      : 1;  // Matching in progress
  DWORD  interval;              // Polling interval (time to sleep in between)
  DWORD  dhcsr;                 // DHCSR value for next update
  DWORD  suspendCount;          // Suspend Monitor Reference Counter
  DWORD  periodicTime;          // Minimum period between periodic updates
  DWORD  lastPeriodicUpdate;    // Tick count of last periodic update (use GetTickCount())
  DWORD  matchTimeout;
  BYTE   matchValue;
  BYTE   matchMask;
  BOOL   matchAll;
} DSM_THREAD;

extern DSM_THREAD DSMonitorThread;
extern BYTE       DSMonitorState;

#define DSM_CFG_RECOVERY           0x01   // Enable/Disable Debug Recovery
#define DSM_CFG_WAKEUP             0x02   // Wakeup Monitor to immediately take over changes
#define DSM_CFG_UPDATE_INTERVAL    0x04   // Update wait interval
#define DSM_CFG_SUPPRESS_ERR       0x08   // Supress Error Messages
#define DSM_CFG_SUPPRESS_HOOK      0x10   // Suppress Hook Execution
#define DSM_CFG_SUPPRESS_PERIODIC  0x20   // Suppress Periodic Updates
#define DSM_CFG_SUPPRESS_RUNNINGCB 0x40   // Suppress RunningCB
#define DSM_CFG_TRACE_ACTIVE       0x80   // Trace is active

#define DSM_CFG_RESET_MASK    (DSM_CFG_SUPPRESS_HOOK|DSM_CFG_SUPPRESS_ERR|DSM_CFG_SUPPRESS_PERIODIC|DSM_CFG_SUPPRESS_RUNNINGCB|DSM_CFG_RECOVERY|DSM_CFG_TRACE_ACTIVE)
#define DSM_CFG_BEFORE_RESET  (DSM_CFG_SUPPRESS_HOOK|DSM_CFG_SUPPRESS_ERR|DSM_CFG_SUPPRESS_PERIODIC|DSM_CFG_SUPPRESS_RUNNINGCB)
#define DSM_CFG_AFTER_RESET   (DSM_CFG_RECOVERY)
#define DSM_CFG_EXIT_MASK     (DSM_CFG_SUPPRESS_HOOK|DSM_CFG_SUPPRESS_ERR|DSM_CFG_SUPPRESS_PERIODIC|DSM_CFG_SUPPRESS_RUNNINGCB|DSM_CFG_RECOVERY|DSM_CFG_TRACE_ACTIVE)
#define DSM_CFG_EXIT          (DSM_CFG_SUPPRESS_HOOK|DSM_CFG_SUPPRESS_ERR|DSM_CFG_SUPPRESS_PERIODIC|DSM_CFG_SUPPRESS_RUNNINGCB)

// Error Codes for DSM_WaitForState
#define DSM_WAIT_MATCH             0x00   // Success, DSM_WaitForState resulted in match
#define DSM_WAIT_TIMEOUT           0x01   // DSM_WaitForState timed out without successful match
#define DSM_WAIT_ERR               0x02   // Error


extern int  DSM_StartMonitor();                                                 // Start Device State Monitor
extern int  DSM_StopMonitor();                                                  // Stop Device State Monitor
extern int  DSM_ConfigureMonitor(BYTE mask, BYTE control, DWORD interval, BOOL sync); // Set Device State Monitor Polling Interval
extern int  DSM_SuspendMonitor();                                               // Suspend Device State Monitor, external DHCSR requests still allowed,
                                                                                // function waits for current iteration to finish
extern int  DSM_ResumeMonitor();                                                // Resume Device State Monitor
extern int  DSM_WaitForState(BYTE state, BYTE mask, BOOL all, DWORD timeout);   // Wait for DHCSR to take a certain (maskable) value
                                                                                // Return Value: 0    - successful match
                                                                                //               1    - match timeout (so, no match)
                                                                                //               2    - error
extern int  DSM_ExternalDHCSR(DWORD dhcsr);                                     // External DHCSR Update Request
extern int  DSM_ClearState(BYTE clear);                                         // Clear Device State Monitor State Bits

#endif // __DSMONITOR_H__