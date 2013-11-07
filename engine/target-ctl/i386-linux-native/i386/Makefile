
CC = gcc
CFLAGS = -c -g $(INCDIRS)
INCDIRS = -I../../include/ -I../../../include/ -I../../../
TARGET = i386

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm	$(TARGET).o

