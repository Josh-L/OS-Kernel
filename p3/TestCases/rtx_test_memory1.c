#include "rtx_test.h"
#include "dbug.h"
#include "Hex_to_ASCII.h"

//g_test_fixture.
//g_test_fixture.release_processor();
//g_test_fixture.get_process_priority(0)
//g_test_fixture.set_process_priority(0, 0)
//(UINT32) g_test_fixture.request_memory_block();
//g_test_fixture.release_memory_block((VOID *)tmp);



void test1()
{ 
    VOID * tmp;
    while (1) 
    {
		rtx_dbug_out_char('1');
		rtx_dbug_outs("\r\n");
		UINT8 i;
		tmp = g_test_fixture.request_memory_block();
		rtx_dbug_outs("P1 Got Memory");
		g_test_fixture.release_processor();
		rtx_dbug_outs("P1 Free's Memory");
		g_test_fixture.release_memory_block((VOID *)tmp);
		g_test_fixture.release_processor();
  }
}

void test2()
{
    VOID * tmp;
    while (1) 
    {
		rtx_dbug_out_char('2');
		rtx_dbug_outs("\r\n");
		
		UINT8 i;
                for(i = 0; i < 100; i ++){
                        tmp = g_test_fixture.request_memory_block();
                        rtx_dbug_outs("P2 Got Memory \r\n");
                }
    }	

		
 }


void test3()
{
  
    while (1) 
    {
		rtx_dbug_out_char('3');
		rtx_dbug_outs("\r\n");
		g_test_fixture.release_processor();
	}
}

void test4()
{
    
    while (1) 
    {
		rtx_dbug_out_char('4');
		rtx_dbug_outs("\r\n");
        	g_test_fixture.release_processor();
    }
}

void test5()
{
   
    while (1) 
    {
		rtx_dbug_out_char('5');
		rtx_dbug_outs("\r\n");
        	g_test_fixture.release_processor();
    }
}

void test6()
{
    
    while (1) 
    {
		rtx_dbug_out_char('6');
		rtx_dbug_outs("\r\n");
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
    rtx_dbug_outs((CHAR *)"rtx_test: started\r\n");
    return 0;
}
