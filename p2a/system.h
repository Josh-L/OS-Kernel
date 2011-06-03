#include "memory/rtx_inc.h"
#include "dummy_processes.h"

#define PROCESS_STACK_SIZE 1024 //Temporary for now, needs to be changed once appropriate value is agreed upon.
#define NUM_PROCESSES      6

//Structs
struct s_pcb
{
    UINT32  m_process_ID;
    UINT8   m_priority;
	UINT8   m_state;                      //0 blocked, 1 ready, 2 running
    UINT32  m_stack;
	UINT32  m_a[8];
	UINT32  m_d[8];
    VOID    (*m_entry)();
};

struct s_node
{
    struct s_pcb *m_data;
    struct s_node *m_next;
};

struct s_linked_list
{
    UINT16 length;
    struct s_node *m_front;
    struct s_node *m_back;
};

struct s_pcb           	g_null_proc;
struct s_pcb         	g_test_proc_table[NUM_PROCESSES];
struct s_linked_list 	g_priority_ready[5];
struct s_pcb 	       *g_current_process;
UINT32 					g_asmBridge;

//Function prototypes
VOID    sys_init();
SINT8   scheduler();
SINT8   release_processor();
SINT8   send_message(UINT8 process_ID, VOID * MessageEnvelope);
VOID  * receive_message(UINT8 * sender_ID);
SINT8   set_process_priority(UINT8 process_ID, UINT8 priority);
SINT8   get_process_priority(UINT8 process_ID);
SINT8   pop(struct s_linked_list * q, struct s_node * catcher);
SINT8   push(struct s_linked_list * q, struct s_node * new_back);
struct s_pcb * find_pid(struct s_linked_list * list, UINT8 pid);

//Function definitions
VOID sys_init()
{
	UINT8 i = 0;
	
    //Priority queues
	for(i = 0; i < 5; i++)
	{
		g_priority_ready[i].length  = 0;
		g_priority_ready[i].m_front = 0;
		g_priority_ready[i].m_back  = 0;
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
	
	// Set up the null process
	g_null_proc.m_process_ID = i;
	g_null_proc.m_priority   = 4;
	g_null_proc.m_stack      = PROCESS_STACK_SIZE;
	g_null_proc.m_entry      = null_process;

}

//to be called every 15ms(estimated time, could be changed)
SINT8 scheduler()
{
	struct s_pcb * old_process;
	
	//Check if there exists a current_process, store it in old process.
	if(g_current_process != 0){
		g_current_process->m_state = 1;
		old_process = g_current_process;
	}
	
	
	for(i = 0; i < 5; i++){
		struct * s_linked_list q = g_priority_ready[i];
		if(pop(q, g_current_process) != -1){
            g_current_process->m_state = 2; //change state to running
			break;							//get out of loop
		}
	}
	
	//checking if g_current_process still points to the same address as old process
	struct * s_linked_list q = g_priority_ready[old_process->m_priority];
	push( q,old_process);
	
	return 0;
}

//voluntarily called by process
SINT8 release_processor()
{	
	//backing up data registers.
	asm('move.l D0, g_asmBridge');
	g_current_process->m_d[0] = g_asmBridge;
	asm('move.l D1, g_asmBridge');
	g_current_process->m_d[1] = g_asmBridge;
	asm('move.l D2, g_asmBridge');
	g_current_process->m_d[2] = g_asmBridge;
	asm('move.l D3, g_asmBridge');
	g_current_process->m_d[3] = g_asmBridge;
	asm('move.l D4, g_asmBridge');
	g_current_process->m_d[4] = g_asmBridge;
	asm('move.l D5, g_asmBridge');
	g_current_process->m_d[5] = g_asmBridge;
	asm('move.l D6, g_asmBridge');
	g_current_process->m_d[6] = g_asmBridge;
	asm('move.l D7, g_asmBridge');
	g_current_process->m_d[7] = g_asmBridge;
	
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

SINT8 pop(struct s_linked_list * q, struct s_node * catcher)
{
    if(q->length <= 0)
    {
        return -1;
    }
	if(q == 0)
	{
		return -1;
	}
    catcher = q->m_front;
    q->m_front = q->m_front->m_next;
    q->length--;
    return 0;
}

SINT8 push(struct s_linked_list * q, struct s_node * new_back)
{
    if(q->length == 0)
    {
        q->m_front = new_back;
		q->m_back = new_back;
    }
	if(q == 0 || new_back == 0)
	{
		return -1;
	}
    q->m_back->m_next = new_back;
    new_back->m_next = NULL;
    q->m_back = new_back;
    q->length++;
    return 0;
}

struct s_pcb * find_pid(struct s_linked_list * list, UINT8 pid)
{
	if (list->m_front == 0)
	{
		return 0;
	}
	struct s_node * node = list->m_front;
	while(node != 0)
	{
		if (node->m_data->m_process_ID == pid)
		{
			return (node->m_data);
		}
		node = node->m_next;
	}
	
	return 0;
}
