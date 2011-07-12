#include "system.h"
#include "Hex_to_ASCII.h"

extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[10];
extern struct s_pcb				*g_current_process;

CHAR charIn = 0;
CHAR charOut = 0;

extern struct delayed_send_request send_reqs[10];
extern UINT8 g_hours;
extern UINT8 g_minutes;
extern UINT8 g_seconds;
UINT32 g_counter = 0;
UINT8  g_wall_clock_enabled = 0;
UINT32 ticks_since_last_run = 0;
UINT8  timer_is_scheduled = 0;
BYTE rw;

extern struct s_char_queue				outputBuffer;
extern struct s_char_queue_item			outputBufferSlots[2000];

void uart()
{
	struct s_message * tmp = 0;
	
	// Input buffer that holds the command to be sent to the keyboard decoder
	char inputBuffer[100];
	UINT8 inputBufferIndex = 0;
	
    while(1)
    {
		// If we are reading from the UART
		if((rw & 1) && (inputBufferIndex < 100))
		{
#ifdef _DEBUG_HOTKEYS
			//UINT8 i;
			struct s_message * output;
			if(charIn == '~')
			{
				// Display processes currently on the ready queues and their priority
				output = (struct s_message *)request_memory_block();
				output->type = 3;
				output->msg_text = "Processes currently in ready queues:\n\r";
				send_message(8, (VOID *)output);
				
				/*
				for(i = 0; i < NUM_PROCESSES; i++)
				{
					if(g_proc_table[i].m_state == 3)
					{
						// Do output
					}
				}
				*/
			}
			else if(charIn == '`')
			{
				// Display processes currently on the blocked on memory queue and their priorities
				/*
				for(i = 0; i < NUM_PROCESSES; i++)
				{
					if(g_proc_table[i].m_state == 3)
					{
						// Do output
					}
				}
				*/
			}
			else if(charIn == '!')
			{
				// Display processes currently on the blocking on receive and their priorities
				/*
				for(i = 0; i < NUM_PROCESSES; i++)
				{
					if(g_proc_table[i].m_state == 3)
					{
						// Do output
					}
				}
				*/
			}
			else if(charIn == '\t')
			{
				// Invoke an error LOL
				asm("rte");
			}
			else
			{
#endif
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
					
					// Send the input buffer which holds the user's command in a message to the keyboard decoder
					tmp = 0;
					tmp = (struct s_message *)request_memory_block();
					if(tmp != 0)
					{
						tmp->type = 0;
						tmp->msg_text = inputBuffer;
                        rtx_dbug_outs(tmp->msg_text);
                        rtx_dbug_outs("PROBLEM?\n\r");
						send_message(7, (VOID *)tmp);
					}
					
					// Put a newline character in the output queue
					buffer_push(&outputBuffer, outputBufferSlots, '\n');
				}
#ifdef _DEBUG_HOTKEYS
			}
#endif
			// Enable transmit interrupts so that the user's input is echoed out
			SERIAL1_IMR = 0x03;
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
		}
		
        release_processor();
    }	
}

