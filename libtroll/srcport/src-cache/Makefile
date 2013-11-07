
CC=g++
LD=ld
INCDIRS=-I../include/ -I ../../include/

OBJECTS = src-cache.o c-lex.o

src.o: $(OBJECTS)
	$(LD) -r -o $@ $(OBJECTS)

src-cache.o: src-cache.cxx src-cache.hxx c-lex.h
	$(CC) -g -c -o $@ $< $(INCDIRS)

c-lex.o: lex.c_.c c-lex.h
	$(CC) -c -o $@ $<

lex.c_.c: c-lex.y
	flex $<

c-lex.h: c-lex.y
	flex $<

clean:
	-rm $(OBJECTS) src.o lex.c_.c c-lex.h

