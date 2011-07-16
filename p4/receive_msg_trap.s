	.globl receive_msg_trap
	.even
receive_msg_trap:
	
	move.w #0x2700,%sr
	
	jsr receive_message_trap_handler
	
	rte
	