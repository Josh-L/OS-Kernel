/*
* @author:
* @brief: ECE 354 S11 RTX Project P1-(c)
* @date: 2011/05/02
*/

#include "rtx_inc.h"
#include "memory.h"

BOOLEAN gp_mem_pool_lookup[NUM_MEM_BLKS]; // Stores whether or not the corresponding 
UINT32 gp_mem_pool_list[NUM_MEM_BLKS]; // List of addresses of memory blocks

/**
* @brief memory management initialization routine
*/
void init_memory()
{
    UINT8 i;
    for (i = 0; i < NUM_MEM_BLKS; i++)
    {
		gp_mem_pool_lookup[i] = 0;
        gp_mem_pool_list[i] = MEM_POOL_TOP + (i*MEM_BLK_SIZE);
    }
}

/**
* @brief request a free memory block of size 128 Bytes
* @return starting address of a free memory block
* and NULL on error
*/

void* s_request_memory_block()
{
    UINT8 i;
    for (i = 0; i < NUM_MEM_BLKS; i++)
	{
		if (gp_mem_pool_lookup[i] == 0)
		{
			gp_mem_pool_lookup[i] = 1;
			return (VOID*)gp_mem_pool_list[i];
		}
	}
    return NULL;
}

/**
* @param: address of a memory block to free
* @return: 0 on sucess, non-zero on error
*/

int s_release_memory_block( void* memory_block )
{
    UINT8 i;
    for (i = 0; i < NUM_MEM_BLKS; i++)
	{
		if (gp_mem_pool_list[i] == (UINT32)memory_block && gp_mem_pool_lookup[i] == 1)
		{
			gp_mem_pool_lookup[i] = 0;
			return RTX_SUCCESS;
		}
	}
	return RTX_ERROR;
}
