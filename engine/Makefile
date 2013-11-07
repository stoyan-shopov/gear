#CROSS_COMPILE_PREFIX=i386-mingw32-

CC = $(CROSS_COMPILE_PREFIX)gcc
CPP = $(CROSS_COMPILE_PREFIX)g++
LD = $(CROSS_COMPILE_PREFIX)ld
INCDIRS = -I./ -I./include/ -I./include/target/arm/ -I/home/mgw/mingw32-binutils/include/ -I../libtroll/include/
#CFLAGS = -Wall -c -g $(INCDIRS) -Wall -ftest-coverage -fprofile-arcs
CFLAGS = -Wall -c -g $(INCDIRS) -Wall -DTARGET_ARMV7M -O0
CORE_OBJECTS = \
	cu-access.o subprogram-access.o type-access.o \
	dobj-access.o \
	lexblock-access.o aranges-access.o srcfile.o dwarf-util.o dwarf-loc.o \
	scope.o dwarf-expr.o frame-reg-cache.o symtab.o fdprintf.o \
	engine.o gprintf.o miprintf.o dbx-parser.o cparse/exp-main.o \
	breakpoint.o exec.o \
	dwarf-ranges.o\
	target-dump-cstring.o\
	target-comm.o \
	cxx-hacks.o \
	target-hacks-armv7m.o \
	armv7m-target-desc.o

SS_OBJECTS = \
	cu-access.o subprogram-access.o type-access.o \
	dobj-access.o \
	lexblock-access.o aranges-access.o srcfile.o dwarf-util.o dwarf-loc.o \
	scope.o dwarf-expr.o symtab.o fdprintf.o \
	gprintf.o \
	cxx-hacks.o \
	miprintf.o \
	arena.o \
	dwarf-ranges.o

GENERATED_SOURCES = lex.dbx.c

arm-gear: gear-core.o
	$(CROSS_COMPILE_PREFIX)g++ gear-core.o -o $@ \
		-L/usr/lib/binutils/i686-pc-linux-gnu/2.18/ -lws2_32 -lgdi32 \
		-L /home/mgw/mingw32-binutils/lib/ -ludis86 -luser32 -lkernel32 \
		-L ./ -lstpcpy -lelf -ldwarf -L c:/mingw32-gcc-4.6.3/bin/

ss.dll: $(SS_OBJECTS)
	$(CROSS_COMPILE_PREFIX)g++ $^ -o $@ -shared \
		-lws2_32 -ldwarf -lelf -L c:/mingw32-gcc-4.6.3/bin/

unwind.dll: frame-reg-cache.o gprintf.o ss.dll t-armv7m.dll
	$(CROSS_COMPILE_PREFIX)g++ $^ -o $@ -shared \
		-ldwarf -lelf -L c:/mingw32-gcc-4.6.3/bin/

t-armv7m.dll: armv7m-target-desc.o
	$(CROSS_COMPILE_PREFIX)g++ $^ -o $@ -shared \
		-ldwarf -lelf -L c:/mingw32-gcc-4.6.3/bin/

armv7m-target-desc.o: armv7m-target-desc.c
	$(CC) $(CFLAGS) -o $@ $<

unw-test: unwind-test.o unwind.dll ss.dll t-armv7m.dll ..\..\libgdb\libgdb.dll
	$(CROSS_COMPILE_PREFIX)g++ $^ -o $@ \
		-ldwarf -lelf -L c:/mingw32-gcc-4.6.3/bin/

unwind-test.o: unwind-test.c
	$(CROSS_COMPILE_PREFIX)gcc $^ -o $@ $(CFLAGS) -c -I../../libgdb/

stest: stest.o ss.dll xx.dll
	$(CROSS_COMPILE_PREFIX)g++ $^ -o $@ \
		-lws2_32 -ldwarf -lelf -L c:/mingw32-gcc-4.6.3/bin/

xx.dll: ../c-parse/xx.dll
	copy ..\c-parse\xx.dll

../c-parse/xx.dll: ../c-parse/c-expr-eval.cxx
	make -C ../c-parse xx.dll

stest.o: stest.c
	$(CROSS_COMPILE_PREFIX)g++ $^ -o $@ $(CFLAGS) -I../../dwarf-20120410/libdwarf/ -I../c-parse/ \
		-lws2_32 -ldwarf -lelf -L c:/mingw32-gcc-4.6.3/bin/

gear-core.o: $(CORE_OBJECTS)
	$(CROSS_COMPILE_PREFIX)ld -r $(CORE_OBJECTS) -o $@


dwarf-ranges.o: dwarf-ranges.c
	$(CC) $(CFLAGS) -o $@ $<

