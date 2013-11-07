
CC	= arm-elf-gcc
CPP	= arm-elf-cpp
AS	= arm-elf-as
LD	= arm-elf-ld

OBJECTS = startup.o flash-driver.o


flash-loader: $(OBJECTS)
	$(LD) -e _asm_startup -T linker-script.ld -o $@ $(OBJECTS)
	-rm start-tmp.S

startup.o: startup.S
	$(CPP) $< start-tmp.S
	$(AS) start-tmp.S -o $@

flash-driver.o: flash-driver.c
	$(CC) -c -o $@ $<

clean:
	-rm $(OBJECTS) start-tmp.S flash-loader

