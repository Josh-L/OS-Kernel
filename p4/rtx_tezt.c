#include "rtx_test.h"
#include "dbug.h"

//g_test_fixture.
//g_test_fixture.release_processor();
//g_test_fixture.get_process_priority(0);
//g_test_fixture.set_process_priority(0, 0);
//g_test_fixture.request_memory_block();
//g_test_fixture.release_memory_block((VOID *)tmp);
//g_test_fixture.receive_message(&i);
//g_test_fixture.delayed_send(2, (VOID *)tmp, 2);
//g_test_fixture.send_message(2, (VOID *)tmp);

int test_results;

void test1()
{	
    int prio;
    g_test_fixture.set_process_priority(1, 1);
    g_test_fixture.set_process_priority(2, 2);
    prio = g_test_fixture.get_process_priority(2);
    if(prio == 2)
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: test 1 OK\n\r");
        #endif
        test_results++;
    }
    else
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: test 1 FAIL\n\r");
        #endif
    }
    
    g_test_fixture.set_process_priority(1, 3);
    g_test_fixture.set_process_priority(2, 3);
    
    while(1) 
    {
		g_test_fixture.release_processor();
	}
}

void test2()
{
    int i;
    int * msg_type;
    *msg_type = -1;
	VOID * msg = g_test_fixture.receive_message(&i);
    msg_type = msg + 8;
    
    if(*msg_type == 99)
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: test 2 OK\n\r");
        #endif
        test_results++;
    }
    else
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: test 2 FAIL\n\r");
        #endif
    }
    
    if(test_results == 0)
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: 0/2 tests OK\n\r");
        rtx_dbug_outs((CHAR *)"S201G012_test: 2/2 tests FAIL\n\r");
        rtx_dbug_outs((CHAR *)"S201G012_test: END\n\r");
        #endif
    }
    else if(test_results == 1)
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: 1/2 tests OK\n\r");
        rtx_dbug_outs((CHAR *)"S201G012_test: 1/2 tests FAIL\n\r");
        rtx_dbug_outs((CHAR *)"S201G012_test: END\n\r");
        #endif
    }
    else
    {
        #ifdef _DEBUG
        rtx_dbug_outs((CHAR *)"S201G012_test: 2/2 tests OK\n\r");
        rtx_dbug_outs((CHAR *)"S201G012_test: 0/2 tests FAIL\n\r");
        rtx_dbug_outs((CHAR *)"S201G012_test: END\n\r");
        #endif
    }
    
    while(1)
    {
		g_test_fixture.release_processor();
	}	
 }


void test3()
{
    int * msg_type;
    VOID * msg = g_test_fixture.request_memory_block();
    msg_type = (int *)(msg + 8);
    *msg_type = 99;
    g_test_fixture.send_message(2, msg);
    while(1) 
    {
		g_test_fixture.release_processor();
    }
}

void test4()
{
    while(1)
    {
		g_test_fixture.release_processor();
    }
}

void test5()
{
    while(1) 
    {
		g_test_fixture.release_processor();
    }
}

void test6()
{
    while(1)
    {
        g_test_fixture.release_processor();
    }
}

/* register the third party test processes with RTX */
void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;
    test_results = 0;
    #ifdef _DEBUG
    rtx_dbug_outs((CHAR *)"rtx_test: register_test_proc()\r\n");
    rtx_dbug_outs((CHAR *)"S201G012_test: START\n\r");
    rtx_dbug_outs((CHAR *)"S201G012_test: total 2 tests\n\r");
    #endif

    for (i = 0; i< NUM_TEST_PROCS; i++ ) {
        g_test_proc[i].pid = i + 1;
		g_test_proc[i].priority = 3;
        g_test_proc[i].sz_stack = 2048;
    }
    g_test_proc[0].entry = test1;
    g_test_proc[1].entry = test2;
    g_test_proc[2].entry = test3;
    g_test_proc[3].entry = test4;
    g_test_proc[4].entry = test5;
    g_test_proc[5].entry = test6;
}

/**
 * Main entry point for this program.
 * never get invoked
 */
int main(void)
{
    #ifdef _DEBUG
    rtx_dbug_outs((CHAR *)"rtx_test: started\r\n");
    #endif
    return 0;
}