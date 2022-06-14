/**************************************************************************//**
 *           BOM.H: µVision interface functions and globally
 *                  used structures and type definitions.
 * 
 * @version  V1.0.46
 * @date     $Date: 2020-09-02 09:57:33 +0200 (Wed, 02 Sep 2020) $
 *
 * @note
 * Copyright (C) 1999-2009 KEIL, 2009-2020 ARM Limited. All rights reserved.
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

#ifndef _BOM_H_INCLUDED_
  #define _BOM_H_INCLUDED_

#pragma warning( disable : 4996 )


#include <sys/types.h>        // wg. 'struct _stat'
#include <sys/stat.h>
#include <Windows.h>          // wg. HANDLE
#include <stdio.h>
#include <limits.h>

#define EXPRT __declspec(dllexport)


#include "ComTyp.h"
#include "UVSOCK.h"


#if 0   // these are in ComTyp.h
 #define iMCS51   0           // ToolSet Definitions
 #define iMCS251  1
 #define iMCS166  2
 #define iMCSARM  3           // Gnu-ARM tools
#endif

/*
 * Common Sim/Target Stuff
 */

#ifdef __cplusplus
  extern "C"  {
#endif


extern void   StopDDE (void);
extern void   SendDDE (BYTE *pS, BYTE CmdTyp, int nLen);
extern void   InitDDE (HANDLE hInst);

extern DWORD Uv3Func (DWORD nCode, void *vp1, void *vp2);



/*
 * Find in Files structure
 */

struct FiF  {
  BYTE        bFind;          // Find active flag.
  BYTE        bStop;          // stop thread flag.
  HWND       m_hWnd;          // MainFrame's hwnd.
  HANDLE        hTh;          // thread handle.
//HANDLE     hEvent;

  BYTE       SubDir;
  BYTE       RegExp;
  BYTE        mCase;
  BYTE       mhWord;
  BYTE         bMpw;
  BYTE      bHidden;          // search hidden files and folders

  char       What [300];
  char       Type [300];
  char       Fold [MAX_PATH + 10];
};

extern void FindinFiles (struct FiF *pF);




//--- Types

#define  ACONST    0x01
#define  AVOLAT    0x02

#define  bt_void      0
#define  bt_bit       1
#define  bt_char      2
#define  bt_uchar     3
#define  bt_int       4
#define  bt_uint      5
#define  bt_short     6
#define  bt_ushort    7
#define  bt_long      8
#define  bt_ulong     9
#define  bt_float    10
#define  bt_double   11
#define  bt_ptr      12
#define  bt_union    13
#define  bt_struct   14
#define  bt_func     15
#define  bt_string   16
#define  bt_enum     17
#define  bt_field    18
#define  bt_int64    19
#define  bt_uint64   20
//---new elf/dwarf loaders:
#define  bt_typref   21
#define  bt_class    22


/*
 * Storage class specifiers
 */

#define  sc_strsym      0
#define  sc_static      1
#define  sc_auto        2
#define  sc_regi        3
#define  sc_global      4
#define  sc_extern      5
#define  sc_type        6
#define  sc_const       7
#define  sc_member      8
#define  sc_label       9
#define  sc_ulabel     10
#define  sc_parm       11
#define  sc_sfr        12
#define  sc_abit       13
#define  sc_tag        14
#define  sc_bind       15

#define  sc_enumerator 16
#define  sc_namespace  17
#define  sc_section    18
#define  sc_symbol     19
#define  sc_block      20             // lexical-block
#define  sc_autoreg    21



/*
 * Expression - Node types
 */

#define  x_const     1    /* some constant */
#define  e_strcon    2    /* string object */
#define  e_labcon    3    /* static object */
#define  e_autcon    4    /* local object  */
#define  e_namcon    5    /* dScope function reference */
#define  e_simvar    6    /* dScope predefined name */
#define  e_regid     7    /* some Register name */
#define  e_vtreg     8    /* some vtreg */
#define  e_usrcon    9    /* namcon of user program */
#define  e_vcon      10   /* namcon of a vSym */

#define  e_ref       11   /* some reference */
#define  e_reff      12   /* bit-field-reference */
#define  e_cast      13   /* some cast */
#define  e_postinc   14   /* ++ (postfix !) */
#define  e_postdec   15   /* -- (postfix !) */
#define  e_addr      16   /* & (addr_of) */
#define  e_cont      17   /* * (content_of) */
#define  e_plus      18   /* unary + */
#define  e_minu      19   /* unary - */
#define  e_not       20   /* ! (not) */
#define  e_compl     21   /* ~ (complement) */
#define  e_asn       22   /* = (assign to) */
#define  e_asnor     23   /* |= */
#define  e_asnxor    24   /* ^= */
#define  e_asnand    25   /* &= */
#define  e_asnlsh    26   /* <<= */
#define  e_asnrsh    27   /* >>= */
#define  e_asnadd    28   /* += */
#define  e_asnsub    29   /* -= */
#define  e_asnmul    30   /* *= */
#define  e_asndiv    31   /* /= */
#define  e_asnmod    32   /* %= */
#define  e_hook      33   /* e0 ? e1 : e2 */
#define  e_land      34   /* && */
#define  e_lor       35   /* || */
#define  e_and       36   /* & (bit-and) */
#define  e_or        37   /* | (bit-or) */
#define  e_xor       38   /* ^ (bit-xor) */
#define  e_equ       39   /* == */
#define  e_nequ      40   /* != */
#define  e_lequ      41   /* <= */
#define  e_gequ      42   /* >= */
#define  e_gt        43   /* >  */
#define  e_lt        44   /* <  */
#define  e_lsh       45   /* << */
#define  e_rsh       46   /* >> */
#define  e_add       47   /* +  */
#define  e_sub       48   /* -  */
#define  e_mod       49   /* %  */
#define  e_div       50   /* /  */
#define  e_mul       51   /* *  */
#define  e_preinc    52   /* ++ (prefix !) */
#define  e_predec    53   /* -- (prefix !) */
#define  e_indx      54   /* [] */

#define  e_comma     55   /* expr, expr */
#define  e_param     56   /* actual parameter */
#define  e_void      57   /* void-node */
#define  e_fcall     58   /* direct function call */
#define  e_size      59   /* sizeof (expr) */
#define  e_move      60   /* some memcpy */
#define  e_struid    61   /* 'struct -> id' or 'struct.id' */
#define  e_lea       62   /* load effective address */
#define  e_exec      63   /* exec ("command") */



//--- Symbol-Handling

union amap {
  DWORD     a32;
  WORD16    w16;
  BYTE    ub[4];
};
#define AMAP   union amap



#pragma pack(1)

struct ytp  {                 /* decl.-specifiers Eimer */
  BYTE            base;       /* Basic-Type */
  BYTE        sign : 3;       /* 1=Signed, 2=Unsigned */
  BYTE        regi : 1;       /* register */
  BYTE        cvol : 4;       /* 1=const, 2=volatile */
  BYTE         err : 1;
  BYTE    tblocked : 1;       /* block other types, typedef type given */
  BYTE          Xclass;
};


/*
 * mode-0: default mode
 * mode-1: Typedef mode
 * mode-2: Structure member mode
 * mode-3: Alien Mode
 * mode-4: ANSI-Parameter mode
 */

struct clxs  {
  BYTE       mode;    // declarator Mode
  BYTE     sclass;    // Storage class
  BYTE     dclspc;    // dcl-specifiers given flag
  BYTE     cgiven;    // class given explictly
};


/*
 * mSpaces (F66, 166/167)
 */

#define  xNEAR        1
#define  xFAR         2
#define  xHUGE        3
#define  xXHUGE       4

#define  mDATA        1         // fox _near
#define  mIDATA       2         // fox _idata
#define  mPDATA       3         // fox _shuge
#define  mXDATA       4         // fox _sdata
#define  mBDATA       5         // fox _bdata
#define  mECODE       6         // fox code-space


/*
 * Security attributes (ARM v8-M)
 */

#define   sNONE    0x00         // None
#define   sNSEC    0x01         // Non-Secure
#define   sSEC     0x02         // Secure


/*
 * nfh:   0 := ndata-ptr (16-Bit, page2:pof14)
 *        1 := far-ptr   (32-Bit, seg16:sof16)
 *        2 := huge-ptr  (32-Bit, linear)
 *        3 := xhuge-ptr (32-Bit, linear)
 */

typedef struct typ  {           // Structure for a Type
  BYTE              type;       // bt_???
  DWORD             size;       // Size of Item described

  DWORD       v_flag : 1;       // 1 on Array | Struct | Func
  DWORD          vpf : 1;       // voidpointer
  DWORD         duse : 2;       // 1=const, 2=volatile, 3=both
  DWORD          nfh : 4;       // 1:=near, 2:=far, 3:=huge, 7=short
  DWORD          tps : 1;       // ptr-to-char for 'e_strcon'
  DWORD        iduse : 2;
  DWORD        parmb : 2;       // 1=f(void), 2=f(), 3=f(p,...)'
  DWORD        tType : 1;       // a Typedef-Type
  DWORD        zTemp : 1;
  DWORD       SpcSpc : 1;       // explicite spaced pointer
  DWORD       eSpace : 1;       // explicite mSpace given to spaced-ptr
  DWORD         tNew : 1;       // 1:=enhanced type, new loader
//DWORD              :14;       // unused

  BYTE             space;       // mspace on pointers
  BYTE            tspace;       // typedef'd Space
  BYTE             parms;       // Function: Nr. of Parameters

  struct typ        *btp;       // type-Link
  struct sym       *head;       // subtable struct/union/func
  char            *sname;       // ptr to type-Name
} TYP;
//#define TYP   struct typ




typedef struct tspeci  {        // template type specializer
  struct tspeci    *next;       // next specializer
  DWORD             nFld;       // unused: bit-field marker
  union  {
    struct xtyp      *tp;       // specializer type
  };
  DWORD           nRs[3];       // reserved
} TSPECI;


typedef struct xtyp  {          // Structure for a Type
  BYTE              type;       // bt_???
  union  {
    DWORD           size;       // Size of Item described
    struct {                    // new dwarf-loader uses the following:
      WORD16      nBfWid;       // bt_field - width of field within scalar
      WORD16      nBfOfs;       // bt_field - offset of field within scalar
    };
  };

  DWORD       v_flag : 1;       // 1 on array / struct / func
  DWORD          vpf : 1;       // voidpointer
  DWORD         duse : 2;       // 1=const, 2=volatile, 3=both
  DWORD          nfh : 4;       // 1:=near, 2:=far, 3:=huge, 7=short
  DWORD          tps : 1;       // ptr-to-char for 'e_strcon'
  DWORD        iduse : 2;
  DWORD        parmb : 2;       /* 1=f(void), 2=f(), 3=f(p,...)' */
  DWORD        tType : 1;       // DW_TAG_typedef type
  DWORD        zTemp : 1;
  DWORD       SpcSpc : 1;       /* explicite spaced pointer */
  DWORD       eSpace : 1;       /* explicite mSpace given to spaced-ptr */
  DWORD         tNew : 1;       // 1:=enhanced type, new loader

//---New:
  DWORD       tVolat : 1;       // volatile typeref
  DWORD       tConst : 1;       // const typeref
  DWORD      tSigned : 1;       // DW_AT_signed given
  DWORD        tBool : 1;       // DW_ATE_boolean given
  DWORD         tRef : 1;       // DW_TAG_reference_type
  DWORD      tEncode : 5;       // DW_AT_encoding
  DWORD       secAtr : 2;       // Security Attributes: 0=None, 1=non-secure (mmNSEC), 2=secure (mmSEC), 3=reserved
  DWORD      bUnused : 2;       // currently unused.

//---UV4:
  BYTE             space;       // mspace on pointers
  BYTE            tspace;       // typedef'd Space
  BYTE             parms;       // Function: Nr. of Parameters

  struct xtyp       *btp;       // type-Link
  struct xsym      *head;       // subtable struct/union/func
  char            *sname;       // ptr to type-Name
//--------------------------------

//---New:-------------------------
  struct xtyp       *otp;       // orginal func-return-type
  struct xsym      *tail;       // tail of subtable struct/union/func
  struct xtyp       *ctp;       // DW_AT_containing_type
  TSPECI         *tpSpcH;
  TSPECI         *tpSpcT;
  DWORD         tNum :24;       // type-number for checks...
  DWORD              : 8;       // unsued
} XTYP;





#if 0
/*
 * OMF66 Type descriptors
 */

struct t2_x21  {                /* 0x21: Pointer descriptor */
  BYTE        nTyp;             /* 0x21 */
  TYP          *tp;

  BYTE        Size;             /* pointer size: 16 or 32  */
  BYTE      Attrib;
  WORD16  reserved;             /* unused, set to zero     */
  WORD16        TI;             /* Type-Index of refererred type */
};

struct t2_x22  {                /* 0x22: Array descriptor */
  BYTE        nTyp;             /* 0x22 */
  TYP          *tp;

  BYTE       nDims;             /* number of dimensions */
  BYTE      Attrib;             /* Bit-0: huge-array, Bit-1: xhuge-array */
  WORD16        TI;             /* Index of Array-Type */
  DWORD  DimSz [1];             /* actually 'nDims' values */
};

struct t2_x29  {                /* 0x29: Fox-Struct/union descriptor */
  BYTE        nTyp;             /* 0x29 */
  TYP          *tp;

  BYTE      Attrib;             /* 1:=struct, 2:=union */
  DWORD       Size;             /* struct/union size */
  WORD16        TI;             /* Type-Index of component list */
  BYTE        nLen;             /* NameLen */
  BYTE     name[1];             /* actually 'nLen' characters */
};

struct t2_x23  {                /* 0x23: Function descriptor */
  BYTE        nTyp;             /* 0x23 */
  TYP          *tp;

  BYTE      Attrib;             /* 1:=near Function, 2:=far Function */
  WORD16       rTI;             /* Return-Type Index */
  WORD16       pTI;             /* Parameter-List Index (Component List) */
};

struct t2_x25  {                /* 0x25: Bitfield descriptor */
  BYTE        nTyp;             /* 0x25 */
  TYP          *tp;

  WORD16        TI;             /* type of underlying object */
  BYTE      Offset;             /* Field-Offset in bits */
  BYTE       Width;             /* Field-Width in bits  */
};

struct t2_x26  {                /* 0x26: Typedef descriptor */
  BYTE        nTyp;
  TYP          *tp;

  WORD16        TI;             /* Type index */
  BYTE       Attr8;             /* unused */
  DWORD      Res32;             /* unused */
  BYTE        nLen;             /* NameLen */
  BYTE     name[1];             /* actually 'nLen' characters */
};

struct t2_x27  {                /* 0x27: Component */
  BYTE        nTyp;
  TYP          *tp;

  WORD16        TI;             /* component's type */
  DWORD     Offset;             /* component's offset */
  BYTE        Rep8;             /* unused */
  BYTE        Pos8;             /* unused */
  BYTE        nLen;             /* NameLen */
  BYTE     name[1];             /* actually 'nLen' characters */
};

struct t2_x28  {                /* 0x28: Component-List */
  BYTE        nTyp;
  TYP          *tp;

  DWORD     nComps;             /* Nr. of Components (word->dword by loader) */
  WORD16   nArr[1];             /* 'nComps' entries */
};

union flv {
  DWORD          sizz;      /* Typ muss gleich wie typ.size sein ! */
  struct fld  {
    BYTE          fwd;      /* Field-Width */
    BYTE          fof;      /* Field-Offset in Basic-Skalar */
    BYTE          bas;      /* 0 = char, 1 = unsigned */
    BYTE         sign;      /* 'signed' field */
  } fld;                    /* Field-Code fuer tp->size */
};


typedef union  {
  BYTE            *pB;
  struct t2_x21  *p21;              // 0x21: Pointer
  struct t2_x22  *p22;              // 0x22: Array
  struct t2_x29  *p29;              // 0x29: Fox-struct union
  struct t2_x23  *p23;              // 0x23: function
  struct t2_x25  *p25;              // 0x25: Field
  struct t2_x26  *p26;              // 0x26: TypeTag
  struct t2_x27  *p27;              // 0x27: Component
  struct t2_x28  *p28;              // 0x27: Component-List
} tpU;
#endif




struct zVal  {
  BYTE       Rep;              // 1:=Bits, 2:=Bytes
  UINT64     v64;
};
#define ZVAL  struct zVal

union v  {                     // expression-value union
  unsigned long       ul;
  unsigned int       u32;      // 17.3.2011
  signed char          c;
  unsigned char       uc;
  signed short       i16;
  unsigned short     u16;
  unsigned short      ui;
  signed long          l;
  unsigned long  ul2 [2];
  unsigned char  uc8 [8];     // uc[7] holds dpp-info of near-ptr
  int                  i;
  INT64              i64;
  UINT64             u64;
  float                f;
  double               d;
  struct vtr        *pVR;       // VtReg-descriptor
  char                *s;
  BYTE               *pB;
  WORD16             *pW;
  DWORD             *pDW;
  struct zVal        v64;
  void           *vp [2];     // used for dScope-functions
};


struct lPa  {
  DWORD         n1;
  DWORD         n2;
  DWORD         n3;
  DWORD         n4;
  union v        v;
};



/*
 * VTREG structure
 */

struct vtr  {
  char           *name;          // name of VtReg
  TYP              *tp;          // Type of VtReg
  WORD16         vtrNo;          // laufende nummer (0...MAXVTR - 1)
  WORD16    OscFrq : 1;          // special for 'XTAL'
  WORD16     Clock : 1;          // special for 'CLOCK'
  union v            v;          // VtReg's value
  WORD16           Atr;          // 0 | ATR_RDBRK | ATR_WRBRK
};

#define VTR      struct vtr
#define MAXVTR   256            // max. of 256 VTR-Slots.


struct enode  {                 // Expression-Node
  BYTE              etyp;       // Node-Type
  BYTE              rtyp;       // Rslt-type on operators

  DWORD         cvol : 2;       // const/volatile */
  DWORD          cfl : 1;       // const-expr */
  DWORD         mark : 1;       // expl-cast or struid mark
//DWORD        bable : 1;       // Expr. may have Type 'bit'
  DWORD      TopNeed : 1;       // don't remove Top-cast, if present
  DWORD        tprom : 1;       // Param: promote type (no parm-type given)
  DWORD          one : 1;       // if cfl=1: constant value-1
  DWORD         zero : 1;       // if cfl=1: constant value-0
  DWORD          max : 1;       // if cfl=1: constant has max-value
  DWORD        ssref : 1;       // e_ref for struct/union from nameref()
  DWORD      StruRet : 1;
  DWORD       iStore : 1;       // this e_ref is a store
  DWORD          wid : 6;       // e_reff: field-width
  DWORD          ofs : 6;       // e_reff: field-offset
  DWORD      iNotVis : 1;       // expr is not in Scope
  DWORD       AccBrk : 1;       // Access-break top node
  DWORD       iNoAcc : 1;       // can't access target mem or regs
  DWORD      bStMark : 1;       //
//DWORD         bRHS : 1;       // RHS of an assignment
//                              // 4-bits left

  BYTE            OpType;       // Op-Type on NOT and all RelOps
  WORD16           space;       // Referencing Memory Space
  WORD16          rspace;       // Resulting Memory Space
  BYTE            nCount;       // temp: used for GenParms
  union v              v;       // Node value
  struct typ         *tp;       // Operation-Type, normally Result-Type
  struct typ        *tp1;       // Result-Type on RelOps and Not

  struct enode        *L;       // left child
  struct enode        *R;       // right child
  WORD16            Line;       // Line-Number
  WORD16           index;       // Line-index
  WORD16            nCnt;       // eg. strlen on e_strcon
  WORD16            nPos;       // Reg-based Bit position
  WORD16             Dpp;       // per dppn:override
  BYTE            nRadix;       // output radix for expr (watch)
  void               *vp;       // dScope-f(): rwatch/wwatch() (WATCH *)
};

#define EXP  struct enode



#define WATCH   struct watch
#define ACCESS  struct access

typedef struct watch *pWATCH;


#ifndef __AGSICB_REASON         // needs to be conform to INTERFACE.CPP/AGSI.H
  #define __AGSICB_REASON

  typedef enum {          // !! numbers conform with ATR_READ & ATR_WRITE 
    CB_READ     = 0x02,   // adr = memory address
    CB_WRITE    = 0x04,   // adr = memory address
    CB_VT_READ  = 0x12,   // adr = VTR ** = AGSIVTR
    CB_VT_WRITE = 0x14,   // adr = VTR ** = AGSIVTR
    CB_IFETCH   = 0x08,   // adr = program counter value
  } AGSICB_REASON;  
#endif


struct watch  {
  WATCH            *next;  // next watch
  WATCH            *prev;  // previous watch
  union {
    void    (*fp) (void);  // Watch-function
    void    (*wf) (DWORD adr, AGSICB_REASON r);
    void    (*ef) (DWORD adr, AGSICB_REASON r, DWORD size);  // HS 16.07.2007 added parameter 'size'
    void    (*lf) (DWORD adr, AGSICB_REASON r, WATCH *vp);
  } v;
  DWORD              adr;  // memory Address
  BYTE          acc_type;  // Trigger-Access 1:=READ, 2:=WRITE, 3:=BOTH
  WORD16        acc_size;  // Access-Size: 1:=BYTE, 2:=WORD,  4:=DWORD, 0xFFFF:=VTREG Access
  BYTE           special;  // 1=WBREAK,3=LaWatch
  BYTE              mask;  // iMCS51: Bit-Mask
  WORD16      acc_rights;  // access rights
};

/* ACCESS-LIST STRUCTURE */

typedef struct access *pACCESS;
struct access  {
  DWORD           address;    /* Addresse im Memory */
  WORD16             mask;    /* Read-/Write Mask */
};


                                     // 27.12.2004
typedef struct xlin  {               // LINE-NUMBER structure
  WORD16            line;            // the linenumber
  DWORD             offs;            // linenumber-offset
  struct xlin      *next;
} XLIN;


struct zxfile  {
  struct zxfile    *next;            /* next file descriptor */
  WORD16             nFI;            /* File-Index */
  WORD16           nMode;            /* 0:=Src, 1:=Lst */
  char             *name;            /* File name */
  char             *path;            /* File path */
  DWORD             time;            /* File time */

  union  {
    struct lin      *pLn;            // debug-lines head - old 16-bit
    struct lin32  *pLn32;            // debug lines head - new 32-bit
  };
  int             nLines;            /* lines in 'lhd' */
  int               nMax;            /* lines limit for current 'pLn' */
  DWORD            uLine;            /* highest line number in table */

//HANDLE           hFile;            /* File Handle */
//HANDLE           hFmap;            /* View-map handle */
  BYTE               *pB;            /* mapped view of File */
  DWORD            fSize;            /* File size */
  struct _stat       sts;            /* File status */
  DWORD          *pArray;
//DWORD            cline;            /* current line */
//int              cindx;            /* current View-Index */

//---/27.12.2004/: temporary lines (for new SARM-ELF/DWARF-Loader)
  XLIN              *pLh;            // debug-lines tail
  XLIN              *pLt;            // debug-lines tail

};

#define ZXFIL   struct zxfile



/*
 * Local-List Descriptor (for FrameBase, RegVars etc.)
 */

#define LOC_MODE_ADR      0x01          // Mode 1: nAdr is valid
#define LOC_MODE_REG      0x02          // Mode 2: Rn [+/-nVal]
#define LOC_MODE_USEBREG  0x03          // Mode 3: use BaseReg

typedef struct locdsc  {
  struct locdsc  *next;       // next location-descriptor

  DWORD       mode : 8;       // 1: nAdr [+/- nVal]
                              // 2: Rn [+/- nVal]
                              // 3: use BaseReg [+/-nVal]
  DWORD            :24;       // <free for future use>

  DWORD            beg;       // Start Address Range
  DWORD            end;       // End Address Range  ( a >= beg && a < end )
  DWORD           nAdr;       // Rn / static address
  int             nVal;       // +/- displacement

  DWORD       nRes1[8];       // reserved, unused.
  DWORD       nRes2[8];       // reserved, unused.
  int         nRes3[8];       // reserved, unused.
} LOCDESC;




typedef struct lin  {         // LINE-NUMBER structure
  WORD16          line;       // the linenumber
  DWORD           offs;       // linenumber-offset
} LIN;                        // #define LIN  struct lin

typedef struct lin32  {       // LINE-NUMBER structure   /28.1.2011/
  DWORD       line :28;       // 32-bit linennumber
  DWORD      bStmt : 1;
  DWORD            : 3;
  DWORD           offs;       // linenumber-offset
} LINE32;


//---complex locations - raw block data:
typedef struct locBuf  {
  UINT32          nSize;      // size of buffered content
  UINT32    bEndian : 1;      // 1:=big-endian, 0:=little-endian
  UINT32            :31;
  UINT8          buf[4];      // will actually be 'nSize' bytes.
} LOCBUF;


#pragma pack(1)

typedef struct sym  {           /* SYMBOL structure */
  struct sym       *next;       /* link to next Symbol */
  struct sym       *alnk;
  char             *name;
  struct zbk       *pBlk;       /* bounding Scope-Block */
  WORD16            type;
  BYTE                sc;       // Stoage class
  union v              v;
  TYP                *tp;       /* type index */
  WORD16             Dpp;
  WORD16             Reg;       /* Register number */
  WORD16             Pos;       // BitPos or FuncAttr

  DWORD         nMI : 10;       // ARM-OMF2: Module-Index (no more used)
  DWORD     bStMark :  1;       // sc_member has final address /RVCT/9.3.2007/
  DWORD     bParmRg :  1;       // sc_regi is sc_parm
  DWORD       bXsym :  1;       // 1:= sym is a xsym
  DWORD    bInlined :  1;       // 1:= an inlined DW_TAG_subprogram
  DWORD       bTask :  1;       // 1:= symbol is a task
  DWORD      bExped :  1;
  DWORD             : 16;       // currently unused
  DWORD           nRs[3];       // reserved

  LOCDESC           *pLD;       // Location-Description (ELF/DWARF)
  LOCDESC           *pFD;       // Frame-Description
} SYM;


typedef struct xsym  {          // Structure for a symbol
  struct xsym      *next;       // next symbol
  struct xsym      *alnk;
  char             *name;       // symbol name
  struct zbk       *pBlk;       // bounding Scope-Block
  WORD16            type;
  BYTE                sc;       // Stoage class
  union v              v;       // Symbols value / address
  struct xtyp        *tp;       // type of symbol
  WORD16             Dpp;
  WORD16             Reg;       // Register number
  WORD16             Pos;       // BitPos or FuncAttr

  DWORD         nMI : 10;       // ARM-OMF2: Module-Index
  DWORD     bStMark :  1;       // sc_member has final address /RVCT/9.3.2007/
  DWORD     bParmRg :  1;       // sc_regi is sc_parm
  DWORD       bXsym :  1;       // 1:= sym is a xsym
  DWORD    bInlined :  1;       // 1:= an inlined DW_TAG_subprogram
  DWORD       bTask :  1;       // 1:= symbol is a task
  DWORD      bExped :  1;
  DWORD             : 16;       // currently unused
  DWORD           nRs[3];       // reserved

  LOCDESC           *pLD;       // Location-Description (ELF/DWARF)
  LOCDESC           *pFD;       // Frame-Description
//------------------------------// UV-symbol info ends here.

//---new info:-------------------
  void           *pLocat;       // 'LOCDBG *' - f(): Frame_base or non-f() loc-expr
  struct xtyp    *tpThis;       // C++: 'this' type pointer
  struct xsym   *spSpace;       // parent-namespace-symbol
  struct xtyp       *otp;       // orginal type with possible type_ref's

  DWORD      bPublic : 1;       // 1:=public per .debug_pubnames
  DWORD       bScope : 1;       // 1:=Scope starts at lowPC+scpOfs
  DWORD   bArtifical : 1;       // 1:= artificial
  DWORD        bRegX : 1;       // 1:= DW_OP_regx [M4-FPU] nReg
  DWORD         bReg : 1;       // 1:= DW_OP_reg0...DW_OP_reg31 nReg
//---

  DWORD        bDecl : 1;       // 1:=DW_AT_declaration := 1
#if 0
  DWORD     bDclFile : 1;
  DWORD     bDclLine : 1;
  DWORD      bDclCol : 1;
  DWORD     bCalFile : 1;
  DWORD     bCalLine : 1;
  DWORD      bCalCol : 1;
#endif
  DWORD     bMutable : 1;       // DW_AT_mutable
  DWORD        nVirt : 4;       // DW_AT_virtuality 0/1/2
                                // 0:=DW_VIRTUALITY_none
                                // 1:=DW_VIRTUALITY_virtual
                                // 2:=DW_VIRTUALITY_pure_virtual
  DWORD     bAdrSeen : 1;       // DW_OP_addr is specified
  DWORD      bSpList : 1;       // internal convert() flag
  DWORD      bConstV : 1;       // Symbol has a DW_AT_const_value value

//---complex locations:-------------------------------------------------
  DWORD    bComplexA : 1;       // 'nRs[0]' is a LOCBUF raw descriptor
                                //  for complex static/global locations.
  DWORD     bAbsMemb : 1;       // 1:=absolute member address in 'higPC'
  DWORD    bCplFinal : 1;       // 1:=complex address calc already finalized.
  DWORD      bRanges : 1;       // 1:=DW_AT_ranges, scpOfs := .debug_ranges[entry]
  DWORD        bAlnk : 1;       // 1:='alnk' member holds parent XTYP *  /22.5.2020/
//----------------------------------------------------------------------

  DWORD            higPC;
  INT32           scpOfs;       // start-scope @lowPC+scpOfs
  WORD16            nReg;       // bRegX / bReg register

  struct esym      *pSym;       // parent-symbol with section
  struct eshdr    *lines;       // .debug_line
  struct xsym     *snext;       // 'next' Symbol in 'Section'
  struct xsym     *cnext;       // 'next' Symbol in CU
} XSYM;



/*
 * STL-type preparation and address collections.
 */
typedef enum stl_enc  {
  ENCSTL_NONE      = 0,                          //  0 := not a valid stl object.
  ENCSTL_STRING,                                 //  1 := string<char>
  ENCSTL_WSTRING,                                //  2 := string<wchar_t>
  ENCSTL_U16STRING,                              //  3 := string<uint16_t> - u16string
  ENCSTL_U32STRING,                              //  4 := string<uint16_t> - u16string

  ENCSTL_VECTOR,                                 //  5 := vector<>
  ENCSTL_LIST,                                   //  6 := list<>
  ENCSTL_MAP,                                    //  7 := map<>
  ENCSTL_SET,                                    //  8 := set<>
  ENCSTL_QUEUE,                                  //  9 := queue<>

  ENCSTL_LAST      = ENCSTL_QUEUE,
} STL_ENC;


typedef struct stlres  {
  XTYP                *tp;                       // [IN] stl-class type
  DWORD              nAdr;                       // [IN] object's address
  int               vaMax;                       // [IN] capacity of 'vAdr' in DWORD's
  DWORD             *vAdr;                       // [IN/OUT] stl vectored address collection

                                                 // the following are all [OUT]

  STL_ENC         tEncode;                       // enumerated stl-type
  int              nItems;                       // number of item addresses listed in 'vAdr'
  DWORD             nOffs;                       // specialized type data start at 'address + nOffs'
                                                 // string<char / wchar_t>: nOffs := memory location of string
  union  {
    DWORD          aWords [3];                   // object-base triplet '[begin | end | size]'
    BYTE           aChars [3 * sizeof (DWORD)];  // basicstring<char>
    UINT16         wChars [2 * sizeof (DWORD)];  // basicstring<wchar_t>
  };

  DWORD              nOvf :  1;                  // vAdr[] overflow mark
  DWORD             bSstr :  1;                  // 1:=string/wstring used short format
  DWORD           bInvAdr :  1;                  // 1:=invalid vector address seen.
  DWORD              unus : 29;                  // currently unused

  int              oItems;                       // raw number of items in container
  XTYP             *tSpec;                       // specializer_type
  XTYP             *tpSp2;                       // map<key_type specializer

  DWORD              nRes [8];                   // currenttly unused.
  DWORD              aAdr [256 + 2];
} STLRES;


#pragma pack()




//#define SYM  struct sym

struct adr  {                        /* ADDRESS-FRAME link */
  struct adr     *next;              /* next frame */
  DWORD           ladr;              /* address frame low */
  DWORD           hadr;              /* address frame high */
};


struct vSym  {             // a user created value symbol
  struct vSym  *next;      // next vSym
  char         *name;      // vSym's name
  TYP            *tp;
  union v          v;      // the value (Scalar types only !)
};
#define vSYM  struct vSym



//#define ZBK  struct zbk
#define BLK_THEADR   1
#define BLK_PROC     2
#define BLK_DO       3
#define BLK_TASK     4


typedef struct lne  {         // LINE-Entry  /27.12.2004/
  struct lne      *next;
  DWORD           nLine;      // line number
  DWORD           nAddr;
  DWORD       bStmt : 1;      // is_stmt
  DWORD        nCol :15;      // column          /17.12.2018/
  DWORD             :16;      // currently free
//DWORD             :31;      // currently free
} LINEITEM;

typedef struct lrg  {         // LINE-Range  /27.12.2004/
  struct lrg      *next;      // next Range
  ZXFIL          *pFile;      // current file
  DWORD         nAdrLow;
  DWORD         nAdrHig;
  LINEITEM        *pLnH;
  LINEITEM        *pLnT;
} LINERANGE;


typedef struct zbk  {          // SCOPE-BLOCK structure
  struct zbk      *next;       // next block
  char            *name;       // BlockName
  struct zbk      *anxt;       // Address: next Block
  struct zbk    *parent;       // the Parent Block
  DWORD            offs;       // BLOCK Start address
  DWORD            bend;       // BLOCK End address
  WORD16            nTI;       // Type Index
  struct zbk      *down;       // nested block
  ZXFIL          *pFile;

  union  {
    struct sym     *shd;       // debug-symbols head (sym)
    struct xsym   *xshd;       // debug-symbols head (xsym)  /17.3.2011
  };

  BYTE         mark : 3;       // block marker
  BYTE         ccov : 1;       // nr. of executed instr. has changed
  BYTE         dBlk : 1;       // on-demand THeader - not loaded !
  BYTE        iSymi : 1;       // Block already in SymWin  /26.6.2006/
  BYTE       bSuper : 1;
  BYTE         bInl : 1;       // 'inlined' block  /14.3.2010/

  DWORD           nInst;       // number of instructions in Func-Block
  DWORD           nExec;       // number of executed instructions
  DWORD          nCjmps;       // number of cond-jumps in block
  DWORD          nTjmps;       // number of taken jumps in block

  LINERANGE      *pRngL;       // 27.12.2004, Line-Range Descriptor
  LOCDESC          *pLD;       // 31.12.2004, Location-Description (ELF/DWARF)

//---new elf/dwarf loader:-----
  struct zbk      *tail;       // next/tail
  union  {
    struct sym     *sht;       // debug-symbols tail
    struct xsym   *xsht;       // debug-symbols tail (xsym)  /17.3.2011
  };
  LINERANGE      *pRngT;       // tail of Line-Range Descriptor
  char           *pName;       // orginal name, e.g. 'Src\Measure.c'
  struct zbk      *pBx1;       // inlined functions head
  struct zbk      *pBx2;       // unused
  DWORD         nRes[6];       // reserved
//-----------------------------
} ZBK;


struct iF66  {
  DWORD         ErrAdr;
  SYM           *Hpubs;        // Public symbols - Head
  SYM           *Tpubs;        // Public symbols - Tail
  SYM            *SfrH;        // Head of SFR symbols
  SYM            *SfrT;        // Tail of SFR symbols
  ZXFIL        *FilesH;        // Files - Head
  ZXFIL        *FilesT;        // Files - Tail
  DWORD         nTypes;        // Nr. of Type-Recs (ab 0x80)
  DWORD            tpN;        // current max Type-Slots
  char           **tpA;        // Type-Pointer Array
  int            yindx;
  ZBK            **pA3;        // a24-a31 Cluster
  ZBK           *pRoot;        // Head of Theader Blocks.

  vSYM         *vShead;        // Head of user-create value symbols
  vSYM         *vStail;        // Tail of user-create value symbols
  
  DWORD            low;        // Low Scope address
  DWORD            hig;        // High Scope address
};





struct scop66  {               // Scope-Descriptor
  DWORD            low;        // Low Scope address
  DWORD            hig;        // High Scope address
  ZXFIL         *pFile;
  union  {
    LIN           *pLn;        // extracted from 'pFile'
    LINE32      *pLn32;        // debug lines head - new 32-bit
  };
  int           nLines;        // extracted from 'pFile'

  DWORD           targ;  
  WORD16          mode;        // HLL, ASM
  WORD16       set : 1;
  WORD16     isact : 1;        // HLL File is active

  ZBK              *pT;        // Translation-Unit
  ZBK              *pF;        // Function in Theader
  ZBK              *pB;        // Block in Function
  ZBK           *pTask;        // 166: Task Header
};

#define CUS    struct scop66

#define IF66  struct iF66

extern IF66          i66;      // Fox Symbol-Info
extern struct scop66 Sc6;      // Scope-Info


/*
 * Qualified references
 */

struct iqual  {
  int        comps;          // Nr. of Components
  char      *tname;          // \\TASKNAME
  int         tndx;          // Index of TaskName
  char    *name[3];          // \THEADR\FUNCTION\LOCSYM
  int       ndx[3];          // Component Line-Indices
  int         Bank;
  ZBK          *tk;          // Task specifier
  ZBK          *th;          // Parent-Theader
  ZBK          *fc;          // Parent-Function
  SYM          *sp;          // if a fully qualified Symbol
  DWORD     LineNo;          // Linenumber to search for
  DWORD    LineOfs;          // Linenumber-Offset
  int        ErrIx;          // Line-Index in case of error
  int         bufi;
  char   buf [512];
  BYTE        line;          // Qualifier specifies '\line'
};
#define IQUAL struct iqual   // Qualifier-Reference


struct squal  {
  int        index;
  int         Line;
  ZBK        *Task;
  ZBK      *Theadr;
  ZBK        *Func;
  SYM          *sp;
  ZBK         *pDo;          // named do-block
};
#define SQUAL  struct squal  // Qualifier-Reference


#pragma pack()


//--- Basic-types
extern TYP tp_void;    // = { bt_void,   { 0, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_bit;     // = { bt_bit,    { 0, 0, 0, 0 }, 0, 0, 1,  };
extern TYP tp_char;    // = { bt_char,   { 0, 0, 0, 0 }, 0, 0, 1,  };
extern TYP tp_uchar;   // = { bt_uchar,  { 0, 0, 0, 0 }, 0, 0, 1,  };
extern TYP tp_int;     // = { bt_int,    { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_uint;    // = { bt_uint,   { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_short;   // = { bt_short,  { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_ushort;  // = { bt_ushort, { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_long;    // = { bt_long,   { 0, 0, 0, 0 }, 0, 0, 4,  };
extern TYP tp_ulong;   // = { bt_ulong,  { 0, 0, 0, 0 }, 0, 0, 4,  };
extern TYP tp_float;   // = { bt_float,  { 0, 0, 0, 0 }, 0, 0, 4,  };
extern TYP tp_double;  // = { bt_double, { 0, 0, 0, 0 }, 0, 0, 8,  };
extern TYP tp_func;    // = { bt_func,   { 1, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_funcN;   // = { bt_func,   { 1, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_funcF;   // = { bt_func,   { 1, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_funcS;   // = { bt_func,   { 1, 0, 0, 7 }, 0, 0, 0,  };

extern TYP tp_culong;  // = { bt_ulong,    0, 0, 1, 0, 0, 0, 4,  }; // const !


/*
 * Symbol-Types
 */

#define F66_VAR    0x0001         // non-bit Variables
#define F66_CON    0x0002         // named Constants
#define F66_BIT    0x0004         // Bit in Memory
#define F66_LOC    0x0008         // Location, some label
#define F66_REGV   0x0010         // Register-Variable
#define F66_AUTO   0x0020         // Stack-based variable
#define F66_REGB   0x0040         // Bit in Register
#define F66_SFRB   0x0080         // Bit of SFR
#define F66_TDEF   0x0100         // Typedef name
#define F66_SFR    0x0200         // Fox-SFR name
#define F66_MEMB   0x0400         // MOS/MOU/MOE */

#define F66_FUNC   0x0800         // dScope function name
#define F66_FPARM  0x1000         // dScope function parameter
#define F66_FLAB   0x2000         // dScope function local label
#define F66_FLOC   0x4000         // dScope function local symbol

#define F66_NSPACE 0x8000         // C++ namespace


/*
 * Fox-Compound-Types
 */

#define ZT_CLIST       0x20     /* OMF166: component list descriptor */
#define ZT_PTR         0x21     /* OMF166: pointer descriptor        */
#define ZT_ARRAY       0x22     /* OMF166: array descriptor          */
#define ZT_FUNC        0x23     /* OMF166: function descriptor       */
#define ZT_STRU        0x24     /* OMF166: struct/union descriptor   */
#define ZT_FIELD       0x25     /* OMF166: bitfield descriptor       */
#define ZT_TYPTAG      0x26     /* Fox-Typedef */
#define ZT_COMP        0x27     /* Fox-Component */
#define ZT_FCLIST      0x28     /* Fox-Component-List */
#define ZT_FSTRU       0x29     /* Fox-struct/union descriptor */


/*
 * Fox Scalar-Types
 */

#define TI_NIL         0x40     /* untyped */
#define TI_BIT         0x41     /* bit */
#define TI_CHAR        0x42     /* char */
#define TI_UCHAR       0x43     /* uchar */
#define TI_INT         0x44     /* int */
#define TI_UINT        0x45     /* uint */
#define TI_LONG        0x46     /* long */
#define TI_ULONG       0x47     /* ulong */
#define TI_FLOAT       0x48     /* float (32-bit)  */
#define TI_DOUBLE      0x49     /* double (64-bit) */
#define TI_VOID        0x4A     /* void */
#define TI_LABEL       0x4B     /* label (near) */
#define TI_BITWORD     0x4C     /* a166 bitword */
#define TI_NEAR        0x4D     /* a166 near */
#define TI_FAR         0x4E     /* a166 far */
#define TI_DATA3       0x4F     /* a166 data3 */
#define TI_DATA4       0x50     /* a166 data4 */
#define TI_DATA8       0x51     /* a166 data8 */
#define TI_DATA16      0x52     /* a166 data16 */
#define TI_INTNO       0x53     /* a166 intno */
#define TI_REGBANK     0x54     /* a166 regbank */
#define TI_IFAR        0x55     /* a166 interrupt far */
#define TI_DATA32      0x56     /* a166 data32 */
#define TI_DATA64      0x57     /* a166 data64 */
#define TI_FRACT16     0x58     /* fractional-16 type */
#define TI_FRACT32     0x59     /* fractional-32 type */
#define TI_SHORT       0x7F     /* dummy type for Jumps */


/*
 * Loader-Errors
 */

#define LOD_OK         0x0000


//--- DisAsm Stuff

/*
 * Language Window Definitions
 */

#define HLEVEL   0x00       /* View-Modes */
#define MIXED    0x01
#define ASSMBL   0x02

#define LINENUM  0x20       /* Information Number */
#define ASMLINE  0x21
#define LABLINE  0x22       /* Line with Label only */
#define HISTNUM  0x23
#define HISTLAB  0x24
#define HISTLIN  0x25
#define EMPTYLN  0x26       /* no more lines available */

#define ATTRBRKE 0x40       /* Attributes */
#define ATTRPC   0x80
#define ATTRHIST 0x20
#define ATTRLOG  0x10
#define ATTRHIG  0x08       /* HighLight Attribute */
#define ATTREXEC 0x01       /* hold in member 'cover' */
#define ATTRBRKD   0x100    /* disabled exec-break */
#define ATTRJTAKEN 0x200    // 



#define CRT  struct crt
struct crt  {           /* CRT struct */
  WORD16       inumb;   /* information number     */
  WORD16        cont;   /* content (attribute)    */
  WORD16        attr;   /* attribute CURPC, BRKPT */
  WORD16        bpnr;   /* breakpoint number      */
  ZXFIL       *pFile;
  int             fi;   /* File-Index of 'line'  */
  DWORD          off;   /* offset                 */
  DWORD         line;   /* line number bei HLL    */
  int           hidx;   /* History Index          */
  char         *pLab;   /* LabelName on HISTLAB/LABLINE */
  WORD16        scra;   /* Attribute on Screen    */
  WORD16        nLen;   /* Line-Length            */
  BYTE         cover;   /* Coverage Attribute     */
};
typedef struct crt   *pCRT;





#define ABREAK  0
#define CBREAK  1
#define WBREAK  2
#define RBREAK  3           /* Address-Range Break */
#define TBREAK  4           /* Tracepoint          */

#pragma pack(1)
struct brk  {
  struct brk    *next;
  struct brk    *prev;

  DWORD      type :  4;     // ABREAK, CBREAK, WBREAK, TBREAK
  DWORD   enabled :  1;     // 1:=enabled, 0:=disabled
  DWORD    ReCalc :  1;     // recalc expr flag
  DWORD    BytObj :  1;     // WatchBrk: 0:=Bytes, 1:=Objects
  DWORD   ExtInfo :  1;     // 1:=extended info is present
  DWORD    bScope :  1;     // 1:='pScope' member is present and setup /18.3.2007/
  DWORD   HtxType :  8;     // IFX/Hitex Breakpoint type  /26.1.2011/
  DWORD   fromIni :  1;     // BP defined via .ini File, not-serialized /3.5.2012/
  DWORD  bAlloced :  1;     // BP resource allocated  /15.06.2012/
  DWORD   bKilled :  1;     // BP killed  /8.12.2013/
  DWORD           : 12;     // Reserved for future use

  DWORD           Adr;      // Break-Address
  DWORD        mSpace;      // memory-space
  VTR             *pV;      // used on VTR-access breakpoints

  DWORD         tsize;      // WatchBrk/Tracepoint: size of one object
  DWORD          many;      // WatchBrk/Tracepoint: many objects or bytes
  WORD16          acc;      // WatchBrk/Tracepoint: 1:=Read, 2:=Write, 3:=ReadWrite, 0:=CodeAccess (Tracepoint)
  WORD16       BitPos;      // currently not used
  DWORD        number;      // BreakPoint-Number
  int          rcount;      // Break is taken when rcount = 1

  int          ocount;      // Original Count
  EXP             *ep;      // Cond-Expression
  char           *cmd;      // Exec-Command
  char          *Line;      // Breakpoint-Expression Line for Display
  char            *pF;      // module file name
  DWORD         nLine;      // line number
  BYTE         Opc[8];      // Opcode-Save Area for Monitors

//---7.12.2006: extended info - present only if 'ExtInfo' or 'bScope' is set:
  DWORD       ExtInf0;
  DWORD       ExtInf1;
  DWORD       ExtInf2;
  DWORD       ExtInf3;
  DWORD       ExtInf4;
  DWORD       ExtInf5;
  DWORD       ExtInf6;
  DWORD       ExtInf7;
  DWORD       ExtInf8;
  DWORD       ExtInf9;
//------------

//---these are present of 'bScope' is set:  /18.3.2007/
  ZBK         *pScope;      // full Scope-Descriptor
  DWORD     nTickMark;      // Break via Socket: TimeStamp to identfy later
  DWORD      nRes[15];      // reserved for future use.
//---
};
#pragma pack()

#define BKS  struct brk



struct BpBlk  {
  BKS        Bp;             // Bp to clone from
  BKS      *nBp;             // the new Bp
  WORD16    Err;             // Error-Number
  BYTE     Item;             // 1 := expr, 2 := count, 3 := command */
  BYTE      Atr;             // 1 := Bp redefined
  BYTE    BpDlg;             // 1 := Bp create from Dialog
};



#if 0
extern void       LRwLF (pCRT X);             /* Language-Reverse-Line */
extern void       LFwLF (pCRT X);             /* Language-Forward-Line */
extern void      CrtAtt (pCRT X);             /* get LL-Attributes */
extern char       *LGet (pCRT X);
extern DWORD   AsmBelow (DWORD off);          /* Sync for ThumbTrack */
extern int               yindx;
extern CRT               cwLog;
extern DWORD            MARKPC;        /* used for LWin-HighLight */
extern BYTE   DynaScope (DWORD nAdr);
extern WORD16   DynaHll (DWORD nAdr);
extern ZXFIL *FileScope (DWORD nAdr);  /* map nAdr to 'ZXFIL *' */
extern BYTE    FileLine (ZXFIL *pF, DWORD nLine);
//--- End DisAsm Stuff
#endif


/*
 * Fox-Section types (taken from OMF267-Specification)
 */

#define   SF66_BIT        0x0000        /* Bit-Section */
#define   SF66_DATA       0x0001        /* Data-Section */
#define   SF66_CODE       0x0002        /* Code-Section */
#define   SF66_NCONST     0x0003        /* NConst-Section */
#define   SF66_HDATA      0x0005        /* Hdata-Section */
#define   SF66_HCONST     0x0007        /* HConst-Section */
#define   SF66_XDATA      0x0009        /* Xhuge-Data-Section */
#define   SF66_XCODE      0x000A        /* XCode-Section */
#define   SF66_XCONST     0x000B        /* XConst-Section */








#define ATR_EXEC   0x01         // 'executable ' Attribute
#define ATR_READ   0x02         // 'readable' Attribute
#define ATR_WRITE  0x04         // 'writable' Attribute
#define ATR_BREAK  0x08         // 'Exec-Break' Attribute
#define ATR_EXECD  0x10         // 'Executed' Attribute
#define ATR_WATCH  0x20         // Location has a Watch */
#define ATR_BPDIS  0x40         // 'disabled Exec-Break' Attribute
#define ATR_PAP    0x80         // Location has a Perf.-Analyzer point */
#define ATR_WRBRK  0x100        // Loc has a write-access break
#define ATR_RDBRK  0x200        // Loc has a read-access break
#define ATR_COMC   0x400        // iMCS51/251: Common code marker
#define ATR_VNM    0x800        // iMCS51: von Neumann mapped
#define ATR_BEP    0x1000       // iMCS51: Bank-Entry Point

#define ATR_EXTR   0x2000       // 166/167: within range of an EXTR sequence
#define ATR_JTAKEN 0x4000       // Jump-taken attribute


                                // for DASMFOX:
#define LOBOUND    0x00000000   // current low bound
#define HIBOUND    0xFFFFFFFF   // Fox: current high bound


#define TMAP_ATTR_SECURE  0x00000008  // Secure Memory


struct irx  {
  char       *pS;           // the expression text
  WORD16  uError;           // Error-code
  int     uIndex;           // Error-Index
  int     nRadix;           // Expr.-Radix (-1:=auto, 10 or 16)  
  EXP        *e0;           // watch-expression
  EXP        *e1;           // store-expression
};
#define IRX  struct irx

/*
 * Real Scanner for Fox (SCANFOX1.C)
 */

#define  CINS  struct cins
#define  HNODES    1024   // 13.4.2001, 256
#define  IDHEAP   32768   // 2.8.2000, wg. G&D BullFuncs !!!  (old: 2048)

struct cins  {            // Command Input level structure
  char           achr;
  char         *pLine;    // the input line(s)
  int             ndx;    // read index
  int            lndx;    // index in current line
  int          iStart;    // start of following line index
  char         *fname;    // in case of FuncDef

  char         *iname;    // name of include file
  HANDLE        hFile;
  HANDLE        hFmap;
  DWORD         fSize;    // file length in bytes

  int          inHook;

  int          LineNo;
  int          nNodes;
  EXP          *nPool;
  WORD16         type;
  WORD16    PromptLen;    // length of Line-Prompt
  char         *idbuf;
  int           idndx;
  int            deep;    // FuncDef: '{' depth
//int       LastToken;

  DWORD       eof : 1;
  DWORD LineReady : 1;
  DWORD   InStruc : 1;    // struct/union member mode
  DWORD   LitMode : 1;    // get next token literally
  DWORD       eol : 1;    // end of line marker (delayed eol)
  DWORD      iDbm : 1;    // use ENV_DBM-Env for GetMem
  DWORD     iShow : 1;
  DWORD    iNoRes : 1;    // Watch-Asn expr.: no result output
  DWORD    iResSp : 1;    // resolve id to 'sp' mode

  int           stlin;
  WORD16       uError;    // Error number
  int          uIndex;    // Error index
  BYTE          shoot;    // Line number
  BYTE        ExprErr;    // Expression-Error Flag.
  DWORD        uAddre;    // Error address (eg. load)
  IRX            *vpx;
  void           *yyp;    // Yacc's value Stack

  BYTE          __eoi;    // placeholder
};



#define YYMAXD   300            // from YACCPAR !
#define  IPMAX    30            // input stack 1...29

extern int          nStack;     // Input-Stack index
extern CINS *iStack[IPMAX];     // Input Stack
extern CINS          *cinp;     // current input
extern char  yytext [1024];

//extern YYSTYPE      yylval;

extern int          ExpLin;     // Line-Number
//extern int             ndx;     // line index
extern BYTE           tdef;     // typedef-search flag
extern int           stlin;

#if 0      // moved to 166.h
 extern WORD16       uError;     // Error number
 extern int          uIndex;     // Error index
 extern BYTE          shoot;     // Line number
 extern DWORD        uAddre;     // Error address (eg. load)
 extern char         *uName;     // some name, eg. function name
#endif


extern CINS  *CreateInput (char *pLine, WORD16 PromptLen, WORD16 type);
extern void     KillInput (void);
extern void   InitScanner (void);
extern void     ClearHeap (void);            // after command
extern void    SetLitMode (void);

extern EXP      *HeapNode (void);
extern TYP      *MakeType (BYTE type, DWORD nSz, TYP *btp);

extern SYM      *vFindSym (SYM *table, char *name);
//extern ZBK      *FindTMod (char *name);
extern ZBK      *FindFunc (ZBK *Theadr, char *name);
extern LIN    *FindLineNo (ZBK *Theadr, DWORD LineNo);
extern SYM       *FindSym (char *s);
extern vSYM      *SrcvSym (char *name);
extern vSYM     *MakevSym (char *name, WORD16 index, TYP *tp);

extern int          yylex (void);
extern int        yyparse (void);


/*
 * NodeFox.C:
 */

extern void HandleErr (EXP *ep);    // special node with etyp := 0
extern EXP     *MkOp2 (WORD16 Op, WORD16 index, EXP *L, EXP *R);
extern EXP     *MkOp1 (WORD16 Op, WORD16 index, EXP *L);
extern EXP      *Hook (EXP *X, EXP *L, EXP *R);
extern EXP    *castex (EXP *L, TYP *tp, WORD16 Expl, WORD16 index);
extern EXP      *call (EXP *X, EXP *args);
extern EXP  *parmlist (EXP *e1, EXP *e2);
extern EXP   *SizeObj (WORD16 index, EXP *L);
extern EXP   *SizeTyp (WORD16 index, TYP *tp);
extern EXP   *undefid (EXP *L);
extern EXP   *StruRef (EXP *L, char *Member, WORD16 index, WORD16 ptr);
extern EXP    *RefArr (EXP *L, EXP *R);
extern EXP     *ExecC (WORD16 index, EXP *parms);
extern EXP   *DoQuali (SQUAL *pq);
extern EXP  *HandleDp (int dpn, EXP *e2);

extern TYP    *DoType (void);
extern void     makep (DWORD nAtr, DWORD cv);
extern void    ArrTyp (EXP *ep);
extern void     linki (TYP *t1, TYP *t2);
extern void    dclspc (DWORD mode);

extern void     popdp (void);
extern void    p_type (WORD16 base, WORD16 index);
extern void    p_sign (WORD16 sign, WORD16 index);
extern void    attrib (WORD16 attr, WORD16 index);
extern void        md (BYTE mode);
extern void        cl (BYTE sclass);
extern void    dclspc (DWORD mode);
extern TYP    *DoType (void);


extern void      CmdErr (void);
extern void   LineFlush (void);

extern void   DoExitCmd (void);
extern void   DoExprCmd (EXP *ep, BYTE Run);
extern void   DoEvalCmd (EXP *ep);
extern void   DoLoadCmd (char *name, WORD16 index, int lOpt);  // 1:=nocode
extern void   DoLogShow (void);
extern void    DoLogOff (void);
extern void    LogWrite (int nSel, char *pLine, int nMany);

extern void  ItmLogShow (void);
extern void   ItmLogOff (void);
extern void   ItmLogSet (int nMode, char *name, WORD16 index);
extern void ItmLogWrite (int nSel, char *pLine, int nMany);


extern void     DoReset (int nCmd);
extern void     MkWatch (EXP *ep, int index);
extern void   CreaWatch (IRX *pX);
extern WORD16  AsnWatch (char *exp, int nLen); // Assignment via WatchVw
extern WORD16 WatchCalc (EXP *ep, char *pB);
extern ZBK     *WaScope (DWORD nAdr);          // DasmFox.c
extern LIN    *FindLine (DWORD nAdr);          // search for Line
extern LINE32 *FindLn32 (DWORD nAdr);          // 28.1.2011

extern void      VaShow (char *pVar, WORD16 nIndex);
extern void       VaSet (char *pVar, WORD16 index1, char *pVal, WORD16 index2);
extern void     VaReset (char *pVar, WORD16 index);
extern void   InitLpath (char *name);

extern void    DoLogSet (int nMode, char *name, WORD16 index);
extern void   DoInclude (char *name, WORD16 index);
extern void       GoCmd (EXP *e1, EXP *e2);
extern void      DoStep (int type, EXP *ep);
extern EXP     *CalcAdr (EXP *ep);

extern void      SetBrk (EXP *ep, int ndx, int acc);
extern void       DlgBp (struct BpBlk *pB);     // create Dialog Bp
extern void      LstBrk (void);
extern void      BpComm (int nCode, EXP *ep);

extern void      PaDisp (void);        // list PA items
extern void      PaKill (EXP *ep);     // kill all or given PA items
extern void       PaDef (int ndx, EXP *e1, EXP *e2);

// 13.06.2012 -- Tracepoint extensions
extern void       SetTrc (int nCode, EXP *ep, int ndx);
extern void       LstTrc (void);
extern void      TrcComm (int nCode, EXP *ep);

// 23.01.2015 -- SW Breakpoint Configuration extensions
extern void SwBreakConfigList (void);
extern void SwBreakConfigSet  (EXP *epStart, EXP *epEnd, EXP *epSBEnable, EXP *epAccSz, EXP *epAccAlign);
extern void SwBreakConfigClear(EXP *epId);

// 04.10.2016 -- Target Memory Map extensions
extern void     TMapList (void);                                  // List Target Memory Map items
extern void     TMapSet  (EXP *epStart, EXP *epEnd, DWORD nAcc);  // Set Target Memory Map item
extern void     TMapClear(EXP *epId);                             // Clear Target Memory Map item


extern void     dirsym (void);                  // dir syms of current block
extern void     dirQua (SQUAL *sq, BYTE lines); // syms/lines of given module
extern void     dirlin (void);                  // dir lines of current module
extern void     dirpbs (void);                  // dir publics
extern void     dirioc (void);                  // dir VtRegs
extern void     dirmod (void);                  // dir modules
extern void    dirdsym (void);                  // dir 'define type id' symbols
extern void     dirfnc (int nSel);              // dir functions
extern void   dirScope (SQUAL *sq);             // display scope

extern WORD16   RunCmd (char *cmd, int nLen);
extern void    BootExp (void);

extern void     ClrExp (void);
extern EXP  *UpNodeTop (EXP *ep);            // NodeFox.c
extern void    DumpExp (EXP *ep);            // PriFox.c


extern DWORD Get_pL_Line (struct lin *pL);   // UVView.cpp
extern DWORD Get_pL_Offs (struct lin *pL);   // ----------




//--- Show/Set/Reset env-vars:

#define  VST   struct vset
struct vset  {
  char    name [14];                               // name of env-var
  void    (*sfc) (VST *pV, char *pS, WORD16 ndx);  // 'set' function
  void    (*xfc) (VST *pV);                        // 'show' function
  void    (*rfc) (VST *pV);                        // 'reset' function
  int     which;
};

struct szPAT  {                // search path list
  struct szPAT  *next;
  char          *text;
};


/*
 * CALL-Stack
 */

struct calstk  {
  DWORD      caller;      // callers opcode address
  DWORD      callee;      // called address
  DWORD          sp;      // stack pointer value before call
  DWORD       nextA;      // instruction after Call
  BYTE        icall;      // indirect call mark
};
#define CSTK      struct calstk
#define MAXCSTK   256




/*
 * Performance Analyzer
 */

struct pa  {
  struct pa     *next;     // next PA range
  struct pa     *prev;     // prev PA range
  DWORD         entry;     // PA-Range entry address
  DWORD          exit;     // PA-Range exit address
  DWORD        icount;     // Invocation count
  UINT64          cyc;     // cycles of last measure
  UINT64       mincyc;     // minimum cycles (for average)
  UINT64       maxcyc;     // maximum cycles (for average)
  UINT64       totcyc;     // total cycles of PA-Range
  WORD16         Slot;     // Slot-Number
  char         *ptext;     // PA-Expression
  int            nLen;     // Length of PA-Expr. Text
  int          Extent;     // Extent from GetTextExtent ...
//float        spcent;     // Percent on Screen
  float        cpcent;     // current Percent

  BYTE           type;     // 1:=FuncPA, 0:=RangePA
  BYTE     active : 1;     // active bit
};

struct past  {             // used for PA-Runtime-Stack
  UINT64      ecyc;
  DWORD     paexit;        // for PA entry,  return address, otherwise 0
  struct pa    *pa;
};

#define  PA      struct pa
#define  PAST    struct past
#define  PADEP   256
#define  RPAT    0         // Range-PA
#define  FPAT    1         // Function-PA Range

extern void    PaDef (int ndx, EXP *e1, EXP *e2);
extern void   PaKill (EXP *ep);
extern void   PaDisp (void);
extern void  PaReset (void);

extern DWORD  PaFunc (int nCode, int nAdr, void *p);



//extern void SimInit (WORD16 nFamily);
//extern void SimStep (DWORD nCount);



/*
 * Dynamic Menu construction and maintanance
 */

#pragma pack(1)

#define DLGD struct DlgDat
struct DlgDat   {            // every dialog has it's own structure
  DWORD             iOpen;   // auto reopen dialog (pos := 'rc')
  HWND                 hw;   // Hwnd of Dialog
  BOOL (CALLBACK *wp) (HWND hw, UINT msg, WPARAM wp, LPARAM lp);
  RECT                 rc;   // Position rectangle
  void   (*Update) (void);   // Update dialog content
  void (*Kill) (DLGD *pM);   // Kill dialog
  void                *vp;   // reserved for C++ Dialogs (Dlg *this)
};

#define DYM  struct DynaM
struct DynaM  {             // Menu item data structure
  int              nDelim;  // Menu template delimiter
  char            *szText;  // Menu item text
  void    (*fp) (DYM *pM);  // create/bringDlgtoTop function
  DWORD               nID;  // uv3 assigned ID_xxxx
  DWORD            nDlgId;  // Dialog ID
  DLGD              *pDlg;  // link to dialog attributes
};
#pragma pack()


/*
 * nDelim:  1 := normal Menu entry
 *          2 := Popup-Entry (nested submenu)
 *          3 := relocated entry
 *          4 := Separator (MF_SEPARATOR)
 *         -3 := end of relocated entry
 *         -2 := end of Popup-Group-List
 *         -1 := total end of Menu-List
 *  text:   the name for the menu/popup-menu entry
 *    fp:   Function to be activated on menu-selection
 */


/*
 * Dll-Query structure
 */

struct qdll  {
  HWND       parent;        // parent window handle
  char     *pathUv3;        // path of Target-DLL
  HMODULE     Linst;        // DLL Instance handle

  int       ValSize;        // size of 'value[]' Buffer
  char         *key;        // Register key for driver DLL's
  char       *value;        // some command line, for example
                            // more info could be here ...
  char     *prjname;        // project path+name
};
#define QDLL  struct qdll




/* Memory Management */
struct  MM {
  BYTE    *mem;            // Pointer to Memory Image
  WORD16  *atr;            // Pointer to Memory Attributes
};



union op  {                /* Opcode of current instruction */
  BYTE       b[8];
  WORD16     w[4];
  DWORD        ul;
  DWORD    ula[2];
  UINT64      u64;
};

//static union op      op;     // Opcodes

union rstat  {             /* Execution Status of dScope */
  BYTE          status;

  struct  {
    BYTE     BreakOp : 1;    /* Break-Opcode reached */
    BYTE     CntZero : 1;    /* Step-count reached zero. */
    BYTE     SomeErr : 1;    /* some error occurred */
    BYTE     StopBut : 1;    /* Stop-Button has been pressed. */ 
    BYTE     ExecBrk : 1;    /* stopped by an execution breakpoint */
    BYTE      AccBrk : 1;    /* stopped by an access breakpoint */
    BYTE      CndBrk : 1;    /* stopped by a conditional breakpoint */
    BYTE        Stop : 1;    /* stopped via '_break_' variable */
  } inf;
};

#define  TIMS  256           // 100
#define  TMW   struct tmw

struct tmw  {
  INT64             tmv;     /* Cycle value */
  void     (*tmf)(void);     /* function to be activated */
  int              tndx;     /* Time-Watch Number */
  DWORD            uSec;     // micro-seconds based timewatch
};

extern TMW   tixx [TIMS];    /* Array of Time-Watches */
extern int   numtim;         /* # of time-entries */
extern int   tndx;
extern int   tact;           /* TimeWatch-Activators id */


//--- FOX specific:

union itrp  {              /* Interrupt Status of Instructions */
  struct  {
    WORD16   hwtrap;         /* Hardware Traps */
    WORD16     flag;         /* Interrupt Flag */
  } w;
  DWORD      stat;
};


struct FoxReg  {
  union rfox   {
    DWORD   dregs [16];      /* D0  ... D30 */
    WORD16  wregs [16];      /* R0  ... R15 */
    BYTE    bregs [16];      /* RL0 ... RH7 */
  } r;
  DWORD        Ndpp[4];      /* Normierte DPP Register */ 
  DWORD            nPC;      /* full address ! */
  DWORD           cPsw;      /* DPSW */
  DWORD            cCP;      /* DCP */
  INT64        nCycles;      /* cycle counter */
  DWORD        cGPRCON;      /* DGPRCON */
};

struct FoxCont  {
  union itrp      itrp;    // Interrupt status
  BYTE           sleep;    // CPU sleep mode
  BYTE           trace;    // record trace information
  BYTE       intchange; 
  BYTE          intdis;
  DWORD           cPsw;    // current PSW
  DWORD           oPsw;    // old PSW
  struct FoxReg      r;    // Fox-Registers
};


//--- 166/167 specific's: -------------------------------

#define OFO(s,m)   (DWORD)&(((s *) 0)->m)     /* Offsetof Macro */

#pragma pack(1)
struct Reg166  {
  union  {
    WORD16  wregs [16];      // R0  ... R15
    BYTE    bregs [16];      // RL0 ... RH7
  } r;
  DWORD        Ndpp[4];      // full linear base address values
  DWORD            nPC;      // full address !
  WORD16          cPsw;      // current Psw
  WORD16           cSP;
  WORD16          cMDL;      // current MDL
  WORD16          cMDH;      // current MDH
  DWORD            cCP;      // CP
  INT64        nCycles;      // cycle counter

  INT64           macc;      // 40-Bit value
  WORD16           mah;      // MAH MAC-Unit Accumulator High
  WORD16           mal;      // MAL MAC-Unit Accumulator Low
  WORD16           mas;      // MAS limited MAH/signed
  WORD16           msw;      // MSW MAC-Unit Status Word
  WORD16           mcw;      // MCW MAC-Unit Control Word
  WORD16           mrw;      // MRW MAC-Unit Repeat Word

  WORD16          idx0;
  WORD16          idx1;
  WORD16           qx0;
  WORD16           qx1;
  WORD16           qr0;
  WORD16           qr1;
  WORD16        cSPSEG;      // HS 31.03.2005 XC16x support added
};

#pragma pack()
                             // use default packing !!!
struct Regx51  {             // iMCS51-Registers
//union  {
//  BYTE    bregs [16];      // R0 ... R7
//} r;
  BYTE      bregs [16];
  DWORD            nPC;      // full address !
  BYTE              sp;      // SP
  BYTE             psw;      // PSW-sfr
  BYTE               b;      // B-sfr
  BYTE             acc;      // ACC-sfr

  BYTE          dpl[8];      // DPL-sfr (a0...a7)
  BYTE          dph[8];      // DPH-sfr (a8...a15)
  BYTE          dpx[8];      // DPX-sfr (a16...a23) /Dallas C390/
  BYTE           dpSel;      // DPSEL-sfr
  BYTE             esp;      // ESP-sfr /Dallas C390/

//BYTE        ports[8];
  INT64        nCycles;      // cycle counter
};


#pragma pack(1)
struct Reg51MX  {            // 80C51MX-Registers (Philips)
  BYTE       bregs [8];      // R0...R7
  BYTE              sp;      // SP
  BYTE             spe;      // MX SP-extendend (high byte)
  BYTE             psw;      // PSW-sfr
  BYTE               b;      // B-sfr
  BYTE             acc;      // ACC-sfr

  BYTE          dpl[2];      // DPL0 (DPL sfr)
  BYTE          dph[2];      // DPH0 (DPH sfr)
  BYTE           mxcon;      // MXCON sfr
  BYTE           auxr1;      // AUXR1 sfr
  BYTE             epl;      // EPL sfr
  BYTE             epm;      // EPM sfr
  BYTE             eph;      // EPH sfr
  BYTE        ports[4];

  DWORD            nPC;      // full address !
  INT64        nCycles;      // cycle counter
  DWORD         ZF : 1;      // Zero-Flag: 80C51MX+Ext
};

#if 0
struct Rg5016 {              // Mifare Pro X P8RF516 Registers
  DWORD            nPC;
  INT64        nCycles;      // cycle counter
  BYTE             psw;
  BYTE              sp;
  BYTE               b;
  BYTE             acc;
  BYTE             dpl;
  BYTE             dph;
  BYTE       bregs [8];
  BYTE           dpsel;
  BYTE           adrxh;      // page for MOVX @Ri

  BYTE          rename;      // Accu rename sfr
  WORD16           ssp;      // SP - system mode
  WORD16           usp;      // SP - user mode
  BYTE            pswh;      // PSW high (mode register)
  BYTE          repeat;      // Repeat instr. n times
  BYTE          xbase0;      // BaseAddr of Seg-0 Xram for all modes
  BYTE          xbase1;      // BaseAddr of Seg-1 Xram for all modes
  BYTE          xsize0;      // Size of Seg-0 Xram, user mode
  BYTE          xsize1;      // Size of Seg-1 Xram, user mode
  WORD16         cbas0;      // CodeBaseAdr Seg-0, user mode
  WORD16         cbas1;      // CodeBaseAdr Seg-1, user mode
  BYTE          csize0;      // Size CodeSeg-0, user mode
  BYTE          csize1;      // Size CodeSeg-1, user mode
  BYTE             lbs;      // Low bank select
  BYTE             hbs;      // High bank select
};
#endif


struct Reg251  {         // 80251 Registers
  union  {
    BYTE    bregs [16];
    WORD16  wregs [16];
    DWORD   dregs [16];
  } r;

  DWORD            nPC;  // full address !
  BYTE              sp;  // SP
  BYTE             psw;  // PSW-sfr
  BYTE            psw1;  // PSW1-sfr
  BYTE               b;  // B-sfr
  BYTE             acc;  // ACC-sfr
  BYTE             dpl;  // DPL-sfr
  BYTE             dph;  // DPH-sfr
  BYTE        ports[4];
  INT64        nCycles;  // cycle counter
};

struct RegS6  {          // iMCS51 - SLE66Cxx (ECO2000) Registers
  DWORD          nPC;
  INT64      nCycles;    // cycle counter
  BYTE           psw;
  BYTE            sp;
  BYTE             b;
  BYTE           acc;
  BYTE           dpl;
  BYTE           dph;
  BYTE     bregs [8];
  BYTE         dpsel;
  BYTE         adrxh;    // page for MOVX @Ri

  WORD16    dptx [8];    // DPTR save area
};

struct RegARM   {        // iMCSARM Registers
  DWORD      cur[16];    // Current Mode:   R0..R15(PC)
  DWORD         CPSR;    // CPSR
  DWORD         SPSR;    // Current SPSR
  DWORD       usr[7];    // User & System:  R8..R14
  DWORD       fiq[8];    // Fast Interrupt: R8..R14, SPSR
  DWORD       irq[3];    // Interrupt:      R13,R14, SPSR
  DWORD       svc[3];    // Supervisor:     R13,R14, SPSR
  DWORD       abt[3];    // Abort:          R13,R14, SPSR
  DWORD       und[3];    // Undefined:      R13,R14, SPSR
  INT64      nCycles;    // cycle counter
};

#pragma pack()

#define SZ_R166   (OFO (struct Reg166, macc))
#define SZ_ST10   (sizeof (struct Reg166))
//#define SZ_XCRG   (sizeof (struct Reg166))
#define SZ_RX51   (sizeof (struct Regx51))
#define SZ_SLSZ   (sizeof (struct RegS6))
#define SZ_S251   (sizeof (struct Reg251))
#define SZ_51MX   (sizeof (struct Reg51MX))
#define SZ_RARM   (sizeof (struct RegARM))


/*
 * iMCS-51 SFR-Attribute codes
 */

#define nSF   0x0000      // SFR not present
#define iSF   0x8000      // SFR is present
#define pSF   0x4000      // SFR is present and is Port-0...3
#define cSF   0x2000      // ECO 'external SFR' (EEPROM,TIMER,UART)
#define dSF   0x1000      // 80320: Sfr is DPL/DPH or DPL1/DPH1
// Note: the above need to be combined with ATR_READ/ATR_WRITE
//       for selectively attribute configuration
//       iAV replaced by ATR_READ/ATR_WRITE


/*
 * Memory spaces
 */

#define   mmNONE   0x0000           // not spaced
#define   mmXDATA  0x0001           // XDATA
#define   mmBANK0  0x0080           // BANK0
#define   mmBANK31 0x009F           // BANK31
#define   mmDATA   0x00F0           // DATA
#define   mmBIT    0x00F1           // BIT
#define   mmEDATA  0x00F2           // EDATA (i251)
#define   mmIDATA  0x00F3           // IDATA
#define   mmECODE  0x00F4           // 251 ecode
#define   mmHDATA  0x00F5           // 251 hdata
#define   mmHCONS  0x00F6           // 251 hconst
#define   mmCONST  0x00F7           // 251 const
#define   mmMSFR   0x00F8           // CC7 MSFR
#define   mmPDATA  0x00FE           // PDATA (c51 macht das bei generic)
#define   mmCODE   0x00FF           // CODE 
#define   mmPHYS   0x0100           // Physical SLE66+ Memory
#define   mmPHLIM  0x0110           // 1MB Physical Memory

#define   mmERAM   0x0001           // ARM ERAM
#define   mmSSFR   0x00EF           // Silabs EFM8 banked sfr

#define   mmU1MEM  0x00F8           // UsrMem #1 (E2-PROM)
#define   mmU2MEM  0x00F9           // UsrMem #2 (undefined)
#define   mmU3MEM  0x00FA           // UsrMem #3 (undefined)
#define   mmU4MEM  0x00FB           // UsrMem #4 (undefined)
#define   mmU5MEM  0x00FC           // UsrMem #5 (undefined)
#define   mmU6MEM  0x00FD           // UsrMem #6 (undefined)

#define   mmSFR000 0x0200           // SFR-Bank 0...255
#define   mmSFR255 0x02FF           // SFR-Bank 255 (D:255)


/*
 * Silabs EFM8 banked Sfr addresses for use in SVD / S8051  /26.2.2015/
 *   (mmSSFR << 24) | (SfrPage << 16) | (SfrAdr & 0xFF)
 *   0xEF0000aa :=  sfr(aa) in sfr-bank #0 (standard 8051 Sfr-range 0x80-0xFF)
 *   0xEF0100aa :=  sfr(aa) in sfr-bank #1
 *   0xEFFF00aa :=  sfr(aa) in sfr-bank #255
 */


 
/*
 * Memory spaces (ARM v8-M)
 */

#define   mmNSEC   0x0001           // Non-Secure Memory
#define   mmSEC    0x0002           // Secure Memory


#if 0
//--- SLE88Cxx specific's: -------------------------------

struct RegSle  {
  union  {
//  WORD16   wregs[16];      // ECO-2 word regs
    DWORD    dregs [9];      // SLE R0-R7,A
  } r;
  DWORD            nPC;      // LoWord is relevant !!!
  DWORD            Psw;      // SLE PSW (32-Bit)
  DWORD           Reco;      //
  DWORD           Aram;      // 
  INT64        nCycles;      // cycle counter
  WORD16            sp;
  BYTE              cs;
  BYTE              ss;
  BYTE             ds0;
  BYTE             ds1;
  BYTE             ts0;
  BYTE             ts1;
};
#endif


//--------------------------------------------------------------

#define iSLE88     100     // dead !
#define i80x51     0x00    // generic 80x51
#define iEXTMAC    0x01    // MAC167
#define iSLE66     0x02    // SLE66
#define iC51MX     0x03    // Philips 80C51MX
#define iRF5016    0x04    // Philips P8RF5016 (Mifare pro X)
#define iSMARTMX   0x05    // Philips SmartMX
#define iSMARTMX2  0x06    // NXP SmartMX2


#define xxMENU      UsrDwrd[0]  // holds CAN-Extension Menu (DYM *)
#define xxUPDATE    UsrDwrd[1]  // holds 'void (*CanUpdate) (DWORD nCode)'
#define xxMEMCB     UsrDwrd[2]  // holds 'DWORD (*pFlashAcc) (DWORD nAdr, DWORD nCnt)'
#define xxINSTTAB   UsrDwrd[3]  // holds 'void (_fastcall *insttab [256]) ()'
#define xxAGSIPREX  UsrDwrd[4]  // holds pointer to AGSI Pre-Exec-CallBack-Table (SmartMX only)
#define xxAGSIPOEX  UsrDwrd[5]  // holds pointer to AGSI Post-Exec-CallBack-Table (SmartMX only)
#define xxAGSIADRX  UsrDwrd[6]  // holds pointer to RegisterAdrBreak function (SmartMX only)
#define xxAGSINPC   UsrDwrd[7]  // holds new logical program counter if != 0xFFFFFFFF (SmartMX only)
#define xxAGSIRSTL  UsrDwrd[8]  // holds ResetLevel and is used together with wdog_rst (SmartMX only)

#define xxCWINAPPP UsrDwrd[10]  // holds 'CWinApp *'  /3.11.2004/
#define xxAGSIEXCB UsrDwrd[11]  // holds AGSI Pre-Exec-Ins CallBack
#define xxAGSICO   UsrDwrd[12]  // holds AGSI CmdWinOut (of 'exec ("dir")')
#define xxSFRMUX   UsrDwrd[13]  // holds 'SfrMux' ((BYTE *) &SfrMux[0])  /SLE66/
#define xxDCIPRM   UsrDwrd[14]  // holds 'DWORD &_dciUV2' for DCI Interface
#define Dio    ((struct dciUV2 *)(pio->UsrDwrd[14]))->dio
#define Dconn  ((struct dciUV2 *)(pio->UsrDwrd[14]))->dconn

#define xxCSIOP    UsrWord[0]   // used by UL2uPSD.DLL. holds CSIOP value for TP51.DLL

//
// Sle66: ioc.pUv[0] = WORD16 *pTab;  // 256 Words fuer Peripheral-Config
//



/*
 * RTAH message routing bits
 */
#define xRTAH_ROUTE_INT    0x0001  // route internally to RTAH
#define xRTAH_ROUTE_TCP    0x0002  // route via TCP/IP
#define xRTAH_ROUTE_RTX    0x0004  // route into RTX-Dialog DLL
#define xRTAH_ROUTE_UV3    0x0008  // route into UV3 (for serial window, etc)


#define xRTAH_TYPE_RTX     0
#define xRTAH_TYPE_ISR     1
#define xRTAH_TYPE_USR     2
   
/*
 * DCC-message format, internal UV3/TCP/RTX-Dlg/
 */
#pragma pack(1)
typedef struct dccmsg  {     // DCC-Message-format
  BYTE         nTarg;        // Route-Mask-Bits: RTAH_ROUTE_xxx
  BYTE   rtaType : 4;        // type of record (RTX, ISR, UserEvent)
  BYTE    nUnus1 : 4;        // unused
  WORD16        nLen;        // length of message 'aMsg[]'

  DWORD        nRes1;
  DWORD      nOsTime;        // 32Bit tick counter
  BYTE        *pData;        // incoming agent data
  BYTE     aMsg[256];        // actually 'nLen' bytes !
} DCCMSG;
#pragma pack()


#define MAXDCCB (1024 * 1024)  //32768        // max. buffered DCC-messages (power 2 !)
typedef struct   {           // DCC-Message-Buffering in Uv3.Cpp
  DCCMSG     **pRec;         // DCC-Messages
  int          nEnd;         // 0...MAXLAR
  int          nIdx;         // 0...MAXLAR-1
  int       nMaxRec;         // max. LA-Records (pRec[])  (power of 2)

  int       nTcpIdx;
  int            nR;         // not by Uv3
  int         rCode;         // not by Uv3
  DWORD     Res[16];         // not by Uv3
} DCCINFO;


/*
 * Paramters to 'UV_ADRSYN_ASM_HLL' and 'UV_ADRSYN_ASM_HLL_EXT' Notifications
 *  bAsm:  1:=want disassembly --> bAsmRes: 1:=Ok
 *  bHll:  1:=want hll file    --> bHllRes: 1:=Ok, 0:=not possible
 */
typedef struct showsynca  {
  UINT64               nAdr;                // input address to 'show-sync'
  UINT            bAsm :  1;                // 1:=want disassembly
  UINT            bHll :  1;                // 2:=want hll file/line
  UINT         bAsmRes :  1;                // return: 1:=diassembly Ok. (always)
  UINT         bHllRes :  1;                // return: 1:=hll Ok.
  UINT      bSaveFocus :  1;                // 1:=Don't change focus window, 0:=allow focus window to change
  // The following flags are used for 'UV_ADRSYN_ASM_HLL_EXT' only
  UINT      bAsmNoOpen :  1;                // 0:= Open disassembly view and activate, 1:= don't open disassembly view
  UINT      bHllNoOpen :  1;                // 0:= Open HLL file and activate, 1: don't open HLL file
  UINT  bAsmNoActivate :  1;                // 0:= Activate disassembly view if open, 1:= don't activate...
  UINT  bHllNoActivate :  1;                // 0:= Activate HLL view if open, 1:= don't activate...
  // Reserved
  UINT                 : 23;                // currently not used.
  UINT              nRes[7];                // currently unsued, all zeros.
} SHOWSYNCA;


/*
 * Expand input line ($M, etc.)  // 18.3.2011
 *  pLineIn  := character line to expand
 *  pLineOut := expanded line result
 *  sizeOut  := size of provided pLineOut buffer
 */
typedef struct kexpander  {
  char        *pLineIn;          // in: input line to expand
  char       *pLineOut;          // in: expansion line buffer
  int          sizeOut;          // in: size of expansion line buffer

  DWORD        nRes[8];          // currently reserved
} KEXPANDER;


/*
 * Paramters to 'UV_KILL_PCLINE' Notification (can be combined)
 */
#define KPC_EDITOR  0x00000001    // Kill PC Marker in Editor View
#define KPC_DASM    0x00000002    // Kill PC Marker in Disassembly View (not supported yet)


/*
 * Uv3 - Notification-Commands  /1.8.2005/
 */
typedef enum uv_notify  {
  UV_DBG_INITIALIZED    =  0,     // debugger has been initialized
  UV_RUN_STARTED        =  1,     // Go/Step started
  UV_RUN_COMPLETED      =  2,     // Go/Step completed
  UV_KILL_FOCUSLINE     =  3,     // remove focus line in editor/dasm view(s)
  UV_KILL_WATCHES       =  4,     // kill all Watch-Windows
  UV_DBG_RESET_GIVEN    =  5,     // debugger - Reset was given
  UV_DBG_POST_LOAD      =  6,     // debugger - Load has been completed
  UV_ISTEP              =  7,     // Uv: 'Step Instr.' button klicked
  UV_OSTEP              =  8,     // Uv: 'OverStep Instr.' button klicked
  UV_STEPOUT            =  9,     // Uv: 'StepOut' button klicked
  UV_DISABLE_ALL_BP     = 10,     // Uv: 'Disable all Bp' klicked
  UV_KILL_ALL_BP        = 11,     // Uv: 'Kill all Bp' klicked
  UV_DBG_QUERYSTEPOUT   = 12,     // Uv->Dbg: is StepOut possible
  UV_IS_ASMWIN_OPEN     = 13,     // Uv: is DisAsm-Window open
  UV_UPDATE_GUTTERS     = 14,     // Uv: update gutters of Editor(s)/Dasm
  UV_QUERY_OK_CANCEL    = 15,     // Uv: AfxMessageBox Ok/Cancel
  UV_UPDATE_PA_VIEW     = 16,     // Uv: update performance-Analyzer view
  UV_RECALC_PA_VIEW     = 17,     // Uv: Calc performance-Analyzer internals
  UV_RECALC_PA_UPDATE   = 18,     // Uv: ReCalc PA and Update
  UV_PA_EXECUPDATE      = 19,     // Uv: Update PA while executing
  UV_DBG_ABOUT_TO_STOP  = 20,     // Dbg: About to terminate the debugger
  UV_DBG_SYNC_TO_ADDR   = 21,     // Dbg: Sync Editor(s)/DisAsm to Address
  UV_DBG_MEM_CHANGED    = 22,     // Dbg: possible memory change
  UV_GETLINE_DIALOG     = 23,     // Uv: get line from query dialog
  UV_CREATE_WATCH       = 24,     // Dbg: create Watch in Window[0-3]
  UV_GET_CMDWINSIZES    = 25,     // Dbg: get CmdWin nLines/nChars per line
  UV_DBG_DISASM_ADDR    = 26,     // Dbg: DisAssemble from Address
  UV_DBG_DISASM_PGDOWN  = 27,     // Uv:  DisAssemble (PageDown Mode)
  UV_BP_CHANGED         = 28,     // Uv: global Breakpoint change
  UV_INIT_MEMWINDOW     = 29,     // Uv: Init Memory-WIndow Session Info
  UV_SHOW_NEXT_STMT     = 30,     // Show next executable Stmt (Editor(s)/DisAsm)
  UV_PERIODIC_UPDATE    = 31,     // Dbg: perform a periodic update
  UV_DEFINE_BUTTON      = 32,     // Define a Tool-Command Button
  UV_KILL_BUTTON        = 33,     // Kill a Tool-Command Button
  UV_EXEC_TOOLBOXCMD    = 34,     // Execute a ToolBox-Button Command
  UV_DECODE_AN_ERROR    = 35,     // Dbg: decode an error code into a buffer
  UV_SCANNER_RQ_ILINE   = 36,     // Dbg: request a new input line
  UV_SET_CMD_PROMPT     = 37,     // Set Prompt in CmdView
  UV_DBG_SHUTDOWNDELAY  = 38,     // delayed ShutDown via 'Exit' Command
  UV_QUERY_OK           = 39,     // Uv: AfxMessageBox Ok query
  UV_SYNC_INLASM        = 40,     // Inline-Asm address sync
  UV_SCRIPT_ERROR       = 41,     // Dbg: Put function compile error
  UV_WAITCURSOR         = 42,     // Begin/End-Waitcursor
  UV_PROGRESS_SETUP     = 43,     // Init Progress-Bar
  UV_REG_REGTREE        = 44,     // Register/Unregister RegTree-registers
  UV_UPDATE_MEMWATCH    = 45,     // Update Memory and Watch Window(s)
  UV_GET_TARG_REGKEY    = 46,     // Get user Registry value
  UV_SET_TARG_REGKEY    = 47,     // Set user Registry value
  UV_DBG_SERBOUTPACKED  = 48,     // SerialByte out packed
  UV_CLEAR_WATCH1_OR2   = 49,     // Clear WatchWindow #1/#2
  UV_REFRESH_DISASM     = 50,     // Refresh Disassembly Window
  UV_UPDATE_LA          = 51,     // Refresh Logic-Analyzer Window
  UV_STOP_RUN           = 52,     // Stop running
  UV_RESET              = 53,     // Reset
  UV_UPDATE_LA_VIEW     = 54,     // UV: update performance-Analyzer view
  UV_UPDATE_SELECTIVE   = 55,     // UV: update selected windows (see example below)
  UV_EVENT_FEEDBACK     = 56,     // UV advanced Message Box / error feedback
  UV_BREAKPOINT_RSP     = 57,     // Sxxx: Breakpoint-command Response
  UV_ADRSYN_ASM_HLL     = 58,     // Show Disassembly/HLL for given address
  UV_UVSOCK_GUI_MSG     = 59,     // Process a UVSOCK message meant for the GUI thread
  UV_TRACE_INIT_WIN     = 60,     // Initialize Trace-Display-Window
  UV_TRACE_ADD_SAMPLE   = 61,     // Add a trace-sample to trace-Display-Window
  UV_TRACE_CLEAR_WIN    = 62,     // Clear Trace-Display-Window
  UV_TRACE_RESET_WIN    = 63,     // Reset Trace-Window [NOT USED!!!!!!!]
  UV_CACHE_INVALIDATE   = 64,     // Invalidate code cache (issued via 'Update-Windows' ToolBox Button)
  UV_REG_ITM_CLIENT     = 65,     // register a ITM-client (Cortex-M - viewer receiver)
  UV_UNREG_ITMCLIENT    = 66,     // unregister a ITM-client (Cortex-M - viewer receiver)

  UV_HLL_STEP           = 67,     // UVSC: High Level Language 'Step In' independent of current View
  UV_HLL_OSTEP          = 68,     // UVSC: High Level Language 'Step Over' independent of current View
  UV_HLL_STEPOUT        = 69,     // UVSC: High Level Language 'Step Out' independent of current View
  UV_INSTR_OSTEP        = 70,     // UVSC: 'oStep' (independent of current View !)
  UV_INSTR_STEPOUT      = 71,     // UVSC: 'oStep' (independent of current View !)
  UV_SAVE_WATCHES       = 72,     // extract Watch-panes contents
  UV_UPD_FILE_SYNTAX    = 73,     // update syntax error in editor and error list pane
  UV_RESTART_SYNTCHECK  = 74,     // retrigger syntax check which has been queued
  UV_TRIGGER_UPDERRLST  = 75,     // retrigger updating error list
  UV_TRIGGER_DISPCODECOM = 76,    // trigger showing list of members
  UV_TRIGGER_DSC        = 77,     // trigger syntax check
  UV_SYN_HLL_WITH_ASM   = 78,     // Show HLL for given address

  UV_OUTPDBG_LINE       = 80,     // output line from debugger into CmdView

  UV_ADRSYN_ASM_HLL_EXT = 100,    // Show Disassembly/HLL for given address, using additional flags in SHOWSYNCA
  UV_SYNC_INLASM_NOSRC  = 101,    // deprecated!, Inline-Asm address sync without automatic sync to HLL source file
  UV_GET_RTXCONTEXT     = 102,    // get current RTX-context
  UV_EDIT_DOCUMENT      = 103,    // Open Selected Document File
  UV_KILL_PCLINE        = 104,    // remove PC marker in editor/dasm view(s)
  UV_PRE_LOADAPP        = 105,    // before loading a user application 
  UV_LOGWRITE           = 106,    // queue LogWrite output
  UV_WINHELP            = 107,    // execute WinHelp in UV4
  UV_GET_LLVM_INFO      = 108,    // ask UV for filling translation parameters (by the indexer)
  UP_UPDATE_BROWSER     = 109,    // tell UV for updating the Source Browser

  UV_PERIODIC_UPD_RTX   = 310,    // Dbg: periodic update with RTX-Dailog(s)

  UV_CALC_EXPR          = 0x800,  // DTC: calc expr., used for DTC Event
  UV_CLEAR_BP_BY_ADDR   = 0x801,  // DTC: clear Breakpoint by CodeAddress
  UV_MSG_TO_TARG        = 0x802,  // DTC: Transfer client Message to Target
  UV_MSG_FROM_TARG      = 0x803,  // DTC: Send Message from Target to Client
  UV_READ_WRITE_MEMORY  = 0x804,  // DTC: Read/Write Memory
  UV_INSTR_STEP         = 0x805,  // DTC: 'iStep' (independent of current View !) 
  UV_GET_CURDOC         = 0x806,  // DTC: get name of current editor document
  UV_KICK_IDLE          = 0x807,  // DTC: Kick Idle Message
  UV_TMSG_CONN_DISCONN  = 0x810,  // DTC: TargetMessaging connectet/disconnected
  UV_SMN_CALCEXPR       = 0x820,  // SMN_CALCEXPR [ calc address ]
  UV_SMN_XCALCEXPR      = 0x821,  // SMN_XCALCEXPR [ calc expr. ]
  UV_SOCK_TIME_CB       = 0x822,  // VifSock: Socket TimeIntercal CallBack
  UV_SERIAL_IN          = 0x823,  // VifSock: Serial-Input from client
  UV_SOCK_SER_TIME_CB   = 0x824,  // VifSock: Socket Serial TimeInterval CallBack
  UV_DCC_MESSAGE        = 0x825,  // DCC-Message
  UV_REMOVE_CPRJITEM    = 0x826,  // Remove an Item from the current Project (Group/File)
  UV_RTAH_ERRMSG        = 0x827,  // RTA-Error message (displayed in StatusBar)
  UV_RESOLVE_TO_SYM     = 0x828,  // SMN_RESOLVE_TO_SYM
  UV_GETCALLSTACK       = 0x829,  // SMN_GETCALLSTACK
  UV_UPDATE_RTX_TRACE   = 0x830,  // Update RTX trace data

  UV_NEW_TARG_SETTINGS  = 0x831,  // Update target settings from within debug-session
  UV_DOXEVENTS          = 0x832,  // Run MessagePump 'n' times  /test 17.11.2009/
  UV_EXPANDKEYS         = 0x833,  // expand keys  /18.3.2011/
  UV_DBG_ACCESS_LEVEL   = 0x834,  // Limited debug access detected

  UV_STATUS_MESSAGE     = 0x835,  // message on statusbar at first position

  UV_TRACE_IF_INIT      = 0x900,  // Initialize trace interface
  UV_TRACE_IF_STATUS    = 0x901,  // Trace interface target status update
  UV_TRACE_WIN_CONFIG   = 0x902,  // Trace data window setup
  UV_TRACE_WIN_SYNC     = 0x903,  // Trace windows synchronization

  UV_UPDATE_EVTREC_VIEW = 0x910,  // Update Event Recorder View (GUI only)
  UV_EV_COMPLETED       = 0x911,  // internal - (EVTRECORDER_ITEM *) for Event-Recorder Extensions...
  UV_DECODE_EVITEM      = 0x912,  // decode an event item using 'EVR_DECODE' data
  //UV_EV_OPENTRXCH       = 0x913,  // deprecated! Add an RTX Channel that was found in the EVR RTX5 data
  UV_EV_POWERLOG        = 0x914,  // write power-statistics to log file

  UV_RTOS_SUB_MENU_IF   = 0x1000,     // RTOS Viewer Sub-Menu interface

  UV_PDSCDBG_GET_PROPERTIES = 0x1100, // PDSC Debug Description: Get Properties. Returns 0 if no valid debug description found, 1 if success, else error
  UV_PDSCDBG_SET_PROPERTIES = 0x1101, // PDSC Debug Description: Set Properties as changed in DLL setup dialogs.
  UV_PDSCDBG_EXEC_SEQUENCE  = 0x1102, // PDSC Debug Description: Execute requested sequence (PDSC_SEQUENCE_CONTEXT),
                                      // return 0 if sequence not implemented, 1 if success, else error

} UV_NOTIFY;



/*
 * Centralized Cortex-M/R device identifiers  /27.11.2011/
 */
typedef enum cpuenum  {
  CORTEX_NONE        = 0,
  CORTEX_M0          = 1,
  CORTEX_M1          = 2,
  CORTEX_M3          = 3,
  CORTEX_M4          = 4,
  CORTEX_R4          = 5,
  CORTEX_R4F         = 6,
  CORTEX_M7          = 7,
  CORTEX_M23         = 8,
  CPU_RESERVED_9     = 9,
  CORTEX_M33         = 10,
  CPU_ARMV8MBL       = 11,
  CPU_ARMV8MML       = 12,
  CORTEX_M35P        = 13,
  CPU_ARMV81MML      = 14,
  CORTEX_M55         = 15,
  CPU_ARM9           = 100,
  CPU_ARM7
} CPUENUM;

/*
 *  RTOS Viewer Sub-Menu         /12.06.2012
 *  pio->Notify (UV_RTOS_SUB_MENU_IF, SUB_MENU *menu);
 */
typedef struct rtx_sub_menu  {
  char              * sym;   // Symbol Display Address (Sub-Menu: Memory Window / Watch Window)
  DWORD              nAdr;   // Symbol (Sub-Menu: Show Source-Code)
  DWORD           menu_ID;   // Memory Window Sub-Menu,      0: NONE, 1: Memory Window, 2: Watch Window, 3: Show Source Code
  DWORD           nWindow;   // Window Number
} RTOS_SUB_MENU;


/*
 * Register/Unregister a client ITM-node
 */
typedef struct itm_client  {
  int         itm_channel;                     // itm-channel number ( 0 ... 31 ) to register/unregister
  int  (*pClient) (int itm_channel,            // callback - channel number (0...31)
                   int   num_bytes,            // number of ITM-bytes (currently always 1)
                   BYTE     *pData,            // pointer to 'num_bytes' of ITM-data
                   void      *pRes);           // reserved for future, current unused

  DWORD           nRes[4];                     // reserved for future use, currently unused
} ITM_CLIENT;


/*
 * UV-internal: pio->SXX_ITM_CLIST := ITM_CHA[32]
 */
#define _NUM_ITM_CL  16          // up to '_NUM_ITM_CL' per channel

typedef struct itm_cha  {
  ITM_CLIENT         itm[_NUM_ITM_CL];         // up to '_NUM_ITM_CL' clients per ITM-channel
  BYTE               act[_NUM_ITM_CL];         // channel active, bit-0: 1=active
  DWORD            nRes1[_NUM_ITM_CL];         // reserved, currently unused
  DWORD            nRes2[_NUM_ITM_CL];         // reserved, currently unused
  DWORD            nRes3[_NUM_ITM_CL];         // reserved, currently unused
  DWORD            nRes4[_NUM_ITM_CL];         // reserved, currently unused
} ITM_CHA;






/*
 * Breakpoint-Command-Response Codes:
 *  pio->Notify (UV_BREAKPOINT_RSP, BKS *pB);
 */
typedef enum rspbp  {
  __BP_NEW     = 1,
  __BP_KILL    = 2,
  __BP_ENA     = 3,
  __BP_DIS     = 4,
  __BP_REDEF   = 5,        // Bp-create: existing Bp redefined
  __BP_NEWCANC = 6,        // Bp-create: redefine existing Bp cacelled
  __BP_BADADR  = 7,        // Bp-create: invalid address
  __BP_NOTSUPP = 8,        // Bp-create: break-type not accepted by target
  __BP_FAILED  = 9,        // Bp-command failed (nested/syntax/paramters etc.)
} RSPBP;

typedef struct bpstat  {
  RSPBP     status;
  BKS        *pBrk;
  DWORD   nRes[16];
} BPSTAT;


//---Example on how to use 'UV_RTAH_ERRMSG':
//  pio->Notify (UV_RTAH_ERRMSG, (void *) "Stinky little message...");
//


/*
 * UV_UPDATE_SELECTIVE:    /27.11.2006/
 */
#define VW_LOGIC_ANALYZER  0x00000001
#define VW_MEMORY          0x00000002
#define VW_REGISTERS       0x00000004
#define VW_PERF_ANALYZER   0x00000008
#define VW_COVERAGE        0x00000010
#define VW_RTXVIEW         0x00000020
#define VW_WATCHWINDOW     0x00000040
#define VW_PERIDIALOGS     0x00000080   // 15.12.2009, added
#define VW_AGDIDIALOGS     0x00000100   // -----------------
#define VW_CALLSTACK       0x00000200
#define VW_INISYMWIN       0x00010000   // init symwin
#define VW_COMPONENTS      0x00020000   // component views
#define VW_EVENTRECORDER   0x00040000   // event recorder view
#define VW_ULINKPLUS       0x00080000   // ulinkplus view
#define VW_SYSANALYZE      0x00100000   // [TdB: 09.05.2017] System Analyzer
#define VW_BREAK_PANE      0x01000000   // Breakpoint-Pane

/*
 * Example:
 *  pio->Notify (UV_UPDATE_SELECTIVE, VW_MEMORY | VW_REGISTERS);
 */


/*
 * UV_DBG_ACCESS_LEVEL: /02.07.2013/
 */
#define DA_NORMAL      0  // Normal, all target resourcess accessible
#define DA_ACCESSPORT  1  // AP accesses possible, memory accesses fail
#define DA_DEBUGPOWER  2  // DP accesses possible, debug system powered up, AP accesses fail or not ready
#define DA_DEBUGPORT   3  // DP accesses possible, debug system cannot be powered up
#define DA_INACTIVE    4  // Protocol error happened, debug port seems to be inactive
#define DA_DBGSERVER   5  // Debug Server Lost communication to debugger


/*
 * 26.6.2006, Demand-Load-Interface
 *  DEMANDLOAD  dml;
 *  dml.pB = pB;
 *  dml.nFlags = DM_LINES | DML_SYMS;
 *  nR = Cmd-SimFunc (SMF_DEMANDLOAD, &dml, NULL);
 *  if (nR == 0)  {              // demand-load not implemented
 *  }
 *  else  {
 *    if (dml.nErr != 0)  {      // failed
 *    }
 *  }
 */
#define DML_LINES     0x01       // Demand-Load: need line numbers
#define DML_SYMS      0x02       // Demand-Load: need symbols/types
#define DML_INCLOCK   0x04       // Demand-Load: increment CU lock-count
#define DML_DECLOCK   0x08       // Demand-Load: decrement CU lock-count
#define DML_ISDEMAND  0x10       // Demand-Load: is demand loading active ?

typedef struct DmLoad  {
  DWORD        nFlags;
  struct zbk      *pB;           // ZBK to load on demand
  int            nErr;           // 0:=Ok, Error otherwise.

  DWORD   bDemand : 1;           // answer to DML_ISDEMAND: 1:=yes 0=no
  DWORD      nRes[15];           // reserved for future use.
} DEMANDLOAD;
//------------------------------------


/*
 * 21.7.2006
 * Extended ToolTip, for ELF/DWARF debug_macinf resolution.
 */
typedef struct xttip  {
  char      szMd[512];           // name of current file, e.g. 'Measure.c'
  char         *szTip;           // input-expr / result-text receive buffer
  int           szLen;           // size of szTip[]

  DWORD         nLine;           // current line
  DWORD          nCol;           // current column
  DWORD       nRes[8];           // reserved
} XTTIP;
#define XTIP_SUPP      1000      // SimFunc-Return-code if xttip is supported



//#define   CMD_EXTCOVER     0x1000
// S8051/S166: Extended Coverage:
//      DWORD bR = ioc.SimFunc (CMD_EXTCOVER, char *"filename", DWORD nFlags);
//   bR:      1:=Ok, 0:=Error (file open/close/io)
//   nFlags:  1:=show jump-coverage details (like 'coverage details')



/*
 * /9.7.2007/
 * Resolve a symbol or qualified symbol to some 'sp'
 * Used to communicate SMF_RESOLVE_TO_SYM from/to UvSocket.
 */
typedef struct id_rtype  {
  char     szID[512];            // input id in ascii
                                 // Outputs:
  DWORD    nErr  :12;            // Error-Number if failed
  DWORD     bSym : 1;            // result is a 'sp'
  DWORD    bLine : 1;            // result is a line

  SYM            *sp;            // symbol or null
  TYP            *tp;            // type or null
  ZBK            *pB;            // bounding scope block
  DWORD         nAdr;            // result in case of line number
  DWORD      nRes[8];            // reserved
} ID_RTYPE;


/*
 * Structure for SMF_GETCALLSTACK:
 *  G_CSTACK  cstk;
 *  memset (&cstk, 0, sizeof (cstk));
 *  cstk.bFirst = 1;     // required to be 1 before first call
 *  cstk.bFull  = 1;     // want full path  
 *  n = pio->SimFunc (SMF_GETCALLSTACK, &cstk, NULL)
 *  if (n == 0) then failed
 *  else Ok, loop until n gets 0.
 */
typedef struct G_Cstack  {
  UINT64          nAdr;                // input address
  UINT64       nRetAdr;                // return-address (caller-address)
  UINT        nLine[2];                // Callee,Caller line number (if available)
  UINT64   nLineAdr[2];                // Callee,Caller line number address (if available)
  UINT       bFull : 1;                // want full path file (if available)
  UINT      bFirst : 1;                // set to 1 before first request
  UINT             :30;
  ZBK           *pB[2];                // callee,caller's bounding function
  char szFile[2][1024];                // Callee,Caller file (if available)

  BYTE      xRegs[256];                // opaque RgARM/RgARMCM
  BYTE       nRes[508];                // reserved, currently unused.
  UINT           topSP;                // SP for next level of unwinding.
} G_CSTACK;


/*
 * RVCT Stack-Unwinder
 */ 
#define MAX_UNWIND_SLOTS    512        // total functions + local symbols

typedef struct enlocs  {
  ZBK              *pB;                // bounding Function Scope
  SYM              *sp;                // local Symbol
  UINT      bScope : 1;                // bounding Scope, pB and nAdr are valid.
  UINT       bVReg : 1;                // Local-variable in Reg,   nVal:=Value
  UINT       bVStk : 1;                // Local-variable on Stack, nAdr:=Stack-Address (finalized)
  UINT      bInvis : 1;                // Local-variable is not in scope (invisible)
  UINT        bVal : 1;                // scalar loaded into 'nVal'
  UINT         b64 : 1;                // u64/d64 has value
  UINT             :26;                // unused

  DWORD           nVal;                // value
  DWORD           nAdr;                // finalized stack-address if 'bVStk:=1'
  DWORD        nRetAdr;                // callers-address
//DWORD        nRes[4];                // reserved, all zeros.
  DWORD        nRes[2];                // reserved, all zeros.
  union  {                             // value union
    UINT64         u64;
    double         d64;
    float          f32;
    DWORD          u32;
  };
} ENLOCS;


typedef struct edio_ctx  {                     // Editor/Indexer context
                                               // Input context from editor
  int            nEdLine;                      // line number in editor
  int             nEdCol;                      // column number in editor
  char          szEdFile [MAX_PATH + 32];      // editor file name

                                               // Output context via definition from clang-indexer
#define __X_REF 32
  int          nRefLines[__X_REF];             // first:=def (mostly !!!)
  int           nRefCols[__X_REF];
  const char  *aDefFiles[__X_REF];
//int           nDefLine;                      // definition line number
//int            nDefCol;                      // definition column number
//char         szDefFile [MAX_PATH + 32];      // file name containing defintion

  char             szSel [MAX_PATH + 32];      // word/selection text unter cursor

  XSYM               *sp;                      // Out: resolved symbol
} EDIO_CONTEXT;


/*
 * SimFunc - Command-Messages  /1.8.2005/
 */
#define SMF_CALC_REGEXP           0x0001   // SimFunc: calc expr. from RegTree change
#define SMF_CALC_MEMEXP           0x0002   // SimFunc: calc expr. from MemTab address
#define SMF_ENTER_MEMORY          0x0003   // SimFunc: enter memory
#define SMF_GET_ATTR_SEG          0x0004   // SimFunc: get attribute segment
#define SMF_EVAL_TOOLTIP          0x0005   // SimFunc: calc ToolTip expression
#define SMF_STOP_SAVESESS         0x0006   // SimFunc: Dbg about to terminate, save session info
#define SMF_DECODE_ERROR          0x0007   // SimFunc: decode error in to buffer
#define SMF_ESCAPE_GIVEN          0x0008   // SimFunc: Escape has been pressed
#define SMF_QUERY_EXIT            0x0009   // SimFunc: can we exit the debugger ?
#define SMF_TRY_STOP_DBG          0x000A   // SimFunc: try to stop the debugger
#define SMF_EXECCMD_NECHO         0x000B   // SimFunc: execute command without echo (e.g. ToolBox)
#define SMF_GET_INLINE_ADR        0x000C   // SimFunc: get 'AsmAdr'
#define SMF_DISASM_ADDR           0x000D   // SimFunc: DisAsm line at address
#define SMF_GET_PC                0x000E   // SimFunc: get current PC
#define SMF_CALC_MEMADR_H         0x000F   // SimFunc: calc upper limit for Memory-Window
#define SMF_EVAL_XTOOLTIP         0x0010   // SimFunc: extended - calc ToolTip expression
#define SMF_CRITICAL_REGION       0x0014   // SimFunc: crirical section On/Off
#define SMF_REG_AGSI_VEC          0x007F   // SimFunc: Register AGSI_VEC
#define SMF_MAP_FILE_TO_ZBK       0x0080   // SimFunc: derrive 'ZBK *' from path
#define SMF_EXTCOVER              0x1000   // SimFunc: extended Coverage (s8051/S166)
#define SMF_CREATECSTK            0x1001   // SimFunc: prepare callstack
#define SMF_PROFILETIMES          0x1002   // SimFunc: ARM-Profiling
#define SMF_DATACOVERAGE          0x1003   // SimFunc: Data-Coverage
#define SMF_QUERY_DBGSETTINGS     0x1004   // SimFunc: Query for Debug-Settings-Dialog
#define SMF_DO_DBGSETTINGS        0x1005   // SimFunc: Start Debug-Settings-Dialog
#define SMF_GETLAINFO             0x1100   // SimFunc: get LAINFO
#define SMF_LAKILLALL             0x1101   // SimFunc: kill LA * (from Dialog)
#define SMF_LADEFINE              0x1102   // SimFunc: LA define (from Dialog)
#define SMF_LAKILLSIG             0x1103   // SimFunc: LA kill one Signal
#define SMF_LARESETDATA           0x1104   // SimFunc: LA kill one Signal
#define SMF_CALC_EXPR             0x0800   // DTC/SimFunc: calc expr., used for DTC Event
#define SMF_CLEAR_BPBYADDR        0x0801   // DTC/SimFunc: clear Breakpoint by CodeAddress
#define SMF_MSG_TO_TARG           0x0802   // DTC/SimFunc: Transfer client Message to Target
#define SMF_MSG_FROM_TARG         0x0803   // DTC/SimFunc: Send Message from Target to Client
#define SMF_READWRITE_MEMORY      0x0804   // DTC/SimFunc: Read/Write Memory
#define SMF_CALCEXPR              0x0820   // SimFunc: LA calc expr (via struct Ecalc)
#define SMF_DEMANDLOAD            0x0821   // SimFunc: ensure loaded (Compile-Unit via ZBK *)
#define SMF_RESOLVE_TO_SYM        0x0822   // SimFunc: decode '\mod\sym' into 'sp'
#define SMF_GETCALLSTACK          0x0823   // SimFunc: get/decode next callstack frame
#define SMF_STACKUNWIND           0x0824   // SimFunc: unwind stack frames
#define SMF_CACHE_INVALIDATE      0x0825   // SimFunc: invalidate cache
#define SMF_CSPSTACK_REQ          0x0826   // SimFunc: CallStackPane request
#define SMF_CALC_VSYM             0x0827   // SimFunc: derived/base class address resolver

#define SMF_EXECCMD_ECHO          0x0840   // SimFunc: execute command with echo (e.g. tracepoints)
#define SMF_EXCTRC                0x0841   // SimFunc: [TdB: 20.11.2012] Support for Exception Trace Window
#define SMF_RECOVERY_CONF         0x0842   // SimFunc: Configure Debug Access Recovery
#define SMF_START_DBGEXIT         0x0843   // SimFunc: Announce debug exit to give DLLs a chance to react accordingly
                                           //          to certain interface calls
#define SMF_EVTRECORDER_CONF      0x0844   // SimFunc: Event Recorder Configuration
#define SMF_EVTRECORDER_DATA      0x0845   // SimFunc: Event Recorder Data

#define SMF_LOGWRITE              0x0846   // SimFunc: 1-> queue LogWrite output, 0-> no queue

#define SMF_REGTREE_UPDT_GRP      0x0847   // SimFunc: Update properties of struct RGROUP (see AGDI.H)
#define SMF_LOADAPP               0x0848   // SimFunc: Execute LOAD command with specific parameters
#define SMF_LOCATE_QUAD           0x0849   // SimFunc: locate symbol (sym, file, line, col)
#define SMF_SYM_TO_QUAD           0x084A   // SimFunc: resolve symbol to quad

#define SMF_SA_CHANNEL_INFOS      0x084B   // SimFunc: System Analyzer Channel Info Request
#define SMF_SA_SUBCHANNEL_INFOS   0x084C   // SimFunc: System Analyzer Sub-Channel Info Request
#define SMF_SA_CHANNEL_CONFIG     0x084D   // SimFunc: System Analyzer Channel Configuration Request
#define SMF_SA_CHANNEL_STATUS     0x084E   // SimFunc: System Analyzer Channel Status Request
#define SMF_SA_CHANNEL_SNAPSHOT   0x084F   // SimFunc: System Analyzer Channel Snapshot Request
#define SMF_SA_CHANNEL_DATA       0x0850   // SimFunc: System Analyzer Channel Data Request
#define SMF_SA_GROUP_INFOS        0x0851   // SimFunc: System Analyzer Group Info Request

//#define SMF_SA_CH_RTOSV5_CONFIG   0x0852   // deprecated! SimFunc: configure a RTX-RTX-V5 channel
//#define SMF_SA_CH_RTOSV5_DATA     0x0853   // deprecated! SimFunc: data entry for RTX-RTX-V5 channel
#define SMF_EVENTREC_LOG          0x0854   // SimFunc: event-recorder logging
#define SMF_GETPOWER_INFO         0x0855   // SimFunc: get power statistics snapshot (PWSTAT)
#define SMF_CALCPOWER_DATA        0x0856   // SimFunc: calc power data

#define SMF_SA_ADD_GROUP          0x0857   // SimFunc: configure a external channel
#define SMF_SA_ADD_CHANNEL        0x0858   // SimFunc: configure a external channel
#define SMF_SA_ADD_SUBCHANNEL     0x0859   // SimFunc: configure a external channel
#define SMF_SA_ADD_DATA           0x0860   // SimFunc: data entry for external channel
#define SMF_SA_SET_CHANNELINFO    0x0861   // SimFunc: configure a external channel

#define SMF_SA_STATISTIC_INFOS    0x0870   // SimFunc: System Analyzer Statistics Info Request
#define SMF_DO_PERIODIC           0x0880   // FreeRTOS periodic register capture

#define SMF_STL_PROCESS           0x0890   // Stl-address collection processing   // 20.4.2020/

                                           //---special SMF for Infineon-251:     // 2.8.2006, added
#define SMF_GET_LRUCOUNTER        0x2000   // SimFunc: Get MMU LRU counter
#define SMF_GET_CACHE             0x2001   // SimFunc: Get Address of cache arrays
#define SMF_GET_MMU               0x2002   // SimFunc: Get Address of MMU arrays
#define SMF_GET_CONFIG            0x2003   // SimFunc: Get Address of Configuration table
#define SMF_READMEM_DIRECT        0x2004   // SimFunc: Read directly from memory with EDC
#define SMF_WRITEMEM_DIRECT       0x2005   // SimFunc: Write directly into memory with EDC

#define SMF_READ_V8MSECEXT        0x2006   // SimFunc: Read V8M-Security-Extension Registers

#define SMF_ALLREG                0x2007   // SimFunc: Read/Write Registers
#define SMF_REGACC_READ           0x2008   // SimFunc: Read specific register
#define SMF_REGACC_WRITE          0x2009   // SimFunc: Write specific register
#define SMF_HALFTOFLOAT           0x200a   // SimFunc: Convert float16 (short) to float
#define SMF_FLOATTOHALF           0x200b   // SimFunc: Convert float to float16 (short)
//---


#if 0
#define CMD_DATACOVERAGE      0x1003   // SimFunc: Data-Coverage
#define CMD_QUERY_DBGSETTINGS 0x1004   // SimFunc: Query for Debug-Settings-Dialog
#define CMD_DO_DBGSETTINGS    0x1005   // SimFunc: Start Debug-Settings-Dialog
#define SMN_CALCEXPR          0x0820   // SimFunc/Notify: calc expr (via struct Ecalc)
#define CMD_CREATECSTK        0x1001   // SimFunc: prepare callstack
#define CMD_PROFILETIMES      0x1002   // SimFunc: ARM-Profiling
#define CMD_GETLAINFO         0x1100   // SimFunc: get LAINFO
#define CMD_LAKILLALL         0x1101   // SimFunc: kill LA * (from Dialog)
#define CMD_LADEFINE          0x1102   // SimFunc: LA define (from Dialog)
#endif




/*
 * LA-Definitions
 */

typedef struct  {
  INT64      cycles;         // states value of record
  DWORD         nPC;         // PC-value at time of write
  DWORD  SigNo : 16;         // LASIG-Number (1...255)
  DWORD  tMark :  1;         // t.eTime, t.nClock is valid (otherwise u.v.xxx is valid)
  union  {
    struct  {
      double  eTime;         // elapsed time at change
      DWORD  nClock;         // new clock value
    } t;
    union v       v;         // v.c/uc/i16/u16/l/ul/i64/u64/f/d
  } u;
} LAREC;


#define PLOT_T_ANALOG  1
#define PLOT_T_BOOL    2
#define PLOT_T_DIGITAL 4

#define  SIGTYP_NORMAL     0
#define  SIGTYP_VOLTAGE    1
#define  SIGTYP_CURRENT    2
#define  SIGTYP_POWER      3


#pragma pack(1)
typedef struct {
  DWORD        nAdr;         // memory address
  WATCH         law;         // Write-Access-Watch

  DWORD   SigNo : 8;         // internal number for 'LA DIR'
  DWORD   winst : 1;         // is write-watch 'law' installed
  DWORD   valid : 1;         // the 'LASIG' is valid
  DWORD   isVTR : 1;         // LASIG refers to a VTR, not memory

  union v         v;         // last captured value
  LAREC       first;         // first (=start) value
  LAREC        last;         // last  (=end)   value

//---Display-Info (per Signal)-
  COLORREF      fgc;         // Plot-Color
  double       vmin;         // Min-value (for analog Plot)
  double       vmax;         // Max-value (for analog Plot)
  DWORD    PlotType;         // PLOT_T_ANALOG, ...
  int         PixCy;         // height in pixels
  UINT64   ZoomFact;   

  char   szDispName[128];       // Signal's display name

  DWORD     isActive    : 1;    // 1:Signal is recorded
  DWORD      bMinDec    : 1;    // Min-value is decimal int/long  // Freiwald 16.04.2004							
  DWORD      bMaxDec    : 1;    // Max-value is decimal int/long  // Freiwald 16.04.2004							
  DWORD      bValHex    : 1;    // Display-value is hexadecimal   // Freiwald 31.03.2005							
  DWORD      bViaIni    : 1;    // Installed via .ini file
  DWORD bMinMaxChanged  : 1;    // Min or Max updated by dialog   [TdB: 03.08.2010]
  DWORD bAdaptiveMinMax : 1;    // Adaptive Min / Max
  DWORD      deleted    : 1;    // if sig is deleted this gets set  [TdB: 05.08.2012]
  DWORD        unus     :24;    // free

  int    uvDispNum;          // Signal Number in Display   [TdB: 05.07.2012]
  double heightFactor;
  int    zoomFactIndex;
  DWORD     nRes[4];         // was [8]
  DWORD         dPc;         // PC at time of LA-create

//---internal info-------------
  SYM           *sp;         // Symbol
  EXP           *ep;
  VTR           *vt;         // VtReg-LA (isVTR := 1)
  TYP        *AccTp;         // memory access type
  TYP           *tp;         // result-type
  DWORD        size;
  DWORD    bitf : 1;         // bit-field or bit-type
  DWORD     wid : 8;         // field width
  DWORD     ofs : 8;         // field offset within scalar
  DWORD    vfld : 1;         // a value field 'x & const' (ofs=field offs)
  DWORD   unus2 : 2;
  DWORD  sigTyp : 4;         // Signal Type for power measurements etc (normal, V, A, W, ...)
  DWORD significantNumbers : 4; // significant numbers (i.e.: 4 : 1.234)
  DWORD   unus3 : 4;
  DWORD       vmask;         // value mask '(x & vmask-const)'

  char   szExp[128];         // the expr.-text: LA CREATE <expr>
  void       *pCSig;         // pointer to CLASignal
  DWORD    nRes2[7];         // was nRes2[8]
                             // S8051: 'nRes2[0]' used for BitPos
                             // S8051: 'nRes2[1]' for MultiWatches (as 'WATCH[]*')
//----------------------------
} LASIG;
#pragma pack()


/*
 * CMD_GETLAINFO fill out this structure:
 */
typedef struct   {
  LASIG       *pSig;         // Array of LA-Sigs, [0]:=reserved, 1...255
  LAREC       *pRec;         // LA-Records
  int         LaEnd;         // 0...MAXLAR
  int         LaIdx;         // 0...MAXLAR-1
  int       nMaxSig;         // max. LA-Signals (pSig[])
  int       nMaxRec;         // max. LA-Records (pRec[])  (power of 2)

  int            nR;
  int         rCode;
  int      zoomFact;        // restore Zoom Factor on next debug entry
  DWORD     Res[15];        // [16] reserved
} LAINFO;


//---12.11.2003:----------------------
struct Ecalc  {
  LPCTSTR   pStr;     // Expression-Text
  union v      v;     // Result
  TYP        *tp;     // Result-Type
  int       nErr;     // 1:=Error, 0:=Ok
};
//------------------------------------



/*
 * Limits for LA
 */
#define MAXSIG  256          // max LA-Signals: Slot 0=reserved, 1-255





/*
 * ARM-Profiling
 *
 *  PROFINFO  info;
 *
 *  memset (&info, 0, sizeof (info));
 *  info.nCmd = PROF_CALCCYCLES;
 *  nR = pio->SimFunc (CMD_PROFILETIMES, &info);
 *  if (nR != 0)  {    // failed
 *    ...
 *  }
 */

#define PROF_CALCCYCLES    1   // calc cycles & time over address range
#define PROF_TOTALCYCLES   2   // get total runtime
#define PROF_EXECCOUNT     3   // get number of times executed for address 'nStart'

#pragma pack(1)                // 12.09.2016: Added missing #pragma pack directive, nRes extended from 8 to 9 elements to
                               // reflect previous implicit insertion of a DWORD by the compiler for aligned UINT64 cycles field.
typedef struct proftm  {
  int       nCmd;      // Command

  DWORD   nStart;      // Start-Address
  DWORD     nEnd;      // End-Address   (Adr >= nStart && Adr < nEnd)

  DWORD  nRes[9];      // reserved

  UINT64  cycles;      // Result: cycles
  double    time;      //         time in Sec
  UINT64   count;      //         number of executions
} PROFINFO;
#pragma pack()


/* 
 * Extended Profiling Mode (ARM)
 */
#define EPROF_OFF   0x00   // Execution Profiling Mode := Off
#define EPROF_TIME  0x01   // Execution Profiling Mode := Time
#define EPROF_CALLS 0x02   // Execution Profiling Mode := Calls



/*
 * Data-Coverage (currently only for ARM-Targets):
 *
 *  DATACOVERAGE  info;
 *
 *  memset (&info, 0, sizeof (info));
 *  info.nCmd    = COV_GETCOVERAGE;
 *  info.nStart  = Start_Address;
 *  info.nLength = number of bytes to look for coverage info
 *  nR = pio->SimFunc (CMD_DATACOVERAGE, &info, NULL);
 *  if (nR == 0)  {     // coverage not supported
 *    ...
 *  }
 *  else  {             // process coverage info
 *    switch (info.nStatus)  {
 *      case DC_READ_BEFORE_WRITE:
 *        ...
 *      case DC_WRITE_NO_READ:
 *        ...
 *      case DC_WRITE_THEN_READ:
 *        ...
 *      case DC_NOT_PRESENT:
 *        ...
 *    }
 *  }
 */

#define   COV_GETCOVERAGE       1   // get Coverage-Info Command

#define   DC_NOT_PRESENT        0   // No Coverage Support.
#define   DC_READ_BEFORE_WRITE  1   // READ before WRITE
#define   DC_WRITE_NO_READ      2   // WRITE but never READ
#define   DC_WRITE_THEN_READ    3   // WRITE then READ
#define   DC_NOT_MAPPED         4   // unmapped memory / memory not present


typedef struct datacoverage  {
  int          nCmd;      // Command (COV_GETCOVERAGE)

  DWORD      nStart;      // Start-Address
  DWORD     nLength;      // number of bytes to check
  DWORD     nStatus;      // return-status (DC_???)

  DWORD     nRes[8];      // reserved for Baldisch...
} DATACOVERAGE;



/*
 * Info of last breakpoint hit
 *   nAcc: access-type of breakpoint
 *           0x01:=EXEC,   0x02:=READ,  0x04:=WRITE, 0x08:=CONDitional
 *           0x10:=ESCape, 0x20:=AccViolation, ...
 */
#define HIT_NOREASON 0x0000      // Undefined reason
#define HIT_EXEC     0x0001      // Exec
#define HIT_READ     0x0002      // Read
#define HIT_WRITE    0x0004      // Write
#define HIT_COND     0x0008      // Conditional
#define HIT_ESC      0x0010      // ESCape
#define HIT_VIOLA    0x0020      // Access-Violation
#define HIT_TIMEOVER 0x0040      // Interval-time is over
#define HIT_UNDEFINS 0x0080      // Undefined Instruction
#define HIT_PABT     0x0100      // Prefetch abort
#define HIT_DABT     0x0200      // Data abort
#define HIT_NONALIGN 0x0400      // Non-aligned access

typedef struct hitbp  {
  DWORD          nAdr;         // Address of breakpoint / memory access
  WORD16         nAcc;         // AccType
  WORD16       nBpNum;         // Breakpoint-Number (0xFFFF:=undefined)
  DWORD           nPC;         // Address of PC on BP/violation
  UINT      nTickMark;         // Breakpoint creation tickmark
  DWORD       nRes[6];         // reserved
} HITBP;




//----------------------------------------------------------------------
#define MAX_TASKS       256      // max number of RTX-tasks

/*
 * RTX-OS-information            // 12.1.2012
 */
typedef struct axarm  {          // RTX-Kernel values
  DWORD          nTasks;         // Number of ARTX Kernel-Tasks
  int        MaxTaskRun;         // Maximal number of tasks running at the same time
  int         StackSize;         // Stack size
  int          TimerNum;         // Timer number
  int           PrivCnt;         // Number of tasks with user-provided stack
  int      UserTimCount;         // User Timer Count
  int        RRobinTout;         // Round Robin Timeout
  DWORD       ClockRate;         // Clock rate
  DWORD        CPUClock;         // CPU Clock
  DWORD      os_treload;         // Hw-Timer reload for 1 tick
  int        os_tval_sr;         // Hw-tick right shift in bits in task switch word from target
  int        os_time_sl;         // OS timer left shift in bits in task switch word from target
  double   os_time_loop;         // Time at which the timers loop
  double        os_tick;         // Tick time

  int     ActiveTaskNum;         // SysRes: currently active tasks
  int     ActiveUserTim;         // SysRes: currently active user timers
  int           tvSlots;         // for VartxArm/RtxSys compatibility  /12.1.2012/

  DWORD   LoadTaskPg : 1;
  DWORD   LoadFilePg : 1;
  DWORD      CortexM : 1;        // Cortex-M Flag
  DWORD     StkCheck : 1;        // Check stack overflow 
  DWORD RRobinEnable : 1;        // Round Robin Enabled
  DWORD     StkWmark : 1;        // Stack-pattern watermark enabled
} AXARM;


/*
 * RTX-task data sample          // 12.1.2012
 */
typedef struct taskdt  {         // Data for one task
  DWORD         tID : 8;         // task-id ( is_idle_demon := 255 )
  DWORD       tPrio : 8;         // task priority
  DWORD      tState : 5;         // task state (0=inactive, 1=ready, 2=running,
                                 //             3=wait_dly, 4=wait_itv, 5=wait_or,
                                 //             6=wait_and, 7=wait_sem, 8=wait_mbx,
                                 //             9=wait_mut, 10=<invalid> 
  DWORD      tStVal : 8;         // Task-state value
  DWORD    tNoStack : 1;         // hide stack used output
  DWORD      tV5Rtx : 1;         // 1:=RTX-RTOS V5
  DWORD             : 1;         // currenty unused

  int            tDelay;         // Task-delay value
  DWORD       tEventVal;         // Event-Value
  DWORD      tEventMask;         // Event-Mask
  int         tStackUse;         // Task-Stack-uses: 0...100, -1:=overflow, -2=underflow
  DWORD            tLoc;         // task-start address

  DWORD        tcbStack;         // TCB_STACK - tcbStack+0       := lower-bound of stack range
                                 //             tcbStack+tcbStSz := upper bound of stack
  DWORD         tcbStSz;         // TCB_PRIV_STACK or default StackSize 'mOs.StackSize'
  DWORD       tcbTskStk;         // TCB_TSK_STACK - current SP

  void           *tItem;
  SYM               *sp;         // Task-symbol
  char       tName[128];         // name of task

//-------------------------------// 26.12.2011
  DWORD       bDraw : 1;         // used by CallStackPane in RTX-Mode
  DWORD       bSeen : 1;         // used by CallStackPane in RTX-Mode
//-------------------------------
  DWORD     bVFPAct : 1;         // CL 16.01.2012 '1' if FP Context was used in the Task, else '0'.
  DWORD     saveFSz :10;         // save-frame size (max. 1023)
  DWORD      maxStk : 7;         // max. perc stack used (0...100)

  DWORD             :12;         // currently unused
  //-----------------------------//

  DWORD        nStkSize;         // task-stack size
  DWORD        nStkUsed;         // bytes used from task-stack
  DWORD         nRes[9];         // [10]->[9] reserved for future use.
  DWORD           V5TID;         // RTX-RTOS V5 full 32-bit TID
} TASKDT;



/*
 * Centralized RTX-OS + Tasks-samples
 */
typedef struct rtx_context  {
  DWORD      bLoaded : 1;                  // RTX-OS ymbols have been loaded
  DWORD      bRtxApp : 1;                  // a RTX-application

  int            tvSlots;                  // actual number of task samples
  TASKDT           tasks [MAX_TASKS];

  AXARM              mOs;                  // RTX-OS values
//DWORD          rr[256];
} RTX_CONTEXT;

//----------------------------------------------------------------------



/*
 * IRQ-Table exposition
 */
typedef struct irqslot  {
  char            *pName;      // <name>
  char            *pDesc;      // <i>
  DWORD       bValid : 1;      // 
  DWORD              :31;
  DWORD          nRes[6];      // [6] reserved
} IRQSLOT;

typedef struct irqinfo  {
  DWORD       bValid : 1;      // IrqTab present

  DWORD        bNVPB : 1;      // 1:=NvicPrioBits are present
  DWORD      nvPrioB : 8;
  DWORD              :22;      // currently unused

  int              vHigh;      // highest vector number
  IRQSLOT       arr[256];      // vector array
  const IRQSLOT  *(*GetIrq     )(unsigned int index);
  const char     *(*GetIrqName )(unsigned int index);
  const char     *(*GetIrqDescr)(unsigned int index);  
  IRQSLOT       *arrHigh;      // vector array 256 .. vHigh-1, NULL if unused
  DWORD         nRes[31];      // [31] reserved for future use
} IRQINFO;



/*
 * [TdB: 20.11.2012] Exception Trace & Event Counters Window in UV4
 */

#define EXCTRC_NUM 512            // Number of Exceptions
#define EVTCNT_NUM   6            // Number of max Events

typedef enum {                    // Commands for Exception Trace & Event Counters update & reset
  EXCTRC_NONE         =  0,
  EXCTRC_INIT         =  1,
  EXCTRC_UPDATE       =  2,
  EXCTRC_DELROW       =  3,
  EXCTRC_RESET        =  4,
  EXCTRC_GETIRQLINES  =  5,
  EXCTRC_GET_CFG      =  6,
  EXCTRC_SET_CFG      =  7,
  EVTCNT_INIT         =  8,
  EVTCNT_UPDATE       =  9,
  EVTCNT_DELROW       = 10,
  EVTCNT_RESET        = 11,
} excTrcCmd_e;

/* Exception Trace Data */
typedef struct {
  DWORD    count;                 // Exception Counter
  double  tenter;                 // Exception Enter Time
  double   texit;                 // Exception Exit Time
  double     tin;                 // Time in Exception
  double  ttotal;                 // Total Time in Exception
  double  tinmin;                 // Min Time in Exception
  double  tinmax;                 // Max Time in Exception
  double toutmin;                 // Min Time out of Exception
  double toutmax;                 // Max Time out of Exception
  double  tfirst;                 // First Time in Exception
  DWORD  nRes[6];                 // [6] reserved for future use
} EXCTRC_ENTRY;

typedef enum {
  EVTCNT_CPI      = 0,
  EVTCNT_Exc      = 1,
  EVTCNT_Sleep    = 2,
  EVTCNT_LSU      = 3,
  EVTCNT_Fold     = 4
} EVTCNT_TP;

typedef struct {
  EVTCNT_TP    type;                      // Field that should be updated
  INT64       value;                      // value
  DWORD     nRes[2];                      // [2] reserved for future use
} EVTCNT_ENTRY;


typedef struct {                          // struct for Data exchange
  DWORD           bUpdatedExcTrc :  1;    // Exception Trace Data has changed
  DWORD           bUpdatedEvtCnt :  1;    // Event Counter Data has changed
  DWORD         bTraceConfigValid : 1;    // Trace Config has been read from target
  DWORD                          : 29;

  excTrcCmd_e               excTrcCmd;    // Command (Update, Reset)
  unsigned int           numOfLastIrq;    // not necessary to update whole IRQ table
  unsigned int            extIRQLines;    // number of lines for External Interrupts
  unsigned int                    row;    // IRQ Table row (i.e. for DELETE cmd)
  unsigned int                    col;    // IRQ Table row (unused, for future extension)
  EXCTRC_ENTRY          *TraceExcDisp;    // Trace Exception Data to be displayed
  EVTCNT_ENTRY        *EventCountDisp;    // Event Counters Data to be displayed

  union {
    DWORD                     TraceConf;
    struct {
      DWORD                 bTraceEn  : 1;    // Is Trace enabled ?
      DWORD                 bEnExcTrc : 1;    // Enable Exception Trace or Event Counters
      DWORD                 bEnCPI    : 1;    // Enable CPI Count Event 
      DWORD                 bEnEXC    : 1;    // Enable Interrupt Overhead Event 
      DWORD                 bEnSLEEP  : 1;    // Enable Sleep Count Event 
      DWORD                 bEnLSU    : 1;    // Enable LSU Count Event 
      DWORD                 bEnFOLD   : 1;    // Enable Fold Count Event 
      DWORD                 bEnTS     : 1;    // Enable Timestamps
      DWORD                          : 24;    // [24] reserved for future use
    };
  };
DWORD                      nRes[31];      // [32] reserved for future use

} excTrcData_t;
 
/*
 *  Commands for SMF_RECOVERY_CONF - manually triggered recovery of debug access
 *
 */

#define RECOVERY_CONF_INIT  0x0000    // Initialize Debug Recovery Support in core DLL (if supported)
#define RECOVERY_CONF_CMD   0x0001    // Send a Debug Recovery configuration command


/*
 *  Configuration commands for SMF_RECOVERY_CONF - manually triggered recovery of debug access
 *    Use as *p2 in SimFunc() call
 *
 */

typedef struct recovery_conf {
  DWORD underReset :  1;   // Try to recover under HW reset
  DWORD exitDebug  :  1;   // Debug exit requested from Recovery Dialog
  DWORD            : 30;   // Reserved
  DWORD        nRes[16];   // Reserved
} RECOVERY_CONF;


/*
 *  Commands for SMF_EVTRECORDER_CONF - configure event recorder
 */
#define EVTRECORDER_REGISTER     0  // Register Event Recorder Description
#define EVTRECORDER_UNREGISTER   1  // UnRegister Event Recorder Description (by ID)
#define EVTRECORDER_SET_CFG      2  // Set/Update Event Recorder Configuration
#define EVTRECORDER_GET_CFG      3  // Get Current Event Recorder Configuration

 // Event Recorder Timestamp Source Types
#define EVTREC_TS_DBG_CYC       0  // Debug Cycle Counter
#define EVTREC_TS_SYSTICK       1  // System Tick Counter
#define EVTREC_TS_CMSIS_RTOS    2  // CMSIS RTOS Timer
#define EVTREC_TS_USER_NORM_RST 3  // User Defined Timer (Normal Reset)
#define EVTREC_TS_USER_PO_RST   4  // User Defined Timer (Power-on Reset)

#pragma pack(1)
typedef struct evtrecorder_conf_cmd {
  DWORD                  cmd;  // [in]  : SMF_EVTRECORDER_CONF sub-command
  DWORD           recorderID;  // [out] : Recorder Configuration ID (EVTRECORDER_REGISTER)
                               // [in]  : Recorder Configuration ID (EVTRECORDER_UNREGISTER)
  BYTE          protocolType;  // [in]  : Protocol Type: 1 - DAP
  WORD       protocolVersion;  // [in]  : Protocol Version: [15..8]=major, [7..0]=minor
  DWORD          recordCount;  // [in]  : Number of Records in Event Buffer
  DWORD       eventBufferAdr;  // [in]  : Event Buffer Address
  DWORD       eventFilterAdr;  // [in]  : Event Filter Address
  DWORD       eventStatusAdr;  // [in]  : Event Status Address
  BYTE              tsSrouce;  // [in]  : Timestamp Source Type (EVTREC_TS_xxx)
  BYTE           enabled : 1;  // [in]  : Event Recorder Capturing is enabled
  BYTE           res     : 7;  // reserved for future use  
  BYTE          reserved8[2];  // reserved for future use  

  DWORD        nReserved[31];  // [32] - reserved for future use
  BYTE              *pTrkMap;  // this one taken from former 'nReserved[32]'
} EVTRECORDER_CONF_CMD;
#pragma pack()


/*
 *  Commands for SMF_EVTRECORDER_DATA - event buffer data access
 */
#define EVTRECORDER_CLEAR        0  // Clear event recorder data
#define EVTRECORDER_COUNT        1  // Get number of record items
#define EVTRECORDER_ITEMS        2  // Get record items
#define EVTRECORDER_STATUS       3  // Get event recorder status
#define EVTRECORDER_DIFF_ITEMS   4  // Get record items no already fetched.


#pragma pack(1)
typedef struct evtrecorder_item {
  unsigned long long    timestamp; // Timestamp
  unsigned long         restartID; // Restart ID
  unsigned char               idx; // Index if multiple records for one event
  unsigned char               ctx; // Event Context (to ID multiple events with same timestamp)
  unsigned char            length; // Byte length of item
  unsigned char        recorderID; // Recorder ID
  WORD                         id; // Event ID
  DWORD                      val1; // Payload Value 1
  DWORD                      val2; // Payload Value 2
  unsigned char     overflowL : 1; // First record after recovery from overflow, LowBit
  unsigned char         reset : 1; // First record after a reset
  unsigned char       validID : 1; // id field carries a valid value (e.g. if record with ID not read yet)
  unsigned char          last : 1; // Last Record in Event
  unsigned char      complete : 1; // Record is complete (all indexes between 0 and last item)
  unsigned char     overflowH : 1; // First record after recovery from overflow, HighBit
  unsigned char           irq : 1; // IRQ Flag
  unsigned char        resume : 1; // Resume
} EVTRECORDER_ITEM;



typedef struct evtrecorder_data_cmd {
  DWORD                  cmd;  // [in]  : SMF_EVTRECORDER_DATA sub-command
  DWORD               offset;  // [in]  : Offset of items to return                 (EVTRECORDER_ITEMS)
                               // [out] : Offset of first returned item             (EVTRECORDER_COUNT|EVTRECORDER_ITEMS)                           
  DWORD                  num;  // [out] : Number of items in buffer as per params   (EVTRECORDER_COUNT)
                               // [in]  : Max number of items to return             (EVTRECORDER_ITEMS)
                               // [out] : Number of returned items                  (EVTRECORDER_ITEMS)
  EVTRECORDER_ITEM   * items;  // [out] : Data buffer, allocated by caller          (EVTRECORDER_ITEMS)

  DWORD               status;  // Current Event Recorder status                     (all command codes)
  DWORD               tsFreq;  // Timestamp Frequency in Hz
  UINT64         captRecords;  // Captured Event Records
  UINT64         missRecords;  // Missed Event Records
  unsigned char   recorderID;  // Event Recorder ID
  unsigned char     nRes8[3];  // reserved for future use
  DWORD        nReserved[25];  // reserved for future use
} EVTRECORDER_DATA_CMD;


typedef struct svcdStateDrawInfo  {
  union  {
    UINT32                  all;
    struct  {
      UINT32           plot : 3;           // plot:  off(0), line(1), box(2) (default:=off)
      UINT32          color : 3;           // color: blue(0), red(1), green(21), black(3) (default:=blue)
      UINT32           bold : 1;           // bold:  0=normal, 1:=thick line, 2:=dark box (default:=0)
      UINT32          track : 2;           // track: 1=start tracking RTX-RTOS2 thread
                                           //        2=stop tracking RTX-RTOS2 thread
                                           //        3=Reset
                                           //        0=not a tracked resource (default)
      UINT32        evrInit : 1;           // Event Recorder initialized
      UINT32        dormant : 1;           // dormant="1"/ "0" (default := 1)
      UINT32         unique : 1;           // unique="1" / "0"
      UINT32           ssel : 1;           // ssel="1" / "0"
      UINT32       reserved :19;           // 
    };
  };
  UINT32           reserved2[4];
} SCVD_STATEDRAWINFO;



typedef struct SA_SvcdData  {
  union  {
    UINT32                  all;
    struct  {
      UINT32        stateId :  8;          // ID of state
      UINT32        evrInit :  1;          // Event Recorder initialized
      UINT32         unique :  1;          // unique="1" / "0"
      UINT32       reserved :  6;          // 
      UINT32           evId : 16;          // Event Id to decode event
    };
  };
} SA_SCVD_STATEDATA;



#pragma pack()


// EVTRECORDER_DATA::status Codes
#define EVTREC_STAT_INACTIVE       0x00  // Event Recorder Capture Inactive
#define EVTREC_STAT_ACTIVE         0x01  // Event Recorder Capture Active
#define EVTREC_STAT_TRG_OVERFLOW   0x02  // Target Application Buffer Overflow
#define EVTREC_STAT_BUF_OVERFLOW   0x04  // Host PC Buffer(FrameBuffer, EventRecorderBuffer) Overflow
#define EVTREC_STAT_STICKY_ERROR   0x08  // One or more errors happened, clears on start of run/step/clear
#define EVTREC_STAT_UNINITIALIZED  0x10  // Event Recorder not initialized yet in the target side
#define EVTREC_STAT_TS_READ_ERROR  0x20  // Timestamp Address is invalid and cannot be read
#define EVTREC_STAT_CONFIG_ERROR   0x40  // Unsupported Recorder Configuration
#define EVTREC_STAT_PROTOCOL_NOSUP 0x80  // Unsupported Event Recorder Protocol
#define EVTREC_STAT_STRM_OVERFLOW  0x100 // Host PC Event Record Stream Overflow


// SMF_EVTRECORDER_CONF and SMF_EVTRECORDER_DATA Error Codes
#define EVTREC_ERR_NOSUPP          0   // Unsupported
#define EVTREC_ERR_OK              1   // Success
#define EVTREC_ERR_BADPARAM        2   // Invalid parameter
#define EVTREC_ERR_CMD             3   // Unsupported sub-command
#define EVTREC_ERR_INTERNAL        4   // Internal Event Recorder Error
#define EVTREC_ERR_PROTOCOL        5   // Unsupported Protocol Version
#define EVTREC_ERR_RECCOUNT        6   // Unsupported Event Count
#define EVTREC_ERR_START           7   // Cannot Start Event Recorder Capturing
#define EVTREC_ERR_STOP            8   // Cannot Stop Event Recorder Capturing
#define EVTREC_ERR_READ_STATUS     9   // Cannot read Event Recorder Target Status
#define EVTREC_ERR_READ_EVENT      10  // Cannot read Event Recorder Buffer
#define EVTREC_ERR_INVALID         11  // Invalid Event Recorder
#define EVTREC_ERR_INACTIVE        12  // Event Recorder Capturing disabled
#define EVTREC_ERR_TARGET_DATA     13  // Event Recorder Target Data Inconsistent
#define EVTREC_ERR_OVERFLOW        14  // Event Recorder Event Count Overflow


/*
*  Commands for SMF_LOADAPP - LOAD command with specific parameters
*/
#define LOADAPP_FILE       0   // Load application file

/*
*  Command structure for SMF_LOADAPP - Execute LOAD command with specific parameters
*/
typedef struct loadappex {
  DWORD              cmd;      // Subcommand
  const char*       file;      // File path ('\0'-terminated)
  DWORD incremental :  1;      // Do incremental load
  DWORD nocode      :  1;      // Do not download code to target
  DWORD noreset     :  1;      // Do not execute reset (only support on few targets)
  DWORD autoreload  :  1;      // Automatic reload after subsequent non-incremental load command
  DWORD resBits     : 28;      // Reserved bits for future use
  DWORD          res[32];      // Reserved for future use
} LOADAPP;

// SMF_LOADAPP Error Codes
#define LOADAPP_ERR_NOSUPP          0  // Unsupported
#define LOADAPP_ERR_OK              1  // Success
#define LOADAPP_ERR_BADPARAM        2  // Invalid parameter
#define LOADAPP_ERR_CMD             3  // Unsupported sub-command
#define LOADAPP_ERR_INTERNAL        4  // Internal Error during load operation
#define LOADAPP_ERR_NOTFOUND        5  // File to load not found
#define LOADAPP_ERR_LOADFAIL        6  // File load failed (e.g. file corrupt)


/*
 * RTE-Flash Algoritm Files exposition
 */
typedef struct rteflash  {
  int             nItems;      // number of valid RteFlash[] items
  char   *AlgoFiles[128];      // 0...nItems Flash-Algo files
  DWORD         nRes[32];      // reserved for future use
} RTEFLASH;


/*
 * RTE-Search pathes for component library source files
 */
typedef struct rte_src  {
  int             nItems;      // number of valid search pathes
  char      *SetSrc[512];      // 0...nItems search path definitions
  DWORD         nRes[32];      // reserved for future use
} RTE_SRC;


/*
 * Recent call return value context
 */
typedef struct RecCall  {
  DWORD             nAdr;                     // call is @nAdr
  XSYM              *spF;                     // callee function symbol
  union v           rval;                     // callee return value
  char             szFnc[MAX_PATH * 2 + 32];  // decoded function name
  char             szVal[MAX_PATH + 32];      // decoded ascii return value
  DWORD             nRes[32];                 // reserved, currently unused.
} RECCALL;


/*
 * Remote Tracepoint-create    /17.5.2017/
 *  Version := 1
 * Instance is located in SarmCM3, can be accessed
 * via pio->SXX_BRKTRC_PA in UV4.
 */
typedef struct InfoCmdMsg  {   // required for BreakTracePane  /17.5.2017/
  DWORD         nVers : 8;     // Version-number: currently 1
  DWORD         nUnus :24;

  DWORD        nError;         // Error-code
  int            nPos;         // error position within szCmd[] or -1
  char          szErr [512];   // captured error message
  char          szCmd [512];   // currently unused

  DWORD          nRes [64];    // reserved - currently unused
//---nVers:=1 ends here.
} INFOCMDMSG;



/*
 * Bom-exported FilePathExpand() via pio->SXX_PATHEXPAND
 */
#define FPEXP_NORMAL   1       // perform regular expansion
typedef int  (*TpPATHEXP) (int nCode, const char *pIn, char *pOut);
extern int FilePathExpand (int nCode, const char *pIn, char *pOut);




#define THSAFELOC  UsrByte[0]  // DLL puts a 'TSAFEMARK' into 'ioc.TSAFELOC'
#define TSAFEMARK  0x88        // DLL is 'thread-safe' marker


struct bom  {
  DWORD          nSize;
  WORD16  bInitialized;
  WORD16          iMCS;    // ToolSet Definitions 0=51, 1=251, 2=166, 3=Fox
  WORD16        iMinor;    // SubCode: iMCS166(1):=MAC167
  DWORD         iModel;    // memory model (based on iMCS)
  DWORD           iRtx;    // which RTX (0=none, 1=RtxTiny, 2=RtxFull)

  DWORD     RunDeb : 1;    // dScope is running
  DWORD     CmdLog : 1;    // 1 := command log file is active
  DWORD     SerLog : 1;    // 1 := serial log file active
  DWORD       iRun : 1;    // GO or Step currently running
  DWORD   InfoiRun : 1;    // Sim about to start running
  DWORD      iBoot : 1;    // currently starting up Sim
  DWORD     iReset : 1;    // Reset issued via Button
  DWORD   wdog_rst : 1;    // Reset via Watchdog Timer
  DWORD     logmod : 1;    // 0:=LOG, 1:=SLOG
  DWORD     target : 1;    // 1:=target mode
  DWORD    jSilent : 1;    // kill function without query

  DWORD    MemAccR : 1;    // Memory-access possible while running
  DWORD    RegAccR : 1;    // Register-access possible while running
  DWORD     hTrace : 1;    // trace support
  DWORD     hCover : 1;    // code coverage support
  DWORD    hPaLyze : 1;    // Performance Analyzer support
  DWORD    hMemMap : 1;    // Memory-Map support
  DWORD     ResetR : 1;    // Reset possible while running
  DWORD     WinAcc : 1;    // ReadMem/WriteMem/LoadTyp activated
                           // by some non-simulation vehicle
  DWORD    sLrtime : 1;    // Limit Simulation-Speed to Real-Time  /8.8.2005/
  DWORD     bChina : 1;    // China-Version
  DWORD bScriptLng : 1;    // Arabic or Cyrillic Windows version

  DWORD    bLogItm : 1;    // ITM-Viewer / RealTime-Agent Log

  ZBK            *pScp;    // current Scope via WaScope()
  ZXFIL         *pScpF;    // and the associated file
//LIN             *pLn;    // and the line number
  union  {                 // and the line number
    LIN           *pLn;
    LINE32      *pLn32;
  };

  SYM            *SfrH;    // SFR-Symbols supplied by Cpu/Peri-DLL
  SYM           *hIntr;    // head of Intrinsic's
  SYM           *tIntr;    // head of Intrinsic's
  SYM           *hFunc;    // head of Functions (User/Signal

  struct szPAT  *hSrcP;    // head of search pathes (via 'set src')
  struct szPAT  *tSrcP;    // tail of search pathes (via 'set src')

  DWORD       startasm;    // DasmFox exports for LWinVw
  DWORD       altstart;
  DWORD         MarkPC;

                           // for enhancements:
  BYTE             UsrByte[16];   // currently not used.
  WORD16           UsrWord[16];   // currently not used.
  DWORD               movb : 1;   // MOVB instruction executed (SLE66)
  DWORD             SfrRdy : 1;   // data from sfr is valid (SLE66)
  DWORD              DemoF : 1;   // about to debug a demo file...
  DWORD              ModV2 : 1;   // 166 MODV2
  DWORD            Loading : 1;   // flag: loading a file
  DWORD          FlashLoad : 1;   // Flash-Load mode
  DWORD              Thumb : 1;   // Thumb State (ARM)
  DWORD             LaSupp : 1;   // LA-Supported (set by Sxxx-DLL)
  DWORD         ExtBrkSupp : 1;   // extended Brk-Support (while executing)
  DWORD              bLn32 : 1;   // 'struct lin' - extension active  /28.1.2011/
  DWORD            bElfNew : 1;   // new elf/dwarf loader is present
  DWORD             bRtxLd : 1;   // VartxArm - delayed RTX-info check
  DWORD        bExcTrcSupp : 1;   // [TdB: 20.11.2012] Support for Exception Trace Window
  DWORD        DbgRecovery : 1;   // Target DLL supports debug recovery
  DWORD        SWBreakConf : 1;   // Target DLL supports SW Breakpoint Configuration
  DWORD          TargetMap : 1;   // Target DLL supports Target Memory Map Command
  DWORD   PowerMeasurement : 1;   //  ULINKplus power measurement and pin state support
  DWORD             TpData : 1;   // Tracepoint support: Trace Data/Access Point (TraceDataPoint/TraceAccessPoint commands)
  DWORD     TpInstrRunSusp : 1;   // Tracepoint support: Instruction Trace Run/Suspend (TraceRun/TraceSuspend commands)
  DWORD        TpInstrHalt : 1;   // Tracepoint support: Instruction Trace Halt (TraceHalt command)
  DWORD                    :12;   // currently not used.
  DWORD             UsrDwrd[15];  // currently not used.


//--- debug specific flags: -----------------------------------

  DWORD   periodic : 1;    // periodic window update Flag
  DWORD      aLwin : 1;
  DWORD     aCover : 1;
  DWORD      aSer1 : 1;
  DWORD      aSer2 : 1;
  DWORD        aPa : 1;
  DWORD       aBox : 1;
  DWORD   viewmode : 3;    // LwinVw: view mode
  DWORD      vrSel : 4;    // LwinVw: address-range selection
  DWORD       aSym : 1;    // Symbols Dialog active
  DWORD      aTbox : 1;    // ToolBox is active
  DWORD      AscS1 : 1;    // Serial #1 - 0:=Ascii, 1:=Hex mode
  DWORD      AscS2 : 1;    // Serial #2 - 0:=Ascii, 1:=Hex mode
  DWORD      AscS3 : 1;    // Serial #3 - 0:=Ascii, 1:=Hex mode /5.8.2003/
  DWORD      aSer3 : 1;    // Serial #3 active  /5.8.2003/
  DWORD      eProf : 4;    // Execution-Profiling Mode
                           // 0=None, 1=Time, 2=Calls
  DWORD        aLa : 1;    // LA-View
  DWORD       aPa1 : 1;    // Enhanced Performance Analyzer

  DWORD      AscS4 : 1;    // Serial #4 - 0:=Ascii, 1:=Hex mode /17.1.2007/
  DWORD      aSer4 : 1;    // Serial #4 active  /17.1.2007/
  DWORD     StkLoc : 1;    // UnwindStackLoc active
  DWORD     TrcWin : 1;    // Trace-Display-Window active
                           //   2-Bits free

  BYTE           trace;    // Record trace information
//-------------------------------------------------------------

  BYTE           ptree;    // dump expression tree (_ptree_)
  BYTE           traps;    // enable/disable interrupt/trap messages
  BYTE           *trap;    // Trace buffer
  int           curtra;    // current trace index
  int           numtra;    // records in buffer

  int          cbreaks;    // number of C-breaks
  BKS          *BpHead;    // List head of Breakpoints
  BKS          *BpTail;    // List tail of Breakpoints
  DWORD         AccOff;    // if > 0: ignore access breaks
  BYTE     BpBlock : 1;    // Block Bp-commands if Bp-Dialog is active
  BYTE     PaBlock : 1;    // Block PA-commands if PA-Dialog is active
  BYTE       PaDlg : 1;    // PA-Dialog is open
  BYTE       PaRun : 1;    // PA is running
  BYTE       PaTim : 1;    // Update PA time information
  BYTE         FuncGen;
  BYTE          KeyEsc;    // Escape key has been pressed

                           // log file
  char        *CmdLogN;    // name of command log file
  FILE        *CmdLogH;    // file handle of command log file
  char        *SerLogN;    // name of serial log file
  FILE        *SerLogH;    // file handle of serial log file

  
  HMODULE        Linst;    // DLL Instance handle
  HMODULE        Ginst;    // AGDI Instance handle
  HMODULE        Rinst;    // RTX-DLL Instance handle
  HWND            hwnd;    // CMainFrame's m_hWnd, Parent for Dll-Dialogs
  DWORD           hmsg;    // WM_OUTOUT Message

  DYM           *pMenu;    // attached by peripheral dll
  DYM           *pMtrg;    // attached via AGDI
  DYM           *pMrtx;    // attached via RTX dll

  int       (*vp) (struct bom *pioc, DWORD nCode);   // DLL init/uninit
  int      (*rvp) (DWORD nCode, void *p1);           // RTX-DLL init/uninit

  HWND          curDlg;    // currently active modeless dialog

  union rstat       st;    // Run-Status Info
  DWORD            nPC;    // Offset (16-bit) of the PC
  INT64         cycles;    // number of cycles executed
  int            etime;    // execution time, counts down from tdelay to 0
  int           tdelay;    // Time till next time watch
  WORD16         error;    // Error-code
  DWORD         ErrAdr;    // Opt. Error-address

  DWORD         OscFrq;    // Oscillator frequency

  VTR            *pVTA;    // allocated array of MAXVTR VtReg's
  WATCH        **pVTwa;    // and watch pointer list
  DWORD            nVT;    // number of currently used VtReg's

  CSTK           *pStk;    // call-stack entries
  int             nStk;    // call-stack depth tracker

  int              tos;    // PA-stack index
  PAST           *StPa;    // PA-stack slots
  PA           *PaBgHd;    // background PA always present.
  PA           *PaHead;    // Head of PA ranges
  PA           *PaTail;    // Tail of PA ranges
  DWORD         paEadr;    // PA current range entry/exit address
  INT64         CapCyc;    // captures cycles


//  struct FoxCont   fox;
#if 1
  union itrp      itrp;    // Interrupt status
  BYTE           sleep;    // CPU sleep mode
  BYTE         _break_;    // Stop execution, if _break_ := 1
  BYTE           radix;    // number base, for compat. to dScope
  BYTE       intchange;    // signal Changes in interrupt status
  DWORD        intvect;    // interrupt vector address if itrp.w.flag = 1
  BYTE          intdis;
  DWORD           cPsw;    // current PSW
  DWORD           oPsw;    // old PSW
#endif

  DWORD     EINIT_exec;    // 0=EINIT not yet executed / 1=EINIT executed
  DWORD    SRVWDT_exec;    // 0=SRVWDT not yet executed / 1=SRVWDT executed
  DWORD     WDOGACTIVE;    // 0=Watchdog disabled / 1=Watchdog enabled

                                         // Polymorphic function
  DWORD      (*SimFunc) (DWORD nCode, void *p1, void *p2);

  void         (*LFwLF) (CRT *X);        // DasmFox exports for LwinVw
  void         (*LRwLF) (CRT *X);
  void        (*CrtAtt) (CRT *X);
  char         *(*LGet) (CRT *X);
  BYTE      (*FileLine) (ZXFIL *pF, DWORD nLine);
  ZXFIL   *(*FileScope) (DWORD nAdr);    // End of DasmFox exports
  ZXFIL    *(*DocToZXF) (LPCTSTR pN);    // Map Docname to ZXFIL

  WORD16    *(*GetAttr) (DWORD nAdr);    // get attr for address

  void       (*StopRun) (void);          // Uv3-Stop-Button was pressed
  void      (*ServeWDT) (void);          // Watchdog timer function
  void         (*GoTil) (DWORD nAdr);    // Go until for Fox-Sim test

  int       (*HistFunc) (int nCode, int indx, int n1, BYTE *pR);
  DWORD       (*PaFunc) (int nCode, int nAdr, void *p);

#if 0    // 14.5.2007, orginal
  void        (*GetOpc) (DWORD nAdr, DWORD *vp); // get 4-Bytes Opcode
#else    // 14.5.2007, adapted for SLE7. Warning: any non-SLE7 driver still uses 'void (*)()' !!!
  WORD16     *(*GetOpc) (DWORD nAdr, DWORD *vp); // get 4-Bytes Opcode
#endif

  DWORD       (*SetOpc) (DWORD nAdr, BYTE *pB, DWORD mMany);
  DWORD      (*ReadMem) (DWORD nAdr, DWORD nMany, BYTE *vp);
  DWORD     (*WriteMem) (DWORD nAdr, DWORD nMany, BYTE *vp);

  void          (*Step) (DWORD nMany, DWORD type);   // Step function
  void          (*Stop) (void);          // Notbremse
  void         (*Reset) (WORD16 nFlags); // Reset Simu
  void       (*GetRegs) (void *vp /*struct FoxReg *vp*/);
  void       (*SetRegs) (void *vp /*struct FoxReg *vp*/);
  DWORD       (*MapMem) (DWORD SegTyp, DWORD nAdr1, DWORD nLen);

  int       (*ValidAdr) (DWORD nAdr, int nCode);
  DWORD       (*BpInfo) (DWORD nAdr, WORD16 nCode);
  DWORD     (*BpExists) (DWORD nAdr);
  BKS     *(*BreakFunc) (WORD16 nCode, WORD16 n1, DWORD nAdr, BKS *pB); /* JR, 25.07.2012 - Return value 0xFFFFFFFE:
                                                                           Resource allocation not successfull. */
  BKS      *(*NewBreak) (DWORD nAdr, WORD16 nType, WORD16 nCode);
  void         (*DlgBp) (struct BpBlk *pB);        // create Dialog Bp

  VTR      *(*p_create) (char *name, WORD16 nTyp);    // create VTREG
  void      (*p_putval) (VTR *pV, union v *pCont);
  void      (*p_getval) (VTR *pV, union v *pCont);
  VTR       *(*FindVTR) (char *name);                 // find VTREG
  void        (*w_read) (pACCESS ac, WFP cur, WFP pnew);
  void       (*w_write) (pACCESS ac, WFP pnew);
  void         (*inval) (DWORD val);
  int       (*w_access) (DWORD adr, DWORD mask);
 
  void     (*ResetPeri) (int nCode);        // Reset Peripherals
                                            // 7.10.2003  nCode = 0: PRE-RESET         
                                            //            nCode = 1: standard Reset
  void      (*w_create) (pWATCH wp);                 
  int       (*t_create) (INT64 tv, void (*fc)(void));
  void      (*t_remove) (int indx);
  void      (*intcheck) (void);

                                         // Serial I/O
  void         (*SerIn) (int nCom, WORD16 val);  // Keypress in SerVw

  void     (*DlgUpdate) (void);          // update peripheral dialogs
  void     (*ExtDlgUpd) (void);          // update AGDI extension dialogs
  void     (*RtxUpdate) (DWORD nCode);   // update RTX extension dialog(s)


//------------------------
//--- Kernel functions ---
//------------------------

  void      (*DoEvents) (void);
  DWORD       (*Notify) (DWORD nCode, void *lp1);
  WORD16      (*RunCmd) (char *cmd, int nLen);
  void     (*CmdWinOut) (char *text);
  void     (*CmdCmdOut) (char *text);
  void       (*FreeEnv) (DWORD env);
  void       *(*GetMem) (DWORD many, int env);
  char   *(*SaveString) (char *s, int env);

  void       (*CleanUp) (void);                // clean up after Stop-Debug
  int     (*SerByteOut) (int nCom,   BYTE *pS, DWORD nMany); // write into SerWin
  int     (*SerWordOut) (int nCom, WORD16 *pW, DWORD nMany); // write into SerWin

  void      (*LogWrite) (int nSel, char *pLine, int nMany);
  void      (*BootHelp) (DWORD hlpExpr, DWORD noHelp, DWORD hStart);
  void     (*CreaWatch) (IRX *pX);
  WORD16   (*WatchCalc) (EXP *ep, char *pB);
  WORD16    (*AsnWatch) (char *exp, int nLen);
  void       (*LoadTyp) (char *pS, DWORD nAdr, int base, TYP *tp, union v *pU);
  void       (*LoadStr) (char *pS, DWORD nAdr, int nMax);
  ZBK       *(*WaScope) (DWORD nAdr);
  void        (*m_copy) (DWORD nDest, DWORD nSec, DWORD nMany);

  DWORD      (*TaskRun) (union v *pV);
  SYM       *(*FindPub) (char *name);
  SYM   *(*PubSymByVal) (UINT64 nVal, DWORD nMask);
  DWORD    (*FetchItem) (UINT64 nAdr, TYP *tp, union v *pU);
  INT32    (*DecodeAdr) (DWORD nCode, UINT64 nAdr, char *pB);
  DWORD      (*pCbFunc) (DWORD nCode, void *vp);     // AGDI-style callback

  WORD16      *pSfrTab;      // iMCS51: Sfr-Attribute table
  WORD16         ticks;
  WORD16        nticks;      // default: 12 Ticks per Cycle
  BYTE             iip;
  BYTE          int_ff;
  BYTE           delay;  // 1 = Interupts disabled for next instruction
//BYTE       intchange;  // use above declaration !
  BYTE       interrupt;
  BYTE        cpusleep;
//DWORD        intvect;  // use above declaration !
  DWORD       xpagereg;
  DWORD     *exectimes;  // execution times table
  WORD16      xonepage;
  WORD16   exectimeint;  // i251: interrupt exec time
  WORD16     execsleep;  // i251: sleep exec time
  BYTE          oddjmp;  // i251: odd target jump time
  BYTE        jmptaken;  // i251: taken 251-jumps time
  BYTE       bjmptaken;  // i251: add for jmp taken JB,JNB,JBC
  BYTE       cjnetaken;  // i251: add for cjne taken
  BYTE       djnztaken;  // i251: add for djnz taken
  BYTE      jmp51taken;  // i251: add for 51 jmp taken JZ,JNZ,JB,JNB,JB,JC,JNC 

  double   (*pCalcTime) (INT64 cyc);

  DWORD       *pOldAdr;  // Old address (requ. for Sle66)
  union v    *pUv [15];  // place holder ([16] --> [15] wg. u_create
  int       (*u_create) (float nSec, void (*fc)(void));  // /24.2.2004/

//char     szMsg[2048];  // place holder for future extensions

//---13.9.2005: added Bom-Data vor VIfes-Bom:
//char     szMsg[1744];  // (2000-(64*4)) place holder for future extensions

//---5.1.2007: added a more generic Rtx-Function for Update() etc:
#define RTXUP_RESET      1    // pRtxFunc(): Reset given - Update RTX-Dialog(s).
#define RTXUP_DCCMSG     2    // pRtxFunc(): DCC-Message is coming in
  INT32     (*pRtxFunc) (DWORD nCode, void *vp1, void *vp2);
  DWORD     (*pUv3Func) (DWORD nCode, void *vp1, void *vp2);

//---12.3.2007: RAS RTX task ID to pointer function (cast return type to RTAH_ERR)
#define RTAH_BOM_GET_RTX_TASK     1 // pRTAHFunc(): Get RTX task pointer from task ID (vp1: (unsigned char)tID, vp2: (*unsigned int)pTask)
#define RTAH_BOM_RESET_RTX_INFO   2 // pRTAHFunc(): Reset all RTX info contained in RTA (vp1: NULL, vp2: NULL)
  DWORD      (*pRTAHFunc) (DWORD nCode, void *vp1, void *vp2);

//---16.1.2008: RAS UVMsgbox / UVSOCK async notification functions
  UINT      (*UVMsgBox) (HWND hWnd, 
                         const char *pMsg,
                         const char *pCaption,
                         UINT uiType,
                         UINT uiDefAnswer,
                         UV_OPERATION operation, 
                         UV_STATUS status);

  void      (*UVSOCKEvent) (const char *pMsg,                         
                            UV_OPERATION operation, 
                            UV_STATUS status);


//---6.6.2007: last hit Bp
  struct hitbp   hitbp;

//---19.7.2007: RAS Globally accessible flag to indicate special 3rd party modes such as LabVIEW
#define UV_MODE_NORMAL     0    // Normal mode - no 3rd party control
#define UV_MODE_LABVIEW    1    // LabVIEW session
#define UV_MODE_ECLIPSE    2    // LabVIEW session
#define UV_MODE_LAST       3    // End placeholder
  char          uvMode;
  
  char     szMsg[1683];  // (2000-(64*4)-20-sizeof(hitbp)-sizeof(uvMode)) place holder for future extensions
  //char     szMsg[1687];  // (2000-(64*4)-16-sizeof(hitbp)-sizeof(uvMode)) place holder for future extensions

  DWORD   Usr1Dwrd[64];               // Cortex, etc.  /15.10.2006/
#define SXX_CRITSECT   Usr1Dwrd[63]   // reserved for CritSect export
#define SXX_SIMHANDLE  Usr1Dwrd[62]   // reserved for simulation thread (gth.ThID) export
#define SXX_INSCOUNTCB Usr1Dwrd[61]   // reserved NXP n-instr.-callback  /9.10.2008/
#define SXX_GTH_HTH    Usr1Dwrd[60]   // reserved for simulation thread HANDLE (gth.hTh) export
#define SXX_GTH_HEVENT Usr1Dwrd[59]   // reserved for simulation thread run event handle (gth.hEvent) export
#define SXX_GTH_MODE   Usr1Dwrd[58]   // reserved for address of simulation thread run mode (&gth.mode) export
#define SXX_PMAINWND   Usr1Dwrd[57]   // reserved for 'theApp.m_pMainWnd'  /6.10.2009/
#define SXX_ITM_CLIST  Usr1Dwrd[56]   // reserved for 'ITM_CLIENT *'
#define xxAGSIINT      Usr1Dwrd[55]   // reserved for AGSI AGSI_ONINTERRUPT notification function
#define SXX_PUBLICS    Usr1Dwrd[54]   // reserved for public symbols head
#define SXX_ZBKBLOCKS  Usr1Dwrd[53]   // reserved for ZBK-blocks head
#define SXX_IRQTAB     Usr1Dwrd[52]   // reserved for IrqTab (IRQINFO)
#define SXX_RTEFLASH   Usr1Dwrd[51]   // reserved for Rte-Flash algo exposition
#define SXX_PATHEXPAND Usr1Dwrd[50]   // resrved for Path expand function
#define SXX_RECENTCALL Usr1Dwrd[49]   // reserved for recent call return value
#define SXX_RTESRCPATH Usr1Dwrd[48]   // reserved for Rte-Search path exposition
#define SXX_BRKTRC_PA  Usr1Dwrd[47]   // reserved for BreakTrace-Pane RunCmd()

#define SXX_RES_1      Usr1Dwrd [1]   // reserved for Cortex NVIC simulation 
#define SXX_RES_0      Usr1Dwrd [0]   // reserved for Cortex NVIC simulation 


  DWORD     bSleepSim : 1;  // 'Sleep' the simulator (ie execute thread, but perform no work)
  DWORD       bSockBp : 1;  // Bp-command orginated by Socket
  DWORD        eBpCmd : 1;  // executing a BpCmd() Break-command
  DWORD      bTrcDisp : 1;  // Sxxx supports Trace-Display-Window
  DWORD     bSleepCmd : 1;  // 16.1.2012, _sleep_() is running
  DWORD   bExcTrcDisp : 1;  // Exception Trace Window

                            // Secure Debug
  DWORD    bSecureExt : 1;  // Security Extensions Available
  DWORD    bCPUSecure : 1;  // Current CPU Security State (0 - Non-Secure, 1 - Secure), if bSecureExt '1'
  DWORD     bNSTrcEna : 1;  // Non-Secure Trace Enabled, if bSecureExt '1'
  DWORD     bNSDbgEna : 1;  // Non-Secure Debug Enabled, if bSecureExt '1'
  DWORD      bSTrcEna : 1;  // Secure Trace Enabled, if bSecureExt '1'
  DWORD      bSDbgEna : 1;  // Secure Debug Enabled, if bSecureExt '1'

  DWORD    bNSUTrcEna : 1;  // Non-Secure Unprivileged Trace Enabled, if bSecureExt '1'
  DWORD    bNSUDbgEna : 1;  // Non-Secure Unprivileged Debug Enabled, if bSecureExt '1'
  DWORD     bSUTrcEna : 1;  // Secure Unprivileged Trace Enabled, if bSecureExt '1'
  DWORD     bSUDbgEna : 1;  // Secure Unprivileged Debug Enabled, if bSecureExt '1'

  DWORD               :16;

  HANDLE      VIF_Inst;  // Handle of VIFBOM.DLL
  char        *ItmLogN;  // name of serial log file
  FILE        *ItmLogH;  // file handle of serial log file

//char     cPad[48-16];  // cPad[48-8];  // filler

  DWORD     (*MemAccX) (void *pIn);       // ACCMEMX *pIn
  DWORD      (*DapAcc) (void *pIn);       // DAPACC *pIn
  BYTE        accSizeX;  // Access Size in Bytes to use for AG_MemAccX calls, use old read/write if 0

  char     cPad[48-25];  // cPad[48-16];  // filler
};

//---NOTE: sizeof (bom) := 0xB30 (2864) !



/*
 * pUv3Func (DWORD nCode, void *vp1, void *vp2)
 *  nCodes:
 */
#define FUV3_GETDCCINFO   1   // vp1 := &DCCINFO, vp2 := NULL
#define FUV3_RESETDCCINFO 2   // vp1 := NULL, vp2 := NULL
// more FUV3_xxx TBD.


#if 0  // Example usage for pUv3Func (FUV3_GETDCCINFO):
  DCCINFO   sDcc;
  DWORD nR;
  if (pio->pUv3Func == NULL) nR = 0;  // Uv3Func not present
  else nR = pio->pUv3Func (FUV3_GETDCCINFO, (void *) &sDcc, NULL);
  if (nR != 0)  // Ok.
  else          // failed.
#endif





extern struct bom   ioc;
extern struct bom  *pio;



//#pragma pack(push, 4)
struct EvCA  {        // used for DTC-Calc Expr. Event (Notify --> SimFunc)
  LPCTSTR   pStr;     // expression-text
  __int64   nRes;     // Result
  int       nErr;
};
//#pragma pack(pop)

struct DtcMessage  {  // for DTC-Target-Messaging
  char    szB [12];
  int     len;
  int     seq;
  int     cha;
  INT64   tSt;        // Time-Stamp
};


typedef struct DTC_MRW {   // for DTC-Memory Read/Write
  UINT64     nAdr;    // Memory Address
  int       nMany;    // number of Bytes to read/write
  BYTE        *pB;    // Buffer
  int       nMode;    // Mode: 
  int        nErr;    // Error-Code
  UINT64  nErrAdr;    // Error-Address
  BYTE    Res[32];    // reserved.
  INT64        v1;    // z.B. States (DecodeTimeStamp)
  INT64        v2;
  double       v3;    // z.B. Seconds (-++-)
} MEM_RW;
#define MEM_RW_RD    0x01
#define MEM_RW_WR    0x02
#define CNV_TSTAMP   0x03
#define MEM_DEC_ADR  0x04

/*
* Struct used to queue string output initiated by LogWrite (int nSel, char *pLine, int nMany);
*  pio->Notify (UV_QUEUE_LOGWRITE, struct LogWritePar *pLogWritepar);
*/
struct LogWritePar {
  int nSel;
  char *pLine;
  int nMany;
};


#if 0
/*
 * DCI command messages (uses Windows MSG structure)
 */
#define DCI_BUILDPRJ        0x1000     // build current project
#define DCI_GETBUILDSTATUS  0x1001     // get current project build status
#define DCI_OPENPROJECT     0x1002     // open project
#define DCI_CLOSEPROJECT    0x1003     // close project
//--- DCI-Command return values via 'pAck':
#define DCI_CMDOK           1   // command was Ok.
#define DCI_CMDNOTALLOWED   2   // command cannot be executed now
#define DCI_BLDERRORS       3   // prevous build-project had error(s)
#define DCI_CMDFAILED       4   // dci command failed (e.g. open project)

#define DCI_DONTKNOW       -1   // don't know (example build-status)
#define DCI_UV2READY       -2   // Uv2 booted (Uv2 - not debug mode)
#define DCI_UV2DEBUG       -3   // Uv2 debug modus
#endif


/*
 * Flash-Capabilities provided by Target-DLL (Monxxx.dll)
 */
#define CAP_WRITE   0x0001    // Flash-Write (DownLoad)
#define CAP_ERASE   CAP_WRITE    // Flash-Erase

/*
 * UV MessageBox Macros
 */

extern UINT UVMsgBox(HWND hWnd, 
                     const char *pMsg,
                     const char *pCaption,
                     UINT uiType,
                     UINT uiDefAnswer,
                     UV_OPERATION operation, 
                     UV_STATUS status);

extern void UVSOCKEvent(const char *pMsg,
                        UV_OPERATION operation, 
                        UV_STATUS status);

#define UVMsgBoxSimple(pMsg)              UVMsgBox(this->m_hWnd, pMsg, NULL, MB_OK | MB_APPLMODAL, 1, UV_MSGBOX_MSG, UV_STATUS_INFO)
#define UVMsgBoxWarning(pMsg)             UVMsgBox(this->m_hWnd, pMsg, NULL, MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION, 1, UV_MSGBOX_MSG, UV_STATUS_INFO)
#define UVMsgBoxInfo(pMsg)                UVMsgBox(this->m_hWnd, pMsg, NULL, MB_OK | MB_APPLMODAL | MB_ICONINFORMATION, 1, UV_MSGBOX_MSG, UV_STATUS_INFO)
#define UVMsgBoxYesNo(pMsg, uiDefAnswer)  UVMsgBox(this->m_hWnd, pMsg, NULL, MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION, uiDefAnswer, UV_MSGBOX_MSG, UV_STATUS_INFO)

/*
 * SYSTEM ANALYZER 
 */

#define SA_MAX_NAME         128  // Max length of name string, including terminating '\0'
#define SA_MAX_UNIT           8  // Max length of unit string, including terminating '\0'
#define SA_MAX_FREQUENCIES   16  // Max number of supported capture frequencies

// System Analyzer Data Types
#define SA_DTYPE_ANY            0x0000  // Any data type, look at channel type and data sizes for more info
#define SA_DTYPE_CHAR           0x0001  // Signed Char
#define SA_DTYPE_UCHAR          0x0002  // Unsigned Char
#define SA_DTYPE_SHORT          0x0003  // Signed Short
#define SA_DTYPE_USHORT         0x0004  // Unsigned Short
#define SA_DTYPE_LONG           0x0005  // Signed Long (32-Bit)
#define SA_DTYPE_ULONG          0x0006  // Unsigned Long (32-Bit)
#define SA_DTYPE_LONGLONG       0x0007  // Signed Long Long (64-Bit)
#define SA_DTYPE_ULONGLONG      0x0008  // Unsigned Long Long (64-Bit)
#define SA_DTYPE_FLOAT          0x0009  // Float (32-Bit)
#define SA_DTYPE_DOUBLE         0x000A  // Double (64-Bit)


// System Analyzer Channel Types
#define SA_CTYPE_PRINTF         0x0000  // Printf-Data
                                        // - value.uc8[0] - Character          (Byte     0)
                                        // - Reserved                          (Bytes 1..7)
#define SA_CTYPE_RTXTRACE       0x0001  // RTX Events
                                        // - value.ul - Data                   (Bytes 0..3)
                                        // - value.uc8[4] - Event Type (Size)  (Byte     4)
                                        // - Reserved                          (Bytes 5..7)
#define SA_CTYPE_ITM            0x0002  // Instrumentation Trace Channel
                                        // - value.ul      - Data                 (Bytes 0..3)
                                        // - value.uc8 [4]  - Data size           (Byte     4)
                                        // - value.uc8 [5]  - ITM channel number  (Byte     5)
                                        // - Reserved                             (Bytes 6..8)
#define SA_CTYPE_DTRACE         0x0003  // Data Trace
                                        // - value.ul     - Data               (Bytes 0..3)
                                        // - value.uc8 [4] - Data Size         (Byte     4)
                                        // - Reserved                          (Bytes 5..7)
                                        // - Extended Data: Optional PC, Optional Data Address
#define SA_CTYPE_EXCEPTION      0x0004  // Exception Trace
                                        // - value.u16 - IRQ Number            (Bytes 0..1)
                                        // - value.uc8[2] - Exception Sample Type                       (Byte     2, Values: 1 - Entry, 2 - Exit, 3 - Return)
                                        // - Reserved                          (Bytes 3..7)
#define SA_CTYPE_EVTREC         0x0005  // Event Recorder Event
                                        // - value.ul2[0] - ID                 (Bytes 0..3)
                                        // - Reserved                          (Bytes 4..7)
                                        // - Extended Data: Full Event Payload
#define SA_CTYPE_CMSIS_PC       0x0006  // CMSIS-DAP Performance Counter
                                        // - See given data type               (Bytes 0..7)
#define SA_CTYPE_SIGMADELTA     0x0007  // Performance Counter SIGMADELTA Type 
                                        // - Double                            (Bytes 0..7)
                                        // - Raw Data, external data snapshot  (Bytes 0..n)
#define SA_CTYPE_SYNC           0x0008  // Time synchronization type 
                                        // - value.ull    - DWT Cycles         (Bytes 0..7)
                                        //                - TDT Cycles defined in data header
//#define SA_CTYPE_RTX5EVT        0x0009  // RTX5 Events - deprecated, do not use! -
                                        // - value.ul - Data                   (Bytes 0..3)
                                        // - value.uc8[4] - Event Type (Size)  (Byte     4)
                                        // - Reserved                          (Bytes 5..7)
#define SA_CTYPE_EVTREC_STATES  0x000A  // Event Recorder Event States
                                        // - value.ul1[0] - ID                 (Bytes 0..3)
                                        // - value.ul2[0] - SCVD_DRAWSTATE     (Bytes 4..7)
                                        // - Extended Data: Full Event Payload
#define SA_CTYPE_EXT_WAVEFORM   0x000B  // External Waveform
                                        // - value.ul     - Data               (Bytes 0..7)
                                        // - Extended Data: Optional PC, Optional Data Address
#define SA_CTYPE_EXT_SCVD       0x000C  // External SCVD Data
                                        // - value.ul       - Data             (Bytes 0..3)
                                        // - value.ulArr[1] - SCVD_DRAWSTATE   (Bytes 4..7)
                                        // - Extended Data: Optional PC, Optional Data Address
#define SA_CTYPE_EXT_SCVD_STATE 0x000D  // External SCVD State
#define SA_CTYPE_ALL            0xFFFF  // All types, used with channel info functionality



// System Analyzer - Display Types
#define SA_DISPLAY_STEPS        0x0001  // Draw a continous signal with ideal edges
#define SA_DISPLAY_CURVE        0x0002  // Draw a continous signal as a curve
#define SA_DISPLAY_BIT          0x0004  // Like steps but only drawing two states ('0' if value is 0, else '1')
#define SA_DISPLAY_STATES       0x0008  // Draw states 
#define SA_DISPLAY_SLICES       0x0010  // Draw time slices, e.g. to indicate an active state
#define SA_DISPLAY_DISCRETE     0x0020  // Draw discrete events
// TBD if adding transitions between channels/sub-channels as display type or elsewhere

#define SA_DISPLAY_ALL          0xFFFF  // Can be any of the above


#pragma pack(1)


/*
 * Event-State definitions: sample.tState holds event-state
 */
//#define EVS_STATE_NONE        0                 // Undefined state := 0
//#define EVS_STATE_CREATE      1                 // Create State    := 1
//#define EVS_STATE_DELETE      2                 // Delete State    := 2
//#define EVS_STATE_ACTIVE      3                 // Delete State    := 3
//#define EVS_STATE_WAITING     4                 // Waiting State   := 4
//#define EVS_STATE_IDLE        5                 // Idle State      := 5
//#define EVS_STATE_STALLED     6                 // Stalled State   := 6
                                                  // reserved currently useless states:
//#define EVS_STATE_RESET       7                 // Note: Reset-State not used but reserved
//#define EVS_STATE_CLEAR       8                 // Note: Clear-State not used but reserved


#define EVS_STATE_NONE          0          // 0 := inactive state
#define EVS_STATE_ACTIVE        1          // 1 := Active State
#define EVS_STATE_IDLE          2          // 2 := Idle State
#define EVS_STATE_WAITING       3          // 4 := Waiting State
#define EVS_STATE_STALLED       4          // 4 := Stalled State
#define EVS_STATE_DELETE        5          // 5 := Deleted State
#define EVS_STATE_CREATE        6          // 6 := Create State (Sub-State may be specified !)

#define EVS_STATE_RESET         7          // 7 := Reset State
#define EVS_STATE_CLEAR         8          // 8 := Clear State





// System Analyzer - Group Descriptor
// Information about a group, where channels can be added
typedef struct SA_GroupInfo_t {
  unsigned short                     id;  // Group ID (1..n), refered by parentId in SA_CHANNEL_INFO
  unsigned short               parentId;  // Parent ID where this group should be added, 0 means adding to top level
  unsigned short            channelType;  // Containing Channel Type, see SA_CTYPE_XXX
  char            dispName[SA_MAX_NAME];  // Group Display Name
  unsigned long              bHide :  1;  // Show channels only, hide the parent group element
  unsigned long                    : 31;
  unsigned long            reserved[32];  // Reserved for future use
} SA_GROUP_INFO;


// System Analyzer - Group Info Structure
// Structure to request channel information and to configure
// groups.

#define SA_GROUP_COUNT       0    // Request number of available groups
#define SA_GROUP_GET         1    // Get channel infos

typedef struct SA_GroupInfoReq_t {
  unsigned long                cmd;  // Request type

  unsigned short                id;  // Specific group ID
                                     // - SA_GROUP_GET
                                     //   - [in] : Specific group to request info for
                                     //            Set to (unsigned short)(-1) to get all channel infos

  unsigned short             count;  // Number of groups
                                     // - SA_GROUP_COUNT
                                     //   - [out] : Number of currently available groups
                                     // - SA_GROUP_GET
                                     //   - [in]  : Max number of groups infos to return
                                     //   - [out] : Number of returned groups infos

  SA_GROUP_INFO           *infos;  // Group info buffer
                                     // - SA_GROUP_COUNT : Ignored, can be NULL
                                     // - SA_GROUP_GET
                                     //    - [in/out] : Buffer for copies of group infos. Owned by caller.
                                     //                 Must be large enough to hold 'count' group infos.

  unsigned long       reserved[32];  // Reserved for future use

} SA_GROUP_INFO_REQ;




// System Analyzer - Channel Descriptor
//   (Data size not critical, should be fetched only once)
typedef struct SA_ChannelInfo_t {
  unsigned short                            id;  // ID for reference in later calls
                                                 // returns ID when used with SMF_SA_ADD_CHANNEL
  unsigned short                      parentId;  // Group ID where this channel should be added, 0 means adding to top level
  unsigned short                 syncChannelId;  // ID of Sync reference channel, see SA_CTYPE_SYNC
  unsigned short                   channelType;  // Channel Type, see SA_CTYPE_XXX
  unsigned short                      dataType;  // Data Type, see SA_DTYPE_XXX
  union v                               minVal;  // Minimum possible value (this is the overall possible minVal for CMSIS-DAP Performance Counters)
  union v                               maxVal;  // Maximum possible value (this is the overall possible maxVal for CMSIS-DAP Performance Counters)
  char                   dispName[SA_MAX_NAME];  // Display Name
  char                   dispUnit[SA_MAX_UNIT];  // Display Unit
  unsigned short                      dispType;  // Display Type, see SA_DISPLAY_XXX
  unsigned long                   numCaptFreqs;  // Number of supported frequencies (0 means frequency not configurable)
  unsigned long  captFreqs[SA_MAX_FREQUENCIES];  // Supported frequencies
  unsigned long                         tsFreq;  // (Underlying) Timstamp Frequency
  unsigned long                        cpuFreq;  // Target CPU Frequency
  unsigned long                 numSubChannels;  // Number of available sub-channels, use SA_SUBCHANNEL_INFO_REQ request to get detailed infos
  unsigned long                     numRecords;  // Number of records(number of datablocks) of PC sigmadelta channel
  unsigned long                      blockSize;  // Datablock size of PC sigmadelta channel
  unsigned long                  dataSize :  8;  // Data size  
  unsigned long            decimationRate : 16;  // Decimation rate used for pre calculated sigmadelta data (SA_CTYPE_SIGMADELTA)
  unsigned long                     bHide :  1;  // Hide channel
  unsigned long                           :  7;  // reserved 
  unsigned long                         param0;  // Channel type specific parameter
                                                 // - RTX5 Event Recorder: RTX5 component ID
  unsigned long                         param1;  // Channel type specific parameter
  unsigned long                   reserved[30];  // reserved for future use
} SA_CHANNEL_INFO;


// System Analyzer - Channel Info Structure
// Structure to request channel information and to configure
// channels.

#define SA_CHANNEL_COUNT       0    // Request number of available channels
#define SA_CHANNEL_GET         1    // Get channel infos

typedef struct SA_ChannelInfoReq_t {
  unsigned long                cmd;  // Request type

  unsigned short                id;  // Specific channel ID
                                     // - SA_CHANNEL_GET
                                     //   - [in] : Specific channel to request info for
                                     //            Set to (unsigned short)(-1) to get all channel infos

  unsigned short             count;  // Number of channels
                                     // - SA_CHANNEL_COUNT
                                     //   - [out] : Number of currently available channels
                                     // - SA_CHANNEL_GET
                                     //   - [in]  : Max number of channel infos to return
                                     //   - [out] : Number of returned channel infos

  unsigned short       channelType;  // Channel type filter
                                     // - SA_CHANNEL_COUNT, SA_CHANNEL_GET
                                     //   - [in/out] : Request only for a specific channel type, use SA_TYPE_ALL
                                     //                to include all types.

  SA_CHANNEL_INFO           *infos;  // Channel info buffer
                                     // - SA_CHANNEL_COUNT : Ignored, can be NULL
                                     // - SA_CHANNEL_GET
                                     //    - [in/out] : Buffer for copies of channel infos. Owned by caller.
                                     //                 Must be large enough to hold 'count' channel infos.

  unsigned long       reserved[32];  // Reserved for future use

} SA_CHANNEL_INFO_REQ;

// sizeof(SA_CHANNEL_INFO_REQ) == 18 + 32*4 == 146 Bytes


// System Analyzer - Sub-Channel Descriptor
// Information about a sub-channel, i.e. a data subset of the parent channel.
typedef struct SA_SubChannelInfo_t {
  unsigned long                     id;  // Reference Sub-Channel ID, refers to IDs in SA_DATA payload (e.g. ITM channel number for type SA_TYPE_ITM).
                                         // id usage scheme depends on parent channel type
  char           dispName[SA_MAX_NAME];  // Sub-Channel Display Name
  char           dispUnit[SA_MAX_UNIT];  // Display Unit
  unsigned short              dispType;  // Display Type, can be a bit-wise combination of SA_DISPLAY_XXX types.
                                         // Use parent channel display type if 0.

  union v                      extInfo;  // External Info: Task address for RTX, Interrupt address for EXC   
  unsigned short              parentId;  // Parent ID where this channel should be added, 0 means adding to top level
  unsigned short                   res;  // 
  unsigned long           reserved[31];  // reserved for future use
} SA_SUBCHANNEL_INFO;


// System Analyzer - Sub-Channel Info Structure
// Structure to request sub-channel information and to configure
// channels.

#define SA_SUBCHANNEL_COUNT       0  // Request number of available sub-channels
#define SA_SUBCHANNEL_GET         1  // Get sub-channel infos

typedef struct SA_SubChannelInfoReq_t {
  unsigned long                cmd;  // Request type

  unsigned long                 id;  // Main-channel ID
                                     // - SA_SUBCHANNEL_COUNT
                                     //   - [in] : Specific channel to request subchannel info for
                                     // - SA_SUBCHANNEL_GET
                                     //   - [in]  : Specific channel to subchannel request info for

  unsigned long              count;  // Number of channels
                                     // - SA_SUBCHANNEL_COUNT
                                     //   - [out] : Number of currently available sub-channels for channel with 'id'
                                     // - SA_SUBCHANNEL_GET
                                     //   - [in]  : Max number of sub-channel infos for channel with 'id' to return
                                     //   - [out] : Number of actually returned sub-channel infos for channel with 'id'

  SA_SUBCHANNEL_INFO        *infos;  // Sub-Channel info buffer   
                                     // - SA_SUBCHANNEL_COUNT : Ignored, can be NULL
                                     // - SA_SUBCHANNEL_GET
                                     //    - [in/out] : Buffer for copies of sub-channel infos. Owned by caller.
                                     //                 Must be large enough to hold 'count' sub-channel infos.

  unsigned long       reserved[32];  // Reserved for future use

} SA_SUBCHANNEL_INFO_REQ;

// sizeof(SA_SUBCHANNEL_INFO_REQ) == 16 + 32*4 == 144 Bytes



// System Analyzer - Channel Configuration
typedef struct SA_ChannelConfigReq_t {
  unsigned short           id;    // Channel ID as provided in SA_EVENT_CHANNEL
  unsigned long      captFreq;    // Index of capture frequency, ignored if no configurable capture frequency
  unsigned char    enable : 1;    // Enable all resources required for this channel:
                                  // 1 - Enable, 0 - Disable if channel is only consumer (e.g. for SWO trace)

  unsigned char     start : 1;    // Start Capture: 1 - Start, 0 - Stop
  unsigned char           : 6;    // reserved

  unsigned long  reserved[32];    // reserved for future use

} SA_CHANNEL_CONFIG_REQ;

// sizeof(SA_CHANNEL_CONFIG_REQ) == 9 + 32*4 == 137 Bytes



// System Analyzer - Channel Status Request
//   (Data size not critical, should be fetched only once)
typedef struct SA_ChannelStatusReq_t {
  unsigned short           id;    // Channel ID
  unsigned long      captFreq;    // Index of currently selected capture frequency
  unsigned long       cpuFreq;    // CPU Frequency
  unsigned long        tsFreq;    // (Underlying) Timer Frequency
  unsigned char   enabled : 1;    // Channel is enabled, i.e. all required resources are enabled
  unsigned char    active : 1;    // Channel is active and captures data
  unsigned char           : 6;    // reserved
  double         linearOffset;    // Offset (for data result calculation) [TdB: 09.10.2017] Adjust ULINK_I/U/P Calculation
  double           linearGain;    // Gain   (for data result calculation) [TdB: 09.10.2017] Adjust ULINK_I/U/P Calculation
  double         staticOffset;    // Static offset to add to data results (e.g. for ULINK_I)
  unsigned long  reserved[31];    // reserved for future use
} SA_CHANNEL_STATUS_REQ;

// sizeof(SA_CHANNEL_STATUS_REQ) == 17 + 16*4 == 81 Bytes



// System Analyzer - Data
//   (Data size critical, used to transport data between SARMxxx data storage and System Analyzer Window)
typedef struct SA_DataHeader_t {
  UINT64             timestamp;  // Timestamp in CPU cycles
  unsigned long         offset;  // Item offset if multiple items for a timestamp
  union {
    unsigned char        flags;
    struct {
      unsigned char   overflow : 1;  // First item after recovery from overflow
      unsigned char      error : 1;  // First item after recovery from error
      unsigned char    delayed : 1;  // Item has a delayed timestamps (e.g. ITM trace)
      unsigned char   complete : 1;  // Item is complete (e.g. for Event Recorder Event)
      unsigned char    isStart : 1;  // First package after run
      unsigned char     isStop : 1;  // Last package before stop
      unsigned char   reserved : 2;  // reserved
    };
  };
} SA_DATA_HEADER;
// sizeof(SA_DATA_HEADER) == 13 Bytes


typedef struct SA_DataBlock_t {
  SA_DATA_HEADER     header;  // Fixed data header for all SA_DATAxxx types

  unsigned long       count;  // Byte count in datablock 
  unsigned char*     pValue;  // Raw data value
} SA_DATABLOCK;

typedef struct SA_Data1_t {
  SA_DATA_HEADER  header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
  } value;  // 2 Byte channel data

} SA_DATA1;



