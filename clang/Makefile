
CC = g++
CC = gcc
OBJECTS = lexer.o parser.o main.o calc-expr.o
CFLAGS = -g

cparse: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

main.o: ctest.c
	$(CC) $(CFLAGS) -c -o $@ $<

calc-expr.o: calc-expr.c
	$(CC) $(CFLAGS) -c -o $@ $<

lexer.o: lex.yy.c parser.tab.h
	$(CC) $(CFLAGS) -c -o $@ $<

parser.o: parser.tab.c parser.tab.h
	$(CC) $(CFLAGS) -c -o $@ $<

lex.yy.c: clexer.y
	flex $<

parser.tab.c: parser.y
	bison -d $<

parser.tab.h: parser.y
	bison -d $<

clean:
	-rm $(OBJECTS) cparse lex.yy.c parser.tab.* stack.hh location.hh position.hh
