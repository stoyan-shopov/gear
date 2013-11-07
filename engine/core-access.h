/*!
 *	\file	core-access.h
 *	\brief	target core access header file
 *	\author	shopov
 *
 *	\todo	document module doctrine
 *	\todo	cleanup the names inherited from the armulator sources
 *
 *	read the comments at the beginning of file exec.c;
 *	amongst other things, there it is described why and when
 *	the target state change notification callbacks are
 *	invoked
 *	as stated there:
 *			as an implementaion detail, it is notable that
 *			all the points where the transitions between
 *			the target core connection states are possible to occur,
 *			physically reside within module target-comm.c,
 *			and at all such points a target state change
 *			notification callback function (which, most
 *			commonly, is function exec_update_target_state()
 *			from this module) is invoked; also see comments
 *			about the core_register_target_state_change_callback
 *			data field in structure core_control in file
 *			core-access.h
 *	for further details, read the comments in the beginnig
 *	of file exec.c and the documentation about enum TARGET_CORE_STATE_ENUM
 *	in file target-state.h
 *
 *	\note	in order to minimize perplexity of someone hacking
 *		the gear, all modules, and all callback
 *		functions contained in these modules (along with
 *		a brief description of their purpose), that are
 *		present in the gear engine, *MUST* be listed here;
 *		it is deemed a fatal error for a target state
 *		change callback function to exist and not to be
 *		listed here; the date reference below must
 *		be also brought up to date whenever a modification
 *		to the list is made
 *
 *		to day (17032009) the following modules in the
 *		gear engine, along with the target state change
 *		notification callback functions within them, are present:
 *		- module exec.c, function exec_update_target_state(); this
 *			is the primary callback used to control target
 *			execution operations (stepping, running, etc.)
 *		- module target/flash-loader/target-img-load.c,
 *			function flasher_target_state_change_callback();
 *			a callback used for controlling the target execution
 *			when populating target flash memory areas
 *		- module frame-reg-cache.c, function 
 *			frame_reg_cache_target_state_change_callback();
 *			used for maintaining the target frame/register
 *			cache and providing transparent access to
 *			target registers when crawling the target call stack
 *		- module arch-arm-target.c,
 *			function target_run_state_change_callback();
 *			purpose - used for stepping over breakpointed
 *			instructions for arm targets that do not support
 *			this natively (in hardware)
 *
 *	\todo	somehow enforce the requirements above
 *
 *	\note	basically, all of the function pointers in the core_control
 *		function table declared below, are the ones contained in
 *		module target-comm.c; however, some modules may override
 *		specific function pointers
 *		by installing their own overriding functions (hooks); analogously
 *		to the notes above about the target state change callbacks,
 *		all modules, and all overriding
 *		functions contained within these modules (along
 *		with a brief description of their purpose), that are
 *		present in the gear engine, *MUST* be listed here;
 *		it is deemed a fatal error for an overriding function
 *		to exist and not to be listed here; the date reference below must
 *		be also brought up to date whenever a modification
 *		to the list is made
 *
 *		to day (17032009) the following modules in the
 *		gear engine, along with the overriding functions
 *		contained within them, are present:
 *		- module frame-reg-cache.c, functions:
 *			reg_cache_core_reg_read, overrides function
 *			core_reg_read; 
 *			function reg_cache_core_reg_write, overrides
 *			function core_reg_write;
 *			purpose - provide
 *			transparent access to target registers by
 *			taking in account which is the currently active
 *			(selected) target call stack frame, and
 *			unwinding register values when necessary
 *		- module arch-arm-target.c,
 *			function target_run_over_breakpoints();
 *			purpose - used for stepping over breakpointed
 *			instructions for arm targets that do not support
 *			this natively (in hardware)
 *
 *
 *	\todo	somehow enforce the requirements above
 *
 *	Revision summary:
 *
 *	$Log: $
 */
#ifndef __CORE_ACCESS_H__
#define __CORE_ACCESS_H__

/*
 *
 * include section follows
 *
 */

#include <stdbool.h>

/* the target-state header contains the definition of
 * the TARGET_CORE_STATE_ENUM enumeration; it is in a
 * separate header file because it is also needed by
 * the target controller code */
#include "target-state.h"

/*
 *
 * exported data types follow
 *
 */

