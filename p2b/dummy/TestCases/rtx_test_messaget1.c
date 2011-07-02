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
		g_test_fixture.send_message(2, tmp);
		g_test_fixture.release_processor();
  }
}

void test2()
{
    UINT8 tmp2;
    VOID * tmp21;	
    while (1) 
    {
		rtx_dbug_out_char('2');
		rtx_dbug_outs("\r\n");
		g_test_fixture.receive_message(&tmp2);
		
		if(tmp2 == 1){
			rtx_dbug_outs("2 Received Message From 1 \r\n");
		        tmp21 = g_test_fixture.request_memory_block();
		        g_test_fixture.send_message(4, tmp21);

		} 		
		//g_test_fixture.release_processor();
		
    }	

		
 }


void test3()
{
  
    UINT8 tmp3;
    while (1) 
    {
		rtx_dbug_out_char('3');
		rtx_dbug_outs("\r\n");
		g_test_fixture.receive_message(&tmp3);
		
		if(tmp3 == 4){
		   rtx_dbug_outs("3 Received Message From 4 \r\n");
		}else{
		   rtx_dbug_outs("ERROR 34 \r\n");
		}
	
		
                g_test_fixture.receive_message(&tmp3);

                if(tmp3 == 5){
                   rtx_dbug_outs("3 Received Message From 5 \r\n");
                }else{
                   rtx_dbug_outs("ERROR 35 \r\n");
                }
	
//		g_test_fixture.release_processor();
    }
}

void test4()
{
    
    VOID * tmp41;
    UINT8 tmp4;
    while (1) 
    {
		rtx_dbug_out_char('4');
		rtx_dbug_outs("\r\n");
		
		g_test_fixture.receive_message(&tmp4);
				
                if(tmp4 == 2){
                   rtx_dbug_outs("4 Received Message From 2 \r\n");
                }else{
                   rtx_dbug_outs("ERROR 42 \r\n");
                }
		
	       tmp41 = g_test_fixture.request_memory_block();
               g_test_fixture.send_message(3, tmp41);

        	
//		g_test_fixture.release_processor();
    }
}

void test5()
{
   
    VOID * tmp51;
    while (1) 
    {
		rtx_dbug_out_char('5');
		rtx_dbug_outs("\r\n");

                tmp51 = g_test_fixture.request_memory_block();
                g_test_fixture.send_message(3, tmp51);


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