cu-access.o: cu-access.c
	$(CC) $(CFLAGS) -o $@ $<

symtab.o: symtab.c
	$(CC) $(CFLAGS) -o $@ $<

frame-reg-cache.o: frame-reg-cache.c
	$(CC) $(CFLAGS) -o $@ $<

dwarf-util.o: dwarf-util.c
	$(CC) $(CFLAGS) -o $@ $<

dwarf-loc.o: dwarf-loc.c
	$(CC) $(CFLAGS) -o $@ $<

scope.o: scope.c
	$(CC) $(CFLAGS) -o $@ $<

dwarf-expr.o: dwarf-expr.c
	$(CC) $(CFLAGS) -o $@ $<

srcfile.o: srcfile.c
	$(CC) $(CFLAGS) -o $@ $<

subprogram-access.o: subprogram-access.c
	$(CC) $(CFLAGS) -o $@ $<

type-access.o: type-access.c
	$(CC) $(CFLAGS) -o $@ $<

dobj-access.o: dobj-access.c
	$(CC) $(CFLAGS) -o $@ $<

lexblock-access.o: lexblock-access.c
	$(CC) $(CFLAGS) -o $@ $<

aranges-access.o: aranges-access.c
	$(CC) $(CFLAGS) -o $@ $<

engine.o: engine.c
	$(CC) $(CFLAGS) -I./target/flash-loaders/ -o $@ $<
# $(CPP) $(CFLAGS) -I./target/flash-loaders/ -o $@ $<

fdprintf.o: fdprintf.c
	$(CC) $(CFLAGS) -o $@ $^

miprintf.o: miprintf.c
	$(CC) $(CFLAGS) -o $@ $<

gprintf.o: gprintf.c
	$(CC) $(CFLAGS) -o $@ $<

exec.o: exec.c
	$(CC) $(CFLAGS) -o $@ $<

breakpoint.o: breakpoint.c
	$(CC) $(CFLAGS) -o $@ $<

dbx-parser.o: dbx-parser.c lex.dbx.c
	$(CC) $(CFLAGS) -o $@ $<

target-comm.o: target-comm.c lex.target_.c
	$(CC) $(CFLAGS) -o $@ $<

elfimg-process.o: elfimg-process.c
	$(CC) $(CFLAGS) -o $@ $<

target-dump-cstring.o: target-dump-cstring.c
	$(CC) $(CFLAGS) -o $@ $<

cxx-hacks.o: cxx-hacks.c
	$(CC) $(CFLAGS) -o $@ $<

lex.dbx.c: dbx-lex.y
	flex -Pdbx $<

lex.target_.c: target-lex.y
	flex $<

./cparse/exp-main.o: ./cparse/c-lex.y ./cparse/c-parse.y
	$(MAKE) -C ./cparse/

target/flash-loaders/target-img-load.o: target/flash-loaders/target-img-load.c
	$(MAKE) -C ./target/flash-loaders/


stpcpy.dll: stpcpy.c
	gcc -shared $< -o $@

distclean:
	$(MAKE) -i -C cparse clean
	-rm *.gcda
	-rm *.gcno
	-rm *.gcov
	-rm arm-gear $(CORE_OBJECTS) gear-core.o lex.dbx.c\
		lex.target_.c lex.target_.h
	$(MAKE) -i -C ./target-ctl/ clean
	$(MAKE) -C ./target/flash-loaders/ clean

dist:
	$(MAKE) -i distclean
	-rm arc.tar*
	tar -cf arc.tar --exclude=binutils* --exclude=target \
		--exclude=arm-gear* --exclude=*.o --exclude=*.exe --exclude=logos.txt \
		--exclude=test.elf --exclude=./doc/ *
	gzip arc.tar

test:
	$(CC) -finline-functions -O3 -g -I ../../dwarf-20100808/libdwarf/ test1.c test3.c -ldwarf -o test -L c:/mingw32-gcc-4.5.1/bin/

clean:
	-$(MAKE) -i -C ./cparse/ clean
	-$(MAKE) -i -C ./target-ctl/ clean
	-$(MAKE) -i -C ./target/ clean
	-$(MAKE) -C ./target/flash-loaders/ clean
	-rm *.gcda
	-rm *.gcno
	-rm *.gcov
	-rm arm-gear $(CORE_OBJECTS) gear-core.o lex.dbx.c lex.target_.c lex.target_.h

exp_eval: $(CORE_OBJECTS) cparse/exp_main.o
	$(CC) $(CORE_OBJECTS) ./cparse/exp_main.o -o $@ -ldwarf -lelf -L../dwarf-20071209/libdwarf/