/*! core control data structure */
struct core_control
{
	/*! returns true if a connection to a target core controller is currently established, false otherwise */
	bool (*is_connected)(struct gear_engine_context * ctx);
	/*! establishes a connection to a target core
	 *
	 * \todo	make references to the target state change documentation;
	 *		this is already getting vile... 
	 */
	enum GEAR_ENGINE_ERR_ENUM (*core_open)(struct gear_engine_context * ctx, const char ** errmsg_hint);
	/*! shuts down access to a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_close)(void);
	/*! reads memory in a target system */
	enum GEAR_ENGINE_ERR_ENUM (*core_mem_read)(struct gear_engine_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes);
	/*! writes memory in a target system */
	enum GEAR_ENGINE_ERR_ENUM (*core_mem_write)(struct gear_engine_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes);
	/*! reads register(s) from a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_reg_read)(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! writes register(s) in a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_reg_write)(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! reads register(s) from a coprocessor */
	enum GEAR_ENGINE_ERR_ENUM (*core_cop_read)(unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! writes register(s) to a coprocessor */
	enum GEAR_ENGINE_ERR_ENUM (*core_cop_write)(unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! sets a breakpoint
	 *
	 * \deprecated	this must be deprecated */
	enum GEAR_ENGINE_ERR_ENUM (*core_set_break)(struct gear_engine_context * ctx, ARM_CORE_WORD address);
	/*! clears a breakpoint
	 *
	 * \deprecated	this must be deprecated */
	enum GEAR_ENGINE_ERR_ENUM (*core_clear_break)(struct gear_engine_context * ctx, ARM_CORE_WORD address);
	/*! runs a core until a breakpoint has been hit or until other halting condition comes in effect
	 *
	 * \todo	document what other conditions abort the 'run' mode */
	enum GEAR_ENGINE_ERR_ENUM (*core_run)(struct gear_engine_context * ctx/*ARM_CORE_WORD * halt_addr*/);
	/*! halts a target core if it is running */
	enum GEAR_ENGINE_ERR_ENUM (*core_halt)(struct gear_engine_context * ctx);
	/*! instruction-wise target single step
	 *
	 * \note	this is not required to be supported by a target, and if it is
	 *		not supported, this field should be null */
	enum GEAR_ENGINE_ERR_ENUM (*core_insn_step)(struct gear_engine_context * ctx);
	/*! input-output target control, semantics of this are target dependent; the beast's mark on the gear
	 *
	 * generally, this is a catch-all for everything that doesnt cleanly
	 * fit the target control interface; one can think of this as a placeholder
	 * for hiding hacks, a bag of tricks, an interface for performing anything
	 * that is not yet polished enough to be put in the gear engine proper,
	 * or simply a (temporary) hack that is to be put somewhere instead of
	 * poluting the gear engine proper
	 *
	 * details about supported input/output control request for specific
	 * targets, target architectures and variants, and target access drivers
	 * must be put in the target specific "target-defs.h" header file residing
	 * in directory "./include/targets/xxx/", where "xxx" is the architecture
	 * name (e.g., "arm")
	 *
	 * please, abstain from using this routine */
	enum GEAR_ENGINE_ERR_ENUM (*io_ctl)(struct gear_engine_context * ctx, int request_len, ARM_CORE_WORD * request, ARM_CORE_WORD response_len, ARM_CORE_WORD * response);
	/*! obtains target core status information */
	enum GEAR_ENGINE_ERR_ENUM (*core_get_status)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM * status);
	/*! registers a callback function to be invoked when a (possible) change of the state of a target core is detected by some of the core-control routines here
	 *
	 * this is mainly needed to inform interested parties about (eventual)
	 * changes in a target state, so that these parties be given a chance
	 * to take appropriate actions they see fit; the registered callbacks
	 * are stored on a stack; first, the callback at the top of the
	 * stack is invoked, if it returns true (nonzero), then the callback
	 * further down the stack is invoked, and so on until either a
	 * callback returns false (zero), or the stack is exhausted
	 *
	 * \todo	this may well change, update the comments properly
	 *		if it does
	 *
	 * \note	this note explains the rationale for maintaining such
	 *		a stack of callbacks; in a typical debugging session,
	 *		most of the time there should probably be just one
	 *		callback in the stack - the exec_update_target_state()
	 *		routine in module exec.c (see the comments about
	 *		this routine for details); however, as is often the
	 *		case with embedded targets, in the process of
	 *		loading an executable image in target core, it is
	 *		often needed some memory areas (e.g. consisting of
	 *		flash memory) to be erased and the exact procedure
	 *		for accomplishing this may be non-trivial; this
	 *		is why the target-img-load.c module has been
	 *		introduced to the gear engine; this module needs
	 *		to perform target operations such as the
	 *		insertion/removal of breakpoints, running/halting
	 *		the target core, etc. (for details, see module
	 *		target-img-load.c); in these cases, the function
	 *		that would normally be notified of target state
	 *		change events, namely, exec_update_target_state(),
	 *		should not really be notified of these events as they
	 *		arise from operations internal to the gear engine
	 *		(namely, originating from actions taken by the
	 *		target-img-load.c module) and not from actions
	 *		initiated by the gear engine debugger user; 
	 *		so, in this case, it would be most natural to,
	 *		say, 'temporarily override' the normal processing
	 *		of target state change events for the purposes
	 *		of the target-img-load.c module; the concept
	 *		of target state change callbacks and the
	 *		stacking of these callbacks as a mechanism of
	 *		callback overriding for the purposes outlined
	 *		above, all seemed to me(sgs) capable of solving
	 *		the problems mentioned without overly complicating
	 *		matters and without being too messy - however,
	 *		if a better solution is proposed, i(sgs) will gladly
	 *		incorporate it in the gear engine
	 *
	 *	\note	all target state change callback function, along
	 *		with their comprising modules, *MUST* be listed
	 *		as comments at the start of this file;
	 *		also read the comments there
	 *
	 *	\todo	the current interface is flawed,
	 *		will most probably be changed
	 */
	enum GEAR_ENGINE_ERR_ENUM (*core_register_target_state_change_callback)(struct gear_engine_context * ctx, 
			bool (*target_state_change_notification_callback)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state));
	/*! the opposite of the core_register_target_state_change_callback above
	 *
	 * this function removes the top-of-stack entry in the target
	 * state change callback stack
	 *
	 *	\todo	the current interface is flawed,
	 *		will most probably be changed
	 */
	enum GEAR_ENGINE_ERR_ENUM (*core_unregister_target_state_change_callback)(struct gear_engine_context * ctx, 
			bool (*target_state_change_notification_callback)(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state));
};

/*
 *
 * exported function prototypes follow
 *
 */
struct core_control * init_core_access(void);

#endif /* __CORE_ACCESS_H__ */

