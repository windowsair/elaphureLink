#include "pch.h"
#include <iostream>

static_assert(sizeof(int *) == 8, "should be x64 architecture");

HANDLE kSharedMemoryHandle = nullptr;
void * kSharedMemoryPtr    = nullptr;

HANDLE kProducerEvent = nullptr;
HANDLE kConsumerEvent = nullptr;


void ipc_source_deinit();


BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            return ipc_source_init();
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            ipc_source_deinit();
            break;
    }
    return TRUE;
}


EL_DLL_FUNCTION BOOL ipc_source_init()
{
    kSharedMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                            NULL,
                                            PAGE_READWRITE,
                                            0,
                                            EL_SHARED_MEMORY_SIZE,
                                            EL_SHARED_MEMORY_NAME);
    if (nullptr == kSharedMemoryHandle || INVALID_HANDLE_VALUE == kSharedMemoryHandle) {
        printf("\nerror %d", GetLastError());
        return FALSE;
    }

    kSharedMemoryPtr = MapViewOfFile(kSharedMemoryHandle,
                                     FILE_MAP_ALL_ACCESS,
                                     0,
                                     0,
                                     EL_SHARED_MEMORY_SIZE);
    if (nullptr == kSharedMemoryPtr) {
        printf("\nerror %d", GetLastError());
        return FALSE;
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
        printf("\nerror %d", GetLastError());
        return FALSE;
    }


    return TRUE;
}


void ipc_source_deinit()
{
    CloseHandle(kProducerEvent);
    CloseHandle(kConsumerEvent);
    if (kSharedMemoryPtr)
        UnmapViewOfFile(kSharedMemoryPtr);
    CloseHandle(kSharedMemoryHandle);
}


EL_DLL_FUNCTION void *get_ipc2_shared_memory_ptr()
{
    return kSharedMemoryPtr;
}


EL_DLL_FUNCTION int ipc2_main_thread(int end)
{
    DWORD        waitResult;
    uint32_t     i       = 0;
    el_memory_t *pBuffer = static_cast<el_memory_t *>(get_ipc2_shared_memory_ptr());
    while (1) {
        waitResult = WaitForSingleObject(kProducerEvent, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
            return -1;
        }

        if (pBuffer->a != i) {
            printf("\nipc2 recv data failed. Expect: %d, Actual: %d\n", i, pBuffer->a);
            return -1;
        }

        pBuffer->b = i;
        i++;
        SetEvent(kConsumerEvent);
        if (i % 10000 == 0) {
            printf("%d ", i);
        }


        if (i > end) {
            break;
        }
    }
    return 0;
}