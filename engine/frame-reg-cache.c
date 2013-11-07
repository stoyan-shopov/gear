/*!
 * \file	frame-reg-cache.c
 * \brief	target stack frame operations and register cache support
 * \author	shopov
 *
 *	this file contains code for handling target call stack frame
 *	operations (such as stack frame unwinding and stack crawling)
 *	and, support for the so-called 'register cache' (read below
 *	for details)
 *
 *	in the comments within this module, under 'unwinding a stack frame'
 *	it is understood the 'virtual returning' from the subroutine
 *	currenlty active (also known as activation record) - which
 *	includes the restoral of the registers'
 *	values which are callee-saved, and computing the return address
 *	and canonical frame address (cfa - described below) of the
 *	caller subroutine; thus, the unwinding of the stack has
 *	the effect of the removal of the call stack frame
 *	of the subroutine currently active (the callee),
 *	and moving upwards the target call stack into
 *	the frame of the preceding subroutine (the caller)
 *
 *	\note	in the comments in this module, an 'outermost'
 *		frame is understood to denote the least recent
 *		(also, the oldest, the one first created)
 *		frame, and an 'innermost' frame is understood
 *		to denote the most recent (also, the youngest,
 *		the last created) frame
 *
 *	it is important to note that many operations in the gear
 *	engine, such as register reading/writing, identifier
 *	scope resolution, expression evaluation, etc., operate
 *	in the context of a so-called 'currently selected/active stack frame',
 *	which is generally differrent from the context of the
 *	target stack frame of the subroutine in which the target
 *	has been halted
 *
 *	this 'currently selected/active stack frame' is most commonly
 *	being defined by the debugger user when crawling the
 *	target call stack; this naturally calls for unwinding the target
 *	call stack, which in turn changes the context under which
 *	things such as register values, identifier scope
 *	(and subsequently - expression values) and others are defined
 *
 *	the basic effect and goal of unwinding a stack frame is the population
 *	of (an abi defined subset of) the target core register file
 *	(required to be preserved by the callee) with new values
 *	(namely, the values of the registers at the place of the
 *	return from the call to the currently active routine -
 *	that is, in the caller of the current routine, at the instruction that
 *	is to be passed the control to after returning from the call);
 *	in brief, unwinding a stack frame has the effect of terminating
 *	the currently active routine and returning to its caller - as
 *	if a 'return' statement has been executed in the currently
 *	active routine
 *
 *	note that, in order to be able to restore the callee-preserved
 *	registers, a callee must save any such clobbered registers
 *	(usually on the stack, or in unused callee non-preserved
 *	registers)
 *
 *	this means that, when unwinding, the register
 *	contents of a caller routine may have to be fetched from memory
 *	and/or other registers (where the register has been saved
 *	prior to being overwritten); now, suppose that a debugger user
 *	requests to modify a register which is in a frame above the
 *	innermost stack frame (the one of the subroutine that the target
 *	is halted in); if that register has been saved somewhere on the
 *	stack, for example, then what the gear engine must modify is
 *	actually the memory (in the stack) where the register has
 *	been saved, and this should be happenning transparently to
 *	the user - this will have the effect that, when a subroutine
 *	returns to its caller when the target resumes execution,
 *	it will restore the register from
 *	the memory location already modified by the gear engine,
 *	effectively loading the new value desired by the user; things
 *	are similiar if a register has been saved to another register;
 *	in all, the procedures to be performed when unwinding and/or
 *	modifying target context in different call
 *	stack frames, may be non-trivial; thus, it would
 *	be bothersome and awkward for routines such as identifier
 *	scope resolution and expression evaluation to be burdened
 *	with the details of stack unwinding when doing their job
 *	
 *	this module addresses this by introducing a so-called
 *	'target register cache', defined below
 *
 *	as the register file contents are usually sufficient to
 *	determine the context in which most operations such as
 *	identifier scope resolution and expression evaluation
 *	operate, this module serves the purpose
 *	of maintaining an up-to-date register file for each target
 *	stack frame that has been unwound - the collection of these
 *	(register file sets for different call stack frames)
 *	is the so-called 'target register cache'; in this way, the mentioned
 *	operations are being saved the trouble of explicitly requesting
 *	stack frame unwinding, computing which register is saved
 *	where, in order to properly perform target state manipulations
 *	(such as changing a register or a variable upwards the call
 *	stack frame hierarchy), etc.
 *
 *	in brief, this module provides the gear engine with
 *	convenient access to target core registers in different
 *	call stack frame contexts; this is generally transparent
 *	to other modules, because, technically, this is
 *	achieved by intercepting and overriding the calls to
 *	ctx->cc->core_reg_read() and ctx->cc->core_reg_write()
 *	(the target core register access routines in
 *	the current gear engine context); thus, when registers
 *	are read or written, this module properly calculates
 *	and redirects target core accesses to the proper
 *	memory and/or register locations appropriate for the
 *	currently selected target call stack frame
 *
 *	\note	there are a couple of coding details
 *		worth mentioning
 *			- first, the register cache is active (operative)
 *			if, and only if, the target is halted;
 *			the register cache is inoperative whenever
 *			the target is running or is dead; whenever
 *			the register cache is inoperative, requests
 *			to crawl the stack shall fail and
 *			requests to read/write target core registers
 *			shall be passed directly to the overridden
 *			ctx->cc->core_reg_read() and ctx->cc->core_reg_write()
 *			routines; whenever the register cache is
 *			operative, stack crawling requests are
 *			accepted and processed and requests
 *			to read/write target core registers
 *			are properly routed to access appropriate
 *			register contents for the currently selected/active
 *			stack frame; the local functions in this module,
 *			overriding the general target core
 *			register access functions, are
 *			reg_cache_core_reg_read() and
 *			reg_cache_core_reg_write()
 *			- in order to properly determine when
 *			the register cache should be enabled, and
 *			when disabled, this module installs a
 *			target state change callback routine
 *			(by invoking routine
 *			core_register_target_state_change_callback());
 *			whenever the target halts, this module
 *			reads and caches the target core registers,
 *			thus creating the innermost frame of
 *			the target call stack - this initializes
 *			the frame_list pointer field in the
 *			struct frame_data_struct data structure
 *			(detailed below) to a nonzero value;
 *			also, whenever the target resumes execution
 *			or dies, this frame_list is destroyed,
 *			and a null value is written to it;
 *			therefore, whether the frame_list
 *			value is null or not reliably determines
 *			whether the target register cache
 *			should be (respectively) non-operative
 *			or operative
 *
 *	\note	to support the ability to reference a specific
 *		frame on the target call frame stack, a simple
 *		register numbering scheme is introduced; frames
 *		start to be numbered from zero, with frame
 *		zero being the innermost (most recent) frame,
 *		and the frame numbers grow up, one at a time,
 *		for each less recent frame on the stack; negative
 *		frame numbers are invalid, it is expected that
 *		whenever there is no valid active frame selected
 *		(i.e., the selected_frame pointer below is zero),
 *		then the selected_frame_nr field equals -1
 *
 *	\note	for crawling the stack, one frame at a time,
 *		up or down the target call stack, the
 *		frame_move_to_relative() routine is used;
 *		it is also used for rewinding the target call stack -
 *		that is, moving to the innermost (most recent) target
 *		call stack frame
 *
 * 	\todo	properly trigger the updating of things like variable
 *		watches, memory dump views, register views, etc.
 *
 *	\todo	decide how to handle accesses to undefined
 *		register values
 *
 *	\todo	what is referred to as a 'stack frame' in this module
 *		may be a bit messy - clean up the comments once
 *		this module matures
 *	\todo	currently, cpsr/spsr and cpu modes are not handled at all - fix this
 *	\todo	it may become necessary to supply some means
 *		for selecting the base platform application
 *		binary interface (bpabi) and procedure call standard
 *		(pcs) variants to use; currently, supported is only
 *		the combination of, as referred to in various
 *		arm documents - notably, the document that can be found at:
 *		http://infocenter.arm.com/help/topic/com.arm.doc.ihi0036b/IHI0036B_bsabi.pdf
 *		"bare metal / aapcs" base platform abi / procedure
 *		call standard combination
 *
 * Revision summary:
 *
 * $Log: $
 */

