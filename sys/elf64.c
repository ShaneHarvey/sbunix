#include <sbunix/elf64.h>
#include <sbunix/vfs/vfs.h>
#include <sbunix/tarfs.h>
#include <sbunix/sbunix.h>
#include <errno.h>
#include <sbunix/string.h>

/*
 * Simple ELF x86-64 file loader, probably need to support
 * .text
 * .rodata
 * .got  and .got.plt
 * .bss
 * .data
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
    return -ENOEXEC;
}