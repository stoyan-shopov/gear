/*!
 *	\file	exec.c
 *	\brief	target execution control logic
 *
 *	this is the heart, the core of the gear debugger engine
 *
 *	the code in this module is responsible for maintaining
 *	the target and engine states and to react appopriately
 *	to various events and changes of state in the target and
 *	the engine; basically, the logic here is a bunch of state
 *	machines; the code here is not complex, but the need to handle
 *	many special cases and various quirks may make it look a bit
 *	complicated at times
 *
 *	\note	at places, the code that deals with maintatining the
 *		target state and controlling the target execution
 *		is referred to as the "executor"; this executor
 *		is comprised of two major parts:
 *			- the executor proper - the executor state
 *			machine which gets invoked whenever the
 *			target halts; the purpose of the executor
 *			in this case is to properly determine the
 *			target halt reason and to react appropriately
 *			depending on the executor's state itself
 *			(e.g. the target may be left halted and the
 *			user notified in the case of single instruction
 *			stepping, but the target could be silently
 *			resumed and allowed to run some more in the
 *			case of source code single stepping, when
 *			a source code construct boundary has not been
 *			reached); physically, this is the
 *			exec_target_halted() function below
 *			- the executor target execution control
 *			request external interface; physically,
 *			this is a set of externally visible functions
 *			used by code residing outside of this module
 *			to initiate various target execution operations
 *			(e.g., for instruction stepping over - there
 *			is the function exec_single_step_insn(), etc.);
 *			as usual, the prototypes of these functions
 *			are in the header file corresponding to this source
 *			file
 *
 *	in compliance with the architecture of the gear engine
 *	mandated by the gear engine architecture document
 *	\todo	provide a reference to the documentation files
 *		when they are written)
 *	the interaction between the gear engine and a target
 *	core controller must obey these rules:
 *		- no communication in the direction from the
 *		target core controller to the gear engine shall
 *		occur on the target core controllers initiative -
 *		all communication in the direction from the
 *		target core controller to the gear engine shall
 *		be in reply to a request from the gear engine
 *		to the target core controller; this means that
 *		the target core controller is strictly a slave
 *		of the gear engine master
 *		and never speaks to the gear engine unless
 *		explicitly addressed
 *		- there are two major target core (not
 *		target core controller) connection states
 *		(documented in the TARGET_CORE_CONNECTION_STATUS_ENUM
 *		enumeration in header file target-state.h),
 *		connection to the target core is either alive, or dead
 *		(with the addition that, when in the alive state,
 *		a target core may be running or halted - these two
 *		are logical "substates" of the alive target
 *		core connection state);
 *		as mandated by the gear engine architecture
 *		(and as outlined in the comments about
 *		the TARGET_CORE_CONNECTION_STATUS_ENUM enumeration),
 *		the transitions between these two states are
 *		detected within this module, and registered callback
 *		functions interested in these transitions
 *		are also invoked from within this module, as soon
 *		as a target core connection state transition is
 *		detected; the gear engine architecture mandates
 *		that the target core connection state transitions 
 *		are detected only within this module, and can
 *		be only the result of events in the communication
 *		flow between the gear engine and a target core controller;
 *		the transition from the dead to the alive state
 *		(provided the target connection state is currently dead)
 *		is performed on one occasion only - after a
 *		successful connection to a target core is established;
 *		the transition from the alive to the dead state is performed
 *		(provided the target connection state is currently alive)
 *		on three occasions: (1) the target core dies
 *		and the target core controller brings news
 *		about the death when interrogated by the gear
 *		engine; (2) - the connection between the gear engine
 *		and the target core controller drops down - in this
 *		case both the target core and the target core
 *		controller; (3) - the gear engine itself terminates
 *		the connection to the target core
 *
 *	\note	as an implementaion detail, it is notable that
 *		all the points where the transitions between
 *		the target core connection states are possible to occur,
 *		physically reside within module target-comm.c,
 *		and at all such points a target state change
 *		notification callback function (which, most
 *		commonly, is function exec_update_target_state()
 *		in this module) is invoked; also see comments
 *		about the core_register_target_state_change_callback
 *		data field in structure core_control in file
 *		core-access.h
 *
 *	\todo	enforce that the architectural requirements
 *		of the statement above are met at all times
 *
 *	\note	to decrease the risk of blunders maintaining the
 *		target core connection state, the target core
 *		connection state variable (namely, data field
 *		"core_state" in data structure exec_data_struct)
 *		is allowed to be modified in one, and only one
 *		subroutine within this module - exec_update_target_state();
 *		the only permitted exception to this rule is
 *		during the gear engine initialization;
 *		this state variable is otherwise freely available
 *		for inspection
 *	\todo	enforce that the architectural requirements
 *		of the statement above are met at all times
 *
 *	\todo	provide detailed descriptions of the operations
 *		of the main subroutines here (machine/source
 *		step into/over, etc.)
 *
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * include section follows
 *
 */

#include <stdlib.h>
#include <stdarg.h>

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


/*
 *
 * local types follow
 *
 */

