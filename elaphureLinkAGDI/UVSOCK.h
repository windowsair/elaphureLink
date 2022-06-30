/** @file  UVSOCK.h
  * @brief UVSOCK Interface
  * @version V2.29
  *
  * API for uVision UVSOCK socket interface.
  *
  * <b>General API Rules</b>
  *
  * @li Multiple TCP connections are possible simultaneously
  * @li All 'reserved' data fields must be sent as 0 and ignored when received
  * @li Non-relevant data fields in requests must be sent as 0
  *
  *
  * <b>Version History</b>
  *
  * Author, Date, Version
  * @li PH , 12.03.2006, V1.00
  * @li RAS, 11.11.2007, V2.00
  * @li RAS, 03.01.2008, V2.01
  * @li RAS, 08.02.2008, V2.02
  * @li RAS, 20.02.2008, V2.03
  * @li RAS, 18.03.2008, V2.04
  * @li RAS, 22.04.2008, V2.05
  * @li RAS, 25.04.2008, V2.06
  * @li RAS, 09.05.2008, V2.07
  * @li RAS, 25.09.2008, V2.08
  * @li PH,  22.07.2009, V2.09
  * @li TdB, 25.11.2010, V2.10
  * @li ED,  20.02.2011, V2.11
  * @li ED,  20.03.2011, V2.12 / V2.13
  * @li PH,  28.04.2011, V2.14
  * @li JR,  18.04.2012, V2.24
  * @li JR,  16.04.2013, V2.25
  * @li TdB, 09.06.2016, V2.26
  * @li TdB, 09.06.2016, V2.27
  * @li TdB, 09.06.2016, V2.28
  * @li TdB, 25.01.2017, V2.29
  *
  * <b>Change Log</b>
  *
  * Version 2.00:
  * @li Socket server move from UI thread to own thread
  * @li DBG WAKE/SLEEP implemented for Simulated I/O
  *
  * Version 2.01:
  * @li API renamed to UVSOCK(.h)
  * @li BKRSP command default length increased to 512 bytes
  *
  * Version 2.02:
  * @li API documented
  * @li Changed defines to enumerations
  *
  * Version 2.03:
  * @li Added more status codes
  *
  * Version 2.04:
  * @li Increased size of UVSC internal message queue to 200
  *
  * Version 2.05:
  * @li Added xBOOL type definition
  *
  * Version 2.06:
  * @li Enhancement: Modified Message Box blocking mechanism, reducing the amount of messages
  *                  required to complete several commands
  * @li Baseline:    Backwards compatibility for UVSC_C.h from this version
  *
  * Version 2.07:
  * @li UVSC changes only
  *
  * Version 2.08:
  * @li Enhancement: Added UVSC_DBG_EXEC_CMD / GetCmdOutput / GetCmdOutputSize functions. They
  *                  allows any command line to be executed via UVSOCK
  * @li Enhancement: Added UVSC_PRG_GET_OUTPUTNAME command. It allows the current executable name to be retrieved.
  *
  * Version 2.09:
  * @li Enhancement: Changed display of Breakpoint-command responses to show BP_DELETED, BP_ENABLED etc.
  *                  This requires to look at 'cmdRsp.status' for exact Bp-action.
  *
  * Version 2.10:
  * @li Enhancement: Added #UVSC_PSTAMP, #UV_DBG_POWERSCALE_SHOWCODE, #UV_DBG_POWERSCALE_SHOWPOWER, #UVSC_POWERSCALE_SHOWCODE
  *                  to support Hitex PowerScale.
  *
  * Version 2.11:
  * @li Enhancement: Added #UV_DBG_EVAL_EXPRESSION_TO_STR and  #UV_DBG_FILELINE_TO_ADR
  *                  to support Eclipse client
  * Version 2.12:
  * @li Enhancement: Added #UV_DBG_ENUM_REGISTER_GROUPS, #UV_DBG_ENUM_REGISTERS, #UV_DBG_READ_REGISTERS and #UV_DBG_REGISTER_SET
  *                  to support Eclipse client
  * Version 2.13:
  * @li Enhancement: Added #UV_DBG_DSM_READ
  *                  to support Eclipse client
  *
  * Version 2.14:
  * @li Correction:  Mantis #4789: removed 'UVSC_PSTAMP' from response created by OkToClientGeneric()
  *
  * Version 2.15:
  * @li Bugfix:      Writing beyond buffer size is fixed in UV_DBG_ENUM_STACK
  *
  * Version 2.17:
  * @li Enhancement: Added #UV_DBG_EVAL_WATCH_EXPRESSION, #UV_DBG_REMOVE_WATCH_EXPRESSION
  *                        #UV_DBG_ENUM_VARIABLES,  #UV_DBG_VARIABLE_SET
  *                  to support Eclipse client
  *
  * Version 2.18:
  * @li Enhancement: Added #UV_DBG_ENUM_TASKS
  *                  to support Eclipse client
  *
  * Version 2.19:
  * @li Enhancement: Added #UV_DBG_SERIAL_OUTPUT
  *                  to support Eclipse client
  *
  * Version 2.20:
  * @li Enhancement: Added #UV_DBG_ENUM_MENUS, #UV_DBG_MENU_EXEC
  *                  to support Eclipse client
  *
  * Version 2.21:
  * @li Enhancement: Struct VARINO is extended with number of flags
  *                  to support Eclipse client
  *
  * Version 2.22:
  * @li Correction:  Comments are changed to match implementation
  *
  *
  * Version 2.23:
  * @li Enahncement: Added UV_GEN_SET_OPTIONS to set general UVSC options
  *
  *
  * Version 2.24:
  * @li Documentation Change: #UVSC_PSTAMP, #UV_DBG_POWERSCALE_SHOWCODE, #UV_DBG_POWERSCALE_SHOWPOWER, #UVSC_POWERSCALE_SHOWCODE
  *
  * Version 2.25:
  * @li Enhancement: Added 'showSyncErr' flag to UVSC_PSTAMP to show sync error message in uVision
  *
  * Version 2.26:
  * @li Enhancement: Added #UV_DBG_ITM_OUTPUT
  *                  to support ITM serial output
  *
  * Version 2.27:
  * @li Enhancement: fixed Copyright
  *
  * Version 2.28:
  * @li Enhancement: fixed Version numbers for all UVSock related files
  *
  * Version 2.29:
  * @li Enhancement: added Event Recorder
  *
  */


/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2004-2017 KEIL - An ARM Company. All rights reserved.        */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#ifndef _UVSOCK_H_
#define _UVSOCK_H_

/** UVSOCK API version number
  *
  * This number is converted to an X.YY version number by the following formula:
  * <pre> X.YY = UV3_SOCKIF_VERS / 100 </pre>
  *
  * For example:
  * <pre> UV3_SOCKIF_VERS = 201 ==> V2.01 </pre>
  *
  */
#define UV3_SOCKIF_VERS 229

/** Maximum transfer size of a single UVSOCK packet (bytes)
  *
  * No UVSOCK packet may be larger than @a SOCK_NDATA bytes. This includes both the packet header and packet data.
  */
#define SOCK_NDATA 32768

/** Boolean false
  *
  * Value of false for the xBOOL data type
  */
#define xFALSE ((xBOOL)0)

/** Boolean true
  *
  * Value of true for the xBOOL data type
  */
#define xTRUE (!xFALSE)


#ifdef WIN32
/** Boolean data type
  *
  * This may need to be redefined if not running on Microsoft Windows.
  */
typedef unsigned char xBOOL;

/** 8-bit unsigned data type
  *
  * This may need to be redefined if not running on Microsoft Windows.
  */
typedef unsigned char xUC8;

/** 16-bit unsigned data type
  *
  * This may need to be redefined if not running on Microsoft Windows.
  */
typedef unsigned short int xWORD16;

/** 16-bit signed data type
  *
  * This may need to be redefined if not running on Microsoft Windows.
  */
typedef signed short int xINT16;

/** 64-bit unsigned data type
  *
  * This may need to be redefined if not running on Microsoft Windows.
  */
typedef unsigned __int64 xU64;

/** 64-bit signed data type
  *
  * This may need to be redefined if not running on Microsoft Windows.
  */
typedef signed __int64 xI64;

#else
// Other platform type definitions
#endif /* #ifdef WIN32 */

/****************************************************************************/
// Socket-Commmands
/****************************************************************************/

/** UVSOCK command codes
  *
  * Each request, response and asynchronous message has a unique command code.
  * The response to a message will contain the same command code as the request
  * in its UVSOCK_CMD_RESPONSE structure.
  *
  */
