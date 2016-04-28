#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

char *p_type(u32 type)
{
    switch (type) {
    case PT_NULL        : return "PT_NULL";
    case PT_LOAD        : return "PT_LOAD";
    case PT_DYNAMIC     : return "PT_DYNAMIC";
    case PT_INTERP      : return "PT_INTERP";
    case PT_NOTE        : return "PT_NOTE";
    case PT_SHLIB       : return "PT_SHLIB";
    case PT_PHDR        : return "PT_PHDR";
    case PT_TLS         : return "PT_TLS";
    case PT_NUM         : return "PT_NUM";
    case PT_LOOS        : return "PT_LOOS";
    case PT_GNU_EH_FRAME: return "PT_GNU_EH_FRAME";
    case PT_GNU_STACK   : return "PT_GNU_STACK";
    case PT_GNU_RELRO   : return "PT_GNU_RELRO";
    case PT_LOSUNW      : return "PT_LOSUNW PT_SUNWBSS";
    case PT_SUNWSTACK   : return "PT_SUNWSTACK";
    case PT_HISUNW      : return "PT_HISUNW PT_HIOS";
    case PT_LOPROC      : return "PT_LOPROC";
    case PT_HIPROC      : return "PT_HIPROC";
    default: return "unknown";
    }
}

char *sh_type(u32 type)
{
    switch (type) {
    case SHT_NULL          : return "SHT_NULL";
    case SHT_PROGBITS      : return "SHT_PROGBITS";
    case SHT_SYMTAB        : return "SHT_SYMTAB";
    case SHT_STRTAB        : return "SHT_STRTAB";
    case SHT_RELA          : return "SHT_RELA";
    case SHT_HASH          : return "SHT_HASH";
    case SHT_DYNAMIC       : return "SHT_DYNAMIC";
    case SHT_NOTE          : return "SHT_NOTE";
    case SHT_NOBITS        : return "SHT_NOBITS";
    case SHT_REL           : return "SHT_REL";
    case SHT_SHLIB         : return "SHT_SHLIB";
    case SHT_DYNSYM        : return "SHT_DYNSYM";
    case SHT_INIT_ARRAY    : return "SHT_INIT_ARRAY";
    case SHT_FINI_ARRAY    : return "SHT_FINI_ARRAY";
    case SHT_PREINIT_ARRAY : return "SHT_PREINIT_ARRAY";
    case SHT_GROUP         : return "SHT_GROUP";
    case SHT_SYMTAB_SHNDX  : return "SHT_SYMTAB_SHNDX";
    case SHT_NUM           : return "SHT_NUM";
    case SHT_LOOS          : return "SHT_LOOS";
    case SHT_GNU_ATTRIBUTES: return "SHT_GNU_ATTRIBUTES";
    case SHT_GNU_HASH      : return "SHT_GNU_HASH";
    case SHT_GNU_LIBLIST   : return "SHT_GNU_LIBLIST";
    case SHT_CHECKSUM      : return "SHT_CHECKSUM";
    case SHT_LOSUNW        : return "SHT_LOSUNW SHT_SUNW_move";
    case SHT_SUNW_COMDAT   : return "SHT_SUNW_COMDAT";
    case SHT_SUNW_syminfo  : return "SHT_SUNW_syminfo";
    case SHT_GNU_verdef    : return "SHT_GNU_verdef";
    case SHT_GNU_verneed   : return "SHT_GNU_verneed";
    case SHT_GNU_versym    : return "SHT_GNU_versym SHT_HISUNW SHT_HIOS";
    case SHT_LOPROC        : return "SHT_LOPROC";
    case SHT_HIPROC        : return "SHT_HIPROC";
    case SHT_LOUSER        : return "SHT_LOUSER";
    case SHT_HIUSER        : return "SHT_HIUSER";
    default: return "unknown";
    }
}

char *symbol_type(unsigned char st_info)
{
    switch (ELF64_ST_TYPE(st_info)) {
    case STT_NOTYPE: return "STT_NOTYPE";
    case STT_OBJECT: return "STT_OBJECT";
    case STT_FUNC: return "STT_FUNC";
    case STT_SECTION: return "STT_SECTION";
    case STT_FILE: return "STT_FILE";
    case STT_COMMON: return "STT_COMMON";
    case STT_TLS: return "STT_TLS";
    case STT_NUM: return "STT_NUM";
    case STT_LOOS: return "STT_LOOS STT_GNU_IFUNC";
    case STT_HIOS: return "STT_HIOS";
    case STT_LOPROC: return "STT_LOPROC";
    case STT_HIPROC: return "STT_HIPROC";
    default: return "unknown";
    }
}

