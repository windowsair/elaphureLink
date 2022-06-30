/**************************************************************************/ /**
 *           TracePointDefs.h: TracePoint related definitions used across
 *                             multiple modules.
 *
 * @version  V1.0.0
 * @date     $Date: 2016-10-17 12:51:38 +0200 (Mon, 17 Oct 2016) $
 *
 * @note
 * Copyright (C) 2016 ARM Limited. All rights reserved.
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

#ifndef __TRACEPOINTDEFS_H__
#define __TRACEPOINTDEFS_H__

// Tracepoint Types
#define TP_TYPE_RUN  0x1 // Trace Run
#define TP_TYPE_SUSP 0x2 // Trace Suspend
#define TP_TYPE_HALT 0x3 // Trace Halt
#define TP_TYPE_DATA 0x4 // Trace Data Point
#define TP_TYPE_ACC  0x5 // Trace Access Point


// Tracepoint Access Types
#define TP_ACC_TYPE_CODE 0x0 // Code Access
#define TP_ACC_TYPE_RD   0x1 // Data Read Access
#define TP_ACC_TYPE_WR   0x2 // Data Write Access
#define TP_ACC_TYPE_RW   0x3 // Data Read/Write Access


// Tracepoint Extended Info Usage (breakpoint structure)
#define TP_INFO     ExtInf0 // Additional tracepoint info
#define TP_DATA     ExtInf2 // Data value on data match tracepoint
#define TP_TYPE     ExtInf3 // Tracepoint type
#define TP_EXT_PTR  ExtInf4 // Extension pointer for complex trace halt condition
#define TP_HALT_OP  ExtInf5 // Operators used for trace halt conditions
#define TP_ORG_ADDR ExtInf6 // The original address if tracepoint was modified to be aligned
#define TP_ORG_LEN  ExtInf7 // The original range length if tracepoint was modified to be aligned

// Additional Tracepoint Info Flags (TP_INFO)
#define TP_INFO_WVAL (1UL << 0) // TP_DATA carries value for data match
#define TP_INFO_WMOD (1UL << 1) /* The original address range has been modified,
                                        TP_ORG_ADDR and TP_ORG_LEN hold valid information. */

// Trace Halt Operator Flags
#define TP_HALT_OP_NOT (1UL << 0) // DWT output for the halt (sub)-condition is inverted
#define TP_HALT_OP_AND (1UL << 1) // Logic operator to combine this subcondition with \
                                  // the subcondition in TP_EXT_PTR

#endif //__TRACEPOINTDEFS_H__