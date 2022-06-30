/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.1
 * @date     $Date: 2015-04-28 15:09:20 +0200 (Tue, 28 Apr 2015) $
 *
 * @note
 * Copyright (C) 2009-2015 ARM Limited. All rights reserved.
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


#ifndef __SWV_H__
#define __SWV_H__


/* SWV Trace Data */
#define SWV_DATACNT 8192               // Max. Items
extern BYTE  SWV_DataBuf[SWV_DATACNT]; // Buffer
extern DWORD SWV_DataHead;             // Head Pointer
extern DWORD SWV_DataTail;             // Tail Pointer


// SWV Trace Check Baudrate
//   brate  : Baudrate
//   return : 0 - OK,  else error code
extern int SWV_Check(DWORD brate);

// SWV Trace Setup
//   brate  : Baudrate (0 to turn off)
//   return : 0 - OK,  else error code
extern int SWV_Setup(DWORD brate);

// SWV Trace Flush
//   return : 0 - OK,  else error code
extern int SWV_Flush(void);

// SWV Trace Read
//   time   : time in ms
//   return : 0 - OK,  else error code
extern int SWV_Read(DWORD time);


#endif
