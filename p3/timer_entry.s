	.globl timer_entry
	.even		
timer_entry:
	
	move.w #0x2700,%sr
	
	jsr	c_timer_handler
	
	rte
	