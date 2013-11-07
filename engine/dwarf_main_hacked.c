#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"

#include "cu-access.h"
#include "srcfile.h"
#include "util.h"

#include <windows.h>


/* process each compilation unit in .debug_info */
static void exit_error(char * errmsg)
{
	gprintf("fatal error: %s\n", errmsg);
	exit(1);
}



void print_error(Dwarf_Debug dbg, char * errmsg, int sres, Dwarf_Error err)
{
	gprintf("print_error(): %s\n", errmsg);
}

void dwarf_hacked_init(struct gear_engine_context * ctx)
{
int elf_fd;
Dwarf_Debug dbg;
int res = DW_DLV_OK;
Dwarf_Error err;
Dwarf_Unsigned	next_cu_header_offset;
Dwarf_Off cur_cu_offset;
Elf * libelf_d;

	elf_fd = open(ctx->dbg_info_elf_disk_file_name, O_RDONLY | O_BINARY, 0);
	/* validate libelf version */
	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		exit_error("libelf out of date\n");
	}
	if (elf_fd < 0)
	{
		exit_error("could not open input elf file\n");
		exit(1);
	}
	ctx->dbg_elf_fd = elf_fd;
	if (!(libelf_d = elf_begin(elf_fd, ELF_C_READ, 0)))
	{
		exit_error("elf_begin() failed\n");
	}
	ctx->libelf_elf_desc = libelf_d;

	//if ((res = dwarf_init(elf_fd, DW_DLC_READ, NULL, NULL, &dbg, &err)) != DW_DLV_OK)
	if ((res = dwarf_elf_init(libelf_d, DW_DLC_READ, NULL, NULL, &dbg, &err)) != DW_DLV_OK)
	{
		exit_error("dwarf_elf_init() failure");
	}
	ctx->dbg = dbg;

	while ((res = dwarf_next_cu_header(ctx->dbg,
					/* dont need these */
					0,
					0,
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
		/* kill time; as weve bloody got any of it to waste... */	 
		;
	if (0) init_aranges(ctx);
	if (res != DW_DLV_NO_ENTRY)
		panic("");
	/* start from offset 0 - the start of the debug information
	 * (.dbg_info) section */	 
	cur_cu_offset = 0;
	/* process all compilation units */
	while ((res = dwarf_next_cu_header(ctx->dbg,
					/* dont need these */
					0,
					0,
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
	{
		if (dwarf_get_cu_die_offset_given_cu_header_offset
			(ctx->dbg, cur_cu_offset, &cur_cu_offset, &err)
				!= DW_DLV_OK)
			panic("");
		if (0) {
		extern HWND hwnd;
		MSG msg;
		HDC hdc;
		PAINTSTRUCT ps;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				printf("msg: %i;", msg.message);
				if (msg.message == WM_PAINT)
					break;
			}
			if (!(hdc = BeginPaint(hwnd, &ps)))
				exit(1);
			TextOut(hdc, 0, 0, "hello, world", 5);
			EndPaint(hwnd, &ps);

		}
		cu_process(ctx, cur_cu_offset);
		cur_cu_offset = next_cu_header_offset;
	}

	gprintf("dwarf engine successfully initialized\n");

}
