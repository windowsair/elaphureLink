/*
 * rddi_dap_cmsis.h - Debug header for CMSIS-DAP
 * Copyright (C) 2012 ARM Limited. All rights reserved.
 */

#ifndef RDDI_DAP_CMSIS_H
#define RDDI_DAP_CMSIS_H


// RDDI.h defines the dll exports etc.
#include "rddi.h"


// Identifiers (DAP_Identify)
#define RDDI_CMSIS_DAP_ID_VENDOR        1
#define RDDI_CMSIS_DAP_ID_PRODUCT       2
#define RDDI_CMSIS_DAP_ID_SER_NUM       3
#define RDDI_CMSIS_DAP_ID_FW_VER        4
#define RDDI_CMSIS_DAP_ID_DEVICE_VENDOR 5
#define RDDI_CMSIS_DAP_ID_DEVICE_NAME   6


// DAP Capabilities
#ifndef BIT
#define BIT(x) (1UL << (x))
#endif

#define INFO_CAPS_SWD                 BIT(0)
#define INFO_CAPS_JTAG                BIT(1)
#define INFO_CAPS_SWO_UART            BIT(2)
#define INFO_CAPS_SWO_MANCHESTER      BIT(3)
#define INFO_CAPS_ATOMIC_CMDS         BIT(4)
#define INFO_CAPS_TEST_DOMAIN_TIMER   BIT(5)
#define INFO_CAPS_SWO_STREAMING_TRACE BIT(6)
#define INFO_CAPS_UART_PORT           BIT(7)
#define INFO_CAPS_USB_COM_PORT        BIT(8)
#define INFO_CAPS__NUM_CAPS           9



/*!
 *
 * Detect DAP interfaces.
 *
 * Must be present in all levels of implementations
 *
 * @param[in] handle Implementation specific opaque pointer.
 * @param[out] noOfIFs will be filled with the number of detected interfaces.
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_Detect(const RDDIHandle handle, int *noOfIFs);

/*!
 *
 * Identify specified DAP interface.  Must be called after DAP_Detect and provides specified
 * string identifiers.
 *
 * Must be present in all levels of implementations
 *
 * @param[in] handle Implementation specific opaque pointer.
 * @param[in] ifNo interface number (0 .. noOfIFs-1)
 * @param[in] idNo identifier number (RDDI_CMSIS_DAP_ID_xxx)
 * @param[out] str a buffer that will be filled with the indentification string
 * @param[in] len the length of the str buffer in bytes
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_Identify(const RDDIHandle handle, int ifNo, int idNo, char *str, const int len);


/*!
 * Configure the interface with the settings it needs, e.g. "Port=SW;Clock=1000000;SWJ=Y"
 *
 * @param[in] handle Implementation specific opaque pointer.
 * @param[in] ifNo interface number (0 .. noOfIFs-1)
 * @param[in] str CMSIS_DAP configuration string
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_ConfigureInterface(const RDDIHandle handle, int ifNo, char *str);

/*!
 * Inspect the connected CMSIS_DAP interface to get the no of DAPs accessible via it
 *
 * @param[in] handle Implementation specific opaque pointer.
 * @param[out] noOfDAPs the number of DAPs accessible on this interface
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_DetectNumberOfDAPs(const RDDIHandle handle, int *noOfDAPs);

/*!
 * Inspect the connected CMSIS_DAP interface to the get the DAP ID for each DAP accessible via this interface
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[out] DAP_ID_Array will be filled with the IDs for the DAPs in the configure/connected system.  Must be big enough to hold the list.
 * Size should be at leat the number returned by DAP_GetNumberOfDAPs * sizeof(int).
 * @param[in] sizeOfArray size of the DAP_ID_Array parameter in bytes
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_DetectDAPIDList(const RDDIHandle handle, int *DAP_ID_Array, size_t sizeOfArray);

/*!
 * Execute CMSIS_DAP protocol commands on connected interface
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[in] num number of commands
 * @param[in] request array of command requests
 * @param[in] req_len array of command requests length
 * @param[out] response array of command responses
 * @param[out] resp_len array of command responses length
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_Commands(const RDDIHandle handle, int num, unsigned char **request, int *req_len, unsigned char **response, int *resp_len);



/**
 * @brief Configure the interface with the settings it needs, e.g. "SWJSwitch=0xE79E"
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[in] str CMSIS_DAP configuration string
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_ConfigureDAP(const RDDIHandle handle, const char *str);

/**
 * @brief Get the GUID of DAP hardware interface(like USB vid, pid, class GUID)
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[in] ifNo interface number (0 .. noOfIFs-1)
 * @param[out] str a buffer that will be filled with the GUID string
 * @param[in] len the length of the str buffer in bytes
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_GetGUID(const RDDIHandle handle, int ifNo, char *str, const int len);


/**
 * @brief Obtains information about the available interface to the DAP device
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[in] ifNo interface number (0 .. noOfIFs-1)
 * @param[out] cap_info information about the interfaces available to the device
 *      Bit 0: 1 = SWD Serial Wire Debug communication is implemented (0 = SWD Commands not implemented).
 *      Bit 1: 1 = JTAG communication is implemented (0 = JTAG Commands not implemented).
 *      Bit 2: 1 = SWO UART - UART Serial Wire Output is implemented (0 = not implemented).
 *      Bit 3: 1 = SWO Manchester - Manchester Serial Wire Output is implemented (0 = not implemented).
 *      Bit 4: 1 = Atomic Commands - Atomic Commands support is implemented (0 = Atomic Commands not implemented).
 *      Bit 5: 1 = Test Domain Timer - debug unit support for Test Domain Timer is implemented (0 = not implemented).
 *      Bit 6: 1 = SWO Streaming Trace is implemented (0 = not implemented).
 *      Bit 7: 1 = UART Communication Port is implemented (0 = not implemented).
 *      Bit 8: 1 = USB COM Port is implemented (0 = not implemented).
 *
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_FUNC int CMSIS_DAP_Capabilities(const RDDIHandle handle, int ifNo, int *cap_info);

#endif
