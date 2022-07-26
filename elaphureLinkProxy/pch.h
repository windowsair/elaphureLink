#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "proxy_export.hpp"
#include "ipc_common.hpp"


extern bool k_is_proxy_init;

extern HANDLE       k_shared_memory_handle;
extern el_memory_t *k_shared_memory_ptr;

extern HANDLE k_producer_event;
extern HANDLE k_consumer_event;