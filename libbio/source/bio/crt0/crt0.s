.section .text.jmp, "x"

.global _start
_start:
	b _entry
	.word __module_header - _start

.section .data.mod0

.global __module_header
__module_header:
	.ascii "MOD0"
	.word __dynamic_start - __module_header
	.word __bss_start - __module_header
	.word __bss_end - __module_header
	.word __eh_frame_hdr_start - __module_header
	.word __eh_frame_hdr_end - __module_header
	// Runtime-generated module object offset - ignore it
	.word 0

.section .text, "x"

__normal_entry:
	// Set aslr base address as 3rd argument
	adrp x2, _start

	// Set lr as 4th argument
	mov x3, x30

	// Set .bss start and end as 5th and 6th arguments
	adrp x4, __bss_start
	add x4, x4, #:lo12:__bss_start
	adrp x5, __bss_end
	add x5, x5, #:lo12:__bss_end
	
	// bio::crt0::NormalEntry(context_ptr, main_thread_handle_v, aslr_base_address, lr, bss_start, bss_end)
	b _ZN3bio4crt011NormalEntryEPvyS1_PFviES1_S1_

__exception_entry:
	// bio::crt0::ExceptionEntry(error_desc, stack_top)
	b _ZN3bio4crt014ExceptionEntryENS0_20ExceptionDescriptionEPv

// Actual entrypoint called

_entry:
	// Determine which entry we need to call (if x0 != 0 and x1 != -1, we're being called to handle an exception)
	cmp x0, #0
    ccmn x1, #1, #4, ne
    beq __normal_entry
    bne __exception_entry