/*! main executor state enumeration */
enum EXECUTOR_STATE_ENUM
{
	/*! an invalid state, used for catching errors */
	EXEC_STATE_INVALID = 0,
	/*! the executor is idle
	 *
	 * the executor is idle and not waiting for any specific target
	 * related state change event, this state implies the target
	 * is halted (not running) */
	EXEC_STATE_IDLE,
	/*! the executor is in instruction single step mode
	 *
	 * in this mode, the target should be running and the
	 * executor is waiting for a target halt event */
	EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP,
	/*! the executor is in source code single step mode
	 *
	 * in this mode, the target should be running and the
	 * executor is waiting for a target halt event to determine
	 * if the target should be allowed to run some more, or
	 * the end of the source code single stepping mode
	 * has been reached */
	EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP_SRC,
	/*! the executor is in instruction step over calls mode
	 *
	 * in this mode, the target should be running and the
	 * executor is waiting for a target halt event */
	EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER,
	/*! the executor is in instruction step over calls mode
	 *
	 * in this mode, the target should be running and the
	 * executor is waiting for a target halt event to determine
	 * if the target should be allowed to run some more, or
	 * the end of the source code step over calls mode
	 * has been reached */
	EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER_SRC,
	/*! the target is running freely
	 *
	 * in this mode the target is allowed to run freely and the
	 * executor is waiting for a target execution state change
	 * event such as a breakpoint hit, target halt by user request,
	 * some exceptional condition, etc. */
	EXEC_STATE_FREE_RUNNING,
};


/*! \todo	handle recursion */
/*! executor state data structure
 *
 * this one holds various data items used in different executor modes
 * (e.g. single stepping, stepping over calls, etc.), not all data items
 * are relevant and/or necessary for all executor modes
 */
struct exec_data_struct
{
	/*! primary executor state variable */
	enum EXECUTOR_STATE_ENUM	exec_state;
	/*! gear engine state variable
	 *
	 * read comments about enum TARGET_CORE_CONNECTION_STATUS_ENUM
	 * in file target-state.h for details */
	enum TARGET_CORE_STATE_ENUM	core_state;

	/*! various flags */
	struct
	{
		/*! denotes if stepping mode should be aborted when the target halts
		 *
		 * this is mainly needed for the proper handling of stepping modes
		 * in the case when there already exists a breakpoint on the expected
		 * stepping halt address - in this case no thing more should be done;
		 * when the target halts (if it halts, that is), the active stepping mode
		 * must get terminated (in all cases - no matter what
		 * the halt address), and reported must be that the target has stopped
		 *
		 * if this flag is set to nonzero, then the expected stepping
		 * into/over mode halt address fields below are not applicable
		 * and are invalid (fields expected_single_step_halt_addr and
		 * expected_step_over_halt_addr); not applicable
		 * and invalid also is the is_stepping_over flag, therefore,
		 * this flag shall be inspected prior to utilizing any
		 * of the is_stepping_over flag and the expected stepping
		 * halt fields below
		 *
		 * \todo	when the expected halt address fields below are fused,
		 *		provide actual references to the expected halt
		 *		address field within the paragraphs of this comment
		 *
		 * in short, this is basically a stepping mode abortion flag */
		int	must_step_mode_be_terminated : 1;

		/*! denotes whether stepping over a function call is in progress
		 *
		 * this is here to help coping with the case when a breakpoint
		 * has been hit, but this is due to a recursive call and therefore
		 * the breakpoint should be ignored this time; this flag is used in
		 * conjunction with the expected_stack_ptr_for_recursion data
		 * field below - when this flag is nonzero, the
		 * expected_stack_ptr_for_recursion field below is applicable
		 * and valid
		 *
		 * \note	this field is applicable and valid only if the
		 *		must_step_mode_be_terminated flag above
		 *		is zero
		 *
		 * also read the comments about the must_step_mode_be_terminated flag above */
		int	is_stepping_over	: 1;
	}
	flags;

	/*! \todo	the next two fields are redundant - fuse them */
	/*! a helper variable for single stepping mode (both instruction-wise and source code line-wise)
	 *
	 * this holds the expected target halt address when performing
	 * instruction level single stepping; also, this is the
	 * address at which a breakpoint has been inserted for
	 * the purpose of the single step algorithm, and care
	 * must be taken to make sure the breakpoint is cleared properly
	 *
	 * \note	this field is applicable and valid only if the
	 *		must_step_mode_be_terminated flag above
	 *		is zero
	 *
	 * also read the comments about the must_step_mode_be_terminated flag above */
	ARM_CORE_WORD			expected_single_step_halt_addr;
	/*! a helper variable for stepping over calls mode (both instruction-wise and source code line-wise)
	 *
	 * this holds the expected target halt address when performing
	 * instruction level stepping over calls; also, this is the
	 * address at which a breakpoint has been inserted for
	 * the purpose of the step over calls algorithm, and care
	 * must be taken to make sure the breakpoint is cleared properly
	 *
	 * \note	this field is applicable and valid only if the
	 *		must_step_mode_be_terminated flag above
	 *		is zero
	 *
	 * also read the comments about the must_step_mode_be_terminated flag above */
	ARM_CORE_WORD			expected_step_over_halt_addr;
	/*! expected stack pointer value for a breakpoint when performing step over calls
	 *
	 * this is here to cope with the case of a breakpoint being hit
	 * by a recursive invocation of a routine when performing a
	 * step over calls target execution; the algorithms for performing
	 * step over calls target execution should ignore breakpoint
	 * hits in descendant recursive invocations of a routine, and
	 * this is here to help cope with this case; this data field is
	 * used in conjuction with the is_stepping_over flag above, and is
	 * applicable if, and only if, the is_stepping_over flag is nonzero
	 *
	 * how are recursive invocations being detected? when a breakpoint
	 * is hit, and its address matches the expected halt address for
	 * step over calls mode, one last check is performed to discriminate
	 * between recursive invocations of a routine and "genuine" step over
	 * calls breakpoint hits (i.e. such hits that could lead to an
	 * end of the stepping over calls mode) - this check is comparing the current
	 * stack pointer against the stack pointer's expected value (stored here),
	 * and the breakpoint hit is termed a "genuine" one only if the
	 * two values match - otherwise the algorithms decide that a recursive
	 * subroutine invocation is in progress and ignore the breakpoint hit;
	 * this is an extremely simplistic approach, but i(sgs) hope it
	 * will work well in practice and bad surprises shall not arise
	 *
	 * \note	this field is applicable and valid only if the
	 *		is_stepping_over flag above is nonzero
	 */
	ARM_CORE_WORD			expected_stack_ptr_for_recursion;
};