typedef enum {

    //---Command codes:
    //--- General functions:
    UV_NULL_CMD        = 0x0000,    ///< Not a command. A message containing this code should be ignored
    UV_GEN_GET_VERSION = 0x0001,    ///< Get the UVSOCK interface version number
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> UINT / #UVSOCK_ERROR_RESPONSE
    UV_GEN_UI_UNLOCK = 0x0002,      ///< Enable message boxes and user input in uVision
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_UI_LOCK = 0x0003,        ///< Disable message boxes and user input in uVision
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_HIDE = 0x0004,           ///< Completely hide the uVision window
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_SHOW = 0x0005,           ///< Show the uVision window (bringing it to the front if it is behind other windows)
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_RESTORE = 0x0006,        ///< Restore the uVision window
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_MINIMIZE = 0x0007,       ///< Minimise the uVision window
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_MAXIMIZE = 0x0008,       ///< Maximise the uVision window
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_EXIT = 0x0009,           ///< Exit uVision
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_GET_EXTVERSION = 0x000A, ///< Get extended version number information for uVision (in ASCII format)
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #EXTVERS / #UVSOCK_ERROR_RESPONSE
    UV_GEN_CHECK_LICENSE = 0x000B,  ///< Check toolchain licensing
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #UVLICINFO / #UVSOCK_ERROR_RESPONSE
    UV_GEN_CPLX_COMPLETE = 0x000C,  ///< Complex command has completed
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_GEN_SET_OPTIONS = 0x000D,    ///< Sets UVSOCK options
                                    ///< @li Request format  : #UVSOCK_OPTIONS
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE


    //--- Project functions:
    UV_PRJ_LOAD = 0x1000,             ///< Load a uVision project
                                      ///< @li Request format  : #PRJDATA
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_CLOSE = 0x1001,            ///< Close the currently loaded uVision project
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ADD_GROUP = 0x1002,        ///< Add one or more groups to the current project
                                      ///< @li Request format  : #PRJDATA
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_DEL_GROUP = 0x1003,        ///< Remove one or more groups from the current project
                                      ///< @li Request format  : #PRJDATA
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ADD_FILE = 0x1004,         ///< Add one or more files to a group in the current project
                                      ///< @li Request format  : #PRJDATA
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_DEL_FILE = 0x1005,         ///< Remove one or more files from a group in the current project
                                      ///< @li Request format  : #PRJDATA
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_BUILD = 0x1006,            ///< Build the current project
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_REBUILD = 0x1007,          ///< Rebuild the current project
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_CLEAN = 0x1008,            ///< Clean current project
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_BUILD_CANCEL = 0x1009,     ///< Stop a currently progressing build / rebuild
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_FLASH_DOWNLOAD = 0x100A,   ///< Download the built binary to flash
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_GET_DEBUG_TARGET = 0x100B, ///< Get the currently configured debug target
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #DBGTGTOPT / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_SET_DEBUG_TARGET = 0x100C, ///< Set the currently configured debug target
                                      ///< @li Request format  : #DBGTGTOPT
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_GET_OPTITEM = 0x100D,      ///< Get an option item for the current project, see #OPTSEL
                                      ///< @li Request format  : #TRNOPT
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #TRNOPT / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_SET_OPTITEM = 0x100E,      ///< Set an option item for the current project, see #OPTSEL
                                      ///< @li Request format  : #TRNOPT
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ENUM_GROUPS = 0x100F,      ///< Enumerate the groups of the current project
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ENUM_FILES = 0x1010,       ///< Enumerate the files of a given group in the current project
                                      ///< @li Request format  : #SSTR
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_CMD_PROGRESS = 0x1011,     ///< Control the uVision UI progress bar
                                      ///< @li Request format  : #PGRESS
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ACTIVE_FILES = 0x1012,     ///< Get number of active files for the current project (i.e. how many files would be built on a rebuild)
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> UINT / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_FLASH_ERASE = 0x1013,      ///< Erase flash device
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_GET_OUTPUTNAME = 0x1014,   ///< Get the executable/library output object name for the current project
                                      ///< @li Request format  : #iPATHREQ
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ENUM_TARGETS = 0x1015,     ///< Enumerate the targets of the current project
                                      ///< @li Request format  : no data
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_SET_TARGET = 0x1016,       ///< Set a target active
                                      ///< @li Request format  : #PRJDATA
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_GET_CUR_TARGET = 0x1017,   ///< returns a string containing the current active target
                                      ///< @li Request format  : #iPATHREQ
                                      ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE

    UV_PRJ_SET_OUTPUTNAME = 0x1018, ///< Set a target name
                                    ///< @li Request format  : #PRJDATA
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE

    //--- Debug functions:
    UV_DBG_ENTER = 0x2000,              ///< Start the debugger
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_EXIT = 0x2001,               ///< Stop the debugger
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_START_EXECUTION = 0x2002,    ///< Start target execution
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_RUN_TO_ADDRESS = 0x2102,     ///< Start target execution and run to specified address
                                        ///< @li Request format  : xU64
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STOP_EXECUTION = 0x2003,     ///< Stop target execution
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #BPREASON / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STATUS = 0x2004,             ///< Check if the target / simulation is running
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> UINT / #UVSOCK_ERROR_RESPONSE
    UV_DBG_RESET = 0x2005,              ///< Reset the target / simulation
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_HLL = 0x2006,           ///< Step one line over HLL code(High Level Language code, eg C)
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_HLL_N = 0x2106,         ///< Step N lines over HLL code(High Level Language code, eg C)
                                        ///< @li Request format  : UINT
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_INTO = 0x2007,          ///< Step into HLL code(High Level Language code, eg C)
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_INTO_N = 0x2107,        ///< Perform N steps into HLL code(High Level Language code, eg C)
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_INSTRUCTION = 0x2008,   ///< Step one ASM Instruction
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_INSTRUCTION_N = 0x2108, ///< Step N ASM Instruction
                                        ///< @li Request format  : UINT
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_STEP_OUT = 0x2009,           ///< Step out of the current function
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_CALC_EXPRESSION = 0x200A,    ///< Calculate the value of an expression
                                        ///< @li Request format  : #VSET
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VSET / #UVSOCK_ERROR_RESPONSE
    UV_DBG_MEM_READ = 0x200B,           ///< Read memory
                                        ///< @li Request format  : #AMEM
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #AMEM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_MEM_WRITE = 0x200C,          ///< Write memory
                                        ///< @li Request format  : #AMEM
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #AMEM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_TIME_INFO = 0x200D,          ///< Get the current simulation cycles and time-stamp (NOTE: This information is also available in every UVSOCK message) [SIMULATOR ONLY]
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #CYCTS / #UVSOCK_ERROR_RESPONSE
    UV_DBG_SET_CALLBACK = 0x200E,       ///< Set a time-interval for callback [SIMULATOR ONLY]
                                        ///< @li Request format  : float (callback time in seconds)
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_VTR_GET = 0x200F,            ///< Read a Virtual Register (VTR) value
                                        ///< @li Request format  : #VSET
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VSET / #UVSOCK_ERROR_RESPONSE
    UV_DBG_VTR_SET = 0x2010,            ///< Write a Virtual Register (VTR) value
                                        ///< @li Request format  : #VSET
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_SERIAL_GET = 0x2011,         ///< DEPRECATED use DBG_SERIAL_OUTPUT response. Read serial output from a uVision serial window
                                        ///< @li Request format  : #SERIO
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #SERIO / #UVSOCK_ERROR_RESPONSE
    UV_DBG_SERIAL_PUT = 0x2012,         ///< Write serial output to a uVision serial window
                                        ///< @li Request format  : #SERIO
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_VERIFY_CODE = 0x2013,        ///< Verify the code in flash against built binary
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_CREATE_BP = 0x2014,          ///< Create a new breakpoint
                                        ///< @li Request format  : #BKPARM
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUMERATE_BP = 0x2015,       ///< Enumerate all currently defined breakpoints
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_CHANGE_BP = 0x2016,          ///< Enable, disable or delete an existing breakpoint
                                        ///< @li Request format  : #BKCHG
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #BKRSP
    UV_DBG_ENUM_SYMTP = 0x2017,         ///< Enumerate the struct members of a variable, i.e. the member size and packing
                                        ///< @li Request format  : #ENUMTPM
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ADR_TOFILELINE = 0x2018,     ///< Map an address to code file & linenumber
                                        ///< @li Request format  : #ADRMTFL
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #AFLMAP / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_STACK = 0x2019,         ///< Enumerate the call stack
                                        ///< @li Request format  : #iSTKENUM
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_VTR = 0x201A,           ///< Enumerate all virtual registers (VTRs)
                                        ///< @li Request format  : #iVTRENUM
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_UNUSED       = 0x201B,       ///< Unused
    UV_DBG_ADR_SHOWCODE = 0x201C,       ///< Show disassembly and/or HLL (High Level Language) file for an address
                                        ///< @li Request format  : #iSHOWSYNC
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
    UV_DBG_WAKE = 0x201D,               ///< Set sleep callback and/or wake up simulation [SIMULATOR ONLY]
                                        ///< @li Request format  : #iINTERVAL
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_SLEEP = 0x201E,              ///< Sleep the simulation [SIMULATOR ONLY]
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_MSGBOX_MSG = 0x201F,             ///< Notification of a UV message box
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_DBG_EXEC_CMD = 0x2020,           ///< Execute a command (as if via the command line)
                                        ///< @li Request format  : #EXECCMD
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE

    UV_DBG_POWERSCALE_SHOWCODE = 0x2021, ///< Show disassembly / HLL and trace entry in uVision for timestamp
                                         ///< @li Request format  : #UVSC_PSTAMP
                                         ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #UVSC_PSTAMP / #UVSOCK_ERROR_RESPONSE

    UV_DBG_POWERSCALE_SHOWPOWER = 0x2022, ///< Show power in PowerScale for timestamp
                                          ///< @li Request format  : #UVSC_PSTAMP
                                          ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #UVSC_PSTAMP / #UVSOCK_ERROR_RESPONSE

    POWERSCALE_OPEN = 0x2023, ///< Register PowerScale device

    UV_DBG_EVAL_EXPRESSION_TO_STR = 0x2024, ///< Evaluate expression and return result as string
                                            ///< @li Request format  : #VSET - value field is used to submit stack frame pointer
                                            ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VSET / #UVSOCK_ERROR_RESPONSE

    UV_DBG_FILELINE_TO_ADR = 0x2025, ///< Map a file & line number to an address
                                     ///< @li Request format  : #AFLMAP
                                     ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VSET / #UVSOCK_ERROR_RESPONSE
                                     ///< @li Async format    : N/A

    //---Registers:
    UV_DBG_ENUM_REGISTER_GROUPS = 0x2026, ///< Enumerate register groups
                                          ///< @li Request format  : #VSET - value field is used to submit stack frame pointer
                                          ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE

    UV_DBG_ENUM_REGISTERS = 0x2027, ///< Evaluate registers
                                    ///< @li Request format  : #REGENUM
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #REGENUM / #UVSOCK_ERROR_RESPONSE

    UV_DBG_READ_REGISTERS = 0x2028, ///< Get register values
                                    ///< @li Request format  : #SSTR
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> \#char[] / #UVSOCK_ERROR_RESPONSE

    UV_DBG_REGISTER_SET = 0x2029, ///< Set register value
                                  ///< @li Request format  : #VSET - value field is used to submit stack frame pointer
                                  ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_DSM_READ = 0x202A, ///< Read disassembly block
                              ///< @li Request format  : #AMEM
                              ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #AMEM / #UVSOCK_ERROR_RESPONSE

    UV_DBG_EVAL_WATCH_EXPRESSION = 0x202B,   ///< Add watch expression / evaluate existing
                                             ///< <b>Extended stack mode only</b>
                                             ///< @li Request format  : #VSET
                                             ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VARINFO / #UVSOCK_ERROR_RESPONSE
    UV_DBG_REMOVE_WATCH_EXPRESSION = 0x202D, ///< Remove watch expression
                                             ///< <b>Extended stack mode only</b>
                                             ///< @li Request format  : #VSET
                                             ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VARINFO / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_VARIABLES = 0x202E,          ///< Get variables for given stack frame, globals or struct/array members of a variable/watch
                                             ///< <b>Extended stack mode only</b>
                                             ///< @li Request format  : #IVARENUM
                                             ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VARINFO / #UVSOCK_ERROR_RESPONSE
    UV_DBG_VARIABLE_SET = 0x202F,            ///< Set variable value
                                             ///< <b>Extended stack mode only</b>
                                             ///< @li Request format  : #VARINFO
                                             ///< @li Response format : #UVSOCK_CMD_RESPONSE --> #VARINFO / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_TASKS = 0x2030,              ///< Enumerate task list - in non - Rtx case returns main thread
                                             ///< <b>Extended stack mode only</b>
                                             ///< @li Request format  : #iSTKENUM
                                             ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_ENUM_MENUS = 0x2031, ///< Enumerate available dynaic menus and peripheral dialogs
                                ///< @li Request format  : #MENUENUM
                                ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_MENU_EXEC = 0x2032, ///< Execute menu entry (for example to show view or or peripheral dialog)
                               ///< @li Request format  : #MENUENUM
                               ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_ITM_REGISTER = 0x2033, ///< Book ITM Channel (0..31) to receive the data sent to this channels
                                  ///< @li Request format  : #ITMOUT
                                  ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_ITM_UNREGISTER = 0x2034, ///< Unbook ITM Channel (0..31) to not receive the data sent to this channels anymore
                                    ///< @li Request format  : #ITMOUT
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_EVTR_REGISTER = 0x2035, ///< Register to receive the Event Recorder Data
                                   ///< @li Request format  : no data
                                   ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_EVTR_UNREGISTER = 0x2036, ///< Unregister to not receive the data sent from Event Recorder anymore
                                     ///< @li Request format  : no data
                                     ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_EVTR_GETSTATUS = 0x2037, ///< Status of Event Recorder
                                    ///< @li Request format  : no data
                                    ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE

    UV_DBG_EVTR_ENUMSCVDFILES = 0x2038, ///< Get SCVD files
                                        ///< @li Request format  : no data
                                        ///< @li Response format : #UVSOCK_CMD_RESPONSE --> no data / #UVSOCK_ERROR_RESPONSE


    //---Answers/Error from uVision to Client:
    UV_CMD_RESPONSE = 0x3000, ///< Response to a command from the client (the UVSOCK_CMD_RESPONSE structure will contain the command code to which this is a response)
                              ///< @li Response format : #UVSOCK_CMD_RESPONSE --> XXXX

    //---Asynchronous messages:
    UV_ASYNC_MSG = 0x4000, ///< Asynchronous message from uVision (the UVSOCK_CMD_RESPONSE structure will contain the relevant command code)
                           ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> XXXX

    //--- Special Asynchronous messages:
    //--- Project functions:
    UV_PRJ_BUILD_COMPLETE = 0x5000, ///< Notification of build completion
                                    ///< @li Async format    : #PRJDATA
    UV_PRJ_BUILD_OUTPUT = 0x5001,   ///< Notification of a line of build output
                                    ///< @li Async format    : #PRJDATA

    //--- Debug functions:
    UV_DBG_CALLBACK = 0x5002, ///< Notification of expiration of the callback timeout set by UV_DBG_SET_CALLBACK
                              ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE

    //--- Response to UV_DBG_ENUMERATE_BP:
    UV_DBG_BP_ENUM_START = 0x5004, ///< Start of breakpoint enumeration (no breakpoint info)
                                   ///< @li Async format    : no data
    UV_DBG_BP_ENUMERATED = 0x5005, ///< Breakpoint enumeration; zero, one or more Response(s) with breakpoint info
                                   ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #BKRSP / #UVSOCK_ERROR_RESPONSE
    UV_DBG_BP_ENUM_END = 0x5006,   ///< End of breakpoint enumeration (no breakpoint info)
                                   ///< @li Async format    : no data

    //--- Response to UV_PRJ_ENUM_GROUPS:
    UV_PRJ_ENUM_GROUPS_START = 0x5007, ///< Start of group enumeration
                                       ///< @li Async format    : no data
    UV_PRJ_ENUM_GROUPS_ENU = 0x5008,   ///< Group enumeration; zero, one or more Responses with group name
                                       ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ENUM_GROUPS_END = 0x5009,   ///< End of group enumeration
                                       ///< @li Async format    : no data

    //--- Response to UV_PRJ_ENUM_FILES:
    UV_PRJ_ENUM_FILES_START = 0x500A, ///< Start of files enumeration
                                      ///< @li Async format    : no data
    UV_PRJ_ENUM_FILES_ENU = 0x500B,   ///< File enumeration; zero, one or more Response(s) with file name
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ENUM_FILES_END = 0x500C,   ///< End of files enumeration
                                      ///< @li Async format    : no data

    //--- Progress bar functions
    UV_PRJ_PBAR_INIT = 0x500D, ///< Notification of progress bar initialisation
                               ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_PBAR_STOP = 0x500E, ///< Notification of progress bar stopping
                               ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_PBAR_SET = 0x500F,  ///< Notification of progress bar position change
                               ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE
    UV_PRJ_PBAR_TEXT = 0x5010, ///< Notification of progress bar text change
                               ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE

    //--- Response to UV_DBG_ENUM_SYMTP:
    UV_DBG_ENUM_SYMTP_START = 0x5011, ///< Start of structure member enumeration
                                      ///< @li Async format    : no data
    UV_DBG_ENUM_SYMTP_ENU = 0x5012,   ///< Structure member enumeration; zero, one or more Responses with member information
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #ENUMTPM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_SYMTP_END = 0x5013,   ///< End of structure member enumeration
                                      ///< @li Async format    : no data

    //--- Response to UV_DBG_ENUM_STACK:
    UV_DBG_ENUM_STACK_START = 0x5014, ///< Start of stack enumeration
                                      ///< @li Async format    : no data
    UV_DBG_ENUM_STACK_ENU = 0x5015,   ///< Stack enumeration; one or more Response(s) with stack frame information
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #STACKENUM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_STACK_END = 0x5016,   ///< End of stack enumeration
                                      ///< @li Async format    : no data

    //--- Response to UV_DBG_ENUM_VTR:
    UV_DBG_ENUM_VTR_START = 0x5017, ///< Start of vtr enumeration
                                    ///< @li Async format    : no data
    UV_DBG_ENUM_VTR_ENU = 0x5018,   ///< Vtr enumeration; one or more Response(s) of structure
                                    ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #AVTR / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_VTR_END = 0x5019,   ///< End of vtr enumeration
                                    ///< @li Async format    : no data

    //--- Command Window output
    UV_DBG_CMD_OUTPUT = 0x5020, ///< Notification of a line of command window output
                                ///< @li Async format    : #SSTR
    //--- Serial output
    UV_DBG_SERIAL_OUTPUT = 0x5120, ///< Notification of a serial output (debug or UART 1, 2 or 3)
                                   ///< @li Request format  : #SERIO

    //--- Response to UV_PRJ_ENUM_TARGETS:
    UV_PRJ_ENUM_TARGETS_START = 0x5021, ///< Start of targets enumeration
                                        ///< @li Async format    : no data
    UV_PRJ_ENUM_TARGETS_ENU = 0x5022,   ///< targets enumeration; zero, one or more Responses with group name
                                        ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE
    UV_PRJ_ENUM_TARGETS_END = 0x5023,   ///< End of targets enumeration
                                        ///< @li Async format    : no data

    //--- Response to UV_DBG_ENUM_REGISTER_GROUPS:
    UV_DBG_ENUM_REGISTER_GROUPS_START = 0x5024, ///< Start of register group enumeration
                                                ///< @li Async format    : no data
    UV_DBG_ENUM_REGISTER_GROUPS_ENU = 0x5025,   ///< register group enumeration; one or more Response(s) of structure
                                                ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_REGISTER_GROUPS_END = 0x5026,   ///< End of register group enumeration
                                                ///< @li Async format    : no data

    //--- Response to UV_DBG_ENUM_REGISTERS:
    UV_DBG_ENUM_REGISTERS_START = 0x5027, ///< Start of register enumeration
                                          ///< @li Async format    : no data
    UV_DBG_ENUM_REGISTERS_ENU = 0x5028,   ///< register enumeration; one or more Response(s) of structure
                                          ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #REGENUM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_REGISTERS_END = 0x5029,   ///< End of register enumeration
                                          ///< @li Async format    : no data

    // --- async Response to UV_DBG_ITM_REGISTER
    UV_DBG_ITM_OUTPUT = 0x5030, ///< Notification of an ITM data output (Channel 0..31)
                                ///< @li Async format  : #ITMOUT


    //--- Response to UV_DBG_ENUM_VARIABLES:
    UV_DBG_ENUM_VARIABLES_START = 0x5040, ///< Start of variable enumeration
                                          ///< @li Async format    : no data
    UV_DBG_ENUM_VARIABLES_ENU = 0x5041,   ///< variable enumeration; one or more Response(s) of structure
                                          ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #VARINFO / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_VARIABLES_END = 0x5042,   ///< End of variable enumeration
                                          ///< @li Async format    : no data

    //--- Response to UV_DBG_ENUM_TASKS:
    UV_DBG_ENUM_TASKS_START = 0x5050, ///< Start of task list enumeration
                                      ///< @li Async format    : no data
    UV_DBG_ENUM_TASKS_ENU = 0x5051,   ///< Task list enumeration; one or more Response(s) with stack frame information
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #STACKENUM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_TASKS_END = 0x5052,   ///< End of task list  enumeration
                                      ///< @li Async format    : no data


    //--- Response to UV_DBG_ENUM_MENUS:
    UV_DBG_ENUM_MENUS_START = 0x5060, ///< Start of available view enumeration
                                      ///< @li Async format    : no data
    UV_DBG_ENUM_MENUS_ENU = 0x5061,   ///< View list enumeration; one or more Response(s) with view item information
                                      ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #MENUENUM / #UVSOCK_ERROR_RESPONSE
    UV_DBG_ENUM_MENUS_END = 0x5062,   ///< End of task list  enumeration
                                      ///< @li Async format    : no data

    // Response to UV_DBG_EVTR_REGISTER
    UV_DBG_EVTR_OUTPUT = 0x5063, ///<  Notification of Event Recorder Data
                                 ///< @li Async format  : #EVTROUT

    //--- Response to UV_DBG_EVTR_ENUMSCVDFILES:
    UV_DBG_EVTR_ENUMSCVDFILES_START = 0x5064, ///< Start of available view enumeration
                                              ///< @li Async format    : no data
    UV_DBG_EVTR_ENUMSCVDFILES_ENU = 0x5065,   ///< View list enumeration; one or more Response(s) with view item information
                                              ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #SSTR / #UVSOCK_ERROR_RESPONSE
    UV_DBG_EVTR_ENUMSCVDFILES_END = 0x5066,   ///< End of task list  enumeration
                                              ///< @li Async format    : no data


    //--- Real-Time Agent:
    UV_RTA_MESSAGE = 0x6000,      ///< Notification of a Real-Time Agent message from the target
                                  ///< @li Async format    : #PRJDATA
    UV_RTA_INCOMPATIBLE = 0x6001, ///< Notification of an incompatible Real-Time Agent in the current target
                                  ///< @li Async format    : #UVSOCK_CMD_RESPONSE --> #UVSOCK_ERROR_RESPONSE

    //--- Test definititions (for testing only):
    UV_TST_1  = 0xFF00,
    UV_TST_2  = 0xFF01,
    UV_TST_3  = 0xFF02,
    UV_TST_4  = 0xFF03,
    UV_TST_5  = 0xFF04,
    UV_TST_6  = 0xFF05,
    UV_TST_7  = 0xFF06,
    UV_TST_8  = 0xFF07,
    UV_TST_9  = 0xFF08,
    UV_TST_10 = 0xFF09
} UV_OPERATION;

