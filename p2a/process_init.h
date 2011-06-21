#define NULL_PROC_ID	0
#define TEST_PROC_1_ID	1
#define TEST_PROC_2_ID	2
#define TEST_PROC_3_ID	3
#define TEST_PROC_4_ID	4
#define TEST_PROC_5_ID	5

const struct s_pcb process_init_table [] =
{
	{NULL_PROC_ID, 4, 1, 1024, null_process},
	{TEST_PROC_1_ID, 2, 1, 1024, test_proc_1},
	{TEST_PROC_2_ID, 2, 1, 1024, test_proc_2},
	{TEST_PROC_3_ID, 2, 1, 1024, test_proc_3},
	{TEST_PROC_4_ID, 2, 1, 1024, test_proc_4},
	{TEST_PROC_5_ID, 2, 1, 1024, test_proc_5}
};

#define NUM_PROCESSES sizeof(process_init_table)/sizeof(struct s_pcb)