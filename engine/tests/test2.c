#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <string.h>

#include <libdwarf.h>


static void foo1_(void) __attribute__ ((section("sec1")));
static void foo2_(void) __attribute__ ((section("sec2")));
static void foo3_(void) __attribute__ ((section("sec3")));

static void foo1_(void) {foo2_();foo2();}
static void foo2_(void) {foo3(); foo3_();}
static void foo3_(void) {exit(10);}

#include "inline-tests.c"


static int (*(*((*(*(*(* fptr_[36][56])[67])(void))[12])(int, char, int * (*)(char))))(int))[23] = { 0 };
#if 0
int (*(*((*(* fptr(void))[12])(int, char, int * (*)(char))))(int))[23]
{
return 0;
}
#endif

static void print_error_1(Dwarf_Debug dbg, char * errmsg, int sres, Dwarf_Error err)
{
	printf("print_error(): %s\n", errmsg);
}

