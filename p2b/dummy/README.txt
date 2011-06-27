Group # = G012
Directory contents: system - Contains system.c, system.h and trap handler *.s files part of the provided test hardness code. 
			
					
Build instructions: cd into memory directory and type "make clean" followed by "make". Upload mdummy.s19 file to Coldfire board to run.
		    press go 10200000 to run.

To test:			edit the rtx_test_dummy.c and add your dummy processes to the file
				edit system.h and change the NUM_PROCESSES macro to whatever number of processes you're testing