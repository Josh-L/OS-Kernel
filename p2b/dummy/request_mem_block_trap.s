	.globl request_mem_block_trap
	.even
request_mem_block_trap:
	
	jsr request_memory_block_trap_handler

	rte
	