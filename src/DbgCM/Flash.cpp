/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 * 
 * @version  V1.1.8
 * @date     $Date: 2019-09-11 16:38:38 +0200 (Wed, 11 Sep 2019) $
 *
 * @note
 * Copyright (C) 2009-2016, 2019 ARM Limited. All rights reserved.
 *
 * @brief     Does the Flash Download, (Chip) Erase and Verify.
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
#include "..\BOM.h"
#include "..\ComTyp.h"
#include "Collect.h"
#include "ELF.h"
#include "Flash.h"
#include "Debug.h"

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_RECOVERY
#include "DebugAccess.h"
#endif // DBGCM_RECOVERY

#if DBGCM_DS_MONITOR
#include "DSMonitor.h"
#endif // 


static const char ErrTitle[] = "Debugger - Cortex-M Error";

struct FlashDevice    FlashDev;    // Flash Device Description Info
struct FlashAlgorithm FlashAlg;    // Flash Device Algorithm Info

static int   SelAlg;               // Selected Algorithm
static int   TimeOut;              // TimeOut Expired
static int   TimeOutTick = 0;      // TimeOut Tick for Progress Bar

static int   Silent = 0;           // Silent Mode without some Warnings

static DWORD Counter;              // Byte Counter
static DWORD TotalCount;           // Total Byte Counter

static int   SecCount;             // Number of Sectors (from FlashDevice)
static int   TotalSec;             // Total Number of Sectors in Device

static DWORD ESecAddr;             // Last Erased Sector Address
static DWORD ESecSize;             // Last Erased Sector Size

static DWORD PageAddr;             // Pending Program Page Address
static DWORD PageSize;             // Pending Program Page Size
static DWORD PageItem;             // Pending Program Page Item Count

static DWORD NextAddr;             // Next Address with known Algorithm

static BOOL  ExeError;             // System Call Execution Error

static BYTE  Buffer[0x10000];

//================================================================

static OIL  oil;                   // Progress-Bar data

static void InitProgress (char *pText)  {
  oil.Job   = PROGRESS_INITTXT;
  oil.pos   = 0;
  oil.low   = 0;
  oil.hig   = 100;
  oil.label = (signed char *) pText;
  pCbFunc (AG_CB_PROGRESS, &oil);
}
static void StopProgress (void)  {
  oil.Job = PROGRESS_SETPOS;
  oil.pos = 100;
  pCbFunc (AG_CB_PROGRESS, &oil);
  oil.Job = PROGRESS_KILL;
  pCbFunc (AG_CB_PROGRESS, &oil);
}
static void UpdatePos (int nPercent)  {
  oil.Job = PROGRESS_SETPOS;
  oil.pos = (nPercent);
  pCbFunc (AG_CB_PROGRESS, &oil);
}
static void SetText (char *pText) {
  oil.Job = PROGRESS_SETTEXT;
  oil.ctext = (SC8 *) pText;
  pCbFunc (AG_CB_PROGRESS, &oil);
}

static void SetHex8 (DWORD val) {
  char buf[32];

  sprintf (buf, "%08XH", val);
  SetText(buf);
}

static void SetPercent (DWORD val) {
  char buf[32];

  sprintf (buf, "%d %%", val);
  SetText(buf);
}

//================================================================


/*
 *  Execute Function in Target
 *    Parameter:      timeout:  Timeout in ms
 *    Return Value:   0 - OK,  1 - Failed
 */

static int ExecuteFunction (unsigned long timeout) {
  DWORD tick1ms;
  DWORD tdisp;
  DWORD tick;
  DWORD rval;
  DWORD val;
  int   status;

  ExeError = TRUE;

  if (TimeOut) return (1);

  RegARM.xPSR = 0x01000000;             // xPSR: T = 1, ISR = 0
  RegARM.SB   = FlashAlg.rSB;           // SB: Static Base
  RegARM.SP   = FlashAlg.rSP;           // SP: Stack Pointer
  RegARM.LR   = FlashAlg.BreakPoint;    // LR: Exit Point

  status = SysCallExec (&RegARM);
  if (status) { OutError (status); return (1); }

  tdisp = 0;
  tick  = GetTickCount();
  do {

    pio->Notify (UV_DOXEVENTS, (void *) 1);   // no need to use more than one events iteraton
                                              // 11.09.2019: Moved up here to have at least one UV_DOXEVENTS
                                              //   per flash algorithm function call. Otherwise it could be often skipped
                                              //   for fast algorithm functions.
#if DBGCM_V8M
    status = ReadD32 (DBG_HCSR, &val, BLOCK_SECTYPE_ANY);
    if (status) { OutError (status); return (1); }
#else // DBGCM_V8M
    status = ReadD32 (DBG_HCSR, &val);
    if (status) { OutError (status); return (1); }
#endif // DBGCM_V8M

    if (val & S_HALT) {
      status = SysCallRes (&rval);
      if (status) { OutError (status); return (1); }
      ExeError = FALSE;
      return (rval);
    }
    tick1ms = GetTickCount() - tick;
    if ((tick1ms - tdisp) > 500) {
      if (TimeOutTick) {
        val = 100 * tick1ms / timeout;
        UpdatePos(val);
      }
      tdisp = tick1ms;
    }
    // pio->Notify (UV_DOXEVENTS, (void *) 50);  // 23.11.2009: prevent uV freeze
    // pio->Notify (UV_DOXEVENTS, (void *) 1);   // no need to use more than one events iteraton  /24.3.2015/
                                                 // 11.09.2019: Moved to beginning of loop
  } while (tick1ms < timeout);

  TimeOut = 1;
  AGDIMsgBox(hMfrm, "Flash Timeout. Reset the Target and try it again.", ErrTitle, MB_ICONERROR, IDOK);
  return (1);
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  RegARM.A1 = adr;                      // R0: Argument 1
  RegARM.A2 = clk;                      // R1: Argument 2
  RegARM.A3 = fnc;                      // R2: Argument 3
  RegARM.PC = FlashAlg.Init;            // PC: Entry Point

  return (ExecuteFunction(3000));
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {

  RegARM.A1 = fnc;                      // R0: Argument 1
  RegARM.PC = FlashAlg.UnInit;          // PC: Entry Point

  return (ExecuteFunction(3000));
}


/*
 *  Calculate CRC in Flash Memory
 *    Parameter:      crc:  Initial CRC Value
 *                    adr:  Block Start Address
 *                    sz:   Block Size
 *                    cpv:  CRC Polynom Value
 *    Return Value:   CRC Code (32-bit)
 */

unsigned int CalculateCRC (unsigned long crc,
                           unsigned long adr,
                           unsigned long sz,
                           unsigned long cpv)
{

  RegARM.A1 = crc;                      // R0: Argument 1
  RegARM.A2 = adr;                      // R1: Argument 2
  RegARM.A3 = sz;                       // R2: Argument 3
  RegARM.A4 = cpv;                      // R3: Argument 4
  RegARM.PC = FlashAlg.CalculateCRC;    // PC: Entry Point

  return (ExecuteFunction(10000));
}


/*
 *  Blank Check Flash Memory
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {

  RegARM.A1 = adr;                      // R0: Argument 1
  RegARM.A2 = sz;                       // R1: Argument 2
  RegARM.A3 = pat;                      // R2: Argument 3
  RegARM.PC = FlashAlg.BlankCheck;      // PC: Entry Point

  return (ExecuteFunction(FlashDev.toErase));
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {
  int  ret;

  RegARM.PC = FlashAlg.EraseChip;       // PC: Entry Point

  TimeOutTick = 1;
  ret = ExecuteFunction(TotalSec*FlashDev.toErase);
  TimeOutTick = 0;
  return (ret);
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  RegARM.A1 = adr;                      // R0: Argument 1
  RegARM.PC = FlashAlg.EraseSector;     // PC: Entry Point

  return (ExecuteFunction(FlashDev.toErase));
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long ba, cnt, n;
  int           status;
  DWORD         rwpage;

  ba  = FlashAlg.PrgBuf;
  cnt = (PageSize + 3) & ~0x00000003;

  rwpage = AP_CurrentRWPage();                       // Get effective RWPage based on DP/AP selection

  while (cnt >= 4) {
    // n = RWPage - (ba & (RWPage - 1));
    n = rwpage - (ba & (rwpage - 1));
    if (cnt < n) n = cnt & 0xFFFFFFFC;

#if DBGCM_V8M
    status = WriteBlock(ba, buf, n, BLOCK_SECTYPE_ANY);
    if (status) { OutError (status); return (1); }
#else // DBGCM_V8M
    status = WriteBlock(ba, buf, n, 0 /*attrib*/);
    if (status) { OutError (status); return (1); }
