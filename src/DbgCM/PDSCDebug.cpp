/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.26
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016-2020 ARM Limited. All rights reserved.
 *
 * @brief     PDSC Debug Description Support
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
#include "resource.h"

#include "Collect.h"

#if DBGCM_DBG_DESCRIPTION

#include "PDSCDebug.h"

#define _IN_TARG_                   // define if used within target
#include <stdio.h>
#include <process.h>
#include <math.h>
#include "..\ALLOC.H"
#include "..\AGDI.h"
#include "..\BOM.h"
#include "..\ComTyp.h"
#include "COLLECT.H"
#include "Debug.h"
#include "Trace.h"
#include "ETB.h"
#include "CSTF.h"
#include "CTI.h"
#include "JTAG.h"
#include "SWD.h"
#include "DbgCM.h"
#include "BreakResources.h"

#if DBGCM_RECOVERY
#include "DebugAccess.h"
#endif // DBGCM_RECOVERY

#if DBGCM_DS_MONITOR
#include "DSMonitor.h"
#endif // DBGCM_DS_MONITOR

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#pragma message ("Attention! This version produces debug messages in the command window!")
#endif


extern HANDLE         CheckCom_mtx; // CheckCom mutex: UL2CM3.cpp
extern HANDLE     PlayDeadShut_mtx; // PlayDeadShut mutex

/*
 * AGDI-Data
 */

// if an extension modeless dialog is activated, then '*pHwnd' *MUST*
// receive the HWND (m_hWnd in MFC) of the dialog.  If the dialog
// looses focus or is closed, then '*pHwnd' *MUST* be set to NULL.
// This is to ensure the proper message chain within Uv2.

static const char ErrTitle[]       = "LINK - Cortex-M Error";

static const char InfoTitle[]      = "LINK - Cortex-M";

static const char JTAGErr[]        = "Could not find an Cortex-M device in the JTAG chain!\n"
                                     "Please check the JTAG cable and the connected devices.";
static const char StopErr[]        = "Could not stop Cortex-M device!\n"
                                     "Please check the JTAG cable.";
static const char VerErr[]         = "Memory Mismatch!\n"
                                     "Address: 0x%08X\n"
                                     "  Value = 0x%02X\n"
                                     "  Expected = 0x%02X\n\n";
static const char FatalAccErr[]    = "Cannot access target.\n"
                                     "Shutting down debug session.";
static const char FatalRstErr[]    = "Cannot reset target.\n"
                                     "Shutting down debug session.";
static const char FatalReInitErr[] = "Cannot reconfigure target.\n"
                                     "Shutting down debug session.";

static const char *BPErr[] = {
  /*0*/ "This target device does not support all the defined breakpoints!\n"
        "Please reduce the number of breakpoints and start again.\n",
  /*1*/ "This target device does not support conditional breakpoints!\n"
        "Please remove all conditional breakpoints and start again.",
  /*2*/ "This target device does not support all the defined watchpoints!\n"
        "Please reduce the number of watchpoints and start again.",
  /*3*/ "This target device supports only the following watchpoints:\n"
        "     size = 2^n; n = 0..15\n"
        "     start must be 2^n aligned\n"
        "     with optional value match\n"
        "Please change the watchpoint definitions and start again.",
  /*4*/ "Operation not possible while the target device is executing.\n"
        "Please stop the target to perform this operation.\n",
  /*5*/ "It was not possible to disable all breakpoints.\n"
        "Please stop the target to perform this operation.\n",
  /*6*/ "It was not possible to kill all breakpoints.\n"
        "Please stop the target to perform this operation.\n",
  /*7*/ "This target device supports only the following data tracepoints:\n"
        "     size = 2^n; n = 0..15\n"
        "     start must be 2^n aligned\n"
        "     with optional value match\n"
        "Please change the tracepoint definitions and start again.",
  /*8*/ "Trace is disabled, please disable all tracepoints.\n",
  /*9*/ "ETM instruction trace is disabled. Please disable\n"
        "all 'Run', 'Suspend', and 'Halt' tracepoints.\n",
  /*10*/ "Target does not support ETM instruction trace. Please disable\n"
         "all 'Run', 'Suspend', and 'Halt' tracepoints.\n"
};


static const char *TrRunWarn = "'Suspend' tracepoint set without 'Run' tracepoint. ETM instruction trace capture will not start.\n"
                               "Do you want to continue?";

/* Specialized Error Messages for Non-Secure Debug */
static const char NonSecureStopErr  [] = "Secure Cortex-M device stop can be delayed!";
static const char NonSecureAccErr   [] = "Cannot access secure target.";
static const char NonSecureRstErr   [] = "Cannot reset secure target.";
static const char NonSecureReInitErr[] = "Cannot reconfigure secure target.";


// Cache management functions from AGDI.cpp
extern void Invalidate (void);
extern void FreeCache  (void);

// variables from AGDI.cpp, review to encapsulate the init (I think that's all that is done here)
extern BYTE  CntBP;       // HW Breakpoint Count
extern BYTE  CntWP;       // HW Watchpoint Count
extern DWORD UseWP[4];    // HW Watchpoint Multi-usage Count

extern VTR  *pCoreClk;       // Core Clock VTR
extern VTR *pTraceClk;       // Trace Clock VTR /* 02.04.2019 */


/*
 * Clustered memory handling.
 */

typedef struct _memmgr  {
  struct _memmgr   *next;
  UINT32            aval;
  BYTE              b[4];                   // actually 4 + cSize bytes.
} _MEMMGR;


static       _MEMMGR *_memHead = NULL;     // head of clusters
static       _MEMMGR *_memTail = NULL;     // tail of clusters
static const int      _memSize = 4096;     // cluster size

static void _MemCleanup (void)  {          // free complete cluster chain
  _MEMMGR  *mem, *next;

  mem = _memHead;                          // head of memory clusters
  for ( ; mem ; )  {
    next = mem->next;                      // save 'next'
    free (mem);                            // free
    mem = next;                            // restore 'next'
  }
  _memHead = NULL;
  _memTail = NULL;
}


static void *_xxalloc (UINT32 many)  {
  void   *tmp;

  tmp = (void *) calloc (many, 1);
  if (tmp == NULL)  {
//  printf ("\n*** FATAL-ERROR: Out of Memory\n");
//  exit (3);
  }
  return (tmp);
}


void *_GetMem (int nMany)  {          // allocate 'nMany' bytes from clustered memory
  _MEMMGR     *tmp;
  UINT32      indx;
  int        aSize;

  if (nMany > _memSize)  {                 // request exceeds cluster size
    aSize = sizeof (_MEMMGR) + nMany;      // create a seperate cluster
    tmp   = (_MEMMGR *) _xxalloc (aSize);  //
    if (_memHead == NULL)  {               // first request
      _memHead  = tmp;
      _memTail  = tmp;
    }
    else  {                                // link in front of chain -
      tmp->next = _memHead;                // so we do not loose the
      _memHead  = tmp;                     // remaining space in the current
    }                                      // tail cluster.
    tmp->aval = nMany;
    return ((void *) tmp->b);              // ptr to free Memory
  }

  aSize = sizeof (_MEMMGR) + _memSize;     // sizeof memmgr + cluster-size.

  if (_memHead == NULL)  {                 // first request
    tmp   = (_MEMMGR *) _xxalloc (aSize);
    _memHead = tmp;
    _memTail = tmp;
  }

  tmp  = _memTail;                         // tail of cluster(s)
  indx = tmp->aval;                        // bytes available
  if ((indx + nMany) > _memSize)  {        // request does not fit
    indx = 0;
    tmp  = (_MEMMGR *) _xxalloc (aSize);
    _memTail->next = tmp;
    _memTail       = tmp;
  }
  tmp->aval += nMany;
  return (&tmp->b[indx]);                  // ptr to free Memory
}


static const char *_SaveString (const char *pS)  {
  if (pS == NULL)  {
    return (NULL);
  }
  if (pS[0] == 0)  {
    return ("");
  }
  int   nL = strlen (pS);
  char *cp = (char *) _GetMem (nL + 1);
  strcpy (cp, pS);
  return (cp);
}


bool          PDSCDebug_DbgInit          = false;
bool          PDSCDebug_Initialized      = false;
U32           PDSCDebug_InitResult       = 0;
bool          PDSCDebug_Supported        = false;
bool          PDSCDebug_ExecutingSeq     = false;
bool          PDSCDebug_DevsScanned      = false;
DEBUG_CONTEXT PDSCDebug_DebugContext     = DBGCON_CONNECT; // Current Debug Context, used for Sequence Execution
bool          PDSCDebug_HasDbgPropBackup = false;
bool          PDSCDebug_SetupChange      = false;
bool          PDSCDebug_SequencesAvailable = false;
bool          PDSCDebug_TraceStarted     = false;          // 13.12.2018: Added state to avoid crash by executed TraceStop sequence after failing debug connection

static bool   StopAfterConnect           = true;

// Default and error strings
static const char* UnknownDP_Str = "Unknown Debug Port";


// Debug Configurations
static PDSC_DEBUG_PROPERTIES  PDSCDebug_DebugProperties;
static PDSC_DEBUG_PROPERTIES  PDSCDebug_DebugPropertiesBackup;        // Backup of current property settings, e.g. when opening setup dialog
static U32                    PDSCDebug_activeDebugDP   = (U32)(-1);  // Init to "-1", _SetActiveDP() will do the rest
static PDSC_SEQUENCE*         PDSCDebug_Sequences[SEQ_ID_COUNT];      // Sequences callable from driver, entry == NULL if not provided
static PDSC_ACCESS_VAR        PDSCDebug_AccessVars[AV_ID_COUNT] = {   // Subset of access vars to be set by driver on sequence call
  { NULL, "__protocol",      AV_ID_PROTOCOL,      0, },  // __protocol,      send to UV4
  { NULL, "__connection",    AV_ID_CONNECTION,    0, },  // __connection,    send to UV4
  { NULL, "__dp",            AV_ID_DP,            0, },  // __dp,     do not send to UV4, managed there
  { NULL, "__ap",            AV_ID_AP,            0, },  // __ap,     do not send to UV4, managed there
  { NULL, "__traceout",      AV_ID_TRACEOUT,      0, },  // __traceout,      send to UV4
  { NULL, "__errorcontrol",  AV_ID_ERRCONTROL,    0, },  // __errorcontrol,  send to UV4
  { NULL, "__FlashAddr",     AV_ID_FLASHADDR,     0, },  // __FlashAddr,     send to UV4
  { NULL, "__FlashLen",      AV_ID_FLASHLEN,      0, },  // __FlashLen,      send to UV4
  { NULL, "__FlashArg",      AV_ID_FLASHARG,      0, },  // __FlashArg,      send to UV4
  { NULL, "__FlashOp",       AV_ID_FLASHOP,       0, },  // __FlashOp,       send to UV4
  { NULL, "__Result",        AV_ID_RESULT,        0, },  // __Result,        send to/received from UV4
};
static DWORD _DbgConfFilePathLen = 0;       // Current length of PDSCDebug_DebugProperties.dbgConfFile->path buffer
static DWORD _PackIdLen          = 0;       // Current length of PDSCDebug_DebugProperties.packid buffer
static DWORD _LogFileLen         = 0;       // Current length of PDSCDebug_DebugProperties.logfile buffer



static const char* PDSCDebug_SequenceNames[SEQ_ID_COUNT+1] = {
  "DebugPortSetup",
  "DebugPortStart",
  "DebugPortStop",
  "DebugDeviceUnlock",
  "DebugCoreStart",
  "DebugCoreStop",
  "DebugCodeMemRemap",
  "ResetSystem",
  "ResetProcessor",
  "ResetHardware",
  "ResetHardwareAssert",
  "ResetHardwareDeassert",
  "ResetCatchSet",
  "ResetCatchClear",
  "FlashEraseDone",
  "FlashProgramDone",
  "RecoverySupportStart",
  "RecoverySupportStop",
  "RecoveryAcknowledge",
  "TraceStart",
  "TraceStop",
  "ResetCustomized",
  "FlashInit",
  "FlashUninit",
  "FlashEraseSector",
  "FlashEraseChip",
  "FlashProgramPage",
  "Unknown Sequence",
};

static const char* PDSCDebug_AccessVarNames[AV_ID_COUNT+1] = {
  "__protocol",
  "__connection",
  "__dp",
  "__ap",
  "__traceout",
  "__errorcontrol",
  "__FlashAddr",
  "__FlashLen",
  "__FlashArg",
  "__FlashOp",
  "__Result",
  "Unknwon Access Variable",
};



static const char* PDSCDebug_SequenceDescriptions[SEQ_ID_COUNT] = {
  "Prepares Debug Port for connection.",                                              // SEQ_DebugPortSetup
  "Connect to Debug Port and power up.",                                              // SEQ_DebugPortStart
  "Power down Debug Port and disconnect from it.",                                    // SEQ_DebugPortStop
  "Unlock device after connection to Debug Port.",                                    // SEQ_DebugDeviceUnlock
  "Initialize core debug system.",                                                    // SEQ_DebugCoreStart
  "Uninitialize code debug system.",                                                  // SEQ_DebugCoreStop
  "Remap application code memory to core reset vector. Used for code verification.",  // SEQ_DebugCodeMemRemap
  "Execute system-wide reset via software mechanisms. Wait for reset to finish.",     // SEQ_ResetSystem
  "Execute processor reset via software mechanisms. Wait for reset to finish.",       // SEQ_ResetProcessor
  "Execute system-wide reset via debugger reset line. Wait for reset to finish.",     // SEQ_ResetHardware
  "Assert system-wide reset via debugger reset line.",                                // SEQ_ResetHardwareAssert
  "Deassert system-wide reset via debugger reset line.",                              // SEQ_ResetHardwareDeassert
  "Configure processor where to stop after reset.",                                   // SEQ_ResetCatchSet
  "Clear target configuration applied in \"ResetSetCatch\" sequence.",                // SEQ_ResetCatchClear
  "Configure device specific logic supporting recovery from a connection loss, "      // SEQ_RecoverySupportStart
  "e.g. because of a low-power mode.",
  "Clear configuration of device specific logic supporting recovery from a "          // SEQ_RecoverySupportStop
  "connection loss, e.g. because of a low-power mode.",
  "Acknowledge recovery from a connection less to device specific logic supporting "  // SEQ_RecoverySupportAcknowledge
  " the recovery.",
  "Configure device specific logic for trace capture.",                               // SEQ_TraceStart
  "Clear configuration of device specific logic for trace capture.",                  // SEQ_TraceStop
  "Execute custom reset sequence.",                                                   // SEQ_ResetCustomized
  "Initialize target before flash operation.",                                        // SEQ_FlashInit
  "Uninitialize target after flash operation.",                                       // SEQ_FlashUninit
  "Erase a flash sector.",                                                            // SEQ_FlashEraseSector
  "Erase all on-chip flash memory of the target device.",                             // SEQ_FlashEraseChip
  "Program a flash page.",                                                            // SEQ_FlashProgramPage
};




static U32 PDSCDebug_ClockValues[] =
{
#if 0
  50000000,  //  50MHz
  33000000,  //  33MHz
  25000000,  //  25MHz
  20000000,  //  20MHz
#endif
  10000000,  //  10MHz
  5000000,   //   5MHz
  2000000,   //   2MHz
  1000000,   //   1MHz
  500000,    // 500kHz
  200000,    // 200kHz
  100000,    // 100kHz
  50000,     //  50kHz
  20000,     //  20kHz
  10000,     //  10kHz
  5000,      //   5kHz
};


void PDSCDebug_SendDebugProperties() {
  // Write Changed Settings to UV4
  pio->Notify(UV_PDSCDBG_SET_PROPERTIES, &PDSCDebug_DebugProperties);
}

U32 PDSCDebug_CreateDebugPropertiesBackup() {
  memcpy(&PDSCDebug_DebugPropertiesBackup, &PDSCDebug_DebugProperties, sizeof(PDSCDebug_DebugPropertiesBackup));
  PDSCDebug_HasDbgPropBackup = true;
  PDSCDebug_SetupChange      = false;
  return (0);
}


U32 PDSCDebug_ConfirmDebugPropertiesChange() {
  PDSCDebug_HasDbgPropBackup = false;
  PDSCDebug_SetupChange      = true;
  return (0);
}


U32 PDSCDebug_DiscardDebugPropertiesChange() {
  memcpy(&PDSCDebug_DebugProperties, &PDSCDebug_DebugPropertiesBackup, sizeof(PDSCDebug_DebugPropertiesBackup));
  PDSCDebug_HasDbgPropBackup = false;
  PDSCDebug_SetupChange      = false;
  return (0);
}


bool PDSCDebug_HasDebugPropertiesBackup() {
  return PDSCDebug_HasDbgPropBackup;
}


bool PDSCDebug_SetupChanged() {
  return PDSCDebug_SetupChange;
}


U32 PDSCDebug_SetDeviceListSWD(JDEVS *DevList, unsigned int maxdevs) {
  PDSC_DEBUG_PORT* port = NULL;
  size_t nameLen = sizeof(DevList->icname[0]);
  int status = 0;

  if (!DevList || !maxdevs) return (EU39);

  status = ClearDeviceList(&JTAG_devs);
  if (status) return (status);

  port = PDSCDebug_DebugProperties.ports;

  while (port) {
    if (port->swd.implemented) {
      if (DevList->cnt > 0) {
        return (EU29);                                // PDSC: Multiple SW Debug Port definitions.
      }
      if (DevList->icname[DevList->cnt][0] != '\0') { // && port->name[0] != '\0') {
        return (EU30);                                // PDSC: Debug Port name redifinition
      }
      //strncpy(DevList->icname[0], port->name, nameLen);
      DevList->icname[DevList->cnt][nameLen-1] = '\0';

      DevList->ic[DevList->cnt].id     = port->swd.idcode;       // 0 if not specified
      DevList->ic[DevList->cnt].ir_len = 0;
      DevList->cnt++;
    }
    port = port->next;
  }

  return (0);
}