/** UVSOCK status codes
  *
  * UVSOCK status codes are returned in #UV_CMD_RESPONSE and #UV_ASYNC_MSG messages from
  * uVision to the client. They represent the result of the operation relating to the
  * UVSOCK command code in the same message. If the code is not @a UV_STATUS_SUCCESS, it
  * will be accompanied by an error string.
  */
typedef enum _tag_UV_STATUS {
    UV_STATUS_SUCCESS          = 0,  ///< Operation successful: No error
    UV_STATUS_FAILED           = 1,  ///< Operation failed: Generic / unknown error
    UV_STATUS_NO_PROJECT       = 2,  ///< Operation failed: No project is currently open
    UV_STATUS_WRITE_PROTECTED  = 3,  ///< Operation failed: The current project is write protected
    UV_STATUS_NO_TARGET        = 4,  ///< Operation failed: No target is selected for the current project
    UV_STATUS_NO_TOOLSET       = 5,  ///< Operation failed: No toolset is selected for the current target
    UV_STATUS_NOT_DEBUGGING    = 6,  ///< Operation failed: The debugger is not running, this operation is only possible in debug mode
    UV_STATUS_ALREADY_PRESENT  = 7,  ///< Operation failed: The group / file is already present in the current project
    UV_STATUS_INVALID_NAME     = 8,  ///< Operation failed: One of the specified group / file / project name(s) is invalid
    UV_STATUS_NOT_FOUND        = 9,  ///< Operation failed: File / group not found in the current project
    UV_STATUS_DEBUGGING        = 10, ///< Operation failed: The debugger is running, this operation is only possible when not in debug mode
    UV_STATUS_TARGET_EXECUTING = 11, ///< Operation failed: The target is executing, this operation is not possible when target is executing
    UV_STATUS_TARGET_STOPPED   = 12, ///< Operation failed: The target is stopped, this operation is not possible when target is stopped
    UV_STATUS_PARSE_ERROR      = 13, ///< Operation failed: Error parsing data in request
    UV_STATUS_OUT_OF_RANGE     = 14, ///< Operation failed: Data in request is out of range

    UV_STATUS_BP_CANCELLED    = 15, ///< Operation failed: Create new breakpoint has been cancelled
    UV_STATUS_BP_BADADDRESS   = 16, ///< Operation failed: Invalid address in create breakpoint
    UV_STATUS_BP_NOTSUPPORTED = 17, ///< Operation failed: Type of breakpoint is not supported (by target)
    UV_STATUS_BP_FAILED       = 18, ///< Operation failed: Breakpoint creation failed (syntax error, nested command etc.)
    UV_STATUS_BP_REDEFINED    = 19, ///< Breakpoint Info: A breakpoint has been redefined
    UV_STATUS_BP_DISABLED     = 20, ///< Breakpoint Info: A breakpoint has been disabled
    UV_STATUS_BP_ENABLED      = 21, ///< Breakpoint Info: A breakpoint has been enabled
    UV_STATUS_BP_CREATED      = 22, ///< Breakpoint Info: A breakpoint has been created
    UV_STATUS_BP_DELETED      = 23, ///< Breakpoint Info: A breakpoint has been deleted
    UV_STATUS_BP_NOTFOUND     = 24, ///< Operation failed: Breakpoint with @a nTickMark cookie not found.

    UV_STATUS_BUILD_OK_WARNINGS = 25, ///< Build Info: A build was successful, but with warnings
    UV_STATUS_BUILD_FAILED      = 26, ///< Build Info: A build failed with errors
    UV_STATUS_BUILD_CANCELLED   = 27, ///< Build Info: A build was cancelled

    UV_STATUS_NOT_SUPPORTED  = 28, ///< Operation failed: Requested operation is not supported
    UV_STATUS_TIMEOUT        = 29, ///< Operation failed: No response to the request occurred within the timeout period (UVSOCK Client DLL only)
    UV_STATUS_UNEXPECTED_MSG = 30, ///< Operation failed: An unexpected message type was returned (UVSOCK Client DLL only)

    UV_STATUS_VERIFY_FAILED = 31, ///< Operation failed: The code downloaded in the target differs from the current binary

    UV_STATUS_NO_ADRMAP = 32, ///< Operation failed: The specified code address does not map to a file / line
    UV_STATUS_INFO      = 33, ///< General Info: This is an information only message. It may contain warning information pertinent to a later error condition.

    UV_STATUS_NO_MEM_ACCESS  = 34, ///< Operation failed: Memory access is blocked (most likely target does not support memory access while running)
    UV_STATUS_FLASH_DOWNLOAD = 35, ///< Operation failed: The target is downloading FLASH, this operation is not possible when FLASH is downloading
    UV_STATUS_BUILDING       = 36, ///< Operation failed: A build is in progress, this operation is not possible when build is in progress
    UV_STATUS_HARDWARE       = 37, ///< Operation failed: The debugger is debugging hardware, this operation is not possible when debugging a hardware target
    UV_STATUS_SIMULATOR      = 38, ///< Operation failed: The debugger is debugging a simulation, this operation not possible when debugging a simulated target

    UV_STATUS_BUFFER_TOO_SMALL = 39, ///< Operation failed: Return buffer was too small (UVSOCK Client DLL only)

    UV_STATUS_EVTR_FAILED = 40, ///< Operation failed: Event Recorder error

    UV_STATUS_END ///<  Always at end
} UV_STATUS;