#endif // DBGCM_V8M

    ba  += n;
    buf += n;
    cnt -= n;
  }

  RegARM.A1 = adr;                      // R0: Argument 1
  RegARM.A2 = sz;                       // R1: Argument 2
  RegARM.A3 = FlashAlg.PrgBuf;          // R2: Argument 3
  RegARM.PC = FlashAlg.ProgramPage;     // PC: Entry Point

  return (ExecuteFunction(FlashDev.toProg));
}


/*
 *  Verify Flash Memory
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size
 *                    buf:  Block Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long ba, cnt, n;
  int           status;
  DWORD         rwpage;

  ba  = FlashAlg.PrgBuf;
  cnt = (PageSize + 3) & ~0x00000003;
  rwpage = AP_CurrentRWPage();                       // Get effective RWPage based on DP/AP selection
  while (cnt >= 4) {
    // n = RWPage - (ba & (RWPage - 1));
    n = rwpage - (ba & (rwpage - 1));
    if (cnt < n) n = cnt & 0xFFFFFFFC;

#if DBGCM_V8M
    status = WriteBlock(ba, buf, n, BLOCK_SECTYPE_ANY);
    if (status) { OutError (status); return (1); }
#else // DBGCM_V8M
    status = WriteBlock(ba, buf, n, 0 /*attrib*/);
    if (status) { OutError (status); return (1); }
#endif // DBGCM_V8M

    ba  += n;
    buf += n;
    cnt -= n;
  }

  RegARM.A1 = adr;                      // R0: Argument 1
  RegARM.A2 = sz;                       // R1: Argument 2
  RegARM.A3 = FlashAlg.PrgBuf;          // R2: Argument 3
  RegARM.PC = FlashAlg.Verify;          // PC: Entry Point

  return (ExecuteFunction(FlashDev.toProg));
}


/******************************************************************************/



// ARM SW Break Point Code - Thumb Mode (BKPT 0)
static const BYTE BreakPoint_ARM[] = {
  0x00, 0xBE
};



/*
 * ARM Blank Check Function (Standalone Function)
 */

// C Code
/*
int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {

  while (sz--) {
    if (*((volatile unsigned char *)adr++) != pat) return (1);
  }

  return (0);
}
*/

// Thumb Assembly Code
/*
000000  e005              B        check
000002  7803      loop:   LDRB     r3,[r0,#0]
000004  1c40              ADDS     r0,r0,#1
000006  4293              CMP      r3,r2
000008  d001              BEQ      check
00000a  2001              MOVS     r0,#1
00000c  4770              BX       lr
00000e  1e49      check:  SUBS     r1,r1,#1
000010  d2f7              BCS      loop
000012  2000              MOVS     r0,#0
000014  4770              BX       lr
*/

// ARM - Thumb Blank Check Function
static const BYTE BlankCheck_ARM[] = {
  0x05, 0xE0, 0x03, 0x78, 0x40, 0x1C, 0x93, 0x42,
  0x01, 0xD0, 0x01, 0x20, 0x70, 0x47, 0x49, 0x1E,
  0xF7, 0xD2, 0x00, 0x20, 0x70, 0x47
};



/*
 * ARM CRC Algorithm (Standalone Algorithm)
 */

// C Code
/*
unsigned int CalculateCRC (unsigned long crc,
                           unsigned long adr,
                           unsigned long sz,
                           unsigned long cpv)
{
  register unsigned int i;

  while (sz) {
    crc ^= *((volatile unsigned char *)adr) << 24;
    for (i = 8; i != 0; i--) {
      if (crc & 0x80000000) {
        crc <<= 1;
        crc  ^= cpv;
      } else {
        crc <<= 1;
      }
    }
    adr++;
    sz--;
  }
  return (crc);
}
*/

// Thumb Assembly Code
/*
000000  E00A              B        check
000002  780D      wloop:  LDRB     r5,[r1,#0]
000004  062D              LSLS     r5,r5,#24
000006  4068              EORS     r0,r0,r5
000008  2408              MOVS     r4,#8
00000A  0040      floop:  LSLS     r0,r0,#1
00000C  D300              BCC      next
00000E  4058              EORS     r0,r0,r3
000010  1E64      next:   SUBS     r4,r4,#1
000012  D1FA              BNE      floop
000014  1C49              ADDS     r1,r1,#1
000016  1E52              SUBS     r2,r2,#1
000018  2A00      check:  CMP      r2,#0
00001A  D1F2              BNE      wloop
00001C  4770              BX       lr
*/

// ARM - Thumb CRC Algorithm
static const BYTE CRC32_ARM[] = {
  0x0A, 0xE0, 0x0D, 0x78, 0x2D, 0x06, 0x68, 0x40,
  0x08, 0x24, 0x40, 0x00, 0x00, 0xD3, 0x58, 0x40,
  0x64, 0x1E, 0xFA, 0xD1, 0x49, 0x1C, 0x52, 0x1E,
  0x00, 0x2A, 0xF2, 0xD1, 0x70, 0x47
};


#define CRC_Polynom 0x04C11DB7
#define CRC_InitVal 0xFFFFFFFF


/*
 * PC CRC Calculation
 *    Parameter:      dat:  Block Data
 *                    sz:   Block Size
 *    Return Value:   CRC Code (32-bit)
 */

static DWORD CRC32 (BYTE *dat, DWORD sz) {
  DWORD crc;
  DWORD i;

  crc = CRC_InitVal;
  while (sz) {
    crc ^= *dat++ << 24;
    for (i = 8; i != 0; i--) {
      if (crc & 0x80000000) {
        crc <<= 1;
        crc  ^= CRC_Polynom;
      }
      else {
        crc <<= 1;
      }
    }
    sz--;
  }
  return (crc);
}



/*
 *  Load Flash Device Description
 */

