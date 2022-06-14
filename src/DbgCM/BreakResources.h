/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.1
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016 ARM Limited. All rights reserved.
 *
 * @brief     Breakpoint Resource Manager
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

#ifndef __BREAKRESOURCES_H__
#define __BREAKRESOURCES_H__

#include "..\AGDI.H"

/* Breakpoint Resource Request Types  */
#define BRK_RES_REQ_PC        0   // Resources compare PC
#define BRK_RES_REQ_DADDR     1   // Resources compare data address
#define BRK_RES_REQ_DRANGE    2   // Resources compare data address range
#define BRK_RES_REQ_DVALUE    3   // Resources compare data value
#define BRK_RES_REQ_DLINKED1  4   // Resources compare data value and one address
#define BRK_RES_REQ_DLINKED2  5   // Resources compare data value and two addresses (range)
#define BRK_RES_REQ_CYCLE     6   // Resources compare cycle value


/* Breakpoint Resource Flags */
#define BRK_RES_FLAG_CAN_SWBREAK   0x01  // A SW breakpoint is possible for the requested break
#define BRK_RES_FLAG_ETM_START     0x02  // Tracepoint is a  Start Tracepoint
#define BRK_RES_FLAG_ETM_STOP      0x04  // Tracepoint is a  Stop Tracepoint
#define BRK_RES_FLAG_ETM_TRIG      0x08  // Tracepoint is a  Trigger Tracepoint
#define BRK_RES_FLAG_ETM_ENA_EVT   0x10  // Tracepoint is an Enable Event Tracepoint

/* FPB/Breakpoint Resources - Available and reserved */
typedef struct {
  bool           FullAddrSpace;        // Supports HW breakpoints in full address space
  bool           LHSupported;          // Single HW breakpoint can trigger on instructions on higher and lower halfword
  // Number of Breakpoint comparators and specialized types
  unsigned short NumBreaks;            /* Address breakpoints, in theory up to 127,
                                 but even A-class usually implements only up to 6. */
  unsigned short NumLiterals;          // Literal comparators for remapping

  // Number of allocated Breakpoint comparators
  unsigned short NumBreaksAlloced;
  unsigned short NumLiteralsAlloced;
} BREAK_RESOURCES;


/* DWT Resources - Available and reserved. */
typedef struct {
  // Number of DWT comparators and specialized types
  unsigned short NumComps;              // Overall number of DWT comparators
  unsigned short NumValueComps;         // DWT Comparators capable of value matching, subset of NumComps
  unsigned short NumLink2ndComps;       // DWT value match comparators capable of linking to a second address, subset of NumValueComps
  unsigned short NumLimitComps;         // DWT Comparators capable of being address limit, subset of NumComps, implies being linkable
  unsigned short NumLinkComps;          // DWT Comparators that are linkable, subset of NumValueComps (ValueComp, LimitComp, or both)
  unsigned short NumCycleComps;         // DWT Comparators capable of cycle matching

  // Maximum DWT Mask Size (v7-M only)
  unsigned short MaxMaskBits;

  // Number of allocated DWT comparators and specialized types
  unsigned short NumCompsAlloced;        // Overall number of DWT comparators
  unsigned short NumValueCompsAlloced;   // DWT Comparators capable of value matching, subset of NumComps
  unsigned short NumLink2ndCompsAlloced; // DWT value match comparators capable of linking to a second address, subset of NumValueComps
  unsigned short NumLimitCompsAlloced;   // DWT Comparators capable of being address limit, subset of NumComps, implies being linkable
  unsigned short NumLinkCompsAlloced;    // DWT Comparators that are linkable, subset of NumValueComps (ValueComp, LimitComp, or both)
  unsigned short NumCycleCompsAlloced;   // DWT Comparators capable of cycle matching

  // Number of comparators allocated as breakpoints
  unsigned short NumCompsBreakAlloced;   // DWT Comparators allocated a breakpoints (because of insufficient break resources)
} DWT_RESOURCES;


