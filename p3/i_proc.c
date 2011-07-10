#include "system.h"
#include "Hex_to_ASCII.h"

extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[5];

CHAR charIn = 0;
extern struct delayed_send_request send_reqs[10];
extern UINT8 g_hours;
extern UINT8 g_minutes;
extern UINT8 g_seconds;

UINT32 g_counter = 0;
UINT8  g_wall_clock_enabled = 0;
UINT8  g_update_clock = 0;

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
	UINT8 i = 0;
	while(1)
	{
		rtx_dbug_out_char('*');
		
		for(i = 0; i < 10; i++)
		{
			if(send_reqs[i].exp == 0)
			{
				send_message(send_reqs[i].process_ID, send_reqs[i].envelope);
			}
		}
		
		if(g_wall_clock_enabled == 1 && g_update_clock == 1)
		{
			//format clock output string and send to crt()
		}
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
    int                sender_ID;
    int                to_ID;     //process ID of process to send a message to in the case of %C command
    int                new_priority; //new priority to use in set_process_priority in the case of %C command
    struct s_message * msg; 
    char             * msg_body;
    
    while(1)
    {     
		msg = (struct s_message *)receive_message(&sender_ID);
		msg_body = msg->msg_text;
		printHexAddress(msg->sender_id);
		printHexAddress(msg->dest_id);
		printHexAddress(msg->type);
       
		//Before you can decode the command, you must first create the ANTLR O_O RECURSIVE DESCENT AHOY!
        if(msg_body[0] == '%')
        {
            if(msg_body[1] == 'W')
            {
                if(msg_body[2] == 'S')
				{
					switch(msg_body[3])
					{
						case '0':
						case '1':
							switch(msg_body[4])
							{
								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
								case '8':
								case '9':
									break;
								default:
									rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59\n\r");
									return;
									break;
							}
							break;
						case '2':
							switch(msg_body[4])
							{
								case '0':
								case '1':
								case '2':
								case '3':
									break;
								default:
									rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59\n\r");
									return;
									break;
							}
							break;
						default:
							rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59\n\r");
							break;
					}
					if(msg_body[5] == 0x3a)
					{
						switch(msg_body[6])
						{
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
								switch(msg_body[7])
								{
									case '0':
									case '1':
									case '2':
									case '3':
									case '4':
									case '5':
									case '6':
									case '7':
									case '8':
									case '9':
										if(msg_body[8] == 0x3a)
										{
											switch(msg_body[9])
											{
												case '0':
												case '1':
												case '2':
												case '3':
												case '4':
												case '5':
													switch(msg_body[10])
													{
														case '0':
														case '1':
														case '2':
														case '3':
														case '4':
														case '5':
														case '6':
														case '7':
														case '8':
														case '9':
															if(msg_body[11] == 0xd)
															{
																rtx_dbug_out_char('$');
																//Invoke timer i-process
															}
															else
															{
																rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
															}
															break;
														default:
															break;
													}
													break;
												default:
													break;
											}
										}
										else
										{
											rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59\n\r");
										}
										break;
									default:
										break;
								}
								break;
							default:
								rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59\n\r");
								break;
						}
					}
				}
				else if(msg_body[2] == 'T')
				{
					if(msg_body[3] == 0xd)
					{
						//Turn wall clock off
					}
					else
					{
						rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
					}
				}
				else
				{
					rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
				}
            }
            else if(msg_body[1] == 'C')
            {
                if(msg_body[2] == 0x20)
                {
                    switch(msg_body[3])
                    {
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                            to_ID = (int)(msg_body[3] - 0x30);
                            if(msg_body[4] == 0x20)
                            {
                                switch(msg_body[5])
                                {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                        new_priority = (int)(msg_body[5] - 0x30);
                                        if(msg_body[6] == 0xd)
                                        {
                                            set_process_priority(to_ID, new_priority);
                                        }
                                        else
                                        {
                                            rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
                                        }
                                        break;
                                    default:
                                        rtx_dbug_outs((CHAR *)"Not a valid priority level. Try again\n\r");
                                        break;
                                }
                            }
                            else
                            {
                                rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
                            }
                            break;
                        default:
                            rtx_dbug_outs((CHAR *)"Not a valid process ID. Try again\n\r");
                            break;
                    }
                }
                else
                {
                    rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
                }
            }
            else
            {
                rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
            }
        }
        else
        {
            rtx_dbug_outs((CHAR *)"Not a recognized command. Try again\n\r");
        }
		release_memory_block((VOID *)msg);
		release_processor();
    }
}


/*********************************************************************************************************
Call receive_message. Once a message is received, check if it is the correct type (type 5). If it is not
the right type, do nothing. If it is the correct message type, output the text contained in the 
message body using the UART i-process. Finally, before returning, call release_memory_block to free the 
memory used by the message.
*********************************************************************************************************/
void crt()
{
	int                sender_ID;
    struct s_message * msg; 

    while(1)
    {
        rtx_dbug_out_char('c');
        
        msg = (struct s_message *)receive_message(&sender_ID);
        
        if(msg->type == 5)
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

void c_timer_handler()
{
	/*
	Decrement expiration counter of all pending delayed_send requests and schedule timer i-process
	*/
	UINT8 i = 0;
	for(i = 0; i < 10; i++)
	{
		if(send_reqs[i].exp > 0)
		{
			send_reqs[i].exp--;
		}
	}
	if(g_wall_clock_enabled == 1)
	{
		g_counter++;
		if(g_counter >= 1000)
		{
			g_update_clock = 1;
			g_counter = 0;
			g_seconds++;
			if (g_seconds >= 60)
			{
				g_seconds = 0;
				g_minutes++;
				if (g_minutes >= 60)
				{
					g_minutes = 0;
					g_hours++;
					if (g_hours >= 24)
					{
						g_hours = 0;
					}
				}
			}
		}
	}
	//push timer() to i-process queue
}