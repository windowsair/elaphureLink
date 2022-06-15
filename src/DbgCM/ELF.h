/**************************************************************************//**
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



#ifndef __ELF_H__
#define __ELF_H__


#define ELF32_ST_BIND(i)   ((i) >> 4)
#define ELF32_ST_TYPE(i)   ((i) & 0x0F)
#define ELF32_ST_INFO(b,t) (((b) << 4) + ((t) & 0x0F))


/* Fields in e_ident[] */

#define EI_MAG0       0  /* File identification byte 0 index */
#define ELFMAG0    0x7F  /* Magic number byte 0 */

#define EI_MAG1       1  /* File identification byte 1 index */
#define ELFMAG1      'E' /* Magic number byte 1 */

#define EI_MAG2       2  /* File identification byte 2 index */
#define ELFMAG2      'L' /* Magic number byte 2 */

#define EI_MAG3       3  /* File identification byte 3 index */
#define ELFMAG3      'F' /* Magic number byte 3 */

#define EI_CLASS      4  /* File class */
#define ELFCLASSNONE  0  /* Invalid class */
#define ELFCLASS32    1  /* 32-bit objects */
#define ELFCLASS64    2  /* 64-bit objects */

#define EI_DATA       5  /* Data encoding */
#define ELFDATANONE   0  /* Invalid data encoding */
#define ELFDATA2LSB   1  /* 2's complement, little endian */
#define ELFDATA2MSB   2  /* 2's complement, big endian */

#define EI_VERSION    6  /* File version */

//---TODO: format

#define EI_OSABI              7   /* Operating System/ABI indication */
#define ELFOSABI_NONE         0   /* UNIX System V ABI */
#define ELFOSABI_HPUX         1   /* HP-UX operating system */
#define ELFOSABI_NETBSD       2   /* NetBSD */
#define ELFOSABI_LINUX        3   /* GNU/Linux */
#define ELFOSABI_HURD         4   /* GNU/Hurd */
#define ELFOSABI_SOLARIS      6   /* Solaris */
#define ELFOSABI_MONTEREY     7   /* Monterey */
#define ELFOSABI_IRIX         8   /* IRIX */
#define ELFOSABI_FREEBSD      9   /* FreeBSD */
#define ELFOSABI_TRU64       10   /* TRU64 UNIX */
#define ELFOSABI_MODESTO     11   /* Novell Modesto */
#define ELFOSABI_OPENBSD     12   /* OpenBSD */
#define ELFOSABI_STANDALONE 255   /* Standalone (embedded) application */
#define ELFOSABI_ARM         97   /* ARM */

#define EI_ABIVERSION         8   /* ABI version */
#define EI_PAD                9   /* Start of padding bytes */


/* Values for e_type, which identifies the object file type */

#define ET_NONE         0x0000    /* No file type */
#define ET_REL          0x0001    /* Relocatable file */
#define ET_EXEC         0x0002    /* Executable file */
#define ET_DYN          0x0003    /* Shared object file */
#define ET_CORE         0x0004    /* Core file */
#define ET_LOOS         0xFE00    /* Operating system-specific */
#define ET_HIOS         0xFEFF    /* Operating system-specific */
#define ET_LOPROC       0xFF00    /* Processor-specific */
#define ET_HIPROC       0xFFFF    /* Processor-specific */


/* Values for e_machine, which identifies the architecture */

#define EM_NONE         0 /* No machine */
#define EM_M32          1 /* AT&T WE 32100 */
#define EM_SPARC        2 /* SUN SPARC */
#define EM_386          3 /* Intel 80386 */
#define EM_68K          4 /* Motorola m68k family */
#define EM_88K          5 /* Motorola m88k family */
#define EM_486          6 /* Intel 80486 */
#define EM_860          7 /* Intel 80860 */
#define EM_MIPS         8 /* MIPS R3000 (officially, big-endian only) */
#define EM_S370         9 /* IBM System/370 */
#define EM_MIPS_RS4_BE 10 /* MIPS R4000 big-endian */ /* Depreciated */
#define EM_MIPS_RS3_LE 10 /* MIPS R3000 little-endian (Oct 4 1999 Draft)*/ /* Depreciated */

