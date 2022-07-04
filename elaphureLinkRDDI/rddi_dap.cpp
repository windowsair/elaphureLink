#include "pch.h"

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "ElaphureLinkRDDIContext.h"


RDDI_EXPORT int RDDI_Open(RDDIHandle *pHandle, const void *pDetails)
{
    //EL_TODO_IMPORTANT
    //__debugbreak();
    *pHandle = 1;
    return RDDI_SUCCESS;
}

RDDI_EXPORT int RDDI_Close(RDDIHandle handle)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int RDDI_GetLastError(int *pError, char *pDetails, size_t detailsLen)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT void RDDI_SetLogCallback(RDDIHandle handle, RDDILogCallback pfn, void *context, int maxLogLevel)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
}

RDDI_EXPORT int DAP_GetInterfaceVersion(const RDDIHandle handle, int *version)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_Configure(const RDDIHandle handle, const char *configFileName)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_Connect(const RDDIHandle handle, RDDI_DAP_CONN_DETAILS *pConnDetails)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_Disconnect(const RDDIHandle handle)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_GetSupportedOptimisationLevel(const RDDIHandle handle, int *level)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_GetNumberOfDAPs(const RDDIHandle handle, int *noOfDAPs)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_GetDAPIDList(const RDDIHandle handle, int *DAP_ID_Array, size_t sizeOfArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_ReadReg(const RDDIHandle handle, const int DAP_ID, const int regID, int *value)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_WriteReg(const RDDIHandle handle, const int DAP_ID, const int regID, const int value)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_RegAccessBlock(const RDDIHandle handle, const int DAP_ID, const int numRegs,
                                   const int *regIDArray, int *dataArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_RegWriteBlock(const RDDIHandle handle, const int DAP_ID, const int numRegs,
                                  const int *regIDArray, const int *dataArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_RegReadBlock(const RDDIHandle handle, const int DAP_ID, const int numRegs,
                                 const int *regIDArray, int *dataArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_RegWriteRepeat(const RDDIHandle handle, const int DAP_ID, const int numRepeats,
                                   const int regID, const int *dataArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_RegReadRepeat(const RDDIHandle handle, const int DAP_ID, const int numRepeats,
                                  const int regID, int *dataArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_RegReadWaitForValue(const RDDIHandle handle, const int DAP_ID, const int numRepeats,
                                        const int regID, const int *mask, const int *requiredValue)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_Target(const RDDIHandle handle, const char *request_str, char *resp_str,
                           const int resp_len)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_DefineSequence(const RDDIHandle handle, const int seqID, void *seqDef)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int DAP_RunSequence(const RDDIHandle handle, const int seqID, void *seqInData, void *seqOutData)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


// This function will check the num of hardware debugger that connected to the PC.
RDDI_EXPORT int CMSIS_DAP_Detect(const RDDIHandle handle, int *noOfIFs)
{
    //EL_TODO_IMPORTANT
    //__debugbreak();

    // we have only one dap instance.
    *noOfIFs = 1; // TODO:check handle
    return RDDI_SUCCESS;
}

RDDI_EXPORT int CMSIS_DAP_Identify(const RDDIHandle handle, int ifNo, int idNo, char *str, const int len)
{
    //EL_TODO_IMPORTANT
    //__debugbreak();

    // TODO: check handle

    assert(idNo == 2 || idNo == 3 || idNo == 4);

    switch (idNo) {
        case 2:
            // port identify string, must include substring "CMSIS-DAP"
            sprintf_s(str, len, "elaphureLink CMSIS-DAP");
            break;
        case 3:
            // Serial No string
            sprintf_s(str, len, "elaphureLink");
            break;
        case 4:
            // Firmware string
            sprintf_s(str, len, "001");
            break;
        default:
            break;
    }


    return RDDI_SUCCESS;
}


RDDI_EXPORT int CMSIS_DAP_ConfigureInterface(const RDDIHandle handle, int ifNo, char *str)
{
    // parse configure string like:
    // "Master=Y;Port=SW;SWJ=Y;Clock=10000000;Trace=Off;TraceBaudrate=0;TraceTransport=None;"

    // TODO:check handle

    const char *p = str;

    std::vector<std::pair<std::string, std::string>> command_list;

    int status  = 0;
    int key_len = 0, value_len = 0;
    while (*p != '\0') {
        switch (status) {
            case 0: // parse key
                if (*p == '=') {
                    status = 1;
                } else {
                    key_len++;
                }
                break;
            case 1: // parse value
                if (*p == ';') {
                    // "key=value;"
                    //  |   |_ p - value_len
                    //  |_____ p - key_len - value_len - 1
                    std::string value_str = std::string(p - value_len, value_len);
                    std::string key_str   = std::string(p - key_len - value_len - 1, key_len);
                    command_list.emplace_back(key_str, value_str);

                    // clean up
                    key_len   = 0;
                    value_len = 0;
                    status    = 0;
                } else {
                    value_len++;
                }
                break;
            default:
                break;
        }

        p++;
    }

    kContext.setDebugConfigureFromList(command_list);

    return RDDI_SUCCESS;
}

RDDI_EXPORT int CMSIS_DAP_DetectNumberOfDAPs(const RDDIHandle handle, int *noOfDAPs)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}

RDDI_EXPORT int CMSIS_DAP_DetectDAPIDList(const RDDIHandle handle, int *DAP_ID_Array, size_t sizeOfArray)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int CMSIS_DAP_Commands(const RDDIHandle handle, int num, unsigned char **request, int *req_len,
                                   unsigned char **response, int *resp_len)
{
    //EL_TODO_IMPORTANT
    __debugbreak();
    return 8204;
}