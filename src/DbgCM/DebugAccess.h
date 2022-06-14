/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.0.1
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016 ARM Limited. All rights reserved.
 *
 * @brief     Debug Access Detection and Recovery
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

#ifndef __DEBUGACCESS_H__
#define __DEBUGACCESS_H__

#include "COLLECT.H"

#include "..\AGDI.H"

// Debug Access Level, also defined in BOM.h
//#define DA_NORMAL      0  // Normal, all target resourcess accessible
//#define DA_ACCESSPORT  1  // AP accesses possible, memory accesses fail
//#define DA_DEBUGPOWER  2  // DP accesses possible, debug system powered up, AP accesses fail or not ready
//#define DA_DEBUGPORT   3  // DP accesses possible, debug system cannot be powered up
//#define DA_INACTIVE    4  // Protocol error happened, debug port seems to be inactive
//#define DA_DBGSERVER   5  // Debug Server for Multiple Debug Connections down. Fatal, cannot recover.

extern int dbgAccLevel;

extern int DebugAccessDetection();  // Detects which kind of debug accesses are currently possible

extern int DebugAccessRecovery();   // Tries to recover from a limited debug access level

extern int DebugAccessEnsure();     // Ensure that debug access is available. Sequence of Detection and Recovery (if required).

extern int ConfigureDebugAccessRecovery(AG_RECOVERY *pRecovery); // Configure Debug Access Recovery

#endif //__DEBUGACCESS_H__