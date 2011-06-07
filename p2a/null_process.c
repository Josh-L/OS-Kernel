#include "memory/rtx_inc.h"
#include "system.h"

VOID null_process()
{
	rtx_dbug_outs((CHAR *)"null_process\n");
	while(1)
	{
		release_processor();
	}
}