#define EM_PARISC      15 /* HPPA */
#define EM_VPP550      17 /* Fujitsu VPP500 */
#define EM_SPARC32PLUS 18 /* Sun's "v8plus" */
#define EM_960         19 /* Intel 80960 */
#define EM_PPC         20 /* PowerPC */
#define EM_PPC64       21 /* 64-bit PowerPC */

#define EM_V800        36 /* NEC V800 series */
#define EM_FR20        37 /* Fujitsu FR20 */
#define EM_RH32        38 /* TRW RH32 */
#define EM_MCORE       39 /* Motorola M*Core */ /* May also be taken by Fujitsu MMA */
#define EM_RCE         39 /* Old name for MCore */
#define EM_ARM         40 /* ARM */
#define EM_OLD_ALPHA   41 /* Digital Alpha */
#define EM_SH          42 /* Hitachi SH */
#define EM_SPARCV9     43 /* SPARC v9 64-bit */
#define EM_TRICORE     44 /* Siemens Tricore embedded processor */
#define EM_ARC         45 /* Argonaut RISC Core, Argonaut Technologies Inc. */
#define EM_H8_300      46 /* Hitachi H8/300 */
#define EM_H8_300H     47 /* Hitachi H8/300H */
#define EM_H8S         48 /* Hitachi H8S */
#define EM_H8_500      49 /* Hitachi H8/500 */
#define EM_IA_64       50 /* Intel IA-64 Processor */
#define EM_MIPS_X      51 /* Stanford MIPS-X */
#define EM_COLDFIRE    52 /* Motorola Coldfire */
#define EM_68HC12      53 /* Motorola M68HC12 */
#define EM_MMA         54 /* Fujitsu Multimedia Accelerator */
#define EM_PCP         55 /* Siemens PCP */
#define EM_NCPU        56 /* Sony nCPU embedded RISC processor */
#define EM_NDR1        57 /* Denso NDR1 microprocesspr */
#define EM_STARCORE    58 /* Motorola Star*Core processor */
#define EM_ME16        59 /* Toyota ME16 processor */
#define EM_ST100       60 /* STMicroelectronics ST100 processor */
#define EM_TINYJ       61 /* Advanced Logic Corp. TinyJ embedded processor */

#define EM_FX66        66 /* Siemens FX66 microcontroller */
#define EM_ST9PLUS     67 /* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define EM_ST7         68 /* STMicroelectronics ST7 8-bit microcontroller */
#define EM_68HC16      69 /* Motorola MC68HC16 Microcontroller */
#define EM_68HC11      70 /* Motorola MC68HC11 Microcontroller */
#define EM_68HC08      71 /* Motorola MC68HC08 Microcontroller */
#define EM_68HC05      72 /* Motorola MC68HC05 Microcontroller */
#define EM_SVX         73 /* Silicon Graphics SVx */
#define EM_ST19        74 /* STMicroelectronics ST19 8-bit microcontroller */
#define EM_VAX         75 /* Digital VAX */

#define EM_PJ          99 /* picoJava */


/* Values for e_version */

#define EV_NONE         0   /* Invalid ELF version */
#define EV_CURRENT      1   /* Current version */


/* Values for section header, sh_type field */
#define SHT_NULL           0    /* Section header table entry unused */
#define SHT_PROGBITS       1    /* Program specific (private) data */
#define SHT_SYMTAB         2    /* Link editing symbol table */
#define SHT_STRTAB         3    /* A string table */
#define SHT_RELA           4    /* Relocation entries with addends */
#define SHT_HASH           5    /* A symbol hash table */
#define SHT_DYNAMIC        6    /* Information for dynamic linking */
#define SHT_NOTE           7    /* Information that marks file */
#define SHT_NOBITS         8    /* Section occupies no space in file */
#define SHT_REL            9    /* Relocation entries, no addends */
#define SHT_SHLIB         10    /* Reserved, unspecified semantics */
#define SHT_DYNSYM        11    /* Dynamic linking symbol table */

