/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.3
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016, 2019-2020 ARM Limited. All rights reserved.
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

#ifndef __PDSCDEBUG_H__
#define __PDSCDEBUG_H__

#include "Collect.h"

#if DBGCM_DBG_DESCRIPTION

#include "..\AGDI.H"
#include "..\PDSCDebugEngine.h"
#include "JTAG.h"


// Initialization/Uninitialization
extern DEBUG_CONTEXT  PDSCDebug_DebugContext;                // Current Debug Context, used for Sequence Execution

extern          U32   PDSCDebug_DebugClockId(U32 freq);      // Calculate Clock ID from frequency
extern          U32   PDSCDebug_Init(void);                  // Initialize PDSC configuration
extern          U32   PDSCDebug_Reinit(void);                // Initialize PDSC configuration
extern          U32   PDSCDebug_UnInit(void);                // UnInitialize PDSC configuration
extern          bool  PDSCDebug_IsInitialized(void);         // PDSC Debug is initialized
extern __inline bool  PDSCDebug_IsSupported(void);           // PDSC Debug Description provided for target
extern          void  PDSCDebug_Enable(bool enable);         // Generally enable PDSC Debug Description
extern __inline bool  PDSCDebug_IsEnabled(void);             // PDSC Debug Description generally enabled
extern          void  PDSCDebug_LogEnable(bool enable);      // Enable PDSC Sequence/Command Log
extern __inline bool  PDSCDebug_IsLogEnabled(void);          // PDSC Sequence/Command Log Enabled
extern __inline bool  PDSCDebug_IsExecutingSeq(void);        // PDSC Debug Sequence is executing
extern          bool  PDSCDebug_DevicesScanned(void);        // Devices have been scanned

extern          void  PDSCDebug_SendDebugProperties();          // PDSC Config Updated, Write New Settings to UV
extern          U32   PDSCDebug_CreateDebugPropertiesBackup();
extern          U32   PDSCDebug_ConfirmDebugPropertiesChange();
extern          U32   PDSCDebug_DiscardDebugPropertiesChange();
extern          bool  PDSCDebug_HasDebugPropertiesBackup();
extern          bool  PDSCDebug_SetupChanged();


// PDSC Debug Properties
extern PROTOCOL_TYPE PDSCDebug_GetDefaultProtocol();
extern          bool PDSCDebug_ProtocolSupported(PROTOCOL_TYPE prot);  // Does default DP support the protocol?
extern          bool PDSCDebug_ProtocolSwitchable(void);               // Is protocol switchable (SWJ-DP)?
extern           U32 PDSCDebug_SetActiveDP(U32 id);   // Set ID if active DP (id - internal ID as used in JDEVS)
extern           U32 PDSCDebug_GetActiveDP();         // Get ID if active DP (returns internal ID as used in JDEVS)

// Debug Port Management
extern U32              PDSCDebug_GetInternalDeviceId(U32 id);  // Get internal ID (as used in JDEVS) for given Port ID
extern PDSC_DEBUG_PORT* PDSCDebug_GetDebugPort(U32 portId);     // Get Debug Port by PDSC port ID

// PDSC Sequences
extern const char*   PDSCDebug_GetSequenceName(U32 id);         // Get name for a sequence ID
extern const char*   PDSCDebug_GetSequenceDescription(U32 id);  // Get description for a sequence ID
extern       bool    PDSCDebug_IsSequenceEnabled(U32 id);       // Sequence implemented and enabled

// PDSC DBGCONF File
extern const char*   PDSCDebug_GetDbgConfFilePath();            // Get currently used DBGCONF file path or NULL if not existent

// SDF Definitions
extern       bool    PDSCDebug_HasSDF();                        // Definitions from SDF available

// Other Getters
extern const char*   PDSCDebug_GetPackId();                     // Get Pack ID from debug properties
extern const char*   PDSCDebug_GetLogFile();                    // Get currently used log file name


// Data Patches
/*
 * Patch Data as read from target
 * accType : Memory, AP, or DP access. See ACCESS_TYPE in PDSCDebugEngine.h
 * accSize : Access size, use ACCMX defines (ACCMX_U8 - 1, ACCMX_U16 - 2, ACCMX_U32 - 4)
 * addr    : Start address of the target access.
 * many    : Number of bytes read from the target.
 * data    : Read data to patch, this is the complete buffer as read during the target access.
 * attrib  : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
 * return value: error code
 */
extern __inline U32 PDSCDebug_PatchData(U32 accType, BYTE accSize, U32 addr, U32 many, UC8 *data, BYTE attrib);

// Internal Sequences
extern U32 PDSCDebug_InitDriver(void);
extern U32 PDSCDebug_InitDebugger(void);
extern U32 PDSCDebug_UnInitDebugger(void);
extern U32 PDSCDebug_DebugGetDeviceList(JDEVS *DevList, unsigned int maxdevs, bool merge);
extern U32 PDSCDebug_DebugGetDeviceNames(JDEVS *DevList, unsigned int maxdevs, bool merge);
extern U32 PDSCDebug_DebugReadRomTable(void);
extern U32 PDSCDebug_DebugReadTargetFeatures(void);
extern U32 PDSCDebug_DebugReadDAPFeatures(void);

// Overridable Sequences

// Debug Port Sequences
extern U32 PDSCDebug_DebugPortSetup(void);
extern U32 PDSCDebug_DebugPortStart(void);
extern U32 PDSCDebug_DebugPortStop(void);

// Target Setup Sequences
extern U32 PDSCDebug_DebugDeviceUnlock(void);
extern U32 PDSCDebug_DebugCoreStart(void);
extern U32 PDSCDebug_DebugCoreStop(void);

// Reset Sequences
extern U32 PDSCDebug_ResetSystem(void);
extern U32 PDSCDebug_ResetProcessor(void);
extern U32 PDSCDebug_ResetHardware(BYTE bPreReset);
extern U32 PDSCDebug_ResetCustomized(void);
extern U32 PDSCDebug_ResetHardwareAssert(void);
extern U32 PDSCDebug_ResetHardwareDeassert(void);
extern U32 PDSCDebug_ResetCatchSet(void);
extern U32 PDSCDebug_ResetCatchClear(void);

// Recovery Sequences
extern U32 PDSCDebug_RecoverySupportStart(void);
extern U32 PDSCDebug_RecoverySupportStop(void);
extern U32 PDSCDebug_RecoveryAcknowledge(void);

// Trace Sequences
extern U32 PDSCDebug_TraceStart(void);
extern U32 PDSCDebug_TraceStop(void);

// Other Sequences
extern U32 PDSCDebug_DebugCodeMemRemap(void);

// Flash Sequences
extern U32 PDSCDebug_FlashEraseDone(void);
extern U32 PDSCDebug_FlashProgramDone(void);

// Top Level Sequences
extern U32   PDSCDebug_InitTarget   (void);
extern U32   PDSCDebug_ReInitTarget (void);
extern U32   PDSCDebug_ResetTarget  (void);
extern void  PDSCDebug_StopTarget   (void);

#endif // DBGCM_DBG_DESCRIPTION

#endif // __PDSCDEBUG_H__
