/**
 * @file rddi_dap.cpp
 * @author windowsair (msdn_01@sina.com)
 * @brief Handles the main interface for RDDI
 *
 * @copyright BSD-2-Clause
 *
 */
#include "pch.h"

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <array>

#include "ElaphureLinkRDDIContext.h"

#define EL_FORCE_DEBUGBREAK 0


#if (EL_FORCE_DEBUGBREAK == 1)
#define EL_DEBUG_BREAK() __debugbreak()
#else
#define EL_DEBUG_BREAK()
#endif

inline const uint8_t k_dap_reg_offset_map[] = {
    0x00, 0x04, 0x08, 0x0C, // for DP_0x0, DP_0x4, DP_0x8, DP_0xC
    0x01, 0x05, 0x09, 0x0D, // for AP_0x0, AP_0x4, AP_0x8, AP_0xC  ---> this field set APnDP
};


RDDI_EXPORT int RDDI_Open(RDDIHandle *pHandle, const void *pDetails)
{
    EL_DEBUG_BREAK();


    if (kContext.get_rddi_handle() != -1) {
        // already open
        return RDDI_TOOMANYCONNECTIONS;
    }

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    if (pHandle == nullptr) {
        return RDDI_BADARG;
    }

    if (*pHandle == kContext.get_rddi_handle()) {
        // already open
        return RDDI_TOOMANYCONNECTIONS;
    }

    *pHandle = 1;
    kContext.set_rddi_handle(1);

    // Reset to default state to prevent false wake up
    ResetEvent(k_consumer_event);
    ResetEvent(k_producer_event);

    // TODO: context status clean up

    return RDDI_SUCCESS;
}

RDDI_EXPORT int RDDI_Close(RDDIHandle handle)
{
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    kContext.set_rddi_handle(-1); // set invalid handle

    // TODO: context status clean up

    return RDDI_SUCCESS;
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
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }
    assert(configFileName == nullptr);
    return RDDI_SUCCESS;
}

RDDI_EXPORT int DAP_Connect(const RDDIHandle handle, RDDI_DAP_CONN_DETAILS *pConnDetails)
{
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    return RDDI_SUCCESS;
}

RDDI_EXPORT int DAP_Disconnect(const RDDIHandle handle)
{
    return RDDI_SUCCESS;
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
    EL_DEBUG_BREAK();

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    const uint16_t reg_high = regID >> 16;
    const uint16_t reg_low  = regID & 0xFFFF;

    assert(reg_high == 0 || reg_high == 1 || reg_high == 3);


    assert(reg_low < 8 || reg_low == 16 || reg_low == 17);
    if (reg_low == 16 || reg_low == 17) {
        __debugbreak();
    }

    uint8_t transfer_request = k_dap_reg_offset_map[reg_low] | 0x2; // read register

    std::vector<uint8_t> res_array = {
        ID_DAP_Transfer,
        static_cast<uint8_t>(DAP_ID),
        1, // transfer count
        transfer_request
    };

    memcpy(&(k_shared_memory_ptr->producer_page.data), res_array.data(), res_array.size());

    produce_and_wait_consumer_response(
        1, res_array.size());

    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK
        || k_shared_memory_ptr->consumer_page.data_len != 4) {
        return RDDI_INTERNAL_ERROR;
    }

    memcpy(value, k_shared_memory_ptr->consumer_page.data, 4);

    return RDDI_SUCCESS;
}


