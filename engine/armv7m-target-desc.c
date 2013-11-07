#include <stdarg.h>
#include <stdlib.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"
#include "gprintf.h"
#include "dwarf-expr.h"
#include "cu-access.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "breakpoint.h"
#include "exec.h"

#include "gear-constants.h"


#define MEMBUF_BYTE_SIZE	256



static int get_nr_target_core_regs(struct gear_engine_context * ctx) { return 16; } 
static int get_target_pc_reg_nr(struct gear_engine_context * ctx) { return 15; }
static int get_target_sp_reg_nr(struct gear_engine_context * ctx) { return 13; }
static int get_target_pstat_reg_nr(struct gear_engine_context * ctx) { return 17; } 
static bool is_dwarf_reg_nr_callee_saved(struct gear_engine_context * ctx, int dwarf_reg_nr)
{
	switch (dwarf_reg_nr)
	{
		case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 14:
			return true;
			break;
		default: return false;
	}
	panic("");
}
enum GEAR_ENGINE_ERR_ENUM translate_dwarf_reg_nr_to_target_reg_nr(struct gear_engine_context * ctx, int * inout_reg_nr) { return GEAR_ERR_NO_ERROR; }
static const char * translate_target_core_reg_nr_to_human_readable(struct gear_engine_context * ctx,
		unsigned int target_core_reg_nr)
{
static const char * reg_names[16] =
{
	[0] = "r0",
	[1] = "r1",
	[2] = "r2",
	[3] = "r3",
	[4] = "r4",
	[5] = "r5",
	[6] = "r6",
	[7] = "r7",
	[8] = "r8",
	[9] = "r9",
	[10] = "r10",
	[11] = "r11",
	[12] = "r12",
	[13] = "r13",
	[14] = "r14",
	[15] = "r15",
};

	return (target_core_reg_nr < 16) ? reg_names[target_core_reg_nr] : 0;
}


static int armv7m_print_disassembled_insn(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
		int (*print_fn)(const char * format, ...))
{
	print_fn("<disassembler currently broken>");
	return 2;
}

static int armv7m_insn_decode(struct gear_engine_context * ctx,
			ARM_CORE_WORD addr,
			ARM_CORE_WORD * next_insn_addr,
			bool * is_probably_a_function_call_insn)
{
	return 0;
}


static struct target_desc_struct armv7m_desc_struct =
{
	.get_nr_target_core_regs = get_nr_target_core_regs,
	.get_target_pc_reg_nr = get_target_pc_reg_nr,
	.get_target_sp_reg_nr = get_target_sp_reg_nr,
	.get_target_pstat_reg_nr = get_target_pstat_reg_nr,
	.is_dwarf_reg_nr_callee_saved = is_dwarf_reg_nr_callee_saved,
	.translate_dwarf_reg_nr_to_target_reg_nr = translate_dwarf_reg_nr_to_target_reg_nr,
	.translate_target_core_reg_nr_to_human_readable	= translate_target_core_reg_nr_to_human_readable,
	.decode_insn = armv7m_insn_decode,
	.print_disassembled_insn = armv7m_print_disassembled_insn,
};

void init_armv7m_target_desc(struct gear_engine_context * ctx)
{
	ctx->tdesc = &armv7m_desc_struct;
}

