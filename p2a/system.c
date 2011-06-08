#include "system.h"

struct s_pcb        g_null_proc;
struct s_pcb        g_test_proc_table[NUM_PROCESSES];
struct s_pcb 		g_priority_ready[4][NUM_PROCESSES];
SINT8               g_priority_ready_tracker[4][2];
struct s_pcb 	    *g_current_process;
UINT32				*g_kernelStack = 0;
UINT32				g_asmBridge = 0;
UINT8				g_first_run = 1;


//Function definitions
VOID sys_init()
{
	// Set A7 to our kernel stack
	g_asmBridge = &__end;
	g_kernelStack = &__end;
	asm("move.l g_asmBridge, %a7");
	      
	asm( "move.l #asm_trap_entry,%d0" );
	asm( "move.l %d0,0x10000080" );	
	
	asm( "move.w #0x2000,%SR" );
	
	UINT8 i = 0;
	
    //Priority queues
	for(i = 0; i < 4; i++)
	{
		UINT8 j = 0;
		for(j = 0; j < 2; j++)
		{
			g_priority_ready_tracker[i][j] = -1;
		}
	}
	
	//Initialize processes
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		UINT32 * addr;
		g_test_proc_table[i].m_process_ID = i + 1;
		g_test_proc_table[i].m_priority   = 3;
		g_test_proc_table[i].m_stack      = &__end + KERNEL_STACK_SIZE + (i+1)*(PROCESS_STACK_SIZE);
		g_test_proc_table[i].m_state	  = 1;
		
		// Stack information output
		rtx_dbug_outs("Process ");
		printHexAddress(i);
		rtx_dbug_outs(":\r\nStack begins := ");
		printHexAddress(g_test_proc_table[i].m_stack);
		rtx_dbug_outs(", Entry := ");
		printHexAddress(g_test_proc_table[i].m_entry);
		rtx_dbug_outs("\r\n");
		
		//Construct ESF and initial values for d0..d7, and a0..a6 for each process
		addr = g_test_proc_table[i].m_stack;
		*addr = g_test_proc_table[i].m_entry;
		addr--;
		*addr = 0x40000000;
		
		UINT8 j;
		for(j = 0; j < 15; j++)
		{
			addr--;
			*addr = 0x00000000;
		}
		g_test_proc_table[i].m_stack = addr;
		
		// Add processes to appropriate ready queues
		push(g_test_proc_table[i].m_priority, &g_test_proc_table[i]);
	}
	g_test_proc_table[0].m_entry = test_proc_1;
	g_test_proc_table[1].m_entry = test_proc_2;
	g_test_proc_table[2].m_entry = test_proc_3;
	g_test_proc_table[3].m_entry = test_proc_4;
	g_test_proc_table[4].m_entry = test_proc_5;
	g_test_proc_table[5].m_entry = test_proc_6;
	
	// Set up the null process
	g_null_proc.m_process_ID = 0;
	g_null_proc.m_priority   = 4;
	g_null_proc.m_stack      = &__end + KERNEL_STACK_SIZE;
	g_null_proc.m_entry      = null_process;
	
	asm("move.l %a7, g_asmBridge"); //Save kernel stack location
	g_kernelStack = g_asmBridge;
	scheduler(); // Scheduler picks the next process, and loads it's stack pointer into g_asmBridge
	asm("move.l g_asmBridge, %a7"); // Load the selected process' stack pointer into A7
	
	// Back up registers and then RTE
	asm("move.l (%a7)+, %a6");
	asm("move.l (%a7)+, %a5");
	asm("move.l (%a7)+, %a4");
	asm("move.l (%a7)+, %a3");
	asm("move.l (%a7)+, %a2");
	asm("move.l (%a7)+, %a1");
	asm("move.l (%a7)+, %a0");
	asm("move.l (%a7)+, %d7");
	asm("move.l (%a7)+, %d6");
	asm("move.l (%a7)+, %d5");
	asm("move.l (%a7)+, %d4");
	asm("move.l (%a7)+, %d3");
	asm("move.l (%a7)+, %d2");
	asm("move.l (%a7)+, %d1");
	asm("move.l (%a7)+, %d0");
	
	asm("rte");
}

