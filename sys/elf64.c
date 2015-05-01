#include <sbunix/elf64.h>
#include <sbunix/tarfs.h>
#include <sbunix/sbunix.h>
#include <errno.h>
#include <sbunix/string.h>

/*
 * Simple ELF x86-64 file loader, probably need to support
 * .text
 * .rodata
 * .got  and .got.plt
 * .data
 * .bss
 */

int validate_elf64_hdr(Elf64_Ehdr *hdr) {
    unsigned char *e_ident;
    if(!hdr)
        return -ENOEXEC;

    /* valid e_indent */
    e_ident = hdr->e_ident;
    if(memcmp(e_ident, ELFMAG, 4) != 0 ||
            e_ident[EI_CLASS] != ELFCLASS64 ||
            e_ident[EI_DATA] != ELFDATA2LSB ||
            e_ident[EI_VERSION] != EV_CURRENT ||
            (e_ident[EI_OSABI] != ELFOSABI_NONE &&
             e_ident[EI_OSABI] != ELFOSABI_LINUX))
        return -ENOEXEC;

    /* Must be an executable */
    if(hdr->e_type != ET_EXEC)
        return -ENOEXEC;
    /* Only support x86-64 */
    if(hdr->e_machine != EM_X86_64)
        return -ENOEXEC;
    /* Only support original version */
    if(hdr->e_version != EV_CURRENT)
        return -ENOEXEC;

    return 0;
}


/**
 * Validate the given file is a supported ELF file
 *
 * @fp: opened file for reading
 * @return: 0 on success, -ERRNO on error
 */
int elf_validiate_exec(struct file *fp) {
    Elf64_Ehdr *hdr;
    Elf64_Shdr *shdr;
    Elf64_Phdr *phdr;
    Elf64_Half i;
    int err;
    if(!fp)
        kpanic("file is NULL!");

    if(fp->f_size < sizeof(Elf64_Ehdr))
        return -ENOEXEC;

    /* Hack: we know it's a tarfs file in memory, so just get the data pointer */
    hdr = (Elf64_Ehdr *)(((char *)fp->private_data) + sizeof(struct posix_header_ustar));
    err = validate_elf64_hdr(hdr);
    if(err)
        return err;
    /* Valid, try searching for .text, .rodata, .got, .got.plt, .data, and .bss */
    printk("ELF virtual entry point: %p\n", hdr->e_entry);
    /* First entry is NULL */
    printk("Section headers:\n");
    for(i = 0; i < hdr->e_shnum; i++) {
        char *name;
        shdr = elf_sheader(hdr, i);
        if(shdr->sh_name == SHN_UNDEF) {
            name = "NO_NAME";
        } else {
            name = elf_lookup_string(hdr, shdr->sh_name);
        }
        printk("%s, ", name);
    }
    printk("\nProgram headers:\nType   Offset   VirtAddr   PhysAddr   FileSiz   MemSiz   Flag\n");
    for(i = 0; i < hdr->e_phnum; i++) {
        phdr = elf_pheader(hdr, i);
        elf_print_phdr(phdr);
    }
    return 0;
}

/* Type   Offset   VirtAddr   PhysAddr   FileSiz   MemSiz   Flag */
void elf_print_phdr(Elf64_Phdr *phdr) {
    char *type;
    switch(phdr->p_type) {
        case PT_NULL: type = "NULL"; break;
        case PT_LOAD: type = "LOAD"; break;
        case PT_DYNAMIC: type = "DYNAMIC"; break;
        case PT_INTERP: type = "INTERP"; break;
        case PT_NOTE: type = "NOTE"; break;
        case PT_SHLIB: type = "SHLIB"; break;
        case PT_PHDR: type = "PHDR"; break;
        case PT_TLS: type = "TLS"; break;
        case PT_LOOS: type = "LOOS"; break;
        case PT_HIOS: type = "HIOS"; break;
        case PT_LOPROC: type = "LOPROC"; break;
        case PT_HIPROC: type = "HIPROC"; break;
        case PT_GNU_EH_FRAME: type = "GNU_EH_FRAME"; break;
        case PT_GNU_STACK: type = "GNU_STACK"; break;
        default: type = "UNKOWN";
    }
    printk("%s, %p, %p, %p, %p, %p, %p\n", type, phdr->p_filesz, phdr->p_offset,
           phdr->p_vaddr, phdr->p_paddr, phdr->p_memsz, (uint64_t)phdr->p_flags);
}

void elf_test_load(char *filename) {
    struct file f, *fp = &f;
    int err;

    fp->f_op = &tarfs_file_ops;
    fp->f_count = 1;
    err = fp->f_op->open(filename, fp);
    if(err) {
        printk("Error open: %s\n", strerror(-err));
        return;
    }
    elf_validiate_exec(fp);
    err = fp->f_op->close(fp);
    if(err) {
        printk("Error close: %s\n", strerror(-err));
        return;
    }
}