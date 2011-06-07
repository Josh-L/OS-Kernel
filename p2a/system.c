#include "system.h"

struct s_pcb        g_null_proc;
struct s_pcb        g_test_proc_table[NUM_PROCESSES];
struct s_pcb 		g_priority_ready[4][NUM_PROCESSES];
SINT8               g_priority_ready_tracker[4][2];
struct s_pcb 	    *g_current_process;
UINT32				g_asmBridge = 0;

//Function definitions
VOID sys_init()
{
	g_asmBridge = 0;	
	rtx_dbug_outs((CHAR *)"init trap\r\n ");        
        asm( "move.l #asm_trap_entry,%d0" );
        asm( "move.l %d0,0x10000080" );	
        
        asm( "move.w #0x2000,%SR" );
	
	rtx_dbug_outs((CHAR *)"sys_init\r\n");
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
		g_test_proc_table[i].m_stack      = &__end + i*(PROCESS_STACK_SIZE);
		g_test_proc_table[i].m_state	  = 1;
		
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
		
		rtx_dbug_outs("Process ");
		printHexAddress(i);
		rtx_dbug_outs(":\r\nStack begins := ");
		printHexAddress(g_test_proc_table[i].m_stack);
		rtx_dbug_outs(", Entry := ");
		printHexAddress(g_test_proc_table[i].m_entry);
		rtx_dbug_outs("\r\n");
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
	        /*rtx_dbug_outs((CHAR *)"Head and Tails\r\n");
	        rtx_dbug_out_char('0' + g_priority_ready_tracker[3][0]);
        	rtx_dbug_outs((CHAR *)"\r\n");
        	rtx_dbug_out_char('0' + g_priority_ready_tracker[3][1]);
       		rtx_dbug_outs((CHAR *)"\r\n");
		*/

    }
	
	// Set up the null process
	/*g_null_proc.m_process_ID = i;
	g_null_proc.m_priority   = 4;
	g_null_proc.m_stack      = (&__end + 4096)+ (NUM_PROCESSES )*PROCESS_STACK_SIZE;
	g_null_proc.m_entry      = null_process;*/
	
	/*	
	UINT8 z;
	for(z = 0; z < 6; z++){
	 g_priority_ready[3][z];
	rtx_dbug_outs((CHAR *)"Process\r\n");
        rtx_dbug_out_char('0' +  g_priority_ready[3][z].m_process_ID);
	 rtx_dbug_outs((CHAR *)"\r\n");
	rtx_dbug_out_char('0' + g_priority_ready[3][z].m_priority);
	 rtx_dbug_outs((CHAR *)"\r\n");
	rtx_dbug_out_char('0' + g_priority_ready[3][z].m_state);
	 rtx_dbug_outs((CHAR *)"\r\n");

	}
	

	for(z = 0; z < 6 ; z++){
	 
	rtx_dbug_outs((CHAR *)"Head and Tails\r\n");
        rtx_dbug_out_char('0' + g_priority_ready_tracker[3][0]);
        rtx_dbug_outs((CHAR *)"\r\n");
        rtx_dbug_out_char('0' + g_priority_ready_tracker[3][1]);
        rtx_dbug_outs((CHAR *)"\r\n");
        printHexAddress((UINT32)&g_priority_ready_tracker[3][0]);
	rtx_dbug_outs((CHAR *)"\r\n");

	struct g_pcb *kotaku;
	struct g_pcb *kotaku2;
	kotaku2 = &g_priority_ready[3][g_priority_ready_tracker[3][0]];
	pop (3, kotaku);

	rtx_dbug_outs((CHAR *)"Kotaku and Kotaku2\r\n");
	printHexAddress((UINT32)kotaku);
	rtx_dbug_outs((CHAR *)"\r\n");
	printHexAddress((UINT32)kotaku2);
        rtx_dbug_outs((CHAR *)"\r\n");
	
	if(kotaku2 == kotaku){
		rtx_dbug_outs((CHAR *)"Same Pop\r\n");

	 }else{
		 rtx_dbug_outs((CHAR *)"Different Pops\r\n");

	 }
	}
	*/
	
	
	/*
	UINT8 k;
        for(k = 0; k < 4; k++)
        {     
	   if(pop(k, &g_current_process) != -1)
                {
                   break;
                }
        }

         

        if(k == 4)
        {
                g_current_process = &g_null_proc;
        }
	
	
	rtx_dbug_outs((CHAR *)"Set state\r\n");             	
 	
//        g_current_process = &g_priority_ready[3][g_priority_ready_tracker[3][0]];

	g_current_process->m_state = 2;
	
	rtx_dbug_outs((CHAR *)"Set Stack\r\n");
	
	g_asmBridge = g_current_process->m_stack; 
	asm("move.l g_asmBridge, %a7");
       

        rtx_dbug_outs((CHAR *)"Going to first process\r\n");

         g_current_process->m_entry();
	*/
	g_asmBridge = 0;
	
	scheduler();
	
	printHexAddress(g_asmBridge);
	rtx_dbug_outs("\r\n");
	asm("move.l g_asmBridge, %a7");
	
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
	
	asm("debug:");
	
	asm("rte");
}

VOID scheduler( VOID )
{

	if(g_asmBridge != 0)
	{
	
		asm("move.l %a7, g_asmBridge");
        g_current_process->m_stack = g_asmBridge;   

//	rtx_dbug_outs((CHAR *)"scheduler\r\n");
	
	if(g_current_process != 0)
	{
                g_current_process->m_state = 1;
    }
        
		push(g_current_process->m_priority, g_current_process);
	
	}

	UINT8 i;
	for(i = 0; i < 4; i++)
	{
		if(pop(i, &g_current_process) != -1)
		{
			break;							
		}
	}
	
	if(i == 4)
	{
		g_current_process = &g_null_proc;
	}
	
/*
		rtx_dbug_outs((CHAR *)":");
        printHexAddress((UINT32) g_current_process);
        rtx_dbug_outs((CHAR *)"\r\n");
*/
	
	g_current_process->m_state = 2;
/*	
	        rtx_dbug_outs((CHAR *)"Head and Tails\r\n");
                rtx_dbug_out_char('0' + g_prioity_ready_tracker[3][0]);
                rtx_dbug_outs((CHAR *)"\r\n");
                rtx_dbug_out_char('0' + g_priority_ready_tracker[3][1]);
                rtx_dbug_outs((CHAR *)"\r\n");
*/
	
	g_asmBridge = g_current_process->m_stack;
}


//voluntarily called by process
SINT8 release_processor()
{	

        /*rtx_dbug_outs((CHAR *)"init release_processor\r\n");

	if(g_current_process != 0){
		g_current_process->m_state = 1;
	}
	rtx_dbug_outs((CHAR *)"dbug \r\n");
	push(g_current_process->m_priority, g_current_process);
	*/

//	rtx_dbug_outs((CHAR *)"start release_processor\r\n");
        asm( "TRAP #0" );	
	
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