/*
 *
 * include section follows
 *
 */

#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "gear-limits.h"
#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "engine-err.h"
#include "core-access.h"
#include "frame-reg-cache.h"
#include "gprintf.h"
#include "util.h"

#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "srcfile.h"

/*
 *
 * local data follows
 *
 */

/*! the data structure holding dwarf frame unwind data for the executable being debugged */
struct frame_data_struct
{
	/*! the array containing cies, returned by dwarf_get_fde_list */
	Dwarf_Cie	* cie_data;
	/*! the number of elements in the ::cie_data array */
	Dwarf_Signed	cie_element_count;
	/*! the array containing fdes, returned by dwarf_get_fde_list */
	Dwarf_Fde	* fde_data;
	/*! the number of elements in the ::fde_data array */
	Dwarf_Signed	fde_element_count;
	/*! the list head of the target call stack frames unwound so far
	 * 
	 * this points to the innermost (most recent) frame; if null,
	 * no frames have been unwound so far - this should be
	 * the case whenever the target is dead or running; it
	 * can never be null when the target is halted
	 *
	 * \note	this is null, if, and only if,
	 * 		the selected_frame field below is null;
	 * 		also, this is null, if, and only if,
	 * 		the target is currently halted
	 *
	 * whether this is null or not serves as an indication
	 * if the target register cache must be (respectively)
	 * non-operative or operative
	 * also see the comments at the start of this file */
	struct frame_reg_struct
	{
		/*! canonical frame address (cfa) for this frame
		 *
		 * the cfa is defined to be (as the dwarf3 standard document says):
		 * "Typically, the CFA is defined to be the value of the stack pointer
		 * at the call site in the previous frame (which may be different
		 * from its value on entry to the current frame)."
		 *
		 * \note	this value is valid for all but the
		 *		outermost frame in the target call stack
		 *		computed (because it is the only one that
		 *		is not unwound) */
		ARM_CORE_WORD cfa;
		/*! return address for the subroutine which owns this frame
		 *
		 * \note	this value is valid for all but the
		 *		outermost frame in the target call stack
		 *		computed (because it is the only one that
		 *		is not unwound) */
		ARM_CORE_WORD ret_pc;
		/*! a pointer to the older (less recent) frame upwards the call stack
		 *
		 * if this is null, then this is the outermost frame (past
		 * which further unwinding of the stack may be not possible,
		 * e.g. when no unwind information is available) */
		struct frame_reg_struct * older;
		/*! a pointer to the younger (more recent) frame downwards the call stack
		 *
		 * if this is null, then this is the innermost frame */
		struct frame_reg_struct * younger;