BOOL LoadFlashDevice (char *fname) {
  DWORD  adr, sz;
  DWORD     n, m;
  BOOL        ok;
  char       szP [MAX_PATH + 32];
  TpPATHEXP  FpP;

//---17.11.2012: expand path first, e.g. '$RTE_ROOT$.\Device\Flash\LPC_IAP_512.FLM'
  strcpy (szP, fname);  
  FpP = (TpPATHEXP) pio->SXX_PATHEXPAND;
  if (FpP != NULL)  {                     // path-expansion interface is present
    FpP (FPEXP_NORMAL, fname, szP);       // nCode, pIn, pOut
  }
//txtout ("---LoadFlashDevice (%s)\n", szP);
  Elf.fh = fopen (szP, "rb");             // (fname, "rb");
//-------------
//txtout ("--- %s.\n", Elf.fh == NULL ? "failed" : "passed");

  if (Elf.fh == NULL) return (FALSE);

  ElfInit();

  ok = ReadElf();

  if (ok) {
    // Search for FlashDevice structure
    ok = FALSE;
    for (n = 0; n < Elf.sym_cnt; n++) {
      if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "FlashDevice") == 0) {
        adr = Elf.sym[n].st_value;
        sz  = Elf.sym[n].st_size;
        ok = TRUE;
        break;
      }
    }
  }

  if (ok) {
    // Load the FlashDevice structure
    ok = FALSE;
    for (n = 0; n < Elf.ehdr.e_phnum; n++) {
      if ((Elf.phdr[n].p_type   == PT_LOAD) &&
          (Elf.phdr[n].p_vaddr  == adr) &&
          (Elf.phdr[n].p_filesz == sz))
      {
        if (fseek(Elf.fh, Elf.phdr[n].p_offset, SEEK_SET)) break;
        if (fread(&FlashDev, 1, sizeof(struct FlashDevice), Elf.fh) == sz) {
          if (Elf.ehdr.e_ident[EI_DATA] == ELFDATA2MSB) {   // Big-Endian
            FlashDev.Vers    = Swap16(FlashDev.Vers);
            FlashDev.DevType = Swap16(FlashDev.DevType);
            FlashDev.DevAdr  = Swap32(FlashDev.DevAdr);
            FlashDev.szDev   = Swap32(FlashDev.szDev);
            FlashDev.szPage  = Swap32(FlashDev.szPage);
            FlashDev.Res     = Swap32(FlashDev.Res);
            FlashDev.toProg  = Swap32(FlashDev.toProg);
            FlashDev.toErase = Swap32(FlashDev.toErase);
            for (m = 0; m < SECTOR_NUM; m++) {
              FlashDev.sectors[m].szSector   = Swap32(FlashDev.sectors[m].szSector);
              FlashDev.sectors[m].AddrSector = Swap32(FlashDev.sectors[m].AddrSector);
            }
          }
          if (FlashDev.toProg  == 0) FlashDev.toProg  = 1;
          if (FlashDev.toErase == 0) FlashDev.toErase = 1;
          ok = TRUE;
          break;
        }
      }
    }
  }

  ElfUnInit();

  fclose(Elf.fh);

  return (ok);
}


/*
 *  Load Flash Device Algorithm
 */

