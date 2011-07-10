	.globl asm_serial_entry
	.even		
asm_serial_entry:
	
	move.w #0x2700,%sr
	
	jsr	c_serial_handler
	
	rte
	