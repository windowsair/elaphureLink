#pragma once

#include <cstddef>
#include <stdint.h>

#ifdef ELL_DLL_USE_IMPORT
#define EL_DLL_FUNCTION extern "C" __declspec(dllimport)
#else
#define EL_DLL_FUNCTION extern "C" __declspec(dllexport)
#endif


#define EL_SHARED_MEMORY_NAME  "elamemory"
#define EL_SHARED_MEMORY_SIZE  4096 * 1000

#define EL_EVENT_PRODUCER_NAME "elaproducer"
#define EL_EVENT_CONSUMER_NAME "elaconsumer"

EL_DLL_FUNCTION void *get_ipc1_shared_memory_ptr();
EL_DLL_FUNCTION void *get_ipc2_shared_memory_ptr();
EL_DLL_FUNCTION int   ipc1_main_thread(int end);
EL_DLL_FUNCTION int   ipc2_main_thread(int end);
EL_DLL_FUNCTION int   ipc_source_init();



typedef struct el_memory_ {
    uint32_t a;
    uint32_t b;
} el_memory_t;

#ifdef __cplusplus
static_assert(offsetof(el_memory_t, a) == 0, "wrong");
static_assert(offsetof(el_memory_t, b) == 4, "wrong");
#endif