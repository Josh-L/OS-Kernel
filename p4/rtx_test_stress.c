#include "Hex_to_ASCII.h"
#include "system.h"

void proc_a()
{
	struct s_message * p;
	
	//Register the 'Z' command	
	struct s_message * z = request_memory_block();
	z->type = 2;
	z->msg_text = z + 4;
	strCopy("Z", z->msg_text);
	send_message(10, (VOID *)z);
	
	int num;
	
    while(1) 
	{	
		rtx_dbug_outs((CHAR *)"Process A waiting for Z command.\r\n");
		
		// Receive message
		p = (struct s_messsage *)receive_message(0);
		
		// Check if message was Z command
		if(p->msg_text[0] == 'Z' && p->msg_text[1] == 0)
		{
			rtx_dbug_outs((CHAR *)"Process A received Z command execution message.\r\n");
			release_memory_block((VOID *)p);
			break;
		}
		else
		{
			rtx_dbug_outs((CHAR *)"Process A did not received non-command message.\r\n");
			release_memory_block((VOID *)p);
		}
	}
	
	num = 0;
	while(1)
	{
		rtx_dbug_outs((CHAR *)"Process A sending messages to Process B.\r\n");
		
		p = (struct s_message *)request_memory_block();
		// Message type of 4 is “count_report"
		p->type = 4;
		p->msg_text = p + 4;
		p->msg_text[0] = num;
		
		rtx_dbug_outs((CHAR *)"Process A sending num = ");
		printHexAddress(p->msg_text[0]);
		rtx_dbug_outs((CHAR *)"\r\n");
		
		// Send message to process B
		send_message(8, (VOID *)p);
		num++;
		release_processor();
	}
}

void proc_b()
{
	VOID * tmp;
    while(1)
    {
		rtx_dbug_outs((CHAR *)"Process B waiting for message.\r\n");
		tmp = receive_message(0);
		rtx_dbug_outs((CHAR *)"Process B forwarding message to process C.\r\n");
		// Send message to process C
		send_message(9, (VOID *)tmp);
	}	
 }


void proc_c()
{		
	// Create and set up local message queue
    struct	s_message_queue l_msg_queue;
	struct	s_message_queue_item l_msg_queue_slots[NUM_MEM_BLKS];
	l_msg_queue.num_slots = NUM_MEM_BLKS;
	l_msg_queue.front = 0;
	l_msg_queue.back = 0;
	
	UINT8 i;
	for(i = 0; i < NUM_MEM_BLKS; i++)
	{
		l_msg_queue_slots[i].data = 0;
		l_msg_queue_slots[i].next = 0;
	}
	
	struct s_message *  p;
	struct s_message *  q;
	
	int count;
	
	while(1)
    {
		rtx_dbug_outs((CHAR *)"Process C checking local queue.\r\n");
		//Check if local queue is empty, if empty wait for message from global queue
		if(message_pop(&l_msg_queue, l_msg_queue_slots, &p) == -1)
		{
			rtx_dbug_outs((CHAR *)"Process C no local messages found.\r\n");
			p = receive_message(0);
		}
		
		// Check if message type is of type “count_report"
		if(p->type == 4)
		{
			count = (int)p->msg_text[0];
			
			rtx_dbug_outs((CHAR *)"Process C got count_report = ");
			printHexAddress(count);
			rtx_dbug_outs((CHAR *)"\n\r");
			
			// Check if contents at msg_data[0] are divisble by 20
			if(count%20 == 0)
			{
				rtx_dbug_outs((CHAR *)"Process C writing to UART1.\r\n");
				//Send a message to CRT
				p->type = 3;
				p->msg_text = p + 4;
				strCopy("Process C\n\r", p->msg_text);
				send_message(11, (VOID *)p);
				
				// To hibernate for 10 seconds, delay send itself a message
				q = (struct s_message *)request_memory_block();
				q->type = 5; //Message Type of 5 is "WakeUp10"
				delayed_send(9, (VOID *)q, 10);
				
				rtx_dbug_outs((CHAR *)"Process C hibernating.\r\n");
				
				while(1)
				{	
					p = (struct s_message *)receive_message(0);
					
					// Check if received message is WakeUp10, signalling hibernation of 10 seconds is over.
					if(p->type == 5)
					{
						//break if true
						rtx_dbug_outs((CHAR *)"Process C got wake-up message.\r\n");
						break;
					}
					else
					{
						rtx_dbug_outs((CHAR *)"Process C got message while hibernating, adding to queue.\r\n");
						message_push(&l_msg_queue, l_msg_queue_slots, p);
					}
				}
			}
		}
		// Release p's memory block
		release_memory_block((VOID *)p);
		release_processor();
		
    }
}