/* Breakpoints set while running */
typedef struct runBreak {
  DWORD         nAddr;    // Address of breakpoint set while running (does not consider HW Break Comparator reuse for subsequent breaks)
  BYTE    bActive : 1;    // Breakpoint is active
  BYTE   bRunComp : 1;    // Used comparator only allocated for breakpoints set during run
} RUN_BREAK;

typedef struct runBreakResources {
  DWORD              MaxRunBreaks;  // Max number of breakpoints set during run (including those reusing already allocated resources)
  DWORD              SetRunBreaks;  // Number of breakpoints set during run (including those reusing already allocated resources)
  DWORD         AddCompsBreakUsed;  // Number of additionally used comparators for HW breakpoints set during run
  RUN_BREAK            *RunBreaks;  // Details about breakpoints set during run
} RUN_BREAK_RESOURCES;


/* Potential Software Breakpoints */

// Struct containing the address of a potential software breakpoint
typedef struct swBreak {
  swBreak  *pNext;        // next potential software breakpoint (linked list)
  DWORD     nAddr;        // address of the potential software breakpoint
  BYTE      bCanSet : 1;  // can set a SW breakpoint at nAddr
  BYTE      bValid  : 1;  // SW Break Info currently valid
} SW_BREAK;

typedef struct {
  bool               Allowed;  // Software breakpoints allowed
  bool         SetSWBreakRun;  // Set software breakpoints while target is running, clear is always allowed
  unsigned long  NumSWBreaks;  // Number of potential software breakpoints
  SW_BREAK        *pSwBreaks;  // Linked list containing information about the SW breaks
} SW_BREAK_RESOURCES;


// Available ETM Features
#define ETM_CAP_IMPLEMENTED    0x01
#define ETM_CAP_START_STOP     0x02
#define ETM_CAP_START_STOP_DWT 0x04

/* ETM Resources - Available and reserved. */
typedef struct {
  unsigned short Capabilities;         // Capabilities of the ETM resource, 0 if ETM not available.
  unsigned short NumDwtInputs;         // Number of embedded ICE inputs used for DWT signals.

  unsigned short NumTrigInputsAlloced; // Number of allocated trigger logic inputs, restricted to two.
} ETM_RESOURCES;


// Used HW Resources - Resources which are actually used for implementing the breakpoints/watchpoints during target run
typedef struct {
  unsigned short NumBreaks;
  unsigned short NumLiterals;
} USED_RESOURCES;