BOOL LoadFlashAlgorithm (char *fname) {
  DWORD  adr, ofs;
  DWORD  sz, szb, szm;
  DWORD  n, m, i;
  BYTE   buf[0x10000];
  int    status;
  BOOL       ok;
  char       szP [MAX_PATH + 32];
  TpPATHEXP  FpP;

//---21.2.2013: expand path first, e.g. '$RTE_ROOT$.\Device\Flash\LPC_IAP_512.FLM'
  strcpy (szP, fname);  
  FpP = (TpPATHEXP) pio->SXX_PATHEXPAND;
  if (FpP != NULL)  {                     // path-expansion interface is present
    FpP (FPEXP_NORMAL, fname, szP);       // nCode, pIn, pOut
  }
//txtout ("---LoadFlashDevice (%s)\n", szP);
  Elf.fh = fopen (szP, "rb");             // (fname, "rb");
//-------------
//txtout ("--- %s.\n", Elf.fh == NULL ? "failed" : "passed");

  if (Elf.fh == NULL) return (FALSE);
  ElfInit();

  ok = ReadElf();

  memset(&FlashAlg, 0xFF, sizeof(struct FlashAlgorithm));
  FlashAlg.StackSize = 64; // Default Stack size

  if (ok) {

    // Search for Programming Algorithms
    for (n = 0; n < Elf.sym_cnt; n++) {
      if      (strcmp(&Elf.strtab[Elf.sym[n].st_name], "STACK_SIZE")   == 0) FlashAlg.StackSize    = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "BreakPoint")   == 0) FlashAlg.BreakPoint   = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "Init")         == 0) FlashAlg.Init         = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "UnInit")       == 0) FlashAlg.UnInit       = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "CalculateCRC") == 0) FlashAlg.CalculateCRC = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "CheckPattern") == 0) FlashAlg.BlankCheck   = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "BlankCheck")   == 0) FlashAlg.BlankCheck   = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "EraseChip")    == 0) FlashAlg.EraseChip    = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "EraseSector")  == 0) FlashAlg.EraseSector  = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "ProgramPage")  == 0) FlashAlg.ProgramPage  = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "Verify")       == 0) FlashAlg.Verify       = Elf.sym[n].st_value;
      else if (strcmp(&Elf.strtab[Elf.sym[n].st_name], "FlashDevice")  == 0) adr                   = Elf.sym[n].st_value;
    }

    // Search for .data or .bss section for Static Base offset
    ok = FALSE;
    for (n = 0; n < Elf.ehdr.e_shnum; n++)  {
      if (Elf.shdr[n].sh_flags == (SHF_WRITE | SHF_ALLOC)) {
        if ((Elf.shdr[n].sh_type == SHT_PROGBITS) ||
            (Elf.shdr[n].sh_type == SHT_NOBITS)) {
          FlashAlg.rSB = Elf.shdr[n].sh_addr;
          ok = TRUE;
          break;
        }
      }
    }

    if (ok) {
      // Check if minimum of necessary Algorithms are found
      if ((FlashAlg.Init         == 0xFFFFFFFF) ||
          (FlashAlg.UnInit       == 0xFFFFFFFF) ||
//        (FlashAlg.EraseChip    == 0xFFFFFFFF) ||
          (FlashAlg.EraseSector  == 0xFFFFFFFF) ||
          (FlashAlg.ProgramPage  == 0xFFFFFFFF)) {
        ok = FALSE;
        txtout("Missing Flash Algorithms !");
      }
    }

    if (ok) {
      // Check if Functions are coded in Thumb
      if (!(FlashAlg.BreakPoint   & 1) ||
          !(FlashAlg.Init         & 1) ||
          !(FlashAlg.UnInit       & 1) ||
          !(FlashAlg.CalculateCRC & 1) ||
          !(FlashAlg.BlankCheck   & 1) ||
          !(FlashAlg.EraseChip    & 1) ||
          !(FlashAlg.EraseSector  & 1) ||
          !(FlashAlg.ProgramPage  & 1) ||
          !(FlashAlg.Verify       & 1)) {
        ok = FALSE;
        txtout("Flash Algorithms not in Thumb Code !");
      }
    }

    if (ok) {

      // Check Internal Algorithms and Calculate External Algorithms Offset
      ofs = 0;
      if (FlashAlg.BreakPoint   == 0xFFFFFFFF) ofs += sizeof(BreakPoint_ARM);
      if (FlashAlg.CalculateCRC == 0xFFFFFFFF) ofs += sizeof(CRC32_ARM);
      if (FlashAlg.BlankCheck   == 0xFFFFFFFF) ofs += sizeof(BlankCheck_ARM);
      ofs = (ofs + 3) & ~0x00000003;

      // Relocate Programming Algorithms
      n = 0;
      if (FlashAlg.BreakPoint == 0xFFFFFFFF) {
        FlashAlg.BreakPoint    = FlashConf.RAMStart + n;
        FlashAlg.BreakPoint   |= 1;
        memcpy(Buffer + n, BreakPoint_ARM, sizeof(BreakPoint_ARM));
        n += sizeof(BreakPoint_ARM);
      } else {
        FlashAlg.BreakPoint   += FlashConf.RAMStart + ofs;
      }
      if (FlashAlg.CalculateCRC == 0xFFFFFFFF) {
        FlashAlg.CalculateCRC  = FlashConf.RAMStart + n;
        FlashAlg.CalculateCRC |= 1;
        memcpy(Buffer + n, CRC32_ARM, sizeof(CRC32_ARM));
        n += sizeof(CRC32_ARM);
      } else {
        FlashAlg.CalculateCRC += FlashConf.RAMStart + ofs;
      }
      if (FlashAlg.BlankCheck == 0xFFFFFFFF) {
        FlashAlg.BlankCheck    = FlashConf.RAMStart + n;
        FlashAlg.BlankCheck   |= 1;
        memcpy(Buffer + n, BlankCheck_ARM, sizeof(BlankCheck_ARM));
        n += sizeof(BlankCheck_ARM);
      } else {
        FlashAlg.BlankCheck   += FlashConf.RAMStart + ofs;
      }
      FlashAlg.Init           += FlashConf.RAMStart + ofs;
      FlashAlg.UnInit         += FlashConf.RAMStart + ofs;
      if (FlashAlg.EraseChip  != 0xFFFFFFFF) {
        FlashAlg.EraseChip    += FlashConf.RAMStart + ofs;
      }
      FlashAlg.EraseSector    += FlashConf.RAMStart + ofs;
      FlashAlg.ProgramPage    += FlashConf.RAMStart + ofs;
      if (FlashAlg.Verify     != 0xFFFFFFFF) {
        FlashAlg.Verify       += FlashConf.RAMStart + ofs;
      }
      FlashAlg.rSB            += FlashConf.RAMStart + ofs;
      FlashAlg.rSP             = FlashConf.RAMStart + FlashConf.RAMSize;
      FlashAlg.PrgBuf          = FlashConf.RAMStart;

      FlashAlg.StackSize = (FlashAlg.StackSize + 7) & ~0x00000007;
      if (FlashAlg.StackSize < 32) FlashAlg.StackSize = 32;

      // Load Programming Algorithms
      ok = FALSE;
      for (n = 0; n < Elf.ehdr.e_phnum; n++) {
        if ((Elf.phdr[n].p_type  == PT_LOAD) &&
            (Elf.phdr[n].p_vaddr != adr))
        {
          sz  = Elf.phdr[n].p_filesz;
          szm = Elf.phdr[n].p_memsz + ofs;
          szm = (szm + 3) & ~0x00000003;
          szb = sz + ofs;

          if ((szm + FlashDev.szPage + FlashAlg.StackSize) > FlashConf.RAMSize) {
            txtout("Insufficient RAM for Flash Algorithms !");
            break;
          }

          FlashAlg.PrgBuf += szm;    // Append Buffer at end of Algorithm

          // Read Flash Algoritm from ELF file
          if (fseek(Elf.fh, Elf.phdr[n].p_offset, SEEK_SET)) break;
          if (fread(Buffer + ofs, 1, sz, Elf.fh) != sz) break;

          status = 0;


#if DBGCM_V8M
          // Write the Flash Algorithm to RAM
          i   = 0;
          m   = szb;
          adr = FlashConf.RAMStart;
          status = WriteARMMem (&adr, Buffer, m, BLOCK_SECTYPE_ANY);
          if (status) { OutError (status); break; }

          // Check Downloaded Algorithm
          i   = 0;
          m   = szb;
          adr = FlashConf.RAMStart;
          status = ReadARMMem (&adr, buf, m, BLOCK_SECTYPE_ANY);
          if (status) { OutError (status); break; }
          if (memcmp(Buffer, buf, szb)) {
            txtout("Cannot Write to RAM for Flash Algorithms !");
            break;
          }
#else // DBGCM_V8M
          // Write the Flash Algorithm to RAM
          i   = 0;
          m   = szb;
          adr = FlashConf.RAMStart;
          status = WriteARMMem (&adr, Buffer, m);
          if (status) { OutError (status); break; }

          // Check Downloaded Algorithm
          i   = 0;
          m   = szb;
          adr = FlashConf.RAMStart;
          status = ReadARMMem (&adr, buf, m);
          if (status) { OutError (status); break; }
          if (memcmp(Buffer, buf, szb)) {
            txtout("Cannot Write to RAM for Flash Algorithms !");
            break;
          }
#endif // DBGCM_V8M

          ok = TRUE;
          break;
        }
      }
    }

  }

  ElfUnInit();

  fclose(Elf.fh);

  return (ok);
}


static void ComposeFlashDevAlgString (char* szFP) {
  char    buf[512], *pFname;
  TpPATHEXP  FpP;

  pFname = NULL;
  if (FlashConf.Dev[SelAlg].fPath[0] != 0)  {    // fullpath .flm file is specified via RTE-package
    strcpy (buf, FlashConf.Dev[SelAlg].fPath);
    pFname = FlashConf.Dev[SelAlg].fPath;

    FpP = (TpPATHEXP) pio->SXX_PATHEXPAND;       // expand .flm filename path  /21.2.2013/
    if (FpP != NULL)  {                          // path-expansion interface is present
      FpP (FPEXP_NORMAL, pFname, buf);           // nCode, pIn, pOut
      pFname = &buf[0];
    }
  }
  else  {                                        // not a RTE-algo: use traditional path for .flm file
    strcpy (buf, MonConf.DriverPath);
    strcat (buf, "..\\flash\\");
    strcat (buf, FlashConf.Dev[SelAlg].FileName);
    strcat (buf, ".flm");
    pFname = &buf[0];
  }
  PathCanonicalize (szFP, pFname);               // resolve relative parts of path, ..\..\ etc.
}


static const char *SectorError[] = {
  "None!",
  "Zero Size!",
  "Address not Zero!",
  "Out of Range!",
  "Not Consistent!"
};

static DWORD oldadr, oldsz;

static int SectorCheck (DWORD item, DWORD adr, DWORD sz) {
  char  buf[512];
  char  szFP[MAX_PATH + 32];
  int   error;

  // Check Size
  if (sz == 0) {
    error = 1;
    goto err;
  }

  // Check Address
  if ((item == 0) && (adr != 0)) {
    error = 2;
    goto err;
  }

  // Check Range
  if ((adr + sz) > FlashDev.szDev) {
    error = 3;
    goto err;
  }

  // Check Consistency
  if ((item != 0) && ((adr < (oldadr + oldsz)) || ((adr - oldadr) % oldsz))) {
    error = 4;
    goto err;
  }

  oldadr = adr;
  oldsz  = sz;

  return (0);

err:
  ComposeFlashDevAlgString(szFP);
  sprintf(buf, "Sector (%d): %s\n\n%s", item, SectorError[error], szFP);
  AGDIMsgBox (hMfrm, buf, ErrTitle, MB_ICONERROR, IDOK);
  return (error);
}



