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
#include <vector>

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


    return RDDI_SUCCESS;
}
