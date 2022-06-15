/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.0
 * @date     $Date: 2020-07-30 14:15:04 +0200 (Thu, 30 Jul 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM CoreSight Trace Funnel Definitions
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

#ifndef __CSTF_H__
#define __CSTF_H__

extern BOOL  CSTF_Single; // Single or no Trace Funnel detected in system
extern DWORD CSTF_Addr;

// CoreSight Trace Funnel (CSTF) Registers

// CSTF Registers Address Offsets
#define CSTF_CONTROL_OFS    0x0000 // Trace Funnel Control Register Offset
#define CSTF_PRIORITY_OFS   0x0004 // Trace Funnel Priority Register Offset
#define CSTF_ITATBCTR2_OFS  0x0EF0 // Trace Funnel Integration Test Register 2 Offset
#define CSTF_ITATBCTR0_OFS  0x0EF8 // Trace Funnel Integration Test Register 0 Offset
#define CSTF_ITATBMODE_OFS  0x0F00 // Trace Funnel Integration Mode Control Register Offset
#define CSTF_LOCKACCESS_OFS 0x0FB0 // Trace Funnel Lock Access Register Offset
#define CSTF_LOCKSTATUS_OFS 0x0FB4 // Trace Funnel Lock Status Register Offset
#define CSTF_AUTHSTATUS_OFS 0x0FB8 // Trace Funnel Authentication Status Register Offset
#define CSTF_DEVICEID_OFS   0x0FC8 // Trace Funnel Device ID Register Offset

// CSTF Registers Addresses
#define CSTF_CONTROL(offset)    (offset + 0x0000) // Trace Funnel Control Register
#define CSTF_PRIORITY(offset)   (offset + 0x0004) // Trace Funnel Priority Register
#define CSTF_ITATBCTR2(offset)  (offset + 0x0EF0) // Trace Funnel Integration Test Register 2
#define CSTF_ITATBCTR0(offset)  (offset + 0x0EF8) // Trace Funnel Integration Test Register 0
#define CSTF_ITATBMODE(offset)  (offset + 0x0F00) // Trace Funnel Integration Mode Control Register
#define CSTF_LOCKACCESS(offset) (offset + 0x0FB0) // Trace Funnel Lock Access Register
#define CSTF_LOCKSTATUS(offset) (offset + 0x0FB4) // Trace Funnel Lock Status Register
#define CSTF_AUTHSTATUS(offset) (offset + 0x0FB8) // Trace Funnel Authentication Status Register
#define CSTF_DEVICEID(offset)   (offset + 0x0FC8) // Trace Funnel Device ID Register


// CSTF Control Register definitions
#define CSTF_CONTROL_ENS0 (1UL << 0) // Enable Slave 0
#define CSTF_CONTROL_ENS1 (1UL << 1) // Enable Slave 1
#define CSTF_CONTROL_ENS2 (1UL << 2) // Enable Slave 2
#define CSTF_CONTROL_ENS3 (1UL << 3) // Enable Slave 3
#define CSTF_CONTROL_ENS4 (1UL << 4) // Enable Slave 4
#define CSTF_CONTROL_ENS5 (1UL << 5) // Enable Slave 5
#define CSTF_CONTROL_ENS6 (1UL << 6) // Enable Slave 6
#define CSTF_CONTROL_ENS7 (1UL << 7) // Enable Slave 7
#define CSTF_CONTROL_HT_M 0x00000F00 // Hold Time Mask
#define CSTF_CONTROL_HT_P 0x8        // Hold Time Position

// CSTF Priority Register definitions
#define CSTF_PRIORITY_GEN_M 0x00000007 // General Slave Mask (Position 0)
#define CSTF_PRIORITY_S0_M  0x00000007 // Slave 0 Priority Mask
#define CSTF_PRIORITY_S1_M  0x00000038 // Slave 1 Priority Mask
#define CSTF_PRIORITY_S2_M  0x000001C0 // Slave 2 Priority Mask
#define CSTF_PRIORITY_S3_M  0x00000E00 // Slave 3 Priority Mask
#define CSTF_PRIORITY_S4_M  0x00007000 // Slave 4 Priority Mask
#define CSTF_PRIORITY_S5_M  0x00038000 // Slave 5 Priority Mask
#define CSTF_PRIORITY_S6_M  0x001C0000 // Slave 6 Priority Mask
#define CSTF_PRIORITY_S7_M  0x00E00000 // Slave 7 Priority Mask

#define CSTF_PRIORITY_S0_P  (1UL << (0 * 3)) // Slave 0 Priority Position
#define CSTF_PRIORITY_S1_P  (1UL << (1 * 3)) // Slave 1 Priority Position
#define CSTF_PRIORITY_S2_P  (1UL << (2 * 3)) // Slave 2 Priority Position
#define CSTF_PRIORITY_S3_P  (1UL << (3 * 3)) // Slave 3 Priority Position
#define CSTF_PRIORITY_S4_P  (1UL << (4 * 3)) // Slave 4 Priority Position
#define CSTF_PRIORITY_S5_P  (1UL << (5 * 3)) // Slave 5 Priority Position
#define CSTF_PRIORITY_S6_P  (1UL << (6 * 3)) // Slave 6 Priority Position
#define CSTF_PRIORITY_S7_P  (1UL << (7 * 3)) // Slave 7 Priority Position


// Trace Funnel Integration Test Register 2
//  Required for topology detection.
//  Reads  access master port.
//  Writes access slave port specified in CSTF_CONTROL.
#define CSTF_ITATBCTR2_ATREADY (1UL << 0) // ATREADY signal
#define CSTF_ITATBCTR2_AFVALID (1UL << 1) // AFVALID signal

// Trace Funnel Integration Test Register 0
//  Required for topology detection.
//  Reads  access master port.
//  Writes access slave port specified in CSTF_CONTROL.
#define CSTF_ITATBCTR0_ATVALID   (1UL << 0) // ATVALID signal
#define CSTF_ITATBCTR0_AFREADY   (1UL << 1) // AFREADY signal
#define CSTF_ITATBCTR0_ATBYTES_M 0x00000300 // ATBYTES mask
#define CSTF_ITATBCTR0_ATBYTES_P 8          // ATBYTES position


// Trace Funnel Integration Mode Control Register
//  Switch between integration mode and functional mode
#define CSTF_ITATBMODE_EN (1UL << 0) // Enable integration mode


// CSTF Lock Access Register definitions
// CSTF Lock Status Register definitions
// CSTF Authentication Status Register definitions
// CSTF Device ID Register definitions
#define CSTF_DEVICEID_PORTS_M      0x0000000F // Number of Trace Funnel Input Ports (Mask)
#define CSTF_DEVICEID_PRIOSCHEME_M 0x000000F0 // Priority Scheme (Mask)
#define CSTF_DEVICEID_PRIOSCHEME_P 4          // Priority Scheme (Position)


// CSTF Instance Structure
typedef struct CSTF_Instance_t {
    CSTF_Instance_t *next;       // Next CSTF Instance
    DWORD            addr;       // CSTF Instance Address
    BYTE             ap;         // CSTF Instance Access Port
    DWORD            dp;         // CSTF Instance Debug Port
    BYTE             slavesConn; // CSTF Connected Slave Ports
    BYTE             slavesIETM; // CSTF Slave Ports to enable for ETM (Instruction Trace)
    BYTE             slavesDETM; // CSTF Slave Ports to enable for ETM (Data Trace)
    BYTE             slavesITM;  // CSTF Slave Ports to enable for DWT/ITM Trace
} CSTF_Instance;


// CSTF Functions
extern int            CSTF_AddSlavePort(DWORD addr, BYTE ap, DWORD dp, BYTE port); // Add CSTF Port and Instance if required
extern int            CSTF_AddInstance(DWORD addr, BYTE ap, DWORD dp);             // Add CSTF Instance
extern CSTF_Instance *CSTF_GetInstance(DWORD addr, BYTE ap, DWORD dp);             // Get CSTF Instance

extern int   CSTF_Setup();        // Enables ITM and ETM slave ports in Trace Funnel
extern DWORD CSTF_GetSlavesNum(); // Get number of connected ATB slave ports
extern int   CSTF_ETMConnected(); // ETM is connected to the CSTF and hence to the ETB
extern int   CSTF_ITMConnected(); // ITM is connected to the CSTF and hence to the ETB
extern int   CSTF_Recovery();     // CSTF Recovery, e.g. after low-power mode

#endif // __CSTF_H__