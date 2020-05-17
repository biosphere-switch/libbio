.section .text.jmp, "x"

.global _start
_start:
	b _entry
	.word __module_header - _start

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

.global _entry
_entry:
	// Get aslr base address as 3rd argument
	adrp x2, _start
	// Save lr as 4th argument
	mov x3, x30
	// Execute - bio::crt0::Entry(context_args_ptr, main_thread_handle_v, aslr_base_address, exit_lr)
	b _ZN3bio4crt05EntryEPvyS1_PFviE