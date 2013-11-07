
LD = ld
CC = gcc
CFLAGS = -g -DLIBTROLL_TEST_DRIVE
INCDIRS = -I./include/ -I./bkpt/ -I./disasm/ -I./regview/ -I./exec/ -I./stackframe/ \
	  -I./srcinfo/ -I./vardisplay/ -I./memdump/

OBJECTS = objtemplate/objname.o regview/regview.o \
	  disasm/disasm.o bkpt/bkpt.o exec/exec.o stackframe/stackframe.o \
	  srcinfo/srcinfo.o vardisplay/vardisplay.o memdump/memdump.o

libtroll: $(OBJECTS) libtroll.c
	$(CC) $(CFLAGS) libtroll.c -o $@ $(OBJECTS) $(INCDIRS)

clean:
	$(MAKE) -C objtemplate -i clean
	$(MAKE) -C srcinfo -i clean
	$(MAKE) -C exec -i clean
	$(MAKE) -C regview -i clean
	$(MAKE) -C disasm -i clean
	$(MAKE) -C bkpt -i clean
	$(MAKE) -C stackframe -i clean
	$(MAKE) -C vardisplay -i clean
	$(MAKE) -C memdump -i clean
	-rm libtroll

objtemplate/objname.o:
	$(MAKE) -C objtemplate

srcinfo/srcinfo.o:
	$(MAKE) -C srcinfo

stackframe/stackframe.o:
	$(MAKE) -C stackframe

exec/exec.o:
	$(MAKE) -C exec

regview/regview.o:
	$(MAKE) -C regview

disasm/disasm.o:
	$(MAKE) -C disasm

bkpt/bkpt.o:
	$(MAKE) -C bkpt

vardisplay/vardisplay.o:
	$(MAKE) -C vardisplay

memdump/memdump.o:
	$(MAKE) -C memdump

