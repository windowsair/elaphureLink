#ifndef PCH_H
#define PCH_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "framework.h"

#include "rddi_dap.h"
#include "rddi_dap_cmsis.h"
#include "rddi_dap_jtag.h"
#include "rddi_dap_swo.h"

#include "../common/ipc_common.hpp"
#include "../common/dap.hpp"


extern HANDLE       k_shared_memory_handle;
extern el_memory_t *k_shared_memory_ptr;

extern HANDLE k_producer_event;
extern HANDLE k_consumer_event;


#define EL_SHOW_WARNING_MSG_BOX(msg, title) MessageBox(NULL, const_cast<LPCSTR>(msg), const_cast<LPCSTR>(title), MB_ICONWARNING | MB_SYSTEMMODAL)


#endif //PCH_H