/*
 *  Find Flash Device Algorithm
 *    Parameter:      nAdr:  Address
 *                    nMany: Number of Bytes
 *    Return Value:   Index of found Algorithm (when >= 0)
 *                    -1: None, No more Algorithms
 *                    -2: None, Next Algorithm at NextAddr
 *                    -3: Error, Overlapping of Algorithms
 */

static int FindFlashAlgorithm (DWORD nAdr, DWORD nMany) {
  char  buf[128];
  int   count, index, i;

  if (SelAlg != -1) {
    if ((nAdr >= FlashConf.Dev[SelAlg].Start) &&
        (nAdr < (FlashConf.Dev[SelAlg].Start + FlashConf.Dev[SelAlg].Size))) {
      return (SelAlg);
    }
  }

  NextAddr = 0xFFFFFFFF;

  count = 0;
  index = -1;
  for (i = 0; i < FlashConf.Nitems; i++) {
    if ((nAdr >= FlashConf.Dev[i].Start) &&
        (nAdr < (FlashConf.Dev[i].Start + FlashConf.Dev[i].Size))) {
      index = i;
      count++;
      continue;
    }
    if ((index < 0) && (nAdr < FlashConf.Dev[i].Start)) {
      if (FlashConf.Dev[i].Start < NextAddr) NextAddr = FlashConf.Dev[i].Start;
      index = -2;
    }
  }

  if (count == 1) return (index);
  if (count > 1) {
    if (Silent) return (-3);
    sprintf(buf, "Overlapping of Algorithms at Address %08XH.", nAdr);
    AGDIMsgBox(hMfrm, buf, ErrTitle, MB_ICONERROR, IDOK);
    return (-3);
  }
  else {
    if (Silent) return (index);
    if ((index == -2) && (nMany > (NextAddr - nAdr))) nMany = NextAddr - nAdr;
    sprintf(buf, "No Algorithm found for: %08XH - %08XH", nAdr, nAdr + nMany - 1);
    txtout (buf);
//  AGDIMsgBox(hMfrm, buf, ErrTitle, MB_ICONERROR);
    return (index);
  }
}


/*
 *  Load Flash Device Description & Algorithm
 *    Return Value:   0 - OK,  (-1) - Failed
 */

static int LoadFlashDevAlg (void) {
  DWORD   adr, sz;
  char    buf[512];
  char   szFP[MAX_PATH + 32];
  int          i;

  ComposeFlashDevAlgString(szFP);

  if (LoadFlashDevice (szFP) == FALSE)  {
    sprintf (buf, "Cannot Load Flash Device Description!\n\n%s", szFP);
    AGDIMsgBox (hMfrm, buf, ErrTitle, MB_ICONERROR, IDOK);
    return (-1);
  }
  if (LoadFlashAlgorithm (szFP) == FALSE)  {
    sprintf (buf, "Cannot Load Flash Programming Algorithm!\n\n%s", szFP);
    AGDIMsgBox (hMfrm, buf, ErrTitle, MB_ICONERROR, IDOK);
    return (-1);
  }
  if (FlashDev.szDev == 0)  {
    sprintf (buf, "Zero Device Size!\n\n%s", szFP);
    AGDIMsgBox (hMfrm, buf, ErrTitle, MB_ICONERROR, IDOK);
    return (-1);
  }

#if 0         // 7.11.2012
  if (LoadFlashDevice (buf) == FALSE)  {
    sprintf(buf, "%s: %s.FLM", ErrTitle, FlashConf.Dev[SelAlg].FileName);
    AGDIMsgBox(hMfrm, "Cannot Load Flash Device Description !", buf, MB_ICONERROR, IDOK);
    return (-1);
  }
  if (LoadFlashAlgorithm(buf) == FALSE) {
    sprintf(buf, "%s: %s.FLM", ErrTitle, FlashConf.Dev[SelAlg].FileName);
    AGDIMsgBox(hMfrm, "Cannot Load Flash Programming Algorithm !", buf, MB_ICONERROR, IDOK);
    return (-1);
  }

  if (FlashDev.szDev == 0) {
    sprintf(buf, "%s: %s.FLM", ErrTitle, FlashConf.Dev[SelAlg].FileName);
    AGDIMsgBox(hMfrm, "Zero Device Size !", buf, MB_ICONERROR, IDOK);
    return (-1);
  }
#endif

  // Check Page Size and correct if necessary
  /* Page Size is not limited to 2^x anymore
  for (i = 31; i >= 0; i--) {
    if (FlashDev.szPage & (1 << i)) {
      FlashDev.szPage = (1 << i);
      break;
    }
  }
  */
  if (FlashDev.szPage  > PAGE_MAX) FlashDev.szPage = PAGE_MAX;
  if (FlashDev.szPage == 0)        FlashDev.szPage = 1;

  // Search and Check Flash Sectors
  SecCount = 0;
  TotalSec = 0;
  for (i = 0; i < SECTOR_NUM; i++) {
    adr = FlashDev.sectors[i].AddrSector;
    sz  = FlashDev.sectors[i].szSector;
    if ((adr == 0xFFFFFFFF) && (sz == 0xFFFFFFFF)) {
      SecCount  = i;
      TotalSec += (FlashDev.szDev - oldadr) / oldsz;
      break;
    }
    if (i != 0) {
      TotalSec += (adr - oldadr) / oldsz;
    }
    if (SectorCheck(i, adr, sz)) return (-1);
  }
  if (SecCount == 0)  {
    sprintf (buf, "Missing Flash Sectors Description!\n\n%s", szFP);
    AGDIMsgBox (hMfrm, buf, ErrTitle, MB_ICONERROR, IDOK);
    return (-1);
  }

  memset(&RegARM, 0, sizeof(RegARM));   // Clear Initial ARM Registers

  ESecAddr = 0;
  ESecSize = 0;
  PageAddr = 0;
  PageSize = FlashDev.szPage;
  PageItem = 0;

  return (0);
}



/*
 *  Erase Flash Memory
 *    Parameter:      nAdr:  Address
 *                    nMany: Size
 *    Return Value:   0 - OK,  1 - Failed
 */

static int EraseFlash (DWORD nAdr, DWORD nMany) {
  DWORD adr, sz, n;
  int   i;

  while (nMany) {

    // Check for Last Erased Sector
    if ((nAdr >= ESecAddr) && (nAdr < (ESecAddr + ESecSize))) {
      n = (ESecAddr + ESecSize) - nAdr;
      if (nMany > n) {
        nAdr  += n;
        nMany -= n;
        Counter += n;
        UpdatePos(100*Counter/TotalCount);
      }
      else {
        Counter += nMany;
        UpdatePos(100*Counter/TotalCount);
        return (0);                    // Previously Erased
      }
    }

    // Search for Sector to be erased
    for (i = 0; i < SecCount; i++) {
      adr = FlashDev.sectors[i].AddrSector + FlashConf.Dev[SelAlg].Start;
      sz  = FlashDev.sectors[i].szSector;
      if (nAdr < adr) {
        if (i != 0) {
          n   = oldadr + oldsz;
          sz  = oldsz;
          adr = n + sz*((nAdr - n) / sz);
          break;
        }
      }
      else if (nAdr >= (adr + sz)) {
        if ((i + 1) == SecCount) {
          n   = adr + sz;
          adr = n + sz*((nAdr - n) / sz);
          break;
        }
      }
      else break;
      oldadr = adr;
      oldsz  = sz;
    }

    // Erase Sector (if not already erased)
    SetHex8(adr);
    if (BlankCheck(adr, sz, FlashDev.valEmpty)) {
      if (EraseSector(adr)) return (1);
    }
    ESecAddr = adr;
    ESecSize = sz;

  }

  return (0);
}


