#include "pch.h"

bool k_is_proxy_init = false;

HANDLE k_shared_memory_handle = nullptr;
void  *k_shared_memory_ptr    = nullptr;

HANDLE k_producer_event = nullptr;
HANDLE k_consumer_event = nullptr;

inline void el_proxy_deinit();

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
    // TODO: init shared memory handle

    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            el_proxy_deinit();
            break;
    }
    return TRUE;
}


PROXY_DLL_FUNCTION int el_proxy_init()
{
    if (k_is_proxy_init) {
        return 0;
    }
    k_shared_memory_handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                               NULL,
                                               PAGE_READWRITE,
                                               0,
                                               EL_SHARED_MEMORY_SIZE,
                                               EL_SHARED_MEMORY_NAME);
    if (nullptr == k_shared_memory_handle || INVALID_HANDLE_VALUE == k_shared_memory_handle) {
        return -1;
    }

    k_shared_memory_ptr = MapViewOfFile(k_shared_memory_handle,
                                        FILE_MAP_ALL_ACCESS,
                                        0,
                                        0,
                                        EL_SHARED_MEMORY_SIZE);
    if (nullptr == k_shared_memory_ptr) {
        return -1;
    }

    k_producer_event = CreateEvent(NULL,
                                   FALSE, // auto reset
                                   FALSE,
                                   EL_EVENT_PRODUCER_NAME);

    k_consumer_event = CreateEvent(NULL,
                                   FALSE, // auto reset
                                   FALSE,
                                   EL_EVENT_CONSUMER_NAME);

    if (nullptr == k_producer_event || nullptr == k_consumer_event
        || INVALID_HANDLE_VALUE == k_producer_event || INVALID_HANDLE_VALUE == k_consumer_event) {
        return -1;
    }

    return 0;
}

inline void el_proxy_deinit()
{
    CloseHandle(k_producer_event);
    CloseHandle(k_consumer_event);
    if (k_shared_memory_ptr)
        UnmapViewOfFile(k_shared_memory_ptr);
    CloseHandle(k_shared_memory_handle);
}