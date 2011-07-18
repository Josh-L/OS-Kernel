#include "system.h"
#include "Hex_to_ASCII.h"

// Variables used in system.c, needed for reference by i_proc.c
extern struct s_pcb 			g_proc_table[NUM_PROCESSES];
extern struct s_pcb_queue		g_iProc_queue;
extern struct s_pcb_queue_item 	g_iProc_queue_slots[10];
extern struct s_pcb				*g_current_process;
extern struct s_pcb_queue		g_priority_queues[NUM_PRIORITIES];
extern struct s_pcb_queue_item 	g_queue_slots[NUM_PROCESSES]; // Have an array of ready queue slots

// Variables for i_proc.s that we delcared in system.c so they could be initialized in sys_init
extern struct delayed_send_request send_reqs[NUM_DELAYED_SLOTS];
extern struct s_char_queue				outputBuffer;
extern struct s_char_queue_item			outputBufferSlots[2000];
extern UINT32 g_clock_counter;
extern UINT8  g_clock_enabled;
extern UINT8  g_timer_is_scheduled;

BYTE rw;
CHAR charIn;
CHAR charOut;

UINT8 g_hours;
UINT8 g_minutes;
UINT8 g_seconds;

void strCopy(CHAR * from, CHAR * to)
{
	UINT32 i;
	
	*to = *from;
	while(*from != '\0')
	{
		from += 1;
		to += 1;
		*to = *from;
	}
}

void uart()
{
	charOut = 0;
	
	struct s_message * tmp = 0;
	
    while(1)
    {
		// If we are reading from the UART
		if(rw & 1)
		{
			tmp = (struct s_message *)request_memory_block();
			if(tmp != 0)
			{
				tmp->type = 1;
				tmp->msg_text = tmp + 4;
				tmp->msg_text[0] = charIn;
				tmp->msg_text[1] = '\0';
				send_message(10, (VOID *)tmp);
			}
			// Enable transmit interrupts so that the user's input is echoed out
			SERIAL1_IMR = 0x03;
		}
		else if (rw & 4)
		{
			// Check for messages from CRT that want to be output
			int sender;
			tmp = (struct s_message *)receive_message(&sender);
			// If we actually get a message
			if(tmp != 0)
			{
				// Add message text to the output buffer
				while(tmp->msg_text[0] != 0)
				{
					buffer_push(&outputBuffer, outputBufferSlots, tmp->msg_text[0]);
					tmp->msg_text += 1;
				}
				
				// Release the memory block this message holds
				release_memory_block((VOID *)tmp);
			}
			// Write the next character waiting in the output buffer
			charOut = buffer_pop(&outputBuffer, outputBufferSlots);
			
			// If the buffer is not empty
			if(charOut != 0)
			{
				// Enable transmit interrupts so that the rest of the the output buffer is printed
				SERIAL1_IMR = 0x03;
			}
		}
		
        release_processor();
    }	
}