// System Analyzer - Data (2 Bytes)

typedef struct SA_Data2_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    unsigned char     ucArr[2];
  } value;  // 2 Byte channel data

} SA_DATA2;



// System Analyzer - Data (3 Bytes)

typedef struct SA_Data3_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    unsigned char     ucArr[3];
  } value;  // 3 Byte channel data

} SA_DATA3;



// System Analyzer - Data (4 Bytes)

typedef struct SA_Data4_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    signed int             i32;
    unsigned int           u32;
    float                    f;
    unsigned char     ucArr[4];
    unsigned short    usArr[2];
  } value;  // 4 Byte channel data

} SA_DATA4;



// System Analyzer - Data (5 Bytes)

typedef struct SA_Data5_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    signed int             i32;
    unsigned int           u32;
    float                    f;
    unsigned char     ucArr[5];
    unsigned short    usArr[2];
  } value;  // 5 Byte channel data

} SA_DATA5;



// System Analyzer - Data (6 Bytes)

typedef struct SA_Data6_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    signed int             i32;
    unsigned int           u32;
    float                    f;
    unsigned char     ucArr[6];
    unsigned short    usArr[3];
  } value;  // 6 Byte channel data

} SA_DATA6;

// System Analyzer - Data (6 Bytes)

typedef struct SA_Data7_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    signed int             i32;
    unsigned int           u32;
    float                    f;
    unsigned char     ucArr[7];
    unsigned short    usArr[3];
  } value;  // 7 Byte channel data

} SA_DATA7;



