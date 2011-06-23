#include "system.h"
#include "process_init.h"

struct s_pcb        		g_proc_table[NUM_PROCESSES];
struct s_pcb 	    		*g_current_process;

struct s_pcb_queue			g_priority_queues[NUM_PRIORITIES]; // 0 to 3 priority, don't worry about NULL process priority
struct s_pcb_queue_item 	g_queue_slots[NUM_PROCESSES]; // Have an array of ready queue slots

UINT32						*g_kernelStack = 0;
UINT32						g_asmBridge = 0;
UINT8						g_first_run = 1;

UINT32						g_free_mem = &__end;

//Function definitions
VOID sys_init()
{
	// Set A7 to our kernel stack
	g_kernelStack = g_free_mem = g_free_mem + KERNEL_STACK_SIZE;
	g_asmBridge = g_kernelStack;
	asm("move.l g_asmBridge, %a7");
	      
	// Insert TRAP subroutines into vector table
	asm( "move.l #asm_trap_entry,%d0" );
	asm( "move.l %d0,0x10000080" );	
	asm( "move.l #trap_1_handler,%d0" );
	asm( "move.l %d0,0x10000084" );
	
	// Setting the status register to allow interrupts, not needed now
	//asm( "move.w #0x2000,%SR" );
	
	UINT8 i = 0;
	UINT32 * addr;
	
    // Initialize priority queues
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		g_priority_queues[i].front = 0;
		g_priority_queues[i].back = 0;
	}
	
	// Setup all processes
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		g_proc_table[i].m_process_ID = process_init_table[i].m_process_ID;
		g_proc_table[i].m_priority = process_init_table[i].m_priority;
		g_proc_table[i].m_state = process_init_table[i].m_state;
		g_proc_table[i].m_entry = process_init_table[i].m_entry;
		g_proc_table[i].m_stack = g_free_mem = g_free_mem + process_init_table[i].m_stack;
		
		// Setup a blank queue slot for this process
		g_queue_slots[i].data = 0;
		g_queue_slots[i].next = 0;
		
		// Push process into proper priority queue
		push(g_proc_table[i].m_priority, &g_proc_table[i]);
		
		// Setup blank ESF
		addr = g_proc_table[i].m_stack;
		*addr = g_proc_table[i].m_entry;
		addr--;
		*addr = 0x40000000;
		
		// Push blank data and address register values to the stack to restore on first run
		UINT8 k = 0;
		for(k = 0; k < 15; k++)
		{
			addr--;
			*addr = 0x00000000;
		}
		
		// Set the process stack pointer to the top of the stack
		g_proc_table[i].m_stack = addr;
		
		// Stack information output
		rtx_dbug_outs("Process ");
		printHexAddress(i);
		rtx_dbug_outs(":\r\nStack begins := ");
		printHexAddress(g_proc_table[i].m_stack);
		rtx_dbug_outs(", Entry := ");
		printHexAddress(g_proc_table[i].m_entry);
		rtx_dbug_outs("\r\n");
	}
	
	asm("move.l %a7, g_asmBridge"); //Save kernel stack location
	g_kernelStack = g_asmBridge;
	
	scheduler(); // Scheduler picks the next process, and loads it's stack pointer into g_asmBridge
	
	asm("move.l g_asmBridge, %a7"); // Load the selected process' stack pointer into A7
	
	// Pop blank registers for first process and then RTE
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
		if (g_current_process->m_process_ID != 0)
		{
			push(g_current_process->m_priority, g_current_process);
		}
	}
	g_first_run = 0; // Record that we have run the scheduler once

	UINT8 i;
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		if(pop(i, &g_current_process) != -1)
		{
			break;
		}
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
	
	asm("move.l (%a6), %a6");
	
    asm("TRAP #0");
	return 0;
}

VOID iterate(UINT8 priority)
{
	struct s_pcb_queue_item * temp = g_priority_queues[priority].front;

	while (temp != g_priority_queues[priority].back)
	{
		printHexAddress(temp->data->m_process_ID);
		rtx_dbug_outs("\r\n");
		temp = temp->next;
	}
	if (g_priority_queues[priority].front != 0)
	{
		printHexAddress(temp->data->m_process_ID);
		rtx_dbug_outs("\r\n");
	}
	rtx_dbug_outs("Done\r\n");
}

SINT8 send_message(UINT8 process_ID, VOID * MessageEnvelope)
{
    
    return 0;   
}

VOID * receive_message(UINT8 * sender_ID)
{
    return (VOID*)0;
}

SINT8 pop(UINT8 priority, struct s_pcb ** catcher)
{
	if (priority < 0 || priority > (NUM_PRIORITIES - 2))
	{
		return -1;
	}
	
    if (g_priority_queues[priority].front == 0)
	{
		return -1;
	}
	
	*catcher = g_priority_queues[priority].front->data;
	g_priority_queues[priority].front->data = 0;
	
	if (g_priority_queues[priority].front == g_priority_queues[priority].back)
	{
		g_priority_queues[priority].front = 0;
		g_priority_queues[priority].back = 0;
	}
	else
	{
		g_priority_queues[priority].front = g_priority_queues[priority].front->next;
	}
	
    return 0;
}

