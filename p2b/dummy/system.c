#include "system.h"

UINT8				    	gp_mem_pool_lookup[NUM_MEM_BLKS]; // Stores whether or not the corresponding 
UINT32 						gp_mem_pool_list[NUM_MEM_BLKS]; // List of addresses of memory blocks


struct s_pcb 	    		*g_current_process;

struct s_pcb_queue			g_priority_queues[NUM_PRIORITIES];
struct s_pcb_queue_item 	g_queue_slots[NUM_PROCESSES]; // Have an array of ready queue slots

// Array of queues to hold processes that are blocked waiting on memory
struct s_pcb_queue			g_mem_blocking_queue[NUM_PRIORITIES];
struct s_pcb_queue_item 	g_mem_blocking_queue_slots[NUM_PROCESSES];

UINT32						*g_kernelStack = 0; // Pointer to kernel stack
UINT32						g_asmBridge = 0;
UINT8						g_first_run = 1;

extern struct s_pcb        	g_proc_table[NUM_PROCESSES];
extern  UINT32				g_free_mem; // Keep track of the beginning of the free memory region

//Initializations
VOID sys_init()
{
	// Set A7 to our kernel stack
	g_kernelStack = g_free_mem = g_free_mem + KERNEL_STACK_SIZE;
	g_asmBridge = g_kernelStack;
	asm("move.l g_asmBridge, %a7");
	      
	// Insert TRAP subroutines into vector table
	asm("move.l #release_proc_trap,%d0");
	asm("move.l %d0,0x10000080");	
	asm("move.l #set_proc_priority_trap,%d0");
	asm("move.l %d0,0x10000084");
	asm("move.l #get_proc_priority_trap,%d0");
	asm("move.l %d0,0x10000088");
	asm("move.l #request_mem_block_trap,%d0");
	asm("move.l %d0,0x1000008C");
	asm("move.l #release_mem_block_trap,%d0");
	asm("move.l %d0,0x10000090");
	asm("move.l #send_msg_trap,%d0");
	asm("move.l %d0,0x10000094");
	asm("move.l #receive_msg_trap,%d0");
	asm("move.l %d0,0x10000098");
	
	// Setting the status register to allow interrupts, not needed now
	//asm( "move.w #0x2000,%SR" );
	
	UINT8 i = 0;
	UINT32 * addr;
	
	// Initialize free memory blocks
	for (i = 0; i < NUM_MEM_BLKS; i++)
    {
		gp_mem_pool_lookup[i] = 0;
        gp_mem_pool_list[i] = g_free_mem = g_free_mem + (i * MEM_BLK_SIZE);
    }
	
    // Initialize priority and mem blocking queues
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		g_priority_queues[i].front = 0;
		g_priority_queues[i].back = 0;
		g_priority_queues[i].num_slots = NUM_PROCESSES;
		
		g_mem_blocking_queue[i].front = 0;
		g_mem_blocking_queue[i].back = 0;
		g_mem_blocking_queue[i].num_slots = NUM_PROCESSES;

	}
	
	// Setup all processes
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		// Initialize message box and message waiting data structures
		UINT8 k = 0;
		for(k = 0; k < NUM_PROCESSES; k++)
		{
			g_proc_table[i].msg_queue_slots[k].data = 0;
			g_proc_table[i].msg_queue_slots[k].next = 0;
		}
		
		g_proc_table[i].msg_queue.front = 0;
		g_proc_table[i].msg_queue.back = 0;
		g_proc_table[i].msg_queue.num_slots = NUM_PROCESSES;
		
		
		// Setup a blank queue slot for this process
		g_queue_slots[i].data = 0;
		g_queue_slots[i].next = 0;
		g_mem_blocking_queue_slots[i].data = 0;
		g_mem_blocking_queue_slots[i].next = 0;
		
		// Push process into proper priority queue
		push(&g_priority_queues[g_proc_table[i].m_priority], g_queue_slots, &g_proc_table[i]);
		
		// Setup blank ESF
		addr = g_proc_table[i].m_stack;
		*addr = g_proc_table[i].m_entry;
		addr--;
		*addr = 0x40000000;
		
		// Push blank data and address register values to the stack to restore on first run
		for(k = 0; k < 15; k++)
		{
			addr--;
			*addr = 0x00000000;
		}
		
		// Set the process stack pointer to the top of the stack
		g_proc_table[i].m_stack = addr;
		
		/*// Stack information output
		rtx_dbug_outs("Process ");
		printHexAddress(i);
		rtx_dbug_outs(":\r\nStack begins := ");
		printHexAddress(g_proc_table[i].m_stack);
		rtx_dbug_outs(", Entry := ");
		printHexAddress(g_proc_table[i].m_entry);
		rtx_dbug_outs("\r\n");*/
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
		if(g_current_process->m_state == 2) //If calling process was running 
		{
			g_current_process->m_state = 1;
			if (g_current_process->m_process_ID >= 0)
			{
				push(&g_priority_queues[g_current_process->m_priority], g_queue_slots, g_current_process);
			}
		}
	}
	else
	{
		g_first_run = 0; // Record that we have run the scheduler once
	}
	
	UINT8 i;
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		//if(pop(i, &g_current_process) != -1)
		if(pop(&g_priority_queues[i], g_queue_slots, &g_current_process) != -1)
		{
			break;
		}
	}
	
	// Set process state of selected process to running and restore its stack
	g_current_process->m_state = 2;
	g_asmBridge = g_current_process->m_stack;

	
	//rtx_dbug_outsrtx_dbug_outs("RELEASE PROCESSOR NEW PROC ");
	//rtx_dbug_out_char('0' + g_current_process->m_process_ID);
 	//rtx_dbug_outs((CHAR *)"\r\n");
}