		/*! this structure denotes, for each register, where it is stored
		 *
		 * the entries in this table are to be considered valid only
		 * if the the reg_content_type field has value ARM_CORE_REG_VALID
		 *
		 * also see the comments about the reg_content_type field below
		 *
		 * \note	this is a, as called in c99, 'flexible'
		 *		array member'; it must go last in the
		 *		frame register structure; the number
		 *		of elements in it equals the number of target
		 *		core registers and can be obtained by
		 *		invoking ctx->tdesc->get_nr_target_core_regs()
		 */
		struct reg_info_struct
		{
			/*! denotes whether a register in this frame contains a valid value (i.e., denotes if the reg_val field below is valid)
			 *
			 * \note	in the following paragraph, referenced
			 *		are the rules for computing the
			 *		unwound value of a register in the target -
			 *		these are detailed in the freely
			 *		available dwarf 3 standard document,
			 *		paragraph 6.4 - 'call frame information';
			 *		also explained there is the notion of
			 *		the 'canonical frame address' (cfa)
			 *
			 * when unwinding the target call stack, the value of
			 * a register in a previous (less recent/older)
			 * frame may be preserved at various places:
			 *	- (1) it may be undefined; this corresponds to the
			 *	'undefined' rule
			 *	- (2) it may assume the value of the same register
			 *	in the current frame (the one unwound) - this
			 *	is the 'same value' rule
			 *	- (3) it may be stored at the address [cfa + n],
			 *	and this is the offset(n) rule
			 *	- (4) it may be the value 'cfa + n', and this is
			 *	rule val_offset(n)
			 *	- (5) it may reside in another register - this is
			 *	rule 'register(r)'
			 *	- (6) it may equal the value of a dwarf expression -
			 *	rule 'val_expression(e)'
			 *	- (7) it may reside at the address, which value is
			 *	given by a dwarf expression - rule 'expression(e)'
			 *	- (8) it may be defined in some special way - rule
			 *	'architectural'
			 *
			 *	under some of these rules, the contents of a register
			 *	may be saved in memory, in another register, or even
			 *	not at all - but even then they may have valid contents;
			 *	these several cases are captured by the following fields (below):
			 *	- reg_content_type[reg_nr]
			 *	- is_reg_addr_applicable
			 *	- is_reg_stored_in_memory
			 *	- reg_addr
			 *
			 *	these fields are now explained
			 *
			 *	if a register is unavailable, its reg_content_type field
			 *	equals ARM_CORE_REG_UNDEFINED, and none of the other fields apply;
			 *	therefore, the reg_val field below does not contain a valid value
			 *	for the register
			 *
			 *	otherwise, the register is available, its reg_content_type
			 *	field equals ARM_CORE_REG_VALID, the reg_val field
			 *	below contains a valid value for the register,
			 *	and two cases are possible: the register is either stored
			 *	somewhere (either in memory, or in another register), or not
			 * 		- if the register is stored somewhere (either in memory, or in
			 *		another register), then its is_reg_addr_applicable field is nonzero,
			 *		and the value of its is_reg_stored_in_memory field denotes
			 *		where the register is stored - and also the meaning of the
			 *		reg_addr field - if is_reg_stored_in_memory is nonzero, then
			 *		the register is stored at the memory location (address) reg_addr,
			 *		otherwise, is_reg_stored_in_memory is zero, and the register is stored
			 *		in the register with number reg_addr
			 *		- if, on the other hand,
			 *		the register is not stored anywhere, then the is_reg_addr_applicable
			 *		field is zero and the is_reg_stored_in_memory and reg_addr
			 *		fields do not apply
			 *
			 *	\note	in brief, if a register is not undefined, then its value is
			 *		always present in the reg_val field, even though
			 *		this value may be stored (either in target memory
			 *		or in a target core register), or not stored
			 *		anywhere in the target
			 *
			 *	the dwarf unwinding rules above can be summarized
			 *	in the following table
			 *	 _______________________________________________________________________________________________________________________________________________________________________________
			 *	| rule			| reg_content_type	| reg_val field valid?	| is_reg_addr_applicable value	| is_reg_stored_in_memory value	| reg_addr meaning			|
			 *	+-----------------------+-----------------------+-----------------------+-------------------------------+-------------------------------+---------------------------------------+
			 *	| undefined		| ARM_CORE_REG_UNDEFINED| no			| n/a				| n/a				| n/a					|
			 *	| same value		| ARM_CORE_REG_VALID	| yes			| true				| false				| register holding the value		|
			 *	| offset(n)		| ARM_CORE_REG_VALID	| yes			| true				| true				| memory address holding the value	|
			 *	| val_offset(n)		| ARM_CORE_REG_VALID	| yes			| false				| n/a				| n/a					|
			 *	| register(n)		| ARM_CORE_REG_VALID	| yes			| true				| false				| register holding the value		|
			 *	| val_expression(e)	| ARM_CORE_REG_VALID	| yes			| false				| false				| n/a					|
			 *	| expression(e)		| ARM_CORE_REG_VALID	| yes			| true				| true				| memory address holding the value	|
			 *	| architectural		| ARM_CORE_REG_UNDEFINED| no			| n/a				| n/a				| n/a					|
			 *	+-----------------------+-----------------------+-----------------------+-------------------------------+-------------------------------+---------------------------------------+
			 */
			enum
			{
				/*! invalid value, used for catching errors */
				ARM_CORE_REG_INVALID = 0,
				/*! the corresponding register contains valid data */
				ARM_CORE_REG_VALID,
				/*! the corresponding register's content is undefined */
				ARM_CORE_REG_UNDEFINED,
			}
			reg_content_type;
			/*! various flags, currently only one defined */
			struct
			{
				/*! denotes if the notion of a register 'address' has meaning for a register
				 *
				 * if zero, the notion of a register 'address' is
				 * devoid of meaning and the is_reg_stored_in_memory
				 * and reg_addr fields below must be ignored;
				 * if, on the other hand, this is nonzero,
				 * then the is_reg_stored_in_memory should
				 * be further inspected to see where the
				 * register's contents reside 
				 *
				 * also see the comments about the reg_content_type field above
				 */
				unsigned is_reg_addr_applicable : 1;
				/*! denotes whether the contents of a register reside in memory, or in another (possibly the same) register (only if the is_reg_addr_applicable flag above is nonzero)
				 *
				 * this flag is only applicable if the
				 * is_reg_addr_applicable flag above is nonzero;
				 * if this is nonzero, the corresponding register's
				 * contents reside in memory, and the value
				 * of the reg_addr field below is the memory
				 * address where the register is stored
				 * on the other hand, if this field is
				 * zero, then the corresponding register
				 * is stored in another register, and
				 * the number of this other register
				 * is given by the value of the reg_addr
				 * field below
				 *
				 * also see the comments about the reg_content_type field above
				 */
				unsigned is_reg_stored_in_memory	: 1;
			};
			/*! the location of the corresponding register's contents
			 *
			 * the meaning of this is determined by the
			 * is_reg_stored_in_memory flag value above - see
			 * the comments about it for details
			 *
			 * also see the comments about the reg_content_type field above
			 */
			ARM_CORE_WORD	reg_addr;
			/*! at last, the target register value itself (for this frame)
			 *
			 * this here is valid only if its corresponding
			 * reg_content_type element equals ARM_CORE_REG_VALID
			 *
			 * also see the comments about the reg_content_type field above
			 */
			ARM_CORE_WORD reg_val;
		}
		reg_info[/*NR_CORE_REGS*/];
	}
	* frame_list;
	/*! a pointer to the currently selected/active target call stack frame
	 *
	 * this is a pointer to some of the entries in the frame_list list
	 * above; if null, no frame is currently active - this should be
	 * the case whenever the target is dead or running; it
	 * can never be null when the target is halted
	 *
	 * \note	this is null, if, and only if,
	 * 		the frame_list field above is null */
	struct frame_reg_struct *	selected_frame;

	/*! the number of the currently selected frame
	 *
	 * see the notes about frame numbering at the start of
	 * this file; when there is no valid active frame,
	 * this should equal -1
	 *
	 * \note	this is negative, if, and only if,
	 * 		the selected_frame field above is null */
	int	selected_frame_nr;

	/*! original value of the ctx->cc->core_reg_read() function pointer
	 *
	 * this is overrriden by the reg_cache_core_reg_read() function within this module */
	enum GEAR_ENGINE_ERR_ENUM (*core_reg_read_prev)(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! original value of the ctx->cc->reg_cache_core_reg_write() function pointer
	 *
	 * this is overrriden by the xxx() function within this module */
	enum GEAR_ENGINE_ERR_ENUM (*core_reg_write_prev)(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[]);
};

/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	static struct frame_reg_struct * get_frame_reg_struct(struct gear_engine_context * ctx)
 *	\brief	allocates and returns a data structure of type frame_reg_struct
 *
 *	\param	ctx	context to work in
 *	\return	the newly allocated struct frame_reg_struct */
static struct frame_reg_struct * get_frame_reg_struct(struct gear_engine_context * ctx)
{
struct frame_reg_struct * p;
int i;

	if (!(i = ctx->tdesc->get_nr_target_core_regs(ctx)))
		panic("");

	i = sizeof(struct frame_reg_struct [i]);