SINT8 push(UINT8 priority, struct s_pcb * new_back)
{
    if (priority < 0 || priority > (NUM_PRIORITIES - 2))
	{
		return -1;
	}
	
	// Find a free node to use
	UINT8 i = 0;
	for (i = 0; i < NUM_PROCESSES; i++)
	{
		if(g_queue_slots[i].data == 0)
		{
			break;
		}
	}
	
	// Trying to push a process that is already on the queues
	if (i == NUM_PROCESSES)
	{
		return -1;
	}
	
	g_queue_slots[i].data = new_back;
	
	if (g_priority_queues[priority].back == 0)
	{
		g_priority_queues[priority].front = &g_queue_slots[i];
	}
	else
	{
		g_priority_queues[priority].back->next = &g_queue_slots[i];
	}
	g_priority_queues[priority].back = &g_queue_slots[i];
	
    return 0;
}

SINT8 set_process_priority(UINT8 process_ID, UINT8 priority)
{
	SINT8 returnVal = 0;
	
	//Backup address and data registers
	asm("move.l %d0, -(%a7)");
	asm("move.l %d1, -(%a7)");
	asm("move.l %d2, -(%a7)");
	asm("move.l %d3, -(%a7)");
	asm("move.l %d4, -(%a7)");
	asm("move.l %d5, -(%a7)");
	asm("move.l %d6, -(%a7)");
	asm("move.l %d7, -(%a7)");
	asm("move.l %a0, -(%a7)");
	asm("move.l %a1, -(%a7)");
	asm("move.l %a2, -(%a7)");
	asm("move.l %a3, -(%a7)");
	asm("move.l %a4, -(%a7)");
	asm("move.l %a5, -(%a7)");
	asm("move.l %a6, -(%a7)");
	
	//Place process_ID and priority into d2 and d3 respectively to pass to set_process_priority_trap_handler
	g_asmBridge = (UINT32)process_ID;
	asm("move.l g_asmBridge, %d2");
	g_asmBridge = (UINT32)priority;
	asm("move.l g_asmBridge, %d3");
	
	asm("TRAP #1");
	
	//Take return value from d1
	asm("move.l %d1, g_asmBridge");
	returnVal = (SINT8)(g_asmBridge << 24);
	
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
	
	return returnVal;
}

VOID set_process_priority_trap_handler()
{
	UINT8 priority = 0;
	UINT8 process_ID = 0;
	
	//Retrieve passed parameters from d2 and d3
	asm("move.l %d3, g_asmBridge");
	priority = (UINT8)(g_asmBridge);
	asm("move.l %d2, g_asmBridge");
	process_ID = (UINT8)(g_asmBridge);
	printHexAddress(priority);
	printHexAddress(process_ID);
	asm("debug:");
	// Check valid priority
	if (priority < 0 || priority > (NUM_PRIORITIES - 2))
	{
		//Put return value in d2
		g_asmBridge =  -1;
		asm("move.l g_asmBridge, %d2");
		return;
	}
	
	// Find current priority of process
	UINT8 i = 0;
	UINT8 oldPriority = 0;
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if (g_proc_table[i].m_process_ID == process_ID)
		{
			oldPriority = g_proc_table[i].m_priority;
			break;
		}
	}
	
	// If the process ID doesn't exist
	if (i == NUM_PROCESSES)
	{
		 //Put return value in d2
                g_asmBridge =  -1;
                asm("move.l g_asmBridge, %d2");
                return;
	}
	
	// If the new priority is the same as the old, just return
	if (oldPriority == priority)
	{
		 //Put return value in d2
                g_asmBridge =  0;
                asm("move.l g_asmBridge, %d2");
                return;

	}
	
	if(g_current_process->m_process_ID == process_ID)
	{
		g_proc_table[i].m_priority = priority;
		 //Put return value in d2
                g_asmBridge =  0;
                asm("move.l g_asmBridge, %d2");
                return;

	}
	
	struct s_pcb_queue_item * cur = g_priority_queues[oldPriority].front;
	struct s_pcb_queue_item * prev = g_priority_queues[oldPriority].front;
	
	
	// First remove the process from the old priority list
	if (g_priority_queues[oldPriority].front == 0)
	{
		// Do nothing in this case
	}
	else if (g_priority_queues[oldPriority].front->data == &g_proc_table[i])
	{
		struct s_pcb * temp;
		pop(oldPriority, &temp);
		//g_priority_queues[oldPriority].front->data = 0;
		//g_priority_queues[oldPriority].front = g_priority_queues[oldPriority].front->next;
	}
	else
	{
		if (g_priority_queues[oldPriority].front == g_priority_queues[oldPriority].back)
		{
			struct s_pcb * temp;
			pop(oldPriority, &temp);
		}
		else
			{
			cur = cur->next;
			while (cur != g_priority_queues[oldPriority].back)
			{
				if (cur->data->m_process_ID == process_ID)
				{
					prev->next = cur->next;
					cur->data = 0;
					break;
				}
				prev = cur;
				cur = cur->next;
			}
			if (cur->data->m_process_ID == process_ID)
			{
				cur->data = 0;
				g_priority_queues[oldPriority].back = prev;
			}
		}
	}
	
	// Update the process priority
	g_proc_table[i].m_priority = priority;
	
	// Now push to new priority queue
	push(priority, &g_proc_table[i]);
	
	//Put return value in d2
        g_asmBridge =  0;
        asm("move.l g_asmBridge, %d2");
}

SINT8 get_process_priority(UINT8 process_ID)
{
    UINT8 i = 0;
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if (g_proc_table[i].m_process_ID == process_ID)
		{
			return g_proc_table[i].m_priority;
		}
	}
	return -1;
}
