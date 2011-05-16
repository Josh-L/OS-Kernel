
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
ASM=./start.s ./timer_entry.s ./timer_output_char.s
LDFLAGS = -Trtx.ld -Wl,-Map=timer0.map

OBJS=dbug.o timer0.o

# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.
timer0.s19: $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o timer0.bin $(ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec timer0.bin timer0.s19


dbug.o: dbug.c dbug.h
	$(CC) $(CFLAGS) -c dbug.c

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.map
