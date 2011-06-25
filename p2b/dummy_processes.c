#include "rtx_inc.h"
#include "system.h"
#include "dbug.h"

VOID test_proc_1()
{
	asm("proc1:");
	while(1)
	{
		//rtx_dbug_outs("a\r\n");
		rtx_dbug_out_char('1');
		release_processor();
		set_process_priority(2,1);
	}
}

VOID test_proc_2()
{
	while(1)
	{
		//rtx_dbug_outs((CHAR *)"2\n\r");
		rtx_dbug_out_char('2');
		release_processor();
	}
}

VOID test_proc_3()
{
	while(1)
	{
		//rtx_dbug_outs((CHAR *)"3\n\r");
		rtx_dbug_out_char('3');
		release_processor();
	}
}

VOID test_proc_4()
{
	while(1)
	{
		//rtx_dbug_outs((CHAR *)"4\n\r");
		rtx_dbug_out_char('4');
		release_processor();
	}
}

VOID test_proc_5()
{
	while(1)
	{
		//rtx_dbug_outs((CHAR *)"5\n\r");
		rtx_dbug_out_char('5');
		release_processor();
	}
}

VOID test_proc_6()
{
	while(1)
	{
		//rtx_dbug_outs((CHAR *)"6\n\r");
		rtx_dbug_out_char('6');
		release_processor();
	}
}