/**************************************************************************//**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.2
 * @date     $Date: 2016-03-24 09:07:53 +0100 (Thu, 24 Mar 2016) $
 *
 * @note
 * Copyright (C) 2009-2015 ARM Limited. All rights reserved.
 *
 * @brief     ELF Handling Functions for the Flash Downloader
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
#include "ELF.h"
#include "Collect.h"


#define DSWAP32(a) ((a >> 24) & 0xFF) | (((a >> 16) & 0xFF) << 8) | (((a >> 8) & 0xFF) << 16) | ((a & 0xFF) << 24)
#define DSWAP16(a) ((a >>  8) & 0xFF) | ((a & 0xFF) << 8)


Elf32_Info Elf;  // ELF Information


// Offset addresses of special sections
static DWORD   text_i;     // .text
static DWORD rodata_i;     // .rodata
static DWORD   data_i;     // .data
static DWORD    bss_i;     // .bss
static DWORD symtab_i;     // .symtab
static DWORD strtab_i;     // .strtab


static int   Endian;


static BYTE ubyte (void) {
  BYTE b;

  fread(&b, 1, 1, Elf.fh);
  return (b);
}

static WORD uhalf (void) {
  WORD w;

  fread(&w, 1, 2, Elf.fh);
  if (Endian)  w = DSWAP16(w);
  return (w);
}

static DWORD uword (void) {
  DWORD d;

  fread(&d, 1, 4, Elf.fh);
  if (Endian)  d = DSWAP32(d);
  return (d);
}


static char name[0x200];

static int getname (void)  {
  int i;

  for (i = 0; i < sizeof (name); i++)  {
    name[i] = ubyte();
    if (name[i] == 0) return (i);
  }
  return (i);
}


/*
 * read the header of the dwarf file
 */

static BOOL readelfheader (void) {
  int  i;

  i = fread(&Elf.ehdr, 1, sizeof(Elf32_Ehdr), Elf.fh);
  if (i != sizeof(Elf32_Ehdr)) return (FALSE);

  if (!((Elf.ehdr.e_ident[0] == ELFMAG0) &&
        (Elf.ehdr.e_ident[1] == ELFMAG1) &&
        (Elf.ehdr.e_ident[2] == ELFMAG2) &&
        (Elf.ehdr.e_ident[3] == ELFMAG3)))
    return (FALSE);

  Endian = (Elf.ehdr.e_ident[EI_DATA] == ELFDATA2MSB) ? 1 : 0;

  if (Endian) {
    Elf.ehdr.e_type      = DSWAP16(Elf.ehdr.e_type);
    Elf.ehdr.e_machine   = DSWAP16(Elf.ehdr.e_machine);
    Elf.ehdr.e_version   = DSWAP32(Elf.ehdr.e_version);
    Elf.ehdr.e_entry     = DSWAP32(Elf.ehdr.e_entry);
    Elf.ehdr.e_phoff     = DSWAP32(Elf.ehdr.e_phoff);
    Elf.ehdr.e_shoff     = DSWAP32(Elf.ehdr.e_shoff);
    Elf.ehdr.e_flags     = DSWAP32(Elf.ehdr.e_flags);
    Elf.ehdr.e_ehsize    = DSWAP16(Elf.ehdr.e_ehsize);
    Elf.ehdr.e_phentsize = DSWAP16(Elf.ehdr.e_phentsize);
    Elf.ehdr.e_phnum     = DSWAP16(Elf.ehdr.e_phnum);
    Elf.ehdr.e_shentsize = DSWAP16(Elf.ehdr.e_shentsize);
    Elf.ehdr.e_shnum     = DSWAP16(Elf.ehdr.e_shnum);
    Elf.ehdr.e_shstrndx  = DSWAP16(Elf.ehdr.e_shstrndx);
  }

  if (Elf.ehdr.e_type    != ET_EXEC) return (FALSE);
  if (Elf.ehdr.e_machine != EM_ARM)  return (FALSE);
  return (TRUE);
}


/*
 * look into the sections table
 */

static BOOL readelfsections (void) {
  int  i;
  int  n = 0;

  Elf.shdr = (Elf32_Shdr *) calloc (Elf.ehdr.e_shnum, sizeof (Elf32_Shdr));
  if (Elf.shdr == NULL) return (FALSE);

  if (fseek(Elf.fh, Elf.ehdr.e_shoff, SEEK_SET)) return (FALSE);

  fread(Elf.shdr, 1, Elf.ehdr.e_shentsize * Elf.ehdr.e_shnum, Elf.fh);

  for (i = 0; i < Elf.ehdr.e_shnum; i++) {
    if (Endian)  {
      Elf.shdr[i].sh_name      = DSWAP32 (Elf.shdr[i].sh_name);
      Elf.shdr[i].sh_addr      = DSWAP32 (Elf.shdr[i].sh_addr);
      Elf.shdr[i].sh_addralign = DSWAP32 (Elf.shdr[i].sh_addralign);
      Elf.shdr[i].sh_entsize   = DSWAP32 (Elf.shdr[i].sh_entsize);
      Elf.shdr[i].sh_flags     = DSWAP32 (Elf.shdr[i].sh_flags);
      Elf.shdr[i].sh_info      = DSWAP32 (Elf.shdr[i].sh_info);
      Elf.shdr[i].sh_link      = DSWAP32 (Elf.shdr[i].sh_link);
      Elf.shdr[i].sh_offset    = DSWAP32 (Elf.shdr[i].sh_offset);
      Elf.shdr[i].sh_size      = DSWAP32 (Elf.shdr[i].sh_size);
      Elf.shdr[i].sh_type      = DSWAP32 (Elf.shdr[i].sh_type);
    }
  }

  for (i = 0; i < Elf.ehdr.e_shnum; i++)  {
    if (Elf.shdr[i].sh_type == SHT_STRTAB)  {
      if (fseek(Elf.fh, Elf.shdr[i].sh_name + Elf.shdr[i].sh_offset, SEEK_SET)) return (FALSE);
      getname();
      if (strcmp (name, ".shstrtab") == 0)  n = i;
    }
  }

  for (i = 0; i < Elf.ehdr.e_shnum; i++)  {
    if (Elf.shdr[i].sh_name == 0)  continue;
    if (fseek(Elf.fh, Elf.shdr[i].sh_name + Elf.shdr[n].sh_offset, SEEK_SET)) return (FALSE);
    getname();
    if      (strcmp (name, ".text")   == 0)   text_i = i;
    else if (strcmp (name, ".rodata") == 0) rodata_i = i;
    else if (strcmp (name, ".data")   == 0)   data_i = i;
    else if (strcmp (name, ".bss")    == 0)    bss_i = i;
    else if (strcmp (name, ".symtab") == 0) symtab_i = i;
    else if (strcmp (name, ".strtab") == 0) strtab_i = i;
  }

  return (TRUE);
}