U32 PDSCDebug_SetDeviceListJTAG(JDEVS *DevList, unsigned int maxdevs) {
  PDSC_DEBUG_PORT* port = NULL;
  int status     = 0;
  U32 tapindex   = 0;
  size_t nameLen = sizeof(DevList->icname[tapindex]);

  if (!DevList || !maxdevs) return (EU39);

  status = ClearDeviceList(&JTAG_devs);
  if (status) return (status);

  port = PDSCDebug_DebugProperties.ports;
  while (port) {
    if (port->jtag.implemented) {
      tapindex = port->jtag.tapindex;
      if (tapindex >= maxdevs) {
        return (EU33);                                 // PDSC: JTAG TAP Index out of bounds
      }

      // Fill in the defined JTAG ports by their tapindex, count will be max index + 1,
      // missing items are supposed to be filled in by the JTAG scan
      if (DevList->ic[tapindex].id != 0 && port->jtag.idcode != 0) {
        return (EU31);                                 // PDSC: JTAG Debug Port ID Code redifinition
      }
      DevList->ic[tapindex].id = port->jtag.idcode;
      if (DevList->ic[tapindex].ir_len != 0 && port->jtag.irlen != 0) {
        return (EU32);                                 // PDSC: JTAG Debug Port IR Length redifinition
      }
      DevList->ic[tapindex].ir_len = port->jtag.irlen;
      if (DevList->icname[tapindex][0] != '\0') {
        return (EU30);                                 // PDSC: Debug Port name redifinition
      }
      DevList->icname[tapindex][nameLen-1] = '\0';

      if (tapindex + 1 > DevList->cnt) {
        DevList->cnt = tapindex + 1;
      }

      //port->jtag.targetsel  // TODO: support multi-drop
    }
    port = port->next;
  }

  return (0);
}

/*
 * PDSCDebug_SetDeviceList(): Fill Link Devs list with available debug port information
 *
 *
 */
U32 PDSCDebug_SetDeviceList(JDEVS *DevList, unsigned int maxdevs) {
  if (PDSCDebug_DebugProperties.protocol != PROTOCOL_SWD && PDSCDebug_DebugProperties.protocol != PROTOCOL_JTAG) {
    return (EU20);  // Internal DLL Error: Unsupported Debug Protocol
  }

  if (PDSCDebug_DebugProperties.protocol == PROTOCOL_SWD) {
    // Serial Wire Debug
    return PDSCDebug_SetDeviceListSWD(DevList, maxdevs);
  } else {
    // JTAG
    return PDSCDebug_SetDeviceListJTAG(DevList, maxdevs);
  }
  return (0);
}


char* _DuplicateString (const char* src) {
  U32 length;
  char *dest;

  length = strlen(src);
  dest   = (char*)_GetMem(length + 1);
  if (length) {
    strncpy(dest, src, length);
  }
  dest[length] = '\0';
  return dest;
}


bool _StoreDebugAttribute(PDSC_DEBUG_ATTRIB* dest, PDSC_DEBUG_ATTRIB* src) {
  PDSC_DEBUG_ATTRIB* next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  dest->name  = _DuplicateString(src->name);
  dest->value = _DuplicateString(src->value);
  dest->next  = next;

  return true;
}


bool _StoreDebugAttributes(PDSC_DEBUG_ATTRIB** dest, PDSC_DEBUG_ATTRIB* src) {
  PDSC_DEBUG_ATTRIB* destH;
  PDSC_DEBUG_ATTRIB* destT;
  PDSC_DEBUG_ATTRIB* sAttr;
  PDSC_DEBUG_ATTRIB* dAttr;

  if (dest == NULL || src == NULL) {
    return false;
  }

  destH = destT = NULL;
  for (sAttr = src; sAttr != NULL; sAttr = sAttr->next) {
    dAttr = (PDSC_DEBUG_ATTRIB*)_GetMem(sizeof(PDSC_DEBUG_ATTRIB));
    memset(dAttr, 0, sizeof(PDSC_DEBUG_ATTRIB));
    if (!_StoreDebugAttribute(dAttr, sAttr)) {
      return false;
    }
    if (destH == NULL) {
      destH = destT = dAttr;
    } else {
      destT->next = dAttr;
      destT       = dAttr;
    }
    destT->next = NULL;
  }

  *dest = destH;

  return true;
}


bool _StoreDebugPort(PDSC_DEBUG_PORT* dest, PDSC_DEBUG_PORT* src) {
  PDSC_DEBUG_PORT *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_DEBUG_PORT));
  _StoreDebugAttributes(&(dest->attribH), src->attribH);
  dest->next = next;

  return true;
}

bool _StoreDebugPorts(PDSC_DEBUG_PROPERTIES* dest, PDSC_DEBUG_PROPERTIES* src) {
  // Reuse existing memory if possible
  PDSC_DEBUG_PORT *srcPort   = NULL;
  PDSC_DEBUG_PORT *destPort  = NULL;
  PDSC_DEBUG_PORT *destPortT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->ports == NULL) {
    dest->ports = NULL;
    return true;
  }

  srcPort  = src->ports;
  destPort = dest->ports;
  while (srcPort != NULL || destPort != NULL) {
    if (srcPort != NULL) {
      // New port to add
      if (destPort == NULL) {
        // Create new destination port
        destPort = (PDSC_DEBUG_PORT*)_GetMem(sizeof(PDSC_DEBUG_PORT));
        memset(destPort, 0, sizeof(PDSC_DEBUG_PORT));
        if (dest->ports == NULL) {
          dest->ports = destPortT = destPort;
        } else {
          destPortT->next = destPort;
          destPortT       = destPortT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreDebugPort(destPort, srcPort)) {
        return false;
      }

      srcPort  = srcPort->next;
      destPort = destPort->next;
    } else if (destPort != NULL) {
      // Remove destination port
      destPortT      = destPort->next;
      destPort->next = NULL;
      destPort       = destPortT;
    }
  }
  return true;
}


bool _StoreDataPatch(PDSC_DATA_PATCH *dest, PDSC_DATA_PATCH *src) {
  PDSC_DATA_PATCH *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_DATA_PATCH));
  dest->next = next;

  return true;
}

bool _StoreDataPatches(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  // Reuse existing memory if possible
  PDSC_DATA_PATCH *srcPatch   = NULL;
  PDSC_DATA_PATCH *destPatch  = NULL;
  PDSC_DATA_PATCH *destPatchT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->dataPatches == NULL) {
    dest->dataPatches = NULL;
    return true;
  }

  srcPatch  = src->dataPatches;
  destPatch = dest->dataPatches;
  while (srcPatch != NULL || destPatch != NULL) {
    if (srcPatch != NULL) {
      // New port to add
      if (destPatch == NULL) {
        // Create new destination port
        destPatch = (PDSC_DATA_PATCH*)_GetMem(sizeof(PDSC_DATA_PATCH));
        memset(destPatch, 0, sizeof(PDSC_DATA_PATCH));
        if (dest->dataPatches == NULL) {
          dest->dataPatches = destPatchT = destPatch;
        } else {
          destPatchT->next = destPatch;
          destPatchT       = destPatchT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreDataPatch(destPatch, srcPatch)) {
        return false;
      }

      srcPatch  = srcPatch->next;
      destPatch = destPatch->next;
    } else if (destPatch != NULL) {
      // Remove destination port
      destPatchT      = destPatch->next;
      destPatch->next = NULL;
      destPatch       = destPatchT;
    }
  }


  return true;
}


bool _StoreTraceCapSWO(PDSC_TRACE_SWO *dest, PDSC_TRACE_SWO *src) {
  PDSC_TRACE_SWO *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_TRACE_SWO));
  dest->next = next;

  return true;
}


bool _StoreTraceCapsSWO(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  // Reuse existing memory if possible
  PDSC_TRACE_SWO *srcSWO   = NULL;
  PDSC_TRACE_SWO *destSWO  = NULL;
  PDSC_TRACE_SWO *destSWOT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->trace.swo == NULL) {
    dest->trace.swo = NULL;
    return true;
  }

  srcSWO  = src->trace.swo;
  destSWO = dest->trace.swo;
  while (srcSWO != NULL || destSWO != NULL) {
    if (srcSWO != NULL) {
      // New port to add
      if (destSWO == NULL) {
        // Create new destination port
        destSWO = (PDSC_TRACE_SWO*)_GetMem(sizeof(PDSC_TRACE_SWO));
        memset(destSWO, 0, sizeof(PDSC_TRACE_SWO));
        if (dest->trace.swo == NULL) {
          dest->trace.swo = destSWOT = destSWO;
        } else {
          destSWOT->next = destSWO;
          destSWOT       = destSWOT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreTraceCapSWO(destSWO, srcSWO)) {
        return false;
      }

      srcSWO  = srcSWO->next;
      destSWO = destSWO->next;
    } else if (destSWO != NULL) {
      // Remove destination port
      destSWOT      = destSWO->next;
      destSWO->next = NULL;
      destSWO       = destSWOT;
    }
  }

  return true;
}


bool _StoreTraceCapPort(PDSC_TRACE_PORT *dest, PDSC_TRACE_PORT *src) {
  PDSC_TRACE_PORT *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_TRACE_PORT));
  dest->next = next;

  return true;
}




bool _StoreTraceCapsPort(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  // Reuse existing memory if possible
  PDSC_TRACE_PORT *srcPort   = NULL;
  PDSC_TRACE_PORT *destPort  = NULL;
  PDSC_TRACE_PORT *destPortT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->trace.port == NULL) {
    dest->trace.port = NULL;
    return true;
  }

  srcPort  = src->trace.port;
  destPort = dest->trace.port;
  while (srcPort != NULL || destPort != NULL) {
    if (srcPort != NULL) {
      // New port to add
      if (destPort == NULL) {
        // Create new destination port
        destPort = (PDSC_TRACE_PORT*)_GetMem(sizeof(PDSC_TRACE_PORT));
        memset(destPort, 0, sizeof(PDSC_TRACE_PORT));
        if (dest->trace.port == NULL) {
          dest->trace.port = destPortT = destPort;
        } else {
          destPortT->next = destPort;
          destPortT       = destPortT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreTraceCapPort(destPort, srcPort)) {
        return false;
      }

      srcPort  = srcPort->next;
      destPort = destPort->next;
    } else if (destPort != NULL) {
      // Remove destination port
      destPortT      = destPort->next;
      destPort->next = NULL;
      destPort       = destPortT;
    }
  }

  return true;
}


bool _StoreTraceCapBuffer(PDSC_TRACE_BUFFER *dest, PDSC_TRACE_BUFFER *src) {
  PDSC_TRACE_BUFFER *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_TRACE_BUFFER));
  dest->next = next;

  return true;
}


bool _StoreTraceCapsBuffer(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  // Reuse existing memory if possible
  PDSC_TRACE_BUFFER *srcBuffer   = NULL;
  PDSC_TRACE_BUFFER *destBuffer  = NULL;
  PDSC_TRACE_BUFFER *destBufferT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->trace.buffer == NULL) {
    dest->trace.buffer = NULL;
    return true;
  }

  srcBuffer  = src->trace.buffer;
  destBuffer = dest->trace.buffer;
  while (srcBuffer != NULL || destBuffer != NULL) {
    if (srcBuffer != NULL) {
      // New port to add
      if (destBuffer == NULL) {
        // Create new destination port
        destBuffer = (PDSC_TRACE_BUFFER*)_GetMem(sizeof(PDSC_TRACE_BUFFER));
        memset(destBuffer, 0, sizeof(PDSC_TRACE_BUFFER));
        if (dest->trace.buffer == NULL) {
          dest->trace.buffer = destBufferT = destBuffer;
        } else {
          destBufferT->next = destBuffer;
          destBufferT       = destBufferT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreTraceCapBuffer(destBuffer, srcBuffer)) {
        return false;
      }

      srcBuffer  = srcBuffer->next;
      destBuffer = destBuffer->next;
    } else if (destBuffer != NULL) {
      // Remove destination port
      destBufferT      = destBuffer->next;
      destBuffer->next = NULL;
      destBuffer       = destBufferT;
    }
  }

  return true;
}

bool _StoreTraceCaps(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  PDSC_TRACE_SWO    *swo;
  PDSC_TRACE_PORT   *port;
  PDSC_TRACE_BUFFER *buffer;

  if (dest == NULL || src == NULL) {
    return false;
  }

  swo    = dest->trace.swo;
  port   = dest->trace.port;
  buffer = dest->trace.buffer;
  memcpy(&(dest->trace), &(src->trace), sizeof(PDSC_TRACE));
  dest->trace.swo    = swo;
  dest->trace.port   = port;
  dest->trace.buffer = buffer;

  // SWO Caps
  if (!_StoreTraceCapsSWO(dest, src)) {
    return false;
  }

  // Trace Port Caps
  if (!_StoreTraceCapsPort(dest, src)) {
    return false;
  }

  // Trace Buffer Caps
  if (!_StoreTraceCapsBuffer(dest, src)) {
    return false;
  }

  return true;
}


bool _StoreSequence(PDSC_SEQUENCE *dest, PDSC_SEQUENCE *src) {
  PDSC_SEQUENCE *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_SEQUENCE));
  dest->next = next;

  if (dest->id < SEQ_ID_COUNT) {
    if (strcmpi(dest->name, PDSCDebug_SequenceNames[dest->id]) == 0) {
      // Known Sequence, add to sequences array
      PDSCDebug_Sequences[dest->id] = dest;
      PDSCDebug_SequencesAvailable  = true;
    }
  }
  if (dest->id == PDSCDebug_DebugProperties.debugConfig.defaultRstSeqId) {
    PDSCDebug_Sequences[SEQ_ResetCustomized] = dest;
    PDSCDebug_SequencesAvailable             = true;
  }

  return true;
}


bool _StoreSequences(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  // Reuse existing memory if possible
  PDSC_SEQUENCE *srcSeq   = NULL;
  PDSC_SEQUENCE *destSeq  = NULL;
  PDSC_SEQUENCE *destSeqT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->sequences == NULL) {
    dest->sequences = NULL;
    return true;
  }

  srcSeq  = src->sequences;
  destSeq = dest->sequences;
  while (srcSeq != NULL || destSeq != NULL) {
    if (srcSeq != NULL) {
      // New port to add
      if (destSeq == NULL) {
        // Create new destination port
        destSeq = (PDSC_SEQUENCE*)_GetMem(sizeof(PDSC_SEQUENCE));
        memset(destSeq, 0, sizeof(PDSC_SEQUENCE));
        if (dest->sequences == NULL) {
          dest->sequences = destSeqT = destSeq;
        } else {
          destSeqT->next = destSeq;
          destSeqT       = destSeqT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreSequence(destSeq, srcSeq)) {
        return false;
      }

      srcSeq  = srcSeq->next;
      destSeq = destSeq->next;
    } else if (destSeq != NULL) {
      // Remove destination port
      destSeqT      = destSeq->next;
      destSeq->next = NULL;
      destSeq       = destSeqT;
    }
  }

  return true;
}


bool _StoreDbgConfFileInfo(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  DWORD     len = 0;
  char *modpath = NULL;
  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->dbgConfFile == NULL) {
    dest->dbgConfFile = NULL;
    return (true);
  }

  if (dest->dbgConfFile == NULL) {
    dest->dbgConfFile = (PDSC_DBGCONF_FILE*)_GetMem(sizeof(PDSC_DBGCONF_FILE));
  }
  if (dest->dbgConfFile == NULL) {
    return (false);
  }

  if (src->dbgConfFile->path) {
    len = strlen(src->dbgConfFile->path);
  }

  if (len < 0) {
    return (false);
  }

  if (dest->dbgConfFile->path == NULL) {
    if (len == 0 && len <= MAX_PATH) {
      _DbgConfFilePathLen = (MAX_PATH+1);
    } else {
      _DbgConfFilePathLen = (len+1);
    }
    dest->dbgConfFile->path = (char*)_GetMem(sizeof(char)*(_DbgConfFilePathLen));
  } else {
    if (len + 1 > _DbgConfFilePathLen) {
      // Throw away the existing buffer and allocate new memory, not nice but should not happen frequently
      _DbgConfFilePathLen     = (len+1);
      dest->dbgConfFile->path = (char*)_GetMem(sizeof(char)*(_DbgConfFilePathLen));
    }
  }

  // Not nice but reduces overhead of handling additional variable
  modpath = const_cast<char*>(dest->dbgConfFile->path);

  if (len <= 0) {
    modpath[0] = '\0';
  } else {
    strncpy(modpath, src->dbgConfFile->path, len);
  }

  return true;
}


bool _StorePackId(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  DWORD len = 0;
  char *modid = NULL;
  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->packid == NULL) {
    dest->packid = NULL;
    return (true);
  }

  len = strlen(src->packid);
  if (len < 0) {
    return (false);
  }

  if (len > 0) {
    if (dest->packid == NULL || len+1 > _PackIdLen) {
      _PackIdLen = len+1;
      dest->packid = (const char*)_GetMem(sizeof(char)*_PackIdLen);
    }
  }

  if (dest->packid == NULL) {
    return true;
  }

  modid = const_cast<char*>(dest->packid);

  if (len <= 0) {
    modid[0] = '\0';
  } else {
    strncpy(modid, src->packid, len);
  }

  return (true);
}


bool _StoreLogFile(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  DWORD len = 0;
  char *modpath = NULL;
  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->logfile == NULL) {
    dest->logfile = NULL;
    return (true);
  }

  len = strlen(src->logfile);
  if (len < 0) {
    return (false);
  }

  if (len > 0) {
    if (dest->logfile == NULL || len+1 > _LogFileLen) {
      _LogFileLen = len+1;
      dest->logfile = (const char*)_GetMem(sizeof(char)*_LogFileLen);
    }
  }

  if (dest->logfile == NULL) {
    return (true);
  }

  modpath = const_cast<char*>(dest->logfile);

  if (len <= 0) {
    modpath[0] = '\0';
  } else {
    strncpy(modpath, src->logfile, len);
  }

  return (true);
}


bool _StoreAccessPort(PDSC_ACCESS_PORT *dest, PDSC_ACCESS_PORT *src) {
  PDSC_ACCESS_PORT *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_ACCESS_PORT));
  _StoreDebugAttributes(&(dest->attribH), src->attribH);
  dest->next = next;

  return true;
}


