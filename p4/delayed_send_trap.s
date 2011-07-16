	.globl delayed_send_trap
	.even
delayed_send_trap:
	
	move.w #0x2700,%sr
	
	jsr delayed_send_trap_handler
	
	rte
	