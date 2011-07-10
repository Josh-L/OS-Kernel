	.globl get_proc_priority_trap
	.even
get_proc_priority_trap:
	
	move.w #0x2700,%sr
	
	jsr get_process_priority_trap_handler
	
	rte
	