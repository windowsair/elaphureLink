/**
 * @file device_jtag_idcode.cpp
 * @author windowsair (msdn_01@sina.com)
 * @brief device jtag idcode list
 *
 * @copyright BSD-2-Clause
 *
 */

#include "pch.h"
#include "device_jtag_idcode.h"

jtag_idcode_info_t k_jtag_idcode_list[] = {
    { 0x0BA06477, 4 }, // Soc-600
    { 0x0BA07477, 8 }, // Soc-600
    { 0x1BA06477, 4 }, // Soc-600
    { 0x1BA07477, 8 }, // Soc-600
    { 0x2BA06477, 4 }, // Soc-600
    { 0x2BA07477, 8 }, // Soc-600

    { 0x5BA00477, 4 }, // Cortex-M7F
    { 0x5BA02477, 4 }, // Cortex-M7F?
    { 0x6BA00477, 4 }, // Soc-400 Cortex-M7F
    { 0x0BA03477, 8 }, // Soc-400

    { 0x0BA01477, 4 }, // Cortex-M0/M0+
    { 0x0BB11477, 4 }, // Cortex-M0
    { 0x3BA00477, 4 }, // Cortex-M3 r1p1-rel0
    { 0x0BA00477, 4 }, // Cortex-M3 r2p0
    { 0x1BA00477, 4 }, // Cortex-M3
    { 0x2BA00477, 4 }, // Cortex-M3
    { 0x4BA00477, 4 }, // Cortex-M3/M4 DAP-Lite2 CoreSight_DK
    { 0x0BA02477, 4 }, // Cortex-M7
    { 0x0BA05477, 4 }, // Cortex-M23
    { 0x0BA04477, 4 }, // Cortex-M33


    { 0x16410041, 5 }, // STM32F1 Boundary Scan
    { 0x06433041, 5 }, // STM32F401 Boundary Scan
    { 0x06413041, 5 }, // STM32F405 Boundary Scan
    { 0x06458041, 5 }, // STM32F410 Boundary Scan
    { 0x06431041, 5 }, // STM32F411 Boundary Scan
    { 0x06441041, 5 }, // STM32F412 Boundary Scan
    { 0x06463041, 5 }, // STM32F413 Boundary Scan
    { 0x06419041, 5 }, // STM32F427 Boundary Scan
    { 0x06421041, 5 }, // STM32F446 Boundary Scan
    { 0x06434041, 5 }, // STM32F469 Boundary Scan
    { 0x06449041, 5 }, // STM32F7 Boundary Scan
    { 0x06452041, 5 }, // STM32F7 Boundary Scan
    { 0x06451041, 5 }, // STM32F7 Boundary Scan
    { 0x06483041, 5 }, // STM32H7 Boundary Scan
    { 0x06450041, 5 }, // STM32H7 Boundary Scan
    { 0x06480041, 5 }, // STM32H7 Boundary Scan
    { 0x06500041, 5 }, // STM32H7 Boundary Scan
    { 0x06468041, 5 }, // STM32G4 Boundary Scan
    { 0x06469041, 5 }, // STM32G4 Boundary Scan
    { 0x06495041, 5 }, // STM32WB Boundary Scan

    { INVALID_IDCODE, INVALID_IDCODE }
};
