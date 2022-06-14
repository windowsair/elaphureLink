/**************************************************************************//**
 *           TraceDat.h: Data Definitions for Trace Records Window Support
 * 
 * @version  V1.0.0
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 2020 ARM Limited. All rights reserved.
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

#ifndef __TRACE_DAT_INCLUDED__
 #define __TRACE_DAT_INCLUDED__

#include <Windows.h>
#include <CommCtrl.h>


/*
 * User-supplied Column-descriptor for ListControl
 */
typedef struct trc_col  {     // a single column
  UINT32       nAlign;        // LVCFMT_LEFT/LVCFMT_RIGHT/LVCFMT_CENTER
  char         *pName;        // Column name, e.g.  "Instruction"
  char        *pW_Str;        // string used to calc column width

  INT32       nPixWid;        // Pixel Column-width, used if 'pW_Str[0]==0'

  UINT32      nMaxChr;        // max. Chars in decoded ascii buffer (max.=4096)
  char          *pBuf;        // uVision provides 'nMaxChar' + 2 character buffer
} TRC_COL;


#define TSAMP_EXEC              0x001   // ASM-execution 'Exec' sample
#define TSAMP_READ              0x002   // ASM-execution 'Read' sample
#define TSAMP_WRITE             0x004   // ASM-execution 'Write' sample
#define TSAMP_HLL               0x008   // HLL-source line
#define TSAMP_INTERRUPT         0x016   // Interrupts



/*
 * User-supplied Trace-Data-Sample
 */
#pragma pack(1)
typedef struct trc_data  {
  UINT32          nSize;      // total-size for this record (including 'nSize')
  UINT32         nRecNo;      // Record-number
  UINT32   nSampTyp :20;      // type of Trace-Sample
  UINT32     bFlag1 : 1;      // internal: used for 'highlight search hit'
  UINT32     bFlag2 : 1;      // internal: reserved for BookMark
  UINT32     bFlag3 : 1;      // internal: reserved
  UINT32     bFlag4 : 1;      // internal: reserved
  UINT32     bFlag5 : 1;      // free
  UINT32     bFlag6 : 1;      // free
  UINT32     bFlag7 : 6;      // free

  UINT32          nAddr;      // Execution-address

//---Note: for user-supplied trace-data, the above members need to have
//         the layout and packing shown here. User-specific data may follow
//         the 'nAddr' member and the 'nSize' member need to be setup to the
//         actual size of the user-supplied trace-data item.
} TRC_DATA;
#pragma pack()




#define TRC_INIOK       1      // Ok.
#define TRC_ADDSAMPOK   1      // Add Trace-sample was Ok.

#define TRC_PWR2ERR    -1      // number of trace entires not a power of 2
#define TRC_TOOMANY    -2      // out of range number of trace sample entries
#define TRC_INVALIDCL  -3      // invalid column list
#define TRC_BADDESC    -4      // invalid/incomplete TRC_INIT descriptor
#define TRC_ADDTRC_F   -5      // Add Trace-sample failed
#define TRC_NOTACTIVE  -6      // Trace-Window not implemented



/*
 * Version-2 decoder commands:
 */
#define T_DV2NUMRECS    1      // Get number of relevant trace records
                               //  --> trace records relevant by 'filtering'
                               //  --> uses to define the scroll range
#define T_DV2DECODE     2      // decode trace-record with index '*pData1'
#define T_DV2GETADDR    3      // get execution-address for record '*pData1'
#define T_DV2FILTERCHG  4      // Trace-Filter-Combo has changed
#define T_DV2COLORCN    5      // get text color indices for column '*pdata1'

#define T_DV2READSTATS  100    // [TdB: 15.01.2010] read Cache Statistics, needed for JTrace Test
#define T_DV2RESETCACHE 101    // [TdB: 15.01.2010] reset cache

/*
 * Color-definitions (for use with T_DV2SELECTED/T_DV2COLORCN)
 */
#define TRC_WHITE       1
#define TRC_BLACK       2
#define TRC_RED         3
#define TRC_BLUE        4
#define TRC_GREEN       5
#define TRC_LGREY       6     // light grey
#define TRC_DGREY       7     // dark grey

#define TRC_LASTCOLOR   TRC_DGREY

/*
 * Version-2 error codes:
 */
#define T_DV2OK         1      // V2Decoder() - command Ok.
#define T_DV2WHAT      -1      // V2Decoder() - unknown command
#define T_DV2PARM      -2      // V2Decoder() - invalid parameter(s)



/*
 * User-supplied Setup-Data for Trace-Window
 */
typedef struct trc_filter  {
  char          *pName;        // Filter-name (in ComboSel-Box)
  UINT32         nMask;        // selection mask TSAMP_xxx (combination may be ored)
} TRC_FILTER;


typedef struct trc_init  {     // Setup Trace-Display-Window
  TRC_COL        *pCol;        // Column-descriptor-array

//UINT32      nEntries;        // max. number of trace samples (power-2 number !)
  UINT32  nEntries :28;        // V1/V2: max. number of trace samples (power-2 number !)
  UINT32    bUnus1 : 1;        // V2: currently unused
  UINT32    bUnus2 : 1;        // V2: currently unused
  UINT32    bUnus3 : 1;        // V2: currently unused
  UINT32     modV2 : 1;        // V2: 1:=Version-2 Bit

  union  {
    INT32     (*pDecode) (TRC_DATA *pD);  // TRC_DATA to Ascii-Decoder Function
    INT32   (*pDecodeV2) (UINT32 nCmd, UINT32 *pIn, UINT32 *pOut);  // V2-Decoder
  };

  TRC_FILTER  *pFilter;        // Trace-Display Filter(s)
} TRC_INIT;


typedef struct trc_add  {      // Add Sample(s)
  INT32       nSamples;        // number of samples to add
  TRC_DATA      *pSamp;        // sample-pointer (or first sample of an array if nSamples > 1)
} TRC_ADDSAMPLE;


/*
 * uVision internal interface functions
 */
extern int        Trace_Setup (TRC_INIT *pT);
extern int     AddTraceSample (TRC_ADDSAMPLE *pS);
extern void Trace_ClearWindow (void);
extern void      Trace_Update (void);



#if 0
#endif

#endif __TRACE_DAT_INCLUDED__