void timer()
{
	g_hours = 0;
	g_minutes = 0;
	g_seconds = 0;
	
	// This is to hold "left over" milliseconds for the clock
	UINT32 ticks_since_last_run = 0;
	// This is to store the g_clock_counter value so that it can be instantly reset and not change during this process' execution
	UINT32 temp_counter = 0;
	
	UINT8 i = 0;
    UINT8 tmp = 0;
    struct s_message * msg = 0;
    char * msg_text = "23:59:59\r\0";

	while(1)
	{
		temp_counter = g_clock_counter;
		g_clock_counter = 0;
		
        //Update pending delay request message expiration counters
		for(i = 0; i < NUM_DELAYED_SLOTS; i++)
		{
			if (send_reqs[i].envelope != 0)
			{
				send_reqs[i].exp -= temp_counter;
				if(send_reqs[i].exp <= 0)
				{
					// Send the message
					message_push(&g_proc_table[send_reqs[i].process_slot].msg_queue, g_proc_table[send_reqs[i].process_slot].msg_queue_slots, (struct s_message *)send_reqs[i].envelope);
					
					// If receiving process is currently blocking on message from sender, unblock and push to ready queue
					if(g_proc_table[i].m_state == 3)
					{
						g_proc_table[i].m_state = 1;
						push(&g_priority_queues[g_proc_table[i].m_priority], g_queue_slots, &g_proc_table[i]);
					}
					
					// Free up the delayed send space
					send_reqs[i].envelope = 0;
				}
				
			}
		}
		
        /*If wall clock is enabled, check if an update is necessary and perform all counter updates as necessary
        before sending output message to CRT for output.*/
		ticks_since_last_run += temp_counter;
		
		if(g_clock_enabled == 1)
        {
            if(ticks_since_last_run >= 1000)
            {
				//rtx_dbug_out_char('#');
				ticks_since_last_run = 0;
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
				msg = 0;
                msg = (struct s_message *)request_memory_block();
				
                if(msg != 0)
                {
                    msg->type = 3;
                    msg->msg_text = msg + 4;
					strCopy(msg_text, msg->msg_text);
                    send_message(11, (VOID *)msg);
                }
            }
        }
		g_timer_is_scheduled = 0;
		release_processor();
	}
}

