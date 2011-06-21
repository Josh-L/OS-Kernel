Group # = G012
Directory contents: system - Contains system.c, system.h, and all relevant files to build main.s19
					memory - Contains memory.c, memory.h, and all relevant files to build main.s19
					
Build instructions: cd into memory directory and type "make clean" followed by "make". Upload main.s19 file to Coldfire board to run.
To test:			edit the dummy_processes.c and add your dummy processes to the file
					edit system.h and change the NUM_PROCESSES macro to whatever number of processes you're testing