# Makefile Winter 2011 
# by
# David Grant
# Thomas Reidemeister
# Irene Huang

#-D_CFSERVER_

CC=m68k-elf-gcc
CXX=m68k-elf-g++
CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG -D_DEBUG_HOTKEYS -D_CFSERVER_
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDUMP=m68k-elf-objdump
ASM=./start.s
LDFLAGS_RTX = -Trtx.ld -Wl,-Map=rtx.map
LDFLAGS_RTX_LOADER = -Trtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_RTX_TEST = -Trtx_test.ld -Wl,-Map=rtx_test.map

RTX_OBJS = system.o dbug.o main_rtx.o i_proc.o Hex_to_ASCII.o rtx_test_stress.o
RTX_LOADER_OBJS = dbug.o rtx_loader.o
RTX_TEST_DUMMY_OBJS = dbug.o rtx_tezt.o

all: mdummy.s19 


# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.

system.s19: $(RTX_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS_RTX) -o system.bin $(ASM) serial_entry.s release_proc_trap.s set_proc_priority_trap.s get_proc_priority_trap.s request_mem_block_trap.s release_mem_block_trap.s send_msg_trap.s receive_msg_trap.s timer_entry.s delayed_send_trap.s $(RTX_OBJS) 
	$(OBJCPY) --output-format=srec system.bin system.s19
	$(OBJDUMP) -xdC system.bin > system.lst

rtx_loader.s19: $(RTX_LOADER_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_LOADER) -o rtx_loader.bin $(ASM) $(RTX_LOADER_OBJS) 
	$(OBJCPY) --output-format=srec rtx_loader.bin rtx_loader.s19
	$(OBJDUMP) -xdC rtx_loader.bin > rtx_loader.lst

rtx_test_dummy.s19: $(RTX_TEST_DUMMY_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o rtx_test_dummy.bin $(ASM) $(RTX_TEST_DUMMY_OBJS) 
	$(OBJCPY) --output-format=srec rtx_test_dummy.bin rtx_test_dummy.s19
	$(OBJDUMP) -xdC rtx_test_dummy.bin > rtx_test_dummy.lst

m2.s19: rtx_test_dummy.s19 system.s19
	python merge.py m2.s19 system.s19 rtx_test_dummy.s19 
	chmod u+x m2.s19

mdummy.s19: m2.s19 rtx_loader.s19
	python merge.py mdummy.s19 m2.s19 rtx_loader.s19 
	chmod u+x mdummy.s19
    
.c.o:
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm -f *.bin *.s19 *.o *.map *.lst
