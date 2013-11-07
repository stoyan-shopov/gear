#include <unistd.h>
#if __LINUX__
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#else
#define _WIN32_WINNT	0x0501
#include <windows.h>
#include <wincon.h>
#include <winsock2.h>
#include <winuser.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int gprintf(const char * format, ...);

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-constants.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "gear.h"
#include "arm-dis.h"
#include "symtab.h"
#include "dobj-access.h"
#include "type-access.h"
#include "aranges-access.h"
#include "util.h"
#include "srcfile.h"
#include "exec.h"
#include "breakpoint.h"
#include "dbx-support.h"
#include "target-comm.h"
#include "frame-reg-cache.h"
#include "target-dump-cstring.h"
#include "target-description.h"

#include "cu-access.h"
#include "subprogram-access.h"

#include "libgdb.h"


struct libgdb_ctx * gdbctx;


static enum GEAR_ENGINE_ERR_ENUM memread(struct gear_engine_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes)
{
	if (libgdb_readwords(gdbctx, source, * nbytes >> 2, (uint32_t *) dest))
		return GEAR_ERR_GENERIC_ERROR;
	return GEAR_ERR_NO_ERROR;
}
static enum GEAR_ENGINE_ERR_ENUM memwrite(struct gear_engine_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes)
{
	if (libgdb_writewords(gdbctx, dest, * nbytes >> 2, (uint32_t *) source))
		return GEAR_ERR_GENERIC_ERROR;
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM regread(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i;
uint32_t * buf = buffer;

	i = 0;
	while (mask)
	{
		if (mask & 1)
		{
			if (libgdb_readreg(gdbctx, i, buf ++))
				return GEAR_ERR_GENERIC_ERROR;
		}
		mask >>= 1;
		i ++;
	}
	return GEAR_ERR_NO_ERROR;
}

static enum GEAR_ENGINE_ERR_ENUM regwrite(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
int i;
uint32_t * buf = buffer;

	i = 0;
	while (mask)
	{
		if (mask & 1)
		{
			if (libgdb_writereg(gdbctx, i, * buf ++))
				return GEAR_ERR_GENERIC_ERROR;
		}
		mask >>= 1;
		i ++;
	}
	return GEAR_ERR_NO_ERROR;
}
bool (*unwind_runctl_callback)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state);
static enum GEAR_ENGINE_ERR_ENUM register_runctl_callback(struct gear_engine_context * ctx, 
			bool (*target_state_change_notification_callback)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state))
{
	unwind_runctl_callback = target_state_change_notification_callback;
	return GEAR_ERR_NO_ERROR;
}

extern void dwarf_hacked_init(struct gear_engine_context * ctx);
extern struct srcinfo_type_struct * srcfile_get_srcinfo(struct gear_engine_context * ctx);

static struct core_control target_cc =
{
	.is_connected = 0,
	.core_open = 0,
	0,
	.core_mem_read = memread,
	.core_mem_write = memwrite,
	.core_reg_read = regread,
	.core_reg_write = regwrite,
	0,
	0,
	.core_set_break = 0,
	.core_clear_break = 0,
	.core_run = 0,
	.core_halt = 0,
	.core_insn_step = 0,
	.io_ctl = 0,
	.core_get_status = 0,
	.core_register_target_state_change_callback = register_runctl_callback,
	.core_unregister_target_state_change_callback = 0,
};


extern void init_armv7m_target_desc(struct gear_engine_context * ctx);

