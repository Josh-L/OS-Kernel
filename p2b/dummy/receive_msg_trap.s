	.globl receive_msg_trap
	.even
receive_msg_trap:
	
	jsr receive_message_trap_handler
	
	rte
	