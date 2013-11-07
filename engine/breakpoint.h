/*!
 *	\file	breakpoint.h
 *	\brief	breakpoint utilities header file
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * exported types follow
 *
 */

/*! basic breakpoint data structure
 * 
 *	\note	this is one of the few exported data structures in the gear
 *		engine, because it is needed to handle breakpoints at several
 *		places throughout the engine
 *
 *	\todo	right now, only one breakpoint can be set up for a given target
 *		core address, tracepoints and watchpoints are not supported at all
 */
struct bkpt_struct
{
	/*! each breakpoint is put in a list, sorted by address (ascending) */
	struct bkpt_struct	* next;
	/*! breakpoint number - most commonly used for easy referencing a breakpoint by the frontend */
	/*! this holds various breakpoint flags */
	struct
	{
		/*! denotes if a breakpoint is currently active
		 *
		 * if nonzero, the breakpoint is active and will be installed
		 * in the target whenever the target is run, when zero - it will
		 * not be installed when the target is being run, and is effectively
		 * ignored */
		int is_active	: 1;
	};
	/*! target core address at which the breakpoint is installed */
	ARM_CORE_WORD	addr;
	/*! an expression to evaluate whenever the breakpoint is hit, in order to decide whether the breakpoint is indeed taken
	 *
	 * if 0, then the breakpoint is always taken, if non-null, then
	 * this expression is evaluated whenever the breakpoint is hit
	 * (the expression must evaluate to a scalar type) - if the
	 * expression evaluates successfully to a scalar that is nonzero,
	 * then the breakpoint is taken (the target is halted), otherwise -
	 * if the expression evaluates to a scalar that is equal to zero,
	 * or does not evaluate to a scalar type, or there is some other
	 * error when evaluating the expression - the breakpoint is not
	 * taken, and the target is allowed to continue execution
	 *
	 * \todo	currently nowhere used */
	const char	* cond_exp;
};

/*
 *
 * exported function declarations follow
 *
 */ 

struct bkpt_struct * bkpt_setup_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
enum GEAR_ENGINE_ERR_ENUM bkpt_clear_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
struct bkpt_struct * bkpt_locate_at_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
void init_bkpt(struct gear_engine_context * ctx);

