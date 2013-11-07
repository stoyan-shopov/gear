
CC = gcc
LD = ld
CFLAGS = -c -g $(INCDIRS)
TARGET = arm7tdmis
INCDIRS = -I../include/ -I../../include/


OBJECTS = target-ctl.o $(TARGET)/$(TARGET).o


target-ctl: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) -lftdi -lusb

target-ctl.o: target-ctl-parse.tab.c lex.yy.c
	$(CC) -o $@ $< $(CFLAGS)

target-ctl-parse.tab.c: target-ctl-parse.y
	bison -d $<

lex.yy.c: target-ctl-lex.y target-ctl-parse.tab.h
	flex $<

$(TARGET)/$(TARGET).o:	
	$(MAKE) -C $(TARGET)

clean:
	-rm $(OBJECTS)
	-rm target-ctl-parse.tab.h target-ctl-parse.tab.c lex.yy.c
	-rm target-ctl.exe target-ctl
	$(MAKE) -i -C $(TARGET) clean