bool _StoreAccessPorts(PDSC_DEBUG_PROPERTIES* dest, PDSC_DEBUG_PROPERTIES* src) {
  // Reuse existing memory if possible
  PDSC_ACCESS_PORT *srcPort   = NULL;
  PDSC_ACCESS_PORT *destPort  = NULL;
  PDSC_ACCESS_PORT *destPortT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->accessPorts == NULL) {
    dest->accessPorts = NULL;
    return true;
  }

  srcPort  = src->accessPorts;
  destPort = dest->accessPorts;
  while (srcPort != NULL || destPort != NULL) {
    if (srcPort != NULL) {
      // New port to add
      if (destPort == NULL) {
        // Create new destination port
        destPort = (PDSC_ACCESS_PORT*)_GetMem(sizeof(PDSC_ACCESS_PORT));
        memset(destPort, 0, sizeof(PDSC_ACCESS_PORT));
        if (dest->accessPorts == NULL) {
          dest->accessPorts = destPortT = destPort;
        } else {
          destPortT->next = destPort;
          destPortT       = destPortT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreAccessPort(destPort, srcPort)) {
        return false;
      }

      srcPort  = srcPort->next;
      destPort = destPort->next;
    } else if (destPort != NULL) {
      // Remove destination port
      destPortT      = destPort->next;
      destPort->next = NULL;
      destPort       = destPortT;
    }
  }
  return true;
}


bool _StoreDebugBlock(PDSC_DEBUG_BLOCK *dest, PDSC_DEBUG_BLOCK *src) {
  PDSC_DEBUG_BLOCK *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_DEBUG_BLOCK));
  _StoreDebugAttributes(&(dest->attribH), src->attribH);
  dest->next = next;

  return true;
}


bool _StoreDebugBlocks(PDSC_DEBUG_PROPERTIES* dest, PDSC_DEBUG_PROPERTIES* src) {
  // Reuse existing memory if possible
  PDSC_DEBUG_BLOCK *srcBlock   = NULL;
  PDSC_DEBUG_BLOCK *destBlock  = NULL;
  PDSC_DEBUG_BLOCK *destBlockT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->debugBlocks == NULL) {
    dest->debugBlocks = NULL;
    return true;
  }

  srcBlock  = src->debugBlocks;
  destBlock = dest->debugBlocks;
  while (srcBlock != NULL || destBlock != NULL) {
    if (srcBlock != NULL) {
      // New Block to add
      if (destBlock == NULL) {
        // Create new destination Block
        destBlock = (PDSC_DEBUG_BLOCK*)_GetMem(sizeof(PDSC_DEBUG_BLOCK));
        memset(destBlock, 0, sizeof(PDSC_DEBUG_BLOCK));
        if (dest->debugBlocks == NULL) {
          dest->debugBlocks = destBlockT = destBlock;
        } else {
          destBlockT->next = destBlock;
          destBlockT       = destBlockT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreDebugBlock(destBlock, srcBlock)) {
        return false;
      }

      srcBlock  = srcBlock->next;
      destBlock = destBlock->next;
    } else if (destBlock != NULL) {
      // Remove destination Block
      destBlockT      = destBlock->next;
      destBlock->next = NULL;
      destBlock       = destBlockT;
    }
  }
  return true;
}


bool _StoreTopologyLink(PDSC_TOPOLOGY_LINK *dest, PDSC_TOPOLOGY_LINK *src) {
  PDSC_TOPOLOGY_LINK *next;

  if (dest == NULL || src == NULL) {
    return false;
  }

  next = dest->next;
  memcpy(dest, src, sizeof(PDSC_TOPOLOGY_LINK));
  dest->next = next;

  return true;
}


bool _StoreTopologyLinks(PDSC_DEBUG_PROPERTIES* dest, PDSC_DEBUG_PROPERTIES* src) {
  // Reuse existing memory if possible
  PDSC_TOPOLOGY_LINK *srcLink   = NULL;
  PDSC_TOPOLOGY_LINK *destLink  = NULL;
  PDSC_TOPOLOGY_LINK *destLinkT = NULL;

  if (dest == NULL || src == NULL) {
    return false;
  }

  if (src->topologyLinks == NULL) {
    dest->topologyLinks = NULL;
    return true;
  }

  srcLink  = src->topologyLinks;
  destLink = dest->topologyLinks;
  while (srcLink != NULL || destLink != NULL) {
    if (srcLink != NULL) {
      // New Link to add
      if (destLink == NULL) {
        // Create new destination Link
        destLink = (PDSC_TOPOLOGY_LINK*)_GetMem(sizeof(PDSC_TOPOLOGY_LINK));
        memset(destLink, 0, sizeof(PDSC_TOPOLOGY_LINK));
        if (dest->topologyLinks == NULL) {
          dest->topologyLinks = destLinkT = destLink;
        } else {
          destLinkT->next = destLink;
          destLinkT       = destLinkT->next;
        }
      }
      // Copy src -> dest
      if (!_StoreTopologyLink(destLink, srcLink)) {
        return false;
      }

      srcLink  = srcLink->next;
      destLink = destLink->next;
    } else if (destLink != NULL) {
      // Remove destination Link
      destLinkT      = destLink->next;
      destLink->next = NULL;
      destLink       = destLinkT;
    }
  }
  return true;
}

bool _StoreDebugProperties(PDSC_DEBUG_PROPERTIES *dest, PDSC_DEBUG_PROPERTIES *src) {
  if (dest == NULL || src == NULL) {
    return false;
  }

  memcpy(&(dest->debugConfig),   &(src->debugConfig),   sizeof(PDSC_DEBUG_CONFIG));
  dest->protocol       = src->protocol;
  dest->debugClock     = src->debugClock;
  dest->enabled        = src->enabled;
  dest->log            = src->log;
  dest->sdf            = src->sdf;
  dest->sdf_version    = src->sdf_version;
  memcpy(dest->reserved, src->reserved, sizeof(src->reserved));

  // Debug Ports
  if (!_StoreDebugPorts(dest, src)) {
    return (false);
  }

  // Data Patches
  if (!_StoreDataPatches(dest, src)) {
    return (false);
  }

  // Trace Capabilities
  if (!_StoreTraceCaps(dest, src)) {
    return (false);
  }

  // Sequences
  if (!_StoreSequences(dest, src)) {
    return (false);
  }

  // DBGCONF file info
  if (!_StoreDbgConfFileInfo(dest, src)) {
    return (false);
  }

  // Pack ID
  if (!_StorePackId(dest, src)) {
    return (false);
  }

  // Log File Name
  if (!_StoreLogFile(dest, src)) {
    return (false);
  }

  // SDF Access Ports
  if (!_StoreAccessPorts(dest, src)) {
    return (false);
  }

  // SDF Debug Blocks
  if (!_StoreDebugBlocks(dest, src)) {
    return (false);
  }

  // SDF Topology Links
  if (!_StoreTopologyLinks(dest, src)) {
    return (false);
  }

  return true;
}


void _ClearSequences() {
  memset(PDSCDebug_Sequences, 0, sizeof(PDSCDebug_Sequences));
}


void _ClearAccessVar(ACCESSVAR_ID id) {
  if (id >= AV_ID_UNKNOWN) {
    return;
  }
  PDSCDebug_AccessVars[id].value = 0;
}


void _ClearAccessVars() {
  int i;
  for (i = 0; i < AV_ID_COUNT; i++) {
    PDSCDebug_AccessVars[i].value = 0;
  }
}

void _LinkAccessVars() {
  // !!! Cannot simply iterate over table, __dp and __ap need to be excluded when sending variables to UV4
  PDSCDebug_AccessVars[AV_ID_PROTOCOL].next   = &PDSCDebug_AccessVars[AV_ID_CONNECTION];
  PDSCDebug_AccessVars[AV_ID_CONNECTION].next = &PDSCDebug_AccessVars[AV_ID_TRACEOUT];
  PDSCDebug_AccessVars[AV_ID_TRACEOUT].next   = &PDSCDebug_AccessVars[AV_ID_ERRCONTROL];
  PDSCDebug_AccessVars[AV_ID_ERRCONTROL].next = NULL;
}


void _SetAccessVars() {
  PDSC_ACCESS_VAR *var;
  U32 rstType = 0;

  // __protocol
  var = &(PDSCDebug_AccessVars[AV_ID_PROTOCOL]);
  if (var != NULL) {
    var->value = (MonConf.Opt & PORT_SW) ? AV_PROT_TYPE_SWD : AV_PROT_TYPE_JTAG;
    if (MonConf.Opt & USE_SWJ) {
      var->value |= AV_PROTOCOL_SWJ;
    }
    // 07.10.2019: Support for switch throuhg dormant state in DebugPortSetup.
    if (PDSCDebug_DebugProperties.debugConfig.dormant) {
      var->value |= AV_PROTOCOL_DORMANT;
    }
  }

  // __connection
  var = &(PDSCDebug_AccessVars[AV_ID_CONNECTION]);
  if (var != NULL) {
    var->value = (pio->FlashLoad) ? AV_CONN_TYPE_FLASH : AV_CONN_TYPE_DEBUG;
    rstType = (MonConf.Opt & RESET_TYPE) >> 8;
    if (rstType == 0) {
      rstType = AV_CONN_RST_SYS;
    } else if (rstType > AV_CONN_RST_VEC) {
      rstType = 0; // error
    }
    var->value |= rstType << AV_CONN_RST_P;
    if (MonConf.Opt & INIT_RST_HOLD) {      // 06.06.2018: Indicate connection under reset in __connection variable
      var->value |= AV_CONN_HOLD_RST;
    }
  }

#if 0  // These are handled in UV4 and should not be sent with the sequence context
  // __dp
  var = &(PDSCDebug_AccessVars[AV_ID_DP]);
  if (var != NULL) {
    var->value = PDSCDebug_DebugProperties.debugConfig.defaultDP;
  }

  // __ap
  var = &(PDSCDebug_AccessVars[AV_ID_AP]);
  if (var != NULL) {
    var->value = PDSCDebug_DebugProperties.debugConfig.defaultAP;
  }
#endif

  // __traceout
  var = &(PDSCDebug_AccessVars[AV_ID_TRACEOUT]);
  if (var != NULL) {
    var->value = 0;

    if (TraceConf.Opt & TRACE_ENABLE) {
      switch (TraceConf.Protocol) {
      case TPIU_SWO_MANCHESTER:
      case TPIU_SWO_UART:
        var->value |= AV_TRACEOUT_SWO;
        break;
      case TPIU_TRACE_PORT:
        switch (TraceConf.PortSize) {
        case 0x1:
          var->value |= (1 << AV_TRACEOUT_PORTSZ_P);
          break;
        case 0x2:
          var->value |= (2 << AV_TRACEOUT_PORTSZ_P);
          break;
        case 0x8:
          var->value |= (4 << AV_TRACEOUT_PORTSZ_P);
          break;
        default:
          break;
        }
        if (var->value != 0) {
          var->value |= AV_TRACEOUT_PORT;
        }  // else a configuration error
        break;
      case TPIU_ETB:
        var->value |= AV_TRACEOUT_BUFFER;
        break;
      default:
        break;
      }
    }
  }

  // __errorcontrol
  var = &(PDSCDebug_AccessVars[AV_ID_ERRCONTROL]);
  if (var != NULL) {
    var->value = 0;
  }
}

U32 _ExecuteSequence(SEQUENCE_ID id) {
  DWORD notfiyRes = 0;

  if (id >= SEQ_ID_UNKNOWN) {
    return (EU35);      // PDSC: Unknown Sequence ID.\nCannot execute Sequence.
  }

  PDSC_SEQUENCE *sequence = PDSCDebug_Sequences[id];
  if (sequence == NULL) {
    return (EU26);      // PDSC: Sequence not implemented
  }

  if (sequence->disable != 0) {
    return (EU37);      // PDSC: Sequence disabled
  }

  PDSC_SEQUENCE_CONTEXT context;
  memset(&context, 0, sizeof(PDSC_SEQUENCE_CONTEXT));

  context.id           = sequence->id;
  context.vars         = PDSCDebug_AccessVars;   // Change this if the first access var needs to be excluded
  context.debugContext = PDSCDebug_DebugContext; // Current Debug Context

  PDSCDebug_ExecutingSeq = true;
  notfiyRes = pio->Notify(UV_PDSCDBG_EXEC_SEQUENCE, &context);
  PDSCDebug_ExecutingSeq = false;
  if (notfiyRes == 0) { // not implemented
    return (EU26);      // PDSC: Sequence not implemented
  } else if (notfiyRes != 1) {
    return (EU27);      // PDSC: Sequence Execution failed
  }

  return (0);
}

PDSC_DEBUG_BLOCK* _GetDebugBlock (U32 id) {
  PDSC_DEBUG_BLOCK *result = PDSCDebug_DebugProperties.debugBlocks;
  for (; result != NULL; result = result->next) {
    if (result->id == id) {
      break;
    }
  }
  return result;
}

PDSC_DEBUG_ATTRIB* _GetDebugAttribute(const char *name, PDSC_DEBUG_ATTRIB *head) {
  for (; head != NULL; head = head->next) {
    if (!strcmpi(name, head->name)) {
      return head;
    }
  }
  return NULL;
}


DWORD _GetETMVersion (PDSC_DEBUG_BLOCK *block) {
  bool             hasDot;
  PDSC_DEBUG_ATTRIB* attr;
  DWORD           version;

  attr   = _GetDebugAttribute("VERSION", block->attribH);
  if (attr == NULL) {
    return (3);      // Fall back to version 3
  }
  hasDot = (strchr(attr->value, '.') != NULL);

  // Only intersted in major version
  if (sscanf_s(attr->value, "%d", &version) == 0) {
    version = 3;     // Fall back to version 3
  }
  return version;
}


DWORD _GetTCMType (PDSC_DEBUG_BLOCK *block) {
  DWORD              type;
  PDSC_DEBUG_ATTRIB* attr;

  attr = _GetDebugAttribute("CONFIG_TYPE", block->attribH);
  if (attr == NULL) {
    return TMC_CONFIGTYPE_ETB;  // Fall back to ETB mode
  }

  if (!strcmpi(attr->value, "ETB")) {
    type = TMC_CONFIGTYPE_ETB;
  } else if (!strcmpi(attr->value, "ETR")) {
    type = TMC_CONFIGTYPE_ETR;
  } else if (!strcmpi(attr->value, "ETF")) {
    type = TMC_CONFIGTYPE_ETF;
  } else {
    type = TMC_CONFIGTYPE_ETB;  // Fall back to ETB mode
  }

  return type;
}


DWORD _GetTPIUArchitecture (PDSC_DEBUG_BLOCK *block) {
  DWORD              arch;
  PDSC_DEBUG_ATTRIB* attr;

  attr = _GetDebugAttribute("ARCHITECTURE", block->attribH);
  if (attr == NULL) {
    return TPIU_TYPE_CM;  // Fall back to Cortex-M TPIU
  }

  if (!strcmpi(attr->value, "V7-M") || !strcmpi(attr->value, "V8-M")) {
    arch = TPIU_TYPE_CM;
  } else if (!strcmpi(attr->value, "CoreSight")) {
    arch = TPIU_TYPE_CS;
  } else if (!strcmpi(attr->value, "TPIU-Lite")) {
    // Relevant TPIU Lite diffs to CoreSight TPIU:
    // - No Formatter
    arch = TPIU_TYPE_LITE;
  } else {
    arch = TPIU_TYPE_CM;  // Fall back to Cortex-M TPIU
  }

  return arch;
}


U32 _ReadSdfDebugBlockInfo (PDSC_DEBUG_BLOCK *block, PDSC_TOPOLOGY_LINK *link) {
  U32 status = 0;

  switch (block->type) {
  case BLOCK_CPU:               // CPU Debug Block (SCS)
    if (NVIC_Addr == 0) {
      NVIC_Addr  = (U32)block->addr;
      DBG_Addr   = NVIC_Addr + DBG_OFS;
    }
    break;
  case BLOCK_FPB:               // Flash Patch and Breakpoint Unit
    FPB_Addr = (U32)block->addr;
    break;
  case BLOCK_DWT:               // Data Watch and Trace Unit
    DWT_Addr = (U32)block->addr;
    break;
  case BLOCK_ITM:               // Instrumentation Trace Macrocell
    ITM_Addr = (U32)block->addr;
    break;
  case BLOCK_ETM:               // Embedded Trace Macrocell
    ETM_Version = _GetETMVersion(block);
    ETM_Addr    = (U32)block->addr;
    break;
  case BLOCK_MTB:               // Micro Trace Buffer
#if 0
    MTB_Addr   = (U32)block->addr;
    MTB_Enable = TRUE;
    MTB_M0P    = TRUE;
#endif
    break;
  case BLOCK_TMC:               // Trace Memory Controller
    ETB_TMC           = TRUE; //_GetTCMType(block);
    ETB_Location.DP   = block->dp;
    ETB_Location.AP   = block->ap;
    ETB_Location.Addr = (U32)block->addr;
    break;
  case BLOCK_TPIU:              // Trace Port Interface Unit
    TPIU_Type = _GetTPIUArchitecture(block);
    if (TraceConf.Protocol == TPIU_TRACE_PORT ||
      ((TraceConf.Protocol == TPIU_SWO_MANCHESTER || TraceConf.Protocol == TPIU_SWO_UART) && TPIU_Type == TPIU_TYPE_CM) ) {
      TPIU_Location.DP   = block->dp;
      TPIU_Location.AP   = block->ap;
      TPIU_Location.Addr = (U32)block->addr;
      // TPIU_Sync determined by CPU revision.
    }
    break;
  case BLOCK_SWO:               // Serial Wire Output
    // SWO, reuse TPIU mechanisms (same programming interface)
    if (TraceConf.Protocol == TPIU_SWO_MANCHESTER || TraceConf.Protocol == TPIU_SWO_UART) {
      TPIU_Type          = TPIU_TYPE_SWO;
      TPIU_Location.DP   = block->dp;
      TPIU_Location.AP   = block->ap;
      TPIU_Location.Addr = (U32)block->addr;
    }
    break;
  case BLOCK_ETB:               // Embedded Trace Buffer
    ETB_Location.DP   = block->dp;
    ETB_Location.AP   = block->ap;
    ETB_Location.Addr = (U32)block->addr;
    break;
  case BLOCK_ATBFUN:            // ATB Trace Funnel
    CSTF_Single = FALSE;
    if (link && link->type == TOPLINK_TRACE) {
      CSTF_AddSlavePort((U32)block->addr, block->ap, block->dp, link->slaveif);
    } else {
      CSTF_AddInstance((U32)block->addr, block->ap, block->dp);
    }
    break;
  case BLOCK_CTI:               // Cross Trigger Interface
    CTI_AddInstance((U32)block->addr, block->ap, block->dp);
    break;


  // Not supported
  case BLOCK_ATBREP:            // (Programmable) ATB Trace Replicator
  case BLOCK_TSGEN:             // Timestamp Generator
  case BLOCK_GPR:               // Granular Power Requester
  case BLOCK_PMU:               // Performance Monitor Unit
  case BLOCK_PTM:               // Program Trace Macrocell
  case BLOCK_STM:               // System Trace Macrocell
  case BLOCK_HTM:               // AHB Trace Macrocell
  case BLOCK_ELA:               // Embedded Logic Analyzer
    break;
  }

  return (0);
}


