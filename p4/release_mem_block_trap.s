	.globl release_mem_block_trap
	.even
release_mem_block_trap:
	
	move.w #0x2700,%sr
	
	jsr release_memory_block_trap_handler

	rte
	