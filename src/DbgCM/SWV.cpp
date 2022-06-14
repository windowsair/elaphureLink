/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.1.2
 * @date     $Date: 2016-03-24 09:07:53 +0100 (Thu, 24 Mar 2016) $
 *
 * @note
 * Copyright (C) 2009-2015 ARM Limited. All rights reserved.
 *
 * @brief     Low Level Layer for Serial Wire Viewer Interface
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

#include "stdafx.h"
#include "..\AGDI.h"
#include "Collect.h"
#include "Trace.h"
#include "SWV.h"


/* SWV Data */
BYTE  SWV_DataBuf[SWV_DATACNT]; // Buffer
DWORD SWV_DataHead;             // Head Pointer
DWORD SWV_DataTail;             // Tail Pointer


// SWV Trace Check Baudrate
//   brate  : Baudrate
//   return : 0 - OK,  else error code
int SWV_Check (DWORD brate) {

//...
  DEVELOP_MSG("Todo: \nImplement Function: int SWV_Check (DWORD brate)");
//return (EU17);                // Selected Trace Clock not supported
//return (EU19);                // Selected Trace Port is not supported (TraceConf.Protocol)
  return (0);
}


// SWV Trace Setup
//   brate  : Baudrate (0 to turn off)
//   return : 0 - OK,  else error code
int SWV_Setup (DWORD brate) {

//...
  DEVELOP_MSG("Todo: \nImplement Function: int SWV_Setup (DWORD brate)");
//return (EU17);                // Selected Trace Clock not supported
//return (EU19);                // Selected Trace Port is not supported (TraceConf.Protocol)
  return (0);
}


// SWV Trace Flush
//   return : 0 - OK,  else error code
int SWV_Flush (void) {

//...
  DEVELOP_MSG("Todo: \nImplement Function: int SWV_Flush (void)");
  return (0);
}


// SWV Trace Read
//   time   : time in ms
//   return : 0 - OK,  else error code
int SWV_Read (DWORD time) {

//...
  DEVELOP_MSG("Todo: \nImplement Function: int SWV_Read (DWORD time)");
  return (0);
}

void InitSWV() {
  memset(SWV_DataBuf, 0, sizeof(SWV_DataBuf));  // Buffer
  SWV_DataHead = 0;                             // Head Pointer
  SWV_DataTail = 0;                             // Tail Pointer
}
