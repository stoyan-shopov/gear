#CROSS_COMPILE_PREFIX = i386-mingw32-

CC = $(CROSS_COMPILE_PREFIX)gcc
LD = $(CROSS_COMPILE_PREFIX)ld
INCDIRS = -I../ -I../armemu -I../include/ -I../include/target/arm/
CFLAGS = -c -g $(INCDIRS) -shared
OBJECTS = c-parse.o

c-exp.dll: $(OBJECTS) ../ss.dll
	$(CC) $(OBJECTS) -o $@ -shared ../ss.dll

exp-main.o: $(OBJECTS)
	$(LD) $(OBJECTS) -o $@ -r

c-parse.o: c-parse.tab.c lex.c_lang.c
	$(CC) $(CFLAGS) -o $@ $<

lex.c_lang.c: c-lex.y
	flex -Pc_lang $<

c-parse.tab.h: c-parse.y
	bison -d $<

c-parse.tab.c: c-parse.y
	bison -d $<

clean:
	-rm $(OBJECTS) lex.c_lang.c c-parse.tab.c c-parse.tab.h exp-main.o
	-del $(OBJECTS) lex.c_lang.c c-parse.tab.c c-parse.tab.h exp-main.o