/** Variant-Types used in #TVAL
  *
  * Gives the type of the corresponding data in a #TVAL structure.
  */
typedef enum vtt_type {
    VTT_void   = 0,  ///< val.u64
    VTT_bit    = 1,  ///< val.ul & 1
    VTT_char   = 2,  ///< val.sc
    VTT_uchar  = 3,  ///< val.uc
    VTT_int    = 4,  ///< val.i
    VTT_uint   = 5,  ///< val.ul
    VTT_short  = 6,  ///< val.i16
    VTT_ushort = 7,  ///< val.u16
    VTT_long   = 8,  ///< val.l
    VTT_ulong  = 9,  ///< val.ul
    VTT_float  = 10, ///< val.f
    VTT_double = 11, ///< val.d
    VTT_ptr    = 12, ///< val.ul
    VTT_union  = 13, ///< Unused
    VTT_struct = 14, ///< Unused
    VTT_func   = 15, ///< Unused
    VTT_string = 16, ///< Unused
    VTT_enum   = 17, ///< Unused
    VTT_field  = 18, ///< Unused
    VTT_int64  = 19, ///< val.i64
    VTT_uint64 = 20, ///< val.u64
    VTT_end          ///< Always at end
} VTT_TYPE;


#pragma pack(1)

/** UVSOCK options
  *
  * Flags to configure UVSOCK
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_GEN_SET_OPTIONS
  */
typedef struct tag_UVSOCK_OPTIONS {
    DWORD bExtendedStack : 1;  ///< Extended stack mode: allows task enumeration, read/write variables and expressions
    DWORD                : 31; ///< reserved
} UVSOCK_OPTIONS;


/** Cycles and Time data
  *
  * Represents the execution time of a simulated target.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_TIME_INFO (All members are written by uVision)
  *
  */
typedef struct cycts {
    xU64   cycles; ///< Execution time in cycles
    double tStamp; ///< Execution time in seconds
} CYCTS;

/** String data
  *
  * Transfers string data between the client and uVision.
  *
  * NOTE: In some cases, the string may be longer the the statically
  * defined @a szStr string length.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_PRJ_ENUM_FILES    (Set @a nLen to the length of the NULL terminated string in @a szStr)
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_PRJ_ENUM_GROUPS_ENU  (All members are written by uVision)
  * @li #UV_PRJ_ENUM_FILES_ENU   (All members are written by uVision)
  * @li #UV_PRJ_ENUM_TARGETS_ENU (All members are written by uVision)
  * @li #UV_PRJ_GET_CUR_TARGET   (All members are written by uVision)*
  * @li #UV_PRJ_SET_OUTPUTNAME   (All members are written by uVision)
  * @li #UV_PRJ_GET_OUTPUTNAME   (All members are written by uVision)
  * @li #UV_DBG_CMD_OUTPUT       (All members are written by uVision)
  * @li #UV_DBG_EVTR_ENUMSCVDFILES_ENU (All members are written by uVision)
  */
typedef struct sstr {
    int  nLen;       ///< Length of name (including NULL terminator)
    char szStr[256]; ///< NULL terminated name string
} SSTR;

/** Virtual Register (VTR) value
  *
  * This structure contains a Virtual Register (VTR) value. The value is
  * contained in the @a v union and its type is contained in the @a vType variable.
  *
  * This is a sub-structure and is not contained in any message directly
  */
typedef struct tval {
    VTT_TYPE vType; ///< Indicates the type of data in @a v
    union {
        unsigned long  ul;  ///< #VTT_ulong
        signed char    sc;  ///< #VTT_char
        unsigned char  uc;  ///< #VTT_uchar
        signed short   i16; ///< #VTT_short
        unsigned short u16; ///< #VTT_ushort
        signed long    l;   ///< #VTT_long
        int            i;   ///< #VTT_int
        xI64           i64; ///< #VTT_int64
        xU64           u64; ///< #VTT_uint64
        float          f;   ///< #VTT_float
        double         d;   ///< #VTT_double
    } v;                    ///< Data type of this union depends on @a vType
} TVAL;

/** Virtual Register (VTR) data / Register data / Expression to calculate / Watch exprerssion to set/change
  *
  * This structure contains a Virtual Register (VTR) name and value. The value is
  * contained in @a val and its name is contained in the @a str variable.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_VTR_GET (Set @a str to the name of the VTR)
  * @li #UV_DBG_VTR_SET (Set @a str to the name of the VTR, and @a val to the value to set)
  * @li #UV_DBG_CALC_EXPRESSION (Set @a str to the expression to calculate)
  * @li #UV_DBG_REGISTER_SET (Set @a str to the expression for register value and val to register index
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_VTR_GET (uVision writes the VTR value to @a val, @a str remains unchanged)
  * @li #UV_DBG_CALC_EXPRESSION (uVision writes the result of the expression to @a val, @a str remains unchanged)
  *
  */
typedef struct vset_t {
    TVAL val; ///< Value of VTREG or register index
    SSTR str; ///< Name of VTREG or expression
} VSET;


/** Variable enumeration/ Variable data request
  *
  * <b>Extended stack mode only</b>
  *
  * Contains data indicating how a variables enumeration should be performed.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ENUM_VARIABLES ()
  *
  */
typedef struct ivarenum_t {
    int  nID;           ///< Variable/watch expression ID returned in VARINFO (1-based), set to 0 to enumerate stack root variables or globals
    int  nFrame;        ///< Stack frame ID (1- based), 0 if refers to global variables (nID == 0) or to watch expression (nID != 0)
    int  nTask;         ///< Task ID (1-based), RTX case only, otherwise should be 0;
    UINT count    : 16; ///< maximum number of variables to return
    UINT bChanged : 1;  ///< request to fill VARINFO::value field only if value has been changed from previous request
    UINT          : 15; ///< Reserved
} IVARENUM;


/** Variable set request
  *
  * <b>Extended stack mode only</b>
  *
  * Contains data indicating  enumeration should be performed.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_VARIABLE_SET ()
  *
  */
typedef struct varval_t {
    IVARENUM variable; ///< Variable to set new value
    SSTR     value;    ///< Value to set
} VARVAL;


/** Information about watch expression, variable or its member
  *
  * <b>Extended stack mode only</b>
  *
  * This structure contains a variable id, name, type, value and member count/array size
  *
  * <i>This structure is used in the following messages:</i>
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_EVAL_WATCH_EXPRESSION (uVision writes @a VARINFO result of evaluation)
  * @li #UV_DBG_ENUM_VARIABLES  (uVision writes @a VARINFO array results of enumeraton)
  *
  */
typedef struct varinfo_t {
    int  nID;             ///< Unique variable/watch expression id (1- based)
    int  index;           ///< Variable index in parent's list (0 - based)
    int  count;           ///< Array size/ struct member count  (0 - not an array or struct)
    int  typeSize;        ///< sizeof variable
    UINT bEditable  : 1;  ///< variable value can be edited
    UINT bPointer   : 1;  ///< variable is a pointer
    UINT bFunction  : 1;  ///< variable is a function pointer
    UINT bStruct    : 1;  ///< variable is a struct
    UINT bUnion     : 1;  ///< variable is a union
    UINT bClass     : 1;  ///< variable is a class
    UINT bArray     : 1;  ///< variable is an array
    UINT bEnum      : 1;  ///< variable is an enum
    UINT bAuto      : 1;  ///< variable is an auto local variable
    UINT bParam     : 1;  ///< variable is a function parameter
    UINT bStatic    : 1;  ///< variable is a local static variable
    UINT bGlobal    : 1;  ///< variable is a global variable or a member of a global variable
    UINT bValue     : 1;  ///< value field is filled
    UINT            : 12; ///< Reserved
    UINT bType      : 1;  ///< type field is filled
    UINT bName      : 1;  ///< name field is filled
    UINT bQualified : 1;  ///< qualifiedName field is filled
    UINT            : 4;  ///< Reserved
    SSTR value;           ///< Value of variable or expression
    SSTR type;            ///< Type of variable or expression
    SSTR name;            ///< Name of variable or watch expression member
    SSTR qualifiedName;   ///< Fully qualified name to acces variable or member like an expression
} VARINFO;



/** View types
  *
  * View types used by menuenum_t structure
  */
typedef enum uvviewtypes {
    UVMENU_DEBUG    = 0, ///< Static uVision debug views (Disassembly, ToolBox, Trace Data, etc.)
    UVMENU_SYS_VIEW = 1, ///< System viewer views
    UVMENU_PERI     = 2, ///< Peripheral dialogs
    UVMENU_RTX      = 3, ///< Rtx dialogs (reserved)
    UVMENU_CAN      = 4, ///< CAN dialogs (reserved)
    UVMENU_AGDI     = 5, ///< AGDI Extention dialogs (reserved)
    UVMENU_TOOLBOX  = 6, ///< ToolBox entries
    UVMENU_END           ///< Always at end
} UVMENUTYPES;


/** Menu enumeration data request
  *
  * Contains data what view types should be unumerated
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ENUM_MENUS
  * @li #UV_DBG_MENU_EXEC
   *
  */

typedef struct menuid_t {
    int menuType; ///< Type of menus to enumerate or execute: one of UVMENUTYPES constants
    int nID;      ///< Menu ID to execute
    UINT : 32;    ///< RESERVED
} MENUID;


/** View types
  *
  * Menu structure enumeration used in menuenum_t structure
  */
typedef enum menuinfotypes {
    MENU_INFO_ITEM      = 1,  ///< Individual menu item
    MENU_INFO_GROUP     = 2,  ///< Menu group
    MENU_INFO_GROUP_END = -2, ///< End of group marker
    MENU_INFO_LIST_END  = -1, ///< End of menu list
    MENU_INFO_RESERVED  = 3,  ///< reserved
    MENU_INFO_END       = 4,  ///< Always at end
} MENUENUMTYPES;


/** Menu enumeration information
  *
  * This structure contains a menu id, type, and menu label
  *
  * <i>This structure is used in the following messages:</i>
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_ENUM_MENUS (uVision writes @a MENUENUM array results of enumeraton)
  *
  */
typedef struct menuenum_t {
    int  nID;           ///< Unique menu / group ID
    int  menuType;      ///< Menu type: one of #UVMENUTYPES constants
    int  infoType;      ///< Info type: one of #MENUENUMTYPES
    UINT bEnabled : 1;  ///< Menu item is enabled
    UINT          : 31; ///< RESERVED
    SSTR menuLabel;     ///< Menu label
} MENUENUM;


