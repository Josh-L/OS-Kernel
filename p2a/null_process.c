#include "memory/rtx_inc.h"
#include "system.h"

VOID null_process()
{
	while(1)
	{
		rtx_dbug_outs("n\n\r");
		release_processor();
	}
}