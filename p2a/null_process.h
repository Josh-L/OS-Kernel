#include "memory/rtx_inc.h"

VOID null_process()
{
	while(1)
	{
		release_processor();
	}
}