/** Memory Read / Write data
  *
  * Address, length and either data to be written, or data read from memory.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_MEM_READ (Set @a nAddr to the address to read, and @a nBytes to the length of data to read)
  * @li #UV_DBG_MEM_WRITE (Set @a nAddr to the address to write, @a nBytes to the length of data to write and @a aBytes to the data to write)
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_MEM_READ (uVision sets @a aBytes to the data read from the specified location)
  * @li #UV_DBG_MEM_WRITE (uVision returns the same structure as in the request)
  *
  */
typedef struct amem {
    xU64 nAddr;     ///< Address to read / write
    UINT nBytes;    ///< Number of bytes read / write
    xU64 ErrAddr;   ///< Unused
    UINT nErr;      ///< Unused
    xUC8 aBytes[1]; ///< @a nBytes of data read or to be written
} AMEM;

/** Terminal I/O data
  *
  * Contains uVision terminal window data, and the channel the data is
  * associated with. The channel list is target dependent.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_SERIAL_PUT (Set @a nChannel to the channel to write data to, set @a itemMode to the data width, set @a nMany to the number of data items and set @a s to the data to be written)
  * @li #UV_DBG_SERIAL_GET (Set @a nChannel to the channel to read data from) - DEPRECATED use DBG_SERIAL_OUTPUT response
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_SERIAL_GET : deprecated, use DBG_SERIAL_OUTPUT response
    (uVision sets @a itemMode to the data width, @a nMany to the number of data items available, and @a s to the data. The number of items available may be 0)
  * @li #UV_DBG_SERIAL_OUTPUT (uVision sets
                               @a nChannel to the channel supplying data [0...3],
                               @a itemMode to the data width,
                               @a nMany to the number of data sent (may vary from 1 to 4096),
                               @a s to the data)
  *
  */
typedef struct serio {
    xWORD16 nChannel; ///< 0:=UART#1, 1:=UART#2, 2:=UART#3, 3:=Debug (printf) output
    xWORD16 itemMode; ///< 0:=Bytes, 1:=WORD16
    DWORD   nMany;    ///< number of items (BYTE or WORD16)
    union {
        xUC8    aBytes[1]; ///< @a nMany Bytes follow here.
        xWORD16 aWords[1]; ///< @a nMany Word16 follow here.
    } s;                   ///< @a nMany data items.
} SERIO;



/** ITM Channel booking / Output data
*
* Contains ITM Ch.0..31 channel booking and data
*
* <i>This structure is used in the following messages:</i>
*
* <b>Client ==> uVision (Request)</b>
* @li #UV_DBG_ITM_REGISTER / #UV_DBG_ITM_UNREGISTER (Set @a nChannel to the channel to book or get data from, set @a itemMode to the data width)
*
* <b>Client <== uVision (Response)</b>
* @li #UV_DBG_ITM_OUTPUT (uVision sets
@a nChannel to the channel supplying data [0...31],
@a itemMode to the data width,
@a nMany to the number of data sent (may vary from 1 Byte to 16 KB),
@a s to the data)
*
*/
typedef struct itmOut {
    xWORD16 nChannel; ///< ITM Channel number (0..31)
    xWORD16 itemMode; ///< 0:=Bytes
    DWORD   nMany;    ///< number of items (BYTE)
    union {
        xUC8    aBytes[1]; ///< @a nMany Bytes follow here.
        xWORD16 aWords[1]; ///< @a nMany Word16 follow here.
    } s;                   ///< @a nMany data items.
} ITMOUT;


/** Event Viewer register / unregister
*
* <b>Client ==> uVision (Request)</b>
* @li #UV_DBG_EVTR_REGISTER / #UV_DBG_EVTR_UNREGISTER
*
* <b>Client <== uVision (Response)</b>
*/
typedef struct evtrCmdStat {
    DWORD cmdStat; ///< Command or Status
    DWORD nMany;   ///< number of items (BYTE)
    char *data;    ///< reserved
} EVTR_CMDSTAT;


/** Multiple string data
  *
  * Contains one or more NULL terminated strings.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_PRJ_ADD_GROUP (Set @a szNames to the group name to add, and set @a nLen to the length of the group name (including terminator))
  * @li #UV_PRJ_DEL_GROUP (Set @a szNames to the group name to remove, and set @a nLen to the length of the group name (including terminator))
  * @li #UV_PRJ_SET_TARGET (Set @a szNames to the new active target, and set @a nLen to the length of the target name (including terminator))
  * @li #UV_PRJ_SET_OUTPUTNAME (Set @a szNames to the new output name, and set @a nLen to the length of the output name (including terminator))
  * @li #UV_PRJ_ADD_FILE (Set string 1 in @a szNames to the group name to add files to, set strings 2 to N to the filename(s) to add, and set @a nLen to the length of all strings (including all terminators))
  * @li #UV_PRJ_DEL_FILE (Set string 1 in @a szNames to the group name to remove files from, set strings 2 to N to the filename(s) to remove, and set @a nLen to the length of all strings (including all terminators))
  * @li #UV_PRJ_LOAD (Set @a szNames to the path and filename of the project to load, and set @a nLen to the length of the path and filename (including terminator))
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_RTA_MESSAGE (uVision writes the message to @a szNames, the message size to @a nLen, and the message routing to @a nCode. The message may be raw bytes rather than a string)
  * @li #UV_PRJ_BUILD_OUTPUT (uVision writes the line of build output to @a szNames, and the line size to @a nLen)
  * @li #UV_PRJ_BUILD_COMPLETE (uVision writes the build completion code to @a nCode)
  *
  */
typedef struct prjdat {
    UINT nLen;       ///< Length of @a szNames including NULL terminators
    UINT nCode;      ///< Informational code
    char szNames[1]; ///< Information ('string 1',0 [,'string 2',0] ... [,'string N',0])
} PRJDATA;

/** Build Completion Codes
  *
  * The build completetion codes indicate the completion status of a uVision build.
  */
typedef enum uvbuildcodes {
    UVBUILD_OK          = 1, ///< Build was Ok - no errors and warnings
    UVBUILD_OK_WARNINGS = 2, ///< Build was Ok - with warnings
    UVBUILD_ERRORS      = 3, ///< Build failed - with error(s)
    UVBUILD_CANCELLED   = 4, ///< Build was stopped/cancelled
    UVBUILD_CLEANED     = 5, ///< Project clean was completed
    UVBUILD_CODES_END        ///< Always at end
} UVBUILDCODES;

/** Breakpoint type definition
  *
  * Defines the possible types of breakpoints in uVision.
  */
typedef enum bktype {
    BRKTYPE_EXEC      = 1, ///< Execution Breakpoint
    BRKTYPE_READ      = 2, ///< Read Access Breakpoint
    BRKTYPE_WRITE     = 3, ///< Write Access Breakpoint
    BRKTYPE_READWRITE = 4, ///< ReadWrite Access Breakpoint
    BRKTYPE_COMPLEX   = 5, ///< Complex Breakpoint (Expression Breakpoint)
    BRKTYPE_END            ///< Always at end
} BKTYPE;

/** Breakpoint parameter data (new)
  *
  * Data required to create a new breakpoint.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_CREATE_BP (Set @a type to the type of breakpoint to create, set @a count to the number of occurences before breaking, set @a accSize to the data width for non-execution types (otherwise 0), set @a nExpLen to the length of the breakpoint expression, set @a nCmdLen to the length of the command to execute on hitting the breakpoint (or 0 for a non-command breakpoint) and set @a szBuffer to the breakpoint expression and the command to execute)
  *
  */
typedef struct bkparm {
    BKTYPE type;           ///< Type of breakpoint
    UINT   count;          ///< Number of occurrances before actually hit
    UINT   accSize;        ///< Access size for non-execution type breakpoints
    UINT   nExpLen;        ///< Length of breakpoint expression, including zero terminator
    UINT   nCmdLen;        ///< Length of breakpoint command, including zero terminator, or 0 if no breakpoint command is required
    char   szBuffer[1024]; ///< Breakpoint strings ('breakpoint expression',0 [,'breakpoint command',0])
} BKPARM;

/** Breakpoint parameter data (existing)
  *
  * Data representing an existing breakpoint.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_CHANGE_BP (uVision writes the breakpoint type to @a type, the occurrence count to @a count, the enable state to @a enabled, the creation timestamp to @a nTickMark, the address to @a nAddress, the expression length to @a nExpLen and the expression to @a szBuffer)
  * @li #UV_DBG_ENUMERATE_BP (uVision writes the breakpoint type to @a type, the occurrence count to @a count, the enable state to @a enabled, the creation timestamp to @a nTickMark, the address to @a nAddress, the expression length to @a nExpLen and the expression to @a szBuffer)
  *
  */
typedef struct bkrsp {
    BKTYPE type;          ///< Type of breakpoint
    UINT   count;         ///< Number of occurrances before actually hit
    UINT   enabled;       ///< 1:=Breakpoint is enabled, 0:=Breakpoint is disabled
    UINT   nTickMark;     ///< Time of breakpoint creation, used to identify individual breakpoints
    xU64   nAddress;      ///< Breakpoint address
    UINT   nExpLen;       ///< Length of breakpoint expression, including zero terminator
    char   szBuffer[512]; ///< Breakpoint expression ('breakpoint expression',0)
} BKRSP;

/** Breakpoint change operation definition
  *
  * Indicates the type of change to make to a breakpoint in the #BKCHG structure
  */
typedef enum chg_type {
    CHG_KILLBP    = 1, ///< Delete breakpoint
    CHG_ENABLEBP  = 2, ///< Enable breakpoint
    CHG_DISABLEBP = 3, ///< Disable breakpoint
    CHG_END            ///< Always at end
} CHG_TYPE;

/** Breakpoint change data
  *
  * Data indentifying an operation to be applied to an individual breakpoint.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_CHANGE_BP (Set @a type to the operation to perform and set @a nTickMark to the value returned by the #UV_DBG_ENUMERATE_BP command)
  *
  */
typedef struct bkchg {
    CHG_TYPE type;      ///< Type of operation to perform on the breakpoint
    UINT     nTickMark; ///< Timestamp of breakpoint as returned by #UV_DBG_ENUMERATE_BP
    UINT     nRes[8];   ///< Reserved (set to 0)
} BKCHG;

/** Project option type definition
  *
  * Identifies the type of uVision project option to get or set in the #UV_PRJ_GET_OPTITEM and #UV_PRJ_SET_OPTITEM commands
  */
typedef enum optsel {
    OPT_LMISC        = 1,  ///< Link-Misc string (valid for Target only)
    OPT_CMISC        = 2,  ///< C-Misc string
    OPT_AMISC        = 3,  ///< Asm-Misc string
    OPT_CINCL        = 4,  ///< C-Include-path string
    OPT_AINCL        = 5,  ///< Asm-Include-path string
    OPT_CDEF         = 6,  ///< C-Defines string
    OPT_ADEF         = 7,  ///< Asm-Defines string
    OPT_CUNDEF       = 8,  ///< C-Undefines (valid for Group- and File-level only)
    OPT_AUNDEF       = 9,  ///< Unused: Asm-Undefines, non-existant in uVision
    OPT_COPTIMIZE    = 10, ///< C optimization options [Level, Time]
    OPT_CODEGEN      = 11, ///< ARM/Thumb mode for ARM7TDMI targets
    OPT_MEMRANGES    = 12, ///< Memory ranges available to be assigned
    OPT_ASNMEMRANGES = 13, ///< Assigned memory ranges at specified level
    OPT_UBCOMP1      = 14, ///< Run User Programs Before Compilation of a C/C++ File 1
    OPT_UBCOMP2      = 15, ///< Run User Programs Before Compilation of a C/C++ File 2
    OPT_UBBUILD1     = 16, ///< Run User Programs Before Build / Rebuild 1 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
    OPT_UBBUILD2     = 17, ///< Run User Programs Before Build / Rebuild 2 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
    OPT_UABUILD1     = 18, ///< Run User Programs After  Build / Rebuild 1 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
    OPT_UABUILD2     = 19, ///< Run User Programs After  Build / Rebuild 2 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
    OPT_UBEEP        = 20, ///< Beep When Complete \n Format: 'beep' or empty string
    OPT_USTARTDEB    = 21, ///< Start Debugging \n Format: 'start debug' or empty string
    OPT_END                ///< Always at end
} OPTSEL;

