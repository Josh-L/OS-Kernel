#include "process_init.h"

const struct s_pcb_init process_init_table [] =
{
	{NULL_PROC_ID, 4, 1, 1024, null_process},
	{TEST_PROC_1_ID, 3, 1, 1024, test_proc_1},
	{TEST_PROC_2_ID, 3, 1, 1024, test_proc_2},
	{TEST_PROC_3_ID, 3, 1, 1024, test_proc_3},
	{TEST_PROC_4_ID, 3, 1, 1024, test_proc_4},
	{TEST_PROC_5_ID, 3, 1, 1024, test_proc_5}
};