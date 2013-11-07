
CC = gcc
CPP = g++
LD = ld
INCDIRS = -I./ -I./armemu/ -I./include/
CFLAGS = -c -DMODE32 -DMODET -g $(INCDIRS) -D GEAR_UI_BARE_METAL

dist:
	-rm arc.tar*
	$(MAKE) -i clean
	tar -cf arc.tar --exclude=armemu \
		--exclude=arm-gear* --exclude=*.o --exclude=*.exe --exclude=logos.txt \
		--exclude=test.elf --exclude=*.gz --exclude=doc *
	gzip arc.tar

clean:
	$(MAKE) -i -C ./engine/ clean
	$(MAKE) -i -C ./engine/target-ctl/sim clean
	$(MAKE) -i -C ./engine/target-ctl/i386-linux-native/ clean
	$(MAKE) -i -C ./ui/console clean
	$(MAKE) -i -C ./ui/ncurses clean
	$(MAKE) -i -C ./ui/xcurses clean
	$(MAKE) -i -C ./ui/xaw clean
	$(MAKE) -i -C ./ui/ncd clean
	$(MAKE) -i -C ./ui/xcd clean
	$(MAKE) -i -C ./ui/console/mi-parsers/ clean
	$(MAKE) -i -C ./ui/lib/ clean
	$(MAKE) -i -C ./ui/lib/mi-parse/ clean

