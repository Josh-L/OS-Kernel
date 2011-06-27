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
	//UINT32 * tmp = 2;
	//UINT8 tmp  = 2;
    while (1) 
    {
		rtx_dbug_out_char('1');
		rtx_dbug_outs("\r\n");
		int * sender_id;
		void *received;
		void *tmp =  g_test_fixture.request_memory_block();
		int *data = (int*)(tmp+100);
		*data = 8;
		g_test_fixture.send_message(2, tmp);
		received = g_test_fixture.receive_message(sender_id);
		// Build message
/*		tmp = (UINT32) g_test_fixture.request_memory_block();
		*(tmp + 2) = 0x00000009;
		*(tmp + 3) = 0xFFFFFFFF;
		
		g_test_fixture.send_message(2, tmp);
		g_test_fixture.receive_message(&tmp);
*/
	g_test_fixture.release_processor();  

  }
}

void test2()
{
 //   UINT8 tmp = 1;
    while (1) 
    {
//		g_test_fixture.receive_message(&tmp);
//		tmp = (UINT32) g_test_fixture.request_memory_block();
//		g_test_fixture.send_message(1, tmp);
		rtx_dbug_out_char('2');
		rtx_dbug_outs("\r\n");
		int * sender_id;
		void *received = g_test_fixture.receive_message(sender_id);
		int *data = (int*)(received+100);
		rtx_dbug_outs("Test2 received ");
		rtx_dbug_out_char('0'+*data);
		rtx_dbug_outs(" from ");
		rtx_dbug_out_char('0'+ *sender_id);
		rtx_dbug_outs("\r\n");
		void *tmp =  g_test_fixture.request_memory_block();
		data = (int*)(tmp+100);
		*data = 7;
		g_test_fixture.send_message(3, tmp);
		rtx_dbug_outs("Test2 sent 7 to 3");
		g_test_fixture.release_processor();
    }
}

void test3()
{
    UINT32 tmp = 0;
    while (1) 
    {
		rtx_dbug_out_char('3');
		rtx_dbug_outs("\r\n");
		int sender_id;
		void *received = g_test_fixture.receive_message(&sender_id);
		int *data = (int*)(received+100);
		rtx_dbug_outs("Test3 received ");
		rtx_dbug_out_char('0'+*data);
		rtx_dbug_outs(" from ");
		rtx_dbug_out_char('0'+sender_id);
		rtx_dbug_outs("\r\n");
		void *tmp =  g_test_fixture.request_memory_block();
		data = (int*)(tmp+100);
		*data = 7;
		g_test_fixture.send_message(2, tmp);
		rtx_dbug_outs("Test3 sent 9 to 2");
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