/*
 *
 * local functions follow
 *
 */


/*!
 *	\fn	static void dump_halt_context(struct gear_engine_context * ctx, ARM_CORE_WORD pc)
 *	\brief	dumps common information when the target halts, in machine interface forma
 *
 *	this routine dumps, in machine interface format, as much information
 *	as possible about the target execution context at the time when
 *	the target halts execution - e.g. address, source file and line
 *	number where the target stopped
 *
 *	\param	ctx	context to work into
 *	\param	pc	program counter value to use when retrieving
 *			information about the halt context of the target;
 *			this normally equals the value of the program
 *			counter of the target when it halted
 *	\return	none
 */
static void dump_halt_context(struct gear_engine_context * ctx, ARM_CORE_WORD pc)
{
struct cu_data * cu;
struct subprogram_data * subp;
char * srcfile;
int srcline_nr;

	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&pc) != GEAR_ERR_NO_ERROR)
		panic("");
	miprintf("HALT_ADDR = 0x%08x,", (int) pc);

	srcfile_get_srcinfo_for_addr(ctx, pc, &cu, &subp,
		&srcfile, &srcline_nr, 0);

	if (cu)
	{
		miprintf("HALT_COMP_UNIT = \"%s\", ", cu->name);
	}
	if (subp)
	{
		miprintf("HALT_SUBPROGRAM = \"%s\", ", subp->name);
	}
	if (srcfile)
	{
		miprintf("HALT_SRCFILE = \"%s\", ", srcfile);
	}
	if (srcline_nr)
		miprintf("HALT_SRCLINE_NR = %i, ", srcline_nr);
	/* dump machine interface message footer */
	miprintf("]\n");

}


/*
 *
 * exported functions follow
 *
 */


void exec_target_run(struct gear_engine_context * ctx)
{
struct exec_data_struct * p;

	p = ctx->exec_data;

	if (p->core_state == TARGET_CORE_STATE_DEAD)
		return GEAR_ERR_TARGET_CORE_DEAD;
	if (p->exec_state == EXEC_STATE_FREE_RUNNING && p->core_state == TARGET_CORE_STATE_RUNNING)
		return GEAR_ERR_NO_ERROR;

	/* sanity checks */
	if (p->exec_state != EXEC_STATE_IDLE ||
			p->core_state != TARGET_CORE_STATE_HALTED)
		panic("");
	p->exec_state = EXEC_STATE_FREE_RUNNING;
	if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
		panic("");
}


void exec_target_halt(struct gear_engine_context * ctx)
{
struct exec_data_struct * p;

	p = ctx->exec_data;

	if (p->core_state == TARGET_CORE_STATE_DEAD)
		return GEAR_ERR_TARGET_CORE_DEAD;
	if (p->exec_state == EXEC_STATE_IDLE && p->core_state == TARGET_CORE_STATE_HALTED)
		return GEAR_ERR_NO_ERROR;

	/* sanity checks */
	if (p->exec_state != EXEC_STATE_FREE_RUNNING ||
			p->core_state != TARGET_CORE_STATE_RUNNING)
		panic("");
	if (ctx->cc->core_halt(ctx) != GEAR_ERR_NO_ERROR)
		panic("");
}



