
CC = gcc-4.3.2
CC = elfarm-gcc

CFLAGS = -nostdlib -g -O2 -finline-functions
CFLAGS = -nostdlib -g

test.elf: test_0.c
	$(CC) $(CFLAGS) -o $@ test_0.c -T mmap.ld