U32 _ReadSdfTracePath(PDSC_DEBUG_BLOCK *start, PDSC_DEBUG_BLOCK *cur, PDSC_TOPOLOGY_LINK *slavelink) {
  PDSC_DEBUG_BLOCK  *block;
  PDSC_TOPOLOGY_LINK *link;
  U32 status;

  if (cur == NULL) {
    return (0);
  }

  // Check block
  if (cur->dp != start->dp || cur->ap != start->ap) {
    status = _ReadSdfDebugBlockInfo(cur, slavelink);
    if (status) return (status);
  }

  // Find linked blocks
  for (link = PDSCDebug_DebugProperties.topologyLinks; link != NULL; link = link->next) {
    if (cur->id == link->master) {
      if (link->type == TOPLINK_TRACE || (cur->type == BLOCK_CPU && link->type == TOPLINK_CPU)) {
        block = _GetDebugBlock(link->slave);
        if (block != NULL) {
          status = _ReadSdfTracePath(start, block, link);
          if (status) return status;

        }
      }
    }
  }

  return (0);
}


U32 _ReadSdfDebugBlocks(void) {
  U32 status;
  U32 dp = MonConf.JtagCpuIndex;    // Currently selected DP
  U32 ap = MonConf.AP;
  PDSC_DEBUG_BLOCK *cpuBlock = NULL;

  // First pass, look at blocks for selected DP/AP combination
  PDSC_DEBUG_BLOCK *block = PDSCDebug_DebugProperties.debugBlocks;
  for (; block != NULL; block = block->next) {
    // Add all CTI instances and blocks behind selected DP/AP combination
    if (block->type == BLOCK_CTI || (dp == block->dp && ap == block->ap)) {

      if (block->type == BLOCK_CPU) {
        if ( /*block->addr == 0ULL  ||*/     // _ReadSdfDebugBlockInfo will set NVIC = 0x00     // CORESIGHT_BASE_ADDRESS not given
             PDSCDebug_DebugProperties.debugConfig.defaultDbgAddr == 0ULL  ||          // <debug address="..." ... > not given
             block->addr == PDSCDebug_DebugProperties.debugConfig.defaultDbgAddr) {  // Block matches selected debug block address
          // CPU selectd for debug
          cpuBlock = block;
          status = _ReadSdfDebugBlockInfo(block, NULL);
          if (status) return (status);
        }
      } else {
        status = _ReadSdfDebugBlockInfo(block, NULL);
        if (status) return (status);
      }
    }
  }
  if (DBG_Addr == 0) return EU55;  // No supported CPU behind selected debug access port

  // Detect further Trace Components from link paths
  status = _ReadSdfTracePath(cpuBlock, cpuBlock, NULL);
  if (status) return (status);

  // Init blocks
#if DBGCM_V8M
  status = WriteD32 (DBG_EMCR, TRCENA, BLOCK_SECTYPE_ANY);  // Enable (DWT, ITM, ETM, TPIU)
#else // DBGCM_V8M
  status = WriteD32 (DBG_EMCR, TRCENA);  // Enable (DWT, ITM, ETM, TPIU)
#endif // DBGCM_V8M
  if (status) return (status);

  return (0);
}


U32 _ReadSdfDescription(void) {
  U32 status;

  if (PDSCDebug_DebugProperties.sdf == 0) {
    // Nothing to do
    return (0);
  }

  status = _ReadSdfDebugBlocks();
  if (status) return status;

  return (0);
}

U32 PDSCDebug_DebugClockId(U32 freq) {
  // Translate frequency into supported discrete debug clock ID;
  // use ID for closest supported frequency <= freq
  if (freq >= 10000000UL) {        // 10 MHz
    return (0);
  } else if (freq >=  5000000UL) { //  5 MHz
    return (1);
  } else if (freq >=  2000000UL) { //  2 MHz
    return (2);
  } else if (freq >=  1000000UL) { //  1 MHz
    return (3);
  } else if (freq >=   500000UL) { // 500 kHz
    return (4);
  } else if (freq >=   200000UL) { // 200 kHz
    return (5);
  } else if (freq >=   100000UL) { // 100 kHz
    return (6);
  } else if (freq >=    50000UL) { //  50 kHz
    return (7);
  } else if (freq >=    20000UL) { //  20 kHz
    return (8);
  } else if (freq >=    10000UL) { //  10 kHz
    return (9);
  } else if (freq >=     5000UL) { //   5 kHz
    return (10);
  } else if (freq >=     3000UL) { //   3 kHz
    return (11);
  }
  return (0);   // Default to 10 MHz
}


U32 PDSCDebug_Init(void) {
  U32 status = 0;
  PDSC_DEBUG_PROPERTIES dbgProperties;

  PDSCDebug_ExecutingSeq = false;

  if (PDSCDebug_Initialized) {
    return PDSCDebug_InitResult;
  }

  PDSCDebug_DevsScanned      = false;
  PDSCDebug_DebugContext     = DBGCON_CONNECT;
  PDSCDebug_HasDbgPropBackup = false;

#if DBGCM_WITHOUT_STOP
  StopAfterConnect           = !(MonConf.Opt & CONN_NO_STOP);
#endif // DBGCM_WITHOUT_STOP

  _MemCleanup();

  // Init properties struct
  memset(&PDSCDebug_DebugProperties, 0, sizeof(PDSCDebug_DebugProperties));

  _LinkAccessVars();
  _ClearAccessVars();

  _ClearSequences();

  // Query PDSC Debug Properties
  memset(&dbgProperties, 0, sizeof(dbgProperties));
  if (pio->Notify(UV_PDSCDBG_GET_PROPERTIES, &dbgProperties) != 1) {
    status = (EU22);  // PDSC: Debug Description not available
    goto end;
  }

  // Store Debug Configuration Info

  // Make local copy of the properties to minimize chance of memory corruptions
  _StoreDebugProperties(&PDSCDebug_DebugProperties, &dbgProperties);

  if (!PDSCDebug_DebugProperties.enabled) {
    status = (EU36);   // PDSC: Debug Description disabled
    goto end;
  }

  // Defaults/fixed values from PSDC
  if (PDSCDebug_DebugProperties.debugConfig.swj) {
    MonConf.Opt |= USE_SWJ;
  } else {
    MonConf.Opt &= ~USE_SWJ;
  }

  MonConf.AP              = PDSCDebug_DebugProperties.debugConfig.defaultAP;
  MonConf.JtagCpuIndex    = PDSCDebug_GetInternalDeviceId(PDSCDebug_DebugProperties.debugConfig.defaultDP);
  if (MonConf.JtagCpuIndex == (U32)(-1)) {
    status = (EU25);      // PDSC: Unknown Debug Port ID.\nCannot switch to Debug Port.
  }

  if (PDSCDebug_DebugProperties.debugClock == 0) { // not set, use defaults
    MonConf.SWJ_Clock = PDSCDebug_DebugClockId((U32)PDSCDebug_DebugProperties.debugConfig.debugClock);
  } else {
    MonConf.SWJ_Clock = PDSCDebug_DebugClockId((U32)PDSCDebug_DebugProperties.debugClock);
  }


  if (PDSCDebug_DebugProperties.protocol == PROTOCOL_SWD) {
    MonConf.Opt |= PORT_SW;
  } else {
    MonConf.Opt &= ~PORT_SW;
  }

  // Set values to be passed as sequence context
  _SetAccessVars();


  status = PDSCDebug_SetDeviceList(&JTAG_devs, NJDEVS);
  if (status) goto end;

end:
  PDSCDebug_Initialized = true;
  PDSCDebug_InitResult  = status;
  PDSCDebug_Supported   = (status == 0) || (status == EU36); // Supported but possibly disabled

  return (status);
}

U32 PDSCDebug_Reinit(void) {
  U32 status = 0;

  if (!SetupMode) {
    // Below changed values cannot be altered in debug mode,
    //  no reason to re-initialize this (breaks the JTAG chain
    //  display). However, some settings must be adopted which
    //  don't need debugger reinitialization, e.g. reset type
    //  can change.

    // Update Access Variables for Sequence Execution
    _SetAccessVars();

    if (PDSCDebug_GetActiveDP() == (U32)(-1)) {
      // Not initialized yet, presumably started with debug description disabled
      PDSCDebug_SetActiveDP(nCPU);
    }

    return (0);
  }

  PDSCDebug_DevsScanned  = false;

  // Check if currently selected protocol is supported
  PROTOCOL_TYPE prot = (MonConf.Opt & PORT_SW) ? PROTOCOL_SWD : PROTOCOL_JTAG;
  if (!PDSCDebug_ProtocolSupported(prot)) {
    prot = PDSCDebug_GetDefaultProtocol();
    switch(prot) {
    case PROTOCOL_JTAG:
      MonConf.Opt &= (~PORT_SW);
      break;
    case PROTOCOL_SWD:
      MonConf.Opt |= PORT_SW;
      break;
    default:
      return (EU20);    // PDSC: Debug Description not available
    }
  }
  // Set Protocol, do this before determining the internal device ID
  PDSCDebug_DebugProperties.protocol = prot;

  // Override default port settings
  MonConf.AP              = PDSCDebug_DebugProperties.debugConfig.defaultAP;
  MonConf.JtagCpuIndex    = PDSCDebug_GetInternalDeviceId(PDSCDebug_DebugProperties.debugConfig.defaultDP);
  if (MonConf.JtagCpuIndex == (U32)(-1)) {
    return (EU25);      // PDSC: Unknown Debug Port ID.\nCannot switch to Debug Port.
  }

  if (MonConf.SWJ_Clock < (sizeof(PDSCDebug_ClockValues)/sizeof(PDSCDebug_ClockValues[0]))) {
    PDSCDebug_DebugProperties.debugClock = PDSCDebug_ClockValues[MonConf.SWJ_Clock];
  }

  status = PDSCDebug_SetDeviceList(&JTAG_devs, NJDEVS);
  if (status) return (status);

  // Update Access Variables for Sequence Execution
  _SetAccessVars();

  return (status);
}


U32 PDSCDebug_UnInit(void) {
  if (!PDSCDebug_Initialized) {
    // nothing to do
    return (0);
  }

  PDSCDebug_SendDebugProperties();

  _MemCleanup();

  PDSCDebug_Initialized = false;
  return (0);
}

bool PDSCDebug_IsInitialized(void) {
  return PDSCDebug_Initialized;
}

__inline bool PDSCDebug_IsSupported(void) {
  return PDSCDebug_Supported;
}

__inline bool PDSCDebug_IsExecutingSeq(void) {
  return PDSCDebug_ExecutingSeq;
}

bool PDSCDebug_DevicesScanned(void) {
  return PDSCDebug_DevsScanned;
}

void PDSCDebug_Enable(bool enable) {
  PDSCDebug_DebugProperties.enabled = enable ? 1 : 0;
}

__inline bool  PDSCDebug_IsEnabled(void) {
  return (PDSCDebug_Supported && (PDSCDebug_DebugProperties.enabled == 1));
}

// Enable PDSC Sequence/Command Log
void PDSCDebug_LogEnable(bool enable) {
  PDSCDebug_DebugProperties.log = enable ? 1 : 0;
}

// PDSC Sequence/Command Log Enabled
__inline bool  PDSCDebug_IsLogEnabled(void) {
  return (PDSCDebug_Supported && (PDSCDebug_DebugProperties.log == 1));
}

// PDSC Debug Properties
PROTOCOL_TYPE PDSCDebug_GetDefaultProtocol() {
  return (PROTOCOL_TYPE)PDSCDebug_DebugProperties.debugConfig.defaultProtocol;
}

// Does default DP support the protocol?
bool PDSCDebug_ProtocolSupported(PROTOCOL_TYPE prot) {
  if (prot >= PROTOCOL_COUNT) {
    return false;
  }

  PDSC_DEBUG_PORT* port = PDSCDebug_GetDebugPort(PDSCDebug_DebugProperties.debugConfig.defaultDP);
  if (port == NULL) {
    // unknown debug port
    return false;
  }

  switch(prot) {
  case PROTOCOL_JTAG:
    return (port->jtag.implemented == 1);
  case PROTOCOL_SWD:
    return (port->swd.implemented  == 1);
  }

  return false;
}

// Is protocol switchable (SWJ-DP)?
bool PDSCDebug_ProtocolSwitchable(void) {
  return PDSCDebug_DebugProperties.debugConfig.swj ? true: false;
}

// Set ID if active DP (id - internal ID as used in JDEVS)
U32 PDSCDebug_SetActiveDP(U32 id) {
  PDSC_DEBUG_PORT* port = PDSCDebug_DebugProperties.ports;
  bool          foundDP = false;
  U32            status = 0;

  if (PDSCDebug_GetActiveDP() == id) {
    // Active DP remains the same
    return (0);
  }

  while (port != NULL && !foundDP) {
    //if (port->portid != 0 /*not auto*/) {
    switch (PDSCDebug_DebugProperties.protocol) {
    case PROTOCOL_JTAG:
      if (port->jtag.implemented && port->jtag.tapindex == id) {
        PDSCDebug_activeDebugDP = port->portid;
        foundDP = true;
      }
      break;
    case PROTOCOL_SWD:
      if (port->swd.implemented) {
        // TODO: Multi-drop support for SW-DP v2
        PDSCDebug_activeDebugDP = port->portid;
        foundDP = true;
      }
      break;
    default:
      warntxtout(EU20); // Internal DLL Error: Unsupported Debug Protocol
      break;
    }
    //}
    port = port->next;
  }
  if (!foundDP) {
    if (PDSCDebug_DebugProperties.ports == NULL) {
      return (EU25);        // PDSC: Unknown Debug Port ID.
    } else {
      PDSCDebug_activeDebugDP = PDSCDebug_DebugProperties.ports->portid;  // 'auto' DP
    }
  }

  return (status);
}


// Get ID if active DP (returns internal ID as used in JDEVS)
U32 PDSCDebug_GetActiveDP() {
  return PDSCDebug_GetInternalDeviceId(PDSCDebug_activeDebugDP);
}

// Get PDSC Default Reset Sequence ID
U32 PDSCDebug_GetDefaultResetSequenceID() {
  return PDSCDebug_DebugProperties.debugConfig.defaultRstSeqId;
}

U32 PDSCDebug_GetInternalDeviceId(U32 id) {
  PDSC_DEBUG_PORT* port = PDSCDebug_DebugProperties.ports;

  if (port == NULL && id > 0) {
    return (U32)(-1);
  }

  // Find PDSC port by ID
  while (port != NULL) {
    if (port->portid == id) {
      // Found the ID, check for protocol support
      if (PDSCDebug_DebugProperties.protocol == PROTOCOL_JTAG && port->jtag.implemented) {
        // Return the tapindex, this is the internal ID used in JDEVS
        return port->jtag.tapindex;
      } else if (PDSCDebug_DebugProperties.protocol == PROTOCOL_SWD && port->swd.implemented) {
        // TODO: multi-drop support
        return (0);
      } else {
        return (U32)(-1);
      }
    }

    port = port->next;
  }

  return (U32)(-1);  // Port not found
}


PDSC_DEBUG_PORT* PDSCDebug_GetDebugPort(U32 portId) {
  PDSC_DEBUG_PORT* port = PDSCDebug_DebugProperties.ports;
  for (; port != NULL; port = port->next) {
    if (port->portid == portId) {
      return port;
    }
  }
  return NULL;
}


// Get name for a sequence ID
const char* PDSCDebug_GetSequenceName(U32 id) {
  if (id >= 0 && id < SEQ_ID_COUNT) {
    return PDSCDebug_SequenceNames[id];
  } else {
    return PDSCDebug_SequenceNames[SEQ_ID_UNKNOWN];
  }
}

// Get description for a sequence ID
const char* PDSCDebug_GetSequenceDescription(U32 id) {
  if (id >= 0 && id < SEQ_ID_COUNT) {
    return PDSCDebug_SequenceDescriptions[id];
  }
  return (NULL);
}

// Sequence implemented and enabled (Exception: Even if not implemented return as enabled)
bool PDSCDebug_IsSequenceEnabled_Ex(U32 id) {
  if (id >= SEQ_ID_UNKNOWN) {
    return (false);      // PDSC: Unknown Sequence ID.\nCannot execute Sequence.
  }

  PDSC_SEQUENCE *sequence = PDSCDebug_Sequences[id];
  if (sequence == NULL) {
    return (true);       // PDSC: Sequence not implemented
  }

  if (sequence->disable != 0) {
    return (false);      // PDSC: Sequence disabled
  }

  return (true);
}

