/**
 * @author: 
 * @brief: ECE 354 S11 RTX Project P1-(c) 
 * @date: 2011/05/02
 */

#include "rtx_inc.h"
#include "memory.h"

/**
 * @brief memory management initialization routine
 */
void init_memory()
{
    /* Add your own code here */
}

/**
 * @brief request a free memory block of size 128 Bytes
 * @return starting address of a free memory block
 *         and NULL on error 
 */

void* s_request_memory_block()
{
    /* Add your own code here */

    /* replace NULL with a non-zero memory address */
    return NULL; 
}

/**
 * @param: address of a memory block to free
 * @return: 0 on sucess, non-zero on error
 */

int s_release_memory_block( void* memory_block )
{
    /* Add your own code here */


    /* add code to return non-zero if releasing memory block fails */

    return 0; // 0 on success
}
