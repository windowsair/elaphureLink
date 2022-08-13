/**
 * @file dap_jtag.cpp
 * @author windowsair (msdn_01@sina.com)
 * @brief JTAG device probe
 *
 * @copyright BSD-2-Clause
 *
 */
#include "pch.h"

#include <array>
#include <cassert>
#include <ios>
#include <sstream>
#include <vector>

#include "data/device_jtag_idcode.h"
#include "ElaphureLinkRDDIContext.h"


inline uint32_t count_1bits(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
    x = (x & 0x00FF00FF) + ((x >> 8) & 0x00FF00FF);
    x = (x & 0x0000FFFF) + ((x >> 16) & 0x0000FFFF);
    return x & 0x3F;
}

inline uint32_t count_0bits(uint32_t x)
{
    return 32U - count_1bits(x);
}

inline int get_jtag_scan_chain_devices_num(const uint32_t *tdo_response, int tdo_u32_length)
{
    // Devices in BYPASS mode will delay TDI for one cycle.
    // So we just check the number of 0 bits on TDO to get the number of devices.

    int count = 0;
    for (int i = 0; i < tdo_u32_length; i++) {
        count += static_cast<int>(count_0bits(tdo_response[i]));
    }

    // The number of 0 bits on the LSB is the same as the number of devices, and the rest of the high bits must be 1.
    // We check for noise on the line in this way.
    int tmp_count = count;
    for (int i = 0; i < tdo_u32_length; i++) {
        uint32_t bitmask = 0xFFFFFFFF;
        if (tmp_count > 0) {
            if (tmp_count % 32 == 0) {
                bitmask = 0;
                tmp_count -= 32;
            } else {
                bitmask <<= (tmp_count % 32);
                tmp_count -= tmp_count % 32;
            }
        }

        if (bitmask != tdo_response[i]) {
            count = 0;
            break;
        }
    }

    return count;
}

inline void get_jtag_device_idcode(const int device_num)
{
    auto &idcode_list = kContext.get_dap_idcode_list();
    idcode_list.clear();


    // step1: constructing IDCODE requests
    std::vector<uint8_t> req_array = {
        0x14, 0x04, // command count == 4
        0x46, 0xFF, // goto Test-Logic-Reset
        0x01, 0xFF, // goto Run-Test/Idle
        0x41, 0xFF, // goto Select-DR-Scan
        0x02, 0xFF, // goto Shift-DR
        // now add command to read idcode
    };

    // Up to 2 (8byte total) IDCODEs can be read at one time.
    int read_idcode_times = (device_num / 2) + (device_num % 2);
    req_array[1] += read_idcode_times;

    for (int i = 0; i < device_num / 2; i++) {
        req_array.insert(req_array.end(),
                         { 0x80, 0xFE, 0xFE, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0xFE }); // 64byte
    }

    if (device_num % 2) {
        req_array.insert(req_array.end(),
                         { 0xA0, 0xFE, 0xFE, 0x00, 0xFE }); // 32byte
    }

    // send requests
    memcpy(&(k_shared_memory_ptr->producer_page.data[0]), req_array.data(), req_array.size());

    // start transfer!
    produce_and_wait_consumer_response(
        device_num * 4, // 32byte idcode
        req_array.size());

    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
        return;
    }

    std::vector<uint32_t> idcode_from_device;
    idcode_from_device.resize(device_num);
    memcpy(idcode_from_device.data(), k_shared_memory_ptr->consumer_page.data, 4 * device_num);

    std::vector<uint32_t> ir_length_list;
    uint32_t              irlen;

    auto get_ir_length = [](uint32_t idcode) -> uint32_t {
        int i = 0;
        while (k_jtag_idcode_list[i].idcode != INVALID_IDCODE) {
            if (k_jtag_idcode_list[i].idcode == idcode) {
                return k_jtag_idcode_list[i].irlen;
            }
            i++;
        }
        return 0;
    };

    // step2: check IDCODE
    for (uint32_t idcode_ : idcode_from_device) {
        // The JATG spec requires that the first position of the LSB must be 1.
        // If it is not 1, it indicates that the device does not implement IDCODE.
        // At this point we cannot use a priori knowledge to obtain information about the device.
        if ((idcode_ & 0x1) == 0) {
            EL_SHOW_WARNING_MSG_BOX(
                "A device in JTAG does not implement IDCODE.\nelaphureLink can not perform JTAG auto probe.",
                "elaphureLink Warning");
            idcode_list.clear();
            return;
        }

        if ((irlen = get_ir_length(idcode_)) == 0) {
            std::stringstream ss;
            ss << "elaphureLink can not recognize the device IDCODE, the corresponding device IDCODE is\n        0x"
               << std::hex << idcode_;
            ss << std::endl
               << std::endl
               << "Please open an issue here to notify us:\n    https://github.com/windowsair/elaphureLink/issues";
            std::string msg = ss.str();
            EL_SHOW_WARNING_MSG_BOX(msg.c_str(), "elaphureLink Warning");

            idcode_list.clear();
            return;
        }

        ir_length_list.push_back(irlen);
    }

    // Great! Now all devices have been correctly identified.

    // step3: send JTAG configure
    std::vector<uint8_t> ir_len_req_array = {
        0x7F, 0x01,
        0x15, static_cast<uint8_t>(device_num)
    };

    for (uint32_t irlen_ : ir_length_list) {
        ir_len_req_array.push_back(static_cast<uint8_t>(irlen_));
    }

    // reset JTAG
    ir_len_req_array.insert(ir_len_req_array.end(),
                            {
                                0x12, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DAP_SWJ_Sequence (Reset sequence)
                                0x12, 0x10, 0x3c, 0xe7,                               // DAP_SWJ_Sequence (SWD-to-JTAG switch)
                                0x12, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DAP_SWJ_Sequence (Reset sequence)
                                0x14, 0x02, 0x46, 0xFF, 0x01, 0xFF                    // goto Test-Logic-Reset - > Run-Test/Idle
                            });
    ir_len_req_array[1] += 4; // command count: 4


    memcpy(&(k_shared_memory_ptr->producer_page.data[0]), ir_len_req_array.data(), ir_len_req_array.size());
    produce_and_wait_consumer_response(
        0, // nothing to receive
        ir_len_req_array.size());

    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
        idcode_list.clear();
        return;
    }

    // step4: add to idcode list

    for (uint32_t idcode_ : idcode_from_device) {
        idcode_list.push_back(idcode_);
    }
}