// System Analyzer - Data (8 Bytes)

typedef struct SA_Data8_t {
  SA_DATA_HEADER        header;  // Fixed data header for all SA_DATAxxx types

  union {
    signed char              c;
    unsigned char           uc;
    signed short           i16;
    unsigned short         u16;
    signed int             i32;
    unsigned int           u32;
    signed long long       i64;
    unsigned long long     u64;
    float                    f;
    double                   d;
    unsigned char     ucArr[8];
    unsigned short    usArr[4];
    unsigned int      ulArr[2];
    float              fArr[2];
  } value;  // 8 Byte channel data

} SA_DATA8;




// System Analyzer - Extended Data
//   (Data size critical, used to transport data between SARMxxx data storage and System Analyzer Window)

// Data Trace Details
typedef struct SA_Detail_DTrace_t {
  unsigned char   has_addr : 1;  // Data Address present
  unsigned char     has_pc : 1;  // PC present
  unsigned char   reserved : 6;  // reserved
  unsigned long        address;  // Data Address associated with data trace sample
  unsigned long             pc;  // PC associated with data trace sample
} SA_DETAIL_DTRACE;

// sizeof(SA_DETAIL_DTRACE) == 9 Bytes


// Event Record Details
typedef struct SA_Detail_EvtRec_t {   // TODO: Investigate if we can go through the Event Recorder window for decode
  unsigned char   length;             // Payload length in bytes
  unsigned long  *values;             // Value buffer
} SA_DETAIL_EVTREC;

