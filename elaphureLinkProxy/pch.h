#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "proxy_export.hpp"
#include "ipc_common.hpp"
#include "dap.hpp"


extern bool k_is_proxy_init;

extern HANDLE       k_shared_memory_handle;
extern el_memory_t *k_shared_memory_ptr;

extern HANDLE k_producer_event;
extern HANDLE k_consumer_event;

struct WindowsVersionNumber {
    ULONG major_version;
    ULONG build_number;
};
extern WindowsVersionNumber k_windows_version_number;

struct el_proxy_config {
    uint8_t enable_vendor_command;
};
