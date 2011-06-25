#ifndef __PROC_INIT_TABLE__
#define __PROC_INIT_TABLE__

#include "dummy_processes.h"
#include "null_process.h"

#define NULL_PROC_ID	0
#define TEST_PROC_1_ID	1
#define TEST_PROC_2_ID	2
#define TEST_PROC_3_ID	3
#define TEST_PROC_4_ID	4
#define TEST_PROC_5_ID	5

#define NUM_PROCESSES 6//sizeof(process_init_table)/sizeof(struct s_pcb_init)

struct s_pcb_init
{
    UINT32  m_process_ID;
    UINT8   m_priority;
    UINT8   m_state;       // 0 blocked, 1 ready, 2 running
    UINT32  m_stack;
    VOID    (*m_entry)();
};

extern const struct s_pcb_init process_init_table [];

#endif