// Sequence implemented and enabled
bool PDSCDebug_IsSequenceEnabled(U32 id) {
  if (id >= SEQ_ID_UNKNOWN) {
    return (false);      // PDSC: Unknown Sequence ID.\nCannot execute Sequence.
  }

  PDSC_SEQUENCE *sequence = PDSCDebug_Sequences[id];
  if (sequence == NULL) {
    return (false);      // PDSC: Sequence not implemented
  }

  if (sequence->disable != 0) {
    return (false);      // PDSC: Sequence disabled
  }

  return (true);
}


// At least one sequence is implemented
bool PDSCDebug_HasSequences(void) {
  return PDSCDebug_SequencesAvailable;
}

// Get currently used DBGCONF file path or NULL if not existent
const char* PDSCDebug_GetDbgConfFilePath() {
  int len = 0;

  if (PDSCDebug_DebugProperties.dbgConfFile == NULL) {
    return (NULL);
  }

  if (PDSCDebug_DebugProperties.dbgConfFile->path == NULL) {
    return (NULL);
  }

  if (PDSCDebug_DebugProperties.dbgConfFile->path[0] == '\0') {
    return (NULL);
  }

  return (PDSCDebug_DebugProperties.dbgConfFile->path);
}

// Definitions from SDF available
bool PDSCDebug_HasSDF() {
  return (PDSCDebug_DebugProperties.sdf != 0);
}

// Get Pack ID from debug properties
const char* PDSCDebug_GetPackId() {
  int len = 0;

  if (PDSCDebug_DebugProperties.packid == NULL) {
    return (NULL);
  }

  if (PDSCDebug_DebugProperties.packid[0] == '\0') {
    return (NULL);
  }

  return (PDSCDebug_DebugProperties.packid);
}

// Get Pack ID from debug properties
const char* PDSCDebug_GetLogFile() {
  int len = 0;

  if (PDSCDebug_DebugProperties.logfile == NULL) {
    return (NULL);
  }

  if (PDSCDebug_DebugProperties.logfile[0] == '\0') {
    return (NULL);
  }

  return (PDSCDebug_DebugProperties.logfile);
}

/*
 * Patch Data as read from target
 * accType : Memory, AP, or DP access. See ACCESS_TYPE in PDSCDebugEngine.h
 * accSize : Access size, use ACCMX defines (ACCMX_U8 - 1, ACCMX_U16 - 2, ACCMX_U32 - 4)
 * addr    : Start address of the target access.
 * many    : Number of bytes read from the target.
 * data    : Read data to patch, this is the complete buffer as read during the target access.
 * attrib  : Attributes for memory access (Bit 0 - NoAddrIncr)
 * return value: error code
 */
__inline U32 PDSCDebug_PatchData(U32 accType, BYTE accSize, U32 addr, U32 many, UC8 *data, BYTE attrib) {
  PDSC_DATA_PATCH* patch = PDSCDebug_DebugProperties.dataPatches;
  U32 endAddr, patchAddr, patchEndAddr;
  U32 patchStartOfs, dataStartOfs, patchLen;
  UC8 *patchPtr, *maskPtr, *dataPtr;
  U32 i, j, activeAP, activeDP, patchDP;
  U32 iterations = (attrib & BLOCK_NADDRINC) ? (many/accSize) : 1;  // Iterations for applying single patch
  U32 itOfs      = (attrib & BLOCK_NADDRINC) ? accSize        : 0;  // Offset bytes per finished iteration
  bool validContext, patched;

  patched = false;

  if (many <= 0) {
    return (EU39);   // Parameter Error
  }

  if (attrib & BLOCK_NADDRINC) {
    endAddr  = addr + accSize - 1;
  } else {
    endAddr  = addr + many - 1;
  }
  activeAP = (AP_Sel & APSEL) >> 24;
  activeDP = PDSCDebug_GetInternalDeviceId(PDSCDebug_activeDebugDP);
  if (activeDP == (U32)(-1)) {
    return (EU25);   // PDSC: Unknown Debug Port ID.
  }



  while (patch != NULL) {

    if (patch->enabled) {
      // Get internal ID of DP the patch applies to
      patchDP = PDSCDebug_GetInternalDeviceId(patch->dp);
      if (patchDP == (U32)(-1)) {
        warntxtout(EU25);
        patch = patch->next;
        continue;
      }

      // Check "context"
      if (patch->type == accType) {
        switch ((ACCESS_TYPE)patch->type) {
        case ACCESS_MEM:
        case ACCESS_AP:
          validContext = (patch->ap == activeAP && patchDP == activeDP);
          break;
        case ACCESS_DP:
          validContext = (patchDP == activeDP);
          break;
        default:
          validContext = false;
          break;
        }
      } else {
        validContext = false;
      }

      if (validContext) {
        patchAddr    = (U32)(patch->addr & 0x00000000FFFFFFFFULL);
        patchEndAddr = patchAddr + 4 - 1;

        // Patch size currently limited to 4 bytes + mask
        if ( (patchAddr <= addr    && patchEndAddr >= addr)           // start address in patch range
          || (patchAddr <= endAddr && patchEndAddr >= endAddr)        // end address in patch range
          || (patchAddr >  addr    && patchEndAddr <  endAddr) ) {    // complete read range in patch range

          if (patchAddr >= addr) {
            patchStartOfs = 0;
            dataStartOfs  = patchAddr - addr;
          } else {
            patchStartOfs = addr - patchAddr;
            dataStartOfs  = 0;
          }
          if (patchEndAddr >= endAddr) {
            patchLen = endAddr - addr + 1 - dataStartOfs;
          } else {
            patchLen = patchEndAddr - patchAddr + 1 - patchStartOfs;
          }

          for (j = 0; j < iterations; j++) {                          // Repeat if block read w/o address increment
            patchPtr = (UC8*)(&patch->value) + patchStartOfs;
            maskPtr  = (UC8*)(&patch->mask)  + patchStartOfs;
            dataPtr  = data + j*itOfs + dataStartOfs;
            for (i = 0; i < patchLen; i++, patchPtr++, dataPtr++, maskPtr++) {
              *dataPtr &= ~(*maskPtr);
              *dataPtr |= (*patchPtr & *maskPtr);
              patched = true;
            }
          }
        }
      }
    }
    patch = patch->next;
  }
  return (patched ? 0 : EU38);  // success or no data patch available
}


// Initialize internal driver variables and function pointers (as possible so far)
U32 PDSCDebug_InitDriver(void) {
  U32  status = 0;

  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    DP_Type = SW_DP;
    DAPAbort       = SWD_DAPAbort;
    SwitchDP       = SWD_SwitchDP;
    ReadDP         = SWD_ReadDP;
    WriteDP        = SWD_WriteDP;
    ReadAP         = SWD_ReadAP;
    WriteAP        = SWD_WriteAP;
    TestSizesAP    = SWD_TestSizesAP;
    ReadD32        = SWD_ReadD32;
    WriteD32       = SWD_WriteD32;
    ReadD16        = SWD_ReadD16;
    WriteD16       = SWD_WriteD16;
    ReadD8         = SWD_ReadD8;
    WriteD8        = SWD_WriteD8;
    ReadBlockD8    = SWD_ReadBlockD8;
    ReadBlockD16   = SWD_ReadBlockD16;
    ReadBlockD32   = SWD_ReadBlockD32;
    ReadBlock      = SWD_ReadBlock;
    WriteBlockD8   = SWD_WriteBlockD8;
    WriteBlockD16  = SWD_WriteBlockD16;
    WriteBlockD32  = SWD_WriteBlockD32;
    WriteBlock     = SWD_WriteBlock;
    VerifyBlock    = SWD_VerifyBlock;
    ReadARMMemD8   = SWD_ReadARMMemD8;
    ReadARMMemD16  = SWD_ReadARMMemD16;
    ReadARMMemD32  = SWD_ReadARMMemD32;
    ReadARMMem     = SWD_ReadARMMem;
    WriteARMMemD8  = SWD_WriteARMMemD8;
    WriteARMMemD16 = SWD_WriteARMMemD16;
    WriteARMMemD32 = SWD_WriteARMMemD32;
    WriteARMMem    = SWD_WriteARMMem;
    VerifyARMMem   = SWD_VerifyARMMem;
    GetARMRegs     = SWD_GetARMRegs;
    SetARMRegs     = SWD_SetARMRegs;
    SysCallExec    = SWD_SysCallExec;
    SysCallRes     = SWD_SysCallRes;
    SWJ_Sequence   = SWD_SWJ_Sequence;
    SWJ_Clock      = SWD_SWJ_Clock;
    DAPAbortVal    = SWD_DAPAbortVal;
    break;
  case PROTOCOL_JTAG:
    DP_Type = JTAG_DP;
    DAPAbort       = JTAG_DAPAbort;
    SwitchDP       = JTAG_SwitchDP;
    ReadDP         = JTAG_ReadDP;
    WriteDP        = JTAG_WriteDP;
    ReadAP         = JTAG_ReadAP;
    WriteAP        = JTAG_WriteAP;
    TestSizesAP    = JTAG_TestSizesAP;
    ReadD32        = JTAG_ReadD32;
    WriteD32       = JTAG_WriteD32;
    ReadD16        = JTAG_ReadD16;
    WriteD16       = JTAG_WriteD16;
    ReadD8         = JTAG_ReadD8;
    WriteD8        = JTAG_WriteD8;
    ReadBlockD8    = JTAG_ReadBlockD8;
    ReadBlockD16   = JTAG_ReadBlockD16;
    ReadBlockD32   = JTAG_ReadBlockD32;
    ReadBlock      = JTAG_ReadBlock;
    WriteBlockD8   = JTAG_WriteBlockD8;
    WriteBlockD16  = JTAG_WriteBlockD16;
    WriteBlockD32  = JTAG_WriteBlockD32;
    WriteBlock     = JTAG_WriteBlock;
    VerifyBlock    = JTAG_VerifyBlock;
    ReadARMMemD8   = JTAG_ReadARMMemD8;
    ReadARMMemD16  = JTAG_ReadARMMemD16;
    ReadARMMemD32  = JTAG_ReadARMMemD32;
    ReadARMMem     = JTAG_ReadARMMem;
    WriteARMMemD8  = JTAG_WriteARMMemD8;
    WriteARMMemD16 = JTAG_WriteARMMemD16;
    WriteARMMemD32 = JTAG_WriteARMMemD32;
    WriteARMMem    = JTAG_WriteARMMem;
    VerifyARMMem   = JTAG_VerifyARMMem;
    GetARMRegs     = JTAG_GetARMRegs;
    SetARMRegs     = JTAG_SetARMRegs;
    SysCallExec    = JTAG_SysCallExec;
    SysCallRes     = JTAG_SysCallRes;
    SWJ_Sequence   = JTAG_SWJ_Sequence;
    SWJ_Clock      = JTAG_SWJ_Clock;
    DAPAbortVal    = JTAG_DAPAbortVal;
    break;
  default:
    status = (EU20);                       // Internal DLL Error: Unsupported Debug Protocol
    break;
  }

  AP_Sel = MonConf.AP << 24;

  // Initialize breakpoint and register variables
  NTrWP = 0;
  MTrWP = 0;
  MDtWP = 0;

  // 27.01.2020: Tracepoint support
  NTrRP = 0;
  NTrSP = 0;
  NTrHP = 0;
  NTrDP = 0;
  MTrRP = 0;
  MTrSP = 0;
  MTrHP = 0;
  MTrDP = 0;

  CntBP = 0;
  ClearUsedBreakResources();                      // Clear used breakpoint resources in resource manager
  CntWP = 0;

  RegARM.nCycles = 0;

  memset(&RegFPB, 0, sizeof(RegFPB));
  memset(&RegDWT, 0, sizeof(RegDWT));
  memset(&UseWP,  0, sizeof(UseWP));

  xFPU = FALSE;

  return (status);
}


static U32 _UnInitDriver(void) {
  U32 status = 0;

  FreeCache();           // free allocated cache memory.

  return (status);
}


// Initialize debug adapter
U32 PDSCDebug_InitDebugger(void) {

  OutMsg("");

  //---TODO:
  // Init Debug Unit and configure according MonConf (Debug Port & Clock ...)
  DEVELOP_MSG("Todo: \nInit Debug Unit and configure according MonConf (Debug Port & Clock ...)");

  return (0);
}


U32 PDSCDebug_UnInitDebugger(void) {

  //---TODO:
  // Uninit Debug Unit
  DEVELOP_MSG("Todo: \nUninit Debug Unit");

  return (0);
}


static U32 _EvalCPUID (void) {
  U32   status = 0;
  DWORD v0, v1, val;
  AP_CONTEXT *apCtx;

  status = AP_CurrentCtx(&apCtx);
  if (status) goto end;

#if DBGCM_V8M
  status = ReadD32(NVIC_CPUID, &val, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#else // DBGCM_V8M
  status = ReadD32(NVIC_CPUID, &val);
  if (status) goto end;
#endif // DBGCM_V8M

  switch (val & (CPUID_IMPL|CPUID_PARTNO)) {
    case (CPUID_IMPL_ARM|0xC330):
      xxCPU  = ARM_SC300;
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      break;
    case (CPUID_IMPL_ARM|0xC300):
      xxCPU  = ARM_SC000;
      // AP_PT  = FALSE;  // Tested with transfer sizes
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|0xC270):
      xxCPU  = ARM_CM7;

#if DBGCM_V8M
      status = ReadD32(MVFR0, &v0, BLOCK_SECTYPE_ANY);
      if (status) goto end;
#else // DBGCM_V8M
      status = ReadD32(MVFR0, &v0);
      if (status) goto end;
#endif // DBGCM_V8M

      // Change this to only check the Single and Double Precision Support fields
      // of MVFR0. Values changed between Cortex-M7 FPGA revisions, so this should
      // more reliable.
      if (v0 & (MVFR0_SPF|MVFR0_DPF)) {
        xFPU = TRUE;
      }

      // Extend CSW value to used cacheable memory accesses
      CSW_Val |= CSW_HPROT_CACHE;
      CSW_Val_Base |= CSW_HPROT_CACHE;  // 07.02.2019: Was missing before
      apCtx->CSW_Val_Base |= CSW_HPROT_CACHE;

      // DAP for current Cortex-M7 FPGA based on M0+-DAP, auto-increment boundary is 1K opposed to M4's 4K.
      // This may change if silicon vendors decide to use a full DAP (the FPGA uses a MinDP).
      RWPage = 0x0400;

      switch (val & (CPUID_VARIANT|CPUID_REVISION)) {
      case 0x00000000:  // r0p0
      case 0x00000001:  // r0p1
        // MASKINTS on stepping not working properly, also mask interrupts when stopping
        DHCSR_MaskIntsStop = C_MASKINTS;
        break;
      }

      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      DWT_ITM_F_R_W = TRUE;             // Separate DWT ITM Functions for Data R/W
      break;
    case (CPUID_IMPL_ARM|0xC240):
      xxCPU  = ARM_CM4;

#if DBGCM_V8M
      status = ReadD32(MVFR0, &v0, BLOCK_SECTYPE_ANY);
      if (status) goto end;
      status = ReadD32(MVFR1, &v1, BLOCK_SECTYPE_ANY);
      if (status) goto end;
#else // DBGCM_V8M
      status = ReadD32(MVFR0, &v0);
      if (status) goto end;
      status = ReadD32(MVFR1, &v1);
      if (status) goto end;
#endif // DBGCM_V8M

      if ((v0 == MVFR0_CM4_VAL) && (v1 == MVFR1_CM4_VAL)) {
        xFPU = TRUE;
      }
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      DWT_ITM_F_R_W = TRUE;               // Separate DWT ITM Functions for Data R/W
      break;
    case (CPUID_IMPL_ARM|0xC230):
      xxCPU  = ARM_CM3;
      if ((val & (CPUID_VARIANT | CPUID_REVISION)) == 0x00000001) {
        // r0p1 (0x410FC231)
        // Enable TPIU Formatter (otherwise Trace Output doesn't work)
        TraceConf.Opt |= TPIU_FORMAT;
      } else {
        // r0p0 (0x410FC230), r1p1 (0x411FC231), r2p0 (0x412FC230), r2p1 (0x412FC231)
        if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
          TraceConf.Opt |=  TPIU_FORMAT;  // Enable TPIU Formatter
        } else {
          TraceConf.Opt &= ~TPIU_FORMAT;  // Bypass TPIU Formatter
        }
      }
      if ((val & CPUID_VARIANT) >= (2 << 20)) {
        // r2p0 or higher
        DWT_ITM_F_R_W = TRUE;             // Separate DWT ITM Functions for Data R/W
      }
      break;
    case (CPUID_IMPL_ARM|0xC210):
      xxCPU  = ARM_CM1;
      // AP_PT  = FALSE;  // Tested with transfer sizes
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|0xC200):
      xxCPU  = ARM_CM0;
      // AP_PT  = FALSE;  // Tested with transfer sizes
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|0xC600):  // Cortex-M0+
      xxCPU  = ARM_CM0P;
      // AP_PT  = FALSE;  // Tested with transfer sizes
      AM_WP  = 0x1F;
      RWPage = 0x0400;
      TraceCycDwt = FALSE;
      break;
#if DBGCM_V8M
    case (CPUID_IMPL_ARM|CPUID_PN_ARM_CM23):    // Cortex-M23 (v8-M Baseline)
      xxCPU     = ARM_CM23;
      xMainline = FALSE;

      // 1 KByte Auto-Increment Page Size
      RWPage = 0x400;
      TraceConf.Opt &= TRACE_BASELINE_SUPP;  // Remove unsupported trace features from options
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }

      // No DWT with CYCCNT to sync/adjust
      TraceCycDwt = FALSE;
      break;
    case (CPUID_IMPL_ARM|CPUID_PN_ARM_CM33):    // Cortex-M33 (v8-M Mainline)
      xxCPU     = ARM_CM33;
      xMainline = TRUE;

      // 1 KByte Auto-Increment Page Size
      RWPage = 0x400;
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      break;
    case (CPUID_IMPL_ARM|CPUID_PN_ARM_CM35P): // ARM Cortex-M35P (v8-M Mainline with Physical Security)
      xxCPU     = ARM_CM35P;
      xMainline = TRUE;

      if (Cache_Addr != 0) {
        // Extend CSW value to use cacheable memory accesses (Cortex-M35P has optional I-cache, cachelines invalidated by debugger access with this bit set)
        CSW_Val |= CSW_HPROT_CACHE;
        CSW_Val_Base |= CSW_HPROT_CACHE;
        apCtx->CSW_Val_Base |= CSW_HPROT_CACHE;
      }

      // 1 KByte Auto-Increment Page Size
      RWPage = 0x400;
      if (((TraceConf.Opt & ETM_TRACE) && (TraceConf.Protocol == TPIU_ETB)) || (TraceConf.Protocol == TPIU_TRACE_PORT)) {
        TraceConf.Opt |=  TPIU_FORMAT;    // Enable TPIU Formatter
      } else {
        TraceConf.Opt &= ~TPIU_FORMAT;    // Bypass TPIU Formatter
      }
      break;
