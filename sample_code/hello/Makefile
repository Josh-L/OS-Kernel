# Makefile
# David Grant, 2004
# Irene Huang, 2011/01/04 

CC=m68k-elf-gcc
CXX=m68k-elf-g++
CFLAGS=-Wall -m5307 -pipe -nostdlib
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDMP=m68k-elf-objdump
ASM=../shared/start.s
LDFLAGS=-T../shared/rtx.ld -Wl,-Map=hello.map

HELLO_OBJS=../dbug/dbug.o hello.o

# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.
hello.s19: $(HELLO_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o hello.bin $(ASM) $(HELLO_OBJS) 
	$(OBJCPY) --output-format=srec hello.bin hello.s19
	$(OBJDMP) -xdC hello.bin > hello.lst 
	chmod u+x hello.s19 

hello.o: hello.c
	$(CC) $(CFLAGS) -c hello.c 

../dbug/dbug.o: ../dbug/dbug.c ../dbug/dbug.h
	make -C ../dbug 

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.lst *.map ../dbug/dbug.o
