#include <stdarg.h>


#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "gear.h"
#include "arm-dis.h"
#include "cu-access.h"
#include "srcfile.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "aranges-access.h"
#include "dwarf-frame-sup.h"
#include "util.h"
#include "gprintf.h"
#include "miprintf.h"


/* data for breakpoints */
#define NR_BREAKPOINTS	2
/*! used to limit the depth of frame unwinding in backtraces */
#define MAX_BACKTRACE_DEPTH	64

static struct
{
	ARM_CORE_WORD		addr;
	int		active;
	//PointHandle	handle;
}
breakpoint_data[NR_BREAKPOINTS];

void dump_mem(struct gear_engine_context * ctx)
{
char fmt[16];
unsigned char byte_buf[8];
int nbytes;
int fmt_idx;
int i;

	fmt_idx = 0;
	/* construct format string */
	fmt[fmt_idx++] = '%';
	switch (def_exam_format.print_format)
	{
		case 'i':
			gprintf("disassembling\n");
			for (i = 0; i < def_exam_format.nr_elements; i++)
			{
				gprintf("0x%08x\t", def_exam_addr);
				arm_disassemble_insn(ctx->cc, def_exam_addr, vgprintf);
				gprintf("\n");
				def_exam_addr += 4;
			}
			return;
			break;
		case 's':
			panic("string: \n");
			return;
			break;
		default:
			break;
	}
	switch (def_exam_format.data_size)
	{
		case 1:
			fmt[fmt_idx++] = 'h';
			fmt[fmt_idx++] = 'h';
			break;
		case 2:
			fmt[fmt_idx++] = 'h';
			break;
		case 4:
			fmt[fmt_idx++] = 'l';
			break;
		case 8:
			fmt[fmt_idx++] = 'l';
			fmt[fmt_idx++] = 'l';
			break;
		default:
			panic("unsupported data size requested");
			return;
			break;
	}

	fmt[fmt_idx++] = def_exam_format.print_format;
	fmt[fmt_idx] = 0;

	gprintf("memory dump at 0x%08x\n", def_exam_addr);
	/*! \todo	optimize this for block access */
	switch (def_exam_format.data_size)
	{
		case 1:
			nbytes = 1;
			for (i = 0; i < def_exam_format.nr_elements; i++)
			{
				ctx->cc->core_mem_read(ctx, byte_buf, def_exam_addr, &nbytes);
				def_exam_addr += 1;
				gprintf(fmt, *byte_buf);
				gprintf(" ");
			}
			break;
		case 2:
			nbytes = 2;
			for (i = 0; i < def_exam_format.nr_elements; i++)
			{
				ctx->cc->core_mem_read(ctx, byte_buf, def_exam_addr, &nbytes);
				def_exam_addr += 2;
				gprintf(fmt, *(unsigned short *) byte_buf);
				gprintf(" ");
			}
			break;
		case 4:
			nbytes = 4;
			for (i = 0; i < def_exam_format.nr_elements; i++)
			{
				ctx->cc->core_mem_read(ctx, byte_buf, def_exam_addr, &nbytes);
				def_exam_addr += 4;
				gprintf(fmt, *(unsigned int *) byte_buf);
				gprintf(" ");
			}
			break;
		case 8:
			nbytes = 8;
			for (i = 0; i < def_exam_format.nr_elements; i++)
			{
				ctx->cc->core_mem_read(ctx, byte_buf, def_exam_addr, &nbytes);
				def_exam_addr += 8;
				gprintf(fmt, *(unsigned long long *) byte_buf);
				gprintf(" ");
			}
			break;
		default:
			panic("unsupported data size requested");
			return;
			break;
	}
	gprintf("\n");
}


void dump_core_regs_mi(struct gear_engine_context * ctx)
{
ARM_CORE_WORD regs[NR_ARM_CORE_REGS + 3];
int i;

	ctx->cc->core_reg_read(ctx, 0, (1 << (NR_ARM_CORE_REGS + 3)) - 1, regs);
	for (i = 0; i < NR_ARM_CORE_REGS; i++)
		miprintf("\"r%i\" = 0x%08x, ", i, regs[i]);

	miprintf("\"pc\" = 0x%08x, ", regs[i++]);
	miprintf("\"cpsr\" = 0x%08x, ", regs[i++]);
	miprintf("\"spsr\" = 0x%08x, ", regs[i]);

}

void set_reg(struct gear_engine_context * ctx, int reg_nr, ARM_CORE_WORD val)
{
	ctx->cc->core_reg_write(ctx, 0, 1 << reg_nr, &val);
}

