/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.0
 * @date     $Date: 2020-07-30 14:15:04 +0200 (Thu, 30 Jul 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM Embedded Cross Trigger Interface Definitions
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

#ifndef __CTI_H__
#define __CTI_H__

// CTI Registers

// CTI Registers Address Offsets
#define CTI_CONTROL_OFS       0x000 // CTI Control Register Offset
#define CTI_INTACK_OFS        0x010 // CTI Interrupt Acknowledge Register Offset
#define CTI_APPSET_OFS        0x014 // CTI Application Trigger Set Register Offset
#define CTI_APPCLEAR_OFS      0x018 // CTI Application Trigger Clear Register Offset
#define CTI_APPPULSE_OFS      0x01C // CTI Application Trigger Pulse Register Offset
#define CTI_INEN0_OFS         0x020 // CTI Trigger to Channel Enable Register 0 Offset (CTIINEN  0-7: 0x020-0x03C)
#define CTI_OUTEN0_OFS        0x0A0 // CTI Channel to Trigger Enable Register 0 Offset (CTIOUTEN 0-7: 0x0A0-0x0BC)
#define CTI_TRIGINSTATUS_OFS  0x130 // CTI Trigger In Status Register Offset
#define CTI_TRIGOUTSTATUS_OFS 0x134 // CTI Trigger Out Status Register Offset
#define CTI_CHINSTATUS_OFS    0x138 // CTI Channel In Status Register Offset
#define CTI_CHOUTSTATUS_OFS   0x13C // CTI Channel Out Status Register Offset
#define CTI_GATE_OFS          0x140 // CTI Channel Gate Register Offset
#define CTI_DEVID_OFS         0xFC8 // CTI Device Configuration Register Offset


// CTI Registers Addresses
#define CTI_CONTROL(offset)       (offset + CTI_CONTROL_OFS)
#define CTI_INTACK(offset)        (offset + CTI_INTACK_OFS)
#define CTI_APPSET(offset)        (offset + CTI_APPSET_OFS)
#define CTI_APPCLEAR(offset)      (offset + CTI_APPCLEAR_OFS)
#define CTI_APPPULSE(offset)      (offset + CTI_APPPULSE_OFS)
#define CTI_INEN0(offset)         (offset + CTI_INEN0_OFS)
#define CTI_OUTEN0(offset)        (offset + CTI_OUTEN0_OFS)
#define CTI_TRIGINSTATUS(offset)  (offset + CTI_TRIGINSTATUS_OFS)
#define CTI_TRIGOUTSTATUS(offset) (offset + CTI_TRIGOUTSTATUS_OFS)
#define CTI_CHINSTATUS(offset)    (offset + CTI_CHINSTATUS_OFS)
#define CTI_CHOUTSTATUS(offset)   (offset + CTI_CHOUTSTATUS_OFS)
#define CTI_GATE(offset)          (offset + CTI_GATE_OFS)
#define CTI_DEVID(offset)         (offset + CTI_DEVID_OFS)


// CTI Control Register definitions
#define CTI_CONTROL_GLBEN 0x00000001 // Cross Trigger Mapping Enabled

// CTI Device Configuration Register definitions
#define CTI_DEVID_NUMCH     0x000F0000 // CTI DEVID Number of Channels
#define CTI_DEVID_NUMCH_P   16
#define CTI_DEVID_NUMTRIG   0x0000FF00 // CTI DEVID Number of Triggers
#define CTI_DEVID_NUMTRIG_P 8

// CTI Instance Structure
typedef struct CTI_Instance_t {
    CTI_Instance_t *next;     // Next CTI Instance
    DWORD           addr;     // CTI Instance Address
    BYTE            ap;       // CTI Instance Access Port
    DWORD           dp;       // CTI Instance Debug Port
    DWORD           triggers; // CTI Number of Triggers
    DWORD           channels; // CTI Number of Channels
} CTI_Instance;


// CTI Functions
extern int  CTI_AddInstance(DWORD addr, BYTE ap, DWORD dp);   // Add CTI Instance
extern bool CTI_Activated(void);                              // One or more CTIs activated
extern int  CTI_ClearAppTriggers(DWORD mask);                 // Clear Application Triggers for all activated CTIs
extern int  CTI_AcknowledgeTriggers(DWORD mask, bool setclr); // Acknowledge Output Triggers for all activated CTIs (setclr "pulses" the acknowledge bits)
extern int  CTI_RunStepProcessor();                           // Execute handshake mechanisms to release core from CTI EDBGRQ

// Not supported:
// - Simultaneous Restart in Multi-Core System via DBGRESTART (CTITRIGOUT[7]).

#endif
