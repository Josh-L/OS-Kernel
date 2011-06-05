#include "system.h"

struct s_pcb           	g_null_proc;
struct s_pcb         	g_test_proc_table[NUM_PROCESSES];
struct s_pcb 			g_priority_ready[4][NUM_PROCESSES];
UINT8                   g_priority_ready_tracker[4][2];
struct s_pcb 	       *g_current_process;
UINT32 					g_asmBridge;                       //Used as a temp variable to get contents of address and data registers into C structs.

//Function definitions
VOID sys_init()
{
	rtx_dbug_outs((CHAR *)"sys_init\n\r");
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
		g_test_proc_table[i].m_process_ID = i + 1;
		g_test_proc_table[i].m_priority   = 3;
		g_test_proc_table[i].m_stack      = PROCESS_STACK_SIZE;
		g_test_proc_table[i].m_state	  = 1;
	}
	g_test_proc_table[0].m_entry = test_proc_1;
	g_test_proc_table[1].m_entry = test_proc_2;
	g_test_proc_table[2].m_entry = test_proc_3;
	g_test_proc_table[3].m_entry = test_proc_4;
	g_test_proc_table[4].m_entry = test_proc_5;
	g_test_proc_table[5].m_entry = test_proc_6;
	
	//Add processes to appropriate ready queues
	for(i = 0; i < 6; i++)
	{
		push(g_test_proc_table[i].m_priority, &g_test_proc_table[i]);
	}
	
	// Set up the null process
	g_null_proc.m_process_ID = i;
	g_null_proc.m_priority   = 4;
	g_null_proc.m_stack      = PROCESS_STACK_SIZE;
	g_null_proc.m_entry      = null_process;
	
	rtx_dbug_outs((CHAR *)"init scheduler\r\n");
	for(i = 0; i < 4; i++)
	{
		if(pop(i, g_current_process) != -1)
		{
			break;							
		}
	}
	
	if(i == 4)
	{
		g_current_process = &g_null_proc;
	}
	rtx_dbug_outs((CHAR *)"init scheduler finished\r\n");
}

SINT8 scheduler()
{
	

	
	rtx_dbug_outs((CHAR *)"scheduler\r\n");
	UINT8 i;
	for(i = 0; i < 4; i++)
	{
		if(pop(i, g_current_process) != -1)
		{
			break;							
		}
	}
	
	if(i == 4)
	{
		g_current_process = &g_null_proc;
	}
	
	
	g_asmBridge = g_current_process->m_d[0];
	asm("move.l g_asmBridge, %d0");
	g_asmBridge = g_current_process->m_d[1];
	asm("move.l g_asmBridge, %d1");
	g_asmBridge = g_current_process->m_d[2];
	asm("move.l g_asmBridge, %d2");
	g_asmBridge = g_current_process->m_d[3];
	asm("move.l g_asmBridge, %d3");
	g_asmBridge = g_current_process->m_d[4];
	asm("move.l g_asmBridge, %d4");
	g_asmBridge = g_current_process->m_d[5];
	asm("move.l g_asmBridge, %d5");
	g_asmBridge = g_current_process->m_d[6];
	asm("move.l g_asmBridge, %d6");
	g_asmBridge = g_current_process->m_d[7];
	asm("move.l g_asmBridge, %d7");
	g_asmBridge = g_current_process->m_a[0];
	asm("move.l g_asmBridge, %a0");
	g_asmBridge = g_current_process->m_a[1];
	asm("move.l g_asmBridge, %a1");
	g_asmBridge = g_current_process->m_a[2];
	asm("move.l g_asmBridge, %a2");
	g_asmBridge = g_current_process->m_a[3];
	asm("move.l g_asmBridge, %a3");
	g_asmBridge = g_current_process->m_a[4];
	asm("move.l g_asmBridge, %a4");
	g_asmBridge = g_current_process->m_a[5];
	asm("move.l g_asmBridge, %a5");
	g_asmBridge = g_current_process->m_a[6];
	asm("move.l g_asmBridge, %a6");
	g_asmBridge = g_current_process->m_a[7];
	asm("move.l g_asmBridge, %a7");
	
	g_current_process->m_state = 2;
	
	return 0;
}

