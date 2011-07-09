#include "system.h"
#include "Hex_to_ASCII.h"

extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[5];

CHAR charIn = 0;

void uart()
{	
    while(1)	
    {
        rtx_dbug_outs("UART\r\n");
        release_processor();
    }	
}

void timer()
{
	while(1)
	{
		rtx_dbug_outs("TIMER\r\n");
		release_processor();
	}
}

/*********************************************************************************************************
Call receive_message. Once a message is received, check if it is one of 2 allowable types (types 3 and 4).
If it is not one of these types, do nothing. If it is a valid message type, perform the necessary decoding
by reading the contents of the message body and acting accordingly (ie. if command is "%C..." call 
set_process_priority, and if command is "%W..." call the timer i-process).
*********************************************************************************************************/
void kdc()
{
    /*
    char               buf[64] = {0}; //Store characters received from UART i-process
    UINT8              buf_index = 0; //Track last character received 
    int                sender_ID;
    int                to_ID;     //process ID of process to send a message to in the case of %C command
    int                new_priority; //new priority to use in set_process_priority in the case of %C command
    struct s_message * msg; 
    char             * msg_body;
    
    while(1)
    {     
        do
        {
            msg = (struct s_message *)receive_message(&sender_ID);
            msg_body = msg->msg_text;
            buf[buf_index] = msg_body[0];
            rtx_dbug_out_char(buf[buf_index]);
            buf_index++;
            release_memory_block((VOID *)msg);
        }while(buf[buf_index - 1] != '\r');
        
        buf[buf_index] = '\n';
        rtx_dbug_out_char(buf[buf_index]);
        buf_index++;
        buf[buf_index] = '\0';
        rtx_dbug_out_char(buf[buf_index]);
        
        rtx_dbug_out_char('k');
       
        if(buf[0] == '%')
        {
            if(buf[1] == 'W')
            {
                
            }
            else if(buf[1] == 'C')
            {
                rtx_dbug_out_char(buf[1]);
                if(buf[2] == 0x20)
                {
                    rtx_dbug_out_char(buf[2]);
                    switch(buf[3])
                    {
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            rtx_dbug_out_char(buf[3]);
                            to_ID = (int)buf[3];
                            if(buf[4] == 0x20)
                            {
                                switch(buf[5])
                                {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                        new_priority = (int)buf[5];
                                        if(buf[6] == 0xd)
                                        {
                                            set_process_priority(to_ID, new_priority);
                                        }
                                        else
                                        {
                                            rtx_dbug_out_char('!');
                                        }
                                        break;
                                    default:
                                        rtx_dbug_out_char('!');
                                        break;
                                }
                            }
                            else
                            {
                                rtx_dbug_out_char('!');
                            }
                            break;
                        default:
                            rtx_dbug_out_char('!');
                            break;
                    }
                }
                else
                {
                    rtx_dbug_out_char('!');
                }
            }
            else
            {
                rtx_dbug_out_char('!');
            }
        }
        else
        {
            rtx_dbug_out_char('!');
        }
    }
    */
}


/*********************************************************************************************************
Call receive_message. Once a message is received, check if it is the correct type (type 5). If it is not
the right type, do nothing. If it is the correct message type, output the text contained in the 
message body using the UART i-process. Finally, before returning, call release_memory_block to free the 
memory used by the message.
*********************************************************************************************************/
void crt()
{
    while(1)
    {
        int                sender_ID;
        struct s_message * msg; 
        
        rtx_dbug_out_char('c');
        
        msg = (struct s_message *)receive_message(&sender_ID);
        
        if(msg->type_message == 5)
        {
            
        }
    }
}

void c_serial_handler()
{
    // Just schedule the UART i process
	charIn = SERIAL1_RD;
	//SERIAL1_IMR = 0x02; // Disable transmit interrupts
	push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[7]);
	release_processor(); 
}

