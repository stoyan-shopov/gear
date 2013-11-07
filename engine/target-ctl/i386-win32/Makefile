
CC = gcc
CFLAGS = -c -g $(INCDIRS)
LDFLAGS = -lpthreadGC2d -lws2_32 -L c:/mingw32-gcc-4.5.2/bin/ -lpsapi
TARGET = i386-win32
INCDIRS = -I../include/ -I../../include/ -I../../


OBJECTS = target-ctl.o $(TARGET)/$(TARGET).o $(TARGET)/getfname-from-handle.o


target-ctl: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

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