RDDI_EXPORT int DAP_WriteReg(const RDDIHandle handle, const int DAP_ID, const int regID, const int value)
{
    EL_DEBUG_BREAK();

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }


    assert((regID & 0xFFFF) <= 8 || (regID & 0xFFFF) == 16 || (regID & 0xFFFF) == 17);
    assert((regID & DAP_REG_RnW) == 0); // write register

    uint8_t reg_address = regID & 0xFF;
    if (reg_address == 16 || reg_address == 17) {
        __debugbreak(); // FIXME: this case
    }

    const uint8_t *data = reinterpret_cast<const uint8_t *>(&value);

    if (reg_address == DAP_REG_DP_ABORT) {
        std::vector<uint8_t> res_array = {
            ID_DAP_WriteABORT,
            static_cast<uint8_t>(DAP_ID),
            data[0], data[1], data[2], data[3] // data
        };

        memcpy(&(k_shared_memory_ptr->producer_page.data), res_array.data(), res_array.size());
        produce_and_wait_consumer_response(
            1, res_array.size()); // 1: transfer count

        if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
            return RDDI_INTERNAL_ERROR;
        }

        return RDDI_SUCCESS;
    }

    uint8_t transfer_request = k_dap_reg_offset_map[reg_address];

    constexpr int command_length = 8;

    std::vector<uint8_t> res_array = {
        ID_DAP_Transfer,
        static_cast<uint8_t>(DAP_ID),
        1, // transfer count
        transfer_request,
        data[0], data[1], data[2], data[3] // data
    };

    memcpy(&(k_shared_memory_ptr->producer_page.data), res_array.data(), command_length);

    produce_and_wait_consumer_response(
        1, command_length); // 1: transfer count

    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
        return RDDI_INTERNAL_ERROR;
    }

    return RDDI_SUCCESS;
}