void kcd()
{
	char c;
	struct s_message * input;
	struct s_message * output;
	int * sender_ID;
	char inputBuffer[100];
	UINT32 inputBufferIndex = 0;
	
	char registered_commands[REGISTERED_COMMANDS][20];
	int registered_ID [REGISTERED_COMMANDS];
	
	UINT8 i;
	UINT8 j;
	UINT8 matched;
	
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		registered_ID[i] = -1;
	}
	
	while(1)
	{
		inputBufferIndex = 0;
		while(1)
		{
			// Receive character in message
			input = (struct s_message *)receive_message(&sender_ID);
			
			// Register command
			if(input->type == 2)
			{
				for(i = 0; i < REGISTERED_COMMANDS; i++)
				{
					if(registered_ID[i] == -1)
					{
						registered_ID[i] = sender_ID;
						strCopy(input->msg_text, &registered_commands[i][0]);
						break;
					}
				}
			}
			// Parse input
			else if(input->type == 1)
			{
				// Save character
				c = input->msg_text[0];
				
				// If this character is a hotkey
#ifdef _DEBUG_HOTKEYS
				if(c == '-')
				{
					if(g_clock_enabled == 0)
					{
						g_clock_enabled = 1;
						rtx_dbug_outs("Clock turning ON\r\n");
					}
					else
					{
						g_clock_enabled = 0;
						rtx_dbug_outs("Clock turning OFF\r\n");
					}
				}
				else if(c == '=')
				{
					if(g_clock_enabled == 0)
					{
						rtx_dbug_outs("Clock OFF\r\n");
					}
					else
					{
						rtx_dbug_outs("Clock ON\r\n");
					}
					rtx_dbug_outs("Seconds: ");
					printHexAddress(g_seconds);
					rtx_dbug_outs("\r\n");
					rtx_dbug_outs("Minutes: ");
					printHexAddress(g_minutes);
					rtx_dbug_outs("\r\n");
					rtx_dbug_outs("Hours: ");
					printHexAddress(g_hours);
					rtx_dbug_outs("\r\n");
				}
				else if(c == '~')
				{
					// Display processes currently on the ready queues and their priority
					output = (struct s_message *)request_memory_block();
					if(output != 0)
					{
						output->type = 3;
						output->msg_text = output + 4;
						strCopy("Processes currently in ready queues:\n\r", output->msg_text);
						send_message(11, (VOID *)output);
					}
					
					printProcessesByState(1);
				}
				else if(c == '|')
				{
					// Display processes currently on the blocked on memory queue and their priorities
					output = (struct s_message *)request_memory_block();
					if(output != 0)
					{
						output->type = 3;
						output->msg_text = output + 4;
						strCopy("Processes currently blocking on memory:\n\r", output->msg_text);
						send_message(11, (VOID *)output);
					}
					
					printProcessesByState(0);
				}
				else if(c == '!')
				{
					// Display processes currently on the blocking on receive and their priorities
					output = (struct s_message *)request_memory_block();
					if(output != 0)
					{
						output->type = 3;
						output->msg_text = output + 4;
						strCopy("Processes currently blocking on receive:\n\r", output->msg_text);
						send_message(11, (VOID *)output);
					}
					
					printProcessesByState(3);
				}
				else if(c == '\t')
				{
					// Invoke an error
					asm("rte");
				}
				else
				{
#endif
					// Else echo character to user
					output = (struct s_message *)request_memory_block();
					output->type = 3;
					output->msg_text = output + 4;
					output->msg_text[0] = c;
					output->msg_text[1] = '\0';
					send_message(11, (VOID *)output);
					
					if(inputBufferIndex < 100 && c != '\0')
					{
						inputBuffer[inputBufferIndex] = c;
						inputBufferIndex ++;
					}
					
					// If this the end of the command
					if(c == '\r')
					{	
						// Send a newline character to the CRT
						output = (struct s_message *)request_memory_block();
						output->type = 3;
						output->msg_text = output + 4;
						output->msg_text[0] = '\n';
						output->msg_text[1] = '\0';
						send_message(11, (VOID *)output);
						
						inputBuffer[inputBufferIndex-1] = '\0';
						inputBufferIndex = 0;
						
						// Start parsing
						break;
					}
					// If the character is a backspace
					else if(c == 0x08)
					{
						if(inputBufferIndex > 1 && inputBufferIndex != 99)
						{
							inputBufferIndex -= 2;
						}
						else
						{
							inputBufferIndex -= 1;
						}
					}
#ifdef _DEBUG_HOTKEYS
				}
#endif
			}
			release_memory_block((VOID *)input);
		}
		
		matched = 0;
		output = (struct s_message *)request_memory_block();
		output->type = 3;
		output->msg_text = output + 4;
		
		if(inputBuffer[0] == '%')
		{
			// Check process registered commands
			for(i = 0; i < REGISTERED_COMMANDS; i++)
			{
				if(registered_ID[i] != -1)
				{
					for(j = 0; j < 20; j++)
					{
						if(registered_commands[i][j] == 0)
						{
							break;
						}
						else if(registered_commands[i][j] == inputBuffer[j+1])
						{
							matched = 1;
						}
						else
						{
							matched = 0;
							break;
						}
					}
				}
				
				if(matched == 1)
				{
					// Make sure there's a space, or end of command
					if(inputBuffer[j+1] == 0 || inputBuffer[j+1] == ' ')
					{
						strCopy(&inputBuffer[1], output->msg_text);
						send_message(registered_ID[i], (VOID *)output);
						break;
					}
					matched = 0;
				}
			}
		}
		
		if(matched == 0)
		{
			strCopy("Invalid command.\n\r", output->msg_text);
			send_message(11, (VOID *)output);
		}
	}
}

void crt()
{	
	int					sender_ID;
    struct s_message	* msg;
	struct s_message	* out;
	char				c;
	
    while(1)
    {
		msg = (struct s_message *)receive_message(&sender_ID);
		
		// If we get the correct message type
        if(msg->type == 3)
        {
			// Send message to UART to output the text
			out = (struct s_message *)request_memory_block();
			out->type = 0;
			out->msg_text = out + 4;
			strCopy(msg->msg_text, out->msg_text);
			send_message(12, (VOID *)out);
			
			// Enable transmit interrupts for the message to be displayed
			SERIAL1_IMR = 0x03;
        }
		release_memory_block((VOID *)msg);
    }
}

