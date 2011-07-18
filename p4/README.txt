File breakdown:

	*.s: Various TRAP routines and ISRs
	main_rtx.c: Contains all of the process initialization code.
	system.c: Contains all core kernel functions
	i_proc.c: Contains all i-processes and system processes
	rtx_tezt.c: Contains test processes 1 to 6
	rtx_stress_test: Contains processes A, B, and C for stress testing
	rtx_test_dummy.c: This file is no longer used in the make, but can be swapped for rtx_tezt.c

Makefile is configured for cf-server and has flags set to enable hotkeys(_DEBUG_HOTKEYS) and to enable test processes to run tests (_DEBUG).
Run make to compile everything together, and use mdummy.s19 to execute.