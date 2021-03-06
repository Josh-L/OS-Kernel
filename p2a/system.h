#include "memory/rtx_inc.h"
#include "dummy_processes.h"
#include "null_process.h"
#include "memory/memory.h"
#include "Hex_to_ASCII.h"
#include "memory/dbug.h"

#ifndef __SYSTEM__
#define __SYSTEM__

#define KERNEL_STACK_SIZE	2048 // Another temporary value that is suitable for now
#define NUM_PRIORITIES		5

//Structs
struct s_pcb
{
    UINT32  m_process_ID;
    UINT8   m_priority;
    UINT8   m_state;       //0 blocked, 1 ready, 2 running
    UINT32  m_stack;
    VOID    (*m_entry)();
};

struct s_pcb_queue_item
{
	struct s_pcb_queue_item * next;
	struct s_pcb * data;
};


struct s_pcb_queue
{
	struct s_pcb_queue_item * front;
	struct s_pcb_queue_item * back;
};

/*extern struct s_pcb         	g_proc_table[NUM_PROCESSES];
extern struct s_pcb 	       	*g_current_process;
extern UINT32					g_asmBridge;*/

//Function prototypes
VOID    sys_init();
VOID    scheduler( VOID );
SINT8   release_processor();
SINT8   send_message(UINT8 process_ID, VOID * MessageEnvelope);
VOID  * receive_message(UINT8 * sender_ID);
SINT8 pop(UINT8 priority, struct s_pcb ** catcher);
SINT8 push(UINT8 priority, struct s_pcb * new_back);
SINT8   set_process_priority(UINT8 process_ID, UINT8 priority);
VOID iterate(UINT8 priority);

#endif
