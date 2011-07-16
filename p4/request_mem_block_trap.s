	.globl request_mem_block_trap
	.even
request_mem_block_trap:
	
	move.w #0x2700,%sr
	
	jsr request_memory_block_trap_handler

	rte
	