void wall_clock()
{
	int i;
	struct s_message * output;
	
	// Register WS command
	struct s_message * tmp = request_memory_block();
	tmp->type = 2;
	tmp->msg_text = tmp + 4;
	strCopy("WS", tmp->msg_text);
	send_message(10, (VOID *)tmp);
	
	// Register WT command
	tmp = request_memory_block();
	tmp->type = 2;
	tmp->msg_text = tmp + 4;
	strCopy("WT", tmp->msg_text);
	send_message(10, (VOID *)tmp);
	
	while(1)
	{
		tmp = receive_message(&i);
		
		// Prepare output message
		output = (struct s_message *)request_memory_block();
		output->type = 3;
		output->msg_text = output + 4;
		
		int i = 0;
		
		if(tmp->msg_text[0] == 'W')
		{
			if(tmp->msg_text[1] == 'T')
			{
				if(tmp->msg_text[2] == 0)
				{
					g_clock_enabled = 0;
					strCopy("Turning clock off\n\r", output->msg_text);
				}
				else
				{
					strCopy("Invalid command.\n\r", output->msg_text);
				}
			}
			else if(tmp->msg_text[1] == 'S')
			{
				if(tmp->msg_text[2] == ' ')
				{
					if(tmp->msg_text[3] > 0x2F && tmp->msg_text[3] < 0x33)
					{
						int hours = (tmp->msg_text[3] - 0x30)*10;
						if(tmp->msg_text[4] > 0x2F && tmp->msg_text[4] < 0x3A)
						{
							hours += (tmp->msg_text[4] - 0x30);
							if(hours < 24)
							{
								if(tmp->msg_text[5] == 0x3A)
								{
									if(tmp->msg_text[6] < 0x36 && tmp->msg_text[6] > 0x2F)
									{
										int minutes = (tmp->msg_text[6] - 0x30)*10;
										if(tmp->msg_text[7] < 0x3A && tmp->msg_text[7] > 0x2F)
										{
											minutes += (tmp->msg_text[7] - 0x30);
											if(tmp->msg_text[8] == 0x3A)
											{
												if(tmp->msg_text[9] < 0x36 && tmp->msg_text[9] > 0x2F)
												{
													int seconds = (tmp->msg_text[9] - 0x30)*10;
													if(tmp->msg_text[10] < 0x3A && tmp->msg_text[10] > 0x2F)
													{
														seconds += (tmp->msg_text[10] - 0x30);
														if(tmp->msg_text[11] == '\0')
														{
															g_hours = hours;
															g_minutes = minutes;
															g_seconds = seconds;
															g_clock_enabled = 1;
															strCopy("Turning clock on.\n\r", output->msg_text);
														}
														else
														{
															strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
														}
													}
													else
													{
														strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
													}
												}
												else
												{
													strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
												}
											}
											else
											{
												strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
											}
										}
										else
										{
											strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
										}
									}
									else
									{
										strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
									}
								}
								else
								{
									strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
								}
							}
							else
							{
								strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
							}
						}
						else
						{
							strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
						}
					}
					else
					{
						strCopy("Time format incorrect. Format is HH:MM:SS.\n\r", output->msg_text);
					}
				}
				else
				{
					strCopy("Invalid command.\n\r", output->msg_text);
				}
			}
			else
			{
				strCopy("Invalid command.\n\r", output->msg_text);
			}
		}
		else
		{
			strCopy("Invalid command.\n\r", output->msg_text);
		}
		
		release_memory_block((VOID *)tmp);
		
		// Send message to CRT
		send_message(11, (VOID *)output);
	}
}

