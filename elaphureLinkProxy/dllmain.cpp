#include "pch.h"

bool KIsProxyInit = false;

HANDLE kSharedMemoryHandle = nullptr;
void * kSharedMemoryPtr    = nullptr;

HANDLE kProducerEvent = nullptr;
HANDLE kConsumerEvent = nullptr;

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
    if (KIsProxyInit) {
        return 0;
    }
    kSharedMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                            NULL,
                                            PAGE_READWRITE,
                                            0,
                                            EL_SHARED_MEMORY_SIZE,
                                            EL_SHARED_MEMORY_NAME);
    if (nullptr == kSharedMemoryHandle || INVALID_HANDLE_VALUE == kSharedMemoryHandle) {
        return -1;
    }

    kSharedMemoryPtr = MapViewOfFile(kSharedMemoryHandle,
                                     FILE_MAP_ALL_ACCESS,
                                     0,
                                     0,
                                     EL_SHARED_MEMORY_SIZE);
    if (nullptr == kSharedMemoryPtr) {
        return -1;
    }

    kProducerEvent = CreateEvent(NULL,
                                 FALSE, // auto reset
                                 FALSE,
                                 EL_EVENT_PRODUCER_NAME);

    kConsumerEvent = CreateEvent(NULL,
                                 FALSE, // auto reset
                                 FALSE,
                                 EL_EVENT_CONSUMER_NAME);

    if (nullptr == kProducerEvent || nullptr == kConsumerEvent
        || INVALID_HANDLE_VALUE == kProducerEvent || INVALID_HANDLE_VALUE == kConsumerEvent) {
        return -1;
    }

    return 0;
}

inline void el_proxy_deinit()
{
    CloseHandle(kProducerEvent);
    CloseHandle(kConsumerEvent);
    if (kSharedMemoryPtr)
        UnmapViewOfFile(kSharedMemoryPtr);
    CloseHandle(kSharedMemoryHandle);
}