	.globl get_proc_priority_trap
	.even
get_proc_priority_trap:

	jsr get_process_priority_trap_handler
	
	rte
	