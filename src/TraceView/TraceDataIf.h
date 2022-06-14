/**************************************************************************//**
 *           TraceDataIf.h: Function declarations for Trace Data Interface
 * 
 * @version  V2.0.0
 * @date     $Date: 2015-04-28 15:40:26 +0200 (Tue, 28 Apr 2015) $
 *
 * @note
 * Copyright (C) 2011-2015 ARM Limited. All rights reserved.
 * 
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef __TRACEDATAIF_H__
#define __TRACEDATAIF_H__

#include "TraceDataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// ***DEPRECATED***
/* SetTraceConfig() : Modify the trace settings during runtime. Optional (not supported in first release).
*/
INT32 SetTraceConfig(TRACE_CONFIG* pTraceConfig);  

// ***DEPRECATED***
/* GetTraceConfig() : Get current trace configuration, e.g. the clock settings.
*/
INT32 GetTraceConfig(TRACE_CONFIG* pTraceConfig);  // OBSOLETE


/* GetFirstCycle() : Get cycle information for the oldest available trace record.

   Parameters      : nFirstCyle - [OUT] : Cycle value of the oldest available record.

   Return value    : Error code if the function call failed, '0' if succeeded.

   Error Codes     : TDI_ERR_SUCCESS       - Function call succeeded.
                     TDI_ERR_INVALID_PARAM - Invalid parameter (nLastCycle == NULL).
                     TDI_ERR_EMPTY         - No trace records available in entire storage.
*/
INT32 GetFirstCycle(UINT64 *nFirstCycle);


/* GetLastCycle()  : Get cycle information for the latest available trace record.

   Parameter       : nLastCycle - [OUT] : Cycle value of the latest available record.
                                  
   Return Value    : Error code if the function call failed, '0' if succeeded.

   Error Codes     : TDI_ERR_SUCCESS       - Function call succeeded.
                     TDI_ERR_INVALID_PARAM - Invalid parameter (nLastCycle == NULL).
                     TDI_ERR_EMPTY         - No trace records available in entire storage.
*/
INT32 GetLastCycle(UINT64 *nLastCycle);


// ***DEPRECATED***
/* GetTraceRecords() : Requests a number of trace records within a specified range of cycles (pTrace != NULL).
                       Returns trace record count or cycle boundaries within the specified range (if pTrace == NULL).

    Parameters       : nFirstCycle - [IN]  : Start of requested cycle range (inclusive).
                                     [OUT] : Cycle of first trace record in requested range.
                                     Notes : - Cycle values of records returned in pTrace are relative to the input value
                                               of nFirstCycle.
                                             - nFirstCycle must not be NULL.

                       nLastCycle  - [IN]  : End of requested cycle range (inclusive).
                                     [OUT] : Cycle of last trace record in requested range. Takes nCount into account.
                                     Notes : - nLastCycle must not be NULL.

                       nCount      - [IN]  : Maximum number of records to return for requested cycle range (pTrace != NULL).
                                             pTrace == NULL: - Set nCount == 0 to obtain the number of ALL records within
                                                               specified cycle range.
                                                             - Set nCount != 0 to obtain cycle boundaries for requested number
                                                               of records within requested cycle range.
                                     [OUT] : Number of actually returned records (pTrace != NULL).
                                             Number of records in requested cycle range (pTrace == NULL).
                                     Notes : - nCount must not be NULL.

                       pTrace      - [OUT] : Buffer for requested records. Must be large enough to store *nCount records.

    Return Value     : Error code if the function call failed, '0' if succeeded.

    Error Codes      :  TDI_ERR_SUCCESS       - Function call succeeded.
                        TDI_ERR_INVALID_PARAM - Invalid parameter (nFirstCycle == NULL, nLastCycle == NULL, or nCount == NULL).
                        TDI_ERR_BOUNDS        - Requested cycle range is outside the available record range.
                        TDI_ERR_EMPTY         - No trace records available in requested range.

    Usage: 1. Call GetTraceRecords() with pTrace == NULL to find out the number of expected records.
              Allocate record buffer with sufficient size and use it in subsequent GetTraceRecords() call
              with pTrace != NULL.

           2. Obtain ALL records from a certain start cycle with the following settings
              - *nLastCycle = (UINT64)(-1); // Largest possible value.
              - *nCount     = (UINT64)(-1); // Largest possible value.
              - pTrace      = NULL;         // Indicate record number/cycle boundary request without data.

           3. Store input value of *nFirstCycle at calling level. Value has to be used to reconstruct the absolute
              cycle count at this level.
*/
INT32 GetTraceRecords(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, TRACE_RECORD *pTrace);  // DEPRECATED, USE GetTraceRecords64()

