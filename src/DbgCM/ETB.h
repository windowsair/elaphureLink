/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.1
 * @date     $Date: 2020-07-30 14:15:04 +0200 (Thu, 30 Jul 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
 *
 * @brief     ARM Embedded Trace Buffer Definitions
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

#ifndef __ETB_H__
#define __ETB_H__

#include "Debug.h"

extern BOOL ETB_TMC; // ETB is TMC in ETB mode

extern BOOL ETB_Configured; // ETB Global Enable Flag
// CS Location (Potentially Shared in Multi-Core System)
extern CS_LOCATION ETB_Location; // ETB Register Base Location
extern BOOL        ETB_Active;   // ETB is active, use to ignore Sync errors
extern DWORD       ETB_TrigDelay;

extern DWORD ETB_PC_Value;

// Trace sources connected to ETB
extern BOOL ETB_ETMConnected;
extern BOOL ETB_ITMConnected;

// ETB Registers

// ETB Registers Address Offsets
#define ETB_ID_OFS      0x00  // ETB ID Register Offset
#define ETB_DEPTH_OFS   0x04  // ETB RAM Depth Register Offset
#define ETB_WIDTH_OFS   0x08  // ETB RAM Width Register Offset
#define ETB_STATUS_OFS  0x0C  // ETB Status Register Offset
#define ETB_RD_DATA_OFS 0x10  // ETB RAM Read Data Register Offset
#define ETB_RD_PTR_OFS  0x14  // ETB RAM Read Pointer Register Offset
#define ETB_WR_PTR_OFS  0x18  // ETB RAM Write Pointer Register Offset
#define ETB_TRG_CNT_OFS 0x1C  // ETB Trigger Counter Register Offset
#define ETB_CTRL_OFS    0x20  // ETB Control Register Offset
#define ETB_WR_DATA_OFS 0x24  // ETB RAM Write Data Register Offset
#define ETB_FFSR_OFS    0x300 // ETB Formatter and Flush Status Register
#define ETB_FFCR_OFS    0x304 // ETB Formatter and Flush Control Register

// ETB Registers Addresses
#define ETB_ID      (ETB_Location.Addr + ETB_ID_OFS)
#define ETB_DEPTH   (ETB_Location.Addr + ETB_DEPTH_OFS)
#define ETB_WIDTH   (ETB_Location.Addr + ETB_WIDTH_OFS)
#define ETB_STATUS  (ETB_Location.Addr + ETB_STATUS_OFS)
#define ETB_RD_DATA (ETB_Location.Addr + ETB_RD_DATA_OFS)
#define ETB_RD_PTR  (ETB_Location.Addr + ETB_RD_PTR_OFS)
#define ETB_WR_PTR  (ETB_Location.Addr + ETB_WR_PTR_OFS)
#define ETB_TRG_CNT (ETB_Location.Addr + ETB_TRG_CNT_OFS)
#define ETB_CTRL    (ETB_Location.Addr + ETB_CTRL_OFS)
#define ETB_WR_DATA (ETB_Location.Addr + ETB_WR_DATA_OFS)
#define ETB_FFSR    (ETB_Location.Addr + ETB_FFSR_OFS)
#define ETB_FFCR    (ETB_Location.Addr + ETB_FFCR_OFS)

// ETB ID Register definitions
#define ETB_ID_VALUE 0x1B900F0F

// ETB RAM Width Register definitions
#define ETB_WIDTH_MASK  0x0000003F // RAM Width Mask
#define ETB_WIDTH_24BIT (3UL << 3) // RAM Width 24-bit
#define ETB_WIDTH_32BIT (1UL << 5) // RAM Width 32-bit

// ETB Status Register definitions
#define ETB_STATUS_MASK      0x0000000F // Status Register Mask
#define ETB_STATUS_FULL      (1UL << 0) // ETB RAM Full
#define ETB_STATUS_TRIGGERED (1UL << 1) // Trigger Detected
#define ETB_STATUS_ACQCOMP   (1UL << 2) // Acquisition Complete
#define ETB_STATUS_DFEMPTY   (1UL << 3) // Data Formatter Pipe Empty

// ETB RAM Read Data Register definitions
#define ETB_DATA_MASK 0x00FFFFFF // ETB RAM Data Mask

// ETB RAM Read Pointer Register definitions
// ETB RAM Write Pointer Register definitions
// ETB Trigger Counter Register definitions
// maximum ETB RAM Width - 1 bits

// ETB Control Register definitions
#define ETB_CTRL_MASK   0x00000007 // ETB Control Mask
#define ETB_CTRL_CAPTEN (1UL << 0) // Trace Capture Enable
#define ETB_CTRL_DEMUX  (1UL << 1) // Demultiplexed Mem Support
#define ETB_CTRL_SWCTRL (1UL << 2) // JTAG/SW Register Access

// ETB Formatter and Flush Status Register definitions
#define ETB_FFSR_FLUSHING  (1UL << 0) // ETB Formatter Flush in Progress
#define ETB_FFSR_FTSTOPPED (1UL << 1) // ETB Formatter Stopped

// ETB Formatter and Flush Control Register definitions
#define ETB_FFCR_ENFTC   (1UL << 0) // Enable Formatting, no triggers in stream
#define ETB_FFCR_ENFCONT (1UL << 1) // Continuous Formatting Mode, triggers in stream
// Flushing
#define ETB_FFCR_FONFLIN (1UL << 4) // Flush on FLUSHIN signal
#define ETB_FFCR_FONTRIG (1UL << 5) // Flush on incoming trigger event (trigger detected and trigger counter reached 0)
#define ETB_FFCR_FONMAN  (1UL << 6) // Flush manually
// Triggering
#define ETB_FFCR_TRIGIN  (1UL << 8)  // Indicate trigger on TRIGIN high
#define ETB_FFCR_TRIGEVT (1UL << 9)  // Indicate trigger on trigger event (trigger detected and trigger counter reached 0)
#define ETB_FFCR_TRIGFL  (1UL << 10) // Indicate trigger on completed flush
// Stopping
#define ETB_FFCR_STOPFL   (1UL << 12) // Stop formatter on completed flush
#define ETB_FFCR_STOPTRIG (1UL << 13) // Stop formatter on trigger event (trigger detected and trigger counter reached 0)

// TMC (ETB Mode) Registers (additional registers or registers different from standard ETB)
#define TMC_MODE_OFS  0x028 // TMC Mode Register Offset
#define TMC_DEVID_OFS 0xFC8 // TMC Device ID Register Offset

// TMC (ETB Mode) Register Addresses
#define TMC_MODE  (ETB_Location.Addr + TMC_MODE_OFS)
#define TMC_DEVID (ETB_Location.Addr + TMC_DEVID_OFS)

// TMC Mode Register definitions
#define TMC_MODE_MASK     0x00000003 // TMC Operating Mode
#define TMC_MODE_HWFIFO   0x00000002 // Hardware FIFO Mode (ETF only)
#define TMC_MODE_SWFIFO   0x00000001 // Software FIFO Mode
#define TMC_MODE_CIRCULAR 0x00000000 // Circular Buffer Mode

// TMC Device ID Register definitions
#define TMC_DEVID_MEMWIDTH_POS  8UL        // Memory Interface Width
#define TMC_DEVID_MEMWIDTH_MASK 0x00000700 // Memory Interface Width
#define TMC_CONFIGTYPE_MASK     0x000000C0 // TMC Configuration Type Value
#define TMC_CONFIGTYPE_ETB      0x00000000 // TMC Configuration Type: ETB
#define TMC_CONFIGTYPE_ETR      0x00000040 // TMC Configuration Type: ETR
#define TMC_CONFIGTYPE_ETF      0x00000080 // TMC Configuration Type: ETF

// ETB Registers
typedef struct {
    DWORD ID;
    DWORD DEPTH;
    DWORD WIDTH;
    DWORD STATUS;
    DWORD DATA;
    DWORD RD_PTR;
    DWORD WR_PTR;
    DWORD TRG_CNT;
    DWORD CTRL;
} RgETB;

// ETB Functions
extern int  ETB_Process();       // ETB Process Trace
extern int  ETB_Activate(void);  // ETB Activate Trace
extern int  ETB_Setup(void);     // ETB Setup Trace
extern int  ETB_Init(void);      // ETB Init Trace
extern BOOL ETB_CanAccess(void); // ETB is stopped and accessible
extern int  ETB_Clear(void);     // ETB Clear, basically allowing another update
extern int  ETB_Recovery(void);  // ETB Recovery, e.g. after low-power mode
extern int  ETB_Flush();         // ETB and ATB flush
#endif