// Error Codes
#define RES_OK                     0  // Resources available/allocated
#define RES_ERR_UNKNOWN            1  // Unknown request type
#define RES_ERR_NOTSUPPORTED       2  // Requested breakpoint type not supported by target
#define RES_ERR_WADDR_UNALIGNED    3  // Requested watch address not properly aligned
#define RES_ERR_RNG_UNALIGNED      4  // Requested range (end) not aligned
#define RES_ERR_RNG_SIZE           5  // Requested range too large
#define RES_ERR_NO_ALLOC           6  // One of the resources to free is not allocated
#define RES_ERR_INVALID_EXPR       7  // Special handling of watchpoints in driver, expression analyzed only here
#define RES_ERR_NO_BREAK           8  // Target does not support breakpoints
#define RES_ERR_NO_LITERAL         9  // Target does not support literals
#define RES_ERR_NO_WATCH          10  // Target does not support watches (DWT)
#define RES_ERR_NO_WATCHVAL       11  // Target does not support watches with value compare (DWT)
#define RES_ERR_NO_WATCHLINK1     12  // Target does not support watches with value compare for one address (DWT)
#define RES_ERR_NO_WATCHLINK2     13  // Target does not support watches with value compare for two addresses (DWT)
#define RES_ERR_NO_WATCHCYCLE     14  // Target does not support watches with cycle compare (DWT)
#define RES_ERR_NO_ETM            15  // Target does not have required ETM support
#define RES_ERR_NO_ETM_STARTSTOP  16  // Target ETM does not have Start/Stop Logic support
#define RES_ERR_NO_ETM_DWT_IN     17  // Target ETM does not have DWT inputs
#define RES_ERR_NO_ETM_TRIG       18  // Target ETM does not have trigger logic
#define RES_ERR_USED_BREAK        19  // All hardware breakpoints in target are used
#define RES_ERR_USED_LITERAL      20  // All literals in target are used
#define RES_ERR_USED_WATCH        21  // All watches in target are used
#define RES_ERR_USED_WATCHVAL     22  // All watches with value compare in target are used
#define RES_ERR_USED_WATCHLINK1   23  // All watches with value compare for one address in target are used
#define RES_ERR_USED_WATCHLINK2   24  // All watches with value compare for two addresses in target are used
#define RES_ERR_USED_WATCHCYCLE   25  // All watches with cycle compare in target are used
#define RES_ERR_USED_ETM_DWT_IN   26  // All DWT inputs to ETM are used
#define RES_ERR_USED_ETM_TRIG_IN  27  // All inputs for ETM trigger logic are used
#define RES_ERR_ADDR_NOTSUPPORTED 28  // Requested address not supported, e.g. addr >= 0x20000000 for old FPB
#define RES_ERR_INTERNAL          29  // Internal DLL error

// Initialize Resource Info
extern           int DetectBreakResources(BYTE *pVer);  // Detect breakpoint resources (FPB). Returns -1 if error, else number
                                                        // of HW breaks, pVer receives FPB architecture version
extern           int DetectDwtResources();              // Detect DWT resources. Returns -1 if error, else the number of DWT comparators.
extern unsigned long InitBreakResources();              // Initializes the rest of the hardware resource information.

// Functions to check for availability of requested resources
extern unsigned long CheckBreakResources(unsigned short nReqType, unsigned short nFlags);
extern unsigned long   CheckWatchAddress(unsigned long     nAddr, unsigned long  nMany);
// Finds the closest valid memory range which covers the requested ones
extern unsigned long  AdjustWatchAddress(AG_BP *pB);

// Actual resource management
extern unsigned long  AllocBreakResources(unsigned short nReqType, unsigned short nFlags);
extern unsigned long  AllocBreakResources(DWORD nAlloc, AG_BP *pB); // nAlloc: 0 - free, 1 - alloc, 2 - address adjustment
extern unsigned long  FreeBreakResources(unsigned short nReqType, unsigned short nFlags);
extern unsigned long  FreeAllResources();
extern          void  InvalidateBreakResources();       // Invalidate Caches for Breakpoint Resources

// Access functions for hardware capabilities
extern BOOL CanValueWatch();
extern unsigned long GetDWTValueComps();
extern unsigned long GetDWTLinkComps();
extern unsigned long GetDWTLimitComps();
extern unsigned long GetDWTCycleComps();

// Breakpoints while target is running
extern void ClearRunBreaks();
extern int  MergeRunBreaks();

// Used Resources Access Functions
extern __inline void          ClearUsedBreakResources();
extern __inline unsigned long IncUsedBreakResources();
extern __inline unsigned long DecUsedBreakResources();


// SW Breakpoint Configurations
extern const AG_SWBREAKCONF_ITEM* SwBreakGetConf(DWORD addr);
// Return values: AG_SWBC_ERR_XXX (see AGDI.H)
extern int SwBreakConfGetHead(AG_SWBREAKCONF_ITEM **pphead);
extern int SwBreakConfAdd(AG_SWBREAKCONF_ITEM *item);
extern int SwBreakConfRem(AG_SWBREAKCONF_ITEM *item);

// For debugging
extern void DumpBreakResources();
#endif //__BREAKRESOURCES_H__