/*
 * read the program header table
 */

static BOOL readelfprogheader (void) {
  int  i;

  Elf.phdr = (Elf32_Phdr *) calloc (Elf.ehdr.e_phnum, sizeof (Elf32_Phdr));
  if (Elf.phdr == NULL) return (FALSE);

  if (fseek(Elf.fh, Elf.ehdr.e_phoff, SEEK_SET)) return (FALSE);

  for (i = 0; i < Elf.ehdr.e_phnum; i++) {
    Elf.phdr[i].p_type   = uword();
    Elf.phdr[i].p_offset = uword();
    Elf.phdr[i].p_vaddr  = uword();
    Elf.phdr[i].p_paddr  = uword();
    Elf.phdr[i].p_filesz = uword();
    Elf.phdr[i].p_memsz  = uword();
    Elf.phdr[i].p_flags  = uword();
    Elf.phdr[i].p_align  = uword();
  }

  return (TRUE);
}


/*
 *  look into the symbol table
 */

static BOOL readelfsymtable (void) {
  DWORD sym_off, sym_sz;
  DWORD str_off, str_sz;
  DWORD i;


  if (strtab_i == 0) return (TRUE);
  if (symtab_i == 0) return (TRUE);

  sym_off = Elf.shdr[symtab_i].sh_offset;
  sym_sz  = Elf.shdr[symtab_i].sh_size;
  str_off = Elf.shdr[strtab_i].sh_offset;
  str_sz  = Elf.shdr[strtab_i].sh_size;

  if (sym_off == 0 || sym_sz == 0) return (TRUE);
  if (str_off == 0 || str_sz == 0) return (TRUE);

  Elf.sym_cnt = sym_sz / sizeof (Elf32_Sym);
  Elf.sym = (Elf32_Sym *) calloc (Elf.sym_cnt, sizeof (Elf32_Sym));
  if (Elf.sym == NULL) return (FALSE);

  for (i = 0; i < Elf.sym_cnt; i++) {
    if (fseek(Elf.fh, sym_off + (i * sizeof(Elf32_Sym)), SEEK_SET)) return (FALSE);
    Elf.sym[i].st_name  = uword();
    Elf.sym[i].st_value = uword();
    Elf.sym[i].st_size  = uword();
    Elf.sym[i].st_info  = ubyte();
    Elf.sym[i].st_other = ubyte();
    Elf.sym[i].st_shndx = uhalf();
  }

  Elf.strtab = (char *) calloc (str_sz, 1);
  if (Elf.strtab == NULL) return (FALSE);

  if (fseek(Elf.fh, str_off, SEEK_SET)) return (FALSE);
  i = fread(Elf.strtab, 1, str_sz, Elf.fh);
  if (i != str_sz) return (FALSE);

  return (TRUE);
}



void ElfInit (void) {

  Elf.shdr   = NULL;
  Elf.phdr   = NULL;
  Elf.sym    = NULL;
  Elf.strtab = NULL;

    text_i = 0;
  rodata_i = 0;
    data_i = 0;
     bss_i = 0;
  symtab_i = 0;
  strtab_i = 0;

  Elf.sym_cnt = 0;
}


void ElfUnInit (void) {

  if (Elf.shdr)   free(Elf.shdr);
  if (Elf.phdr)   free(Elf.phdr);
  if (Elf.sym)    free(Elf.sym);
  if (Elf.strtab) free(Elf.strtab);
}


BOOL ReadElf (void) {
  BOOL ok;

  ok = readelfheader();
  if (ok) ok = readelfsections();
  if (ok) ok = readelfprogheader();
  if (ok) ok = readelfsymtable();

  return (ok);
}

void InitELF() {
  memset(&Elf, 0, sizeof(Elf));  // ELF Information

  // Offset addresses of special sections
  text_i = 0;       // .text
  rodata_i = 0;     // .rodata
  data_i = 0;       // .data
  bss_i = 0;        // .bss
  symtab_i = 0;     // .symtab
  strtab_i = 0;     // .strtab

  Endian = 0;
  memset(name, 0, sizeof(name));
}
