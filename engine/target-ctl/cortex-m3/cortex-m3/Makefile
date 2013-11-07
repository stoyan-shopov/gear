
CC = gcc
CFLAGS = -c -g $(INCDIRS)
INCDIRS = -I../../include/ -I../../../include/
TARGET = cortex-m3

$(TARGET).o: $(TARGET).c jtag/jtag.h jtag/jtagdrv.o $(TARGET)_t.o
	ld -r $(TARGET)_t.o jtag/jtagdrv.o -o $@

$(TARGET)_t.o: $(TARGET).c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm	$(TARGET).o $(TARGET)_t.o jtag/jtagdrv.o

