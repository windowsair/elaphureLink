/**************************************************************************/ /**
 *           agdi_types.h: Data Types for AGDI Interface
 *
 * @version  V1.0.0
 * @date     $Date: 2015-04-28 15:09:20 +0200 (Tue, 28 Apr 2015) $
 *
 * @note
 * Copyright (C) 1999-2009 KEIL, 2009-2015 ARM Limited. All rights reserved.
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

#ifndef __GDI_TYPES__
#define __GDI_TYPES__


/*
 * Advanced GDI types
 */
#ifdef U8
#undef U8
#endif

#ifdef U16
#undef U16
#endif

#ifdef U32
#undef U32
#endif

#ifdef U64
#undef U64
#endif

#ifdef I8
#undef I8
#endif

#ifdef I16
#undef I16
#endif

#ifdef I32
#undef I32
#endif

#ifdef I64
#undef I64
#endif


typedef unsigned long      UL32;
typedef signed long        SL32;
typedef signed char        SC8;
typedef unsigned char      UC8;
typedef signed int         I32;
typedef unsigned int       U32;
typedef signed short int   I16;
typedef unsigned short int U16;
#ifdef _MSC_VER
typedef __int64          I64;
typedef unsigned __int64 U64;
#else
typedef long long          I64;
typedef unsigned long long U64;
#endif
typedef float  F32;
typedef double F64;



#endif