void timer()
{
	UINT8 i = 0;
    UINT8 tmp = 0;
    struct s_message * msg = 0;
    char * msg_text = "23:59:59\r\0";

	while(1)
	{
        //Update pending delay request message expiration counters
		for(i = 0; i < 10; i++)
		{
            send_reqs[i].exp -= ticks_since_last_run;
            
			if(send_reqs[i].exp <= 0)
			{
				send_message(send_reqs[i].process_ID, send_reqs[i].envelope);
			}
		}
		
        /*If wall clock is enabled, check if an update is necessary and perform all counter updates as necessary
        before sending output message to CRT for output.*/
		if(g_wall_clock_enabled == 1)
        {
            g_counter++;
            if(g_counter >= 1000)
            {
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
                tmp = (g_seconds % 0xa);
                msg_text[7] = (BYTE)(tmp + 0x30);
                msg_text[6] = (BYTE)(((g_seconds - tmp)/0xa) + 0x30);
                tmp = (g_minutes % 0xa);
                msg_text[4] = (BYTE)(tmp + 0x30);
                msg_text[3] = (BYTE)(((g_minutes - tmp)/0xa) + 0x30);
                tmp = (g_hours % 0xa);
                msg_text[1] = (BYTE)(tmp + 0x30);
                msg_text[0] = (BYTE)(((g_hours - tmp)/0xa) + 0x30);
                
                //Request memory block and send message to CRT for display
                msg = (struct s_message *)request_memory_block();
                if(msg!= 0)
                {
                    msg->type = 3;
                    msg->msg_text = msg_text;
                    send_message(8, (VOID *)msg);
                }
            }
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
    int                to_ID;            //process ID of process to send a message to in the case of %C command
    int                new_priority;     //new priority to use in set_process_priority in the case of %C command
    struct s_message * msg;  
	struct s_message * output;
    char             * msg_body;
    char             * result;
	int				   messageType = 0;
    UINT8              tmp_hours   = 0;
    UINT8              tmp_minutes = 0;
    UINT8              tmp_seconds = 0;
    UINT8              valid       = 0;
    
    while(1)
    {
		msg = (struct s_message *)receive_message(&sender_ID);
		msg_body = msg->msg_text;
        rtx_dbug_outs(msg_body);
        rtx_dbug_outs("\n\r");
        rtx_dbug_outs("end\n\r");
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
                                valid = 1;
                                break;
							case '1':
                                valid = 1;
                                tmp_hours = 10;
                                switch(msg_body[4])
                                {
                                    case '0':
                                        valid = 1;
                                        break;
                                    case '1':
                                        valid = 1;
                                        tmp_hours += 1;
                                        break;
                                    case '2':
                                        valid = 1;
                                        tmp_hours += 2;
                                        break;
                                    case '3':
                                        valid = 1;
                                        tmp_hours += 3;
                                        break;
                                    case '4':
                                        valid = 1;
                                        tmp_hours += 4;
                                        break;
                                    case '5':
                                        valid = 1;
                                        tmp_hours += 5;
                                        break;
                                    case '6':
                                        valid = 1;
                                        tmp_hours += 6;
                                        break;
                                    case '7':
                                        valid = 1;
                                        tmp_hours += 7;
                                        break;
                                    case '8':
                                        valid = 1;
                                        tmp_hours += 8;
                                        break;
                                    case '9':
                                        valid = 1;
                                        tmp_hours += 9;
                                        break;
                                    default:
                                        result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                        valid = 0;
                                        //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
                                        break;
                                }
								break;
							case '2':
                                valid = 1;
                                tmp_hours = 20;
                                switch(msg_body[4])
                                {
                                    case '0':
                                        valid = 1;
                                        break;
                                    case '1':   
                                        valid = 1;
                                        tmp_hours += 1;
                                        break;
                                    case '2':   
                                        valid = 1;
                                        tmp_hours += 2;
                                        break;
                                    case '3':
                                        valid = 1;
                                        tmp_hours += 3;
                                        break;
                                    default:
                                        result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                        valid = 0;
                                        //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
                                        break;
                                }
								break;
							default:
								result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                valid = 0;
                                //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
								break;
						}
						if(msg_body[5] == 0x3a)
						{
							switch(msg_body[6])
							{
								case '0':
                                    valid = 1;
                                    break;
								case '1':
                                    valid = 1;
                                    tmp_minutes = 10;
                                    break;
								case '2':
                                    valid = 1;
                                    tmp_minutes = 20;
                                    break;
								case '3':
                                    valid = 1;
                                    tmp_minutes = 30;
                                    break;
								case '4':
                                    valid = 1;
                                    tmp_minutes = 40;
                                    break;
								case '5':
                                    valid = 1;
                                    tmp_minutes = 50;
                                    break;
								default:
                                    result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                    valid = 0;
                                    //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
									break;
							}
                            switch(msg_body[7])
                            {
                                case '0':
                                    valid = 1;
                                    break;
                                case '1':
                                    valid = 1;
                                    tmp_minutes += 1;
                                    break;
                                case '2':
                                    valid = 1;
                                    tmp_minutes += 2;
                                    break;
                                case '3':
                                    valid = 1;
                                    tmp_minutes += 3;
                                    break;
                                case '4':
                                    valid = 1;
                                    tmp_minutes += 4;
                                    break;
                                case '5':
                                    valid = 1;
                                    tmp_minutes += 5;
                                    break;
                                case '6':
                                    valid = 1;
                                    tmp_minutes += 6;
                                    break;
                                case '7':
                                    valid = 1;
                                    tmp_minutes += 7;
                                    break;
                                case '8':
                                    valid = 1;
                                    tmp_minutes += 8;
                                    break;
                                case '9':
                                    valid = 1;
                                    tmp_minutes += 9;
                                    break;
                                default:
                                    result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                    valid = 0;
                                    //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
                                    break;
                            }
                            if(msg_body[8] == 0x3a)
                            {
                                switch(msg_body[9])
                                {
                                    case '0':
                                        valid = 1;
                                        break;
                                    case '1':
                                        valid = 1;
                                        tmp_seconds = 10;
                                        break;
                                    case '2':
                                        valid = 1;
                                        tmp_seconds = 20;
                                        break;
                                    case '3':
                                        valid = 1;
                                        tmp_seconds = 30;
                                        break;
                                    case '4':
                                        valid = 1;
                                        tmp_seconds = 40;
                                        break;
                                    case '5':
                                        valid = 1;
                                        tmp_seconds = 50;
                                        break;
                                    default:
                                        result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                        valid = 0;
                                        //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
                                        break;
                                }
                                 switch(msg_body[10])
                                {
                                    case '0':
                                        valid = 1;
                                        break;
                                    case '1':
                                        valid = 1;
                                        tmp_seconds += 1; 
                                        break;
                                    case '2':
                                        valid = 1;
                                        tmp_seconds += 2; 
                                        break;
                                    case '3':
                                        valid = 1;
                                        tmp_seconds += 3; 
                                        break;
                                    case '4':
                                        valid = 1;
                                        tmp_seconds += 4; 
                                        break;
                                    case '5':
                                        valid = 1;
                                        tmp_seconds += 5; 
                                        break;
                                    case '6':
                                        valid = 1;
                                        tmp_seconds += 6; 
                                        break;
                                    case '7':
                                        valid = 1;
                                        tmp_seconds += 7; 
                                        break;
                                    case '8':
                                        valid = 1;
                                        tmp_seconds += 8; 
                                        break;
                                    case '9':
                                        valid = 1;
                                        tmp_seconds += 9; 
                                        break;
                                    default:
                                        result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                        valid = 0;
                                        //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
                                        break;
                                }
                                if((msg_body[11] == 0xd) && (valid == 1))
                                {
                                    //Update clock hours, minutes, and seconds variables then enable wall clock
                                    g_hours = tmp_hours;
                                    g_minutes = tmp_minutes;
                                    g_seconds = tmp_seconds;
                                    g_wall_clock_enabled = 1;
                                    result = "k lol \n\r";
                                }
                                else
                                {
                                    result = "Not a recognized command. Try again.\n\r";
                                    //rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
                                }
                            }
                            else
                            {
                                result = "Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r";
                                //rtx_dbug_outs((CHAR *)"Not a valid time format. Try HH:MM:SS where 00<=HH<=23, 00<=MM<=59, 00<=SS<=59.\n\r");
                            }
						}
					}
					else if(msg_body[2] == 'T')
					{
						if(msg_body[3] == 0xd)
						{
							//Turn wall clock off
                            g_wall_clock_enabled = 0;
						}
						else
						{
                            result = "Not a recognized command. Try again.\n\r";
							//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
						}
					}
					else
					{
                        result = "Not a recognized command. Try again.\n\r";
						//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
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
                                                result = "Not a recognized command. Try again.\n\r";
												//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
											}
											break;
										default:
                                            result = "Not a valid priority level. Try again. \n\r";
											//rtx_dbug_outs((CHAR *)"Not a valid priority level. Try again.\n\r");
											break;
									}
								}
								else
								{
                                    result = "Not a recognized command. Try again.\n\r";
									//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
								}
								break;
							default:
                                result = "Not a valid process ID. Try again..\n\r";
								//rtx_dbug_outs((CHAR *)"Not a valid process ID. Try again..\n\r");
								break;
						}
					}
					else
					{
                        result = "Not a recognized command. Try again.\n\r";
						//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
					}
				}
				else
				{
                    result = "Not a recognized command. Try again.\n\r";
					//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
				}
			}
			else
			{
                result = "Not a recognized command. Try again.\n\r";
				//rtx_dbug_outs((CHAR *)"Not a recognized command. Try again.\n\r");
			}
		}
		else
		{
            result = "Improper message type.\n\r";
			//rtx_dbug_outs((CHAR *)"Improper message type.\n\r");
		}
        
        output = (struct s_message *)request_memory_block();
        output->type = 3;
        output->msg_text = result;
        send_message(8, (VOID *)output);
        
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
	int					sender_ID;
    struct s_message	* msg;
	char				c;
	
    while(1)
    {
		msg = (struct s_message *)receive_message(&sender_ID);
		
        if(msg->type == 3)
        {
            // Start pushing characters to the outputbuffer (up to 100 chracters, or until a null character is hit)
			UINT32 i = 0;
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
		release_memory_block((VOID *)msg);
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

		push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[9]);
		if(g_current_process->i_process == 0)
		{
			release_processor();
		}
	}
	else if(rw & 4)
	{
		// Output character, default is a null
		SERIAL1_WD = charOut;
		
		// Stop transmit interrupts
		SERIAL1_IMR = 0x02;
		
		// Schedule iProc to see if more characters should be output
		push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[9]);
		
		// Only release if the process is not another i-process
		if(g_current_process->i_process == 0)
		{
			release_processor();
		}
	}

}

void c_timer_handler()
{
	// Disable interrupts
	asm("move.w #0x2700,%sr");
	
	// Increment ticks_since_last_run (tracks how many times the ISR has run since the last time the process
    // actually ran) and schedule the i-process if it's not already currently scheduled.
    
	ticks_since_last_run++;
    
    if(timer_is_scheduled == 0)
    {
        push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[10]);
    }
	
	TIMER0_TER = 2;
}