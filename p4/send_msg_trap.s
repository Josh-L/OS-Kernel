	.globl send_msg_trap
	.even
send_msg_trap:
	
	move.w #0x2700,%sr
	
	jsr send_message_trap_handler
	
	rte
	