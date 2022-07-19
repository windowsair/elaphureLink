#include "pch.h"
#include <iostream>
static_assert(sizeof(int *) == 4, "should be x86 architecture");


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
            return TRUE;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            ipc_source_deinit();
            break;
    }
    return TRUE;
}


EL_DLL_FUNCTION int ipc_source_init()
{
    kSharedMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS,
                                          FALSE,
                                          EL_SHARED_MEMORY_NAME);

    if (nullptr == kSharedMemoryHandle || INVALID_HANDLE_VALUE == kSharedMemoryHandle) {
        return FALSE;
    }

    kSharedMemoryPtr = MapViewOfFile(kSharedMemoryHandle,
                                     FILE_MAP_ALL_ACCESS,
                                     0,
                                     0,
                                     EL_SHARED_MEMORY_SIZE);
    if (nullptr == kSharedMemoryPtr) {
        return FALSE;
    }


    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

    SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle       = FALSE;
    sa.lpSecurityDescriptor = &sd;



    kProducerEvent = OpenEvent(EVENT_ALL_ACCESS,
                               FALSE,
                               EL_EVENT_PRODUCER_NAME);

    if (nullptr == kProducerEvent
        || INVALID_HANDLE_VALUE == kProducerEvent) {
        printf("\nerror %d", GetLastError());
        return FALSE;
    }

    kConsumerEvent = OpenEvent(EVENT_ALL_ACCESS,
                               FALSE,
                               EL_EVENT_CONSUMER_NAME);

    if (nullptr == kConsumerEvent
        || INVALID_HANDLE_VALUE == kConsumerEvent) {
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

EL_DLL_FUNCTION void *get_ipc1_shared_memory_ptr()
{
    return kSharedMemoryPtr;
}
EL_DLL_FUNCTION int ipc1_main_thread(int end)
{
    DWORD        waitResult;
    uint32_t     i       = 0;
    el_memory_t *pBuffer = static_cast<el_memory_t *>(get_ipc1_shared_memory_ptr());
    while (1) {
        pBuffer->a = i;
        SetEvent(kProducerEvent);

        waitResult = WaitForSingleObject(kConsumerEvent, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
            return -1;
        }

        if (pBuffer->b != i) {
            printf("\nipc1 recv data failed. Expect: %d, Actual: %d\n", i, pBuffer->b);
            return -1;
        }

        i++;
        if (i > end) {
            break;
        }
    }
    return 0;
}