// ***DEPRECATED***
/* GetFiltTraceRecords() : Requests a number of trace records within a specified range of cycles (pTrace != NULL).
                           Returns trace record count or cycle boundaries within the specified range (if pTrace == NULL).

    Parameters           : nFirstCycle - [IN]  : Start of requested cycle range (inclusive).
                                         [OUT] : Cycle of first trace record in requested range after adding requested record offset.
                                         Notes : - Cycle values of records returned in pTrace are relative to the input value
                                                   of nFirstCycle.
                                                 - nFirstCycle must not be NULL.

                           nLastCycle  - [IN]  : End of requested cycle range (inclusive).
                                         [OUT] : Cycle of last trace record in requested range. Takes nCount into account.
                                         Notes : - nLastCycle must not be NULL.

                           nCount      - [IN]  : Maximum number of records to return for requested cycle range (pTrace != NULL).
                                                 pTrace == NULL: - Set nCount == 0 to obtain the number of ALL records within
                                                                   specified cycle range.
                                                                 - Set nCount != 0 to obtain cycle boundaries for requested number
                                                                   of records within requested cycle range.
                                         Notes : - nCount must not be NULL.

                           nRecordOfs  - [IN]  : Record offset into requested cycle range.
                                         Notes : - Record offset must be taken into account for both requesting data and
                                                   requesting count/boundary cycle information only.
                                                 - nRecordOfs must not be NULL.
                                               
                           pTrace      - [OUT] : Buffer for requested records. Must be large enough to store *nCount records.

    Return Value         : Error code if the function call failed, '0' if succeeded.

    Error Codes          :  TDI_ERR_SUCCESS       - Function call succeeded.
                            TDI_ERR_INVALID_PARAM - Invalid parameter (nFirstCycle == NULL, nLastCycle, or nCount are NULL-pointers.
                            TDI_ERR_BOUNDS        - Requested cycle range is outside the available record range.
                            TDI_ERR_EMPTY         - No trace records available in requested range (taking nRecordOfs into account).

    Usage: 1. Call GetFiltTraceRecords() with pTrace == NULL to find out the number of expected records.
              Allocate record buffer with sufficient size and use it in subsequent GetFiltTraceRecords() call
              with pTrace != NULL.

           2. Obtain ALL records from a certain start cycle with the following settings
              - *nLastCycle = (UINT64)(-1); // Largest possible value.
              - *nCount     = (UINT64)(-1); // Largest possible value.
              - *nRecordOfs = 0;
              - pTrace      = NULL;         // Indicate record number/cycle boundary request without data.

           3. Store input value of *nFirstCycle at calling level. Value has to be used to reconstruct the absolute
              cycle count at this level.
			  
           4. GetFiltTraceRecords() is supposed to simplify the usage from the upper layers. It basically allows
              to switch from a time-based approach to an index-based approach within a fixed cycle range.
*/
INT32 GetFiltTraceRecords(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, UINT64 *nRecordOfs,  // DEPRECATED, USE GetFiltTraceRecords64()
                                 TRACE_FILTER_CONF* pTraceFilter, TRACE_RECORD *pTrace);

/* ClearTrace() : Clear current trace buffer in debug driver DLL.
*/
INT32 ClearTrace(void);


/* GetTraceRecords64() : Requests a number of trace records (64-bit cycle count format) within a specified range of cycles (pTrace64 != NULL).
                         Returns trace record count or cycle boundaries within the specified range (if pTrace64 == NULL).

    Parameters         : nFirstCycle - [IN]  : Start of requested cycle range (inclusive).
                                       [OUT] : Cycle of first trace record in requested range.
                                       Notes : - Cycle values of records returned in pTrace are relative to the input value
                                                 of nFirstCycle.
                                               - nFirstCycle must not be NULL.

                         nLastCycle  - [IN]  : End of requested cycle range (inclusive).
                                       [OUT] : Cycle of last trace record in requested range. Takes nCount into account.
                                       Notes : - nLastCycle must not be NULL.

                         nCount      - [IN]  : Maximum number of records to return for requested cycle range (pTrace64 != NULL).
                                               pTrace64 == NULL: - Set nCount == 0 to obtain the number of ALL records within
                                                                   specified cycle range.
                                                                 - Set nCount != 0 to obtain cycle boundaries for requested number
                                                                   of records within requested cycle range.
                                       [OUT] : Number of actually returned records (pTrace64 != NULL).
                                               Number of records in requested cycle range (pTrace64 == NULL).
                                       Notes : - nCount must not be NULL.

                         pTrace64    - [OUT] : Buffer for requested records. Must be large enough to store *nCount records.

    Return Value       : Error code if the function call failed, '0' if succeeded.

    Error Codes        :  TDI_ERR_SUCCESS       - Function call succeeded.
                          TDI_ERR_INVALID_PARAM - Invalid parameter (nFirstCycle == NULL, nLastCycle == NULL, or nCount == NULL).
                          TDI_ERR_BOUNDS        - Requested cycle range is outside the available record range.
                          TDI_ERR_EMPTY         - No trace records available in requested range.

    Usage: 1. Call GetTraceRecords64() with pTrace64 == NULL to find out the number of expected records.
              Allocate record buffer with sufficient size and use it in subsequent GetTraceRecords64() call
              with pTrace64 != NULL.

           2. Obtain ALL records from a certain start cycle with the following settings
              - *nLastCycle = (UINT64)(-1); // Largest possible value.
              - *nCount     = (UINT64)(-1); // Largest possible value.
              - pTrace64    = NULL;         // Indicate record number/cycle boundary request without data.

           3. Store input value of *nFirstCycle at calling level. Value has to be used to reconstruct the absolute
              cycle count at this level.
*/
INT32 GetTraceRecords64(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, TRACE_RECORD_64 *pTrace64);

