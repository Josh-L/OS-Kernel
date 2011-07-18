#include "Hex_to_ASCII.h"
#include "system.h"


void proc_a()
{
	struct s_message * p;
	
	//Register Command w/ KCD	
	struct s_message * z = request_memory_block();
	z->type = 2;
	z->msg_text = z + 4;
	strCopy("Z", z->msg_text);
	send_message(10, (VOID *)z);
	
	
    while(1) 
	{	
		rtx_dbug_outs((CHAR *)"a\r\n");
		
		//Receive Message
		p = receive_message(0);
		VOID * m;
		m = (VOID *)p;
		m+=64;
		CHAR * d;
		d = m;
		//Check if message was directed at Process A from UART 
		if(d[0] == 'Z'){
			rtx_dbug_outs((CHAR *)"a1\r\n");
			release_memory_block((VOID *)p);
			break;	
		}else{
			rtx_dbug_outs((CHAR *)"a2\r\n");
			release_memory_block((VOID *)p);
		}
	
	
	}
	int num = 0;
	while(1){
		rtx_dbug_outs((CHAR *)"a3\r\n");
		p = (struct s_message *)request_memory_block();
		//message type of 4 is “count_report"
		p->type = 4;
		VOID * m;
		m = p;
		m+=64;
		int * d;
		d = m;
		*d = num;
		//Send message to process B
		send_message(8, (VOID *)p);
		num = num + 1;
		release_processor();
	}
}

void proc_b()
{
	rtx_dbug_out_char('b');
	rtx_dbug_outs((CHAR *)"\r\n");
	VOID * tmp;
    while (1)
    {
		rtx_dbug_outs((CHAR *)"b1\r\n");
		tmp = receive_message(0);
		//Send message to process C
		send_message(9, (VOID *)tmp);
	}	
 }


void proc_c()
{

	rtx_dbug_out_char('c');
	rtx_dbug_outs((CHAR *)"\r\n");
		
	//Create and set up local message queue
    struct	s_message_queue l_msg_queue;
	struct	s_message_queue_item l_msg_queue_slots[NUM_MEM_BLKS];
	l_msg_queue.front = 0;
	l_msg_queue.back = 0;
	l_msg_queue.num_slots = NUM_MEM_BLKS;
	
	
	UINT8 z;
	for(z = 0; z < NUM_MEM_BLKS; z++)
	{
			l_msg_queue_slots[z].data = 0;
			l_msg_queue_slots[z].next = 0;
	}
	
	struct s_message *  p;
	struct s_message *  q;
	
	while (1) 
    {
		rtx_dbug_outs((CHAR *)"c1\r\n");
		//Check if local queue is empty, if empty wait for message from global queue
		if(message_pop(&l_msg_queue, l_msg_queue_slots, &p) == -1){
			p = receive_message(0);
		}
		
		//check if message type is of type “count_report"
		if( p->type == 4){
		rtx_dbug_outs((CHAR *)"c2\r\n");
			VOID * m;
			m = p;
			m+=64;
			int * d;
			d = m;
			//check if contents at msg_data[0] are divisble by 20
			if(*d%20 == 0){
				rtx_dbug_outs((CHAR *)"c3\r\n");
				//Send a message to CRT
				p->type = 3;
				p->msg_text = p + 4;
				strCopy("Process C\n\r", p->msg_text);
				send_message(11, (VOID *)p);
				//To hibernate for 10 seconds, delay send itself a message
				q = (struct s_message *)request_memory_block();
				q->type = 8; //Message Type of 8 is "WakeUp10" 
				delayed_send(9, (VOID *)q, 10000);
				while(1){
					rtx_dbug_outs((CHAR *)"c4\r\n");
					p = (struct s_message *)receive_message(0);
					//check if received message is WakeUp10, signalling hibernation of 10 seconds is over.
					if(p->type == 8){
						//break if true
						rtx_dbug_outs((CHAR *)"c5\r\n");
						 break;
					}else{
						//if false, queue p into local message queue
						rtx_dbug_outs((CHAR *)"c6\r\n");
						message_push(&l_msg_queue, l_msg_queue_slots, p);
					}
				
				}
			}
		}
		//release memory block p (no need to release q, because at this point q = p)
		release_memory_block((VOID *)p);
		release_processor();
		
    }
}