#endif // DBGCM_V8M
    default:
      status = (EU46); goto end;
  }

end:

  return (status);
}


static U32 _EvalDPID (void) {
  U32   status = 0;
  DWORD val;

  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    if ( (SWD_IDCode & 1) &&
        ((SWD_IDCode & DPID_DESIGN_M) == (DPID_DESIGNER << DPID_DESIGN_P))) {
      DP_Ver = (BYTE)((SWD_IDCode & DPID_VER_M) >> DPID_VER_P);
      DP_Min =        (SWD_IDCode & DPID_MIN) ? TRUE : FALSE;
    }
    break;
  case PROTOCOL_JTAG:
    switch (JTAG_devs.ic[JTAG_devs.com_no].id) {
      case 0x0BA00477:  // ARM Cortex-M3
      case 0x0BA80477:  // ARM Cortex-M1
      case 0x5BA00477:  // ARM Cortex-M7 (CS DAP)
        break;
      case 0x0BA01477:  // ARM Cortex-M0
      case 0x0BA02477:  // ARM Cortex-M7
      case 0x0BA04477:  // ARM Cortex-M33
      case 0x0BA05477:  // ARM Cortex-M23
        // DPIDR exists (JTAG DP V1 or higher)
        status = JTAG_ReadDP(DP_IDCODE, &val);
        if (status) goto end;
        if ((val & 1) && ((val & DPID_DESIGN_M) == (DPID_DESIGNER << DPID_DESIGN_P))) {
          DP_Ver = (BYTE)((val & DPID_VER_M) >> DPID_VER_P);
          DP_Min =        (val & DPID_MIN) ? TRUE : FALSE;
        }
        break;
    }
    break;
  default:
    status = (EU20);  // Internal DLL Error: Unsupported Debug Protocol
    break;
  }

end:

  return (status);
}


static U32 _InitDebugComponents(void) {
  U32   status = 0;
  DWORD ref, val;
  int   n;

#if DBGCM_V8M
  // Read DEMCR to preserve settings from sequences
  status = ReadD32 (DBG_EMCR, &val, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#else // DBGCM_V8M
  // Read DEMCR to preserve settings from sequences
  status = ReadD32 (DBG_EMCR, &val);
  if (status) goto end;
#endif // DBGCM_V8M

  ref = val;
  val |= TRCENA;           // Enable Trace
  if (MonConf.Opt & RST_VECT_CATCH) {
    val |= VC_CORERESET;
  }                        // Else keep value possibly set in preceding sequence
  if (ref != val) {        // Only update DEMCR if necessary

#if DBGCM_V8M
    status = WriteD32 (DBG_EMCR, val, BLOCK_SECTYPE_ANY);
    if (status) goto end;
#else // DBGCM_V8M
    status = WriteD32 (DBG_EMCR, val);
    if (status) goto end;
#endif // DBGCM_V8M

  }

  // Detect number of HW Breakpoint & Watchpoints
  if ((NumBP = DetectBreakResources(&FPB_Ver)) == -1) {
    // Error, error message already shown in DetectBreakResources().
    status = (EU01); goto end;
  }
  if (FPB_Ver > 0) {  // FPB version 1 (Cortex-M7, v8-M)
    FPB_CompMask = FPB_V1_COMP_M;
  }
  if ((NumWP = DetectDwtResources()) == -1) {
    // Error, error message already shown in DetectDwtResources().
    status = (EU01); goto end;
  }

  // Init rest of break resources
  InitBreakResources();

  // Disable Breakpoints
  if (NumBP) {

#if DBGCM_V8M
    status = WriteD32 (FPB_CTRL, FPB_KEY, BLOCK_SECTYPE_ANY);
    if (status) goto end;
#else // DBGCM_V8M
    status = WriteD32 (FPB_CTRL, FPB_KEY);
    if (status) goto end;
#endif // DBGCM_V8M

  }

  // Disable Watchpoints
#if DBGCM_V8M
  for (n = 0; n < NumWP; n++) {
    status = WriteD32 (DWT_FUNC + (n << 4), 0, BLOCK_SECTYPE_ANY);
    if (status) goto end;
    if (IsV8M()) {
      // Get FUNCTION ID
      status = ReadD32 (DWT_FUNC + (n << 4), &RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
      if (status) goto end;
    }
  }
#else // DBGCM_V8M
  for (n = 0; n < NumWP; n++) {
    status = WriteD32 (DWT_FUNC + (n << 4), 0);
    if (status) goto end;
  }
#endif // DBGCM_V8M

  // Check NVIC Features
  switch (xxCPU) {
    case ARM_CM0:
    case ARM_CM0P:
    case ARM_CM1:
    case ARM_SC000:
      NumIRQ = 32;  // Fixed according to ARM (NVIC_ICT does not exist)
      break;
    case ARM_CM3:
    case ARM_CM4:
    case ARM_CM7:
    case ARM_SC300:
    case ARM_CM23:
    case ARM_CM33:
    case ARM_CM35P:

#if DBGCM_V8M
      // Number of External IRQ
      status = ReadD32 (NVIC_ICT, &val, BLOCK_SECTYPE_ANY);
      if (status) goto end;
#else // DBGCM_V8M
      // Number of External IRQ
      status = ReadD32 (NVIC_ICT, &val);
      if (status) goto end;
#endif // DBGCM_V8M

      NumIRQ = ((val & INTLINESNUM) + 1) << 5;
      if (NumIRQ > 240) NumIRQ = 240;
      break;
  }

  switch (xxCPU) {
    case ARM_CM3:
    case ARM_CM4:
    case ARM_CM7:
    case ARM_SC300:
    case ARM_CM33:
    case ARM_CM35P:
      // Enable CYCCNT and clear all counters
    RegDWT.CTRL = DWT_CYCCNTEN;

#if DBGCM_V8M
    if(IsV8M()) {
      DWORD dummyDwtCtrl = 0;
      status = ReadBlock(DWT_CTRL, (BYTE *)&dummyDwtCtrl, 4, BLOCK_SECTYPE_ANY);    // [TdB: 03.02.2017] (SDMDK-6636) preserve DWT_CTRL.CYCDISS Bit
      if (status) { OutErrorMessage (status); return (1); }
      dummyDwtCtrl &= DWT_CYCDISS;
      if(dummyDwtCtrl)
        RegDWT.CTRL |= DWT_CYCDISS;
      else
        RegDWT.CTRL &= ~DWT_CYCDISS;
    }
    status = WriteBlock(DWT_CTRL, (BYTE *)&RegDWT, 0x1C, BLOCK_SECTYPE_ANY);
    if (status) goto end;
#else // DBGCM_V8M
    status = WriteBlock(DWT_CTRL, (BYTE *)&RegDWT, 0x1C, 0);
    if (status) goto end;
#endif // DBGCM_V8M

    break;
  }

end:

  return (status);
}


static U32 _ResetRecovery(void) {
  U32   status = 0;
#if DBGCM_DS_MONITOR
  int   res    = 0;
#else // DBGCM_DS_MONITOR
  DWORD n, val;
#endif // DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
  res = DSM_WaitForState(DSM_STATE_RST_DONE, DSM_STATE_RST_DONE, TRUE, 300);
  if (res >= DSM_WAIT_ERR) { status = EU01; goto end; }

  if (DSMonitorState & DSM_STATE_RST_DONE) {
    if (DSMonitorState & DSM_STATE_CPU_HALTED) {
      if (MonConf.Opt & RST_VECT_CATCH) {  // Execute based on Debug Setup. Reset without "Stop After Reset" will have
                                           // DEMCR.VC_CORERESET set (and some special handlings won't).
        pCbFunc(AG_CB_EXECHOOK, "OnStopAfterReset");
      }
    }
  }
#if DBGCM_V8M
  else {
    // Could not detect a completed reset
    if (pio->bSecureExt && !pio->bSDbgEna) {
      // Possibly failed because of not having required access rights
      warntxtout(EU48);                    // Could not reset target with non-secure debugger
    }
  }
#endif // DBGCM_V8M

#else // DBGCM_DS_MONITOR
  // Reset Recovery (Reset Pulse can be extended by External Circuits)
  n = GetTickCount();
  do {

#if DBGCM_V8M
    status = ReadD32 (DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
    if (status) goto end;
    if ((val & S_RESET_ST) == 0) break;
#else // DBGCM_V8M
    status = ReadD32 (DBG_HCSR, &val);
    if (status) goto end;
    if ((val & S_RESET_ST) == 0) break;
#endif // DBGCM_V8M

  } while ((GetTickCount() - n) < 300);

  if (val & S_RESET_ST) { status = (EU24); goto end; }   // PDSC: Cannot recover from reset
#endif // DBGCM_DS_MONITOR

end:

  return (status);
}


static U32 _WaitForStop(BOOL dsm) {
  U32   status = 0;
  DWORD n, val;
#if DBGCM_DS_MONITOR
  int   res = 0;
#endif // DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
  if (dsm) {                                                 // Device State Monitor running yet?
    res = DSM_WaitForState(DSM_STATE_CPU_HALTED, DSM_STATE_CPU_HALTED, TRUE, 500);
    if (res >= DSM_WAIT_ERR) { status = EU01; goto end; }

    if ((DSMonitorState & DSM_STATE_CPU_HALTED) == 0) {
      status = (EU23); goto end;                               // PDSC: Cannot stop target
    }
  } else {
#endif // DBGCM_DS_MONITOR
    // Wait for target to stop
    n = GetTickCount();
    do {

#if DBGCM_V8M
      status = ReadD32 (DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
      if (status) goto end;
      if (val & S_HALT) break;
#else // DBGCM_V8M
      status = ReadD32 (DBG_HCSR, &val);
      if (status) goto end;
      if (val & S_HALT) break;
#endif // DBGCM_V8M

    } while ((GetTickCount() - n) < 500);

    if ((val & S_HALT) == 0) { status = (EU23); goto end; }    // PDSC: Cannot stop target
#if DBGCM_DS_MONITOR
  }
#endif // DBGCM_DS_MONITOR

end:

  return (status);
}


static U32 _InitTrace(void) {
  U32 status = 0;

  if (pio->FlashLoad) {
    status = 0; goto end;
  }

  if (TraceConf.Opt & TRACE_ENABLE) {
    status = Trace_Init();    // Driver internal stuff
                              //  ETM/ITM: driver internal stuff + unlock + power up
    if (status) goto end;

    if (TraceConf.Protocol == TPIU_ETB) {
      status = ETB_Init();   // Check if ETB selected and available + driver internal stuff
      if (status) goto end;

      // Leave this to the PDSC Debug Description (TraceStart)
      // setup the trace funnel if available
      status = CSTF_Setup();  // Also returns success if CSTF not available, topology detection + driver internal stuff
      if (status) goto end;

      status = ETB_Setup();   // Feature/debugger specific stuff
      if (status) goto end;

      status = Trace_Setup();
      if (status) goto end;

      status = ETB_Flush();                                 // Flush to ensure no leftovers on ATB
      if (status) { OutErrorMessage (status); return (1); }
    } else {
      if (TPIU_Type == TPIU_TYPE_CS || TPIU_Type == TPIU_TYPE_SWO) {
        status = CSTF_Setup();  // Also returns success if CSTF not available, topology detection + driver internal stuff
        if (status) goto end;
      }

      status = Trace_Setup();
      if (status) goto end;
    }


    status = PDSCDebug_TraceStart();
    if (status) goto end;
  }

  PDSCDebug_TraceStarted = true;

end:

  return (status);
}


static U32 _UnInitTrace(void) {
  U32 status = 0;

  if (pio->FlashLoad || ((TraceConf.Opt & TRACE_ENABLE) == 0)) {
    status = 0; goto end;
  }

  status = Trace_UnInit();
  if (status) OutError (status);

  if (PDSCDebug_TraceStarted) {
    status = PDSCDebug_TraceStop();
    if (status) OutError (status);
  }

end:

  return (status);
}





//*************************************************************************
// Internal Sequences
//*************************************************************************

/*
 * Initialize debug driver internals and debugger
 * Return 0 if Ok, else error code.
 */
U32 _InitInternals(void) {
  U32 status = 0;

  // Initialize driver internals
  status = PDSCDebug_InitDriver();
  if (status) goto end;

  // Initialize debug adapter
  status = PDSCDebug_InitDebugger();
  if (status) goto end;

  if (!pio->FlashLoad) {        // 25.01.2019: Don't check supported trace settings for flash download
    //---TODO: Check if trace settings are supported by debugger
    DEVELOP_MSG("Todo: \nCheck if trace settings are supported by debugger");
  }

end:

  return (status);
}


/*
 * Scan JTAG chain or read SWD ID CODE
 * Return 0 if Ok, else error code.
 */
U32 PDSCDebug_DebugGetDeviceList(JDEVS *DevList, unsigned int maxdevs, bool merge) {
  U32 status = 0;

  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    status = SWD_GetDeviceList(DevList, maxdevs, merge);
    break;
  case PROTOCOL_JTAG:
    status = JTAG_GetDeviceList(DevList, maxdevs, merge);
    break;
  default:
    status = (EU20);  // Internal DLL Error: Unsupported Debug Protocol
    break;
  }
  if (status) goto end;

  PDSCDebug_DevsScanned = true;


end:

  return (status);
}


/*
 * Get Device Names of connected ICs.
 * Return 0 if Ok, else error code.
 */
U32 PDSCDebug_DebugGetDeviceNames(JDEVS *DevList, unsigned int maxdevs, bool merge) {
  U32 status = 0;

  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    status = SWD_GetDeviceNames(DevList, maxdevs, merge);
    break;
  case PROTOCOL_JTAG:
    status = JTAG_GetDeviceNames(DevList, maxdevs, merge);
    break;
  default:
    status = (EU20);  // Internal DLL Error: Unsupported Debug Protocol
    break;
  }
  if (status) goto end;

end:

  return (status);
}


/*
 * Read ROM Table and store information
 * Return 0 if Ok, else error code.
 */
U32 PDSCDebug_DebugReadRomTable(void) {
  U32   status = 0;
  DWORD val;

  if (!(MonConf.Opt & INIT_RST_HOLD)) {      // 02.10.2019: Only for connections NOT under reset
    if (pCbFunc(AG_CB_CHECKDEVICE, NULL)) {  // Genuine device check
      status = EU52; goto end;               // Connection refused due to device mismatch!
    }
  }

  if (PDSCDebug_HasSDF()) {
    // Read system information from SDF instead of target ROM Table
    status = _ReadSdfDescription();
    goto end;
  }

  // Read Access Port ROM Table
  status = ReadAP (AP_ROM, &val);
  if (status) goto end;

  status = ROM_Table(val);
  if (status) goto end;


  if (DBG_Addr == 0) { status = (EU12); goto end; }  // Invalid ROM Table

end:

  return (status);
}


/*
 * Read target features (e.g. based on CPU ID)
 * Return 0 if Ok, else error code.
 */
U32 PDSCDebug_DebugReadTargetFeatures(void) {
  U32   status = 0;
  DWORD val;

#if 0  // Moved to PDSCDebug_DebugReadDAPFeatures() and AP_Switch() csall
  // DP version and MINDP feature only required of Debug Port Start
  status = _EvalDPID();
  if (status) goto end;

  // Select AP (this should NOT be necessary before powering up debug)
  status = WriteDP(DP_SELECT, AP_Sel);
  if (status) goto end;

  // JR, 13.01.2014: Added as part of PDSC changes
  status = TestSizesAP();
  if (status) goto end;

  // Now properly init CSW
  // Initial CSW value to be in sync with the driver variable
  CSW_Val = (CSW_Val_Base|CSW_SIZE32|CSW_SADDRINC);
  status = WriteAP(AP_CSW, CSW_Val);
  if (status) goto end;
#endif

  status = PDSCDebug_DebugReadRomTable();
  if (status) goto end;

  // Evaluate CPUID and do specific driver settings
  status = _EvalCPUID();
  if (status) goto end;

#if DBGCM_V8M
  // Check Availability of Security Extensions
  status = DetectSecurityExtensions();
  if (status) return (status);

  // Check FPU Support
  status = ReadD32(MVFR0, &val, BLOCK_SECTYPE_ANY);
  if (status) return (status);

  if (val & (MVFR0_SPF | MVFR0_DPF)) {  // FPU Support present?
    xFPU = TRUE;
  }
#else // DBGCM_V8M
  // Check FPU Support
  status = ReadD32(MVFR0, &val);
  if (status) return (status);

  if (val & (MVFR0_SPF | MVFR0_DPF)) {  // FPU Support present?
    xFPU = TRUE;
  }
#endif // DBGCM_V8M

  status = ReadDP (DP_CTRL_STAT, &val);
  if (status) goto end;

  if (val & STICKYERR) {
    switch (DP_Type) {
      case JTAG_DP:
        status = WriteDP(DP_CTRL_STAT, val);
        break;
      case SW_DP:
        status = WriteDP(DP_ABORT, STKCMPCLR|STKERRCLR|WDERRCLR|ORUNERRCLR);
        break;
      default:
        status = (EU20);   // Internal DLL Error: Unsupported Debug Protocol
        break;
    }
    if (status) goto end;
    status = (EU14); goto end;
  }

end:

  return (status);
}


//*************************************************************************
// Overridable Sequences
//*************************************************************************


//*************************************************************************
//      Debug Port Sequences
//*************************************************************************


/*
 * Configure the debug port protocol
 * Return 0 if Ok, else error code.
 */

U32 PDSCDebug_DebugPortSetup (void) {
  U32  status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_DebugPortSetup);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    status = SWJ_ConfigureProtocol(0); // Try the default switch sequence
    break;
  case PROTOCOL_JTAG:
    status = JSW_ConfigureProtocol(0); // Try the default switch sequence
    break;
  default:
    status = (EU20);  // Internal DLL Error: Unsupported Debug Protocol
    break;
  }
  if (status) goto end;


end:

  return (status);
}