/*! \todo	handle recursion */
static void exec_target_halted(struct gear_engine_context * ctx)
{
	struct exec_data_struct * p;
	ARM_CORE_WORD halt_addr;
	ARM_CORE_WORD pc;
	bool src_start;

	/*! \warning hack hack hack
	 *
	 * \todo	this is a very ugly and dangerous hack */
	int must_run_target = 0;
	/*! \warning end of hack end of hack end of hack */

	p = ctx->exec_data;
	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&halt_addr) != GEAR_ERR_NO_ERROR)
		panic("");
	gprintf("target stopped at address 0x%08x\n", (unsigned int) halt_addr);
	if (!p->flags.must_step_mode_be_terminated)
	{
		gprintf("expected step over halt address 0x%08x\n", p->expected_step_over_halt_addr);
		gprintf("expected single step halt address 0x%08x\n", p->expected_single_step_halt_addr);
	}

	/* first, see if the stepping mode should be aborted */
	if (p->flags.must_step_mode_be_terminated)
	{
		if (p->core_state != TARGET_CORE_STATE_HALTED)
			panic("");
		p->exec_state = EXEC_STATE_IDLE;

		miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
		dump_halt_context(ctx, halt_addr);
	}
	/* otherwise - the stepping mode should not be aborted - proceed
	 * the usual way */
	else switch (p->exec_state)
	{
		case EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP:
			if (p->core_state != TARGET_CORE_STATE_HALTED)
				panic("");
			p->exec_state = EXEC_STATE_IDLE;

			if (halt_addr != p->expected_single_step_halt_addr)
			{
				/* the expected stepping into/over halt address
				 * does not match the current halt address -
				 * that is, execution has been interrupted bacause of
				 * some reason other than hitting the step
				 * into/over breakpoint (e.g., hitting a user breakpoint
				 * or watchpoint) - clean up, and report
				 * that the target has been halted */
				if (!bkpt_locate_at_addr(ctx, halt_addr))
					panic("");
			}
			if (bkpt_clear_at_addr(ctx, p->expected_single_step_halt_addr) != GEAR_ERR_NO_ERROR)
				panic("");

			miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
			dump_halt_context(ctx, halt_addr);
			break;

		case EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP_SRC:
			if (p->core_state != TARGET_CORE_STATE_HALTED)
				panic("");
			p->exec_state = EXEC_STATE_IDLE;

			if (halt_addr != p->expected_single_step_halt_addr)
			{
				/* the expected stepping into/over halt address
				 * does not match the current halt address -
				 * that is, execution has been interrupted bacause of
				 * some reason other than hitting the step
				 * into/over breakpoint (e.g., hitting a user breakpoint
				 * or watchpoint) - clean up, and report
				 * that the target has been halted */
				if (!bkpt_locate_at_addr(ctx, halt_addr))
					panic("");
				if (bkpt_clear_at_addr(ctx, p->expected_single_step_halt_addr) != GEAR_ERR_NO_ERROR)
					panic("");
				miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
				dump_halt_context(ctx, halt_addr);
				break;

			}
			/* first, detect if the breakpoint hit is in a
			 * recursive subroutine invocation, and if so -
			 * ignore the breakpoint hit this time */
			if (p->flags.is_stepping_over)
			{
				ARM_CORE_WORD	stack_ptr;
				/* inspect the stack pointer value */
				/*! \todo	handle mode correctly */
				if (ctx->cc->core_reg_read(ctx,
							0,
							1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
							&stack_ptr) != GEAR_ERR_NO_ERROR)
					panic("");
				if (stack_ptr != p->expected_stack_ptr_for_recursion)
				{
					/* this looks line a recursive call -
					 * ignore it and allow the target
					 * to free run some more */
					p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP_SRC;
					if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
						panic("");
/*! \warning hack hack hack
 *
 * \todo	this is a very ugly and dangerous hack */
must_run_target = 1;
/*! \warning end of hack end of hack end of hack */
					break;
				}
			}
			if (bkpt_clear_at_addr(ctx, halt_addr) != GEAR_ERR_NO_ERROR)
				panic("");
			/* just see if we have fallen on an address that
			 * looks like the exact beginning of a source code
			 * construct and if so - terminate the source step
			 * into; otherwise - continue stepping */
			srcfile_get_srcinfo_for_addr(ctx, halt_addr, 0, 0, 0, 0, &src_start);
			if (src_start)
			{
				/* stop stepping */
				miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
				dump_halt_context(ctx, halt_addr);
			}
			else
			{
				/* continue stepping */
				exec_single_step_src(ctx);
/*! \warning hack hack hack
 *
 * \todo	this is a very ugly and dangerous hack */
must_run_target = 1;
/*! \warning end of hack end of hack end of hack */
			}
			break;

		case EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER:
			/* very similar to the EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP
			 * case */
			if (p->core_state != TARGET_CORE_STATE_HALTED)
				panic("");
			p->exec_state = EXEC_STATE_IDLE;

			if (halt_addr != p->expected_step_over_halt_addr)
			{
				/* the expected stepping into/over halt address
				 * does not match the current halt address -
				 * that is, execution has been interrupted bacause of
				 * some reason other than hitting the step
				 * into/over breakpoint (e.g., hitting a user breakpoint
				 * or watchpoint) - clean up, and report
				 * that the target has been halted */
				if (!bkpt_locate_at_addr(ctx, halt_addr))
					panic("");
				if (bkpt_clear_at_addr(ctx, p->expected_step_over_halt_addr) != GEAR_ERR_NO_ERROR)
					panic("");
				miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
				dump_halt_context(ctx, halt_addr);
				break;

			}
			/* detect if the breakpoint hit is in a
			 * recursive subroutine invocation, and if so -
			 * ignore the breakpoint hit this time */
			if (p->flags.is_stepping_over)
			{
				ARM_CORE_WORD	stack_ptr;
				/* inspect the stack pointer value */
				/*! \todo	handle mode correctly */
				if (ctx->cc->core_reg_read(ctx,
							0,
							1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
							&stack_ptr) != GEAR_ERR_NO_ERROR)
					panic("");
				if (stack_ptr != p->expected_stack_ptr_for_recursion)
				{
					/* this looks like a recursive call -
					 * ignore it and allow the target
					 * to free run some more */
					p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER;
					if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
						panic("");
/*! \warning hack hack hack
 *
 * \todo	this is a very ugly and dangerous hack */
must_run_target = 1;
/*! \warning end of hack end of hack end of hack */
					break;
				}
			}
			if (bkpt_clear_at_addr(ctx, halt_addr) != GEAR_ERR_NO_ERROR)
				panic("");
			miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
			dump_halt_context(ctx, halt_addr);
			break;
		case EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER_SRC:
			/* very similar to the EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP
			 * case */
			if (p->core_state != TARGET_CORE_STATE_HALTED)
				panic("");
			p->exec_state = EXEC_STATE_IDLE;

			if (halt_addr != p->expected_step_over_halt_addr)
			{
				/* the expected stepping into/over halt address
				 * does not match the current halt address -
				 * that is, execution has been interrupted because of
				 * some reason other than hitting the step
				 * into/over breakpoint (e.g., hitting a user breakpoint
				 * or watchpoint) - clean up, and report
				 * that the target has been halted */
				if (!bkpt_locate_at_addr(ctx, halt_addr))
					panic("");
				if (bkpt_clear_at_addr(ctx, p->expected_step_over_halt_addr) != GEAR_ERR_NO_ERROR)
					panic("");
				miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
				dump_halt_context(ctx, halt_addr);
				break;

			}
			/* first, detect if the breakpoint hit is in a
			 * recursive subroutine invocation, and if so -
			 * ignore the breakpoint hit this time */
			if (p->flags.is_stepping_over)
			{
				ARM_CORE_WORD	stack_ptr;
				/* inspect the stack pointer value */
				if (ctx->cc->core_reg_read(ctx,
							0,
							1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
							&stack_ptr) != GEAR_ERR_NO_ERROR)
					panic("");
				/*! \todo	this line used to be:

				if (stack_ptr != p->expected_stack_ptr_for_recursion)

				 *		this, however, does not work for
				 *		alien calling conventions (e.g. pascal)...
				 *		fix this... */
				if (stack_ptr < p->expected_stack_ptr_for_recursion)
				{
					/* this looks line a recursive call -
					 * ignore it and allow the target
					 * to free run some more */
					p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER_SRC;
					if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
						panic("");
/*! \warning hack hack hack
 *
 * \todo	this is a very ugly and dangerous hack */
must_run_target = 1;
/*! \warning end of hack end of hack end of hack */
					break;
				}
			}
			if (bkpt_clear_at_addr(ctx, halt_addr) != GEAR_ERR_NO_ERROR)
				panic("");
			/* just see if we have fallen on an address that
			 * looks like the exact beginning of a source code
			 * construct and if so - terminate the source step
			 * into; otherwise - continue stepping */
			srcfile_get_srcinfo_for_addr(ctx, halt_addr, 0, 0, 0, 0, &src_start);
			if (src_start)
			{
				/* stop stepping */
				miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
				dump_halt_context(ctx, halt_addr);
			}
			else
			{
				/* continue stepping */
				exec_step_over_src(ctx);
/*! \warning hack hack hack
 *
 * \todo	this is a very ugly and dangerous hack */
must_run_target = 1;
/*! \warning end of hack end of hack end of hack */
			}
			break;
		case EXEC_STATE_FREE_RUNNING:
			/*! \todo	properly determine and handle
			 *		target stop reason here */
			if (p->core_state != TARGET_CORE_STATE_HALTED)
				panic("");
			/*! \todo	this below is incorrect */
{
struct bkpt_struct * b;
if (!(b = bkpt_locate_at_addr(ctx, halt_addr)))
	panic("");
}
			miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = STOPPED, ");
			dump_halt_context(ctx, halt_addr);
			p->exec_state = EXEC_STATE_IDLE;
			break;
		default:
			panic("");
	}