void change_priority()
{
	int i;
	struct s_message * output;
	
	// Register C command
	struct s_message * tmp = request_memory_block();
	tmp->type = 2;
	tmp->msg_text = tmp + 4;
	strCopy("C", tmp->msg_text);
	send_message(10, (VOID *)tmp);
	
	while(1)
	{
		tmp = receive_message(&i);
		
		// Prepare output message
		output = (struct s_message *)request_memory_block();
		output->type = 3;
		output->msg_text = output + 4;
		
		if(tmp->msg_text[1] == ' ')
		{
			if(tmp->msg_text[2] < 0x3A && tmp->msg_text[2] > 0x30)
			{
				int process_id = tmp->msg_text[2];
				if(tmp->msg_text[3] == ' ')
				{
					if(tmp->msg_text[4] < 0x34 && tmp->msg_text[4] > 0x2F)
					{
						int priority = tmp->msg_text[4];
						if(tmp->msg_text[5] == '\0')
						{
							strCopy("Changed process ID x to priority level y.\n\r", output->msg_text);
							output->msg_text[19] = (char)process_id;
							output->msg_text[39] = (char)priority;
							process_id = process_id - 0x30;
							priority = priority - 0x30;
							set_process_priority(process_id, priority);
						}
						else
						{
							strCopy("Invalid command.\n\r", output->msg_text);
						}
					}
					else
					{
						strCopy("Invalid priority.\n\r", output->msg_text);
					}
				}
				else
				{
					strCopy("Invalid command.\n\r", output->msg_text);
				}
			}
			else
			{
				strCopy("Invalid process ID.\n\r", output->msg_text);
			}
		}
		else
		{
			strCopy("Invalid command.\n\r", output->msg_text);
		}
		
		release_memory_block((VOID *)tmp);
		
		// Send message to CRT
		send_message(11, (VOID *)output);
	}
}

void printProcessesByState(UINT8 state)
{
	UINT8 i;
	struct s_message * output;
	UINT8 sent = 0;
	CHAR * message = "Process ID: x --> Priority: y\n\r";
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if(g_proc_table[i].m_state == state && g_proc_table[i].i_process == 0 && g_proc_table[i].sys_process == 0)
		{
			output = (struct s_message *)request_memory_block();
			if(output != 0)
			{
				output->msg_text = output + 4;
				output->type = 3;
				strCopy(message, output->msg_text);
				output->msg_text[12] = (char)(0x30 + g_proc_table[i].m_process_ID);
				output->msg_text[28] = (char)(0x30 + g_proc_table[i].m_priority);
				send_message(11, (VOID *)output);
			}
			sent = 1;
		}
	}
	if(sent == 0)
	{
		output = (struct s_message *)request_memory_block();
		if(output != 0)
		{
			output->msg_text = output + 4;
			strCopy("(None)\n\r", output->msg_text);
			output->type = 3;
			output->msg_text[12] = (char)(0x30 + g_proc_table[i].m_process_ID);
			output->msg_text[28] = (char)(0x30 + g_proc_table[i].m_priority);
			send_message(11, (VOID *)output);
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
		// Read character from keyboard
		charIn = SERIAL1_RD;
	}
	else if(rw & 4)
	{
		// Output character, default is a null
		SERIAL1_WD = charOut;
		// Stop transmit interrupts
		SERIAL1_IMR = 0x02;
	}
	
	// Push the UART i-process into the process queue
	push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[9]);
	
	// Only release processor if the running process is not an i-process
	if(g_current_process->i_process == 0)
	{
		release_processor();
	}
}

void c_timer_handler()
{
	
	// Disable interrupts
	asm("move.w #0x2700,%sr");
	
	// Increment g_clock_counter (tracks how many times the ISR has run since the last time the process
    // actually ran) and schedule the i-process if it's not already currently scheduled.
	
	g_clock_counter++;
	
	
    if(g_timer_is_scheduled == 0)
    {
		g_timer_is_scheduled = 1;
        push(&g_iProc_queue, g_iProc_queue_slots, &g_proc_table[10]);
		if(g_current_process->i_process == 0)
		{
			release_processor();
		}
    }
	
	TIMER0_TER = 2;
}