#include "pch.h"
#include "ElaphureLinkRDDIContext.h"

#include "../common/git_info.hpp"


ElaphureLinkRDDIContext kContext;

HANDLE       k_shared_memory_handle = nullptr;
el_memory_t *k_shared_memory_ptr    = nullptr;

HANDLE k_producer_event = nullptr;
HANDLE k_consumer_event = nullptr;

inline int  el_rddi_init();
inline void el_rddi_deinit();

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            return el_rddi_init();
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            el_rddi_deinit();
            break;
    }
    return TRUE;
}


inline int el_rddi_init()
{
    k_shared_memory_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS,
                                             FALSE,
                                             EL_SHARED_MEMORY_NAME);

    if (nullptr == k_shared_memory_handle || INVALID_HANDLE_VALUE == k_shared_memory_handle) {
        return FALSE;
    }

    void *ptr = MapViewOfFile(k_shared_memory_handle,
                              FILE_MAP_ALL_ACCESS,
                              0,
                              0,
                              EL_SHARED_MEMORY_SIZE);

    k_shared_memory_ptr = static_cast<el_memory_t *>(ptr);
    if (nullptr == k_shared_memory_ptr) {
        return FALSE;
    }


    k_producer_event = OpenEvent(EVENT_ALL_ACCESS,
                                 FALSE,
                                 EL_EVENT_PRODUCER_NAME);

    if (nullptr == k_producer_event
        || INVALID_HANDLE_VALUE == k_producer_event) {
        return FALSE;
    }

    k_consumer_event = OpenEvent(EVENT_ALL_ACCESS,
                                 FALSE,
                                 EL_EVENT_CONSUMER_NAME);

    if (nullptr == k_consumer_event
        || INVALID_HANDLE_VALUE == k_consumer_event) {
        return FALSE;
    }

    return TRUE;
}

void el_rddi_deinit()
{
    CloseHandle(k_producer_event);
    CloseHandle(k_consumer_event);
    if (k_shared_memory_ptr != nullptr)
        UnmapViewOfFile(k_shared_memory_ptr);
    CloseHandle(k_shared_memory_handle);
}
