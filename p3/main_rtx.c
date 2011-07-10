/*----------------------------------------------------------------------------
 *              A Dummy RTX for Testing 
 *----------------------------------------------------------------------------
 */
/**
 * @file: main_rtx.c
 * @author: Thomas Reidemeister 
 * @author: Irene Huang 
 * @date: 2010.02.18
 * @brief: Example dummy rtx for testing
 */

#include "system.h"
#include "rtx_test.h"
#include "dbug.h"

/* test proc initializaiton info. registration function provided by test suite.
 * test suite needs to register its test proc initilization info with rtx
 * The __REGISTER_TEST_PROCS_ENTRY__ symbol is in linker scripts
 */

struct s_pcb        		g_proc_table[NUM_PROCESSES];
UINT32						g_free_mem = &__end;
 
extern void __REGISTER_TEST_PROCS_ENTRY__();

/* gcc expects this function to exist */
int __main( void )
{
    return 0;
}

int main() 
{

    rtx_dbug_outs((CHAR *)"rtx: Entering main()\r\n");

    /* get the third party test proc initialization info */
    __REGISTER_TEST_PROCS_ENTRY__();
	
	UINT8 i = 0;
	
	// Setup all processes
	for(i = 0; i < NUM_TEST_PROCS; i++)
	{
		g_proc_table[i].m_process_ID = g_test_proc[i].pid;
		g_proc_table[i].m_priority = g_test_proc[i].priority;
		g_proc_table[i].m_state = 1;
		g_proc_table[i].m_entry = g_test_proc[i].entry;
		g_proc_table[i].m_stack = g_free_mem = g_free_mem + g_test_proc[i].sz_stack;
		g_proc_table[i].i_process = 0;
	}
	
	// Adding Null Proc to g_proc_table
	g_proc_table[i].m_process_ID = 0;
	g_proc_table[i].m_priority = 4;
	g_proc_table[i].m_state = 1;
	g_proc_table[i].m_entry = null_process;
	g_proc_table[i].m_stack = g_free_mem = g_free_mem + 1024;
	g_proc_table[i].i_process = 0;

	// Keyboard decoder process
	g_proc_table[i+1].m_process_ID = 7;
	g_proc_table[i+1].m_priority = 0;
	g_proc_table[i+1].m_state = 1;
	g_proc_table[i+1].m_entry = kdc;
	g_proc_table[i+1].m_stack = g_free_mem = g_free_mem + 1024;
	g_proc_table[i+1].i_process = 0;
	
	// Print to screen process
	g_proc_table[i+2].m_process_ID = 8;
	g_proc_table[i+2].m_priority = 0;
	g_proc_table[i+2].m_state = 1;
	g_proc_table[i+2].m_entry = crt;
	g_proc_table[i+2].m_stack = g_free_mem = g_free_mem + 1024;
	g_proc_table[i+2].i_process = 0;
	
	// UART ISR i process
	g_proc_table[i+3].m_process_ID = 9;
	g_proc_table[i+3].m_priority = 4;
	g_proc_table[i+3].m_state = 1;
	g_proc_table[i+3].m_entry = uart;
	g_proc_table[i+3].m_stack = g_free_mem = g_free_mem + 1024;
	g_proc_table[i+3].i_process = 1;
	
	// Timer ISR i process
	g_proc_table[i+4].m_process_ID = 10;
	g_proc_table[i+4].m_priority = 4;
	g_proc_table[i+4].m_state = 1;
	g_proc_table[i+4].m_entry = timer;
	g_proc_table[i+4].m_stack = g_free_mem = g_free_mem + 1024;
	g_proc_table[i+4].i_process = 1;

	sys_init();

    return 0;
}


/* register rtx primitives with test suite */

void  __attribute__ ((section ("__REGISTER_RTX__"))) register_rtx() 
{
    rtx_dbug_outs((CHAR *)"rtx: Entering register_rtx()\r\n");
    g_test_fixture.send_message = send_message;
    g_test_fixture.receive_message = receive_message;
    g_test_fixture.request_memory_block = request_memory_block;
    g_test_fixture.release_memory_block = release_memory_block;
    g_test_fixture.release_processor = release_processor;
    g_test_fixture.delayed_send = delayed_send;
    g_test_fixture.set_process_priority = set_process_priority;
    g_test_fixture.get_process_priority = get_process_priority;
    rtx_dbug_outs((CHAR *)"rtx: leaving register_rtx()\r\n");
}