int rddi_cmsis_dap_probe_jtag_device(const RDDIHandle handle, int *noOfDAPs)
{
    // Some debugger implementations perform JTAG device probing by reading IDCODE.
    // We use a combination of BYPASS and IDCODE for JTAG device probing.

    // Unfortunately, such an automated probing process cannot reach its goal in all cases.
    // This is mainly because some devices may not implement the IDCODE feature, and IDCODE is not recorded for some devices.
    // In addition, there are some devices that may not meet the JTAG specification.

    // However, for the tasks we need to accomplish, these situations are not actually common.



    // step1: check number of devices

    // clang-format off
    std::vector<uint8_t> req_array = {
        0x03,                                                 // DAP_Disconnect
        0x02, 0x02,                                           // DAP_Connect (JTAG)
        0x11, 0x00, 0x00, 0x00, 0x00,                         // DAP_SWJ_Clock
        0x04, 0x00, 0x64, 0x00, 0x00, 0x00,                   // DAP_TransferConfigure
        0x12, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DAP_SWJ_Sequence (Reset sequence)
        0x12, 0x10, 0x3c, 0xe7,                               // DAP_SWJ_Sequence (SWD-to-JTAG switch)
        0x12, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DAP_SWJ_Sequence (Reset sequence)
        0x14, 0x0D,                                           // DAP_JTAG_Sequence (0x0D: 13 commands) ----> check JTAG devices number
                    0x46, 0xFF, // goto Test-Logic-Reset
                    0x01, 0xFF, // goto Run-Test/Idle
                    0x42, 0xFF, // goto Select-IR-Scan
                    0x02, 0xFF, // goto shift-IR
                    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 64 1s sequence ---> set bypass
                    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 64 1s sequence ---> set bypass
                    // total IR length must less than 128 (64 + 64)
                    0x43, 0xFF, // goto Select-IR-Scan
                    0x02, 0xFF, // goto Shift-DR
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64 0s sequence ---> flush DR register
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64 0s sequence ---> flush DR register
                    0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                    0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Total 192 1s sequence, Capture TDO data
    };
    // clang-format on

    int command_count = 8;

    int tdo_length_u8_num  = 24;
    int tdo_length_u32_num = tdo_length_u8_num / sizeof(uint32_t);

    assert(tdo_length_u32_num == 6);
    uint32_t tdo_device_info_data[6]; // 192bits
    uint8_t *p_tdo_device_info_data = reinterpret_cast<uint8_t *>(tdo_device_info_data);


    // set clock
    int clock = kContext.get_debug_clock();
    memcpy(&req_array[4], &clock, 4);

    // copy to buffer
    k_shared_memory_ptr->producer_page.data[0] = 0x7F;
    k_shared_memory_ptr->producer_page.data[1] = command_count;
    memcpy(&(k_shared_memory_ptr->producer_page.data[2]), req_array.data(), req_array.size());

    // start transfer!
    produce_and_wait_consumer_response(
        tdo_length_u8_num, // 1 for DAP_JTAG_Sequence Capture TDO data (192bits --> 24byte)
        2 + req_array.size());

    if (k_shared_memory_ptr->consumer_page.command_response != DAP_RES_OK) {
        return RDDI_INTERNAL_ERROR;
    }

    memcpy(p_tdo_device_info_data, k_shared_memory_ptr->consumer_page.data, tdo_length_u8_num);


    int device_count = get_jtag_scan_chain_devices_num(tdo_device_info_data, tdo_length_u32_num);
    *noOfDAPs        = device_count;

    if (device_count == 0) {
        return RDDI_SUCCESS;
    }


    // step2: JTAG blind identification
    get_jtag_device_idcode(device_count);

    return RDDI_SUCCESS;
}
