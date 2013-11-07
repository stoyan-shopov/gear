
CC = gcc
LD = ld
INCDIRS = -I../include -I../../engine/include -I../../engine/include/target/arm
CFLAGS = -c -g $(INCDIRS)

OBJNAME = regview
LEXER_PREFIX = yy_regview

OBJECTS = parser.o lexer.o

PARSER_GENERATED = $(OBJNAME)-parse.tab.h
PARSER_MAIN = $(OBJNAME)-parse.tab.c
LEXER_MAIN = lex.$(LEXER_PREFIX).c
LEXER_HEADER = $(OBJNAME)-lex.h
PARSER_GRAMMAR = $(OBJNAME)-parse.y
LEXER_GRAMMAR = $(OBJNAME)-lex.y

$(OBJNAME).o: $(OBJECTS)
	$(LD) $(OBJECTS) -o $@ -r

parser.o: $(PARSER_MAIN) $(PARSER_GENERATED) $(LEXER_HEADER) $(OBJNAME)-common.h
	$(CC) $(CFLAGS) -o $@ $<

lexer.o: $(LEXER_MAIN) $(OBJNAME)-common.h
	$(CC) $(CFLAGS) -o $@ $<

$(LEXER_MAIN): $(LEXER_GRAMMAR)
	flex -o$(LEXER_MAIN) $<

$(LEXER_HEADER): $(LEXER_GRAMMAR)
	flex -o$(LEXER_MAIN) $<

$(PARSER_MAIN): $(PARSER_GRAMMAR)	
	bison $<

$(PARSER_GENERATED): $(PARSER_GRAMMAR)
	bison $<

clean:
	-rm $(OBJECTS) $(PARSER_GENERATED) $(PARSER_MAIN) $(LEXER_MAIN) $(LEXER_HEADER) $(OBJNAME).o