#ifdef TROLL
int stest(int argc, char ** argv)
#else
int main(int argc, char ** argv)
#endif
{
struct gear_engine_context ctx;
struct srcinfo_type_struct * srcinfo;
int i;
uint32_t pc;

	memset(& ctx, 0, sizeof ctx);

	init_types(& ctx);
	init_symtab(& ctx);

	ctx.dbg_info_elf_disk_file_name = strdup((argc == 2) ? argv[1] : "vx2.elf");
	dwarf_hacked_init(& ctx);

	gprintf("initialization complete\n");
	gprintf("dtype_data usage counts:\n");
	gprintf("nr_base_type_nodes == %i\n", type_access_stats.nr_base_type_nodes);
	gprintf("nr_typedef_nodes == %i\n", type_access_stats.nr_typedef_nodes);
	gprintf("nr_tqual_nodes == %i\n", type_access_stats.nr_tqual_nodes);
	gprintf("nr_arr_type_nodes == %i\n", type_access_stats.nr_arr_type_nodes);
	gprintf("nr_ptr_nodes == %i\n", type_access_stats.nr_ptr_nodes);
	gprintf("nr_struct_nodes == %i\n", type_access_stats.nr_struct_nodes);
	gprintf("nr_union_nodes == %i\n", type_access_stats.nr_union_nodes);
	gprintf("nr_member_nodes == %i\n", type_access_stats.nr_member_nodes);
	gprintf("nr_enumerator_nodes == %i\n", type_access_stats.nr_enumerator_nodes);
	gprintf("nr_enumeration_nodes == %i\n", type_access_stats.nr_enumeration_nodes);
	gprintf("nr_dobj_nodes == %i\n", type_access_stats.nr_dobj_nodes);
	gprintf("nr_subprogram_nodes == %i\n", type_access_stats.nr_subprogram_nodes);
	gprintf("nr_lexblocks == %i\n", type_access_stats.nr_lexblocks);
	gprintf("nr_subprogram_prototype_nodes == %i\n", type_access_stats.nr_subprogram_prototype_nodes);
	gprintf("nr_symtab_nodes == %i\n", type_access_stats.nr_symtab_nodes);

	if (!(gdbctx = libgdb_init()))
	{
		eprintf("failed to initialize the libgdb library\n");
		exit(1);
	}
	if (libgdb_connect(gdbctx, "127.0.0.1", 1122))
	{
		eprintf("failed to connect to a gdb server\n");
		exit(2);
	}

	ctx.cc = & target_cc;
	init_armv7m_target_desc(& ctx);
	init_frame_reg_cache(& ctx);

	unwind_runctl_callback(& ctx, TARGET_CORE_STATE_HALTED);
	for (i = 0; i < 16; i ++)
	{
		uint32_t x;
		if (ctx.cc->core_reg_read(& ctx, 0, 1 << i, & x) != GEAR_ERR_NO_ERROR)
		{
			printf("error reading register %i\n", i);
			exit(1);
		}
		printf("reg %i == 0x%08x\n", i, x);
	}

	init_aranges(& ctx);

	printf("backtrace:\n");
	i = 0;
	do
	{
		struct cu_data * cu; struct subprogram_data * subp; char * srcname; int srcline_nr; bool is_addr_at_src_boundary;
		if (ctx.cc->core_reg_read(& ctx, 0, 1 << ctx.tdesc->get_target_pc_reg_nr(& ctx), & pc) != GEAR_ERR_NO_ERROR)
		{
			printf("error reading program counter\n");
			exit(1);
		}
		srcfile_get_srcinfo_for_addr(& ctx, pc, & cu, & subp, & srcname, & srcline_nr, & is_addr_at_src_boundary);
		printf("--------------------------------------------------\n");
		printf("frame %i: ", i);
		printf("pc: 0x%08x, ", pc);
		printf("compilation unit: %s, ", cu ? cu->name : "<<< unknown >>>");
		printf("subprogram: %s, ", subp ? subp->name : "<<< unknown >>>");
		printf("source code file: %s, ", srcname ? srcname: "<<< unknown >>>");
		printf("line number: %i, ", srcline_nr);
		if (is_addr_at_src_boundary) printf("at EXACT source code boundary");
		printf ("\n");
		printf("--------------------------------------------------\n");
		i ++;
	}
	while (frame_move_to_relative(& ctx, -1, 0) == GEAR_ERR_NO_ERROR);

	return 0;
}


