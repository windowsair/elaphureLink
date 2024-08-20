#include "stdafx.h"
#include "rddi_dll.hpp"

namespace rddi
{
decltype(::RDDI_Open)  *rddi_Open  = nullptr;
decltype(::RDDI_Close) *rddi_Close = nullptr;
//decltype(::RDDI_GetLastError)   *GetLastError   = nullptr;
//decltype(::RDDI_SetLogCallback) *SetLogCallback = nullptr;

// RDDI-DAP DLL Level 0 function pointers
//decltype(::DAP_GetInterfaceVersion)           *DAP_GetInterfaceVersion           = nullptr;
//decltype(::DAP_Configure)                     *DAP_Configure                     = nullptr;
//decltype(::DAP_Connect)                       *DAP_Connect                       = nullptr;
//decltype(::DAP_Disconnect)                    *DAP_Disconnect                    = nullptr;
//decltype(::DAP_GetSupportedOptimisationLevel) *DAP_GetSupportedOptimisationLevel = nullptr;
decltype(::DAP_GetNumberOfDAPs) *DAP_GetNumberOfDAPs = nullptr;
decltype(::DAP_GetDAPIDList)    *DAP_GetDAPIDList    = nullptr;
decltype(::DAP_ReadReg)         *DAP_ReadReg         = nullptr;
decltype(::DAP_WriteReg)        *DAP_WriteReg        = nullptr;
decltype(::DAP_RegAccessBlock)  *DAP_RegAccessBlock  = nullptr;
//decltype(::DAP_RegWriteBlock)       *DAP_RegWriteBlock       = nullptr;
//decltype(::DAP_RegReadBlock)        *DAP_RegReadBlock        = nullptr;
decltype(::DAP_RegWriteRepeat) *DAP_RegWriteRepeat = nullptr;
decltype(::DAP_RegReadRepeat)  *DAP_RegReadRepeat  = nullptr;
//decltype(::DAP_RegReadWaitForValue) *DAP_RegReadWaitForValue = nullptr;
//decltype(::DAP_Target)              *DAP_Target              = nullptr;

// RDDI-DAP DLL Level 1 function pointers
//decltype(::DAP_DefineSequence) *DAP_DefineSequence = nullptr;
//decltype(::DAP_RunSequence)    *DAP_RunSequence    = nullptr;

// CMSIS-DAP DLL function pointers
decltype(::CMSIS_DAP_Detect)             *CMSIS_DAP_Detect             = nullptr;
decltype(::CMSIS_DAP_Identify)           *CMSIS_DAP_Identify           = nullptr;
decltype(::CMSIS_DAP_ConfigureInterface) *CMSIS_DAP_ConfigureInterface = nullptr;
decltype(::CMSIS_DAP_DetectNumberOfDAPs) *CMSIS_DAP_DetectNumberOfDAPs = nullptr;
decltype(::CMSIS_DAP_DetectDAPIDList)    *CMSIS_DAP_DetectDAPIDList    = nullptr;
decltype(::CMSIS_DAP_Commands)           *CMSIS_DAP_Commands           = nullptr;
decltype(::CMSIS_DAP_ConfigureDAP) *CMSIS_DAP_ConfigureDAP = nullptr;
//decltype(::CMSIS_DAP_GetGUID)            *CMSIS_DAP_GetGUID            = nullptr;
decltype(::CMSIS_DAP_Capabilities) *CMSIS_DAP_Capabilities = nullptr;
decltype(::CMSIS_DAP_SWJ_Sequence) *CMSIS_DAP_SWJ_Sequence = nullptr;
decltype(::CMSIS_DAP_SWJ_Pins)     *CMSIS_DAP_SWJ_Pins     = nullptr;


RDDIHandle k_rddi_handle;
int        k_rddi_if_index = -1;
} // namespace rddi