	if (!(p = calloc(1, sizeof * p + i)))
		panic("");
	return p;
}

/*!
 *	\fn	static ARM_CORE_WORD get_reg_checked(struct gear_engine_context * ctx, struct frame_reg_struct * frame, unsigned int reg_nr)
 *	\brief	returns a register value for a given frame, checking that its contents are valid
 *
 *	\param	ctx	context to work in
 *	\param	frame	frame for which to read the register needed
 *	\param	reg_nr	the target core register number to read
 *	\return	the requested register's value, if its value is not undefined
 *
 *	\todo	currently, this function panics if the requested
 *		register's contents are undefined; this is acceptable
 *		as long as this routine is used internally in this
 *		module to access registers for which it is a fatal
 *		error to contain undefined values (e.g. r13, r14, r15);
 *		this, however, may change in the future and this routine
 *		shall have to be fixed */
static ARM_CORE_WORD get_reg_checked(struct gear_engine_context * ctx, struct frame_reg_struct * frame, unsigned int reg_nr)
{
int i;

	printf("%s(): reg_nr == %i\n", __func__, reg_nr);
	if (!(i = ctx->tdesc->get_nr_target_core_regs(ctx)))
		panic("");
	if (reg_nr >= i
			|| frame->reg_info[reg_nr].reg_content_type != ARM_CORE_REG_VALID)
	{
		panic("");
	}
	return frame->reg_info[reg_nr].reg_val;
}

/*!
 *	\fn	static int dwarf_frame_unwind(struct gear_engine_context * ctx, struct frame_reg_struct * cur_frame)
 *	\brief	unwinds a target call stack frame
 *
 *	see the comments about reg_content_type above
 *
 *	this routine unwinds a target call stack frame and builds the
 *	stack frame of the subroutine that invoked the routine that
 *	owns the passed-in stack frame (builds the next-inner stack frame)
 *
 * 	\note	this function expects that the stack frame passed has not yet
 * 		been unwound, that is - its previous pointer is null
 *
 *	\param	ctx	context to work into
 *	\param	cur_frame	the frame to unwind; expected is that its
 *				'younger' pointer is null
 *	\return	on success, puts the pointer to the frame of the routine
 *		preceding the current one in the target call stack in the
 *		'younger' pointer of the 'cur_frame' frame passed, and returns
 *		GEAR_ERR_NO_ERROR; on failure (because there is no unwind
 *		information available, or some error when accessing the
 *		target occurred), returns GEAR_ERR_CANT_UNWIND_STACK_FRAME
 *
 *	\todo	maybe define more specific error indications for stack unwinding 
 */

static int dwarf_frame_unwind(struct gear_engine_context * ctx, struct frame_reg_struct * cur_frame)
{
/* these variables are unused */
Dwarf_Addr unused0, unused1;
Dwarf_Unsigned bytes_in_cie_unused;
Dwarf_Error err;
Dwarf_Fde fde;
Dwarf_Cie cie;
int res;
Dwarf_Small cie_version;
char * cie_augmenter;
Dwarf_Half ret_addr_column;
Dwarf_Regtable3 reg_tab;
ARM_CORE_WORD cfa_addr;
ARM_CORE_WORD reg_addr;
int i, j;
unsigned int nbytes;
struct frame_data_struct * p;
struct frame_reg_struct * prev_frame;
int nr_target_core_regs;
int target_core_pc_reg_nr;
/* the translated value of a dwarf register number -
 * mapped from a dwarf register number to a target
 * core controller-ready-to-use register number;
 * used in fetching target registers when unwinding */
int translated_dwarf_regnum;

	/* sanity checks */
	if (!cur_frame || cur_frame->older)
		panic("");
	p = ctx->frame_data;
	if (p->selected_frame_nr >= MAX_NR_FRAMES_TO_UNWIND)
		panic("");

	/* first, obtain the fde for the pc requested */
	target_core_pc_reg_nr = ctx->tdesc->get_target_pc_reg_nr(ctx);
	res = dwarf_get_fde_at_pc(p->fde_data,
			(Dwarf_Addr)get_reg_checked(ctx, cur_frame, target_core_pc_reg_nr),
			&fde,
			&unused0,
			&unused1,
			&err);
	if (res == DW_DLV_ERROR)
		panic("dwarf_get_fde_at_pc");
	if (res == DW_DLV_NO_ENTRY)
		/* cannot unwind */
		return GEAR_ERR_CANT_UNWIND_STACK_FRAME;

	/* debugging - dump fde data */
	{
	Dwarf_Addr low_pc;
	Dwarf_Unsigned func_length;
	Dwarf_Ptr fde_bytes;
	Dwarf_Unsigned fde_byte_length;
	Dwarf_Off cie_offset;
	Dwarf_Signed cie_index;
	Dwarf_Off fde_offset;

		if (dwarf_get_fde_range(
			fde,
			&low_pc,
			&func_length,
			&fde_bytes,
			&fde_byte_length,
			&cie_offset,
			&cie_index,
			&fde_offset,
			&err) != DW_DLV_OK)
		panic("");

		printf("%s():\n", __func__);
		printf("low_pc: 0x%08x\n", (unsigned int) low_pc);
		printf("func_length: 0x%08x\n", (unsigned int) func_length);
		printf("cie_offset: 0x%08x\n", (unsigned int) cie_offset);
		printf("cie_index: 0x%08x\n", (unsigned int) cie_index);
		printf("fde_offset: 0x%08x\n", (unsigned int) fde_offset);
		
	}

	/* ok, fde found, get its cie */
	if (dwarf_get_cie_of_fde(fde, &cie, &err) != DW_DLV_OK)
		panic("dwarf_get_cie_of_fde");
	/* validate cie */
	if (dwarf_get_cie_info(cie,
			&bytes_in_cie_unused,
			&cie_version,
			&cie_augmenter,
			0,
			0,
			&ret_addr_column,
			0,
			0,
			&err) != DW_DLV_OK)
		panic("dwarf_get_cie_info");
	if (cie_version != 1)
		panic("unknown dwarf cie version");
	if (cie_augmenter && *cie_augmenter)
		panic("cie augmenter string nonempty, doesnt know what to do");
	/*!	\todo	maybe support this  
	if (ret_addr_column != ARM_CORE_LR_REG_NR)
		panic("unexpected cie return address column, doesnt know what to do");
		*/
	/* initialize reg_tab; dont forget to later deallocate the core for the register rules */
	if (!(nr_target_core_regs = ctx->tdesc->get_nr_target_core_regs(ctx)))
		panic("");
	reg_tab.rt3_reg_table_size = nr_target_core_regs;

	if (!(reg_tab.rt3_rules = (struct Dwarf_Regtable_Entry3_s *)
		malloc(nr_target_core_regs * sizeof(struct Dwarf_Regtable_Entry3_s))))
		panic("out of core");

	/* the cie looks fine, unwind the registers */
	if (dwarf_get_fde_info_for_all_regs3(fde,
			(Dwarf_Addr)get_reg_checked(ctx, cur_frame, target_core_pc_reg_nr),
			&reg_tab,
			0,
			&err) != DW_DLV_OK)
		panic("dwarf_get_fde_info_for_all_regs3");
	/* see if there is enough information to unwind */
	/* first, compute the cfa address */
	switch (reg_tab.rt3_cfa_rule.dw_regnum)
	{
		case DW_FRAME_UNDEFINED_VAL:
			/* it is by far not known if this is possible at all */
			panic("");
			/* no further unwinding possible */
			/* cleanup */
			free(reg_tab.rt3_rules);
			return GEAR_ERR_CANT_UNWIND_STACK_FRAME;
			break;
		case DW_FRAME_SAME_VAL:
			/* it is by far not known if this is possible at all */
			panic("bad cfa frame unwind information");
			break;
		default:
			switch (reg_tab.rt3_cfa_rule.dw_value_type)
			{
				case DW_EXPR_OFFSET:
					if (reg_tab.rt3_cfa_rule.dw_offset_relevant)
					{
						translated_dwarf_regnum = reg_tab.rt3_cfa_rule.dw_regnum;
						gprintf("cfa rule: dwarf reg %i\n", translated_dwarf_regnum);
						if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx,
									&translated_dwarf_regnum)
								!= GEAR_ERR_NO_ERROR)
							panic("");
						gprintf("translated to core reg nr reg %i\n", translated_dwarf_regnum);
						/* this is correct */
						if (translated_dwarf_regnum >= nr_target_core_regs)
							panic("bad cfa frame unwind information");
						cfa_addr = cur_frame->reg_info[translated_dwarf_regnum].reg_val +
							reg_tab.rt3_cfa_rule.dw_offset_or_block_len;
						gprintf("reg_val(core reg %i) + dw_offset_or_block_len == 0x%08x + 0x%08x == cfa_addr == 0x%08x\n",
								translated_dwarf_regnum,
								cur_frame->reg_info[translated_dwarf_regnum].reg_val,
								(ARM_CORE_WORD) reg_tab.rt3_cfa_rule.dw_offset_or_block_len,
								cfa_addr);

					}
					else
						panic("bad cfa frame unwind information");
					break;
				case DW_EXPR_VAL_OFFSET:
					panic("bad cfa frame unwind information");
					break;
				case DW_EXPR_EXPRESSION:
{
	struct subprogram_data * subp;
	ARM_CORE_WORD pc;
	pc = get_reg_checked(ctx, cur_frame, target_core_pc_reg_nr);
	srcfile_get_srcinfo_for_addr(ctx, pc, 0, &subp, 0, 0, 0);
	gprintf("%s(): when unwinding for pc 0x%08x, for subprogram %s()\n", __func__, (int) pc, subp->name);
}
/*! \todo	fix this here... */
		return GEAR_ERR_CANT_UNWIND_STACK_FRAME;
					panic("unsupported frame unwinding rule");
					break;
				case DW_EXPR_VAL_EXPRESSION:
					panic("unsupported frame unwinding rule");
					break;
				default:
					panic("");
					break;
			}
			break;
	}

	/* the cfa address is now known, now unwind other registers */
	/* get a new frame register data structure and make
	 * the registers' values undefined by default */
	prev_frame = get_frame_reg_struct(ctx);
	/* by default, copy the older frame's register
	 * values into the younger's frame registers
	 * for callee saved registers, and make callee destroyed
	 * registers undefined */
	for (i = 0; i < nr_target_core_regs; i++)
		prev_frame->reg_info[i] = (struct reg_info_struct)
				{ .reg_content_type = ARM_CORE_REG_UNDEFINED,
					{ .is_reg_addr_applicable = 0,
					.is_reg_stored_in_memory = 0, },
					.reg_addr = 0 };
	for (i = 0; i < nr_target_core_regs; i++)
		if (ctx->tdesc->is_dwarf_reg_nr_callee_saved(ctx, i))
		{
			translated_dwarf_regnum = i;
			if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx,
						&translated_dwarf_regnum)
					!= GEAR_ERR_NO_ERROR)
				panic("");
			prev_frame->reg_info[translated_dwarf_regnum] = cur_frame->reg_info[translated_dwarf_regnum];
		}

	for (i = 0; i < nr_target_core_regs; i++)
	{
		switch (reg_tab.rt3_rules[i].dw_regnum)
		{
			case DW_FRAME_UNDEFINED_VAL:
				break;
			case DW_FRAME_SAME_VAL:
				gprintf("register rule DW_FRAME_SAME_VAL for register %i\n", i);

				/* handle special register numbers */
				if (i == ret_addr_column)
				{
					/* currently handling the program counter
					 * (return address) register number */
					TRACE();
					translated_dwarf_regnum = target_core_pc_reg_nr;
				}
				else if (i == target_core_pc_reg_nr)
					/* setting the program counter is devoid of meaning;
					 * it *must* equal the dwarf program return address
					 * column value */
					continue;
				else
				{
					/* non-special registers */
					translated_dwarf_regnum = i;
					if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx,
								&translated_dwarf_regnum)
							!= GEAR_ERR_NO_ERROR)
						panic("");
				}

				prev_frame->reg_info[translated_dwarf_regnum] = cur_frame->reg_info[i];
				break;
			default:
				switch (reg_tab.rt3_rules[i].dw_value_type)
				{
					case DW_EXPR_OFFSET:
						if (reg_tab.rt3_rules[i].dw_offset_relevant)
						{
							gprintf("register number (%i) for rule DW_EXPR_OFFSET when unwinding register r%i\n", (int) reg_tab.rt3_rules[i].dw_regnum, i);
							if (reg_tab.rt3_rules[i].dw_regnum != DW_FRAME_CFA_COL3)
							{
								gprintf("bad register number (%i) for rule DW_EXPR_OFFSET when unwinding register r%i\n", (int) reg_tab.rt3_rules[i].dw_regnum, i);
								panic("bad cfa frame unwind information");
							}
							reg_addr = cfa_addr + reg_tab.rt3_rules[i].dw_offset_or_block_len;
							gprintf("register %i: rule DW_EXPR_OFFSET, cfa_addr == 0x%08x, dw_offset_or_block_len == 0x%08x, reg_addr == 0x%08x\n", i, cfa_addr, (unsigned int) reg_tab.rt3_rules[i].dw_offset_or_block_len, reg_addr);
							/* handle special register numbers */
							if (i == ret_addr_column)
							{
								/* currently handling the program counter
								 * (return address) register number */
								TRACE();
								translated_dwarf_regnum = target_core_pc_reg_nr;
							}
							else
							{
								/* non-special registers */
								translated_dwarf_regnum = i;
								if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx,
											&translated_dwarf_regnum)
										!= GEAR_ERR_NO_ERROR)
									panic("");
							}
							prev_frame->reg_info[translated_dwarf_regnum] = (struct reg_info_struct)
									{ .reg_content_type = ARM_CORE_REG_VALID,
										{ .is_reg_addr_applicable = 1,
										.is_reg_stored_in_memory = 1, },
										.reg_addr = reg_addr };
							nbytes = sizeof(ARM_CORE_WORD);
							if (ctx->cc->core_mem_read(ctx,
										&prev_frame->reg_info[translated_dwarf_regnum].reg_val,
										reg_addr, &nbytes) != GEAR_ERR_NO_ERROR
									|| nbytes != sizeof(ARM_CORE_WORD))
							{
								printf("\n\n\nerror reading target memory at address 0x%08x\n\n\n", (unsigned int) reg_addr);
								panic("");
							}
						}
						else
						{
							panic("no test cases observed in detail - please inspect this carefully\n");
							gprintf("register rule DW_EXPR_OFFSET offset not relevant for register %i\n", i);

							translated_dwarf_regnum = i;
							j = reg_tab.rt3_rules[i].dw_regnum;
							if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx,
										&translated_dwarf_regnum)
									!= GEAR_ERR_NO_ERROR)
								panic("");
							if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx,
										&j)
									!= GEAR_ERR_NO_ERROR)
								panic("");
							prev_frame->reg_info[translated_dwarf_regnum] = cur_frame->reg_info[j];

						}
						break;
					case DW_EXPR_VAL_OFFSET:
						if (reg_tab.rt3_rules[i].dw_offset_relevant)
						{
							prev_frame->reg_info[i].reg_val = cfa_addr + reg_tab.rt3_rules[i].dw_offset_or_block_len;
							panic("");
						}
						else
							panic("unsupported frame unwinding rule");
						break;
					case DW_EXPR_EXPRESSION:
						panic("unsupported frame unwinding rule");
						break;
					case DW_EXPR_VAL_EXPRESSION:
						panic("unsupported frame unwinding rule");
						break;
				}
				break;
		}
	}

	/* update the return address and cfa values
	 * for the frame just obtained */
	/* this is defined in the arm abi */
	/* make the stack pointer equal to the cfa */
	i = ctx->tdesc->get_target_sp_reg_nr(ctx);
	prev_frame->reg_info[i].reg_val = cfa_addr;
	prev_frame->reg_info[i].reg_content_type
		= ARM_CORE_REG_VALID;
	/*! \todo	this below should not be needed
	 *		(that's why it has been commented out);
	 *		when checks confirm that it is indeed
	 *		unnecessary, remove this block */
