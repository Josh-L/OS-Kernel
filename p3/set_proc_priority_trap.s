	.globl set_proc_priority_trap
	.even
set_proc_priority_trap:
	
	move.w #0x2700,%sr
	
	jsr set_process_priority_trap_handler
	
	rte
	