// sizeof(SA_DETAIL_EVTREC) == 5 Bytes



// System Analyzer - Snapshot Request Structure
//  A snapshot is a stable, cached extract from the main channel buffers.
//  It is supposed to be used for updates while the CPU is running.
//  Snapshot data is allocated and owned by the caller.
//  Data and extended data are correlated by buffer position.
//  The interface is not thread-safe at the moment!


#define SA_SNAPSHOT_CYCLE_MAIN   0x0  // Request time information from main buffer
#define SA_SNAPSHOT_UPDATE       0x1  // Request update of snapshot buffer


typedef struct SA_SnapshotReq_t {
  unsigned long                cmd;  // Request type

  unsigned short           channel;  // Channel ID
                                     // - SA_SNAPSHOT_TIME_MAIN
                                     //   - [in] : Request time for 'channel' from main buffer, use 0xFFFFFFFF to request global min/max time
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [in] : Request update of snapshot buffer for 'channel'                

  unsigned char           addItems;  // Number of additional items that are added before and after the requested time range
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [in] Number of items that are added before minTime and after maxTime.
                                     //          minTime and maxTime are updated accordingly and can be outside the requested time range.
                                     //          Less items can be added if the requested number is not available before and/or after the requested time range.

  unsigned char           reserved;  // reserved for future use

  UINT64                   minTime;  // Smallest Timestamp
                                     // - SA_SNAPSHOT_TIME_MAIN
                                     //   - [out] : Smallest available time
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [out] : Smallest valid item timestamp in the updated snapshot buffer


  UINT64                   maxTime;  // Largest Timestamp
                                     // - SA_SNAPSHOT_TIME_MAIN
                                     //   - [out] : Largest available time
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [in]  : Largest timestamp to include in snapshot update
                                     //   - [out] : Largest timestamp in updated snapshot buffer


  UINT64          minAvailableTime;  // Minimum available time in the requested time range
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [out] : Smallest available timestamp in requested time range,
                                     //             it can be smaller than minTime when requested count is less than available count         

  unsigned long              count;  // Number of data items
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [in]  : Maximum number of requested items
                                     //   - [out] : Valid items in snapshot buffer 


  unsigned long             offset;  // Offset into buffer
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [out] : Offset of first valid data item in snapshot buffer

  void               *dataSnapshot;  // Pointer to data snapshot buffer  (!! DO NOT REQUEST A BUFFER UPDATE WHILE USING THIS POINTER !!)
                                     //   - [out] : Snapshot buffer for 'channel'

  void            *extDataSnapshot;  // Pointer to extended data snapshot buffer  (!! DO NOT REQUEST A BUFFER UPDATE WHILE USING THIS POINTER !!)
                                     // - SA_SNAPSHOT_UPDATE
                                     //   - [out] : Extended data snapshot buffer for 'channel'

  unsigned long      reserved2[32];  // reserved for future use
} SA_SNAPSHOT_REQ;