#if 0	
	/* update the program counter from the link register */
	prev_frame->reg_info[target_core_pc_reg_nr] = (struct reg_info_struct)
			{ .reg_content_type = ARM_CORE_REG_VALID,
				{ .is_reg_addr_applicable = 0,
				.is_reg_stored_in_memory = 0, }, 
				.reg_val = prev_frame->reg_info[ret_addr_column].reg_val};
#endif

	free(reg_tab.rt3_rules);
	/* if the program counter for this frame is invalid - say that
	 * the unwinding failed; gcc 4.5.2 has been observed to generate
	 * some garbage fdes... */
	if (prev_frame->reg_info[target_core_pc_reg_nr].reg_content_type != ARM_CORE_REG_VALID)
	{
		free(prev_frame);
		return GEAR_ERR_CANT_UNWIND_STACK_FRAME;
	}
	else
	{

		cur_frame->cfa = cfa_addr;
		cur_frame->ret_pc = prev_frame->reg_info[ret_addr_column].reg_val;
		/* link nodes */
		cur_frame->older = prev_frame;
		prev_frame->younger = cur_frame;
		/* frame successfully unwound */
		return GEAR_ERR_NO_ERROR;
	}

}


/*!
 *	\fn	static enum GEAR_ENGINE_ERR_ENUM reg_cache_core_reg_read(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
 *	\brief	local override for the general ctx->cc->core_reg_read() routine
 *
 *	this is here to support access to target core registers,
 *	in the context of an arbitrary frame when the target is halted;
 *	also see the comments at the start of this file
 *
 *	\param	ctx	context to work in
 *	\param	mode	target core mode, currently ignored \todo	fix this
 *	\param	mask	a mask denoting which registers are accessed
 *	\param	buffer	a buffer to hold the registers read
 *	\return	GEAR_ERR_NO_ERROR on success, \todo	define other error codes here
 *
 *	\todo	decide what to do about registers holding undefined values
 *		(ARM_CORE_REG_UNDEFINED)
 */
