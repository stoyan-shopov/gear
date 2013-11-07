
xp: c-parse.y c-lex.y
	bison c-parse.y
	flex c-lex.y
	gcc -g -o $@ lex.yy.c c-parse.tab.c

xx.dll: c-expr-eval.cxx c-expr-eval.hxx
#cpp  c-expr-eval.cxx -I ../../dwarf-20120410/libdwarf/ -I ../engine/ -I  ../engine/include/ -I ../engine/include/target/arm/
	-rm *.gch
	gcc -g -c -o cparse.o c-parse.tab.c -shared
	gcc -g -c -o clex.o lex.yy.c -shared
	g++ -Wall -g c-expr-eval.cxx -I ../../dwarf-20120410/libdwarf/ -I ../engine/ -I  ../engine/include/ -I ../engine/include/target/arm/ \
		../engine/ss.dll -shared cparse.o clex.o -o $@