// sizeof(SA_SNAPSHOT_REQ) == 48 + 32*4 == 176 Bytes



typedef struct SA_AddData_t {
  unsigned short           channel;  // Channel ID

  unsigned long              count;  // Number of data items
                                     // - [in]  : Number of pushed items

  void                       *data;  // Pointer to data snapshot buffer
                                     // - [in] : Snapshot buffer for 'channel'

  void                    *extData;  // Pointer to extended data snapshot buffer
                                     // - [in] : Extended data snapshot buffer for 'channel'

  unsigned long       reserved[32];  // reserved for future use
} SA_ADD_DATA;


typedef struct SA_ScvdExtData_t {
  unsigned char           stateId;  // State Identifier
  unsigned char       reserved[3];  // 
  unsigned long         reserved2;  // reserved for future use
} SA_SCVD_EXT_DATA;

/*** System Analyzer - Request Structure for statistic information
 * SimFunc command: SMF_SA_STATISTIC_INFOS
 */
#define SA_STATISTIC_UPDATE   0x0  // Request statistic information from main buffer
#define SA_STATISTIC_RESET    0x1  // Reset statistic information from main buffer

typedef struct SA_StatisticValue {
  union {
    UINT64    u64;
    double      d;
  };
} SA_STATISTIC_VALUE;