U32 PDSCDebug_DebugPortStart(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_DebugPortStart);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality
  switch(PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    status = SWD_DebugPortStart();
    break;
  case PROTOCOL_JTAG:
    status = JTAG_DebugPortStart();
    break;
  default:
    status = (EU20);  // Internal DLL Error: Unsupported Debug Protocol
    break;
  }
  if (status) goto end;


end:

  return (status);
}


U32 PDSCDebug_DebugPortStop(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_DebugPortStop);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality
  status = WriteDP(DP_CTRL_STAT, 0x00000000);
  if (status) goto end;

end:

  return (status);
}


//*************************************************************************
//      Target Setup Sequences
//*************************************************************************

U32 PDSCDebug_DebugDeviceUnlock(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_DebugDeviceUnlock);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}

U32 PDSCDebug_DebugCoreStart(void) {
  U32 status = 0;
  DWORD value;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_DebugCoreStart);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

  // Read DHCSR first to preserve possibly made settings
#if DBGCM_V8M
  status = ReadD32 (DBG_HCSR, &value, BLOCK_SECTYPE_ANY);
  if (status) { OutErrorMessage (status); goto end; }
  value &= ~0xFFFF0000;
  value |= DBGKEY | C_DEBUGEN;
  // Enable Debug
  status = WriteD32 (DBG_HCSR, value, BLOCK_SECTYPE_ANY);
  if (status) { OutErrorMessage (status); goto end; }
#else // DBGCM_V8M
  status = ReadD32 (DBG_HCSR, &value);
  if (status) { OutErrorMessage (status); goto end; }
  value &= ~0xFFFF0000;
  value |= DBGKEY | C_DEBUGEN;
  // Enable Debug
  status = WriteD32 (DBG_HCSR, value);
  if (status) { OutErrorMessage (status); goto end; }
#endif // DBGCM_V8M

end:

  return (status);
}

U32 PDSCDebug_DebugCoreStop (void)  {
  U32  status = 0;

  // Execute PDSC implementation                                                // 23.03.2020: was missing
  status = _ExecuteSequence(SEQ_DebugCoreStop);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

#if DBGCM_V8M
  status = WriteD32 (DBG_HCSR, DBGKEY, BLOCK_SECTYPE_ANY);
  if (status) { OutError (status); goto end; }
  status = WriteD32 (DBG_EMCR, 0, BLOCK_SECTYPE_ANY);
  if (status) { OutError (status); goto end; }
#else // DBGCM_V8M
  status = WriteD32 (DBG_HCSR, DBGKEY);
  if (status) { OutError (status); goto end; }
  status = WriteD32 (DBG_EMCR, 0);
  if (status) { OutError (status); goto end; }
#endif // DBGCM_V8M

end:

  return (status);
}


//*************************************************************************
//      Reset Sequences
//*************************************************************************


U32 PDSCDebug_ResetSystem(void) {
  DWORD val;
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetSystem);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

#if DBGCM_V8M
  val    = SYSRESETREQ;
  if (pio->bSecureExt) {
    status = WriteD32 (NVIC_AIRCR, VECTKEY | val, pio->bSDbgEna ? BLOCK_SECTYPE_SECURE : BLOCK_SECTYPE_NSECURE);
  } else {
    status = WriteD32 (NVIC_AIRCR, VECTKEY | val, BLOCK_SECTYPE_ANY);
  }
  if ((status == EU14) || (status == EU08) || (status == EU05)) status = 0;
  if (status) goto end;
#else // DBGCM_V8M
  val    = SYSRESETREQ;
  status = WriteD32 (NVIC_AIRCR, VECTKEY | val);
  if ((status == EU14) || (status == EU08) || (status == EU05)) status = 0;
  if (status) goto end;
#endif // DBGCM_V8M

  status = _ResetRecovery();
  if (status) goto end;

end:

  return (status);
}


U32 PDSCDebug_ResetProcessor(void) {
  DWORD val;
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetProcessor);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality
#if DBGCM_V8M
  if (!IsV8M()) {
    val = VECTRESET;
    status = WriteD32 (NVIC_AIRCR, VECTKEY | val, BLOCK_SECTYPE_ANY);
    if (status) goto end;

    status = _ResetRecovery();
    if (status) goto end;
  }
#else // DBGCM_V8M
  val = VECTRESET;
  status = WriteD32 (NVIC_AIRCR, VECTKEY | val);
  if (status) goto end;

  status = _ResetRecovery();
  if (status) goto end;
#endif // DBGCM_V8M

end:

  return (status);
}


U32 PDSCDebug_ResetHardware(BYTE bPreReset) {
  U32 status = 0;

  // Execute PDSC implementation
  PDSCDebug_AccessVars[AV_ID_CONNECTION].value |= (bPreReset ? AV_CONN_PRECONN_RST : 0); // 06.06.2018: Set pre-connect reset bit
  status = _ExecuteSequence(SEQ_ResetHardware);
  PDSCDebug_AccessVars[AV_ID_CONNECTION].value &= ~AV_CONN_PRECONN_RST;                  // 06.06.2018: Clear pre-connect reset bit
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

  //---TODO:
  // HW Chip Reset
  DEVELOP_MSG("Todo: \nHW Chip Reset");

  if (!bPreReset) {               // Doesn't make sense to recover here. We haven't connected yet.
    status = _ResetRecovery();
    if (status) goto end;
  }

end:

  return (status);
}


U32 PDSCDebug_ResetCustomized(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetCustomized);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


U32 PDSCDebug_ResetHardwareAssert(void) {
  U32  status = 0;
  //BYTE io;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetHardwareAssert);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

  //---TODO:
  // Assert HW Chip Reset
  DEVELOP_MSG("Todo: \nAssert HW Chip Reset");


end:

  return (status);
}


U32 PDSCDebug_ResetHardwareDeassert(void) {
  BYTE io     = 0;
  U32  status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetHardwareDeassert);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

  //---TODO:
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

end:

  return (status);
}


U32 PDSCDebug_ResetCatchSet(void) {
  DWORD val;
  U32   status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetCatchSet);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

#if DBGCM_V8M
  // Enable Reset Vector Catch
  status = WriteD32(DBG_EMCR, VC_CORERESET, BLOCK_SECTYPE_ANY);
  if (status) goto end;
  status = ReadD32 (DBG_HCSR, &val, BLOCK_SECTYPE_ANY);     // Clear S_RESET_ST
  if (status) goto end;
#else // DBGCM_V8M
  // Enable Reset Vector Catch
  status = WriteD32(DBG_EMCR, VC_CORERESET);
  if (status) goto end;
  status = ReadD32 (DBG_HCSR, &val);     // Clear S_RESET_ST
  if (status) goto end;
#endif // DBGCM_V8M


end:

  return (status);
}


U32 PDSCDebug_ResetCatchClear(void) {
  DWORD val;
  U32   status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_ResetCatchClear);
  if (status != (EU26)) {  // PDSC: Sequence not implemented
    if (status == EU37) {  // PDSC: Sequence disabled
      status = 0;
    }
    goto end;
  }
  status = 0;    // Reinit for default implementation

  // Execute default functionality

#if DBGCM_V8M
  // Disable Reset Vector Catch
  status = ReadD32(DBG_EMCR, &val, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#if 0  // 05.02.2019: Align with CMSIS ResetCatchClear default sequence, RST_VECT_CATCH feature considered in PDSCDebug_ResetTarget() after call PDSCDebug_ResetCatchClear()
  if (MonConf.Opt & RST_VECT_CATCH) {
    val |= VC_CORERESET;
  } else {
    val &= (~VC_CORERESET);
  }
#endif
  val   &= (~VC_CORERESET);  // Clear reset vector catch, set in PDSCDebug_ResetTarget() if required
  status = WriteD32(DBG_EMCR, val, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#else // DBGCM_V8M
  // Disable Reset Vector Catch
  status = ReadD32(DBG_EMCR, &val);
  if (status) goto end;
#if 0  // 05.02.2019: Align with CMSIS ResetCatchClear default sequence, RST_VECT_CATCH feature considered in PDSCDebug_ResetTarget() after call PDSCDebug_ResetCatchClear()
  if (MonConf.Opt & RST_VECT_CATCH) {
    val |= VC_CORERESET;
  } else {
    val &= (~VC_CORERESET);
  }
#endif
  val   &= (~VC_CORERESET);  // Clear reset vector catch, set in PDSCDebug_ResetTarget() if required
  status = WriteD32(DBG_EMCR, val);
  if (status) goto end;
#endif // DBGCM_V8M

end:

  return (status);
}


//*************************************************************************
//      Recovery Sequences
//*************************************************************************

U32 PDSCDebug_RecoverySupportStart(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_RecoverySupportStart);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


U32 PDSCDebug_RecoverySupportStop(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_RecoverySupportStop);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


U32 PDSCDebug_RecoveryAcknowledge(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_RecoveryAcknowledge);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}

//*************************************************************************
//      Trace Sequences
//*************************************************************************

U32 PDSCDebug_TraceStart(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_TraceStart);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


U32 PDSCDebug_TraceStop(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_TraceStop);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}

//*************************************************************************
//      Other Sequences
//*************************************************************************

U32 PDSCDebug_DebugCodeMemRemap(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_DebugCodeMemRemap);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


//*************************************************************************
//      Flash Sequences
//*************************************************************************

U32 PDSCDebug_FlashEraseDone(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_FlashEraseDone);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


U32 PDSCDebug_FlashProgramDone(void) {
  U32 status = 0;

  // Execute PDSC implementation
  status = _ExecuteSequence(SEQ_FlashProgramDone);
  if (status == (EU26) || status == (EU37)) {  // PDSC: Sequence not implemented / PDSC: Sequence disabled
    status = 0;    // No real error, skips sequence
  }

  // No default implementation

  return (status);
}


//*************************************************************************
// Top Level Sequences
//*************************************************************************


/*
 * Initialize your target communication
 * Return 0 if Ok, else error code.
 */


