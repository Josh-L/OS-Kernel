#include "memory/rtx_inc.h"
#include "system.h"
#include "memory/dbug.h"

VOID test_proc_1()
{
	rtx_dbug_outs((CHAR *)"1\n\r");
	while(1)
	{
		release_processor();
	}
}

VOID test_proc_2()
{
	rtx_dbug_outs((CHAR *)"2\n\r");
	while(1)
	{
		release_processor();
	}
}

VOID test_proc_3()
{
	rtx_dbug_outs((CHAR *)"3\n\r");
	while(1)
	{
		release_processor();
	}
}

VOID test_proc_4()
{
	rtx_dbug_outs((CHAR *)"4\n\r");
	while(1)
	{
		release_processor();
	}
}

VOID test_proc_5()
{
	rtx_dbug_outs((CHAR *)"5\n\r");
	while(1)
	{
		release_processor();
	}
}

VOID test_proc_6()
{
	rtx_dbug_outs((CHAR *)"6\n\r");
	while(1)
	{
		release_processor();
	}
}