typedef struct SA_Stat_Info {
  unsigned int                cnt;  // Number of items used to calculate statistic information
  unsigned int             errCnt;  // Number of timestamp errors found during capturing

  UINT32            bInteger :  1;  // value is UINT64
  UINT32                     : 31;

  UINT64                minCycles;  // first occurrance
  UINT64                maxCycles;  // last occurrance

  SA_STATISTIC_VALUE          min;
  SA_STATISTIC_VALUE          max;
  SA_STATISTIC_VALUE     totalSum;
} SA_STAT_INFO;


#define SA_STATINFO_CNT 256

typedef struct SA_StatisticInfo {
  unsigned int             subChId;  // Subchannel ID
  unsigned int          curStateId;  // current state id
  unsigned int                 cnt;  // overall items
  UINT64                lastCycles;  // last recorded cycles for subchannel
  UINT64                 minCycles;  // overall min Cycles
  UINT64                 maxCycles;  // overall max Cycles
  SA_STAT_INFO statInfo[SA_STATINFO_CNT];  // max. possible states
  unsigned long       reserved[16];   // reserved for future use
} SA_STATISTIC_INFO;


typedef struct SA_StatisticInfoReq {
  unsigned long                cmd;  // Request type

  unsigned short                id;  // Channel ID
                                     // - SA_STATISTICS_UPDATE
                                     //   - [in] : Request time for 'channel' from main buffer
                                     //            0xFFFF (all channels): not supported!
  unsigned short           subChId;  // SubChannel ID
                                     // - SA_STATISTIC_UPDATE
                                     //   - [in]  : Request subChannel ID from channel, use 0xFFFF to request all subChannels
                                     //   - [out] : number of subChannels (number of statistic items)
  unsigned long              count;  // Number of statistic infos
                                     // - [in]  : Number of allocated items
                                     // - [out] : Number of returned items
  SA_STATISTIC_INFO     *info[512];  // actual statistic values

  unsigned long      reserved2[32];  // reserved for future use
} SA_STATISTIC_INFO_REQ;


