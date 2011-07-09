#include "system.h"

extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[5];

CHAR charIn = 0;

void uart()
{
	while(1)
	{
		/*BYTE temp;
		temp = SERIAL1_UCSR;
		if(temp & 1)
		{
			//CharIn = SERIAL1_RD;
			SERIAL1_IMR = 2;
		}
		else if (temp & 4)
		{
			//SERIAL1_WD = CharOut;
			SERIAL1_IMR = 2;
		}*/
		set_process_priority(1, 0);
		release_processor();
	}
}

void timer()
{
	while(1)
	{
		
		release_processor();
	}
}

void kdc()
{
	int * i;
	rtx_dbug_outs("kdc\r\n");
	receive_message(&i);
}

void crt()
{
	int * i;
	rtx_dbug_outs("crt\r\n");
	receive_message(&i);
}

void c_serial_handler()
{
	// Just schedule the UART i process
	rtx_dbug_out_char('T');
	push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[7]);
}