#define SHT_INIT_ARRAY    14    /* Array of pointers to init functions */
#define SHT_FINI_ARRAY    15    /* Array of pointers to finish functions */
#define SHT_PREINIT_ARRAY 16    /* Array of pointers to pre-init functions */

#define SHT_LOOS   0x60000000   /* Operating system specific semantics, lo */
#define SHT_HIOS   0x6fffffff   /* Operating system specific semantics, hi */


/* The next three section types are defined by Solaris, and are named
   SHT_SUNW*.  We use them in GNU code, so we also define SHT_GNU*
   versions.  */

#define SHT_SUNW_verdef  0x6ffffffd  /* Versions defined by file */
#define SHT_SUNW_verneed 0x6ffffffe  /* Versions needed by file */
#define SHT_SUNW_versym  0x6fffffff  /* Symbol versions */

#define SHT_GNU_verdef  SHT_SUNW_verdef
#define SHT_GNU_verneed SHT_SUNW_verneed
#define SHT_GNU_versym  SHT_SUNW_versym

#define SHT_LOPROC  0x70000000  /* Processor-specific semantics, lo */
#define SHT_HIPROC  0x7FFFFFFF  /* Processor-specific semantics, hi */
#define SHT_LOUSER  0x80000000  /* Application-specific semantics */
/* #define SHT_HIUSER 0x8FFFFFFF    *//* Application-specific semantics */
#define SHT_HIUSER  0xFFFFFFFF  /* New value, defined in Oct 4, 1999 Draft */

/* Values for section header, sh_flags field */

#define SHF_WRITE            (1 << 0)  /* Writable data during execution */
#define SHF_ALLOC            (1 << 1)  /* Occupies memory during execution */
#define SHF_EXECINSTR        (1 << 2)  /* Executable machine instructions */
#define SHF_MERGE            (1 << 4)  /* Data in this section can be merged */
#define SHF_STRINGS          (1 << 5)  /* Contains null terminated character strings */
#define SHF_INFO_LINK        (1 << 6)  /* sh_info holds section header table index */
#define SHF_LINK_ORDER       (1 << 7)  /* Preserve section ordering when linking */
#define SHF_OS_NONCONFORMING (1 << 8)  /* OS specifci processing required */


/* #define SHF_MASKOS 0x0F000000    */ /* OS-specific semantics */
#define SHF_MASKOS         0x0FF00000  /* New value, Oct 4, 1999 Draft */
#define SHF_MASKPROC       0xF0000000  /* Processor-specific semantics */

/* Special section indices, which may show up in st_shndx fields, among
   other places. */

#define SHN_UNDEF       0x0000    /* Undefined section reference */
#define SHN_LORESERVE   0xFF00    /* Begin range of reserved indices */
#define SHN_LOPROC      0xFF00    /* Begin range of appl-specific */
#define SHN_HIPROC      0xFF1F    /* End range of appl-specific */
#define SHN_LOOS        0xFF20    /* OS specific semantics, lo */
#define SHN_HIOS        0xFF3F    /* OS specific semantics, hi */
#define SHN_ABS         0xFFF1    /* Associated symbol is absolute */
#define SHN_COMMON      0xFFF2    /* Associated symbol is in common */
#define SHN_HIRESERVE   0xFFFF    /* End range of reserved indices */


/* Values for program header, p_type field */

#define PT_NULL             0   /* Program header table entry unused */
#define PT_LOAD             1   /* Loadable program segment */
#define PT_DYNAMIC          2   /* Dynamic linking information */
#define PT_INTERP           3   /* Program interpreter */
#define PT_NOTE             4   /* Auxiliary information */
#define PT_SHLIB            5   /* Reserved, unspecified semantics */
#define PT_PHDR             6   /* Entry for header table itself */
#define PT_LOOS    0x60000000   /* OS-specific */
#define PT_HIOS    0x6fffffff   /* OS-specific */
#define PT_LOPROC  0x70000000   /* Processor-specific */
#define PT_HIPROC  0x7FFFFFFF   /* Processor-specific */


