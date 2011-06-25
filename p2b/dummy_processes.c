#include "rtx_inc.h"
#include "system.h"
#include "dbug.h"

VOID test_proc_1()
{
	UINT32 tmp = 0;
	while(1)
	{
		rtx_dbug_out_char('1');
		tmp = (UINT32)request_memory_block();
		tmp = (UINT32)request_memory_block();
		set_process_priority(3,0);
		set_process_priority(2,1);
		set_process_priority(1,2);
		release_processor();
		release_memory_block((VOID *)tmp);
	}
}

VOID test_proc_2()
{
	UINT32 tmp = 0;
	while(1)
	{
		rtx_dbug_out_char('2');
		tmp = (UINT32)request_memory_block();
	}
}

VOID test_proc_3()
{
	UINT32 tmp = 0;
	while(1)
	{
		rtx_dbug_out_char('3');
		tmp = (UINT32)request_memory_block();
	}
}

VOID test_proc_4()
{
	while(1)
	{
		rtx_dbug_out_char('4');
		release_processor();
	}
}

VOID test_proc_5()
{
	while(1)
	{
		rtx_dbug_out_char('5');
		release_processor();
	}
}

VOID test_proc_6()
{
	while(1)
	{
		rtx_dbug_out_char('6');
		release_processor();
	}
}