/** Project option data
  *
  * Structure containing the project option level and data information
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_PRJ_GET_OPTITEM (Set @a job to the type of option to get, @a iTarg, @a iGroup and @a iFile to the start positions of target name, group name and file name within @a szBuffer)
  * @li #UV_PRJ_SET_OPTITEM (Set @a job to the type of option to set, @a iTarg, @a iGroup, @a iFile and @a iItem to the start positions of target name, group name, file name and item data string within @a szBuffer)
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_PRJ_GET_OPTITEM (uVision sets @a iItem to the index of the item data in @a szBuffer, all other data is returned as sent)
  *
  * <b>'Item data string' Format</b>
  * In the case of @a job being #OPT_MEMRANGES, 'Item data string' is replaced by the #UV_MEMINFO structure.
  * @li #OPT_LMISC (Free format string)
  * @li #OPT_CMISC (Free format string)
  * @li #OPT_AMISC (Free format string)
  * @li #OPT_CINCL (Free format string)
  * @li #OPT_AINCL (Free format string)
  * @li #OPT_CDEF (Free format string)
  * @li #OPT_ADEF (Free format string)
  * @li #OPT_CUNDEF (Free format string)
  * @li #OPT_AUNDEF (Unused)
  * @li #OPT_COPTIMIZE (Optimize (optimization level (target specific)), OptTime (0:= Optimize for size, 1:= Optimize for time))
  * @li #OPT_CODEGEN (Codegen ['arm':= Generate ARM code, 'thumb':= Generate Thumb code])
  * @li #OPT_MEMRANGES (#UV_MEMINFO structure)
  * @li #OPT_ASNMEMRANGES (Code/Const (index of ROM region), ZiData (index of Zero-Initialized Data region), OtherData (index of Other Data region)). NOTE: Indexes are derived from memory region position in the #UV_MEMINFO structure returned by #UV_PRJ_GET_OPTITEM:#OPT_MEMRANGES)
  * @li #OPT_UBCOMP1   Run User Programs Before Compilation of a C/C++ File 1
  * @li #OPT_UBCOMP2   Run User Programs Before Compilation of a C/C++ File 2
  * @li #OPT_UBBUILD1  Run User Programs Before Build / Rebuild 1 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
  * @li #OPT_UBBUILD2  Run User Programs Before Build / Rebuild 2 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
  * @li #OPT_UABUILD1  Run User Programs After  Build / Rebuild 1 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
  * @li #OPT_UABUILD2  Run User Programs After  Build / Rebuild 2 \n Format: bRUN bDOS16 PATH\n i.e.: 10c:\\Execute\\MyProgram.exe
  * @li #OPT_UBEEP     Beep When Complete \n Format: 'beep' or empty string
  * @li #OPT_USTARTDEB Start Debugging \n Format: 'start debug' or empty string
  */
typedef struct trnopt {
    OPTSEL job;         ///< Project item type
    UINT   iTarg;       ///< 'Target name' starts at &szBuffer[iTarg]
    UINT   iGroup;      ///< 'Group name' starts at &szBuffer[iGroup]
    UINT   iFile;       ///< (no-path) 'File name' starts at &szBuffer[iFile]
    UINT   iItem;       ///< 'Item data string' starts at &szBuffer[iItem]
    char   szBuffer[3]; ///< Option location and data (0, 'Target name',0 [,'Group name',0 [,'FileName',0]] [,'Item data string',0])
} TRNOPT;

/** Memory range type definition
  *
  * Indicates the type of memory range defined in a #UV_MRANGE structure.
  */
typedef enum uv_mr {
    UV_MR_NONE = 0, ///< Memory range is not set within uVision
    UV_MR_ROM  = 1, ///< Memory range is ROM
    UV_MR_RAM  = 2, ///< Memory range is RAM
    UV_MR_END       ///< Always at end
} UV_MR;

/** Individual memory range data
  *
  * Indicates how a memory range is configured.
  *
  * This is a sub-structure and is not contained in any message directly.
  *
  */
typedef struct uv_mrange {
    DWORD mType   : 8;  ///< Memory type (#UV_MR_NONE / #UV_MR_ROM / #UV_MR_RAM)
    DWORD dfltRam : 1;  ///< 1:=default RAM range
    DWORD dfltRom : 1;  ///< 1:=default ROM range
    DWORD isZiRam : 1;  ///< 1:=RAM range zero-initialized
    DWORD         : 21; ///< Reserved
    DWORD nRes[3];      ///< Reserved

    xU64 nStart; ///< Memory range start address
    xU64 nSize;  ///< Memory range size
} UV_MRANGE;

/** Memory ranges data
  *
  * List of memory ranges set within the uVision project.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_PRJ_GET_OPTITEM (uVision sets 'Item data string' of the #TRNOPT structure to #UV_MEMINFO. Within #UV_MEMINFO uVision sets @a nRanges to the number of memory ranges and @a mr to the actual memory ranges data)
  *
  */
typedef struct uv_minfo {
    UINT      nRanges; ///< Number of UV_MRANGE memory ranges pointed to by @a &mr
    UINT      nRes[8]; ///< Reserved
    UV_MRANGE mr;      ///< Zero or more memory ranges
} UV_MEMINFO;

/** Licensing data
  *
  * Indicates which uVision modules are licensed. Currently implemented for ARM RealView MDK only.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_GEN_CHECK_LICENSE (uVision sets @a rvmdk)
  *
  */
typedef struct uvlicinfo {
    DWORD rvmdk : 1;  ///< 1:=RV-MDK is licensed, 0:=RV-MDK is not licensed
    DWORD       : 31; ///< Reserved
    DWORD nRes[10];   ///< Reserved
} UVLICINFO;

/** Debug target type definition
  *
  * Indicates how uVision will debug the target when debug mode is entered.
  */
typedef enum uv_target {
    UV_TARGET_HW  = 0, ///< Target will be debugged using the specified hardware debugger
    UV_TARGET_SIM = 1, ///< Target will be debugged using the uVision simulator
    UV_TARGET_END      ///< Always at end
} UV_TARGET;

/** Target debugging data
  *
  * Indicates how the target will be debugged when debug mode is entered.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_PRJ_SET_DEBUG_TARGET (Set @a target to the type of debugging to perform)
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_PRJ_GET_DEBUG_TARGET (uVision sets @a target to the type of debugging that will be performed)
  *
  */
typedef struct dbgtgtopt {
    DWORD target : 1;  ///< Target type (#UV_TARGET_HW:=Target is hardware, #UV_TARGET_SIM:=Target is simulator)
    DWORD        : 31; ///< Reserved
    DWORD nRes[10];    ///< Reserved
} DBGTGTOPT;

/** uVision Progress bar control type definition
  *
  * Indicates the type of uVision progress bar operation to perform.
  */
typedef enum pgcmd {
    UV_PROGRESS_INIT    = 1, ///< Initialize progress bar with optional label (in %-mode)
    UV_PROGRESS_SETPOS  = 2, ///< Set progress bar percentage (0...100)
    UV_PROGRESS_CLOSE   = 3, ///< Close the progress bar
    UV_PROGRESS_INITTXT = 4, ///< Initialize progress bar (in text mode)
    UV_PROGRESS_SETTEXT = 5, ///< Set inside bar text in text mode
    UV_PROGRESS_END          ///< Always at end
} PGCMD;

/** uVision Progress bar control data
  *
  * Indicates how the uVision progress bar should be updated
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_PRJ_CMD_PROGRESS (Set @a job to the type of progress bar operation to perform, @a perc to the percentage complete (#UV_PROGRESS_SETPOS only), and @a szLabel to the progress bar label (#UV_PROGRESS_INIT / #UV_PROGRESS_INITTXT / #UV_PROGRESS_SETTEXT only))
  *
  */
typedef struct pgress {
    PGCMD job;        ///< PGCMD command
    UINT  perc;       ///< Percentage completed (Used on #UV_PROGRESS_SETPOS only)
    UINT  nRes[8];    ///< Reserved
    char  szLabel[1]; ///< Progress label (0 or labelname on #UV_PROGRESS_INIT / #UV_PROGRESS_INITTXT / #UV_PROGRESS_SETTEXT)
} PGRESS;

/** Extended version data
  *
  * Contains the uVision extended version information in ASCII string format.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_GEN_GET_EXTVERSION (uVision sets @a iV_Uv3 and @a iV_Sock to the version string indexes, and writes the version strings to @a szBuffer)
  *
  */
typedef struct extvers {
    UINT iV_Uv3;      ///< 'UV3 Version' starts at &szBuffer [iV_Uv3V]
    UINT iV_Sock;     ///< 'UVSOCK Version' starts at &szBuffer [iV_Sock]
    UINT nRes[30];    ///< Reserved (for extra version information)
    char szBuffer[3]; ///< Version strings (0, 'uVision=Vx.y',0, 'UVSOCK=Vx.y',0, [Reserved] ,0)
} EXTVERS;

/** Symbol enumeration type definition
  *
  * Identifies the type of symbol enumeration to perform.
  */
typedef enum enTpJob {
    UV_TPENUM_MEMBERS = 1, ///< Enumerate symbols' structure members: name, offset, size
    UV_TPENUM_END          ///< Always at end
} ENTPJOB;

/** Symbol enumeration data
  *
  * Contains symbol enumeration data.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ENUM_SYMTP (Set @a Job to the type of operation to perform, and @a szID to the 'Symbol' to perform the operation on (e.g. '\\Measure\\current.time'))
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_ENUM_SYMTP_ENU (uVision writes @a nOffs to the member offset within the type (bytes), @a nSize to the member size (bytes) and @a szID to the 'Member name')
  *
  */
typedef struct enumtpm {
    ENTPJOB Job;       ///< Type of symbol enumeration to perform
    UINT    nOffs;     ///< Member Offset within type
    UINT    nSize;     ///< Member Size
    UINT    nRes[8];   ///< Reserved
    char    szID[512]; ///< 'Symbol' / 'Member name'
} ENUMTPM;

/** Map address to file / line request data
  *
  * Contains request information for mapping a code address to a file and line number.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ADR_TOFILELINE (Set @a bFull to 1 to retrieve the file as an absolute path, or 0 to retrieve as a relative path, and set @a nAdr to the address to map)
  *
  */
typedef struct adrmtfl {
    UINT bFull : 1;  ///< 1:=want full path, 0:=want relative path
    UINT       : 31; ///< Reserved
    xU64 nAdr;       ///< Address to map
    UINT nRes[7];    ///< Reserved
} ADRMTFL;

/** Map address to file / line return data
  *
  * Contains information for a code address to a file and line number mapping.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_ADR_TOFILELINE (uVision writes @a nLine to the line number, @a nAdr to the code address requested, @a iFile and @a iFunc to the 'function' and 'filename' indexes within @a szFile, and @a szFile to the 'filename' and 'function')
  *
  */
typedef struct aflmap {
    UINT nLine;     ///< Code line number
    xU64 nAdr;      ///< Code address
    UINT iFile;     ///< Index of 'filename' (0 if none)
    UINT iFunc;     ///< Index of 'function' (0 if none)
    int  nRes[5];   ///< Reserved
    char szFile[1]; ///< Filename and function (0 [,'filename',0 [,'function',0]])
} AFLMAP;

/** Stop reason definition
  *
  * Indicates the reason that target / simulation execution has stopped.
  */
typedef enum stopreason {
    STOPREASON_UNDEFINED = 0x0000, ///< Unknown / undefined stop reason
    STOPREASON_EXEC      = 0x0001, ///< Hit execution breakpoint
    STOPREASON_READ      = 0x0002, ///< Hit read access breakpoint
    STOPREASON_HIT_WRITE = 0x0004, ///< Hit write access breakpoint
    STOPREASON_HIT_COND  = 0x0008, ///< Hit conditional breakpoint
    STOPREASON_HIT_ESC   = 0x0010, ///< ESCape key has been pressed
    STOPREASON_HIT_VIOLA = 0x0020, ///< Memory access violation occurred (simulator only)
    STOPREASON_TIME_OVER = 0x0040, ///< Interval time set by #UV_DBG_SET_CALLBACK or #UV_DBG_WAKE elapsed
    STOPREASON_UNDEFINS  = 0x0080, ///< Undefined instruction occurred
    STOPREASON_PABT      = 0x0100, ///< (Instruction) prefetch abort occurred
    STOPREASON_DABT      = 0x0200, ///< Data abort occurred
    STOPREASON_NONALIGN  = 0x0400, ///< Non-aligned access occurred (simulator only)
    STOPREASON_END                 ///< Always at end
} STOPREASON;

