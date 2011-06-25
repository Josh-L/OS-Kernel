	.globl trap_1_handler
	.even
trap_1_handler:

	jsr set_process_priority_trap_handler
	
	rte
	