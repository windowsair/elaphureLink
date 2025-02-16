#include "pch.h"

#include "../common/git_info.hpp"

bool k_is_proxy_init = false;

HANDLE       k_shared_memory_handle = nullptr;
el_memory_t *k_shared_memory_ptr    = nullptr;

HANDLE k_producer_event = nullptr;
HANDLE k_consumer_event = nullptr;

struct WindowsVersionNumber k_windows_version_number;

#define DECLARE_DLL_FUNCTION(fn, type, dll) \
    auto fn = reinterpret_cast<type>(GetProcAddress(GetModuleHandleW(L##dll), #fn))


inline void el_proxy_deinit();

inline void fill_el_version_string()
{
    strncpy_s(k_shared_memory_ptr->info_page.version_string, sizeof(k_shared_memory_ptr->info_page.version_string), EL_GIT_TAG_INFO, sizeof(k_shared_memory_ptr->info_page.version_string) - 1);
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
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

inline void get_windows_version_number()
{
    constexpr LONG kStatusSuccess = 0L;
    DECLARE_DLL_FUNCTION(RtlGetVersion, LONG(WINAPI *)(PRTL_OSVERSIONINFOW), "ntdll.dll");
    if (!RtlGetVersion) {
        k_windows_version_number.major_version = 0;
        k_windows_version_number.build_number  = 0;
        return;
    }

    RTL_OSVERSIONINFOW ovi{ sizeof(ovi) };
    if (RtlGetVersion(&ovi) != kStatusSuccess) {
        k_windows_version_number.major_version = 0;
        k_windows_version_number.build_number  = 0;
        return;
    }

    k_windows_version_number.major_version = ovi.dwMajorVersion;
    k_windows_version_number.build_number  = ovi.dwBuildNumber;
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


    void *ptr = MapViewOfFile(k_shared_memory_handle,
                              FILE_MAP_ALL_ACCESS,
                              0,
                              0,
                              EL_SHARED_MEMORY_SIZE);

    k_shared_memory_ptr = static_cast<el_memory_t *>(ptr);

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

    fill_el_version_string();

    get_windows_version_number();

    k_is_proxy_init = true;

    return 0;
}

PROXY_DLL_FUNCTION void el_proxy_change_config(struct el_proxy_config *config)
{
    if (!k_is_proxy_init) {
        if (el_proxy_init())
            return;
    }

    k_shared_memory_ptr->info_page.enable_vendor_command = config->enable_vendor_command;
}

inline void el_proxy_deinit()
{
    CloseHandle(k_producer_event);
    CloseHandle(k_consumer_event);
    if (k_shared_memory_ptr != nullptr)
        UnmapViewOfFile(k_shared_memory_ptr);
    CloseHandle(k_shared_memory_handle);
}