char *symbol_binding(unsigned char st_info)
{
    switch (ELF64_ST_BIND(st_info)) {
    case STB_LOCAL: return "STB_LOCAL";
    case STB_GLOBAL: return "STB_GLOBAL";
    case STB_WEAK: return "STB_WEAK";
    case STB_NUM: return "STB_NUM";
    case STB_LOOS: return "STB_LOOS STB_GNU_UNIQUE";
    case STB_HIOS: return "STB_HIOS";
    case STB_LOPROC: return "STB_LOPROC";
    case STB_HIPROC: return "STB_HIPROC";
    default: return "unknown";
    }
}

char *special_seciton_name(u16 index)
{
    switch (index) {
    case SHN_UNDEF: return "SHN_UNDEF";
    case SHN_LORESERVE: return "SHN_LORESERVE SHN_LOPROC";
    case SHN_HIPROC: return "SHN_HIPROC";
    case SHN_LOOS: return "SHN_LOOS";
    case SHN_HIOS: return "SHN_HIOS";
    case SHN_ABS: return "SHN_ABS";
    case SHN_COMMON: return "SHN_COMMON";
    case SHN_XINDEX: return "SHN_XINDEX SHN_HIRESERVE";
    default: return "unknown";
    }
}

char *d_tag(uint64_t tag)
{
    switch (tag) {
    case DT_NULL: return "DT_NULL";
    case DT_NEEDED: return "DT_NEEDED";
    case DT_PLTRELSZ: return "DT_PLTRELSZ";
    case DT_PLTGOT: return "DT_PLTGOT";
    case DT_HASH: return "DT_HASH";
    case DT_STRTAB: return "DT_STRTAB";
    case DT_SYMTAB: return "DT_SYMTAB";
    case DT_RELA: return "DT_RELA";
    case DT_RELASZ: return "DT_RELASZ";
    case DT_RELAENT: return "DT_RELAENT";
    case DT_STRSZ: return "DT_STRSZ";
    case DT_SYMENT: return "DT_SYMENT";
    case DT_INIT: return "DT_INIT";
    case DT_FINI: return "DT_FINI";
    case DT_SONAME: return "DT_SONAME";
    case DT_RPATH: return "DT_RPATH";
    case DT_SYMBOLIC: return "DT_SYMBOLIC";
    case DT_REL: return "DT_REL";
    case DT_RELSZ: return "DT_RELSZ";
    case DT_RELENT: return "DT_RELENT";
    case DT_PLTREL: return "DT_PLTREL";
    case DT_DEBUG: return "DT_DEBUG";
    case DT_TEXTREL: return "DT_TEXTREL";
    case DT_JMPREL: return "DT_JMPREL";
    case DT_BIND_NOW: return "DT_BIND_NOW";
    case DT_INIT_ARRAY: return "DT_INIT_ARRAY";
    case DT_FINI_ARRAY: return "DT_FINI_ARRAY";
    case DT_INIT_ARRAYSZ: return "DT_INIT_ARRAYSZ";
    case DT_FINI_ARRAYSZ: return "DT_FINI_ARRAYSZ";
    case DT_RUNPATH: return "DT_RUNPATH";
    case DT_FLAGS: return "DT_FLAGS";
    case DT_ENCODING: return "DT_ENCODING DT_PREINIT_ARRAY";
    case DT_PREINIT_ARRAYSZ: return "DT_PREINIT_ARRAYSZ";
    case DT_NUM: return "DT_NUM";
    case DT_LOOS: return "DT_LOOS";
    case DT_HIOS: return "DT_HIOS";
    case DT_LOPROC: return "DT_LOPROC";
    case DT_HIPROC: return "DT_HIPROC";
    case DT_PROCNUM: return "DT_PROCNUM";
    default: return "unknown";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/elf64_file\n", argv[0]);
        return 1;
    }

    int fd;
    struct stat buf;
    u8 *f;

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (fstat(fd, &buf) == -1) {
        perror("fstat");
        return 1;
    }

    f = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (f == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    Elf64_Ehdr *elf_header = (Elf64_Ehdr *)f;
    printf("ELF header:\n");
    printf("    header size = %#x\n", elf_header->e_ehsize);
    printf("    program header table at = %#lx, entry count = %d, entry size = %#x\n",
                                    elf_header->e_phoff,
                                    elf_header->e_phnum,
                                    elf_header->e_phentsize);
    printf("    section header table at = %#lx, entry count = %d, entry size = %#x\n",
                                    elf_header->e_shoff,
                                    elf_header->e_shnum,
                                    elf_header->e_shentsize);
    printf("    e_entry = %#lx\n", elf_header->e_entry);
    printf("    e_type  = %#x (ET_EXEC = %#x, ET_DYN = %#x, ET_CORE = %#x)\n",
                                elf_header->e_type,
                                ET_EXEC,
                                ET_DYN,
                                ET_CORE);
    printf("    e_machine = %#x (ET_X86_64 = %#x)\n", elf_header->e_machine, EM_X86_64);
    printf("    e_shstrndx = %d\n", elf_header->e_shstrndx);

    printf("\nProgram header table:\n");
    Elf64_Phdr *program_header_table = (Elf64_Phdr *)(f + elf_header->e_phoff);
    int i;
    Elf64_Phdr *ph;
    u8 *pt_note;
    int pt_note_count, pt_note_namesize, pt_note_descsize;
    Elf64_Dyn *dyn;
    for (i = 0; i < elf_header->e_phnum; i++) {
        ph = program_header_table + i;
        printf("    %4d: ", i);
        printf("type = %-16s", p_type(ph->p_type));
        if (ph->p_flags & PF_R) {
            printf("R");
        }
        if (ph->p_flags & PF_W) {
            printf("W");
        }
        if (ph->p_flags & PF_X) {
            printf("X");
        }
        printf("\t");
        printf("offset = %#lx size = %ld ", ph->p_offset, ph->p_filesz);
        if (ph->p_type == PT_LOAD) {
            printf("p_vaddr = %#lx p_memsz = %#lx ", ph->p_vaddr, ph->p_memsz);
        }
        printf("\n");
        if (ph->p_type == PT_INTERP) {
            printf("\t\t%s\n", (char *)(f + ph->p_offset));
        }
        if (ph->p_type == PT_NOTE) {
            pt_note = (u8 *)(f + ph->p_offset);
            while (pt_note_count < ph->p_filesz) {
                printf("\t\t");
                pt_note_namesize = (u32)(*pt_note);
                printf("name size = %d, ", pt_note_namesize);
                pt_note += 4;
                pt_note_count += 4;
                pt_note_descsize = (u32)(*pt_note);
                printf("desc size = %d, ", pt_note_descsize);
                pt_note += 4;
                pt_note_count += 4;
                printf("type = %d\n", (u32)(*pt_note));
                pt_note += 4;
                pt_note_count += 4;
                printf("\t\tname: ");
                while (pt_note_namesize--) {
                    printf("%#x ", *pt_note);
                    if (*pt_note) {
                        printf("(%c) ", *pt_note);
                    }
                    pt_note++;
                    pt_note_count++;
                }
                printf("\n");
                printf("\t\tdesc: ");
                while (pt_note_descsize--) {
                    printf("%#x ", *pt_note);
                    pt_note++;
                    pt_note_count++;
                }
                printf("\n");
            }
        } else if (ph->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn *)(f + ph->p_offset);
            while (dyn->d_tag != DT_NULL) {
                printf("\t\td_tag = %-20s d_un = %#lx\n", d_tag(dyn->d_tag), dyn->d_un.d_val);
                dyn = dyn + 1;
            }
        }
    }

    Elf64_Shdr *section_header_table = (Elf64_Shdr *)(f + elf_header->e_shoff);
    Elf64_Shdr *name_sh = NULL;
    printf("\nFind section name string table:\n");
    if (elf_header->e_shstrndx != SHN_UNDEF) {
        name_sh = section_header_table + elf_header->e_shstrndx;
        printf("    offset = %#lx, size = %ld\n", name_sh->sh_offset, name_sh->sh_size);
    }

    printf("\nSection header table:\n");
    Elf64_Shdr *sh;
    Elf64_Shdr *symtab_sh = NULL, *dynsym_sh = NULL,
               *strtab_sh = NULL, *dynstr_sh = NULL,
               *got_sh    = NULL, *gotplt_sh = NULL;
    char *name;
    Elf64_Rela *rela_sh;
    int rela_count, rela_idx;
    for (i = 0; i < elf_header->e_shnum; i++) {
        sh = section_header_table + i;
        printf("    %4d: ", i);
        if (name_sh) {
            name = (char *)(f + name_sh->sh_offset + sh->sh_name);
            printf("name = %-20s ", name);
            if (sh->sh_type == SHT_STRTAB) {
                if (strcmp(name, ".strtab") == 0) {
                    strtab_sh = sh;
                } else if (strcmp(name, ".dynstr") == 0) {
                    dynstr_sh = sh;
                }
            }
            if (strcmp(name, ".got") == 0) {
                got_sh = sh;
            }
            if (strcmp(name, ".got.plt") == 0) {
                gotplt_sh = sh;
            }
        }
        printf("type = %-16s ", sh_type(sh->sh_type));
        printf("offset = %#lx ", sh->sh_offset);
        printf("size = %ld ", sh->sh_size);
        printf("\n");
        if (sh->sh_type == SHT_SYMTAB) {
            symtab_sh = sh;
        } else if (sh->sh_type == SHT_DYNSYM) {
            dynsym_sh = sh;
        } else if (sh->sh_type == SHT_RELA) {
            printf("\t\tsh_link = %s, sh_info = %s\n",
                                (f + name_sh->sh_offset + (section_header_table + sh->sh_link)->sh_name),
                                (f + name_sh->sh_offset + (section_header_table + sh->sh_info)->sh_name));
            rela_sh = (Elf64_Rela *)(f + sh->sh_offset);
            rela_count = sh->sh_size / sizeof(Elf64_Rela);
            rela_idx = 0;
            while (rela_idx < rela_count - 1) {
                rela_sh = rela_sh + 1;
                printf("\t\tr_offset = %#lx, r_info = (SYM = %lu, TYPE = %#lx), r_addend = %#lx\n",
                                    rela_sh->r_offset,
                                    ELF64_R_SYM(rela_sh->r_info),
                                    ELF64_R_TYPE(rela_sh->r_info),
                                    rela_sh->r_addend);
                rela_idx++;
            }
        }
    }

    if (symtab_sh && strtab_sh) {
        printf("\nFound symbol table:\n");
        Elf64_Sym *symtab = (Elf64_Sym *)(f + symtab_sh->sh_offset);
        int count = symtab_sh->sh_size / symtab_sh->sh_entsize;
        Elf64_Sym *sym;
        for (i = 0; i < count; i++) {
            sym = symtab + i;
            printf("    %d: ", i);
            printf("name = %-30s ", f + strtab_sh->sh_offset + sym->st_name);
            printf("value = %#-8lx   ", sym->st_value);
            printf("size = %#-8lx ", sym->st_size);
            printf("type = %-16s ", symbol_type(sym->st_info));
            printf("binding = %-16s ", symbol_binding(sym->st_info));
            printf("defined section index = ");
            if (sym->st_shndx > 0 && sym->st_shndx < elf_header->e_shnum - 1) {
                printf("%s", f + name_sh->sh_offset + (section_header_table + sym->st_shndx)->sh_name);
            } else {
                printf("%s", special_seciton_name(sym->st_shndx));
            }
            printf("\n");
        }
    }

    if (dynsym_sh && dynstr_sh) {
        printf("\nFound dynamic linking symbol table:\n");
        Elf64_Sym *symtab = (Elf64_Sym *)(f + dynsym_sh->sh_offset);
        int count = dynsym_sh->sh_size / dynsym_sh->sh_entsize;
        Elf64_Sym *sym;
        for (i = 0; i < count; i++) {
            sym = symtab + i;
            printf("    %d: ", i);
            printf("name = %-30s ", f + dynstr_sh->sh_offset + sym->st_name);
            printf("value = %#-8lx   ", sym->st_value);
            printf("size = %#-8lx ", sym->st_size);
            printf("type = %-16s ", symbol_type(sym->st_info));
            printf("binding = %-16s ", symbol_binding(sym->st_info));
            printf("defined section index = ");
            if (sym->st_shndx > 0 && sym->st_shndx < elf_header->e_shnum - 1) {
                printf("%s", f + name_sh->sh_offset + (section_header_table + sym->st_shndx)->sh_name);
            } else {
                printf("%s", special_seciton_name(sym->st_shndx));
            }
            printf("\n");
        }
    }

    if (got_sh) {
        printf("\nGOT:\n");
        int count = got_sh->sh_size / 8;
        uint64_t *got = (uint64_t *)(f + got_sh->sh_offset);
        for (i = 0; i < count; i++) {
            printf("%d: %#lx\n", i, *got);
            got++;
        }
    }

    if (gotplt_sh) {
        printf("\nGOT.PLT:\n");
        int count = gotplt_sh->sh_size / 8;
        uint64_t *gotplt = (uint64_t *)(f + gotplt_sh->sh_offset);
        for (i = 0; i < count; i++) {
            printf("%d: %#lx\n", i, *gotplt);
            gotplt++;
        }
    }

    munmap(f, buf.st_size);
    close(fd);

    return 0;
}