ARM_CORE_WORD get_reg(struct gear_engine_context * ctx, int reg_nr)
{
ARM_CORE_WORD retval;
	ctx->cc->core_reg_read(ctx, 0, 1 << reg_nr, &retval);
	return retval;
}

void insn_step(struct gear_engine_context * ctx)
{
	ctx->cc->core_step();
}


void dump_exec_context(struct gear_engine_context * ctx)
{
ARM_CORE_WORD pcreg, fbreg;
struct cu_data * cu;
struct subprogram_data * subp;
int is_result_reg;

	ctx->cc->core_reg_read(ctx, 0, 1 << 15, &pcreg);
	gprintf("pc: 0x%08x\n", pcreg);
	if (!(cu = aranges_get_cu_for_addr(ctx, pcreg)))
	{
		gprintf("outside known domains");
		return;
	}
	gprintf("in file %s\n", cu->name);

	if (!(subp = aranges_get_subp_for_addr(ctx, pcreg)))
	{
		panic("not in a function within the module");
		return;
	}
	gprintf("in function %s\n", subp->name);

	/*! \todo	fix the cu base address computation */
	if (!subp->is_frame_base_available)
		panic("");
	if(dwarf_loc_eval_loc_from_list(ctx, &fbreg, &is_result_reg, 0, &subp->fb_location, pcreg, cu->low_pc, 0))
		panic("failed to evaluate frame base expression");
	gprintf("frame base at: ");
	gprintf(is_result_reg ? "r%i\n" : "0x%08x\n", fbreg);
	srcfile_print_srclines(ctx, cu, pcreg);
}

#if 0
void set_break(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
int i;

	/* find a free breakpoint */
	for (i = 0; i < NR_BREAKPOINTS; i++)
		if (!breakpoint_data[i].active)
			break;
	if (i == NR_BREAKPOINTS)
	{
		gprintf("no free breakpoints, breakpoint not set\n");
		return;
	}

	breakpoint_data[i].active = 1;
	breakpoint_data[i].addr = addr;

	if (ctx->cc->core_set_break(addr, &breakpoint_data[i].handle) != GEAR_ERR_NO_ERROR)
		panic("core_set_break");
	gprintf("ok, set breakpoint %i at address 0x%08x\n", i, addr);
}
#endif


void show_breakpoints(void)
{
int i;

	gprintf("active breakpoints:\n");
	for (i = 0; i < NR_BREAKPOINTS; i++)
		if (breakpoint_data[i].active)
		{
			gprintf("breakpoint %i at address 0x%08x\n", i, breakpoint_data[i].addr);
		}

}

void clear_break(struct gear_engine_context * ctx, int brekpoint_nr)
{
	if (brekpoint_nr >= NR_BREAKPOINTS)
	{
		gprintf("invalid breakpoint number\n");
		return;
	}
	if (!breakpoint_data[brekpoint_nr].active)
	{
		gprintf("unknown breakpoint number");
		return;
	}
	breakpoint_data[brekpoint_nr].active = 0;
	panic("");
/*
	if (ctx->cc->core_clear_break(breakpoint_data[brekpoint_nr].handle) == GEAR_ERR_NO_ERROR)
		panic("core_clear_break");
*/
	gprintf("ok, breakpoint %i deleted\n", brekpoint_nr);

}


void code_run(struct gear_engine_context * ctx)
{
	panic("");
}

void dump_backtrace(struct gear_engine_context * ctx)
{
ARM_CORE_WORD regs[NR_ARM_CORE_REGS];
ARM_CORE_WORD pcreg;
struct cu_data * cu;
struct subprogram_data * subp;
int backtrace_cnt;


	ctx->cc->core_reg_read(ctx, 0, (1 << NR_ARM_CORE_REGS) - 1, regs);
	pcreg = regs[15];
	backtrace_cnt = 0;
	do
	{
		gprintf("pc: 0x%08x\n", pcreg);
		if (!(cu = aranges_get_cu_for_addr(ctx, pcreg)))
		{
			gprintf("outside known domains");
			return;
		}

		gprintf("[%4i]\t", backtrace_cnt);
		gprintf("in file %s ,", cu->name);

		if (!(subp = aranges_get_subp_for_addr(ctx, pcreg)))
		{
			panic("not in a function within the module");
			return;
		}
		gprintf("in function %s\n", subp->name);
	}
	while (!dwarf_frame_unwind(ctx, (ARM_CORE_WORD (*)[NR_ARM_CORE_REGS])regs,
		pcreg, &pcreg) && ++backtrace_cnt < MAX_BACKTRACE_DEPTH);
}

void init_gear(void)
{

	memset(breakpoint_data, 0, sizeof(breakpoint_data));
}

