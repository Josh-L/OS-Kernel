	.globl release_mem_block_trap
	.even
release_mem_block_trap:

	jsr release_memory_block_trap_handler

	rte
	