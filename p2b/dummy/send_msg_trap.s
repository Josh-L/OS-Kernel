	.globl send_msg_trap
	.even
send_msg_trap:
	
	jsr send_message_trap_handler
	
	rte
	