static enum GEAR_ENGINE_ERR_ENUM reg_cache_core_reg_read(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
struct frame_data_struct * p;
int i, j;
int nr_target_core_regs;

	p = ctx->frame_data;
	/* see if the register cache is active - if not, pass
	 * the call directly to the overriden (old)
	 * core_reg_read() routine */
	if (!p->frame_list)
		return p->core_reg_read_prev(ctx, mode, mask, buffer);
	/* otherwise, satisfy the read from the cache */
	if (!mask)
		panic("");
	j = 0;
	/*! \note	selected_frame is not checked whether it is
	 * 		null, as it can never be null whenever
	 * 		frame_data is non-null */
	/* fetch core registers */
	if (!(nr_target_core_regs = ctx->tdesc->get_nr_target_core_regs(ctx)))
		panic("");
	for (i = 0; i < nr_target_core_regs; i++)
		if (mask & (1 << i))
			buffer[j++] = p->selected_frame->reg_info[i].reg_val;
	/* fetch program counter */
	/*! \todo	fix this */
	/*
	if (mask & (1 << 16))
		buffer[j++] = p->selected_frame->reg_info[ctx->tdesc->get_target_pc_reg_nr(ctx)].reg_val;
		*/

	/* never cache cpsr/spsr */
	/*! \todo	is this ok (not caching cpsr/spsr) */
	/*
	if (mask & ((1 << 17) | (1 << 18)))
	{
		return p->core_reg_read_prev(ctx, mode, mask & ~((1 << 17) - 1),
					buffer + j);
	}
	*/
	return GEAR_ERR_NO_ERROR;
}

/*!
 *	\fn	static enum GEAR_ENGINE_ERR_ENUM reg_cache_core_reg_write(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
 *	\brief	local override for the general ctx->cc->core_reg_read() routine
 *
 *	this is here to support access to target core registers,
 *	in the context of an arbitrary frame when the target is halted;
 *	also see the comments at the start of this file
 *
 *	\param	ctx	context to work in
 *	\param	mode	target core mode, currently ignored \todo	fix this
 *	\param	mask	a mask denoting which registers are accessed
 *	\param	buffer	a buffer that supplies the registers written
 *	\return	GEAR_ERR_NO_ERROR on success, \todo	define other error codes here
 *
 *	\todo	properly trigger reunwinding the stack, reevaluation of
 *		automatic watches, memory dumps, etc. when needed
 *
 *	\todo	decide what to do about registers holding undefined values
 *		(ARM_CORE_REG_UNDEFINED)
 */
static enum GEAR_ENGINE_ERR_ENUM reg_cache_core_reg_write(struct gear_engine_context * ctx, unsigned mode, unsigned long mask, ARM_CORE_WORD buffer[])
{
struct frame_data_struct * p;
struct frame_reg_struct * f;
int i, j;
int nr_target_core_regs;