/*! \warning hack hack hack
 *
 * \todo	this is a very ugly and dangerous hack */
if (must_run_target)
{
target_comm_issue_core_status_request(ctx);
{
enum TARGET_CORE_STATE_ENUM unused;
if (ctx->cc->core_get_status(ctx, &unused)
		!= GEAR_ERR_NO_ERROR)
	panic("");
}

}
/*! \warning end of hack end of hack end of hack */
}

static void exec_update_target_state(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
{
	/* if no transition to a new state is in effect, do nothing */
	if (ctx->exec_data->core_state == state)
		return;
	/* process core state transitions */
	switch (ctx->exec_data->core_state)
	{
		case TARGET_CORE_STATE_DEAD:
			switch (state)
			{
				case TARGET_CORE_STATE_HALTED:
					if (ctx->exec_data->exec_state
							!= EXEC_STATE_IDLE)
						panic("");
					ctx->exec_data->core_state = TARGET_CORE_STATE_HALTED;
					break;
				case TARGET_CORE_STATE_RUNNING:
					if (ctx->exec_data->exec_state
							!= EXEC_STATE_IDLE)
						panic("");
					ctx->exec_data->exec_state = EXEC_STATE_FREE_RUNNING;
					ctx->exec_data->core_state = TARGET_CORE_STATE_RUNNING;
					break;
				default:
					panic("");
			}
			break;
		case TARGET_CORE_STATE_HALTED:
			switch (state)
			{
				case TARGET_CORE_STATE_RUNNING:
					if (ctx->exec_data->exec_state
							== EXEC_STATE_IDLE)
						panic("");
					ctx->exec_data->core_state = state;

					miprintf("TARGET_STATE_CHANGE,[NEW_TARGET_STATE = RUNNING, ]\n");
					break;
				default:
					panic("");
			}
			break;
		case TARGET_CORE_STATE_RUNNING:
			switch (state)
			{
				case TARGET_CORE_STATE_HALTED:
					if (ctx->exec_data->exec_state
							== EXEC_STATE_IDLE)
						panic("");
					ctx->exec_data->core_state = state;
					exec_target_halted(ctx);
					break;
				default:
					panic("");
			}
			break;
		default:
			panic("");
	}
}

/*! \todo	document this */
enum TARGET_CORE_STATE_ENUM exec_get_target_state(struct gear_engine_context * ctx)
{
	return ctx->exec_data->core_state;
}

/*!
 *	void exec_single_step_insn(struct gear_engine_context * ctx)
 *	\brief	perform target single instruction stepping
 *
 *	this is the basic algorithm of this routine:
 *	this function currently uses machine code decoders to determine
 *	the program counter value after the execution of the current
 *	instruction; the instruction at this address (the program counter
 *	value after execution of the current instruction) is breakpointed,
 *	the executor state variables are properly updated, then the
 *	target is requested to run; when the target is halted, the executor
 *	proper determines if the single instruction stepping has been
 *	successful or not and reacts accordingly
 *
 *	the special case in which the program counter after an instruction
 *	execution remains unchanged is not handled here; a most obvious
 *	example of such an instruction is an instruction branching to
 *	itself, but other instructions are also possible; arm axd calls
 *	these 'singular' instructions and refuses to execute such
 *	instructions; the catch here is that in this
 *	case the target is requested to execute an instruction, but
 *	this instruction is breakpointed - the net effect being that
 *	no instruction is executed at all and the target halts immediately;
 *	this can cause various anomalies of many kinds; an (somewhat
 *	far-fetched) example of such an anomaly is that if there is
 *	some counter in the system that counts the number of instructions
 *	executed, it will get out of sync with what would be technically more
 *	correct if the instructions actually got executed
 *	this may not be a great of a deal, but it is maybe technically
 *	cleaner to also moan on such instructions, or at least inform
 *	the user about them
 *
 *	\todo	decide and properly handle singular instructions
 *	\todo	define error codes here
 *
 *	\param	ctx	context to work into
 *	\return	none
 */
void exec_single_step_insn(struct gear_engine_context * ctx)
{
ARM_CORE_WORD break_pc;
struct exec_data_struct * p;
ARM_CORE_WORD current_pc;

	p = ctx->exec_data;

	if (ctx->cc->core_insn_step)
	{
		/* target supports single instruction stepping
		 * natively - use that */
		p->flags.must_step_mode_be_terminated = 1;
		p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP;
		if (ctx->cc->core_insn_step(ctx) != GEAR_ERR_NO_ERROR)
			panic("");
	}
	else
	{
		/* target does not support single instruction stepping
		 * natively, try to decode the current instruction and
		 * make out which the next one should be */
		if (ctx->cc->core_reg_read(ctx,
					0,
					1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
					&current_pc) != GEAR_ERR_NO_ERROR)
			panic("");

		/* sanity checks */
		if (p->exec_state != EXEC_STATE_IDLE ||
				p->core_state != TARGET_CORE_STATE_HALTED)
			panic("");
		if (!ctx->tdesc->decode_insn(ctx, current_pc, &break_pc, 0))
			panic("");
		printf("%s(): expected halt addr: 0x%08x\n", __func__, break_pc);
		if (!bkpt_setup_at_addr(ctx, break_pc))
		{
			/* see if there already is a breakpoint at
			 * the expected stepping halt address */
			if (!bkpt_locate_at_addr(ctx, break_pc))
				panic("");
			/* there already is a breakpoint at the expected
			 * stepping halt address - dont do anything more; when
			 * the target halts (if it halts, that is), the stepping
			 * must get terminated (in all cases - no matter what
			 * the halt address), and it should be reported
			 * that the target has stopped */
			p->flags.must_step_mode_be_terminated = 1;
		}
		else
		{
			p->expected_single_step_halt_addr = break_pc;
			p->flags.must_step_mode_be_terminated = 0;
		}

		p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP;
		p->flags.is_stepping_over = 0;
		if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
			panic("");
	}
}

/*! \todo	document this */
/*!
 *	\fn	void exec_single_step_src(struct gear_engine_context * ctx)
 *	\brief	perform target single stepping at source code level
 *
 *	this is the basic algorithm of this routine:
 *	there are two parts of this algorithm; the first part is within
 *	this function, the second part is within the executor proper
 *	(exec_target_halted()); first, it is made sure that
 *	instruction level single stepping is
 *	initiated here 
 *
 *	\todo	define error codes here
 *
 *	\param	ctx	context to work into
 *	\return	none
 */
void exec_single_step_src(struct gear_engine_context * ctx)
{
ARM_CORE_WORD break_pc;
ARM_CORE_WORD current_pc;
struct exec_data_struct * p;
char * srcfile;
int srcline;
bool is_probably_call_insn;
int insn_len;

	p = ctx->exec_data;
	/* sanity checks */
	if (p->exec_state != EXEC_STATE_IDLE ||
			p->core_state != TARGET_CORE_STATE_HALTED)
		panic("");
	/* first, make sure there indeed *is* line number information
	 * for the code residing at the current target program counter
	 * address */
	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&current_pc) != GEAR_ERR_NO_ERROR)
		panic("");

	srcfile_get_srcinfo_for_addr(ctx, current_pc, 0, 0, &srcfile,
			&srcline, 0);
	if (!srcfile || !srcline)
		panic("");

	/* see if execution of the current instruction would lead
	 * to entering code for which no debugging information is
         * available; under normal circumstances, given that there
	 * is line number information for code residing at the
	 * current program counter address, i(sgs) can think of just
	 * two cases in which it is possible that there is no line
	 * number information for code residing at the new program
	 * counter address, namely:
	 *	- a subroutine call is performed to a routine for
	 *	which line number information is unavailable
	 *	- a return to a caller routine for which line number
	 *	information is unavailable has been performed
	 *
	 * the first case is handled by performing a step over calls
	 * on the routine that is lacking line number informaion instead
	 * of the usual step into; the second case is currently not
	 * handled
	 *
	 *	\todo	the second case is not handled right now;
	 *		decide how it is most appropriate to handle
	 *		it and update the comments above
	 */
	insn_len = ctx->tdesc->decode_insn(ctx, current_pc, &break_pc, &is_probably_call_insn);
	if (!insn_len)
		panic("");
	srcfile_get_srcinfo_for_addr(ctx, break_pc, 0, 0, &srcfile,
			&srcline, 0);
	/* by default, do not use step over calls mode */
	p->flags.is_stepping_over = 0;
	/* see if line number information is available for code at the
	 * new program counter address */
	if (srcfile && srcline)
	{
		/* source line number information available - proceed
		 * the usual way */  
		if (!bkpt_setup_at_addr(ctx, break_pc))
		{
			/* see if there already is a breakpoint at
			 * the expected stepping halt address */
			if (!bkpt_locate_at_addr(ctx, break_pc))
				panic("");
			/* there already is a breakpoint at the expected
			 * stepping halt address - dont do anything more; when
			 * the target halts (if it halts, that is), the stepping
			 * must get terminated (in all cases - no matter what
			 * the halt address), and it should be reported
			 * that the target has stopped */
			p->flags.must_step_mode_be_terminated = 1;
		}
		else
		{
			p->expected_single_step_halt_addr = break_pc;
			p->flags.must_step_mode_be_terminated = 0;
		}
		p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP_SRC;
		if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
			panic("");
	}
	else
	{
		/* source line number information unavailable - take
		 * countermeasures */
		/* determine where the new instruction, for which no
		 * line number information is available, resides -
	         * in the caller of the current routine, or in a
		 * routine called by the current routine */	 
		if (is_probably_call_insn)
		{
			/* assume a routine for which no line number
			 * information is available is to be invoked,
			 * perform a step over calls invocation of this
			 * routine */  
			if (!bkpt_setup_at_addr(ctx, current_pc + insn_len))
			{
				/* see if there already is a breakpoint at
				 * the expected stepping halt address */
				if (!bkpt_locate_at_addr(ctx, current_pc + insn_len))
					panic("");
				/* there already is a breakpoint at the expected
				 * stepping halt address - dont do anything more; when
				 * the target halts (if it halts, that is), the stepping
				 * must get terminated (in all cases - no matter what
				 * the halt address), and it should be reported
				 * that the target has stopped */
				p->flags.must_step_mode_be_terminated = 1;
			}
			else
			{
				/* advance to next instruction */
				p->expected_single_step_halt_addr = current_pc + insn_len;
				p->flags.must_step_mode_be_terminated = 0;
			}
			/* take measures in case of a recursion */
			p->flags.is_stepping_over = 1;
			/*! \todo	handle mode correctly */
			if (ctx->cc->core_reg_read(ctx,
						0,
						1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
						&p->expected_stack_ptr_for_recursion) !=
					GEAR_ERR_NO_ERROR)
				panic("");

			p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_SINGLE_STEP_SRC;
			if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
				panic("");
		}
		else /* if returning_to_caller_with_no_debug_information */
		{
			/* assume a return to a caller function with
			 * no line number information is about to take
		         * place */	 
			/*! \todo	but make sure this is indeed
			 *		the case and react accordingly;
			 *		right now i(sgs) am not sure
			 *		exactly what to do in this case */ 
			panic("");
		}
	}
}