/* GetFiltTraceRecords64() : Requests a number of trace records (64-bit cycle count format) within a specified range of cycles (pTrace64 != NULL).
                             Returns trace record count or cycle boundaries within the specified range (if pTrace64 == NULL).

    Parameters             : nFirstCycle - [IN]  : Start of requested cycle range (inclusive).
                                           [OUT] : Cycle of first trace record in requested range after adding requested record offset.
                                           Notes : - Cycle values of records returned in pTrace are relative to the input value
                                                     of nFirstCycle.
                                                   - nFirstCycle must not be NULL.

                             nLastCycle  - [IN]  : End of requested cycle range (inclusive).
                                           [OUT] : Cycle of last trace record in requested range. Takes nCount into account.
                                           Notes : - nLastCycle must not be NULL.

                             nCount      - [IN]  : Maximum number of records to return for requested cycle range (pTrace64 != NULL).
                                                   pTrace64 == NULL: - Set nCount == 0 to obtain the number of ALL records within
                                                                       specified cycle range.
                                                                     - Set nCount != 0 to obtain cycle boundaries for requested number
                                                                       of records within requested cycle range.
                                           Notes : - nCount must not be NULL.

                             nRecordOfs  - [IN]  : Record offset into requested cycle range.
                                           Notes : - Record offset must be taken into account for both requesting data and
                                                     requesting count/boundary cycle information only.
                                                   - nRecordOfs must not be NULL.
                                                 
                             pTrace      - [OUT] : Buffer for requested records. Must be large enough to store *nCount records.

    Return Value           : Error code if the function call failed, '0' if succeeded.

    Error Codes            :  TDI_ERR_SUCCESS       - Function call succeeded.
                              TDI_ERR_INVALID_PARAM - Invalid parameter (nFirstCycle == NULL, nLastCycle, or nCount are NULL-pointers.
                              TDI_ERR_BOUNDS        - Requested cycle range is outside the available record range.
                              TDI_ERR_EMPTY         - No trace records available in requested range (taking nRecordOfs into account).

    Usage: 1. Call GetFiltTraceRecords64() with pTrace64 == NULL to find out the number of expected records.
              Allocate record buffer with sufficient size and use it in subsequent GetFiltTraceRecords64() call
              with pTrace64 != NULL.

           2. Obtain ALL records from a certain start cycle with the following settings
              - *nLastCycle = (UINT64)(-1); // Largest possible value.
              - *nCount     = (UINT64)(-1); // Largest possible value.
              - *nRecordOfs = 0;
              - pTrace64    = NULL;         // Indicate record number/cycle boundary request without data.

           3. Store input value of *nFirstCycle at calling level. Value has to be used to reconstruct the absolute
              cycle count at this level.
			  
           4. GetFiltTraceRecords64() is supposed to simplify the usage from the upper layers. It basically allows
              to switch from a time-based approach to an index-based approach within a fixed cycle range.
*/
INT32 GetFiltTraceRecords64(UINT64 *nFirstCycle, UINT64 *nLastCycle, UINT64 *nCount, UINT64 *nRecordOfs,
                                 TRACE_FILTER_CONF* pTraceFilter, TRACE_RECORD_64 *pTrace64);


/*
 *  UpdateTraceBuffer() : Update the debug driver DLL trace record buffer. This can be used to load unlimited trace
 *                        data or to refresh data from a target buffer (ETB,MTB).
 *  Parameters          :  nFirstCycle - First cycle of the record range to load to the buffer (Unlimited Trace).
 *                                       Ignored for ETB/MTB, can be set to NULL.
 *                         nLastCycle  - Last cycle of the record range to load to the buffer (Unlimited Trace).
 *                                       Ignored for ETB/MTB, can be set to NULL.
 *
 *  Usage               : UpdateTraceBuffer() can be called with nLastCycle = (UINT64)(-1). In this case, the debug
 *                        driver DLL shall load as much data into the record buffer as possible. This can reduce the
 *                        number of subsequent calls to UpdateTraceBuffer().
 */
INT32 UpdateTraceBuffer(UINT64 *nFirstCycle, UINT64 *nLastCycle);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__TRACEDATAIF_H__