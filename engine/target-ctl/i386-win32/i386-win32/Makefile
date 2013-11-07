
CC = gcc
CFLAGS = -c -g $(INCDIRS)
INCDIRS = -I../../include/ -I../../../include/ -I../../../
TARGET = i386-win32

all: $(TARGET).o getfname-from-handle.o

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm	$(TARGET).o

