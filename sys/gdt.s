#
# gdt.s
#
#  Created on: Dec 29, 2010
#      Author: cds
#

.text

######
# load a new GDT
#  parameter 1 (rdi): address of gdtr
#  parameter 2 (rsi): new code descriptor offset (code selector)
#  parameter 3 (rdx): new data descriptor offset (data selector)
.global _x86_64_asm_lgdt
_x86_64_asm_lgdt:

	lgdt (%rdi)

	pushq %rsi                  # push code selector
	movabsq $.reload_seg_regs, %r10
	pushq %r10                  # push return address
	lretq                       # far-return to new cs descriptor ( the retq below )
.reload_seg_regs:
	movq %rdx, %es
	movq %rdx, %fs
	movq %rdx, %gs
	movq %rdx, %ds
	movq %rdx, %ss
	retq