/** Breakpoint reason data
  *
  * Contains information on the reason why the target / simulation has stopped
  * executing, and the current state of the processor.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_STOP_EXECUTION (uVision writes @a eReason to the reason for stopping execution, @a nPC to the current program counter value, @a nAdr to the break reason address, @a nBpNum to the number of the breakpoint that was hit, and @a nTickMark to the creation timestamp of the breakpoint that was hit)
  *
  */
typedef struct bpreason {
    UINT       nRes1;     ///< Reserved
    UINT       nRes2;     ///< Reserved
    UINT       StrLen;    ///< Unused
    STOPREASON eReason;   ///< Reason for stopping execution
    xU64       nPC;       ///< Address of PC when stopped
    xU64       nAdr;      ///< Address of break reason (i.e. memory access address, or breakpoint address)
    int        nBpNum;    ///< Breakpoint number (-1:=undefined)
    UINT       nTickMark; ///< Time of breakpoint creation, used to identify individual breakpoints (0 if @a nBpNum is undefined)
    UINT       nRes[4];   ///< Reserved
} BPREASON;

/** Stack enumeration request data
  *
  * Contains data indicating how a stack enumeration should be performed.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ENUM_STACK (Set all items to 0)
  *
  */
typedef struct istkenum {
    UINT bFull     : 1;  ///< Unused, kept for backward compatibility
    UINT bExtended : 1;  ///< Get extended information:  nVars, nTotal, iTask (see STACKENUM) <b>Extended stack mode only</b>
    UINT bModified : 1;  ///< Enumerate only modified frames <b>Extended stack mode only</b>
    UINT           : 29; ///< Reserved
    UINT nTask;          ///< Task ID: reserved for RTX case, otherwise ignored
    UINT nRes[6];        ///< Reserved
} iSTKENUM;

/** Stack enumeration return data
  *
  * Contains a stack enumeration item.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_ENUM_STACK_ENU (uVision writes @a nItem to the stack frame number, @a nAdr to the callee address,  @a nRetAdr to the caller address. The #UV_DBG_ADR_TOFILELINE command can be used to convert the addresses to file, function and line numbers)
  *
  */
typedef struct stackenum {
    UINT nItem;   ///< Stack frame number 1...n (if iSTKENUM::bExtended is specified, the stacks are reported from bottom to top)
    xU64 nAdr;    ///< Current address (callee address)
    xU64 nRetAdr; ///< Return address (caller address)
    UINT nVars;   ///< Number of stack variables
    UINT nEqual;  ///< Number of frames not mofified, important when only mofified stack frames are enumerated
    UINT nTotal;  ///< Total number of stack frames, important when only mofified stack frames are enumerated
    UINT nTask;   ///< Task ID this frame belongs to: reserved for RTX case, otherwise 0
    UINT nRes[3]; ///< Reserved
} STACKENUM;


/** Task list enumeration return data
  *
  * Contains a stack enumeration item.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_ENUM_TASKS_ENU (uVision writes @a nItem to the stack frame number, @a nAdr to the callee address,  @a nRetAdr to the caller address. The #UV_DBG_ADR_TOFILELINE command can be used to convert the addresses to file, function and line numbers)
  *
  */
typedef struct taskenum {
    int  nTask;       ///< Task ID (1-based for RTX), 0 corresponds to implicit main thread
    xU64 nAdr;        ///< Task entry address
    UINT nState : 8;  ///< Task state
    UINT        : 24; ///< Reserved
    SSTR name;        // task name
} TASKENUM;


/** Register enumeration return data
  *
  * Contains a register enumeration item.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_ENUM_REGISTERS_ENU (uVision writes all values)
  *
  */

typedef struct rEnumItem { // Register enumeration Item Descriptor
    xWORD16 nGi;           // Group-Index (0...nGroups-1)
    xWORD16 nItem;         // Item indicator (type)
    char    szReg[16];     // Name of Register
    xUC8    isPC   : 1;    // is this the PC
    xUC8    canChg : 1;    // can this Reg be changed
    xUC8           : 6;    // reserved
    char szVal[32];        // it's value in Ascii
} REGENUM;


/** Virtual register (VTR) enumeration request data
  *
  * Contains data indicating how a VTR enumeration should be performed.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ENUM_VTR (Set #bValue to 1 to retreive the VTR values as well as the list of VTRs)
  *
  */
typedef struct ivtrenum {
    UINT bValue : 1;  ///< Retreive VTR value
    UINT        : 31; ///< Reserved
    UINT nRes[7];     ///< Reserved
} iVTRENUM;

/** Virtual register (VTR) enumeration return data
  *
  * Contains a VTR enumeration item.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_ENUM_VTR_ENU (uVision writes @a bValue to the value from the request, @a vtrType, @a vtrFrq, @a vtrClock and @a vtrNo to the type of VTR, @a value to the value of the VTR if requested, and @a szName to the name of the VTR which can be used to read or modify it via the #UV_DBG_CALC_EXPRESSION command)
  *
  */
typedef struct avtr {
    UINT bValue   : 1;  ///< 1:=Client requested VTR value
    UINT vtrType  : 8;  ///< VTR type (VTT_TYPE)
    UINT vtrFrq   : 1;  ///< 1:=VTR is of type 'XTAL'
    UINT vtrClock : 1;  ///< 1:=VTR is of type 'CLOCK'
    UINT vtrNo    : 16; ///< VTR internal number
    UINT          : 5;  ///< Reserved

    UINT nRes[7];   ///< Reserved
    TVAL value;     ///< VTR value
    char szName[1]; ///< VTR name
} AVTR;

/** Wake interval data
  *
  * Contains data indicating how to set a wake interval time.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_WAKE (Set @a bAutoStart to 1 to start the target automatically when it receives this command, set @a bSetInterval to 1 if you would like the simulation to go to sleep after the specified wake interval (if 0 the wake interval time is ignored), set @a bCycles to 1 if wake interval is specified in cycles, or 0 if it is specified in seconds, and set @a fSeconds or @a iCycles to the wake interval time, based on the value of @a bCycles)
  *
  */
typedef struct iInterval {
    UINT bAutoStart   : 1;  ///< 1:=start the target if it is not running, 0:=do not start the target
    UINT bCycles      : 1;  ///< 1:=interval is in cycles, 0:=interval is in seconds
    UINT bSetInterval : 1;  ///< 1:=set the callback interval in this message, 0:=don't set a callback
    UINT              : 29; ///< Reserved
    float fSeconds;         ///< Wake interval in seconds (if bCycles:=0)
    xI64  iCycles;          ///< Wake interval in cycles (if bCycles:=1)
    UINT  nRes[7];          ///< Reserved
} iINTERVAL;

/** Show code in uVision request data
  *
  * Contains data indicating how uVision should display code for the requested
  * address.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_ADR_SHOWCODE (Set @a nAdr to the address to show code for, @a bAsm to 1 to display the assembly code, and @a bHll to show the high level language code (if possible))
  *
  */
typedef struct iShowSync {
    xU64 nAdr;         ///< Address of code to show
    UINT bAsm    : 1;  ///< 1:=show disassembly
    UINT bHll    : 1;  ///< 2:=show high level language code
    UINT bAsmRes : 1;  ///< Unused
    UINT bHllRes : 1;  ///< Unused
    UINT         : 28; ///< Reserved
    UINT nRes[7];      ///< Reserved
} iSHOWSYNC;


/** Timestamp for PowerScale
  *
  * Contains the time for which to show:
  * - disassembly/HLL and trace entries in uVision.
  * - power measurement data in PowerScale.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_POWERSCALE_SHOWCODE
  * @n Set @ref ticks and @ref delta for which to show code and trace entries in uVision.
  *
  * <b>Client <== uVision (Response)</b>
  * @li #UV_DBG_POWERSCALE_SHOWCODE
  * @n If successful, uVision returns the absolute @ref time (seconds) and the instruction address
  @ref nAdr for the displayed code (HLL, ASM) and trace entries.
  *
  * <b>uVision ==> Client (Request)</b>
  * @li #UV_DBG_POWERSCALE_SHOWPOWER
  * @n @ref ticks and @ref delta contain time information for which to show the measured power. @ref time (sec) is the absolute value
  from uVision.
  *
  */
typedef struct iUVSC_PSTAMP {
    xU64   nAdr;               ///< Address of code shown in uVision (for testing purposes).
    INT64  ticks;              ///< ULINKpro Isolation Adapter ticks since last RESET.
    double delta;              ///< Time difference (sec) between \ref ticks and the measurement timestamp.
    double time;               ///< Absolute time value (sec) in uVision of a \ref ticks and \ref delta pair.
    UINT   showSyncErr   : 1;  ///< Error message box shown in uVision if synchronization fails and showSyncErr is '1'.
    UINT   nReservedBits : 31; ///< Reserved.
    UINT   nRes[6];            ///< Reserved.
} UVSC_PSTAMP;


/** Event Recorder
  *
  * Contains data from Event Recorder
  * - foo
  * - bar
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client <== uVision (Async)</b>
  * @li #UV_DBG_EVTR_OUTPUT (uVision sends Event Recorder Records)
  *
  */

typedef struct raw_event { // Raw-data evtrecorder_item
    UINT32 nType : 8;      // 1:=raw format (other values are undefined)
    UINT32 nRes  : 24;     // unused - zero - for later extensions

    UINT64 EvNumber; // consecutive event number
    UINT64 tStamp;   // Timestamp

    UINT32 restartID; // Restart ID
    UINT16 id;        // Event ID
    UINT16 nValues;   // number of values in payload

    unsigned char idx;        // Index if multiple records for one event
    unsigned char ctx;        // Event Context (to ID multiple events with same timestamp)
    unsigned char recorderID; // Recorder ID

    unsigned char overflowL : 1; // First record after recovery from overflow(LowBit) 0: HostBufferOverflow, 1: TargetOverflow
    unsigned char reset     : 1; // First record after a reset
    unsigned char validID   : 1; // id field carries a valid value (e.g. if record with ID not read yet)
    unsigned char last      : 1; // Last Record in Event
    unsigned char complete  : 1; // Record is complete (all indexes between 0 and last item)
    unsigned char overflowH : 1; // First record after recovery from overflow(HighBit) LowBit == 0 => 0: NoOverflow, 1: HostBufferOverflow
                                 //                                                    LowBit == 1 => 0: TargetOverflow, 1: Missed Event
    unsigned char irq    : 1;    // IRQ Flag
    unsigned char resume : 1;    // Resume

    UINT32 payload[2]; // actually [nValues] payload values
} RAW_EVENT;


typedef struct dec_event { // Decoded event-recorder event item
    UINT32 nType : 8;      // 1:=raw format (other values are undefined)
    UINT32 nRes  : 24;     // could use to transfer alert,bold...
    UINT64 EvNumber;       // consecutive event number (256*4G should be enaugh...)
    UINT64 tStamp;         // Timestamp
    char   szText[1];      // actually szText[nSize] characters
} DEC_EVENT;

#define EVFMT_RAW 1
#define EVFMT_DEC 2

typedef struct evtrPack {
    unsigned short nMany;

    union {
        char      data;
        char      nType;
        RAW_EVENT raw;
        DEC_EVENT dec;
    };
} EVTR_PACK;

typedef struct evtrout_item {
    UINT32 nMany; // byte size of data item
    DWORD  nRes;  // reserved

    union {
        char      data;
        EVTR_PACK pack;
    };
} EVTROUT;


/** Command execution request / response data
  *
  * Contains data indicating how and which command uVision should execute.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_DBG_EXEC_CMD (Set @a bEcho to 1 to echo the command and result in the uVision Command Window, or 0 for no echo. Set @a sCmd to the command string to execute)
  *
  */
