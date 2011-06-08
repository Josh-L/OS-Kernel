#include "memory/rtx_inc.h"
#include "system.h"
#include "memory/dbug.h"

VOID test_proc_1()
{
	while(1)
	{
		rtx_dbug_outs("1\r\n");
		release_processor();
	}
}

VOID test_proc_2()
{
	while(1)
	{
		rtx_dbug_outs((CHAR *)"2\n\r");
		release_processor();
	}
}

VOID test_proc_3()
{
	while(1)
	{
		rtx_dbug_outs((CHAR *)"3\n\r");
		release_processor();
	}
}

VOID test_proc_4()
{
	while(1)
	{
		rtx_dbug_outs((CHAR *)"4\n\r");
		release_processor();
	}
}

VOID test_proc_5()
{
	while(1)
	{
		rtx_dbug_outs((CHAR *)"5\n\r");
		release_processor();
	}
}

VOID test_proc_6()
{
	while(1)
	{
		rtx_dbug_outs((CHAR *)"6\n\r");
		release_processor();
	}
}