int release_processor()
{

	
        rtx_dbug_out_char('0' + g_current_process->m_process_ID);
        rtx_dbug_outs(" is attempting to release itself ");
	
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

int send_message(int process_ID, VOID * messageEnvelope)
{
	int returnVal = 0;
	
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
	
	//Place process_ID and messageEnvelope into d2 and d3 respectively to pass to send_message_trap_handler
	g_asmBridge = process_ID;
	asm("move.l g_asmBridge, %d2");
	g_asmBridge = (UINT32 *)messageEnvelope;
	asm("move.l g_asmBridge, %d3");
	
	asm("TRAP #5");
	
	//Take return value from d1
	asm("move.l %d1, g_asmBridge");
	returnVal = g_asmBridge;
	
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

VOID send_message_trap_handler()
{
	int process_ID;
	VOID * messageEnvelope;
	


	asm("move.l %d2, g_asmBridge");
	process_ID = g_asmBridge;
	
	asm("move.l %d3, g_asmBridge");
	messageEnvelope = (VOID *)g_asmBridge;
	
    /*
	Insert message data structure into the destination process' message queue.
	If the destination process was blocked due to waiting for a message,
	unblock it and add it back to the scheduler queues.
	*/
	
	struct s_message * new_message = messageEnvelope;
	new_message->sender_id = g_current_process->m_process_ID;
	new_message->dest_id = process_ID;
	
	UINT8 i = 0;
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if(g_proc_table[i].m_process_ID == process_ID)
		{	
			message_push(&g_proc_table[i].msg_queue, g_proc_table[i].msg_queue_slots, new_message);
			
			// If receiving process is currently blocking on message from sender, unblock and push to ready queue
			if(g_proc_table[i].m_state == 3)
			{
				g_proc_table[i].m_state = 1;
				push(&g_priority_queues[g_proc_table[i].m_priority], g_queue_slots, &g_proc_table[i]);
			}
			break;
		}
	}
	
	if (i == NUM_PROCESSES)
	{
		g_asmBridge = -1;
	}
	else
	{
		g_asmBridge = 0;
	}
	
	asm("move.l g_asmBridge, %d1");
}

VOID * receive_message(int * sender_ID)
{
	VOID * returnVal = 0;
	
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
	
	asm("move.l #0, %d1");
	
	g_asmBridge = sender_ID;
	asm("move.l g_asmBridge, %d2");
	
	asm("TRAP #6");
	
	//Take return value from d1
	asm("move.l %d1, g_asmBridge");
	returnVal = g_asmBridge;
	
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

VOID receive_message_trap_handler()
{
	int * sender_ID;
	asm("move.l %d2, g_asmBridge");
	sender_ID = g_asmBridge;
	
	/*
	Check to see if there is a message waiting in the process' message queue.
	If there is a message, pop it from the queue, set the sender_id parameter
	to the process id the message came from, and return a pointer to the block
	of memory that the message resides in to the process.
	If there is no message, set the process state to blocked on message, and
	release processor. When this process receives a message, the process will
	be re-added to the scheduler queue, and the next time it is executed it
	will handle the message as explained above.
	*/
	
	struct s_message *  msg;
	
	if(message_pop(&g_current_process->msg_queue, g_current_process->msg_queue_slots, &msg) == -1){
		g_current_process->m_state = 3;
		release_processor();
		message_pop(&g_current_process->msg_queue, g_current_process->msg_queue_slots, &msg);
	}

	*sender_ID = msg->sender_id;
	

	g_asmBridge = msg;
	asm("move.l g_asmBridge, %d1");
}

SINT8 pop(struct s_pcb_queue * queue, struct s_pcb_queue_item slots[], struct s_pcb ** catcher)
{
	if (queue->front == 0)
	{
		return -1;
	}
	*catcher = queue->front->data;
	queue->front->data = 0;
	if (queue->front == queue->back)
	{
		queue->front = 0;
		queue->back = 0;
	}
	else
	{
		queue->front = queue->front->next;
	}
	
    return 0;
}

SINT8 push(struct s_pcb_queue * queue, struct s_pcb_queue_item slots[], struct s_pcb * new_back)
{
	// Find a free node to use
	UINT8 i = 0;
	for (i = 0; i < queue->num_slots; i++)
	{
		if(slots[i].data == 0)
		{
			break;
		}
	}
	
	// Trying to push a process that is already on the queues
	if (i == queue->num_slots)
	{
		return -1;
	}
	
	slots[i].data = new_back;
	
	if (queue->back == 0)
	{
		queue->front = &slots[i];
	}
	else
	{
		queue->back->next = &slots[i];
	}
	queue->back = &slots[i];
	
    return 0;
}

int set_process_priority(int process_ID, int priority)
{
	int returnVal = 0;
	
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
	g_asmBridge = process_ID;
	asm("move.l g_asmBridge, %d2");
	g_asmBridge = priority;
	asm("move.l g_asmBridge, %d3");
	
	asm("TRAP #1");
	
	//Take return value from d1
	asm("move.l %d1, g_asmBridge");
	returnVal = g_asmBridge;
	
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
	int priority = 0;
	int process_ID = 0;
	
	//Retrieve passed parameters from d2 and d3
	asm("move.l %d3, g_asmBridge");
	priority = g_asmBridge;
	asm("move.l %d2, g_asmBridge");
	process_ID = g_asmBridge;

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
	int oldPriority = 0;
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
		pop(&g_priority_queues[oldPriority], g_queue_slots, &temp);
	}
	else
	{
		if (g_priority_queues[oldPriority].front == g_priority_queues[oldPriority].back)
		{
			struct s_pcb * temp;
			pop(&g_priority_queues[oldPriority], g_queue_slots, &temp);
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
	push(&g_priority_queues[priority], g_queue_slots, &g_proc_table[i]);
	
	//Put return value in d2
	g_asmBridge =  0;
	asm("move.l g_asmBridge, %d2");
}

int get_process_priority(int process_ID)
{
	int returnVal = 0;
	
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
	
	//Place process_ID into d2 to pass to get_process_priority_trap_handler
	g_asmBridge = process_ID;
	asm("move.l g_asmBridge, %d2");
	
	asm("TRAP #2");
	
	//Take return value from d1
	asm("move.l %d2, g_asmBridge");
	returnVal = g_asmBridge;
	
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

VOID get_process_priority_trap_handler()
{
    	UINT8 i = 0;
	int process_ID;
	
	asm("move.l %d2, g_asmBridge");
	process_ID = g_asmBridge;
	
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if (g_proc_table[i].m_process_ID == process_ID)
		{
			g_asmBridge = g_proc_table[i].m_priority;
			asm("move.l g_asmBridge, %d2");
			return;
		}
	}
	g_asmBridge = -1;
	asm("move.l g_asmBridge, %d2");
}

VOID * request_memory_block()
{
	VOID * returnVal = 0;
	
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
	
	asm("TRAP #3");
	
	//Take return value from d1
	asm("move.l %d2, g_asmBridge");
	returnVal = (VOID *)(g_asmBridge);
	
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

VOID request_memory_block_trap_handler()
{
	VOID * freeBlock = 0;

    UINT8 i;
	for (i = 0; i < NUM_MEM_BLKS; i++)
	{
		if (gp_mem_pool_lookup[i] == 0)
		{
			gp_mem_pool_lookup[i] = 1;
			freeBlock = (VOID *)gp_mem_pool_list[i];
			break;
		}
	}
	
	if(freeBlock == 0)
	{
		/*If program reaches this point, there are no free memory blocks (freeBlock still unassigned).
		This means the current process must be set to blocked, added to the appropriate
		blocking queue and release processor until it is moved to the ready queue by a
		release_memory_block operation.*/
		

		g_current_process->m_state = 0;
		push(&g_mem_blocking_queue[g_current_process->m_priority], g_mem_blocking_queue_slots, g_current_process); //Push current process to memory_blocked queue indicating that it's waiting for a memory block
			
			
		release_processor();
		
		// Obtain reserved block
		for (i = 0; i < NUM_MEM_BLKS; i++)
		{
			if (gp_mem_pool_lookup[i] == 2)
			{
				
				gp_mem_pool_lookup[i] = 1;
				freeBlock = (VOID *)gp_mem_pool_list[i];
				break;
			}
		}
	}
	
    g_asmBridge = freeBlock;
	asm("move.l g_asmBridge, %d2");
}

int release_memory_block(VOID * memory_block)
{
	int returnVal = 0;
	
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
	
	//Put memory_block into d2 to pass to release_memory_block_trap_handler
	g_asmBridge = memory_block;
	asm("move.l g_asmBridge, %d2");
	
	asm("TRAP #4");
	
	//Take return value from d1
	asm("move.l %d2, g_asmBridge");
	returnVal = g_asmBridge;
	
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

VOID release_memory_block_trap_handler()
{
	
	VOID * memory_block;
	asm("move.l %d2, g_asmBridge");
	memory_block = (VOID *)g_asmBridge;


	struct s_pcb * previously_blocking_proc;
    UINT8 i;
	UINT8 j;
	
    for (i = 0; i < NUM_MEM_BLKS; i++)
	{
		if (gp_mem_pool_list[i] == (UINT32)memory_block && gp_mem_pool_lookup[i] == 1)
		{
			gp_mem_pool_lookup[i] = 0;
			
			/*
			We must iterate through the processes blocked waiting for memory blocks, and unblock (set to ready)
			the highest priority process waiting for a memory block. If all blocked_mem queues are empty, that
			means there are no processes waiting for a memory block to become available and we can simply return.
			*/
			  
		    for(j = 0; j < NUM_PRIORITIES; j++)
		    {
				if(pop(&g_mem_blocking_queue[j], g_mem_blocking_queue_slots, &previously_blocking_proc) != -1)
				{
					previously_blocking_proc->m_state = 1;
					gp_mem_pool_lookup[i] = 2;  //This method was the original idea of group 24 and was used with their permission.
					push(&g_priority_queues[g_current_process->m_priority], g_queue_slots, previously_blocking_proc);
					break;
				}
		    }
			g_asmBridge = 0;
			asm("move.l g_asmBridge, %d2");
			return;
		}
	}
	g_asmBridge = -1;
	asm("move.l g_asmBridge, %d2");
}

int delayed_send(int process_ID, void * MessageEnvelope, int delay)
{
    rtx_dbug_outs((CHAR *)"rtx: delayed_send \r\n");
    return 0;
}

SINT8 message_pop(struct s_message_queue * queue, struct s_message_queue_item slots[], struct s_message ** catcher)
{
	if (queue->front == 0)
	{
		return -1;
	}
	
	*catcher = queue->front->data;
	
	queue->front->data = 0;
	
	if (queue->front == queue->back)
	{	
		queue->front = 0;
		queue->back = 0;
	}
	else
	{
		queue->front = queue->front->next;
	}
	
    return 0;
}

SINT8 message_push(struct s_message_queue * queue, struct s_message_queue_item slots[], struct s_message * new_back)
{
	// Find a free node to use
	UINT8 i = 0;
	for (i = 0; i < queue->num_slots; i++)
	{
		if(slots[i].data == 0)
		{
			break;
		}
	}
	
	// Trying to push a process that is already on the queues
	if (i == queue->num_slots)
	{
		return -1;
	}
	
	slots[i].data = new_back;
	
	if (queue->back == 0)
	{
		queue->front = &slots[i];
	}
	else
	{
		queue->back->next = &slots[i];
	}
	queue->back = &slots[i];
	
    return 0;
}

VOID null_process()
{
	while(1)
	{
		rtx_dbug_outs("null\n\r");
		release_processor();
	}
}