// System analyzer Error&Info Codes
#define SA_ERR_NOSUPP          0  // Unsupported
#define SA_ERR_OK              1  // Success
#define SA_ERR_CH_SUPP         2  // Unsupported channel error
#define SA_ERR_REQUEST         3  // Unsupported request error
#define SA_ERR_NOITEM          4  // No valid item error, e.g maxTime, la_signal_no
#define SA_ERR_INTERNAL        5  // System analyzer internal error, e.g null_ptr, buffer_size_inconsistence, memory allocation 
#define SA_ERR_CH_ENABLE       6  // Unenabled channel error
#define SA_ERR_RDDI            7  // RDDI error
#define SA_ERR_CONFIG          8  // Configuration error



/*
 * Data structures required for Power-Event-Statistics
 */

typedef struct pwrcapt  {
   UINT32              Num;       // Event sequence number

   UINT32            Sval0;       // Arguments of start event val1
   UINT32            Sval1;       // Arguments of start event val2

   UINT32            Eval0;       // Arguments of stop event val1
   UINT32            Eval1;       // Arguments of stop event val2

   UINT64            STime;       // time of start/stop event
   UINT64            ETime;       // time of start/stop event

   UINT64            Uones;       // number of ones for voltage
   UINT64            Iones;       // number of ones for current
   UINT64           Icount;       // current - number of bits counted
   UINT64           Ucount;       // voltage - number of bits counted

   UINT32      bStartV : 1;       // 0:=StartX(__FILE__,__LINE__), 1:=StartXv (val0,val1)
   UINT32       bStopV : 1;       // 0:=StopX(__FILE__,__LINE__),  1:=StopXv (val0,val1)
   UINT32              :30;       // unused - free for future enhancement(s)   
} PWRCAPT;


