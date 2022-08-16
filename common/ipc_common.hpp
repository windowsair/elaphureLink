#pragma once

#include <cstddef>
#include <stdint.h>
#include "windows.h"

// TODO: private kernel object namespace
#define EL_SHARED_MEMORY_NAME  "elaphure.Memory"
#define EL_SHARED_MEMORY_SIZE  4096 * 1001

#define EL_EVENT_PRODUCER_NAME "elaphure.Event.Producer"
#define EL_EVENT_CONSUMER_NAME "elaphure.Event.Consumer"


typedef struct el_memory_ {
    // for RDDI
    union {
        struct
        {
            uint32_t command_count;
            uint32_t data_len;
            uint8_t  data[4096 * 500 - 4 * 2];
        };
        uint8_t base[4096 * 500];
    } producer_page;

    // for proxy
    union {
        struct {
            uint32_t command_response;
            uint32_t data_len;
            uint8_t  data[4096 * 500 - 4 * 2];
        };
        uint8_t base[4096 * 500];
    } consumer_page;

    union {
        struct
        {
            // elaphureLink.Proxy version info
            uint32_t major_version;
            uint32_t minor_version;
            uint32_t revision;
            char     version_string[240];

            // elaphureLink.Proxy status
            uint32_t is_proxy_ready; // 1: ready, 0: not ready. The proxy will pre-read the base information, and change this filed to the ready state.

            // DAP info
            uint32_t capabilities;
            char     product_name[160];
            char     serial_number[160];
            char     firmware_version[20];
            uint32_t device_dap_buffer_size;
        };
        uint8_t base[4096];
    } info_page;

} el_memory_t;

#ifdef __cplusplus
#define CHECK_EL_MEMORY_ALIGN(member, offset_index) \
    static_assert(offsetof(el_memory_t, member) == offset_index, "Unpredictable alignment behavior")


CHECK_EL_MEMORY_ALIGN(producer_page.command_count, 0);
CHECK_EL_MEMORY_ALIGN(producer_page.data_len, 4);
CHECK_EL_MEMORY_ALIGN(producer_page.data, 8);

CHECK_EL_MEMORY_ALIGN(consumer_page.command_response, 4096 * 500 + 0);
CHECK_EL_MEMORY_ALIGN(consumer_page.data_len, 4096 * 500 + 4);
CHECK_EL_MEMORY_ALIGN(consumer_page.data, 4096 * 500 + 8);


CHECK_EL_MEMORY_ALIGN(info_page.major_version, 4096 * 500 * 2 + 0);
CHECK_EL_MEMORY_ALIGN(info_page.minor_version, 4096 * 500 * 2 + 4);
CHECK_EL_MEMORY_ALIGN(info_page.revision, 4096 * 500 * 2 + 8);
CHECK_EL_MEMORY_ALIGN(info_page.version_string, 4096 * 500 * 2 + 12);
CHECK_EL_MEMORY_ALIGN(info_page.is_proxy_ready, 4096 * 500 * 2 + 12 + 240);
CHECK_EL_MEMORY_ALIGN(info_page.capabilities, 4096 * 500 * 2 + 16 + 240);
CHECK_EL_MEMORY_ALIGN(info_page.product_name, 4096 * 500 * 2 + 20 + 240);
CHECK_EL_MEMORY_ALIGN(info_page.serial_number, 4096 * 500 * 2 + 20 + 160 + 240);
CHECK_EL_MEMORY_ALIGN(info_page.firmware_version, 4096 * 500 * 2 + 20 + 160 + 160 + 240);
CHECK_EL_MEMORY_ALIGN(info_page.device_dap_buffer_size, 4096 * 500 * 2 + 20 + 160 + 160 + 20 + 240);


#endif


extern el_memory_t *k_shared_memory_ptr;
extern HANDLE       k_producer_event;
extern HANDLE       k_consumer_event;

inline void produce_and_wait_consumer_response(int command_count, int data_len)
{
    k_shared_memory_ptr->producer_page.command_count = command_count;
    k_shared_memory_ptr->producer_page.data_len      = data_len;

    k_shared_memory_ptr->consumer_page.command_response = 0xFFFFFFFF; // invalid value

    SetEvent(k_producer_event);

    WaitForSingleObject(k_consumer_event, INFINITE);
}

inline void set_consumer_status(int status)
{
    k_shared_memory_ptr->consumer_page.command_response = status;
}