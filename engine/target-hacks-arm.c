
#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "core-access.h"
#include "engine-err.h"
#include "breakpoint.h"

#include "util.h"

enum EMBEDDED_ICE_REGISTERS_ENUM
{
	EICE_REG_DBGCTL_ADDR		= 0,
	EICE_REG_DBGCTL_WIDTH		= 6,
	EICE_REG_DBGSTAT_ADDR		= 1,
	EICE_REG_DBGSTAT_WIDTH		= 5,
	EICE_REG_ABORTSTAT_ADDR		= 2,
	EICE_REG_ABORTSTAT_WIDTH	= 1,
	EICE_REG_DBGCOMMCTL_ADDR	= 4,
	EICE_REG_DBGCOMMCTL_WIDTH	= 6,
	EICE_REG_DBGCOMMDATA_ADDR	= 5,
	EICE_REG_DBGCOMMDATA_WIDTH	= 32,
	EICE_REG_WP0ADDRVAL_ADDR	= 8,
	EICE_REG_WP0ADDRVAL_WIDTH	= 32,
	EICE_REG_WP0ADDRMASK_ADDR	= 9,
	EICE_REG_WP0ADDRMASK_WIDTH	= 32,
	EICE_REG_WP0DATAVAL_ADDR	= 10,
	EICE_REG_WP0DATAVAL_WIDTH	= 32,
	EICE_REG_WP0DATAMASK_ADDR	= 11,
	EICE_REG_WP0DATAMASK_WIDTH	= 32,
	EICE_REG_WP0CTLVAL_ADDR		= 12,
	EICE_REG_WP0CTLVAL_WIDTH	= 9,
	EICE_REG_WP0CTLMASK_ADDR	= 13,
	EICE_REG_WP0CTLMASK_WIDTH	= 8,
	EICE_REG_WP1ADDRVAL_ADDR	= 16,
	EICE_REG_WP1ADDRVAL_WIDTH	= 32,
	EICE_REG_WP1ADDRMASK_ADDR	= 17,
	EICE_REG_WP1ADDRMASK_WIDTH	= 32,
	EICE_REG_WP1DATAVAL_ADDR	= 18,
	EICE_REG_WP1DATAVAL_WIDTH	= 32,
	EICE_REG_WP1DATAMASK_ADDR	= 19,
	EICE_REG_WP1DATAMASK_WIDTH	= 32,
	EICE_REG_WP1CTLVAL_ADDR		= 20,
	EICE_REG_WP1CTLVAL_WIDTH	= 9,
	EICE_REG_WP1CTLMASK_ADDR	= 21,
	EICE_REG_WP1CTLMASK_WIDTH	= 8,

	ARM7TDMIS_JTAG_ICEREG_WIDTH	= 38,
	ARM7TDMIS_JTAG_ICEREG_ADDR_BITS	= 5,

	ARM7TDMIS_JTAG_SC1_WIDTH	= 33,
};


enum ARM_IOCTL
{
	ARM_IOCTL_INVALID = 0,
	ARM_IOCTL_EMBEDDED_ICE_CONTROL,
	/* request word - 0 - invalid, 1 read, 2 - write
	 * reading:
	 * word 1 - watchpoint number
	 * response, if no error occurred:
	 * word 0 - data value
	 * word 1 - data mask
	 * word 2 - address value
	 * word 3 - address mask
	 * word 4 - control value
	 * word 5 - control mask
	 * 
	 * writing:
	 * word 1 - watchpoint number
	 * word 2 - data value
	 * word 3 - data mask
	 * word 4 - address value
	 * word 5 - address mask
	 * word 6 - control value
	 * word 7 - control mask
	 * response ok/not ok */
};

#if 0
simulator - breakpointing code must be changed to simulate the emulated target ice resources

/* no watchpoints */
use only hardware breakpoints

static step_over_single_insn_target_state_change_callback();/* step_breakpointed(); */	- uses both watchpoint units
run_over_breakpointed();
set_breakpoint();
clear_breakpoint();
static uninstall_breakpoints();
static install_breakpoints();
#endif



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
	if (is_stepping_over_breakpointed)
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
	else
	{
		/* a special case - stepping over breakpointed instructions */
		/*! \todo	enable this */
#if 0
		if (is_singular_instruction)
			panic("");
#endif
		/* breakpoints should have already been uninstalled - no need to call uninstall_breakpoints(); */
		/* setup_target_ice_for_single_instruction_step(); */
		saved_pc = pc;
		if (ctx->cc->io_ctl(ctx, 2, (ARM_CORE_WORD[2]) { [0] = 1, [1] = pc }, 0, 0)
				!= GEAR_ERR_NO_ERROR)
			panic("");
		is_stepping_over_breakpointed = true;
	}
	last_known_state = TARGET_CORE_STATE_INVALID;
	return core_run_orig(ctx);
}

void target_hacks_init(struct gear_engine_context * ctx)
{
	core_run_orig = ctx->cc->core_run;
	ctx->cc->core_run = core_run_hacked;
	if (ctx->cc->core_register_target_state_change_callback(ctx, step_over_single_insn_target_state_change_callback)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	gprintf("target run/halt execution hook hacks installed\n");
}

/*!
 *	\fn	static bool is_reg_callee_saved(struct gear_engine_context * ctx, int reg_nr)
 *	\brief	determines if a register is saved by a callee in the currently selected abi/pcs variant for the target
 *
 *	\todo	currently only supported is the bare metal/aapcs combination
 *
 *	\param	ctx	context to work in
 *	\param	reg_nr	the register number of interest
 *	\return	true, if the supplied register number is callee saved, false
 *		otherwise */
static bool is_arm_reg_callee_saved(struct gear_engine_context * ctx, int reg_nr)
{
bool res;	
	switch (reg_nr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 12:
		case 13:
		case 15:
			res = false;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 14:
			res = true;
			break;
		default:
			panic("");
	}
	return res;
}



/*!
 *	\fn	static bool is_reg_callee_saved(struct gear_engine_context * ctx, int reg_nr)
 *	\brief	determines if a register is saved by a callee in the currently selected abi/pcs variant for the target
 *
 *	\todo	currently only supported is the bare metal/aapcs combination
 *
 *	\param	ctx	context to work in
 *	\param	reg_nr	the register number of interest
 *	\return	true, if the supplied register number is callee saved, false
 *		otherwise */
static bool is_reg_callee_saved(struct gear_engine_context * ctx, int reg_nr)
{
bool res;	
	switch (reg_nr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 12:
		case 13:
		case 15:
			res = false;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 14:
			res = true;
			break;
		default:
			panic("");
	}
	return res;
}

