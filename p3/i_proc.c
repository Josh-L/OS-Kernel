#include "system.h"
#include "Hex_to_ASCII.h"

extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[10];
extern struct s_pcb				*g_current_process;

CHAR charIn = 0;
CHAR charOut = 0;

UINT8 timer_iproc = 0;
UINT8 read_iproc = 0;
UINT8 write_iproc = 0;

extern struct delayed_send_request send_reqs[10];
extern UINT8 g_hours;
extern UINT8 g_minutes;
extern UINT8 g_seconds;
UINT32 g_counter = 0;
UINT8  g_wall_clock_enabled = 0;
UINT8  g_update_clock = 0;
BYTE rw;

struct s_char_queue				outputBuffer;
struct s_char_queue_item		outputBufferSlots[100];

void uart()
{
	struct s_message * tmp = 0;
	
	// Input buffer that holds the command to be sent to the keyboard decoder
	char inputBuffer[100];
	UINT8 inputBufferIndex = 0;
	
    while(1)
    {
		// If we are reading from the UART
		if(rw & 1)
		{
			// Add the character to the input buffer which will be sent to the keybaord decoder
			inputBuffer[inputBufferIndex] = charIn;
			
			// Put the character in the output buffer
			buffer_push(&outputBuffer, outputBufferSlots, inputBuffer[inputBufferIndex]);
			
			inputBufferIndex++;
			
			// If the user presses enter to finish the command
			if(inputBuffer[inputBufferIndex - 1] == '\r')
			{
				// Reset input buffer
				inputBufferIndex = 0;
				
				/*// Send the input buffer which holds the user's command in a message to the keyboard decoder
				tmp = 0;
				tmp = (struct s_message *)request_memory_block();
				if(tmp != 0)
				{
					tmp->type = 0;
					tmp->msg_text = inputBuffer;
					send_message(7, (VOID *)tmp);
				}*/
				
				// Put a newline character in the output queue
				buffer_push(&outputBuffer, outputBufferSlots, '\n');
			}
			
			//charOut = charIn;
			
			// Enable transmit interrupts so that the user's input is echoed out
			SERIAL1_IMR = 0x03;
			read_iproc = 0;
		}
		else if (rw & 4)
		{
			// Write the next character waiting in the output buffer
			charOut = buffer_pop(&outputBuffer, outputBufferSlots);
			
			// If the buffer is not empty
			if(charOut != 0)
			{
				// Enable transmit interrupts so that the user's input is echoed out
				SERIAL1_IMR = 0x03;
			}
			write_iproc = 0;
		}
		
        release_processor();
    }	
}

void timer()
{
	rtx_dbug_outs("\r\nTIMER\r\n");
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
void kcd()
{
    int                sender_ID;
    int                to_ID;     //process ID of process to send a message to in the case of %C command
    int                new_priority; //new priority to use in set_process_priority in the case of %C command
    struct s_message * msg;  
	struct s_message * output;
    char             * msg_body;
	int				   messageType = 0;
    
    while(1)
    {
		msg = (struct s_message *)receive_message(&sender_ID);
		msg_body = msg->msg_text;
		
		// Ensure message type is correct
		if(msg->type == messageType)
		{
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
										rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
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
										rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
										return;
										break;
								}
								break;
							default:
								rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
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
																	rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
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
												rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
											}
											break;
										default:
											break;
									}
									break;
								default:
									rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
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
							rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
						}
					}
					else
					{
						rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
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
												rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
											}
											break;
										default:
											rtx_dbug_outs((CHAR *)"Not a valid priority level. Try again.\n\r");
											break;
									}
								}
								else
								{
									rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
								}
								break;
							default:
								rtx_dbug_outs((CHAR *)"Not a valid process ID. Try again..\n\r");
								break;
						}
					}
					else
					{
						rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
					}
				}
				else
				{
					rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
				}
			}
			else
			{
				output = (struct s_message *)request_memory_block();
				output->type = 3;
				output->msg_text = "Not a recognized command. Try again.\n\r";
				send_message(8, (VOID *)output);
				//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
			}
		}
		else
		{
			rtx_dbug_outs((CHAR *)"Improper message type.\n\r");
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
	// Initialize the output buffer queue
	UINT8 i;
	outputBuffer.front = 0;
	outputBuffer.back = 0;
	outputBuffer.num_slots = 100;
	
	int					sender_ID;
    struct s_message	* msg;
	char				c;
	
	for(i = 0; i < outputBuffer.num_slots; i++)
	{
		outputBufferSlots[i].data = 0;
		outputBufferSlots[i].next = 0;
	}
	
    while(1)
    {
		msg = (struct s_message *)receive_message(&sender_ID);
		
        if(msg->type == 3)
        {
            // Start pushing characters to the outputbuffer (up to 100 chracters, or until a null character is hit)
			UINT16 i = 0;
			for(i = 0; i < outputBuffer.num_slots; i++)
			{
				c = msg->msg_text[i];
				if(c != '\0')
				{
					buffer_push(&outputBuffer, outputBufferSlots, c);
				}
				else
				{
					break;
				}
			}
			// Enable transmit interrupts for the message to be displayed
			SERIAL1_IMR = 0x03;
        }
    }
}

void c_serial_handler()
{
	// Disable interrupts
	asm("move.w #0x2700,%sr");
	
    // Acknowledge the interrupt and determine if its a read or write
    rw = SERIAL1_UCSR;
    
	if(rw & 1)
	{

		charIn = SERIAL1_RD;
		
		//if(read_iproc == 0)
		//{
			read_iproc = 1;
			push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[9]);
			if(g_current_process->i_process == 0)
			{
				release_processor();
			}
		//}
	}
	else if(rw & 4)
	{
		// Output character, default is a null
		SERIAL1_WD = charOut;
		
		// Stop transmit interrupts
		SERIAL1_IMR = 0x02;
		
		//if(write_iproc == 0)
		//{
			write_iproc = 1;
			// Schedule iProc to see if more characters should be output
			push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[9]);
			
			// Only release if the process is not another i-process
			if(g_current_process->i_process == 0)
			{
				release_processor();
			}
		//}
	}

}

void c_timer_handler()
{
	// Disable interrupts
	asm("move.w #0x2700,%sr");
	
	// Decrement expiration counter of all pending delayed_send requests and schedule timer i-process
	
	/*UINT8 i = 0;
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
	*/
	TIMER0_TER = 2;
}