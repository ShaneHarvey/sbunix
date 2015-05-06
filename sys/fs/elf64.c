#include <sbunix/sbunix.h>
#include <sbunix/fs/elf64.h>
#include <sbunix/fs/tarfs.h>
#include <sbunix/mm/vmm.h>
#include <sbunix/mm/pt.h>
#include <sbunix/string.h>
#include <errno.h>

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
 * Print info about the elf headers
 */
void print_elf_info(Elf64_Ehdr *hdr) {
    Elf64_Half i;
    Elf64_Shdr *shdr;
    Elf64_Phdr *phdr;
    if(!hdr)
        return;
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
    printk("\nProgram headers:\nType  FileOffset  FileSiz  MemSiz  VirtAddr  PhysAddr  Flag\n");
    for(i = 0; i < hdr->e_phnum; i++) {
        phdr = elf_pheader(hdr, i);
        elf_print_phdr(phdr);
    }
}

/**
 * Validate the given file is a supported ELF file
 *
 * @fp: opened file for reading
 * @return: 0 on success, -ERRNO on error
 */
int elf_validiate_exec(struct file *fp) {
    Elf64_Ehdr *hdr;
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

    /* To be valid we must have a loadable segment */
    for(i = 0; i < hdr->e_phnum; i++) {
        phdr = elf_pheader(hdr, i);
        if(phdr->p_type == PT_LOAD)
            return 0;
    }
    return -ENOEXEC;
}

/**
 * Validate the given file is a supported ELF file
 *
 * @fp: opened valid ELF file
 * @return: 0 on success, -ERRNO on error
 */
int elf_load(struct file *fp, struct mm_struct *mm) {
    Elf64_Ehdr *hdr;
    Elf64_Phdr *phdr;
    Elf64_Half i;
    int err;
    if(!fp)
        kpanic("file is NULL!");
    if(!mm)
        kpanic("mm is NULL!");

    /* Hack: we know it's a tarfs file in memory, so just get the data pointer */
    hdr = (Elf64_Ehdr *)(((char *)fp->private_data) + sizeof(struct posix_header_ustar));
    mm->user_rip = hdr->e_entry; /* save virtual entry point */
    for(i = 0; i < hdr->e_phnum; i++) {
        phdr = elf_pheader(hdr, i);
        if(phdr->p_type == PT_LOAD) {
            ulong prot = (phdr->p_flags & PF_W)? PFLAG_RW: 0;
            /* TODO: Enable NXE bit for page tables */
            /*prot |= (phdr->p_flags & PF_X)? 0: PFLAG_NXE;*/
            /*elf_print_phdr(phdr);*/
            err = mmap_area(mm, fp, phdr->p_offset, phdr->p_filesz, prot,
                      phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz);
            if(err) {
                return err;
            }
        }
    }
    return 0;
}

/* Type   FileOffset   FileSiz   MemSiz   VirtAddr   PhysAddr   Flag */
void elf_print_phdr(Elf64_Phdr *phdr) {
    char *type;
    Elf64_Word flags = phdr->p_flags;
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
        case PT_GNU_RELRO: type = "GNU_RELRO"; break;
        default: type = "UNKOWN";
    }
    printk("%s, %p, %p, %p, %p, %p, %s%s%s\n", type, phdr->p_offset, phdr->p_filesz,
           phdr->p_memsz, phdr->p_vaddr, phdr->p_paddr, (flags & PF_R)?"R":"",
           (flags & PF_W)?"W":"", (flags & PF_X)?"X":"");
}

void elf_test_load(char *filename) {
    struct file *fp;
    int err;

    fp = tarfs_open(filename, 0, 0, &err);
    if(err) {
        printk("Error opening: %s: %s\n", filename, strerror(-err));
        return;
    }
    elf_validiate_exec(fp);
    err = fp->f_op->close(fp);
    if(err) {
        printk("Error close: %s\n", strerror(-err));
        return;
    }
}