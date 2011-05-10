/**
 * @author: 
 * @brief: ECE 354 S11 RTX Project P1-(c) 
 * @date: 2011/05/02
 * This file runs simiple tests of memory allocation/de-allocation APIs.
 * During the demo, we may replace this file with one  
 * that contains more testing cases. 
 */

/*
 * DO NOT CHANGE THIS FILE!
 */

#include "rtx_inc.h"
#include "dbug.h"
#include "memory.h"

#define NUM_MEM_BLKS 32

/* gcc expects this function to exist */
int __main( VOID )
{
    return 0;
}

int main( VOID )
{
    int i;
    void* p_mem_array[NUM_MEM_BLKS];

    init_memory();

    for ( i=0; i< NUM_MEM_BLKS; i++ ) 
    {
        p_mem_array[i] = s_request_memory_block();
        if (p_mem_array[i] == NULL) {
            rtx_dbug_outs((CHAR *) "Null pointer.\r\n");
        } else if (p_mem_array[i] > 0x10200000) {
            rtx_dbug_outs((CHAR *) "Memory out of bound. \r\n");
        } else {
            rtx_dbug_outs((CHAR *) "Request meory block: almost OK\r\n");
        }
    }

    for ( i=0; i< NUM_MEM_BLKS; i++ ) 
    {
        int temp;
        temp = s_release_memory_block( p_mem_array[i] );
        if (temp == 0 ) {
            rtx_dbug_outs((CHAR *) "Release memory block: OK\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "Release memory block: Failed \r\n");
        }
    }

    return 0;
}