//voluntarily called by process
SINT8 release_processor()
{	
	rtx_dbug_outs((CHAR *)"release_processor\n\r");
	//backing up data registers.
	asm("move.l %D0, g_asmBridge");
	g_current_process->m_d[0] = g_asmBridge;
	asm("move.l %D1, g_asmBridge");
	g_current_process->m_d[1] = g_asmBridge;
	asm("move.l %D2, g_asmBridge");
	g_current_process->m_d[2] = g_asmBridge;
	asm("move.l %D3, g_asmBridge");
	g_current_process->m_d[3] = g_asmBridge;
	asm("move.l %D4, g_asmBridge");
	g_current_process->m_d[4] = g_asmBridge;
	asm("move.l %D5, g_asmBridge");
	g_current_process->m_d[5] = g_asmBridge;
	asm("move.l %D6, g_asmBridge");
	g_current_process->m_d[6] = g_asmBridge;
	asm("move.l %D7, g_asmBridge");
	g_current_process->m_d[7] = g_asmBridge;
	asm("move.l %a0, g_asmBridge");
	g_current_process->m_a[0] = g_asmBridge;
	asm("move.l %a1, g_asmBridge");
	g_current_process->m_a[1] = g_asmBridge;
	asm("move.l %a2, g_asmBridge");
	g_current_process->m_a[2] = g_asmBridge;
	asm("move.l %a3, g_asmBridge");
	g_current_process->m_a[3] = g_asmBridge;
	asm("move.l %a4, g_asmBridge");
	g_current_process->m_a[4] = g_asmBridge;
	asm("move.l %a5, g_asmBridge");
	g_current_process->m_a[5] = g_asmBridge;
	asm("move.l %a6, g_asmBridge");
	g_current_process->m_a[6] = g_asmBridge;
	asm("move.l %a7, g_asmBridge");
	g_current_process->m_a[7] = g_asmBridge;
	rtx_dbug_outs((CHAR *)"dbug 1");
	if(g_current_process != 0){
		g_current_process->m_state = 1;
	}
	rtx_dbug_outs((CHAR *)"dbug 2");
	push(g_current_process->m_priority, g_current_process);
	rtx_dbug_outs((CHAR *)"dbug 3");
	//Invoke scheduler to reassign g_current_process to new process to run
	scheduler();
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
	UINT8 i = 0;
	for(i = 0; i < NUM_PROCESSES; i++)
	{
		if (g_test_proc_table[i].m_process_ID == process_ID)
		{
			g_test_proc_table[i].m_priority = priority;
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

SINT8 pop(UINT8 priority, struct s_pcb * catcher)
{	
	rtx_dbug_outs((CHAR *)"pop start\n");
    if(g_priority_ready_tracker[priority][0] == -1)
	{
		return -1;
	}
	else
	{
		catcher = &g_priority_ready[priority][g_priority_ready_tracker[priority][0]];
		if(g_priority_ready_tracker[priority][0] == NUM_PROCESSES - 1)
		{
			g_priority_ready_tracker[priority][0] = 0;
		}
		else
		{
			g_priority_ready_tracker[priority][0]++;
		}
	}
    return 0;
}

SINT8 push(UINT8 priority, struct s_pcb * new_back)
{
    if(g_priority_ready_tracker[priority][1] == -1)
	{
		g_priority_ready_tracker[priority][0] = 0;
		g_priority_ready_tracker[priority][1] = 0;
		g_priority_ready[priority][0] = *new_back;
	}
	else
	{
		//Check if queue is full
		if((g_priority_ready_tracker[priority][1] - g_priority_ready_tracker[priority][0])*
		   (g_priority_ready_tracker[priority][1] - g_priority_ready_tracker[priority][0]) == 
		   NUM_PROCESSES*NUM_PROCESSES)
		{
			return -1;
		}
		else
		{
			if(g_priority_ready_tracker[priority][1] + 1 >= NUM_PROCESSES)
			{
				g_priority_ready_tracker[priority][1] = 0;
			}
			else
			{
				g_priority_ready_tracker[priority][1]++;
			}
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_process_ID = new_back->m_process_ID;
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_priority = new_back->m_priority;
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_state = new_back->m_state;
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_stack = new_back->m_stack;
			
			UINT8 i = 0;
			for(i = 0; i < 8; i++)
			{
				g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_a[i] = new_back->m_a[i];
				g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_d[i] = new_back->m_d[i];
			}
			g_priority_ready[priority][g_priority_ready_tracker[priority][1]].m_entry = new_back->m_entry;
		}
	}
    return 0;
}