	gprintf("%s() invoked, regmask is 0x%08x\n", __func__, mask);
	p = ctx->frame_data;
	/* see if the register cache is active - if not, pass
	 * the call directly to the overriden (old)
	 * core_reg_read() routine */
	if (!p->frame_list)
		return p->core_reg_write_prev(ctx, mode, mask, buffer);
	/* otherwise, use the cache when writing */
	if (!mask)
		panic("");
	j = 0;
	/*! \note	selected_frame is not checked whether it is
	 * 		null, as it can never be null whenever
	 * 		frame_data is non-null */
	f = p->selected_frame;
	if (!(nr_target_core_regs = ctx->tdesc->get_nr_target_core_regs(ctx)))
		panic("");
	for (i = 0; i < nr_target_core_regs; i++)
		if (mask & (1 << i))
		{
			if (f->reg_info[i].reg_content_type != ARM_CORE_REG_VALID)
				panic("");
			/* see if memory or the core should be written */
			if (f->reg_info[i].is_reg_stored_in_memory)
			{
			unsigned int nbytes;
				nbytes = sizeof(ARM_CORE_WORD);
				/* the register is stored in memory */
				if (ctx->cc->core_mem_write(ctx,
						f->reg_info[i].reg_addr,
						buffer + j,
						&nbytes) != GEAR_ERR_NO_ERROR
						|| nbytes != sizeof(ARM_CORE_WORD))
					panic("");
			}
			else
			{
				/* the register is stored in another register */
				if (p->core_reg_write_prev(ctx,
						mode,
						1 << f->reg_info[i].reg_addr,
						buffer + j) != GEAR_ERR_NO_ERROR)
					panic("");
				gprintf("%s(): writing value 0x%08x to register %i\n", __func__, buffer[j], f->reg_info[i].reg_addr);
			}
			f->reg_info[i].reg_val = buffer[j++];
		}
	/* fetch program counter */
	/*! \todo	fix this */
	if (mask & (1 << 16))
		buffer[j++] = p->selected_frame->reg_info[ctx->tdesc->get_target_pc_reg_nr(ctx)].reg_val;

	/* never cache cpsr/spsr */
	/*! \todo	is this ok (not caching cpsr/spsr) */
	if (mask & ((1 << 17) | (1 << 18)))
	{
		return p->core_reg_read_prev(ctx, mode, mask & ~((1 << 17) - 1),
					buffer + j);
	}
	return GEAR_ERR_NO_ERROR;
}


/*!
 *	\fn	static bool frame_reg_cache_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
 *	\brief	the register cache target state change callback
 *
 *	this routine determines if, depending on the new target state,
 *	the register cache should be created/destroyed
 *
 *	\param	ctx	context to work in
 *	\param	state	the new target state (may actually be the
 *			same as the previous state, but this
 *			routine is only interested in the current
 *			target state)
 *	\return	always true, denoting that any other target
 *		state change callbacks should also be invoked */
static bool frame_reg_cache_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
{
struct frame_data_struct * p;
struct frame_reg_struct * r0, * r1;
int i;
int nr_target_core_regs;
ARM_CORE_WORD * regs;

	p = ctx->frame_data;

	switch (state)
	{
		case TARGET_CORE_STATE_DEAD:
		case TARGET_CORE_STATE_RUNNING:
			/* destroy the register cache (if appropriate) */
			r0 = p->frame_list;
			if (!r0)
				break;
			do
			{
				r1 = r0->older;
				free(r0);
				r0 = r1;
			}
			while (r0);
			p->frame_list = p->selected_frame = 0;
			p->selected_frame_nr = -1;
			break;
		case TARGET_CORE_STATE_HALTED:
			/* create the register cache (if appropriate) */
			if (p->frame_list)
				/* nothing to do */
				break;
			r0 = get_frame_reg_struct(ctx);
			if (!(nr_target_core_regs = ctx->tdesc->get_nr_target_core_regs(ctx)))
				panic("");
			if (!(regs = malloc(sizeof(ARM_CORE_WORD[nr_target_core_regs]))))
				panic("");
			if (p->core_reg_read_prev(ctx,
					0,
					(1 << nr_target_core_regs) - 1,
					regs) != GEAR_ERR_NO_ERROR)
				panic("");
			for (i = 0; i < nr_target_core_regs; i++)
			{
				r0->reg_info[i] = (struct reg_info_struct)
					{ .reg_content_type = ARM_CORE_REG_VALID,
						{ .is_reg_addr_applicable = 1,
						.is_reg_stored_in_memory = 0,
						},
					.reg_addr = i,
					.reg_val = regs[i],
					};
			}
			free(regs);
			p->frame_list = p->selected_frame = r0;
			/* make the innermost (most recent) frame the active one */
			p->selected_frame_nr = 0;
			break;
		default:
			panic("");
	}
	return true;
}
/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	enum GEAR_ENGINE_ERR_ENUM frame_move_to_relative(struct gear_engine_context * ctx, int amount, int * selected_frame_nr)
 *	\brief	crawls the stack by changing the currently selected/active target call stack frame
 *
 *	\param	ctx	context to work in
 *	\param	amount	the amount to move upwards or downwards
 *			the call stack; negative values move downwards
 *			(towards the innermost - most recent) frame;
 *			positive values move upwards (towards the
 *			outermost - least recent) frame; the absolute
 *			value of this parameter denotes the number of
 *			frames to move in either direction; the special
 *			value zero 'rewinds' the stack - that is,
 *			selects/activates the innermost (most recent) frame
 *	\param	selected_frame_nr	a pointer to where to store the
 *					number of the currently selected/active
 *					frame; can be null if this number
 *					is of no interest
 *	\return	GEAR_ERR_NO_ERROR on success, GEAR_ERR_CANT_UNWIND_STACK_FRAME,
 *		if the movement operation requested the stack to be unwound
 *		past a frame for which no unwind information is available
 *		(and therefore unwinding is not possible);
 *		GEAR_ERR_CANT_REWIND_STACK_FRAME, if an attempt to rewind
 *		the stack past the most recent frame resulted;
 *		GEAR_ERR_BACKTRACE_DATA_UNAVAILABLE when backtrace
 *		data is not available (e.g. because the target is running
 *		or inaccessible)
 *
 *	\todo	maybe silently ignore attempts to rewind past the
 *		most recent frame - an error indication in this
 *		case may be useless
 */
enum GEAR_ENGINE_ERR_ENUM frame_move_to_relative(struct gear_engine_context * ctx, int amount, int * selected_frame_nr)
{
enum GEAR_ENGINE_ERR_ENUM res;
struct frame_data_struct * p;
bool move_to_older;

	p = ctx->frame_data;
	if (!p->frame_list || !p->selected_frame || p->selected_frame_nr < 0)
		return GEAR_ERR_BACKTRACE_DATA_UNAVAILABLE;

