
# Makefile
# David Grant, 2004
# Irene Huang, 2011/01/07

CC=m68k-elf-gcc
CXX=m68k-elf-g++
CFLAGS= -Wall -m5307 -pipe -nostdlib
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDUMP=m68k-elf-objdump
ASM=./start.s scheduler_trap.s proc_priority_trap.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map

OBJS=dbug.o Hex_to_ASCII.o null_process.o dummy_processes.o system.o main.o

# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.
main.s19: scheduler_trap.s proc_priority_trap.s $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o main.bin $(ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec main.bin main.s19
	$(OBJDUMP) -xdC main.bin > main.lst

system.o: system.h system.c
	$(CC) $(CFLAGS) -c system.c
	
dummy_processes.o: dummy_processes.c dummy_processes.h
	$(CC) $(CFLAGS) -c dummy_processes.c dummy_processes.h

null_process.o: null_process.c null_process.h
	$(CC) $(CFLAGS) -c null_process.c

Hex_to_ASCII.o: Hex_to_ASCII.c Hex_to_ASCII.h
	$(CC) $(CFLAGS) -c Hex_to_ASCII.c

dbug.o: dbug.c dbug.h
	$(CC) $(CFLAGS) -c dbug.c

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.map