/*
 *  Erase Entire Flash Memory (which is needed)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int EraseAll (void) {
  FLASHPARM *pF;
  DWORD      n;
  int        alg;
  int        flg = 0;

  InitProgress ("Erase: ");
  UpdatePos(0);

  SelAlg = -1;
  TimeOut = 0;
  Counter = 0;
  pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, NULL);  // get parameters, NOTE: first call with NULL !
  if (pF->ActSize == 0) pF->many = 0;
  TotalCount = pF->ActSize;
  while (pF->many) {
    alg = FindFlashAlgorithm(pF->start, pF->many);
    if (alg == -3) break;
    if (alg  <  0) {
      n = NextAddr - pF->start;
      if ((alg == -2) && (pF->many > n)) {
        pF->start += n;
        pF->many  -= n;
      }
      else {
        pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, pF);
      }
      flg |= 1;
      continue;
    }
    flg |= 2;
    if (alg != SelAlg) {
      if (SelAlg != -1) {
        if (UnInit(1)) { SelAlg = -1; break; }
      }
      SelAlg = alg;
      if (LoadFlashDevAlg())  { SelAlg = -1; break; }
      if (Init(FlashConf.Dev[SelAlg].Start, pdbg->Clock, 1)) break;
    }
    n = (FlashConf.Dev[SelAlg].Start + FlashConf.Dev[SelAlg].Size) - pF->start;
    if (pF->many > n) {
      if (EraseFlash(pF->start, n)) break;
      pF->start += n;
      pF->many  -= n;
      continue;
    }
    else {
      if (EraseFlash(pF->start, pF->many)) break;
    }
    pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, pF);  // Note: use pF from first call, get next parameters
  }
  if (SelAlg != -1) {
    if (UnInit(1)) flg |= 4;
  }
  if (pF->many) flg |= 4;

  StopProgress();

  switch (flg) {
    case 0:
    case 1:
      txtout("Erase skipped!");
      return (1);
    case 2:
      txtout("Erase Done.");
      break;
    case 3:
      txtout("Partial Erase Done (areas with no algorithms skipped!)");
      break;
    default:
      txtout("Erase Failed!");
      return (1);
  }
  return (0);
}


/*
 *  Erase Full Flash Memory (all Chips)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int EraseFull (void) {
  int alg;

  SelAlg = -1;
  TimeOut = 0;

  for (alg = 0; alg < FlashConf.Nitems; alg++) {
    Counter = 0;
    TotalCount = FlashConf.Dev[alg].Size;
    if (TotalCount == 0) continue;
    InitProgress ("Full Chip Erase: ");
    UpdatePos(0);
    if (SelAlg != -1) {
      if (UnInit(1)) { SelAlg = -1; break; }
    }
    SelAlg = alg;
    if (LoadFlashDevAlg()) { SelAlg = -1; break; }
    if (Init(FlashConf.Dev[SelAlg].Start, pdbg->Clock, 1)) break;
    if (FlashAlg.EraseChip != 0xFFFFFFFF) {
      SetText(FlashConf.Dev[SelAlg].FileName);
      if (EraseChip()) break;
    } else {
      if (EraseFlash(FlashConf.Dev[SelAlg].Start, FlashConf.Dev[SelAlg].Size)) break;
    }
    StopProgress();
  }
  if (SelAlg != -1) {
    if (UnInit(1)) alg = -1;
  }

  if (alg == FlashConf.Nitems) {
    txtout("Full Chip Erase Done.");
    return (0);
  } else {
    StopProgress();
    txtout("Full Chip Erase Failed!");
    return (1);
  }
}



/*
 *  Write Flash Memory
 *    Parameter:      nAdr:  Address
 *                    nMany: Size
 *                    image: Data
 *    Return Value:   0 - OK,  1 - Failed
 */

static int WriteFlash (DWORD nAdr, DWORD nMany, BYTE *image) {
  DWORD n, m;
  DWORD i;

  // Check for Pending Program Page
  if (PageItem && ((nMany == 0) || (nAdr < PageAddr) || (nAdr >= (PageAddr + PageSize)))) {
    if (ProgramPage(PageAddr, PageItem, Buffer)) return (1);
    PageItem = 0;
  }

  while (nMany) {

    SetHex8(nAdr);

//  PageAddr = nAdr & ~(PageSize - 1);
    PageAddr = FlashConf.Dev[SelAlg].Start + 
               (((nAdr - FlashConf.Dev[SelAlg].Start) / PageSize) * PageSize);
    n = nAdr - PageAddr;
    m = ((nMany + n) < PageSize) ? nMany + n : PageSize;

    if (PageItem == 0) {
      for (i = 0; i < n; i++) Buffer[i] = (BYTE)FlashDev.valEmpty;
    }
    for (i = n; i < m; i++) Buffer[i] = *image++;

    if (m < PageSize) {
      for (i = m; i < PageSize; i++) Buffer[i] = (BYTE)FlashDev.valEmpty;
      PageItem = m;
    }
    else {
      if (ProgramPage(PageAddr, PageSize, Buffer)) return (1);
      PageItem = 0;
    }

    n = m - n;

    nAdr  += n;
    nMany -= n;
    Counter += n;
    UpdatePos(100*Counter/TotalCount);
  }

  return (0);
}


