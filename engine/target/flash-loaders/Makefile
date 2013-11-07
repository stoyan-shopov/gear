
CC = gcc
INCDIRS = -I../../include -I../../include/target/arm/ -I../../
CFLAGS = -g $(INCDIRS)

target-img-load.o: target-img-load.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm target-img-load.o