RDDI_EXPORT int DAP_RegAccessBlock(const RDDIHandle handle, const int DAP_ID, const int numRegs,
                                   const int *regIDArray, int *dataArray)
{
    EL_DEBUG_BREAK();

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    enum TransferRequestEnum : uint8_t {
        APnDP       = UINT8_C(0x1),
        RnW         = UINT8_C(0x2),
        Value_Match = UINT8_C(0x10),
        Match_Mask  = UINT8_C(0x20)
    };


    std::vector<int> read_reg_index_array;

    constexpr int transfer_version_transfer_count_index = 0x2;
    constexpr int transfer_version_array_initial_length = 3;

    std::vector<uint8_t> dap_transfer_array = {
        ID_DAP_Transfer, static_cast<uint8_t>(DAP_ID),
        0x00 // transfer count (this field should be modify later)  <------ transfer_version_transfer_count_index
    };

    /*
     * xxxx 17 xxxx
     * 17 xxxxx
     * 17
     * xxxx 17 xxxx 17 xxxx
     * xxxx 17
     * 17 xxxx 17
     * 17 xxxx 17 xxxx
     * 17 17 17 ----> evil.....
     */

    constexpr int execute_version_command_num_index    = 0x1;
    constexpr int execute_version_array_initial_length = 11;

    auto set_dap_transfer_array_with_retry = [&](uint16_t retry_count) {
        const uint8_t *p_retry_count = reinterpret_cast<const uint8_t *>(&retry_count);

        dap_transfer_array.clear();
        dap_transfer_array = {
            ID_DAP_ExecuteCommands, 0x02, //  <--- execute_version_command_num_index

            ID_DAP_TransferConfigure,
            0x00,                               // idle cycles
            p_retry_count[0], p_retry_count[1], // WAIT Retry
            p_retry_count[0], p_retry_count[1], // Match Retry

            ID_DAP_Transfer, static_cast<uint8_t>(DAP_ID),
            0x00 // transfer count (this field should be modify later)  // <------ execute_version_transfer_count_index
            // ...Add data here
        };
    };

    constexpr int execute_version_transfer_count_index = 0xA;


    // return 0: OK
    auto start_request_and_get_response = [&](int transfer_count, int read_register_command_count) -> int {
        memcpy(&(k_shared_memory_ptr->producer_page.data), dap_transfer_array.data(), dap_transfer_array.size());
        produce_and_wait_consumer_response(
            transfer_count, dap_transfer_array.size());

        if (k_shared_memory_ptr->consumer_page.command_response == DAP_RES_FAULT) {
            return RDDI_DAP_DP_STICKY_ERR;
        } else if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
            return RDDI_INTERNAL_ERROR;
        }

        if (read_register_command_count == 0) {
            return 0; // nothing to read
        }

        if (k_shared_memory_ptr->consumer_page.data_len != read_register_command_count * 4) {
            return RDDI_INTERNAL_ERROR;
        }

        assert(read_reg_index_array.size() == read_register_command_count);
        const int *p_res_data = reinterpret_cast<const int *>(k_shared_memory_ptr->consumer_page.data);
        for (int j = 0; j < read_register_command_count; j++) {
            int index        = read_reg_index_array[j];
            dataArray[index] = p_res_data[j];
        }

        return 0;
    };

    int i = 0;
    int ret;

    while (i <= numRegs) { // Note the boundary conditions
        // split `DAP_REG_MATCH_RETRY`
        int dap_transfer_command_count  = 0; // uint8_t
        int read_register_command_count = 0;

        read_reg_index_array.clear();

        for (; i < numRegs; i++) {
            const uint32_t regID    = regIDArray[i];
            const uint16_t reg_high = regID >> 16;
            const uint16_t reg_low  = regID & 0xFFFF;

            assert(reg_high != 2); // 0, 1, 3
            assert(reg_low <= 8 || reg_low == DAP_REG_MATCH_RETRY || reg_low == DAP_REG_MATCH_MASK);

            if (reg_low == DAP_REG_MATCH_RETRY) {
                break;
            } else if (reg_low == DAP_REG_MATCH_MASK) {
                // Write Match Mask (instead of Register)
                dap_transfer_command_count++;

                const int      value_to_match   = dataArray[i];
                const uint8_t *p_value_to_match = reinterpret_cast<const uint8_t *>(&value_to_match);

                dap_transfer_array.insert(dap_transfer_array.end(),
                                          { Match_Mask,
                                            p_value_to_match[0], p_value_to_match[1], p_value_to_match[2], p_value_to_match[3] });
                // This case is essentially a write operation.

            } else if (reg_high == (DAP_REG_RnW | DAP_REG_WaitForValue) >> 16) {
                // Value Match Read
                dap_transfer_command_count++;

                const int      value_to_match   = dataArray[i];
                const uint8_t *p_value_to_match = reinterpret_cast<const uint8_t *>(&value_to_match);

                dap_transfer_array.insert(dap_transfer_array.end(),
                                          { static_cast<uint8_t>(k_dap_reg_offset_map[reg_low] | Value_Match | RnW),
                                            p_value_to_match[0], p_value_to_match[1], p_value_to_match[2], p_value_to_match[3] });
                // The case is a read operation, but with an implied write. No value is sent in the response.

            } else {
                dap_transfer_command_count++;

                if (reg_high & (DAP_REG_RnW >> 16)) {
                    // read reg
                    read_register_command_count++;
                    read_reg_index_array.push_back(i);

                    dap_transfer_array.push_back(
                        k_dap_reg_offset_map[reg_low] | RnW);
                } else {
                    // write reg
                    int            write_value   = dataArray[i];
                    const uint8_t *p_write_value = reinterpret_cast<uint8_t *>(&write_value);

                    dap_transfer_array.insert(dap_transfer_array.end(),
                                              { k_dap_reg_offset_map[reg_low],
                                                p_write_value[0], p_write_value[1], p_write_value[2], p_write_value[3] });
                }
            }
        }


        if (i < numRegs) {
            // okay, we meet a `DAP_REG_MATCH_RETRY` request

            if (dap_transfer_array[0] == ID_DAP_Transfer && dap_transfer_array.size() == transfer_version_array_initial_length) { // case 1
                // nothing to send

                // just reset transfer array
                ;
            } else if (dap_transfer_array[0] == ID_DAP_ExecuteCommands && dap_transfer_array.size() == execute_version_array_initial_length) { // case 2
                // After reset the transfer array, no data has been added
                // We met `DAP_REG_MATCH_RETRY` request again!

                // just send `DAP_TransferConfigure` command.
                dap_transfer_array[execute_version_command_num_index] = 0x1; //  Only one command needs to be sent
                dap_transfer_array.resize(8);                                // length of `DAP_ExecuteCommands` + `DAP_TransferConfigure` (2+6)
                if ((ret = start_request_and_get_response(0, 0)) != 0) {
                    return ret;
                }

            } else if (dap_transfer_array[0] == ID_DAP_Transfer) { // case 3
                // Already have some of the DAP_transfer data. Send them.
                dap_transfer_array[transfer_version_transfer_count_index] = dap_transfer_command_count;
                if ((ret = start_request_and_get_response(dap_transfer_command_count, read_register_command_count)) != 0) {
                    return ret;
                }

            } else [[likely]] { // case 4
                // Transfer `DAP_TransferConfigure` and `DAP_Transfer` together

                dap_transfer_array[execute_version_transfer_count_index] = dap_transfer_command_count; // `DAP_Transfer`: transfer_count
                if ((ret = start_request_and_get_response(dap_transfer_command_count, read_register_command_count)) != 0) {
                    return ret;
                }
            }

            // reset dap transfer array
            const uint16_t retry_count = static_cast<uint16_t>(dataArray[i]);
            set_dap_transfer_array_with_retry(retry_count);

        } else [[likely]] { // i == numRegs
            // When the above iteration is finished, or when the last command is `DAP_REG_MATCH_RETRY`, we will come here

            assert(dap_transfer_array.size() != 0);
            if (dap_transfer_array[0] == ID_DAP_Transfer) [[likely]] {
                dap_transfer_array[transfer_version_transfer_count_index] = dap_transfer_command_count;
            } else { // ID_DAP_ExecuteCommands
                // check command count
                if (dap_transfer_array.size() == execute_version_array_initial_length) {
                    // same as case2
                    dap_transfer_array[execute_version_command_num_index] = 1; //  Only one command needs to be sent
                    dap_transfer_array.resize(8);                              // length of `DAP_ExecuteCommands` + `DAP_TransferConfigure` (2+6)
                } else {
                    dap_transfer_array[execute_version_command_num_index]    = 2;                          // `DAP_TransferConfigure` + `DAP_Transfer`
                    dap_transfer_array[execute_version_transfer_count_index] = dap_transfer_command_count; // `DAP_Transfer`: transfer count
                }
            }

            if ((ret = start_request_and_get_response(dap_transfer_command_count, read_register_command_count)) != 0) {
                return ret;
            }

            // It's already the last transmission
        }

        i++;
    }

    return RDDI_SUCCESS;
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
    EL_DEBUG_BREAK();

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    const uint16_t reg_high = regID >> 16;
    const uint16_t reg_low  = regID & 0xFFFF;

    assert(reg_high == 0);

    assert(reg_low < 8 || reg_low == 16 || reg_low == 17);
    if (reg_low == 16 || reg_low == 17) {
        __debugbreak();
    }

    uint8_t transfer_request = k_dap_reg_offset_map[reg_low]; // write register

    int16_t        transfer_count   = 0;
    const uint8_t *p_transfer_count = reinterpret_cast<const uint8_t *>(&transfer_count);


    std::vector<uint8_t> req_array = {
        ID_DAP_TransferBlock,
        static_cast<uint8_t>(DAP_ID),
        0x00, 0x00, // transfer count
        transfer_request
    };

    constexpr int header_length = 5;
    assert(req_array.size() == header_length);

    constexpr int max_transmit_one_time = (1400 - 5) / 4; // 1400 MTU
    for (int i = 0; i < numRepeats; i += max_transmit_one_time) {
        transfer_count = (std::min)(max_transmit_one_time, numRepeats - i);
        assert(transfer_count != 0);
        req_array[2] = p_transfer_count[0];
        req_array[3] = p_transfer_count[1];
        // copy data to buffer
        memcpy(&(k_shared_memory_ptr->producer_page.data), req_array.data(), header_length);
        memcpy(&(k_shared_memory_ptr->producer_page.data[header_length]), &dataArray[i], 4 * transfer_count);

        produce_and_wait_consumer_response(transfer_count, header_length + 4 * transfer_count);

        if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
            return RDDI_INTERNAL_ERROR;
        }
    }


    return RDDI_SUCCESS;
}


