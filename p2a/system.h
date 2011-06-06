#include "memory/rtx_inc.h"
#include "dummy_processes.h"
#include "null_process.h"
#include "memory/memory.h"
#include "Hex_to_ASCII.h"

#ifndef __SYSTEM__
#define __SYSTEM__

#define PROCESS_STACK_SIZE 1024 //Temporary for now, needs to be changed once appropriate value is agreed upon.
#define NUM_PROCESSES      6

//Structs
struct s_pcb
{
    UINT32  m_process_ID;
    UINT8   m_priority;
    UINT8   m_state;       //0 blocked, 1 ready, 2 running
    UINT32  m_stack;
    // UINT32  m_a7;
    //UINT32  m_a[8];
    //UINT32  m_d[8];
    VOID    (*m_entry)();
};

extern struct s_pcb           	g_null_proc;
extern struct s_pcb         	g_test_proc_table[NUM_PROCESSES];
extern struct s_pcb 			g_priority_ready[4][NUM_PROCESSES];
extern SINT8                   g_priority_ready_tracker[4][2];
extern struct s_pcb 	       *g_current_process;
extern UINT32			g_asmBridge;
extern UINT16			g_asmBridge16;

//Function prototypes
VOID    sys_init();
VOID    scheduler( VOID );
SINT8   release_processor();
SINT8   send_message(UINT8 process_ID, VOID * MessageEnvelope);
VOID  * receive_message(UINT8 * sender_ID);
SINT8   set_process_priority(UINT8 process_ID, UINT8 priority);
SINT8   get_process_priority(UINT8 process_ID);
SINT8   pop(UINT8 priority, struct s_pcb ** catcher);
SINT8   push(UINT8 priority, struct s_pcb * new_back);

#endif
