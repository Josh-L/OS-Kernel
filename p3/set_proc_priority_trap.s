	.globl set_proc_priority_trap
	.even
set_proc_priority_trap:

	jsr set_process_priority_trap_handler
	
	rte
	