	if (amount == 0)
	{
		/* special case - rewind the call stack to the most
		 * recent frame */
		p->selected_frame = p->frame_list;
		p->selected_frame_nr = 0;
		res = GEAR_ERR_NO_ERROR;
	}
	else
	{
		/* determine direction to crawl into */
		move_to_older = (amount < 0) ? true : false;
		amount = abs(amount);

		/* mark success by default */
		res = GEAR_ERR_NO_ERROR;
		do
		{
			if (move_to_older)
			{
				if (!p->selected_frame->older)
				{
					/* see if the stack unwinding
					 * has gone too far */
					if (p->selected_frame_nr == MAX_NR_FRAMES_TO_UNWIND)
					{
						/* do not unwind more */
						gprintf("warning: attempted to unwind more than %i target call stack frames, giving up\n", MAX_NR_FRAMES_TO_UNWIND);
						return GEAR_ERR_CANT_UNWIND_STACK_FRAME;
					}
					/* must unwind */
					res = dwarf_frame_unwind(ctx, p->selected_frame);
					if (res != GEAR_ERR_NO_ERROR)
						/* does not unwind more */
						return res;
				}
				p->selected_frame = p->selected_frame->older;
				p->selected_frame_nr++;
			}
			else
			{
				/* move towards the most recent frame */
				if (p->selected_frame->younger)
				{
					p->selected_frame = p->selected_frame->younger;
					p->selected_frame_nr--;
				}
				else
				{
					/* does not rewind more */
					res = GEAR_ERR_CANT_REWIND_STACK_FRAME;
					break;
				}
			}
		}
		while (--amount);
	}

	if (selected_frame_nr)
		*selected_frame_nr = p->selected_frame_nr;
	return res;
}

/*!
 *	\fn	void init_frame_reg_cache(struct gear_engine_context * ctx)
 *	\brief	initializes the dwarf frame access engine
 *
 *	\note	must be invoked prior to any other function residing within this module
 *	\note	this routine installs hooks overriding the
 *		current context core control routines for accessing
 *		(reading and writing) target core registers, namely
 *		ctx->cc->core_reg_read() and ctx->cc->core_reg_write();
 *		it must therefore be invoked after the core control
 *		initialization; also read the comments at the start
 *		of this file
 *
 *	\todo	it may be cleaner to intercept the core_open()/core_close
 *		function calls instead
 *
 *	\param	ctx	context to work in
 *	\return	none
 */
void init_frame_reg_cache(struct gear_engine_context * ctx)
{
Dwarf_Error err;
Dwarf_Half dwhalf;
int res;
struct frame_data_struct * p;
int nr_target_core_regs;

	/* allocate the private visible data in the current gear engine context */
	if (!(p = calloc(1, sizeof * p)))
		panic("");
	p->selected_frame_nr = -1;

	ctx->frame_data = p;
	/* install register access override hooks */
	p->core_reg_read_prev = ctx->cc->core_reg_read;
	p->core_reg_write_prev = ctx->cc->core_reg_write;
	if (!p->core_reg_read_prev || !p->core_reg_write_prev)
		panic("");
	/* install the target state change callback */
	if (ctx->cc->core_register_target_state_change_callback(ctx,
			frame_reg_cache_target_state_change_callback) != GEAR_ERR_NO_ERROR)
		panic("");

	ctx->cc->core_reg_read = reg_cache_core_reg_read;
	ctx->cc->core_reg_write = reg_cache_core_reg_write;

	if (dwarf_get_fde_list(ctx->dbg,
			&p->cie_data,
			&p->cie_element_count,
			&p->fde_data,
			&p->fde_element_count,
			&err) != DW_DLV_OK)
		panic("dwarf_get_fde_list");

	/* initialize the special libdwarf register numbers corresponding to
	 * frame unwinding rules for dwarf unwinding rules 'undefined',
	 * 'same value', 'offset(n)' - information about these rules
	 * is passed by libdwarf using the special register number values
	 * DW_FRAME_UNDEFINED_VAL, DW_FRAME_SAME_VAL, and
	 * DW_FRAME_CFA_COL3 - the exact values for these registers
	 * is adjustable at run time as of libdwarf-20090510 (or so);
	 * set these register values here; this is obviously more flexible
	 * than having the values hard-coded to some arbitrary constants,
	 * large enough so that they do not equal any valid target
	 * dwarf register; for more information, see the libdwarf
	 * documentation
	 *
	 * follows a verbatim excerpt from the libdwarf-20090716 release
	 * to make things more clear
	 *
	 * \todo	is copying source from libdwarf here violating
	 *		something??? */

#if 0 /* start of libdwarf-20090716 excerpt - clarification purposes */

/* Taken as meaning 'undefined value', this is not
   a column or register number.
   Only present at libdwarf runtime. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE.
*/
#define DW_FRAME_UNDEFINED_VAL          1034

/* Taken as meaning 'same value' as caller had, not a column
   or register number
   Only present at libdwarf runtime. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE.
*/
#define DW_FRAME_SAME_VAL               1035

/* For DWARF3 interfaces, make the CFA a column with no
   real table number.  This is what should have been done
   for the DWARF2 interfaces.  This actually works for
   both DWARF2 and DWARF3, but see the libdwarf documentation
   on Dwarf_Regtable3 and  dwarf_get_fde_info_for_reg3()
   and  dwarf_get_fde_info_for_all_regs3()  
   Do NOT use this with the older dwarf_get_fde_info_for_reg()
   or dwarf_get_fde_info_for_all_regs() consumer interfaces.
   Must be higher than any register count for *any* ABI.
   (an unfortunate consequence of a lack of any way
   to set it at run time).
*/
#define DW_FRAME_CFA_COL3               1436

#endif /* end of libdwarf-20090716 excerpt - clarification purposes */
	/*! \todo	careful with setting these - these should work
	 *		for any architecture currently supported, but
	 *		be careful if adding a target with a (very)
	 *		large register set */
	dwhalf = dwarf_set_frame_undefined_value(ctx->dbg, DW_FRAME_UNDEFINED_VAL);
	gprintf("previous dwarf frame unwinder UNDEFINED register number value: 0x%x\n", dwhalf);
	dwhalf = dwarf_set_frame_same_value(ctx->dbg, DW_FRAME_SAME_VAL);
	gprintf("previous dwarf frame unwinder SAME VALUE register number value: 0x%x\n", dwhalf);
	dwhalf = dwarf_set_frame_cfa_value(ctx->dbg, DW_FRAME_CFA_COL3);
	gprintf("previous dwarf frame unwinder CFA register number value: 0x%x\n", dwhalf);

	/* initialize default register rules for stack unwinding */
	res = dwarf_set_frame_rule_inital_value(ctx->dbg, DW_FRAME_UNDEFINED_VAL);
	res = dwarf_set_frame_rule_inital_value(ctx->dbg, DW_FRAME_SAME_VAL);
	gprintf("previos initial frame rule: ");
	switch (res)
	{
		case DW_FRAME_SAME_VAL:
			gprintf("DW_FRAME_SAME_VAL");
			break;
		case DW_FRAME_UNDEFINED_VAL:
			gprintf("DW_FRAME_UNDEFINED_VAL");
			break;
		default:
			gprintf("unknown");
	}
	/*! set the register rule table size
	 *
	 * \todo	this may be very wrong, it doesnt handle floating
	 *		point registers at all; fix it when i(sgs) start coding
	 *		the floating point support
	 */
	if (!(nr_target_core_regs = ctx->tdesc->get_nr_target_core_regs(ctx)))
		panic("");
	res = dwarf_set_frame_rule_table_size(ctx->dbg, nr_target_core_regs);
	gprintf("previous frame rule table size: %i\n", res);

	gprintf("ok, dwarf frame processing module initialized\n");
}

