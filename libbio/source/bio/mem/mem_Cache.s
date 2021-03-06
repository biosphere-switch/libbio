.macro CODE_BEGIN name
	.section .text.\name, "ax", %progbits
	.global \name
	.type \name, %function
	.align 2
	.cfi_startproc
\name:
.endm

.macro CODE_END
	.cfi_endproc
.endm

CODE_BEGIN _ZN3bio3mem14FlushDataCacheEPvy
	add x1, x1, x0
	mrs x8, CTR_EL0
	lsr x8, x8, #16
	and x8, x8, #0xf
	mov x9, #4
	lsl x9, x9, x8
	sub x10, x9, #1
	bic x8, x0, x10
	mov x10, x1

data_cache_flush_l0:
	dc  civac, x8
	add x8, x8, x9
	cmp x8, x10
	bcc data_cache_flush_l0

	dsb sy
	ret
CODE_END

CODE_BEGIN _ZN3bio3mem14CleanDataCacheEPvy
	add x1, x1, x0
	mrs x8, CTR_EL0
	lsr x8, x8, #16
	and x8, x8, #0xf
	mov x9, #4
	lsl x9, x9, x8
	sub x10, x9, #1
	bic x8, x0, x10
	mov x10, x1

data_cache_clean_l0:
	dc  cvac, x8
	add x8, x8, x9
	cmp x8, x10
	bcc data_cache_clean_l0

	dsb sy
	ret
CODE_END

CODE_BEGIN _ZN3bio3mem26InvalidateInstructionCacheEPvy
	add x1, x1, x0
	mrs x8, CTR_EL0
	and x8, x8, #0xf
	mov x9, #4
	lsl x9, x9, x8
	sub x10, x9, #1
	bic x8, x0, x10
	mov x10, x1

instruction_cache_invalidate_l0:
	ic  ivau, x8
	add x8, x8, x9
	cmp x8, x10
	bcc instruction_cache_invalidate_l0

	dsb sy
	ret
CODE_END

CODE_BEGIN _ZN3bio3mem13ZeroDataCacheEPvy
	add x1, x1, x0
	mrs x8, CTR_EL0
	lsr x8, x8, #16
	and x8, x8, #0xf
	mov x9, #4
	lsl x9, x9, x8
	sub x10, x9, #1
	bic x8, x0, x10
	mov x10, x1

data_cache_zero_l0:
	dc  zva, x8
	add x8, x8, x9
	cmp x8, x10
	bcc data_cache_zero_l0

	dsb sy
	ret
CODE_END
