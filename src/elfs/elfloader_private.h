#ifndef __ELFLOADER_PRIVATE_H_
#define __ELFLOADER_PRIVATE_H_

#ifdef DYNAREC
typedef struct dynablocklist_s dynablocklist_t;
#endif

typedef struct library_s library_t;
typedef struct needed_libs_s needed_libs_t;

#include <pthread.h>

struct elfheader_s {
    char*       name;
    char*       path;   // Resolved path to file
    size_t      numPHEntries;
    Elf64_Phdr  *PHEntries;
    size_t      numSHEntries;
    Elf64_Shdr  *SHEntries;
    size_t      SHIdx;
    size_t      numSST;
    char*       SHStrTab;
    char*       StrTab;
    Elf64_Sym*  SymTab;
    size_t      numSymTab;
    char*       DynStr;
    Elf64_Sym*  DynSym;
    size_t      numDynSym;
    Elf64_Dyn*  Dynamic;
    size_t      numDynamic;
    char*       DynStrTab;
    size_t      szDynStrTab;
    Elf64_Half* VerSym;
    Elf64_Verneed*  VerNeed;
    int         szVerNeed;
    Elf64_Verdef*   VerDef;
    int         szVerDef;
    int         e_type;

    intptr_t    delta;  // should be 0

    uintptr_t   entrypoint;
    uintptr_t   initentry;
    uintptr_t   initarray;
    size_t      initarray_sz;
    uintptr_t   finientry;
    uintptr_t   finiarray;
    size_t      finiarray_sz;

    uintptr_t   rel;
    size_t      relsz;
    int         relent;
    uintptr_t   rela;
    size_t      relasz;
    int         relaent;
    uintptr_t   jmprel;
    size_t      pltsz;
    int         pltent;
    uint64_t    pltrel;
    uintptr_t   gotplt;
    uintptr_t   gotplt_end;
    uintptr_t   pltgot;
    uintptr_t   got;
    uintptr_t   got_end;
    uintptr_t   plt;
    uintptr_t   plt_end;
    uintptr_t   text;
    size_t      textsz;

    uintptr_t   paddr;
    uintptr_t   vaddr;
    size_t      align;
    uint64_t    memsz;
    uint64_t    stacksz;
    size_t      stackalign;
    uint64_t    tlssize;
    size_t      tlsalign;

    int64_t     tlsbase;    // the base of the tlsdata in the global tlsdata (always negative)

    int         init_done;
    int         fini_done;

    char*       memory; // char* and not void* to allow math on memory pointer
    void**      multiblock;
    uintptr_t*  multiblock_offs;
    uint64_t*   multiblock_size;
    int         multiblock_n;

    library_t   *lib;
    needed_libs_t *neededlibs;
};

#define R_X86_64_NONE           0       /* No reloc */
#define R_X86_64_64             1       /* Direct 64 bit  */
#define R_X86_64_PC32           2       /* PC relative 32 bit signed */
#define R_X86_64_GOT32          3       /* 32 bit GOT entry */
#define R_X86_64_PLT32          4       /* 32 bit PLT address */
#define R_X86_64_COPY           5       /* Copy symbol at runtime */
#define R_X86_64_GLOB_DAT       6       /* Create GOT entry */
#define R_X86_64_JUMP_SLOT      7       /* Create PLT entry */
#define R_X86_64_RELATIVE       8       /* Adjust by program base */
#define R_X86_64_GOTPCREL       9       /* 32 bit signed PC relative offset to GOT */
#define R_X86_64_32             10      /* Direct 32 bit zero extended */
#define R_X86_64_32S            11      /* Direct 32 bit sign extended */
#define R_X86_64_16             12      /* Direct 16 bit zero extended */
#define R_X86_64_PC16           13      /* 16 bit sign extended pc relative */
#define R_X86_64_8              14      /* Direct 8 bit sign extended  */
#define R_X86_64_PC8            15      /* 8 bit sign extended pc relative */
#define R_X86_64_DTPMOD64       16      /* ID of module containing symbol */
#define R_X86_64_DTPOFF64       17      /* Offset in module's TLS block */
#define R_X86_64_TPOFF64        18      /* Offset in initial TLS block */
#define R_X86_64_TLSGD          19      /* 32 bit signed PC relative offset to two GOT entries for GD symbol */
#define R_X86_64_TLSLD          20      /* 32 bit signed PC relative offset to two GOT entries for LD symbol */
#define R_X86_64_DTPOFF32       21      /* Offset in TLS block */
#define R_X86_64_GOTTPOFF       22      /* 32 bit signed PC relative offset to GOT entry for IE symbol */
#define R_X86_64_TPOFF32        23      /* Offset in initial TLS block */
#define R_X86_64_PC64           24      /* PC relative 64 bit */
#define R_X86_64_GOTOFF64       25      /* 64 bit offset to GOT */
#define R_X86_64_GOTPC32        26      /* 32 bit signed pc relative offset to GOT */
#define R_X86_64_GOT64          27      /* 64-bit GOT entry offset */
#define R_X86_64_GOTPCREL64     28      /* 64-bit PC relative offset to GOT entry */
#define R_X86_64_GOTPC64        29      /* 64-bit PC relative offset to GOT */
#define R_X86_64_GOTPLT64       30      /* like GOT64, says PLT entry needed */
#define R_X86_64_PLTOFF64       31      /* 64-bit GOT relative offset to PLT entry */
#define R_X86_64_SIZE32         32      /* Size of symbol plus 32-bit addend */
#define R_X86_64_SIZE64         33      /* Size of symbol plus 64-bit addend */
#define R_X86_64_GOTPC32_TLSDESC 34     /* GOT offset for TLS descriptor.  */
#define R_X86_64_TLSDESC_CALL   35      /* Marker for call through TLS descriptor.  */
#define R_X86_64_TLSDESC        36      /* TLS descriptor.  */
#define R_X86_64_IRELATIVE      37      /* Adjust indirectly by program base */
#define R_X86_64_RELATIVE64     38      /* 64-bit adjust by program base */
                                        /* 39 Reserved was R_X86_64_PC32_BND */
                                        /* 40 Reserved was R_X86_64_PLT32_BND */
#define R_X86_64_GOTPCRELX      41      /* Load from 32 bit signed pc relative offset to GOT entry without REX prefix, relaxable.  */
#define R_X86_64_REX_GOTPCRELX  42      /* Load from 32 bit signed pc relative offset to GOT entry with REX prefix, relaxable.  */
#define R_X86_64_NUM            43

elfheader_t* ParseElfHeader(FILE* f, const char* name, int exec);

#endif //__ELFLOADER_PRIVATE_H_
