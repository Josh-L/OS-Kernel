/**
 * @author: 
 * @brief: ECE 354 S11 RTX Project P1-(c) 
 * @date: 2011/05/02
 */

#include "rtx_inc.h"
#include "memory.h"

UINT32 gp_mem_pool_stack[NUM_MEM_BLKS];
UINT8 g_mem_pool_stack_index;

/**
 * @brief memory management initialization routine
 */
void init_memory()
{
    for (int i = 0; i < NUM_MEM_BLKS; i++)
    {
        gp_mem_pool_stack[i] = MEM_POOL_TOP + (i*MEM_BLK_SIZE);
    }
    g_mem_pool_stack_index = 0;
}

/**
 * @brief request a free memory block of size 128 Bytes
 * @return starting address of a free memory block
 *         and NULL on error 
 */

void* s_request_memory_block()
{
    if((g_mem_pool_stack_index > NUM_MEM_BLKS - 1) && (g_mem_pool_stack_index < 0))
    {
        return NULL;
    }
    else
    {
        return gp_mem_pool_stack[g_mem_pool_stack_index++];
    }
    
}

/**
 * @param: address of a memory block to free
 * @return: 0 on sucess, non-zero on error
 */

int s_release_memory_block( void* memory_block )
{
    if((g_mem_pool_stack_index > NUM_MEM_BLKS - 1) && (g_mem_pool_stack_index < 1))
    {
        return -1;
    }
    else
    {
        --g_mem_pool_stack_index;
        return 0;
    }
}
