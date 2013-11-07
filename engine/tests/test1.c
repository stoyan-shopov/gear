#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <string.h>

#include <libdwarf.h>

/* process each compilation unit in .debug_info */
static void exit_error(char * errmsg)
{
	printf("fatal error: %s\n", errmsg);
	exit(1);
}


extern void foo1(void) __attribute__ ((section("sec1")));
extern void foo2(void) __attribute__ ((section("sec2")));
extern void foo3(void) __attribute__ ((section("sec3")));

extern void foo1(void) {}
extern void foo2(void) {}
extern void foo3(void) {}

#include "test2.c"

int (*(*((*(*(*(* fptr[36][56])[67])(void))[12])(int, char, int * (*)(char))))(int))[23] = { 0 };
#if 0
int (*(*((*(* fptr(void))[12])(int, char, int * (*)(char))))(int))[23]
{
return 0;
}
#endif

void print_error(Dwarf_Debug dbg, char * errmsg, int sres, Dwarf_Error err)
{
	printf("print_error(): %s\n", errmsg);
}

int main_(void)
{
int elf_fd;
Dwarf_Debug dbg;
int res = DW_DLV_OK;
Dwarf_Error err;
Dwarf_Unsigned	next_cu_header_offset;
Dwarf_Off cur_cu_offset;


	elf_fd = open("test.elf", O_RDONLY | O_BINARY, 0);

	if ((res = dwarf_init(elf_fd, DW_DLC_READ, NULL, NULL, &dbg, &err)) != DW_DLV_OK)
	{
		exit_error("dwarf_init() failure");
	}
	printf("ok\n");
	return 0;
}