RDDI_EXPORT int DAP_RegReadRepeat(const RDDIHandle handle, const int DAP_ID, const int numRepeats,
                                  const int regID, int *dataArray)
{
    EL_DEBUG_BREAK();

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    const uint16_t reg_high = regID >> 16;
    const uint16_t reg_low  = regID & 0xFFFF;

    assert(reg_high == 0);
    assert(numRepeats * 4 < 1400 - 5); // 1400 MTU


    assert(reg_low < 8 || reg_low == 16 || reg_low == 17);
    if (reg_low == 16 || reg_low == 17) {
        __debugbreak();
    }

    uint8_t transfer_request = k_dap_reg_offset_map[reg_low] | 0x2; // read register

    const uint16_t transfer_count = numRepeats;
    assert(numRepeats <= 0xFFFF);
    const uint8_t *p_transfer_count = reinterpret_cast<const uint8_t *>(&transfer_count);

    std::vector<uint8_t> req_array = {
        ID_DAP_TransferBlock,
        static_cast<uint8_t>(DAP_ID),
        p_transfer_count[0], p_transfer_count[1], // transfer count
        transfer_request
    };

    memcpy(&(k_shared_memory_ptr->producer_page.data), req_array.data(), req_array.size());

    produce_and_wait_consumer_response(
        numRepeats, req_array.size());

    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK
        || k_shared_memory_ptr->consumer_page.data_len != numRepeats * 4) {
        return RDDI_INTERNAL_ERROR;
    }

    memcpy(dataArray, k_shared_memory_ptr->consumer_page.data, 4 * numRepeats);

    return RDDI_SUCCESS;
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
    if (resp_str)
        resp_str[0] = '\0';
    return RDDI_SUCCESS;
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
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    // we have only one dap instance.
    *noOfIFs = 1;
    return RDDI_SUCCESS;
}

