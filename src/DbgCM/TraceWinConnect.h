/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.0
 * @date     $Date: 2015-05-26 14:55:12 +0200 (Tue, 26 May 2015) $
 *
 * @note
 * Copyright (C) 2015 ARM Limited. All rights reserved.
 *
 * @brief     ARM Cortex-M Trace Data Window Connection Definitions
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision
 * and Cortex-M processor based microcontrollers.
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

#ifndef __TRACEWINCONNECT_H__
#define __TRACEWINCONNECT_H__

extern void InitTraceInterface();  // Initialize trace interface and register to uVision
extern void ConfigureTraceWin();   // Set up trace data window

// Functions to update "trace running/processing" state.
//   - Use synchronous variants if trace processing is done in the same thread as the
//     run control thread.
//   - Use asynchronous variants of trace processing is done by thread(s) other than the
//     run control thread.
//   - SetTraceRunSynch/Asynch cover all existing and activated trace streams (here ETM+ITM)
// Parameters:
//   - bState: Processing State. "true" if processing, "false" otherwise.
//   - bWinUpdate: Update contents of concerned windows in uVision
extern void SetTraceRunSynch  (bool bState, bool bWinUpdate);
extern void SetTraceRunAsynch (bool bState, bool bWinUpdate);
extern void SetETMRunSynch    (bool bState, bool bWinUpdate);
extern void SetETMRunAsynch   (bool bState, bool bWinUpdate);
extern void SetITMRunSynch    (bool bState, bool bWinUpdate);
extern void SetITMRunAsynch   (bool bState, bool bWinUpdate);

#endif // __TRACEWINCONNECT_H__
