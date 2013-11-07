
PACKAGE = tools

all: dwgen elfcpy troll sterm

arc:
	$(MAKE) clean -i
	tar cf $(PACKAGE).tar *.c *.cxx *.hxx Makefile
	gzip $(PACKAGE).tar

sterm: vt102.c term.c
	gcc -o $@ $^ -g -lX11

dwgen: dwgen.cxx troll.cxx
	g++ -o $@ $^ -g -lelf -ldwarf -I../../dwarf-20090716/libdwarf/ -L../../dwarf-20090716/libdwarf/

elfcpy: elfcpy.c
	gcc -g -o $@ $< -lelf

troll: troll.cxx troll.hxx
	g++ -DTROLL_TESTDRIVE -o $@ $< -g -lelf -ldwarf -I../../dwarf-20090716/libdwarf/ -L../../dwarf-20090716/libdwarf/

clean:
	rm dwgen elfcpy troll sterm arc.tar.gz