VOID scheduler( VOID )
{
	
	// If this isn't the first time the scheduler is run, then save the current_process information
	if(g_first_run == 0)
	{
		asm("move.l %a7, g_asmBridge");
        g_current_process->m_stack = (g_asmBridge + 0xC);
		g_current_process->m_state = 1;
		push(g_current_process->m_priority, g_current_process);
		
	}
	g_first_run = 0; // Record that we have run the scheduler once
	
	UINT8 i;
	for(i = 0; i < 4; i++)
	{
		if(pop(i, &g_current_process) != -1)
		{
			break;							
		}
	}
	
	// If we could not find any ready process, start our null process
	if(i == 4)
	{
		g_current_process = &g_null_proc;
	}
	
	// Set process state of selected process to running and restore its stack
	g_current_process->m_state = 2;
	g_asmBridge = g_current_process->m_stack;
}


// Voluntarily called by process
SINT8 release_processor()
{
	UINT32 *addr = 0;
	asm("move.l %a7, g_asmBridge");
	addr = g_asmBridge + 0x8;
	g_asmBridge = g_asmBridge + 0xC;
	asm("move.l g_asmBridge, %a7");
	g_asmBridge = *addr;
	
    asm("TRAP #0");
	return 0;
}

SINT8 send_message(UINT8 process_ID, VOID * MessageEnvelope)
{
    
    return 0;   
}

VOID * receive_message(UINT8 * sender_ID)
{
    return (VOID*)0;
}