/*
 *  Write Entire Flash Memory (which is needed)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int WriteAll (void) {
  FLASHPARM *pF;
  DWORD      n;
  int        alg;
  int        flg = 0;

  InitProgress ("Program: ");
  UpdatePos(0);

  SelAlg = -1;
  TimeOut = 0;
  Counter = 0;
  pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, NULL);  // get parameters, NOTE: first call with NULL !
  if (pF->ActSize == 0) pF->many = 0;
  TotalCount = pF->ActSize;
  while (pF->many) {
    alg = FindFlashAlgorithm(pF->start, pF->many);
    if (alg == -3) break;
    if (alg  <  0) {
      n = NextAddr - pF->start;
      if ((alg == -2) && (pF->many > n)) {
        pF->start += n;
        pF->many  -= n;
      }
      else {
        pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, pF);
      }
      flg |= 1;
      continue;
    }
    flg |= 2;
    if (alg != SelAlg) {
      if (SelAlg != -1) {
        if (UnInit(2)) { SelAlg = -1; break; }
      }
      SelAlg = alg;
      if (LoadFlashDevAlg())  { SelAlg = -1; break; }
      if (Init(FlashConf.Dev[SelAlg].Start, pdbg->Clock, 2)) break;
    }
    n = (FlashConf.Dev[SelAlg].Start + FlashConf.Dev[SelAlg].Size) - pF->start;
    if (pF->many > n) {
      if (WriteFlash(pF->start, n, pF->image)) break;
      pF->start += n;
      pF->many  -= n;
      pF->image += n;
      continue;
    }
    else {
      if (WriteFlash(pF->start, pF->many, pF->image)) break;
    }
    pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, pF);  // Note: use pF from first call, get next parameters
    if ((pF->many == 0) ||
        (pF->start >= (FlashConf.Dev[SelAlg].Start + FlashConf.Dev[SelAlg].Size))) {
      if (WriteFlash(pF->start, 0, NULL)) {              // Write any Pending Program Pages
        flg |= 4;
        break;
      }
    }
  }
  if (SelAlg != -1) {
    if (UnInit(2)) flg |= 4;
  }
  if (pF->many) flg |= 4;

  StopProgress();

  switch (flg) {
    case 0:
    case 1:
      txtout("Programming skipped!");
      return (1);
    case 2:
      txtout("Programming Done.");
      break;
    case 3:
      txtout("Partial Programming Done (areas with no algorithms skipped!)");
      break;
    default:
      txtout("Programming Failed!");
      return (1);
  }
  return (0);
}



/*
 *  Verify Flash Memory (which was Erased/Programed)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int VerifyAll (void) {
  FLASHPARM *pF;
  DWORD      ofs, cnt;
  DWORD      adr, n, m;
  DWORD      crcarm;
  DWORD      crcpc;
  int        alg;
  int        status;
  int        flg = 0;
  int        error = 0;

  InitProgress ("Verify: ");
  UpdatePos(0);

  SelAlg = -1;
  TimeOut = 0;
  Counter = 0;
  pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, NULL);  // get parameters, NOTE: first call with NULL !
  if (pF->ActSize == 0) pF->many = 0;
  TotalCount = pF->ActSize;

  // Use Verify if present or try CRC Verify first and standard Verify if CRC fails
  while (pF->many) {

    // Skip non-Flash Algorithm Areas
    Silent = 1;
    alg = FindFlashAlgorithm(pF->start, pF->many);
    Silent = 0;
    if (alg == -3) break;
    if (alg < 0) {
      flg |= 1;
      n = NextAddr - pF->start;
      if ((alg == -2) && (pF->many > n)) {
        pF->start += n;
        pF->many  -= n;
        continue;
      }
      goto next;
    }
    flg |= 2;

    if (alg != SelAlg) {
      if (SelAlg != -1) {
        if (UnInit(3)) { SelAlg = -1; break; }
      }
      SelAlg = alg;
      if (LoadFlashDevAlg()) { SelAlg = -1; break; }
      if (Init(FlashConf.Dev[SelAlg].Start, pdbg->Clock, 3)) break;
    }

    if (FlashAlg.Verify != 0xFFFFFFFF) {
      m = (FlashConf.Dev[SelAlg].Start + FlashConf.Dev[SelAlg].Size) - pF->start;
      if (m > pF->many) m = pF->many;
      while (m) {
        if (m > PageSize) {
          n = PageSize;
        } else {
          n = m;
        }
        adr = Verify(pF->start, n, pF->image);
        if (ExeError) { error = 1; break; }
        if (adr != (pF->start + n)) {
          txtout ("Contents mismatch at: %08XH !", adr);
          error = 1;
          break;
        }
        pF->start += n;
        pF->many  -= n;
        pF->image += n;
        m         -= n;
        Counter   += n;
        SetPercent(100*Counter/TotalCount);
        UpdatePos (100*Counter/TotalCount);
      }
      if (error) break;
      if (pF->many) continue;
      goto next;
    }

    cnt = pF->many;
    ofs = 0;

    // CRC Verify
    crcarm = CalculateCRC(CRC_InitVal, pF->start, pF->many, CRC_Polynom);
    crcpc  = CRC32(pF->image, pF->many);
    if (!ExeError && (crcarm == crcpc)) {
      Counter += pF->many;
      SetPercent(100*Counter/TotalCount);
      UpdatePos (100*Counter/TotalCount);
      goto next;
    }

    // Standard Verify (Read & Compare)

    while (cnt > 0x1000) {
      adr = pF->start + ofs;
//    SetHex8(adr);

#if DBGCM_V8M
      status = ReadARMMem (&adr, Buffer + ofs, 0x1000, BLOCK_SECTYPE_ANY);
      if (status) { StopProgress(); OutError (status); return (1); }
#else // DBGCM_V8M
      status = ReadARMMem (&adr, Buffer + ofs, 0x1000);
      if (status) { StopProgress(); OutError (status); return (1); }
#endif // DBGCM_V8M

      ofs      += 0x1000;
      cnt      -= 0x1000;
      Counter  += 0x1000;
      SetPercent(100*Counter/TotalCount);
      UpdatePos (100*Counter/TotalCount);
    }
    if (cnt) {
      adr = pF->start + ofs;
//    SetHex8(adr);

#if DBGCM_V8M
      status = ReadARMMem (&adr, Buffer + ofs, cnt, BLOCK_SECTYPE_ANY);
      if (status) { StopProgress(); OutError (status); return (1); }
#else // DBGCM_V8M
      status = ReadARMMem (&adr, Buffer + ofs, cnt);
      if (status) { StopProgress(); OutError (status); return (1); }
#endif // DBGCM_V8M

      Counter  += cnt;
      SetPercent(100*Counter/TotalCount);
      UpdatePos (100*Counter/TotalCount);
    }

    // Compare data read from Flash with the expected data
    for (n = 0; n < pF->many; n++) {
      if (Buffer[n] != pF->image[n]) {
        if (error < 100) {
          txtout ("Contents mismatch at: %08XH  (Flash=%02XH  Required=%02XH) !",
                   pF->start+n, Buffer[n], pF->image[n]);
        }
        else if (error == 100) {
          txtout("Too many errors to display !");
          StopProgress();
          return (1);
        }
        error++;
      }
    }
    pF->many -= n;

next:
    pF = (FLASHPARM *)pCbFunc(AG_CB_GETFLASHPARAM, pF);  // Note: use pF from first call, get next parameters
  }

  if (SelAlg != -1) {
    if (UnInit(3)) flg |= 4;
  }
  if (pF->many) flg |= 4;
  if (error)    flg |= 4;

  StopProgress();

  switch (flg) {
    case 0:
    case 1:
      txtout("Verify skipped!");
      return (1);
    case 2:
      txtout("Verify OK.");
      break;
    case 3:
      txtout("Partial Verify OK (areas with no algorithms skipped!)");
      break;
    default:
      txtout("Verify Failed!");
      return (1);
  }
  return (0);
}



/*
 *  Run the Flash Application
 */

static void RunFlashAppl (void) {

#if DBGCM_DBG_DESCRIPTION
  int  status;

  if (PDSCDebug_IsEnabled()) {
    status = PDSCDebug_ResetHardware(0 /*bPreReset*/);
    if (status) { OutError (status); return; }
  } else {
    //---TODO:
    // HW Chip Reset
    DEVELOP_MSG("Todo: \nHW Chip Reset after flash load");
  }
#else // DBGCM_DBG_DESCRIPTION
  //---TODO:
  // HW Chip Reset
  DEVELOP_MSG("Todo: \nHW Chip Reset after flash load");
#endif // DBGCM_DBG_DESCRIPTION

  txtout("Application running ...\n");
}



/*
 * Load the right Flash algorithm and Init the Flash
 *
 */

