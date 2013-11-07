#include <stdarg.h>
#include <stdlib.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "core-access.h"
#include "engine-err.h"
#include "breakpoint.h"

#include "util.h"
#include "gprintf.h"

#include <libelf.h>


/*! \todo	move the useful ones of these to the gear_engine_context */
static enum GEAR_ENGINE_ERR_ENUM (* core_run_orig)(struct gear_engine_context * ctx);
static enum GEAR_ENGINE_ERR_ENUM (* core_insn_step_orig)(struct gear_engine_context * ctx);
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

static bool target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
{
	if (last_known_state == state)
		return true;
	last_known_state = state;
	if (state != TARGET_CORE_STATE_HALTED)
		/* pass the notification to others */
		return true;
	{
		/* normal target halt */
		uninstall_breakpoints(ctx);
		return true;
	}
}

static enum GEAR_ENGINE_ERR_ENUM core_run_hacked(struct gear_engine_context * ctx)
{
	install_breakpoints(ctx);
	return core_run_orig(ctx);
}
static enum GEAR_ENGINE_ERR_ENUM core_insn_step_hacked(struct gear_engine_context * ctx)
{
	install_breakpoints(ctx);
	return core_insn_step_orig(ctx);
}

void target_hacks_init(struct gear_engine_context * ctx)
{
	core_run_orig = ctx->cc->core_run;
	core_insn_step_orig = ctx->cc->core_insn_step;
	ctx->cc->core_run = core_run_hacked;
	ctx->cc->core_insn_step = core_insn_step_hacked;

	if (ctx->cc->core_register_target_state_change_callback(ctx, target_state_change_callback)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	gprintf("target run/step/halt execution hook hacks installed\n");
}