/* this is very similar to exec_single_step_insn() */
/*! \todo	handle recursion */
void exec_step_over_insn(struct gear_engine_context * ctx)
{
ARM_CORE_WORD break_pc;
struct exec_data_struct * p;
bool is_probably_call_insn;
int insn_len;
ARM_CORE_WORD current_pc;

	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&current_pc) != GEAR_ERR_NO_ERROR)
		panic("");

	p = ctx->exec_data;

	/* sanity checks */
	if (p->exec_state != EXEC_STATE_IDLE ||
			p->core_state != TARGET_CORE_STATE_HALTED)
		panic("");
	insn_len = ctx->tdesc->decode_insn(ctx, current_pc, &break_pc, &is_probably_call_insn);
	if (!insn_len)
		panic("");
	/* by default, no recursion is expected */
	p->flags.is_stepping_over = 0;
	/* if this looks like a call instruction (sequence), break
	 * at the next instruction - and also take measures in case
	 * of recursion */
	if (is_probably_call_insn)
	{
		p->flags.is_stepping_over = 1;
		/* save stack pointer to help detect recursion */
		/*! \todo	handle mode correctly */
		if (ctx->cc->core_reg_read(ctx,
					0,
					1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
					&p->expected_stack_ptr_for_recursion) !=
				GEAR_ERR_NO_ERROR)
			panic("");

		/*! \todo	handle mode correctly */
		if (ctx->cc->core_reg_read(ctx,
					0,
					1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
					&break_pc) != GEAR_ERR_NO_ERROR)
			panic("");
		/* stop at the instruction after the call instruction */
		break_pc += insn_len;
	}
	if (!bkpt_setup_at_addr(ctx, break_pc))
	{
		/* see if there already is a breakpoint at
		 * the expected stepping halt address */
		if (!bkpt_locate_at_addr(ctx, break_pc))
			panic("");
		/* there already is a breakpoint at the expected
		 * stepping halt address - dont do anything more; when
		 * the target halts (if it halts, that is), the stepping
		 * must get terminated (in all cases - no matter what
		 * the halt address), and it should be reported
		 * that the target has stopped */
		p->flags.must_step_mode_be_terminated = 1;
	}
	else
	{
		p->expected_step_over_halt_addr = break_pc;
		p->flags.must_step_mode_be_terminated = 0;
	}

	p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER;
	if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
		panic("");
}