DWORD InitDevFlash (void)  {

  // To Do: Flash Unlock
  return (0);
}



// ARM Dead loop Code - Thumb Mode (B .)
static const BYTE DeadLoop_ARM[] = {
  0xFE, 0xE7
};


static void TargetDeadLoop (void) {
  DWORD adr;
  int   status;

  if (FlashAlg.BreakPoint == 0xFFFFFFFF) {
    return;
  }

#if DBGCM_V8M
  adr    = (FlashAlg.BreakPoint & (~0x1));  // Mask out thumb bit used for LR writes
  status = WriteARMMem (&adr, const_cast<BYTE*>(DeadLoop_ARM), sizeof(DeadLoop_ARM), BLOCK_SECTYPE_ANY);
  if (status) OutError (status);
#else // DBGCM_V8M
  adr    = (FlashAlg.BreakPoint & (~0x1));  // Mask out thumb bit used for LR writes
  status = WriteARMMem (&adr, const_cast<BYTE*>(DeadLoop_ARM), sizeof(DeadLoop_ARM));
  if (status) OutError (status);
#endif // DBGCM_V8M

}



/*
 * Load the right Flash algorithm and put the bytes into the Flash
 *
 */

DWORD FlashLoad (void)  {
  U32 status = 0;

  if ((FlashConf.Opt & (FLASH_ERASE | FLASH_PROGRAM | FLASH_VERIFY | FLASH_RESETRUN)) == 0) {
    AGDIMsgBox(hMfrm, "No Flash Operation Selected.", "LINK - Cortex-M Warning", MB_OK, IDOK);
    return (0);
  }

#if DBGCM_DBG_DESCRIPTION
  PDSCDebug_DebugContext = DBGCON_FLASH_ERASE;
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
  if (FlashConf.Opt & FLASH_ERASE) {
    status = DSM_SuspendMonitor();
    if (status) { OutErrorMessage(status); return (1); }

    if (FlashConf.Opt & FLASH_ERASEALL) {
      status = EraseFull();
    } else {
      status = EraseAll();
    }
    if (status) {
      DSM_ResumeMonitor();
      return (1);
    }

    status = DSM_ResumeMonitor();
    if (status) { OutErrorMessage(status); return (1); }
  }
#else // DBGCM_DS_MONITOR
  if (FlashConf.Opt & FLASH_ERASE) {
    if (FlashConf.Opt & FLASH_ERASEALL) {
      if (EraseFull()) return (1);
    } else {
      if (EraseAll())  return (1);
    }
  }
#endif // DBGCM_DS_MONITOR

#if DBGCM_DBG_DESCRIPTION
  if (PDSCDebug_IsEnabled()) {
    status = PDSCDebug_FlashEraseDone();
    if (status) { OutErrorMessage(status); return (1); }

#if DBGCM_RECOVERY
    status = DebugAccessEnsure();
    if (status) { OutErrorMessage(status); return (1); }
#endif // DBGCM_RECOVERY

  }

  PDSCDebug_DebugContext = DBGCON_FLASH_PROGRAM;
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
  if (FlashConf.Opt & FLASH_PROGRAM) {
    status = DSM_SuspendMonitor();
    if (status) { OutErrorMessage(status); return (1); }

    if (WriteAll()) {
      DSM_ResumeMonitor();
      return (1);
    }

    status = DSM_ResumeMonitor();
    if (status) { OutErrorMessage(status); return (1); }
  }
#else // DBGCM_DS_MONITOR
  if (FlashConf.Opt & FLASH_PROGRAM) {
    if (WriteAll()) return (1);
  }
#endif // DBGCM_DS_MONITOR

#if DBGCM_DBG_DESCRIPTION
  if (PDSCDebug_IsEnabled()) {
    status = PDSCDebug_FlashProgramDone();
    if (status) { OutErrorMessage(status); return (1); }
#if DBGCM_RECOVERY
    status = DebugAccessEnsure();
    if (status) { OutErrorMessage(status); return (1); }
#endif // DBGCM_RECOVERY
  }

  PDSCDebug_DebugContext = DBGCON_VERIFY_CODE;
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
  if (FlashConf.Opt & FLASH_VERIFY) {
    status = DSM_SuspendMonitor();
    if (status) { OutErrorMessage(status); return (1); }

    if (VerifyAll()) {
      DSM_ResumeMonitor();
      return (1);
    }

    status = DSM_ResumeMonitor();
    if (status) { OutErrorMessage(status); return (1); }
  }
#else // DBGCM_DS_MONITOR
  if (FlashConf.Opt & FLASH_VERIFY) {
    if (VerifyAll()) return (1);
  }
#endif // DBGCM_DS_MONITOR

  // Overwrite Flash Algorithm Breakpoint with dead loop
  TargetDeadLoop();
  
  //PDSCDebug_DebugContext = DBGCON_DISCONNECT;  // Set in ExitDebug()

  ExitDebug();

  if (FlashConf.Opt & FLASH_RESETRUN) {
    RunFlashAppl();
  }

  return (0);
}


/*
 *  Erase Flash Memory (all Chips)
 */

DWORD EraseFlash (void) {
  U32 status = 0;

#if DBGCM_DBG_DESCRIPTION
  PDSCDebug_DebugContext = DBGCON_FLASH_ERASE;
  if (EraseFull()) return (1);

  if (PDSCDebug_IsEnabled()) {
    status = PDSCDebug_FlashEraseDone();
    if (status) { OutErrorMessage(status); return (1); }
#if DBGCM_RECOVERY
    status = DebugAccessEnsure();
    if (status) { OutErrorMessage(status); return (1); }
#endif // DBGCM_RECOVERY
  }
#else // DBGCM_DBG_DESCRIPTION
  if (EraseFull()) return (1);
#endif // DBGCM_DBG_DESCRIPTION

  // Overwrite Flash Algorithm Breakpoint with dead loop
  TargetDeadLoop();
  
  //PDSCDebug_DebugContext = DBGCON_DISCONNECT;  // Set in ExitDebug()

  ExitDebug();

  return (0);
}

// Initialize module variables
void InitFlash() {
  memset(&FlashDev, 0, sizeof(FlashDev));    // Flash Device Description Info
  memset(&FlashAlg, 0, sizeof(FlashAlg));    // Flash Device Algorithm Info

  SelAlg = 0;               // Selected Algorithm
  TimeOut = 0;              // TimeOut Expired
  TimeOutTick = 0;          // TimeOut Tick for Progress Bar

  Silent = 0;               // Silent Mode without some Warnings

  Counter = 0;              // Byte Counter
  TotalCount = 0;           // Total Byte Counter

  SecCount = 0;             // Number of Sectors (from FlashDevice)
  TotalSec = 0;             // Total Number of Sectors in Device

  ESecAddr = 0;             // Last Erased Sector Address
  ESecSize = 0;             // Last Erased Sector Size

  PageAddr = 0;             // Pending Program Page Address
  PageSize = 0;             // Pending Program Page Size
  PageItem = 0;             // Pending Program Page Item Count

  NextAddr = 0;             // Next Address with known Algorithm

  ExeError = 0;             // System Call Execution Error

  memset(Buffer, 0, sizeof(Buffer));

  memset(&oil, 0, sizeof(oil));    // Progress-Bar data
  oldadr = 0;
  oldsz = 0;
}
