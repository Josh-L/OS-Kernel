#include "memory/rtx_inc.h"
#include "system.h"

VOID null_process()
{
	while(1)
	{
		rtx_dbug_outs((CHAR *)"null process\n\r");
		release_processor();
	}
}