typedef struct execCmd {
    UINT bEcho : 1;  ///< 1:=echo command and response in uVision Command Window, 0:=no echo
    UINT       : 31; ///< Reserved
    UINT nRes[7];    ///< Reserved
    SSTR sCmd;       ///< Command to execute
} EXECCMD;


/** Path request data structure
  *
  * Contains data indicating how a path retreival should be performed.
  *
  * <i>This structure is used in the following messages:</i>
  *
  * <b>Client ==> uVision (Request)</b>
  * @li #UV_PRJ_GET_OUTPUTNAME (Set all reserved items to 0)
  * @li #UV_PRJ_GET_CUR_TARGET (Set to 0 for now, unused)
  *
  */
typedef struct ipathreq {
    UINT bFull : 1;  ///< 1:=want full path(s), 0:=want relative path(s)
    UINT       : 31; ///< Reserved
    UINT nRes[7];    ///< Reserved
} iPATHREQ;


#pragma pack(1) // ()

/** Error response data
  *
  * Contains error information. The union in #UVSOCK_CMD_RESPONSE will correspond
  * to this structure if the status code within is anything other than
  * #UV_STATUS_SUCCESS or if within a #UV_ASYNC_MSG.
  *
  * Contained in #UVSOCK_CMD_RESPONSE if an error occurred.
  *
  */
typedef struct _tag_UVSOCK_ERROR_RESPONSE {
    UINT nRes1;                ///< Reserved
    UINT nRes2;                ///< Reserved
    UINT StrLen;               ///< Length of error string (including terminator) in bytes
    BYTE str[SOCK_NDATA - 20]; ///< Error description
} UVSOCK_ERROR_RESPONSE;


/** UVSOCK Command response / async message data format
  *
  * Contains command response data, or async message data. @a cmd indicates either
  * the command to which this is a response, or the asynchronous message type.
  * @a status indicates is the command or asynchronous operation was successful.
  * The value of the union depends on the message type, and whether an error occurred.
  * If status is not #UV_STATUS_SUCCESS, or it within a #UV_ASYNC_MSG packet, then the union will be of type @a err.
  * Otherwise it will be of the type corresponding to the message type in @a cmd.
  *
  * This structure is included in all messages that correspond to the new message
  * response format.
  *
  * Contained in #UVSOCK_CMD_DATA if the message type (@a m_eCmd) is #UV_CMD_RESPONSE or #UV_ASYNC_MSG
  * or #UV_DBG_CHANGE_BP or #UV_DBG_BP_ENUMERATED or #UV_PRJ_ENUM_GROUPS_ENU or #
  * or #UV_DBG_ENUM_SYMTP_ENU or #UV_DBG_ENUM_VTR_ENU or #UV_PRJ_ENUM_TARGETS_ENU
  *
  */
typedef struct _tag_UVSOCK_CMD_RESPONSE {
    UV_OPERATION cmd;    ///< Command or asynchronous operation to which this is a response
    UV_STATUS    status; ///< Status code indicating if the command was successful or not
    union {
        UVSOCK_ERROR_RESPONSE err;            ///< Returned if status is not #UV_STATUS_SUCCESS or if from #UV_ASYNC_MSG
        UINT                  nVal;           ///< Returned by #UV_PRJ_ACTIVE_FILES / #UV_GEN_GET_VERSION / #UV_DBG_STATUS
        CYCTS                 time;           ///< Returned by #UV_DBG_TIME_INFO
        AMEM                  amem;           ///< Returned by #UV_DBG_MEM_READ / #UV_DBG_MEM_WRITE / #UV_DBG_DSM_READ
        SERIO                 serdat;         ///< Returned by #UV_DBG_SERIAL_OUTPUT / #UV_DBG_SERIAL_GET
        ITMOUT                itmdat;         ///< Returned by #UV_DBG_ITM_OUTPUT
        EVTROUT               evtrOut;        ///< Returned by #UV_DBG_EVTR_OUTPUT
        VSET                  vset;           ///< Returned by #UV_DBG_VTR_GET / #UV_DBG_CALC_EXPRESSION
        BKRSP                 brk;            ///< Returned by #UV_DBG_BP_ENUMERATED / #UV_DBG_CHANGE_BP
        TRNOPT                trnopt;         ///< Returned by #UV_PRJ_GET_OPTITEM
        SSTR                  str;            ///< Returned by #UV_PRJ_ENUM_GROUPS_ENU / #UV_PRJ_ENUM_FILES_ENU / #UV_PRJ_ENUM_TARGETS_ENU / #UV_PRJ_GET_CUR_TARGET / #UV_PRJ_GET_OUTPUTNAME/ #UV_PRJ_SET_OUTPUTNAME / #UV_DBG_EVTR_ENUMSCVDFILES_ENU
        EXTVERS               evers;          ///< Returned by #UV_GEN_GET_EXTVERSION
        ENUMTPM               tpm;            ///< Returned by #UV_DBG_ENUM_SYMTP_ENU
        AFLMAP                aflm;           ///< Returned by #UV_DBG_ADR_TOFILELINE
        BPREASON              StopR;          ///< Returned by #UV_DBG_STOP_EXECUTION
        STACKENUM             stack;          ///< Returned by #UV_DBG_ENUM_STACK_ENU
        TASKENUM              task;           ///< Returned by #UV_DBG_ENUM_TASKS
        AVTR                  vtr;            ///< Returned by #UV_DBG_ENUM_VTR_ENU
        UVLICINFO             licinfo;        ///< Returned by #UV_GEN_CHECK_LICENSE
        DBGTGTOPT             dbgtgtopt;      ///< Returned by #UV_PRJ_GET_DEBUG_TARGET
        UVSC_PSTAMP           powerScaleData; ///< Returned by #UV_DBG_POWERSCALE_SHOWCODE / #UV_DBG_POWERSCALE_SHOWPOWER
        REGENUM               regEnum;        ///< Returned by #UV_DBG_ENUM_REGISTERS
        VARINFO               varInfo;        ///< Returned by #UV_DBG_EVAL_WATCH_EXPRESSION, #UV_DBG_ENUM_VARIABLES
        MENUENUM              viewInfo;       ///< Returned by #UV_DBG_ENUM_MENUS_ENU
        char                  strbuf[1];      ///< Returned by #UV_DBG_READ_REGISTERS
    };
} UVSOCK_CMD_RESPONSE;


/** UVSOCK message data format
  *
  * Contains message data. All UVSOCK message data is in this format.
  *
  * <b>Client ==> uVision (Request)</b>
  * Data may be either of zero length, or one of the types indicated.
  *
  * <b>Client <== uVision (Response)</b>
  * Data is of @a cmdRsp format
  *
  * <b>Client <== uVision (Async)</b>
  * New:    Data is of @a cmdRsp format
  * Legacy: Data may be either of zero length, or one of the types indicated.
  *
  */
typedef union _tag_UVSOCK_CMD_DATA {
    BYTE raw[SOCK_NDATA]; ///< Command-dependent raw data

    // Request message, and / or legacy format asynchronous data
    PRJDATA        prjdata;        ///< Sent in #UV_PRJ_LOAD / #UV_PRJ_ADD_GROUP / #UV_PRJ_SET_TARGET / #UV_PRJ_ADD_FILE / #UV_PRJ_DEL_GROUP / #UV_PRJ_DEL_FILE / #UV_PRJ_SET_OUTPUTNAME. Returned by #UV_PRJ_BUILD_OUTPUT / #UV_PRJ_BUILD_COMPLETE / #UV_RTA_MESSAGE
    AMEM           amem;           ///< Sent in #UV_DBG_MEM_READ / #UV_DBG_MEM_WRITE / #UV_DBG_DSM_READ
    SERIO          serdat;         ///< Sent in #UV_DBG_SERIAL_GET / #UV_DBG_SERIAL_PUT
    ITMOUT         itmdat;         ///< Sent in #UV_DBG_ITM_REGISTER / #UV_DBG_ITM_UNREGISTER / #UV_DBG_ITM_OUTPUT
    EVTROUT        evtrOut;        ///< Sent in #UV_DBG_EVTR_OUTPUT
    VSET           vset;           ///< Sent in #UV_DBG_VTR_GET / #UV_DBG_VTR_SET / #UV_DBG_CALC_EXPRESSION
    TRNOPT         trnopt;         ///< Sent in #UV_PRJ_GET_OPTITEM / #UV_PRJ_SET_OPTITEM
    SSTR           sstr;           ///< Sent in #UV_PRJ_ENUM_FILES. Returned by #UV_DBG_CMD_OUTPUT
    BKPARM         bkparm;         ///< Sent in #UV_DBG_CREATE_BP
    BKCHG          bkchg;          ///< Sent in #UV_DBG_CHANGE_BP
    DBGTGTOPT      dbgtgtopt;      ///< Sent in #UV_PRJ_SET_DEBUG_TARGET
    ADRMTFL        adrmtfl;        ///< Sent in #UV_DBG_ADR_TOFILELINE
    iSHOWSYNC      ishowsync;      ///< Sent in #UV_DBG_ADR_SHOWCODE
    iVTRENUM       ivtrenum;       ///< Sent in #UV_DBG_ENUM_VTR
    EXECCMD        execcmd;        ///< Sent in #UV_DBG_EXEC_CMD
    iPATHREQ       iPathReq;       ///< Sent in #UV_PRJ_GET_OUTPUTNAME / #UV_PRJ_GET_CUR_TARGET
    UVSC_PSTAMP    powerScaleData; ///< Sent in #UV_DBG_POWERSCALE_SHOWPOWER
    iSTKENUM       iStkEnum;       ///< Sent in #UV_DBG_ENUM_STACK
    PGRESS         pgress;         ///< Sent in #UV_PRJ_CMD_PROGRESS
    ENUMTPM        enumtpm;        ///< Sent in #UV_DBG_ENUM_SYMTP
    iINTERVAL      iInterval;      ///< Sent in #UV_DBG_WAKE
    UINT           nVal;           ///< Sent in #UV_DBG_STEP_HLL_N, UV_DBG_STEP_INTO_N, UV_DBG_STEP_INSTRUCTION_N
    xU64           nAddress;       ///< Sent in #UV_DBG_RUN_TO_ADDRESS
    UVSOCK_OPTIONS uvSockOpt;      ///< Sent in #UV_GEN_SET_OPTIONS

    // Command response, or new format asynchronous message data
    UVSOCK_CMD_RESPONSE cmdRsp; ///< Command response formatted data
} UVSOCK_CMD_DATA;

/** UVSOCK message format
  *
  * Every UVSOCK message has this format. Each message contains a 32-byte
  * header, and a variable length data section. The total length of the message
  * must not exceed #SOCK_NDATA bytes.
  *
  * <b>Header Section</b>
  *
  * @a m_nTotalLen represents the total length of the message in bytes. @a m_eCmd is
  * the command code and represents the operation the message should perform. The
  * Data Section format is dependent on the value of @a m_eCmd. @a m_nBufLen represents
  * the length of the Data Section in bytes. @a cycles and @a tStamp represent the
  * current execution time of the simulation at the point the message was sent.
  * These values are only valid for response messages, and only when code is being
  * debugged in the simulator. @a m_Id is reserved and is always 0.
  *
  * <b>Data Section</b>
  *
  * @a data contains the message data. It's format is dependent on @a m_eCmd and is
  * described in #UVSOCK_CMD_DATA.
  *
  */
typedef struct _tag_UVSOCK_CMD {
    UINT            m_nTotalLen; ///< Total message length (bytes)
    UV_OPERATION    m_eCmd;      ///< Command code
    UINT            m_nBufLen;   ///< Length of Data Section (bytes)
    xU64            cycles;      ///< Cycle value (Simulation mode only)
    double          tStamp;      ///< time-stamp (Simulation mode only)
    UINT            m_Id;        ///< Reserved
    UVSOCK_CMD_DATA data;        ///< Data Section (Command code dependent data)
} UVSOCK_CMD;

#pragma pack()


#endif /* #ifndef _UVSOCK_H_ */
