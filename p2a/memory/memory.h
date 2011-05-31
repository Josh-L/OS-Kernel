/**
* @author:
* @brief: ECE 354 S10 RTX Project P1-(c)
* @date: 2011/01/07
*/

extern BYTE __end;
#define MEM_POOL_TOP &__end
#define MEM_BLK_SIZE 128
#define NUM_MEM_BLKS 32

/* Prototypes */
void init_memory();
void* s_request_memory_block();
int s_release_memory_block( void* memory_block );