RDDI_EXPORT int CMSIS_DAP_Identify(const RDDIHandle handle, int ifNo, int idNo, char *str, const int len)
{
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    assert(idNo == 2 || idNo == 3 || idNo == 4);
    assert(len > 0);
    int src_len;

    switch (idNo) {
        case 2:
            // port identify string, must include substring "CMSIS-DAP"
            src_len = strlen(k_shared_memory_ptr->info_page.product_name);
            strncpy_s(str, len, k_shared_memory_ptr->info_page.product_name, len - 1);
            break;
        case 3:
            // Serial No string
            src_len = strlen(k_shared_memory_ptr->info_page.serial_number);
            strncpy_s(str, len, k_shared_memory_ptr->info_page.serial_number, len - 1);
            break;
        case 4:
            // Firmware string
            src_len = strlen(k_shared_memory_ptr->info_page.firmware_version);
            strncpy_s(str, len, k_shared_memory_ptr->info_page.firmware_version, len - 1);
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

    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

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

    kContext.set_debug_configure_from_list(command_list);

    return RDDI_SUCCESS;
}


RDDI_EXPORT int CMSIS_DAP_ConfigureDAP(const RDDIHandle handle, const char *str)
{
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    const char *p = str;
    std::string value_str, key_str;

    int status  = 0;
    int key_len = 0, value_len = 0;
    while (true) {
        switch (status) {
            case 0: // parse key
                if (*p == '=') {
                    status = 1;
                } else {
                    key_len++;
                }
                break;
            case 1: // parse value
                if (*p == '\0') {
                    // "key=value\0"
                    //  |   |_ p - value_len
                    //  |_____ p - key_len - value_len - 1
                    value_str = std::string(p - value_len, value_len);
                    key_str   = std::string(p - key_len - value_len - 1, key_len);
                } else {
                    value_len++;
                }
                break;
            default:
                break;
        }
        if (*p == '\0') {
            break;
        }
        p++;
    }

    kContext.set_debug_configure(key_str, value_str);

    return RDDI_SUCCESS;
}

RDDI_EXPORT int CMSIS_DAP_GetGUID(const RDDIHandle handle, int ifNo, char *str, const int len)
{
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    assert(ifNo == 0);

    sprintf_s(str, len, "elaphureLink"); // TODO: this field
    return RDDI_SUCCESS;
}

RDDI_EXPORT int CMSIS_DAP_Capabilities(const RDDIHandle handle, int ifNo, int *cap_info)
{
    //EL_TODO TODO: check device caps
    EL_DEBUG_BREAK();

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    assert(ifNo == 0);

    *cap_info = INFO_CAPS_SWD; // TODO: other caps

    return RDDI_SUCCESS;
}


RDDI_EXPORT int CMSIS_DAP_DetectNumberOfDAPs(const RDDIHandle handle, int *noOfDAPs)
{
    //EL_TODO_IMPORTANT
    EL_DEBUG_BREAK();
    if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
        // proxy not ready
        return RDDI_FAILED;
    }

    // TODO: check device? // send configuration, and get the list of DAP

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }

    // for JTAG
    if (!kContext.is_swd_debug_port()) {
        return rddi_cmsis_dap_probe_jtag_device(handle, noOfDAPs);
    }

    // for SWD
    constexpr int req_array_len = 45;
    constexpr int command_count = 10;

    std::array<uint8_t, req_array_len> req_array = {
        0x03,                                                 // DAP_Disconnect
        0x02, 0x01,                                           // DAP_Connect (SWD)
        0x11, 0x00, 0x00, 0x00, 0x00,                         // DAP_SWJ_Clock
        0x04, 0x00, 0x64, 0x00, 0x00, 0x00,                   // DAP_TransferConfigure
        0x13, 0x00,                                           // DAP_SWD_Configure
        0x12, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DAP_SWJ_Sequence (Reset sequence)
        0x12, 0x10, 0x9e, 0xe7,                               // DAP_SWJ_Sequence (JTAG-to-SWD switch)
        0x12, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DAP_SWJ_Sequence (Reset sequence)
        0x12, 0x08, 0x00,                                     // DAP_SWJ_Sequence (Idle sequence)
        0x05, 0x00, 0x01, 0x02,                               // DAP_Transfer     (Get IDCODE, see ADIv5 spec)
    };

    // set clock
    int clock = kContext.get_debug_clock();
    memcpy(&req_array[4], &clock, 4);

    // copy to buffer
    k_shared_memory_ptr->producer_page.data[0] = 0x7F;
    k_shared_memory_ptr->producer_page.data[1] = command_count;
    memcpy(&(k_shared_memory_ptr->producer_page.data[2]), req_array.data(), req_array_len);

    // start transfer!
    produce_and_wait_consumer_response(
        1, // 1 for DAP_Transfer
        2 + req_array_len);

    // read response data
    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
        return RDDI_INTERNAL_ERROR; // FIXME: RDDI_FAIL
    }

    uint32_t *p_data  = reinterpret_cast<uint32_t *>(&(k_shared_memory_ptr->consumer_page.data[0]));
    uint32_t  idcode1 = *p_data;


    /// step2: resend idcode requset
    constexpr int resend_req_len       = 7;
    constexpr int resend_command_count = 2;

    std::array<uint8_t, resend_req_len> resend_req_array = {
        0x12, 0x08, 0x00,       // DAP_SWJ_Sequence (Idle sequence)
        0x05, 0x00, 0x01, 0x02, // DAP_Transfer     (Get IDCODE, see ADIv5 spec)
    };


    // copy to buffer
    k_shared_memory_ptr->producer_page.data[0] = 0x7F;
    k_shared_memory_ptr->producer_page.data[1] = resend_command_count;
    memcpy(&(k_shared_memory_ptr->producer_page.data[2]), resend_req_array.data(), resend_req_len);


    produce_and_wait_consumer_response(
        1, // 1 for DAP_Transfer
        2 + resend_req_len);

    // read response
    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
        return RDDI_INTERNAL_ERROR;
    }

    uint32_t idcode2 = *p_data;

    if (idcode1 != idcode2) {
        return RDDI_INTERNAL_ERROR;
    }


    auto &idcode_list = kContext.get_dap_idcode_list();
    idcode_list.clear();
    idcode_list.push_back(idcode1);


    *noOfDAPs = 1; // for SWD device

    // FIXME: for multi-drop system

    return RDDI_SUCCESS;
}

