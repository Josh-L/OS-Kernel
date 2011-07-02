/*--------------------------------------------------------------------------
 *                      RTX Test Suite
 *--------------------------------------------------------------------------
 */
/**
 * @file:   rtx_test_dummy.c
 * @author: Thomas Reidemeister
 * @author: Irene Huang
 * @date:   2010.02.11
 * @brief:  rtx test suite
 */
 
#include "rtx_test.h"
#include "system.h"
#include "dbug.h"
 
volatile struct s_message * msgToSend;
/* third party dummy test process 1 */
void test1()
{
	UINT8 count = 0;
	struct s_message * reqBlock[64];
	while (1)
    {
		++count;
		int i;
		for (i = 0; i < 40; i++) {
			//printInt(i, 0);
			rtx_dbug_outs((CHAR *)"th request\n\r");
			
			reqBlock[i] = g_test_fixture.request_memory_block();
			
			int * msg_data; // will hold request number
			msg_data = (int*)((void*)reqBlock[i] + 64);
			*msg_data = i;		
			
			rtx_dbug_outs((CHAR *)"test1: set message with data: ");
			//printInt(*msg_data, 1);
			
			g_test_fixture.send_message(3, reqBlock[i]);
			
		}
		if (g_test_fixture.set_process_priority(1,3))
			rtx_dbug_outs((CHAR *)"test1: failed to lower test 1 priority\n\r ");
		else 
			rtx_dbug_outs((CHAR *)"test1: passed to lower test 1 priority\n\r ");
        /* execute a rtx primitive to test */
		rtx_dbug_outs((CHAR *)"test1: release processor\n\r");
        g_test_fixture.release_processor();
    }
}
 
/* third party dummy test process 2 */
void test2()
{
	UINT8 count = 0;
    rtx_dbug_outs((CHAR *)"rtx_test: test2\r\n");
    struct s_message * reqBlock[64];
	while (1)
    {
		++count;
		
		
		if (count == 10){
			count = 0;
			// should run test 2 10 times then block here
			reqBlock[0] = g_test_fixture.request_memory_block();
		}
        /* execute a rtx primitive to test */
		rtx_dbug_outs((CHAR *)"test2: release processor\n\r");
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 3 */
void test3()
{
	UINT8 count;
    rtx_dbug_outs((CHAR *)"rtx_test: test3\r\n");
    while (1)
    {
		count++;
		struct s_message * received;
		int senderId;
		int * msgData;
		received = g_test_fixture.receive_message(&senderId);
		msgData = (int*)((void*)received + 64);
		// rtx_dbug_outs((CHAR *) "Received Message Type in test3: ");
		// printInt(received->message_type, 1);
	
		rtx_dbug_outs((CHAR *)"test3: got message with data: ");
		//printInt(*msgData, 1);
		if (g_test_fixture.release_memory_block(received)){
			rtx_dbug_outs((CHAR *)"test3: failed release mem ");
		}
        /* execute a rtx primitive to test */
		rtx_dbug_outs((CHAR *)"test3: release processor\n\r");
        g_test_fixture.release_processor();
    }
}
 
/* third party dummy test process 4 */
void test4()
{
	rtx_dbug_outs((CHAR *)"rtx_test: test4\r\n");
    while (1)
    {
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 5 */
void test5()
{
	rtx_dbug_outs((CHAR *)"rtx_test: test5\r\n");
	while (1)
    {
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
	}
}
/* third party dummy test process 6 */
void test6()
{
	rtx_dbug_outs((CHAR *)"rtx_test: test6\r\n");
    while (1)
    {
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
	}
}
 
/* register the third party test processes with RTX */
void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;
 
    rtx_dbug_outs((CHAR *)"rtx_test: register_test_proc()\r\n");
 
	for (i =0; i< NUM_TEST_PROCS; i++ ) {
        g_test_proc[i].pid = i + 1;
        g_test_proc[i].priority = 2;
		if (i < 2)
			g_test_proc[i].priority = 1;
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
    rtx_dbug_outs((CHAR *)"rtx_test: started\r\n");
    return 0;
}