typedef struct pwrslot  {         // collection for one event-power sample
  UINT32          EvErrors;       // number of retriggers of Start without Stop event (displayed as errors value behind slot)
  UINT32             Count;       // number of start events

  UINT64         FirstTime;       // first time start generated
  UINT64          LastTime;       // last time start generated

  UINT64           MinTime;       // minimum execution time
  UINT64           MaxTime;       // maximum execution time
  UINT64           SumTime;       // total execution time

  PWRCAPT           MaxEEv;       // Event with maximum energy consumed
  PWRCAPT           MinEEv;       // Event with minimum energy consumed

  PWRCAPT           MaxTEv;       // Event with maximum time consumed
  PWRCAPT           MinTEv;       // Event with minimum time consumed

  UINT64         SumUCount;       // Sum of U bits
  UINT64          SumUOnes;       // Sum of U ones
  UINT64         SumICount;       // Sum of I bits
  UINT64          SumIOnes;       // Sum of I ones

  PWRCAPT           CurrEv;       // current power event start/stop item.
  UINT32      bStarted : 1;       // helper bit for Event-Recorder power sampling
  UINT32               :31;       // unused, free for future extensions
} PWRSLOT;



typedef struct pwstat  {          // collection of all event power channels
  PWRSLOT         GpA [16];       // StartA/Av (0...15)  __FILE__, __LINE__ / val0,val1
  PWRSLOT         GpB [16];       // StartB/Bv (0...15)  __FILE__, __LINE__ / val0,val1
  PWRSLOT         GpC [16];       // StartC/Cv (0...15)  __FILE__, __LINE__ / val0,val1
  PWRSLOT         GpD [16];       // StartD/Cv (0...15)  __FILE__, __LINE__ / val0,val1

  UINT32          nTotalEv;       // total event counter
  UINT32      bitCountErrs;       // 
  UINT32     onesCountErrs;       //
} PWSTAT;


/*
 * Inteface to calculate power data
 * SimFunc - case 'SMF_CALCPOWER_DATA'
 */

typedef struct powinterf  {
   UINT64            Uones;       // number of ones for voltage
   UINT64            Iones;       // number of ones for current
   UINT64           Icount;       // current - number of bits counted
   UINT64           Ucount;       // voltage - number of bits counted

   double            volts;
   double             amps;
   double           rPower;       // resulting power
   double             time;       // time range
   UINT32         nRes[14];       // not used, reserved for futuer extensions
} POWINTERF;

#define MAX_RAW_VAL      512.0
#define MIN_RAW_VAL    65024.0      // 65536 - 512
#define ZERO_RAW_VAL   32768.0



/*
 * Interface data to decode event numbers
 */

typedef enum evr_cmd  {
  EVR_DECODE_NAME           = 1,
  EVR_DECODE_STATENAME      = 2,
  EVR_DECODE_GETSTATEINFOS  = 3,
} EVR_CMD;


typedef struct SCVD_STATEINFO  {
  const char                   *name;   // name: Ready, Running, Blocked, ...
  SCVD_STATEDRAWINFO   drawStateInfo;   // drawing: Plot, Color, ...
  UINT32                reserved2[2];
} SCVD_STATEINFO;


typedef struct {
  EVR_CMD               cmd;   // In - EVR_DECODE_xxx etc.
  unsigned int          nEv;   // In - Event-identifer number
  unsigned long long   EvTs;   // In - Event-timestamp, currently unused
  unsigned int    EvVals[4];   // In - Event-values, currently unused

  char              *evName;   // Out - Event-name
  char              *evComp;   // Out - Event-component-brief / Event-component name
  char           szVal[256];   // Out - Event-value (currently unused)
  unsigned int       nLevel;   // Out - level - 0:=Op, 1:=API, 2:=Erorr, 3:=Detail
  SCVD_STATEINFO    *pState;
  unsigned int     nRes[14];   // reserved for future extnsions.
} EVR_DECODE;


// Example usage:
//
//  EVR_DECODE evrDecode;
//
//  memset (&evrDecode, 0, sizeof (EVR_DECODE));  // clear first !
//  evrDecode.cmd = EVR_DECODE_NAME;              // command
//  evrDecode.nEv = 0xF203;                       // 'ThreadCreated'
//  int nR = pio->Notify (UV_DECODE_EVITEM, &evrDecode);
//   if (nR == 1)  {                     // Ok.
//    // ...
//  }
//  else  {                             // failed.
//    // ...
//  }



#pragma pack()



#ifdef __cplusplus
  }                           // End of 'extern "C"'
#endif

#endif