SINT8 set_process_priority(UINT8 process_ID, UINT8 priority)
{
	if(priority >= 4) //Invalid priority level
	{
		return -1;
	}
	if(process_ID == 0) 
	{
		if(priority != 4) //Trying to change priority level of null process
		{
			return -1;
		}
	}
	UINT8 i = 0;
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if (g_test_proc_table[i].m_process_ID == process_ID)
		{
			rtx_dbug_outs((CHAR *)"Current priority queue looks like this:\n\r");
			UINT8 j = 0;
			for(j = 0; j < NUM_PROCESSES; j++)
			{
				printHexAddress(g_priority_ready[g_test_proc_table[i].m_priority][j].m_process_ID);
				rtx_dbug_outs((CHAR *)"\n\r");
			}
			//g_test_proc_table[i].m_priority = priority;
			struct s_pcb * catcher;
			UINT8 k = g_priority_ready[g_test_proc_table[i].m_priority][g_priority_ready_tracker[g_test_proc_table[i].m_priority][0]].m_process_ID;
			rtx_dbug_outs((CHAR *)"head of queue is:");
			printHexAddress(g_test_proc_table[i].m_priority][0]);
			rtx_dbug_outs((CHAR *)"\n\r");
			rtx_dbug_outs((CHAR *)"k is:");
			printHexAddress(k);
			rtx_dbug_outs((CHAR *)"\n\r");
			rtx_dbug_outs((CHAR *)"process_ID is:");
			printHexAddress(process_ID);
			rtx_dbug_outs((CHAR *)"\n\r");
			if(k == process_ID)//Process being changed is current queue head
			{
				rtx_dbug_outs((CHAR *)"process being modified is head\n\r");
				pop(g_test_proc_table[i].m_priority, &catcher);
				g_test_proc_table[i].m_priority = priority;
				push(g_test_proc_table[i].m_priority, catcher);
				return 0;
			}
			do//Find element to pop, modify it, and push to new priority queue
			{
				pop(g_test_proc_table[i].m_priority, &catcher);
				rtx_dbug_outs((CHAR *)"catcher process ID:");
				printHexAddress(catcher->m_process_ID);
				rtx_dbug_outs((CHAR *)"\n\r");
				if(catcher->m_process_ID == process_ID)
				{
					rtx_dbug_outs((CHAR *)"proces s being modified has process ID:");
					printHexAddress(g_test_proc_table[i].m_process_ID);
					rtx_dbug_outs((CHAR *)"\n\r");
					g_test_proc_table[i].m_priority = priority;
				}
				push(g_test_proc_table[i].m_priority, catcher);
			}while(k != g_priority_ready[g_test_proc_table[i].m_priority][g_priority_ready_tracker[g_test_proc_table[i].m_priority][0]].m_process_ID);
			/*if(g_priority_ready[g_test_proc_table[i].m_priority][g_priority_ready_tracker[g_test_proc_table[i].m_priority][0]].m_process_ID == process_ID)//We're modifying priority of head
			{
				pop(g_test_proc_table[i].m_priority, &catcher);
				g_test_proc_table[i].m_priority = priority;
				push(g_test_proc_table[i].m_priority, &g_test_proc_table[i]);
			}
			else
			{
				UINT8 k = g_priority_ready_tracker[g_test_proc_table[i].m_priority][0];
				if(k == NUM_PROCESSES - 1)
				{
					k = 0;
				}
				else
				{
					k++;
				}
				while(k != g_priority_ready_tracker[g_test_proc_table[i].m_priority][1])//Find element in array to be modified
				{
					if(g_priority_ready[g_test_proc_table[i].m_priority][k].m_process_ID == process_ID)//Found it!
					{
						while(k != g_priority_ready_tracker[g_test_proc_table[i].m_priority][1])//Iterate from found element to tail, essentially performing a shift on the array elements to delete the desired element
						{
							if(k+1 == NUM_PROCESSES -1)//Account for wrap-around behavour of queue-as-array
							{
								g_priority_ready[priority][k].m_process_ID = g_priority_ready[priority][0].m_process_ID;
								g_priority_ready[priority][k].m_priority = g_priority_ready[priority][0].m_priority;
								g_priority_ready[priority][k].m_state = g_priority_ready[priority][0].m_state;
								g_priority_ready[priority][k].m_stack = g_priority_ready[priority][0].m_stack;	
								g_priority_ready[priority][k].m_entry = g_priority_ready[priority][0].m_entry;
								k = 0;
							}
							else
							{
								g_priority_ready[priority][k].m_process_ID = g_priority_ready[priority][k+1].m_process_ID;
								g_priority_ready[priority][k].m_priority = g_priority_ready[priority][k+1].m_priority;
								g_priority_ready[priority][k].m_state = g_priority_ready[priority][k+1].m_state;
								g_priority_ready[priority][k].m_stack = g_priority_ready[priority][k+1].m_stack;	
								g_priority_ready[priority][k].m_entry = g_priority_ready[priority][k+1].m_entry;
								k++;
							}
						}
						if(g_priority_ready_tracker[g_test_proc_table[i].m_priority][1] == 0)//Update stored tail index after element has been removed.
						{
							g_priority_ready_tracker[g_test_proc_table[i].m_priority][1] = NUM_PROCESSES - 1;
						}
						else
						{
							g_priority_ready_tracker[g_test_proc_table[i].m_priority][1]--;
						} 
						g_test_proc_table[i].m_priority = priority;
						push(g_test_proc_table[i].m_priority, &g_test_proc_table[i]);
						break;
					}
					else
					{
						if(k == NUM_PROCESSES - 1)
						{
							k = 0;
						}
						else
						{
							k++;
						}
					}
				}
			}*/
			return 0;
		}
	}
	return -1;
}

SINT8 get_process_priority(UINT8 process_ID)
{

    UINT8 i = 0;
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if (g_test_proc_table[i].m_process_ID == process_ID)
		{
			return g_test_proc_table[i].m_priority;
		}
	}
	return -1;
}