RDDI_EXPORT int CMSIS_DAP_DetectDAPIDList(const RDDIHandle handle, int *DAP_ID_Array, size_t sizeOfArray)
{
    //EL_TODO_IMPORTANT
    EL_DEBUG_BREAK(); // TODO: JTAG

    if (handle != kContext.get_rddi_handle()) {
        return RDDI_INVHANDLE;
    }


    const auto &idcode_list = kContext.get_dap_idcode_list();

    for (auto i = 0; i < idcode_list.size(); i++) {
        DAP_ID_Array[i] = idcode_list[i];
    }


    return RDDI_SUCCESS;
}


RDDI_EXPORT int CMSIS_DAP_Commands(const RDDIHandle handle, int num, unsigned char **request, int *req_len,
                                   unsigned char **response, int *resp_len)
{
    ////EL_TODO
    //__debugbreak();
    return 8204;
}



//////////////
///
///
///
///
///
///
///
///
///
///
///
#if 0
RDDI_EXPORT int CMSIS_DAP_GetNumberOfDevices()
{
    __debugbreak();
    return 8204;
}
#endif

#if 0
RDDI_EXPORT int CMSIS_DAP_Connect()
{
    __debugbreak();
    return 8204;
}
#endif
#if 0
RDDI_EXPORT int CMSIS_DAP_GetDeviceIDList()
{
    __debugbreak();
    return 8204;
}
#endif