U32 PDSCDebug_InitTarget () {
  U32   status    = 0;
  DWORD sharedDPs = 0;
  DWORD val;

  PDSCDebug_DevsScanned  = false;
  PDSCDebug_DebugContext = DBGCON_CONNECT;

  // Variable Initialization
  DHCSR_MaskIntsStop = 0;

  // Init Internal Driver States and Debugger
  status = _InitInternals();
  if (status) { OutErrorMessage(status); goto end; }

  // Set SWD macroset here, no fill bits to set
  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    nCPU = PDSCDebug_GetInternalDeviceId(PDSCDebug_DebugProperties.debugConfig.defaultDP);

    if (nCPU == -1) {
      AGDIMsgBox (hMfrm, &JTAGErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
      status = EU25;          // PDSC: Unknown Debug Port ID.
      goto end;
    }

    JTAG_devs.com_no = nCPU;

    // Store info about selected DP, no full DP switch (we don't want
    // to power up yet).
    status = PDSCDebug_SetActiveDP(JTAG_devs.com_no);
    if (status) return (status);

    break;
  case PROTOCOL_JTAG:
    break;
  default:
    status = (EU20);   // Internal DLL Error: Unsupported Debug Protocol
    break;
  }
  if (status) goto end;


  // Preconnect/Connect Reset
  if (StopAfterConnect) {
    if (MonConf.Opt & INIT_RST_PULSE) {
      status = PDSCDebug_ResetHardware(1 /*bPreReset*/);
      if (status) { OutErrorMessage (status); goto end; }
    }
    if (MonConf.Opt & INIT_RST_HOLD) {
      status = PDSCDebug_ResetHardwareAssert();
      if (status) { OutErrorMessage (status); goto end; }
    }
  }

  // Configure Debug Port (i.e. execute SWJ switch or do SWD line/jtag reset)
  status = PDSCDebug_DebugPortSetup();
  if (status) { OutErrorMessage (status); goto end; }

///////

  // Scan JTAG chain/SWD IDCODE
  if ((MonConf.Opt & JTAG_MANUAL) == 0) {  // don't update the device list when in manual mode
    status = PDSCDebug_DebugGetDeviceList(&JTAG_devs, NJDEVS, true);
    if (status) { OutErrorMessage (status); goto end; }
  }

  // Internal Stuff
  status = PDSCDebug_DebugGetDeviceNames (&JTAG_devs, NJDEVS, true);
  if (status) { OutErrorMessage(status); goto end; }

  // Now we have consistent data to start switching and powering up DPs

  // Set Macro Set
  switch (PDSCDebug_DebugProperties.protocol) {
  case PROTOCOL_SWD:
    // Already set above
    break;
  case PROTOCOL_JTAG:
    nCPU = PDSCDebug_GetInternalDeviceId(PDSCDebug_DebugProperties.debugConfig.defaultDP);

    if (nCPU == -1) {
      AGDIMsgBox (hMfrm, &JTAGErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
      status = EU25;          // PDSC: Unknown Debug Port ID.
      goto end;
    }

    JTAG_devs.com_no = nCPU;

    break;
  default:
    status = (EU20);   // Internal DLL Error: Unsupported Debug Protocol
    goto end;
  }

  // Perform DP switch, this will do the initial setup of the default DP and
  // the required macro settings; this will also do any outstanding JTAG
  // Reset (manual JTAG setup)
  status = SwitchDP(nCPU, true);
  if (status) { OutErrorMessage(status); goto end; }

  // Read DAP features (DP IDR and AP IDR)
  status = PDSCDebug_DebugReadDAPFeatures();
  if (status) { OutErrorMessage(status); goto end; }

  // Execute device unlocking, e.g. for Kinetis
  status = PDSCDebug_DebugDeviceUnlock();
  if (status) { OutErrorMessage(status); goto end; }

  PDSCDebug_DbgInit = true;

  status = PDSCDebug_DebugReadTargetFeatures();
  if (status) { OutErrorMessage(status); goto end; }

  status = PDSCDebug_DebugCoreStart();
  if (status) { OutErrorMessage (status); goto end; }

  status = PDSCDebug_RecoverySupportStart();
  if (status) { OutErrorMessage (status); goto end; }

  if (StopAfterConnect) {
    if (MonConf.Opt & INIT_RST_HOLD) {
      status = PDSCDebug_ResetCatchSet();
      if (status) { OutErrorMessage (status); goto end; }

      // Release Reset
      status = PDSCDebug_ResetHardwareDeassert();
      if (status) { OutErrorMessage (status); goto end; }

      // 02.10.2019: Device check if connection under reset (see 'if' further above). Not all devices are fully accessible under reset.
      if (pCbFunc(AG_CB_CHECKDEVICE, NULL)) {     // Genuine device check, called here after DAP is ready for accesses
        status = EU52;                            // Connection refused due to device mismatch!
        OutErrorMessage (status); goto end;
      }

    } else {
      if (MonConf.Opt & RST_VECT_CATCH) {

        // Enable Reset Vector Catch
#if DBGCM_V8M
        // Read-Modify-Write to avoid losing other settings
        status = ReadD32 (DBG_EMCR, &val, BLOCK_SECTYPE_ANY);
        if (status) { OutErrorMessage (status); goto end; }
        if (!(val & VC_CORERESET)) {
          status = WriteD32 (DBG_EMCR, val | VC_CORERESET, BLOCK_SECTYPE_ANY);
          if (status) { OutErrorMessage (status); goto end; }
        }
#else // DBGCM_V8M
        // Read-Modify-Write to avoid losing other settings
        status = ReadD32 (DBG_EMCR, &val);
        if (status) { OutErrorMessage (status); goto end; }
        if (!(val & VC_CORERESET)) {
          status = WriteD32 (DBG_EMCR, val | VC_CORERESET);
          if (status) { OutErrorMessage (status); goto end; }
        }
#endif // DBGCM_V8M

      }
    }

#if DBGCM_V8M
    // Enable Debug & Stop Target
    status = WriteD32 (DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);
    if (status) { OutErrorMessage (status); goto end; }
#else // DBGCM_V8M
    // Enable Debug & Stop Target
    status = WriteD32 (DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);
    if (status) { OutErrorMessage (status); goto end; }
#endif // DBGCM_V8M

    // Check if Target is stopped
    status = _WaitForStop(FALSE);
    if (status == (EU23)) {

#if DBGCM_V8M
      if (NonSecureCantStop()) {   // Connected to Secure target with Non-Secure Debugger, CPU still in Secure Mode
        txtout("Warning: %s\n", &NonSecureStopErr[0]);
        status = 0;
      } else {
        AGDIMsgBox (hMfrm, &StopErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
        goto end;
      }
#else // DBGCM_V8M
      AGDIMsgBox (hMfrm, &StopErr[0], &ErrTitle[0], MB_OK | MB_ICONHAND, IDOK);
      goto end;
#endif // DBGCM_V8M

    }
    if (status) {
      OutErrorMessage (status);
      goto end;
    }

#if DBGCM_V8M
    if (!NonSecureCantStop()) {
      // Ensure that MASKINTS is cleared while target is stopped (unless required to be set)
      status = WriteD32 (DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);
      if (status) { OutErrorMessage (status); goto end; }
    }
#else // DBGCM_V8M
    status = WriteD32 (DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);
    if (status) { OutErrorMessage (status); goto end; }
#endif // DBGCM_V8M

  }

  status = _InitDebugComponents();
  if (status) { OutErrorMessage (status); goto end; }


  Invalidate();                          // Registers, Memory cache

#if DBGCM_DS_MONITOR
  status = DSM_StartMonitor();
  if (status) { OutErrorMessage (status); goto end; }
#endif // DBGCM_DS_MONITOR

#if DBGCM_V8M
  if (!pio->FlashLoad && StopAfterConnect && !NonSecureCantStop()) {
    GetRegs((1ULL << nR15) | (1ULL << nPSR));// Read PC & xPSR
  }
#else // DBGCM_V8M
  if (!pio->FlashLoad && StopAfterConnect) {
    GetRegs((1ULL << nR15) | (1ULL << nPSR));// Read PC & xPSR
  }
#endif // DBGCM_V8M

  if (!pio->FlashLoad) {
    status = _InitTrace();
    if (status) { OutErrorMessage (status); goto end; }
  }


end:

  return (status ? 1 : 0);            // say 'Ok.'
}


/*
 * Target settings have changed (Baudrate or ComPort for example)
 * Return 0 if Ok, else error code.
 */

U32 PDSCDebug_ReInitTarget (void)  {
  U32   status = 0;
  union v   v;
  DWORD vp, vn;
  DWORD n, val;

  // 02.04.2019
  if (!ReInit || ReInitInProgress) {
    // Reinitialization already executed or still in progress,
    // nothing to do for us.
    return 0;
  }

//---TODO: shutdown target
//---      reinit communication with current 'MonConf' values
  ReInit = 0;
  ReInitInProgress = 1;      // 02.04.2019

  if (pCoreClk) {
    v.ul = TraceConf.Clk;
    pio->p_putval(pCoreClk, &v);
  }

  // 02.04.2019: Separate Trace Clock setting - TRACE_CLK VTREG
  if (pTraceClk) {
    v.ul = (TraceConf.Opt & TRACE_USE_CORECLK) ? TraceConf.Clk : TraceConf.TPIU_Clk;
    pio->p_putval(pTraceClk, &v);
  }

#if DBGCM_V8M
  // Update Reset Vector Catch Setting
  status = ReadD32(DBG_EMCR, &val, BLOCK_SECTYPE_ANY);
  // if (status) { OutError(status); return (status); }
  if (status) { OutError(status); ReInitInProgress = 0; return (status); }    // 02.04.2019
  if ((val & VC_CORERESET) != MONCONF_RST_VECT_CATCH) {
    val ^= VC_CORERESET;
    status = WriteD32(DBG_EMCR, val, BLOCK_SECTYPE_ANY);
    // if (status) { OutError(status); return (status); }
    if (status) { OutError(status); ReInitInProgress = 0; return (status); }  // 02.04.2019
  }
#else // DBGCM_V8M
  // Update Reset Vector Catch Setting
  status = ReadD32(DBG_EMCR, &val);
  // if (status) { OutError(status); return (status); }
  if (status) { OutError(status); ReInitInProgress = 0; return (status); }    // 02.04.2019
  if ((val & VC_CORERESET) != MONCONF_RST_VECT_CATCH) {
    val ^= VC_CORERESET;
    status = WriteD32(DBG_EMCR, val);
    // if (status) { OutError(status); return (status); }
    if (status) { OutError(status); ReInitInProgress = 0; return (status); }  // 02.04.2019
  }
#endif // DBGCM_V8M

  if (TraceConf.Opt & TRACE_ENABLE) {
    status = 0;  // init for next block
    if (TraceConf.Protocol == TPIU_ETB) {
      status = ETB_Setup();
      if (!status) {
        status = Trace_Setup();
      }
      if (!status) {
        status = ETB_Flush();                        // Flush to ensure no leftovers on ATB
      }
    } else {
      status = Trace_Setup();
    }
    if (status) {
      OutError (status);
      ReInitInProgress = 0;                                                   // 02.04.2019
      return (status);
    }
#if DBGCM_V8M
    if (IsV8M()) {
      if (TraceConf.Opt & TRACE_PC_DATA) {
        vp = DWTv8_DADDR_VAL_W | DWTv8_TRACE0;
        vn = DWTv8_DADDR_VAL_W | DWTv8_TRACE1;
      } else {
        vp = DWTv8_DADDR_VAL_W | DWTv8_TRACE1;
        vn = DWTv8_DADDR_VAL_W | DWTv8_TRACE0;
      }
      for (n = 0; n < NumWP; n++) {
        if ((RegDWT.CMP[n].FUNC &  DWTv8_FUNCTION_CFG) == vp) {
          RegDWT.CMP[n].FUNC = (RegDWT.CMP[n].FUNC & ~DWTv8_FUNCTION_CFG) | vn;
          status = WriteD32(DWT_FUNC + (n << 4), RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
          if (status) {
            OutError (status);
            ReInitInProgress = 0;                                             // 02.04.2019
            return (status);
          }
        }
      }
    } else {
      if (TraceConf.Opt & TRACE_PC_DATA) {
        vp = DWT_ITM_F_R_W ? DWT_ITM_DW    : DWT_ITM_DRW;
        vn = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
      } else {
        vp = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
        vn = DWT_ITM_F_R_W ? DWT_ITM_DW    : DWT_ITM_DRW;
      }
      for (n = 0; n < NumWP; n++) {
        if (RegDWT.CMP[n].FUNC == vp) {
          RegDWT.CMP[n].FUNC = vn;
          status = WriteD32(DWT_FUNC + (n << 4), RegDWT.CMP[n].FUNC, BLOCK_SECTYPE_ANY);
          if (status) {
            OutError (status);
            ReInitInProgress = 0;                                             // 02.04.2019
            return (status);
          }
        }
      }
    }
#else // DBGCM_V8M
    if (TraceConf.Opt & TRACE_PC_DATA) {
      vp = DWT_ITM_F_R_W ? DWT_ITM_DW    : DWT_ITM_DRW;
      vn = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
    } else {
      vp = DWT_ITM_F_R_W ? DWT_ITM_DW_PC : DWT_ITM_DRW_PC;
      vn = DWT_ITM_F_R_W ? DWT_ITM_DW    : DWT_ITM_DRW;
    }
    for (n = 0; n < NumWP; n++) {
      if (RegDWT.CMP[n].FUNC == vp) {
        RegDWT.CMP[n].FUNC = vn;
        status = WriteD32(DWT_FUNC + (n << 4), RegDWT.CMP[n].FUNC);
        if (status) {
          OutError (status);
          ReInitInProgress = 0;                                               // 02.04.2019
          return (status);
        }
      }
    }
#endif // DBGCM_V8M
  }

  ReInitInProgress = 0;                                                       // 02.04.2019

  return (status);            // say 'Ok.'
}


/*
 * Reset your target
 */

U32 PDSCDebug_ResetTarget (void)  {
  U32   status;
  DWORD value;
  SYM  *sym = NULL;

#if DBGCM_DS_MONITOR
  BYTE suppressed  = 0;
  BYTE traceActive = 0;
#endif // DBGCM_DS_MONITOR

  PDSCDebug_DebugContext = DBGCON_RESET;

  if (!iRun) {
    status = PDSCDebug_ResetCatchSet();
    if (status) return (status);
  }

#if DBGCM_DS_MONITOR
  traceActive = DSMonitorThread.traceActive;
  status = DSM_ConfigureMonitor(DSM_CFG_RESET_MASK,
                                DSM_CFG_BEFORE_RESET,
                                0, TRUE);
  if (status) goto end;
  suppressed = 1;

  DSM_ClearState(DSM_STATE_RST_DONE);      // Clear Reset Done Flag to catch unexecuted resets (e.g. SYSRESETREQS '1' for non-secure debugger)
#endif // DBGCM_DS_MONITOR

  switch (MonConf.Opt & RESET_TYPE) {
    case RESET_HW:
      status = PDSCDebug_ResetHardware(0 /*bPreReset*/);
      break;
    case RESET_SW_SYS:
      status = PDSCDebug_ResetSystem();
      break;
    case RESET_SW_VECT:
      status = PDSCDebug_ResetProcessor();
      break;
    default:
      switch (PDSCDebug_DebugProperties.debugConfig.defaultRstSeqId) {
      case SEQ_ResetHardware:
        status = PDSCDebug_ResetHardware(0 /*bPreReset*/);
        break;
      case SEQ_ResetSystem:
        status = PDSCDebug_ResetSystem();
        break;
      case SEQ_ResetProcessor:
        status = PDSCDebug_ResetProcessor();
        break;
      default:
        status = PDSCDebug_ResetCustomized();
        break;
      }
  }
  if (status) goto end;

#if DBGCM_DS_MONITOR
  // Reset Recovery done in above type specific reset functions, do not execute OnStopAfterReset()
  // hook here (as opposed to what happens in ResetTarget() in AGDI.CPP.
#endif // DBGCM_DS_MONITOR

  // 21.04.2016: Move Bootloader delay before DebugAccessEnsure(). Bootloader may prohibit
  // target accesses while running
  if (MonConf.Opt & BOOT_RUN) {
    Sleep(100);                            // Allow Bootloader to run
  }

#if DBGCM_RECOVERY
  // Detect possible connection loss and try to recover
  if (DebugAccessEnsure()) {
    status = (EU24); goto end;             // PDSC: Cannot recover from reset
  }
#endif // DBGCM_RECOVERY

  // Moving recovery up here. This needs to be waited for independently from any bootloader
  // to be run. Extra delay introduced should benegligible.
  // Reset Recovery (Reset Pulse can be extended by External Circuits)
  //status = _ResetRecovery();
  //if (status && status != (EU24)) return (status);

  if (iRun) goto end;

  // Stop Target if Bootloader still running (no valid user code)
  if (MonConf.Opt & BOOT_RUN) {
    if (status == (EU24)) {

#if DBGCM_V8M
      status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);  // Stop Target
      if (status) goto end;
#else // DBGCM_V8M
      status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);  // Stop Target
      if (status) goto end;
#endif // DBGCM_V8M

    }
  }

#if DBGCM_V8M
  // Check Status of DHCSR
  status = ReadD32(DBG_HCSR, &value, BLOCK_SECTYPE_ANY);
  if (status) goto end;
#else // DBGCM_V8M
  // Check Status of DHCSR
  status = ReadD32(DBG_HCSR, &value);
  if (status) goto end;
#endif // DBGCM_V8M

  // Stop Target if core locked up
  if (value & S_LOCKUP) {
    OutErrorMessage(EU45);                                                          // CPU locked up, stopping target

#if DBGCM_V8M
    status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop, BLOCK_SECTYPE_ANY);  // Stop Target
    if (status) goto end;
#else // DBGCM_V8M
    status = WriteD32(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT | DHCSR_MaskIntsStop);  // Stop Target
    if (status) goto end;
#endif // DBGCM_V8M

  }

  // Check if Target is stopped
  status = _WaitForStop(TRUE);

#if DBGCM_V8M
  if (status == (EU23)) {
    if (SevereTargetError(SEV_ERR_STOP)) status = 0;   // Set status to 0 if error degraded to warning
  }
  if (status) goto end;
#else // DBGCM_V8M
  if (status) {
    if (status == (EU23)) {                                                         // Cannot stop target
      SetPlayDead(&StopErr[0]);
    }
    goto end;
  }
#endif // DBGCM_V8M

  status = PDSCDebug_ResetCatchClear();
  if (status) goto end;

#if DBGCM_V8M
  // status = WriteD32 (DBG_EMCR, TRCENA|MONCONF_RST_VECT_CATCH, BLOCK_SECTYPE_ANY);    // Enable Trace
  // if (status) goto end;

  // 05.02.2019: Do a read-modify-write to preserve values
  status = ReadD32 (DBG_EMCR, &value, BLOCK_SECTYPE_ANY);
  if (status) goto end;
  value &= (~VC_CORERESET);                                                          // Clear Reset Vector Catch
  value |= (MONCONF_RST_VECT_CATCH | TRCENA);                                        // Set TRCENA and Reset Vector Catch as per setting
  status = WriteD32 (DBG_EMCR, value, BLOCK_SECTYPE_ANY);                            // Write back DEMCR setting
  if (status) goto end;
#else // DBGCM_V8M
  // status = WriteD32 (DBG_EMCR, TRCENA|MONCONF_RST_VECT_CATCH);    // Enable Trace
  // if (status) goto end;

  // 05.02.2019: Do a read-modify-write to preserve values
  status = ReadD32 (DBG_EMCR, &value);
  if (status) goto end;
  value &= (~VC_CORERESET);                                                          // Clear Reset Vector Catch
  value |= (MONCONF_RST_VECT_CATCH | TRCENA);                                        // Set TRCENA and Reset Vector Catch as per setting
  status = WriteD32 (DBG_EMCR, value);                                               // Write back DEMCR setting
  if (status) goto end;
#endif // DBGCM_V8M

  if (!pio->FlashLoad && (TraceConf.Opt & TRACE_ENABLE)) {
#if 0
    status = _InitTrace();
#else
    // MAY NEED TO DO A FULL _InitTrace(), but in that case we need to make sure that
    //   we reuse the existing buffers. Currently, this will cause a huge memory leak.
    status = PDSCDebug_TraceStart();
#endif
    if (status) goto end;
  }

  Invalidate();                            // Registers, Memory cache

  if (!pio->FlashLoad) {
    GetRegs((1ULL << nR15) | (1ULL << nPSR));// Read PC & xPSR
  }

end:
#if DBGCM_DS_MONITOR
  if (suppressed) {
    if (status) {
      DSM_ConfigureMonitor(DSM_CFG_RESET_MASK,
                           DSM_CFG_AFTER_RESET | (traceActive ? DSM_CFG_TRACE_ACTIVE : 0),
                           0, FALSE);
    } else {
      status = DSM_ConfigureMonitor(DSM_CFG_RESET_MASK,
                                    DSM_CFG_AFTER_RESET | (traceActive ? DSM_CFG_TRACE_ACTIVE : 0),
                                    0, FALSE);
    }
  }
#endif // DBGCM_DS_MONITOR
  if (status) { OutError (status); return (1); }

  return (status);                         // 0 = OK
}


/*
 * Stop your target communication
 * Free all resources (dynamic memory, ...)
 */

void PDSCDebug_StopTarget (void)  {
  U32   status;
  DWORD sharedDPs = 0;
  unsigned int i;

  OutMsg("");

  PDSCDebug_DebugContext = DBGCON_DISCONNECT;

#if DBGCM_WITHOUT_STOP
  status = UnInitRunningTarget();
  if (status) OutError (status);
#endif // DBGCM_WITHOUT_STOP

  status = _UnInitTrace();
  if (status) OutError (status);

  // 23.03.2020: Call ExitDebug here so that sequence support matches the CMSIS specification.
  if (pio && !pio->FlashLoad) {   // Only for normal debug connections. Flash Download calls earlier because of "Reset & Run".
    ExitDebug();
  }

#if DBGCM_DS_MONITOR
  status = DSM_StopMonitor();
  if (status) OutError(status);
#endif // DBGCM_DS_MONITOR

  // Disconnect from accessed debug port
  for (i = 0; i < JTAG_devs.cnt; i++) {
    if (JTAG_devs.icacc[i] == 0) {
      // Not accessed by this debugger instance
      continue;
    }

    // Select Debug Port for power-down
    status = SwitchDP(i, false);
    if (status) {
      OutError (status);
      continue;
    }

    status = PDSCDebug_DebugPortStop();
    if (status) OutError(status);
    JTAG_devs.icacc[i] = 0;
  }
  // Restore device index for standard accesses
  JTAG_devs.com_no = nCPU;   // Do NOT call SwitchDP(), this will power it up again

  status = PDSCDebug_UnInitDebugger();
  if (status) OutError (status);

  status = _UnInitDriver();
  if (status) OutError (status);
}


/*
 * Read DAP features (e.g. based on DP IDR and AP IDR)
 * Return 0 if Ok, else error code.
 */
U32 PDSCDebug_DebugReadDAPFeatures(void) {
  U32 status = 0;
  AP_CONTEXT *apCtx;

  // DP version and MINDP feature only required of Debug Port Start
  status = _EvalDPID();
  if (status) goto end;

  // Select AP (this should NOT be necessary before powering up debug)
  status = WriteDP(DP_SELECT, AP_Sel);
  if (status) goto end;

  // Now properly init CSW
  // Initial CSW value to be in sync with the driver variable
  status = AP_Switch(&apCtx);
  if (status) goto end;

  status = WriteAP(AP_CSW, (apCtx->CSW_Val_Base|CSW_SIZE32|CSW_SADDRINC));
  if (status) goto end;

end:

  return (status);
}


void InitPDSCDebug() {
  _memHead = NULL;     // head of clusters
  _memTail = NULL;     // tail of clusters
  PDSCDebug_DbgInit          = false;
  PDSCDebug_Initialized      = false;
  PDSCDebug_InitResult       = 0;
  PDSCDebug_Supported        = false;
  PDSCDebug_ExecutingSeq     = false;
  PDSCDebug_DevsScanned      = false;
  PDSCDebug_DebugContext     = DBGCON_CONNECT; // Current Debug Context, used for Sequence Execution
  PDSCDebug_HasDbgPropBackup = false;
  PDSCDebug_SetupChange      = false;
  PDSCDebug_SequencesAvailable = false;
  PDSCDebug_TraceStarted     = false;

  StopAfterConnect = true;

  memset(&PDSCDebug_DebugProperties, 0, sizeof(PDSCDebug_DebugProperties));
  memset(&PDSCDebug_DebugPropertiesBackup, 0, sizeof(PDSCDebug_DebugPropertiesBackup));        // Backup of current property settings, e.g. when opening setup dialog
  PDSCDebug_activeDebugDP   = (U32)(-1);  // Init to "-1", _SetActiveDP() will do the rest
  memset(PDSCDebug_Sequences, NULL, sizeof(PDSCDebug_Sequences));      // Sequences callable from driver, entry == NULL if not provided
  _DbgConfFilePathLen = 0;       // Current length of PDSCDebug_DebugProperties.dbgConfFile->path buffer
  _PackIdLen          = 0;       // Current length of PDSCDebug_DebugProperties.packid buffer
  _LogFileLen         = 0;       // Current length of PDSCDebug_DebugProperties.logfile buffer
}
#endif // DBGCM_DBG_DESCRIPTION