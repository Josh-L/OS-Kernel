/**
 * @author: 
 * @brief: ECE 354 S10 RTX Project P1-(c)
 * @date: 2011/01/07
 */

#DEFINE MEM_POOL_TOP    __end+0x2
#DEFINE MEM_BLK_SIZE    0x40

/* Prototypes */
void init_memory();
void* s_request_memory_block(); 
int s_release_memory_block( void* memory_block );
