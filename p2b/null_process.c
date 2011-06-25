#include "system.h"

VOID null_process()
{
	while(1)
	{
		rtx_dbug_outs("null\n\r");
		release_processor();
	}
}