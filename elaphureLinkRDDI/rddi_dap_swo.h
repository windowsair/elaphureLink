#pragma once

#ifndef RDDI_DAP_SWO_H
#define RDDI_DAP_SWO_H

#include "rddi.h"

/**
 * @brief Set SWO communication baud rate
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[in] baudrate baudrate to be set
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_EXPORT int CMSIS_DAP_SWO_Baudrate(const RDDIHandle handle, int baudrate);


/**
 * @brief Start or stop the SWO function
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[in] control start or stop the SWO function
 *            0 - Stop
 *            1 - Start
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_EXPORT int CMSIS_DAP_SWO_Control(const RDDIHandle handle, int control);


/**
 * @brief Get the status of SWO and the number of available bytes in the Trace Buffer that have not yet been read
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[out] count number of bytes in Trace Buffer (not yet read)
 * @param[out] status SWO status
 *             Bit 0: Trace Capture (1 - active, 0 - inactive)
 *             Bit 6: Trace Stream Error
 *             Bit 7: Trace Buffer Overrun
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_EXPORT int CMSIS_DAP_SWO_Status(const RDDIHandle handle, int *count, int *status);


/**
 * @brief Get the SWO data and fill the buffer with it
 *
 * @param[in] handle opaque pointer - obtained from DAP_Open call
 * @param[out] num_written number of bytes written to SWO buffer
 * @param[out] buffer a buffer that will be filled with the swo data
 * @param[out] status SWO status
 *             Bit 0: Trace Capture (1 - active, 0 - inactive)
 *             Bit 6: Trace Stream Error
 *             Bit 7: Trace Buffer Overrun
 * @return RDDI_SUCCESS on success, other on fail
 */
RDDI_EXPORT int CMSIS_DAP_SWO_Data(const RDDIHandle handle, int *num_written, void *buffer, int *status);



#endif