/*! \todo	handle recursion */
void exec_step_over_src(struct gear_engine_context * ctx)
{
ARM_CORE_WORD break_pc;
ARM_CORE_WORD current_pc;
struct exec_data_struct * p;
char * srcfile;
int srcline;
bool is_probably_call_insn;
int insn_len;

	p = ctx->exec_data;
	/* sanity checks */
	if (p->exec_state != EXEC_STATE_IDLE ||
			p->core_state != TARGET_CORE_STATE_HALTED)
		panic("");
	/* first, make sure there indeed *is* line number information
	 * for the code residing at the current target program counter
	 * address */
	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&current_pc) != GEAR_ERR_NO_ERROR)
		panic("");

	srcfile_get_srcinfo_for_addr(ctx, current_pc, 0, 0, &srcfile,
			&srcline, 0);
	if (!srcfile || !srcline)
		panic("");

	/* see the comments about line number information in the
	 * exec_single_step_src() function */
	insn_len = ctx->tdesc->decode_insn(ctx, current_pc, &break_pc, &is_probably_call_insn);
	if (!insn_len)
		panic("");
	srcfile_get_srcinfo_for_addr(ctx, break_pc, 0, 0, &srcfile,
			&srcline, 0);
	/* by default, do not use step over calls mode */
	p->flags.is_stepping_over = 0;
	/* see if line number information is available for code at the
	 * conceived new program counter */  
	if ((srcfile && srcline) || is_probably_call_insn)
	{
		/* source line number information available - proceed
		 * the usual way */  
		if (is_probably_call_insn)
		{
			/* advance to next instruction */
			break_pc = current_pc + insn_len;
			/* take measures in case of a recursion */
			p->flags.is_stepping_over = 1;
			/*! \todo	handle mode correctly */
			if (ctx->cc->core_reg_read(ctx,
						0,
						1 << ctx->tdesc->get_target_sp_reg_nr(ctx),
						&p->expected_stack_ptr_for_recursion) !=
					GEAR_ERR_NO_ERROR)
				panic("");
		}
		if (!bkpt_setup_at_addr(ctx, break_pc))
		{
			/* see if there already is a breakpoint at
			 * the expected stepping halt address */
			if (!bkpt_locate_at_addr(ctx, break_pc))
				panic("");
			/* there already is a breakpoint at the expected
			 * stepping halt address - dont do anything more; when
			 * the target halts (if it halts, that is), the stepping
			 * must get terminated (in all cases - no matter what
			 * the halt address), and it should be reported
			 * that the target has stopped */
			p->flags.must_step_mode_be_terminated = 1;
		}
		else
		{
			p->expected_step_over_halt_addr = break_pc;
			p->flags.must_step_mode_be_terminated = 0;
		}
		p->exec_state = EXEC_STATE_WAITING_FOR_END_OF_STEP_OVER_SRC;
		if (ctx->cc->core_run(ctx) != GEAR_ERR_NO_ERROR)
			panic("");
	}
	else
	{
		/* source line number information unavailable - take
		 * countermeasures */
		/* determine where the new instruction, for which no
		 * line number information is available, resides -
	         * in the caller of the current routine, or in a
		 * routine called by the current routine */	 
		if (is_probably_call_insn)
		{
			/* impossible */
			panic("");
		}
		else
		{
			/* if returning_to_caller_with_no_debug_information */
			/* assume a return to a caller function with
			 * no line number information is about to take
		         * place */	 
			/*! \todo	but make sure this is indeed
			 *		the case and react accordingly;
			 *		right now i(sgs) am not sure
			 *		exactly what to do in this case */ 
			gprintf("no line number information for core address 0x%08x\n", break_pc);
			panic("");
		}
	}
}

/*! \todo	document this */
void init_exec(struct gear_engine_context * ctx)
{

	if (!(ctx->exec_data = calloc(1, sizeof(struct exec_data_struct))))
		panic("");
	/* install the executor target state change callback */
	if (ctx->cc->core_register_target_state_change_callback(ctx, 
			exec_update_target_state) != GEAR_ERR_NO_ERROR)
		panic("");
	/*! \todo	fix this - initialize this field properly */
	ctx->exec_data->exec_state = EXEC_STATE_IDLE;
	ctx->exec_data->core_state = TARGET_CORE_STATE_DEAD;
}