/* Program segment permissions, in program header p_flags field */

#define PF_X    (1 << 0)  /* Segment is executable */
#define PF_W    (1 << 1)  /* Segment is writable */
#define PF_R    (1 << 2)  /* Segment is readable */

/* #define PF_MASKOS  0x0F000000    *//* OS-specific reserved bits */
#define PF_MASKOS   0x0FF00000   /* New value, Oct 4, 1999 Draft */
#define PF_MASKPROC 0xF0000000   /* Processor-specific reserved bits */


#define STB_LOCAL        0   /* Symbol not visible outside obj */
#define STB_GLOBAL       1   /* Symbol visible outside obj */
#define STB_WEAK         2   /* Like globals, lower precedence */
#define STB_LOOS        10   /* OS-specific semantics */
#define STB_HIOS        12   /* OS-specific semantics */
#define STB_LOPROC      13   /* Application-specific semantics */
#define STB_HIPROC      15   /* Application-specific semantics */

#define STT_NOTYPE       0   /* Symbol type is unspecified */
#define STT_OBJECT       1   /* Symbol is a data object */
#define STT_FUNC         2   /* Symbol is a code object */
#define STT_SECTION      3   /* Symbol associated with a section */
#define STT_FILE         4   /* Symbol gives a file name */
#define STT_COMMON       5   /* An uninitialised common block */
#define STT_LOOS        10   /* OS-specific semantics */
#define STT_HIOS        12   /* OS-specific semantics */
#define STT_LOPROC      13   /* Application-specific semantics */
#define STT_HIPROC      15   /* Application-specific semantics */


#define EI_NIDENT 16

#pragma pack(1)

typedef struct  {
  char      e_ident [EI_NIDENT];
  WORD       e_type;
  WORD    e_machine;
  DWORD   e_version;
  DWORD     e_entry;
  DWORD     e_phoff;
  DWORD     e_shoff;
  DWORD     e_flags;
  WORD     e_ehsize;
  WORD  e_phentsize;
  WORD      e_phnum;
  WORD  e_shentsize;
  WORD      e_shnum;
  WORD   e_shstrndx;
} Elf32_Ehdr;

typedef struct  {
  DWORD       sh_name;
  DWORD       sh_type;
  DWORD      sh_flags;
  DWORD       sh_addr;
  DWORD     sh_offset;
  DWORD       sh_size;
  DWORD       sh_link;
  DWORD       sh_info;
  DWORD  sh_addralign;
  DWORD    sh_entsize;
} Elf32_Shdr;

typedef struct {
  DWORD    p_type;
  DWORD  p_offset;
  DWORD   p_vaddr;
  DWORD   p_paddr;
  DWORD  p_filesz;
  DWORD   p_memsz;
  DWORD   p_flags;
  DWORD   p_align;
} Elf32_Phdr;

typedef struct {
  DWORD   st_name;
  DWORD  st_value;
  DWORD   st_size;
  BYTE    st_info;
  BYTE   st_other;
  WORD   st_shndx;
} Elf32_Sym;

#pragma pack()


struct Elf32_Info {
  FILE       *fh;              // File Handle
  Elf32_Ehdr  ehdr;            // ELF Header
  Elf32_Shdr *shdr;            // List of ELF Section Headers
  Elf32_Phdr *phdr;            // List of ELF Program Headers
  Elf32_Sym  *sym;             // List of ELF Symbol Tables
  DWORD       sym_cnt;         // Symbol Count
  char       *strtab;          // String Table
};

extern Elf32_Info Elf;         // ELF Information

extern void ElfInit   (void);  // ELF Initialization
extern void ElfUnInit (void);  // ELF UnInitialization
extern BOOL ReadElf   (void);  // Read ELF Information


#endif
