#pragma once

#define _RDDI_IMPORT 1
#include "rddi_dap.h"
#include "rddi_dap_cmsis.h"

namespace rddi
{
extern decltype(::RDDI_Open)           *rddi_Open;
extern decltype(::RDDI_Close)          *rddi_Close;
extern decltype(::RDDI_GetLastError)   *rddi_GetLastError;
extern decltype(::RDDI_SetLogCallback) *rddi_SetLogCallback;

// RDDI-DAP DLL Level 0 function pointers
extern decltype(::DAP_GetInterfaceVersion)           *DAP_GetInterfaceVersion;
extern decltype(::DAP_Configure)                     *DAP_Configure;
extern decltype(::DAP_Connect)                       *DAP_Connect;
extern decltype(::DAP_Disconnect)                    *DAP_Disconnect;
extern decltype(::DAP_GetSupportedOptimisationLevel) *DAP_GetSupportedOptimisationLevel;
extern decltype(::DAP_GetNumberOfDAPs)               *DAP_GetNumberOfDAPs;
extern decltype(::DAP_GetDAPIDList)                  *DAP_GetDAPIDList;
extern decltype(::DAP_ReadReg)                       *DAP_ReadReg;
extern decltype(::DAP_WriteReg)                      *DAP_WriteReg;
extern decltype(::DAP_RegAccessBlock)                *DAP_RegAccessBlock;
extern decltype(::DAP_RegWriteBlock)                 *DAP_RegWriteBlock;
extern decltype(::DAP_RegReadBlock)                  *DAP_RegReadBlock;
extern decltype(::DAP_RegWriteRepeat)                *DAP_RegWriteRepeat;
extern decltype(::DAP_RegReadRepeat)                 *DAP_RegReadRepeat;
extern decltype(::DAP_RegReadWaitForValue)           *DAP_RegReadWaitForValue;
extern decltype(::DAP_Target)                        *DAP_Target;

// RDDI-DAP DLL Level 1 function pointers
extern decltype(::DAP_DefineSequence) *DAP_DefineSequence;
extern decltype(::DAP_RunSequence)    *DAP_RunSequence;

// CMSIS-DAP DLL function pointers
extern decltype(::CMSIS_DAP_Detect)             *CMSIS_DAP_Detect;
extern decltype(::CMSIS_DAP_Identify)           *CMSIS_DAP_Identify;
extern decltype(::CMSIS_DAP_ConfigureInterface) *CMSIS_DAP_ConfigureInterface;
extern decltype(::CMSIS_DAP_DetectNumberOfDAPs) *CMSIS_DAP_DetectNumberOfDAPs;
extern decltype(::CMSIS_DAP_DetectDAPIDList)    *CMSIS_DAP_DetectDAPIDList;
extern decltype(::CMSIS_DAP_Commands)           *CMSIS_DAP_Commands;
extern decltype(::CMSIS_DAP_ConfigureDAP)       *CMSIS_DAP_ConfigureDAP;
extern decltype(::CMSIS_DAP_GetGUID)            *CMSIS_DAP_GetGUID;
extern decltype(::CMSIS_DAP_Capabilities)       *CMSIS_DAP_Capabilities;

enum {
    RDDI_DAP_ERROR          = 0x2000, // RDDI-DAP Error
    RDDI_DAP_ERROR_NO_DLL   = 0x2001, // CMSIS_DAP.DLL missing
    RDDI_DAP_ERROR_INTERNAL = 0x2002, // Internal DLL Error
    RDDI_DAP_ERROR_POWER    = 0x2003, // Device could not be powered up
    RDDI_DAP_ERROR_DEBUG    = 0x2004, // Cannot enter Debug Mode
    RDDI_DAP_ERROR_MEMORY   = 0x2005, // Cannot access Memory
    RDDI_DAP_ERROR_INUSE    = 0x2006, // Debug Port in use
    RDDI_DAP_ERROR_SWJ      = 0x2007, // SWD/JTAG Communication Error
};


extern RDDIHandle k_rddi_handle;
extern int        k_rddi_if_index;

} // namespace rddi