#if 0
RDDI_EXPORT int CMSIS_DAP_DetectNumberOfDevices()
{
    __debugbreak();
    return 8204;
}
#endif

///
///
///
///
///
///
RDDI_EXPORT int CMSIS_DAP_Disconnect()
{
    //EL_TODO
    //__debugbreak();
    return 8204;
}

RDDI_EXPORT int DAP_SetCommTimeout()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_GetInterfaceVersion()
{
    __debugbreak();
    return 8204;
}



RDDI_EXPORT int CMSIS_DAP_ResetDAP()
{
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int CMSIS_DAP_SWJ_Sequence()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_JTAG_Sequence()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_Atomic_Result()
{
    //EL_TODO
    //__debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_Atomic_Control()
{
    //EL_TODO
    //__debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_WriteABORT()
{
    __debugbreak();
    return 8204;
}


RDDI_EXPORT int CMSIS_DAP_JTAG_GetIDCODEs()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_JTAG_GetIRLengths()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_Delay()
{
    //EL_TODO
    //__debugbreak();
    return 8204;
}

RDDI_EXPORT int CMSIS_DAP_SWJ_Pins()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_SWJ_Clock()
{
    __debugbreak();
    return 8204;
}
RDDI_EXPORT int CMSIS_DAP_ConfigureDebugger()
{
    //EL_TODO
    //__debugbreak();
    return 8204;
}