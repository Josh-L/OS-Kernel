#include "system.h"

extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[5];

void uart(){
	while(1)
    {    
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


/*********************************************************************************************************
Call receive_message. Once a message is received, check if it is one of 2 allowable types (types 3 and 4).
If it is not one of these types, do nothing. If it is a valid message type, perform the necessary decoding
by reading the contents of the message body and acting accordingly (ie. if command is "%C..." call 
set_process_priority, and if command is "%W..." call the timer i-process).
*********************************************************************************************************/
void kdc(){
    int                sender_ID;
    struct s_message * msg; 
    char             * msg_body;
    
    rtx_dbug_out_char('k');
    msg = (struct s_message *)receive_message(&sender_ID);
    msg_body = (char *)(msg + 0x7);
    rtx_dbug_outs((CHAR *)msg_body);
    
    //Keyboard input
    if(msg->type_message == 3)
    {
        
    }
    //Command registration  
    else if(msg->type_message == 4)
    {
    
    }
}


/*********************************************************************************************************
Call receive_message. Once a message is received, check if it is the correct type (type 5). If it is not
the right type, do nothing. If it is the correct message type, output the text contained in the 
message body using the UART i-process. Finally, before returning, call release_memory_block to free the 
memory used by the message.
*********************************************************************************************************/
void crt(){
    int                sender_ID;
    struct s_message * msg; 
    
    msg = (struct s_message *)receive_message(&sender_ID);
    
    if(msg->type_message == 5)
    {
        
    }
}

void c_serial_handler()
{
	   
}

