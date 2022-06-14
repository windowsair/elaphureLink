/**************************************************************************//**
 *           PDSCDebugEngine.h: CMSIS Pack Debug Description Engine - Globally
 *                              used structures and type definitions.
 * 
 * @version  V1.0.15
 * @date     $Date: 2016-10-17 12:51:38 +0200 (Mon, 17 Oct 2016) $
 *
 * @note
 * Copyright (C) 2018-2020 ARM Limited. All rights reserved.
 * 
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision.
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

 #ifndef __PDSC_DEBUG_ENGINE_H__
#define __PDSC_DEBUG_ENGINE_H__
/*
 * PDSCDebugEngine.h: PDSC Debug Engine datatypes and interfaces
 *    For internal testing only, not part of a release build.
 * Copyright: Keil - An ARM Company
 */


#include "AGDI.H"

#ifdef __cplusplus
  extern "C"  {
#endif


#define SIZE_SEQUENCE_NAME   128
#define SIZE_ACCESSVAR_NAME  128
#define SIZE_INFO_STR        128
#define SIZE_FLASHINFO_NAME  128


enum PROTOCOL_TYPE {
  PROTOCOL_ANY,
  PROTOCOL_JTAG,
  PROTOCOL_SWD,
  PROTOCOL_COUNT,
  PROTOCOL_UNKNOWN = PROTOCOL_COUNT,
};


enum ACCESS_PORT_TYPE {
  AP_CSJTAG,               // CoreSight JTAG-AP
  AP_CSAUTH,               // CoreSight AUTH-AP
  AP_CSAHB,                // CoreSight AHB-AP
  AP_CSAPB,                // CoreSight APB-AP
  AP_CSAPBM,               // CoreSight Cortex-M APB-AP
  AP_CSAXI,                // CoreSight AXI-AP
  AP_COUNT,
  AP_UNKNOWN = AP_COUNT,
};


enum DEBUG_BLOCK_TYPE {
  BLOCK_CPU,               // CPU Debug Block
  BLOCK_FPB,               // Flash Patch and Breakpoint Unit
  BLOCK_DWT,               // Data Watch and Trace Unit
  BLOCK_ITM,               // Instrumentation Trace Macrocell
  BLOCK_ETM,               // Embedded Trace Macrocell
  BLOCK_MTB,               // Micro Trace Buffer
  BLOCK_TMC,               // Trace Memory Controller
  BLOCK_TPIU,              // Trace Port Interface Unit
  BLOCK_SWO,               // Serial Wire Output
  BLOCK_ETB,               // Embedded Trace Buffer
  BLOCK_PMU,               // Performance Monitor Unit
  BLOCK_PTM,               // Program Trace Macrocell
  BLOCK_STM,               // System Trace Macrocell
  BLOCK_HTM,               // AHB Trace Macrocell
  BLOCK_ATBREP,            // (Programmable) ATB Trace Replicator
  BLOCK_ATBFUN,            // ATB Trace Funnel
  BLOCK_CTI,               // Cross Trigger Interface
  BLOCK_TSGEN,             // Timestamp Generator
  BLOCK_GPR,               // Granular Power Requester
  BLOCK_ELA,               // Embedded Logic Analyzer
  BLOCK_COUNT,
  BLOCK_UNKNOWN = BLOCK_COUNT,
};


enum TOPOLOGY_LINK_TYPE {
  TOPLINK_CPU,             // CPU Affinity of a Debug Block
  TOPLINK_TRACE,           // Trace Link
  TOPLINK_TRIGGER,         // Cross Trigger Link
  TOPLINK_COUNT,
  TOPLINK_UNKNOWN = TOPLINK_COUNT,
};

enum QUERY_TYPE {
  QUERY_OK,
  QUERY_YESNO,
  QUERY_YESNOCANCEL,
  QUERY_OKCANCEL,
  QUERY_COUNT,
  QUERY_UNKNOWN = QUERY_COUNT,
};

enum QUERY_RESULT {
  QUERY_RESULT_ERROR  = 0,  // Error
  QUERY_RESULT_OK     = 1,  // OK
  QUERY_RESULT_CANCEL = 2,  // Cancel
  QUERY_RESULT_YES    = 3,  // Yes
  QUERY_RESULT_NO     = 4,  // No
  QUERY_RESULT_COUNT,
  QUERY_RESULT_UNKNOWN = QUERY_RESULT_COUNT,
};

enum MESSAGE_TYPE {
  MESSAGE_INFO    = 0,
  MESSAGE_WARNING = 1,
  MESSAGE_ERROR   = 2,
  MESSAGE_COUNT,
  MESSAGE_UNKNOWN = MESSAGE_COUNT,
};

enum ACCESS_TYPE {
  ACCESS_MEM, 
  ACCESS_AP, 
  ACCESS_DP,
  ACCESS_ACCESS_AP,
  ACCESS_COUNT,
  ACCESS_UNKNOWN = ACCESS_COUNT,
};

enum SEQUENCE_ID {
  SEQ_DebugPortSetup,
  SEQ_DebugPortStart,
  SEQ_DebugPortStop,
  SEQ_DebugDeviceUnlock,
  SEQ_DebugCoreStart,
  SEQ_DebugCoreStop,
  SEQ_DebugCodeMemRemap,
  SEQ_ResetSystem,
  SEQ_ResetProcessor,
  SEQ_ResetHardware,
  SEQ_ResetHardwareAssert,
  SEQ_ResetHardwareDeassert,
  SEQ_ResetCatchSet,
  SEQ_ResetCatchClear,
  SEQ_FlashEraseDone,
  SEQ_FlashProgramDone,
  SEQ_RecoverySupportStart,
  SEQ_RecoverySupportStop,
  SEQ_RecoveryAcknowledge,
  SEQ_TraceStart,
  SEQ_TraceStop,
  SEQ_ResetCustomized,         // ID for specialized default reset sequence
  SEQ_FlashInit,
  SEQ_FlashUninit,
  SEQ_FlashEraseSector,
  SEQ_FlashEraseChip,
  SEQ_FlashProgramPage,
  SEQ_ID_COUNT,
  SEQ_ID_UNKNOWN = SEQ_ID_COUNT
};

enum ACCESSVAR_ID {       // IDs of Pre-Defined Debug Access Variables
  AV_ID_PROTOCOL,     // __protocol
  AV_ID_CONNECTION,   // __connection
  AV_ID_DP,           // __dp
  AV_ID_AP,           // __ap
  AV_ID_TRACEOUT,     // __traceout
  AV_ID_ERRCONTROL,   // __errorcontrol
  AV_ID_FLASHADDR,    // __FlashAddr
  AV_ID_FLASHLEN,     // __FlashLen
  AV_ID_FLASHARG,     // __FlashArg
  AV_ID_FLASHOP,      // __FlashOp
  AV_ID_RESULT,       // __Result
  AV_ID_APID,         // __apid
  AV_ID_COUNT,
  AV_ID_UNKNOWN = AV_ID_COUNT,
};

// Access Variable definitions

// "__protocol" definitions
#define AV_PROTOCOL_TYPE    0x0000FFFF  // Protocol Type Mask
#define AV_PROT_TYPE_ERR    0x00000000  // Error
#define AV_PROT_TYPE_JTAG   0x00000001  // JTAG
#define AV_PROT_TYPE_SWD    0x00000002  // Serial Wire Debug
#define AV_PROTOCOL_SWJ     0x00010000  // SWJ-DP
#define AV_PROTOCOL_DORMANT 0x00020000  // Protocol Switch through Dormant State supported

// "__connection" definitions
#define AV_CONNECTION_TYPE  0x000000FF  // Connection Type Mask
#define AV_CONN_TYPE_ERR    0x00000000  // Error
#define AV_CONN_TYPE_DEBUG  0x00000001  // Debug Connection
#define AV_CONN_TYPE_FLASH  0x00000002  // Flash Connection
#define AV_CONN_RST         0x0000FF00  // Reset Type Mask
#define AV_CONN_RST_P                8  // Reset Type Position
#define AV_CONN_RST_ERR              0  // Error
#define AV_CONN_RST_HW               1  // Hardware Reset (reset line)
#define AV_CONN_RST_SYS              2  // System Reset Request (register access)
#define AV_CONN_RST_VEC              3  // Vector/Processor Reset (register access)
#define AV_CONN_HOLD_RST    0x00010000  // Connection under Hardware Reset
#define AV_CONN_PRECONN_RST 0x00020000  // Pre-Connection Hardware Reset Pulse

// "__traceout" definitions
#define AV_TRACEOUT_TYPE     0x00000007  // Trace Type Mask
#define AV_TRACEOUT_SWO      0x00000001  // Serial Wire Trace (SWO)
#define AV_TRACEOUT_PORT     0x00000002  // Parallel Trace Port
#define AV_TRACEOUT_BUFFER   0x00000004  // Trace Buffer
#define AV_TRACEOUT_PORTSZ   0x003F0000  // Parallel Trace Port Size Mask
#define AV_TRACEOUT_PORTSZ_P         16  // Parallel Trace Port Size Position

// "__errorcontrol" definitions
#define AV_ERRCTRL_SKIP      0x00000001  // Skip Errors


enum DEBUG_CONTEXT {
  DBGCON_CONNECT,                 // Connect to target
  DBGCON_DISCONNECT,              // Disconnects from target
  DBGCON_RESET,                   // Reset Request
  DBGCON_TARGET_ACCESS,           // Accessing target memory/registers
  DBGCON_VERIFY_CODE,             // Verifying downloaded code
  DBGCON_RECOVERY,                // Recovering from connection loss
  DBGCON_UPDATE_SETTINGS,         // Updating target configuration settings
  DBGCON_GO,                      // Set target going
  DBGCON_STOP,                    // Stop target (Debug Halt)
  DBGCON_STEP,                    // Single-step target
  DBGCON_FLASH_ERASE,             // Erasing flash memory
  DBGCON_FLASH_PROGRAM,           // Programming flash memory
  DBGCON_COUNT,                   // number of available debug contexts
  DBGCON_UNKNOWN = DBGCON_COUNT,  // 
};


// Data Types
#pragma pack(1)
typedef struct PDSCDebugConfig_t {
  U32            defaultDP;  // Default Debug Port
  U32            defaultAP;  // Default Access Port
  U32      defaultProtocol;  // see PROTOCOL_TYPE
  U32            reserved0;  // Reserved for future use
  U64           debugClock;  // Default debug clock
  U32             swj :  1;  // SWJ debug
  U32         dormant :  1;  // Protocol switch through dormant state is supported
  U32                 : 30;  // reserved
  U32      defaultRstSeqId;  // Default Reset Sequence ID
  U64       defaultDbgAddr;  // Default Base Address for CPU Debug Block (SCS)
  U32         reserved[14];
} PDSC_DEBUG_CONFIG;


typedef struct PDSCDebugPortImp_t {
  U32            protocol;  // see PROTOCOL_TYPE
  U32    implemented :  1;  // protocol implementation available (1)
  U32                : 31;  // reserved 
  U32           targetsel;  // TARGETSEL value for DP v2 with multi-drop, 0 if not specified
  U32              idcode;  // JTAG / SW IDCODE , 0 if not specified
  U32            tapindex;  // JTAG only : Index of the TAP in the JTAG chain, default is 0
  U32               irlen;  // JTAG only: Instruction register length, 0 if not specified
  U32        reserved[16];
} PDSC_DEBUG_PORT_IMP;


typedef struct PDSCDebugAttrib_t {
  PDSCDebugAttrib_t       *next;  // Next Attribute
  char                    *name;  // Attribute Name ('\0'-terminated)
  char                   *value;  // Attribute Value ('\0'-terminated)
  U32              reserved[16];  // 
} PDSC_DEBUG_ATTRIB;


typedef struct PDSCDebugPort_t {
  struct PDSCDebugPort_t  *next;  // Next Debug Port Description
  U32                    portid;  // Debug Port ID for easier reference
  PDSC_DEBUG_PORT_IMP      jtag;  // JTAG implementation of the port
  PDSC_DEBUG_PORT_IMP       swd;  // SWD  implementation of the port
  PDSC_DEBUG_ATTRIB*    attribH;  // Head of Attributes
  U32              reserved[15];
} PDSC_DEBUG_PORT;


typedef struct PDSCAccessPort_t {
  struct PDSCAccessPort_t *next;  // Next Access Port Description
  U32                      type;  // Access Port Type, see ACCESS_PORT_TYPE
  U32                        dp;  // DP to access this Access Port
  union {
    U32                   index;  // Access Port Index on DAP-Bus (APv1)
    U32                    addr;  // Access Port Address (APv2)
  };
  U64                      base;  // ROM Table base address
  PDSC_DEBUG_ATTRIB*    attribH;  // Head of Attributes
  U32                        id;  // ID unique to a list of APs (SDF APs vs PDSC APs)
  U32                    parent;  // ID of parent AP (APv2 only)
  UCHAR               apVersion;  // Access Port Architecture Version
  UCHAR            hasBase :  1;  // 'base' member valid
  UCHAR          hasParent :  1;  // 'parent' member valid
  UCHAR                    :  6;  // reserved for future use
  U16                     res16;  // reserved for future use
  U32              reserved[13];  // reserved for future use
} PDSC_ACCESS_PORT;


typedef struct PDSCDebugBlock_t {
  struct PDSCDebugBlock_t *next;  // Next Debug Block Description
  U32                        id;  // Unique ID for later reference
  U32                      type;  // see DEBUG_BLOCK_TYPE
  U32                        dp;  // DP to access this Debug Block
  U32                        ap;  // AP index to access this Debug Block
  U64                      addr;  // Base Address of Debug Block
  PDSC_DEBUG_ATTRIB*    attribH;  // Head of Attributes
  U32              reserved[16];  // reserved for future use
} PDSC_DEBUG_BLOCK;


typedef struct PDSCTopologyLink_t {
  struct PDSCTopologyLink_t *next;  // Next Debug Block Description
  U32                        type;  // Topology Link Type, see TOPOLOGY_LINK_TYPE
  U32                      master;  // Master Debug Block ID
  U32                    masterif;  // Index in Master Interface
  U32                       slave;  // Slave Debug Block ID
  U32                     slaveif;  // Index in Slave Interface
  U32                     trigger;  // Trigger number if type is TOPLINK_TRIGGER
  U32                reserved[16];  // reserved for future use
} PDSC_TOPOLOGY_LINK;


typedef struct PDSCDataPatch_t {
  struct PDSCDataPatch_t *next;  // Next Data Patch
  U32                     type;  // Patch type, 0 - MEM, 1 - AP, 2 - DP, rest is undefined
  U32                       dp;  // DP to apply the patch for
  U32                       ap;  // AP to apply the patch for
  U64                     addr;  // Address to apply the patch for
  U64                    value;  // Value to patch in
  U64                     mask;  // Bits to patch
  U32             enabled :  1;  // Data Patch enabled
  U32                     : 31;  // Unused
  char     info[SIZE_INFO_STR];  // Info string as to show for logs or error diagnostics (can be empty)
  U32              reserved[9];  // reserved for future use
} PDSC_DATA_PATCH;


typedef struct PDSCTraceSwoParams_t {
  struct PDSCTraceSwoParams_t *next;  // next <serialwire> element
  U32                  reserved[16];  // reserved for future use
} PDSC_TRACE_SWO;


typedef struct PDSCTracePortParams_t {
  struct PDSCTracePortParams_t *next;  // next <serialwire> element
  U32                      portwidth;  // available trace port widths
  U32                   reserved[16];  // reserved for future use
} PDSC_TRACE_PORT;


typedef struct PDSCTraceBufferParams_t {
  struct PDSCTraceBufferParams_t *next;  // next <serialwire> element
  U32                        reserved0;  // reserved for future use
  U64                            start;  // start of trace buffer, 0 if not specified
  U64                             size;  // size of trace buffer, 0 if not specified
  U32                     reserved[16];  // reserved for future use
} PDSC_TRACE_BUFFER;


typedef struct PDSCTrace_t {
  U32             specified :  1;  // <trace> element specified in PDSC
  U32                       : 31;  // reserved for future use
  PDSC_TRACE_SWO            *swo;  // Head of <serialwire> elements, NULL if none specified
  PDSC_TRACE_PORT          *port;  // Head of <traceport> elements, NULL if none specified
  PDSC_TRACE_BUFFER      *buffer;  // Head of <tracebuffer> elements, NULL if none specified
  U32               reserved[32];  // reserved for future use
} PDSC_TRACE;


typedef struct PDSCSequence_t {
  struct      PDSCSequence_t* next; // Next sequence
  U32                           id; // Sequence ID (see SEQUENCE_ID)
  char    name[SIZE_SEQUENCE_NAME]; // Sequence name to show in error messages
  char         info[SIZE_INFO_STR]; // Info string as to show for logs or error diagnostics (can be empty)
  U32                 disable :  1; // Sequence is disabled by PDSC including default implementation
  U32                         : 31; // Unused
  U32                 reserved[16]; // reserved for future use
} PDSC_SEQUENCE;

typedef struct PDSCAccessVar_t {
  struct PDSCAccessVar_t    *next;  // Next Access Variable
  char  name[SIZE_ACCESSVAR_NAME];  // Access Variable Name
  U32                          id;  // Access Variable ID
  U64                       value;  // Access Variable Value
  U32                reserved[16];  // reserved for future use
} PDSC_ACCESS_VAR;


typedef struct PDSCSequenceContext_t {
  U32                           id; // Sequence ID (see SEQUENCE_ID)
  PDSC_ACCESS_VAR            *vars; // List of Debug Access Variables
  U32                 debugContext; // Debug Context under which it is executed (see PDSC_DEBUG_CONTEXT)
  U32                      timeout; // Sequence execution timeout in microseconds, 0 means default timeout
  U32                 reserved[30]; // reserved for future use
} PDSC_SEQUENCE_CONTEXT;


typedef struct PDSCDbgConfFile_t {
  const char                *path; // '\0'-terminated path to used DBGCONF file copy
  DWORD              reserved[32]; // Reserved
} PDSC_DBGCONF_FILE;


typedef struct PDSCFlashBlocks_t {
  PDSCFlashBlocks_t         *next;  // Next Flash Blocks Description      // For consistency with the rest of structs in this header
  U64                        addr;  // Start address of described blocks
  U32                       count;  // Number of subsequent blocks
  U32                   blockSize;  // Size in bytes of each block
  U64                         arg;  // Argument to flash operation
  U32                reserved[16];
} PDSC_FLASH_BLOCKS;


typedef struct PDSCFlashInfo_t {
  struct PDSCFlashInfo_t*    next;  // Next Flash Info
  char  name[SIZE_FLASHINFO_NAME];  // Flash Info Name
  U64                        addr;  // Flash Device Start Address as mapped into target memory map
  U32                        size;  // Flash Device Size
  U32                    pageSize;  // Flash Programming Page Size
  U64                    blankVal;  // Expected value for unprogrammed flash memory
  U64                      filler;  // Value to fill remainders of a flash page
  U32                 progTimeout;  // Timeout for programming a page
  U32                eraseTimeout;  // Timeout for erasing a sector
  PDSC_FLASH_BLOCKS*       blocks;  // Head of blocks of the flash device, list ordered by mapped addresses
  U32                reserved[16];
} PDSC_FLASH_INFO;


typedef struct PDSCDebugProperties_t {
  PDSC_DEBUG_CONFIG   debugConfig;  // Debug Configuration for this target connection
  PDSC_DEBUG_PORT          *ports;  // Debug Ports Head
  PDSC_DATA_PATCH    *dataPatches;  // Data Patches Head
  PDSC_TRACE                trace;  // Trace capabilities
  PDSC_SEQUENCE        *sequences;  // Sequences Head
  U32                    protocol;  // Currently selected protocol
  U64                  debugClock;  // Currently selected debug clock
  
  DWORD              enabled :  1;  // Usage of PDSC is enabled
  DWORD                  log :  1;  // Sequence/Command Log Enabled
  DWORD                  sdf :  1;  // System Description File present
  DWORD             flashSeq :  1;  // Usage of Flash Programming Sequences enabled
  DWORD                      : 28;  // unused

  PDSC_DBGCONF_FILE  *dbgConfFile;  // DBGCONF File Info

  const char              *packid;  // Pack ID of pack providing this Debug Description
  const char             *logfile;  // Current log file name (<name>_<index>.log, <index>: 4-digit decimal index).

  // System Description File (SDF) Information
  PDSC_ACCESS_PORT     *accessPorts; // Access Ports Head (SDF)
  PDSC_DEBUG_BLOCK     *debugBlocks; // Debug Blocks Head
  PDSC_TOPOLOGY_LINK *topologyLinks; // Topology Links Head
  struct {
    U16                     major;  // Major version number 
    U16                     minor;  // Minor version number 
  }                   sdf_version;  // SDF format version 

  PDSC_FLASH_INFO     *flashInfos;  // Flash Information Head
  
  DWORD             reserved[123];  // reserved for future use
} PDSC_DEBUG_PROPERTIES;
#pragma pack()

#ifdef __cplusplus
  }
#endif

#endif // __PDSC_TEST_ENGINE_H__