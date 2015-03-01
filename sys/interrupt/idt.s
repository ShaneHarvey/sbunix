#
# idt.s
#
#  Created on: Dec 31, 2010
#      Author: cds
#

.text

######
# load a new IDT
#  parameter 1: virtual address of idtr
.global _x86_64_asm_lidt
_x86_64_asm_lidt:

    cli
    lidt (%rdi)
    sti
    retq