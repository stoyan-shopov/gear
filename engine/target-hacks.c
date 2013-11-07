
#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "core-access.h"
#include "engine-err.h"
#include "breakpoint.h"

#include "util.h"

#include <libelf.h>


static ARM_CORE_WORD target_entry_point;

/*! \todo	move the useful ones of these to the gear_engine_context */
static enum GEAR_ENGINE_ERR_ENUM (* core_run_orig)(struct gear_engine_context * ctx);
static bool is_stepping_over_breakpointed;
static ARM_CORE_WORD saved_pc;
static enum TARGET_CORE_STATE_ENUM last_known_state;


static void install_breakpoints(struct gear_engine_context * ctx)
{
struct bkpt_struct	* b;
	b = ctx->bkpts;
	while (b)
	{
		if (ctx->cc->core_set_break(ctx, b->addr) != GEAR_ERR_NO_ERROR)
			panic("");
		gprintf("bkpt set at 0x%08x\n", b->addr);
		b = b->next;
	}
}

static void uninstall_breakpoints(struct gear_engine_context * ctx)
{
struct bkpt_struct	* b;
	b = ctx->bkpts;
	while (b)
	{
		if (ctx->cc->core_clear_break(ctx, b->addr) != GEAR_ERR_NO_ERROR)
			panic("");
		b = b->next;
	}
}

static bool step_over_single_insn_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
{
	if (last_known_state == state)
		return true;
	last_known_state = state;
	if (state != TARGET_CORE_STATE_HALTED)
		/* pass the notification to others */
		return true;
	if (0 && is_stepping_over_breakpointed)
	{
	ARM_CORE_WORD pc;
		if (ctx->cc->core_reg_read(ctx, 0, 1 << ctx->tdesc->get_target_pc_reg_nr(ctx), &pc)
				!= GEAR_ERR_NO_ERROR)
			panic("");
		/* the stepping over the breakpointed instruction is complete - revert
		 * to normal target running */
		if (ctx->cc->io_ctl(ctx, 2, (ARM_CORE_WORD[2]) { [0] = 0, [1] = saved_pc }, 0, 0)
				!= GEAR_ERR_NO_ERROR)
			panic("");
		if (bkpt_locate_at_addr(ctx, pc))
		{
			/* notify others that a breakpoint has been hit */
			/*! \note	here the breakpoints are not installed, no
			 *		need to call uninstall_breakpoints() */
			return true;
		}
		is_stepping_over_breakpointed = false;
		install_breakpoints(ctx);
		if (core_run_orig(ctx) != GEAR_ERR_NO_ERROR)
			panic("");
		/* do not pass the target state change notification to others */
		return false;
	}
	else
	{
		/* normal target halt */
		uninstall_breakpoints(ctx);
		return true;
	}
}

static enum GEAR_ENGINE_ERR_ENUM core_run_hacked(struct gear_engine_context * ctx)
{
ARM_CORE_WORD pc;
	if (ctx->cc->core_reg_read(ctx, 0, 1 << ctx->tdesc->get_target_pc_reg_nr(ctx), &pc)
			!= GEAR_ERR_NO_ERROR)
		panic("");

	if (!bkpt_locate_at_addr(ctx, pc))
	{
		/* the more common case - stepping over an instruction that
		 * is not breakpointed */
		install_breakpoints(ctx);
		is_stepping_over_breakpointed = false;
	}
	else if (0)
	{
		bkpt_clear_at_addr(ctx, pc);
		install_breakpoints(ctx);
		is_stepping_over_breakpointed = false;
	}
	else
	{
		/* a special case - stepping over breakpointed instructions */
#define MAX_RELOCATED_INSN_BYTE_SIZE		32
		unsigned char reloc_buf[MAX_RELOCATED_INSN_BYTE_SIZE];
		int len, nbytes;

		is_stepping_over_breakpointed = true;
		/* relocate insn */
extern void x86_relocate_insn_to_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr_to_reloc_to, int * insn_byte_len, 
		/*static*/ unsigned char reloc_buf[MAX_RELOCATED_INSN_BYTE_SIZE]);
		x86_relocate_insn_to_addr(ctx, target_entry_point, &len, reloc_buf);
		nbytes = len;
		if (1) { int i; for (gprintf("patching (run_over_bkpt at 0x%08x): ", target_entry_point), i = 0; i < len; gprintf("%02x ", reloc_buf[i++])); }
		if (ctx->cc->core_mem_write(ctx, target_entry_point, reloc_buf, &nbytes)
				!= GEAR_ERR_NO_ERROR || nbytes != len)
			panic("");
		if (ctx->cc->core_reg_write(ctx, 0, 1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
					&target_entry_point) != GEAR_ERR_NO_ERROR)
			panic("");
		install_breakpoints(ctx);
	}
	last_known_state = TARGET_CORE_STATE_INVALID;
	return core_run_orig(ctx);
}

void target_hacks_init(struct gear_engine_context * ctx)
{
GElf_Ehdr eh;
	core_run_orig = ctx->cc->core_run;
	ctx->cc->core_run = core_run_hacked;
	if (ctx->cc->core_register_target_state_change_callback(ctx, step_over_single_insn_target_state_change_callback)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (!gelf_getehdr(ctx->libelf_elf_desc, &eh))
	{
		panic("gelf_getehdr() failed\n");
	}
	target_entry_point = eh.e_entry;
	gprintf("target_entry_point is 0x%08x\n", target_entry_point);
	gprintf("target run/halt execution hook hacks installed\n");
}

