#pragma once

#ifndef RDDI_DAP_JTAG_H
#define RDDI_DAP_JTAG_H

#include "rddi.h"

int rddi_cmsis_dap_probe_jtag_device(const RDDIHandle handle, int *noOfDAPs);


#endif