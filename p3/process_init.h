#include "rtx_test_dummy.c"

#define NULL_PROC_ID	0
#define TEST_PROC_1_ID	1
#define TEST_PROC_2_ID	2
#define TEST_PROC_3_ID	3
#define TEST_PROC_4_ID	4
#define TEST_PROC_5_ID	5

const struct s_pcb process_init_table [] =
{
	{NULL_PROC_ID, 4, 1, 2048, null_process},
	{TEST_PROC_1_ID, 2, 1, 2048, test1},
	{TEST_PROC_2_ID, 2, 1, 2048, test2},
	{TEST_PROC_3_ID, 2, 1, 2048, test3},
	{TEST_PROC_4_ID, 2, 1, 2048, test4},
	{TEST_PROC_5_ID, 2, 1, 2048, test5},
    {7, 0, 1, 2048, kdc}
};

#define NUM_PROCESSES sizeof(process_init_table)/sizeof(struct s_pcb)