SINT8 pop(UINT8 priority, struct s_pcb ** catcher)
{
    /*      rtx_dbug_outs((CHAR *)"Old Tail\r\n");
	  rtx_dbug_out_char('0' + g_priority_ready_tracker[priority][0]);
          rtx_dbug_outs((CHAR *)"\r\n");
	  printHexAddress((UINT32)&g_priority_ready_tracker[priority][0]);
          rtx_dbug_outs((CHAR *)"\r\n");
*/	
		   
 if(g_priority_ready_tracker[priority][0] == (SINT8) -1)
	{
//		rtx_dbug_outs((CHAR *)"Tail is negative\r\n");
		return -1;
	}
	else
	{

 //		rtx_dbug_outs((CHAR *)"Catching something\r\n");
		
                *catcher = &g_priority_ready[priority][g_priority_ready_tracker[priority][0]];

/*		rtx_dbug_outs((CHAR *)"Caught\r\n");
		printHexAddress((UINT32) *catcher);
		rtx_dbug_outs((CHAR *)"\r\n");
	        printHexAddress((UINT32) &g_priority_ready[priority][g_priority_ready_tracker[priority][0]]);
	        rtx_dbug_outs((CHAR *)"\r\n");
*/  
		if(g_priority_ready_tracker[priority][0] == NUM_PROCESSES - 1)
		{
			//rtx_dbug_outs((CHAR *)"Reset Tail\r\n");
			g_priority_ready_tracker[priority][0] = 0;
		}
		else
		{
			//rtx_dbug_outs((CHAR *)"Incrementing Tail\r\n");
			g_priority_ready_tracker[priority][0]++;
		}
	}
    return 0;
}

SINT8 push(UINT8 priority, struct s_pcb * new_back)
{

      if(g_priority_ready_tracker[priority][1] == -1)
	{
		//rtx_dbug_outs((CHAR *)"Initialize Priority Queue\r\n");
		g_priority_ready_tracker[priority][0] = 0;
		g_priority_ready_tracker[priority][1] = 0;
		g_priority_ready[priority][0] = *new_back;
		/*
		g_priority_ready[priority][0].m_process_ID = new_back->m_process_ID;
                g_priority_ready[priority][0].m_priority = new_back->m_priority;
                g_priority_ready[priority][0].m_state = new_back->m_state;
                g_priority_ready[priority][0].m_stack = new_back->m_stack;
                g_priority_ready[priority][0].m_entry = new_back->m_entry;
		//g_priority_ready[priority][0].m_a7 = new_back->m_a7;
		*/
			
	}
	else
	{
	      //rtx_dbug_outs((CHAR *) "Reg Push in  Priority Queue\r\n");
		//Check if queue is full
		if((g_priority_ready_tracker[priority][1] - g_priority_ready_tracker[priority][0])*
		   (g_priority_ready_tracker[priority][1] - g_priority_ready_tracker[priority][0]) == 
		   NUM_PROCESSES*NUM_PROCESSES)
		{
			return -1;
		}
		else
		{ 
		//  rtx_dbug_outs((CHAR *)"Priority Queue Not Full\r\n");

			if(g_priority_ready_tracker[priority][1] + 1 >= NUM_PROCESSES)
			{
				g_priority_ready_tracker[priority][1] = 0;
			}
			else
			{
				g_priority_ready_tracker[priority][1]++;
				//rtx_dbug_outs((CHAR *)"New head\r\n");
				//rtx_dbug_out_char('0' + g_priority_ready_tracker[priority][1]);	
				//rtx_dbug_outs((CHAR *)"\r\n");
			}
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_process_ID = new_back->m_process_ID;
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_priority = new_back->m_priority;
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_state = new_back->m_state;
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_stack = new_back->m_stack;	
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_entry = new_back->m_entry;
		}
	}
    return 0;
 }
