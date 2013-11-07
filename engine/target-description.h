/*!
 *	\file	target-description.h
 *	\brief	target machine specific parameters
 *	\author	shopov
 *
 *	this module defines the struct target_desc_struct (below),
 *	which provides an interface for accessing target resources
 *	and parameters that require specific handling and knowledge
 *	regarding each supported target and target
 *	application binary interface (abi) variant
 *	for example, this interface provides means for getting information
 *	like the number of target core registers, determining
 *	the values of specific 'distinguished' registers (e.g.
 *	program counter, stack pointer registers, etc.), managing
 *	the correspondence between target register numbers and
 *	human-friendly target register names, determining which
 *	registers values are preserved across subroutine calls, etc.
 *
 *
 *	Revision summary:
 *
 *	$Log: $
 */


/*
 *
 * exported data types follow
 *
 */


/*! target description data structure
 *
 * also read the comments at the start of this file */
struct target_desc_struct
{
	/*! target private data pointer, opaque type defined by each target description module */
	struct tdesc_private_data	* p;
	/*! a function to retrieve the number of target core registers
	 *
	 * if the number of target core registers is unknown, this
	 * returns zero */
	int (* get_nr_target_core_regs)(struct gear_engine_context * ctx);

	/*! a function to retrieve the number of the program counter target register
	 *
	 * \note	it is important to note that this function
	 *		returns the register number as a number that
	 *		should be passed to a target core controller
	 *		in order to access the register contents
	 *		(e.g., via core_reg_read()/core_reg_write),
	 *		the register number returned by this routine
	 *		is *not* a register number as understood by
	 *		a compiler or a debugger (i.e. this is neither
	 *		a gcc internal target register number, nor a
	 *		dwarf vendor specified register number) - instead,
	 *		this is a register number that is understood by
	 *		a target core controller for the purposes of
	 *		physical access to this target core register */
	int (* get_target_pc_reg_nr)(struct gear_engine_context * ctx);

	/*! a function to retrieve the number of the stack pointer target register
	 *
	 * \note	it is important to note that this function
	 *		returns the register number as a number that
	 *		should be passed to a target core controller
	 *		in order to access the register contents
	 *		(e.g., via core_reg_read()/core_reg_write),
	 *		the register number returned by this routine
	 *		is *not* a register number as understood by
	 *		a compiler or a debugger (i.e. this is neither
	 *		a gcc internal target register number, nor a
	 *		dwarf vendor specified register number) - instead,
	 *		this is a register number that is understood by
	 *		a target core controller for the purposes of
	 *		physical access to this target core register */
	int (* get_target_sp_reg_nr)(struct gear_engine_context * ctx);

	/*! a function to retrieve the number of the program status word target register
	 *
	 * \note	it is important to note that this function
	 *		returns the register number as a number that
	 *		should be passed to a target core controller
	 *		in order to access the register contents
	 *		(e.g., via core_reg_read()/core_reg_write),
	 *		the register number returned by this routine
	 *		is *not* a register number as understood by
	 *		a compiler or a debugger (i.e. this is neither
	 *		a gcc internal target register number, nor a
	 *		dwarf vendor specified register number) - instead,
	 *		this is a register number that is understood by
	 *		a target core controller for the purposes of
	 *		physical access to this target core register */
	int (* get_target_pstat_reg_nr)(struct gear_engine_context * ctx);

	/*! tells if a dwarf register number of interest is callee saved in the currently selected/active target/abi/pcs combination
	 *
	 * this is primarily used by/useful to the dwarf target
	 * stack frame register unwinder (in module frame-reg-cache.c),
	 * which, by default, copies the older frame's register
	 * values into the younger's frame registers
	 * for callee saved registers, and makes callee destroyed
	 * registers undefined */
	bool (* is_dwarf_reg_nr_callee_saved)(struct gear_engine_context * ctx, int dwarf_reg_nr);

	/*! a function to map a dwarf register to a target core controller understood register number
	 *
	 * this function translates a dwarf debug information specified
	 * register number to a register number understood by a target
	 * core controller, that is - a register number that can be passed
	 * to a target core controller to physically access (e.g. via
	 * core_reg_read()/core_reg_write()) the register
	 *
	 * the input dwarf register number is passed via the inout_reg_nr
	 * pointer, and the same location is used to return the
	 * target core controller understood register number
	 *
	 * on success, the function returns a GEAR_ERR_NO_ERROR error
	 * code, on failure (e.g. if the dwarf register number is
	 * unknown and cannot be translated), a more specific error
	 * code is returned
	 *
	 * \todo	error codes other than GEAR_ERR_NO_ERROR
	 *		are currently not supported, update the comments
	 *		here to state exactly which error codes on
	 *		exactly which occasions are returned */
	enum GEAR_ENGINE_ERR_ENUM (* translate_dwarf_reg_nr_to_target_reg_nr)(struct gear_engine_context * ctx,
			int * inout_reg_nr);
	/*! translates a target core register number (e.g. r0) to an alternative, more informative name
	 *
	 * this is mainly used by the register printer, if no alternative
	 * name is available, a null pointer is returned
	 */
	const char * (* translate_target_core_reg_nr_to_human_readable)(struct gear_engine_context * ctx,
			unsigned int target_core_reg_nr);
	/*! decodes an instruction at a given target address, determines various information of interest about this instruction, and returns the instruction size in bytes
	 *
	 * this function decodes the instruction residing at target 
	 * address 'addr', sets the value in '* next_insn_addr'
	 * to equal the value of the program counter after execution
	 * of the instruction at address 'addr' has finished
	 * (note that this takes in account the current value
	 * of the program status register in case the instruction
	 * to decode is a conditionally executed one, also - branch
	 * and call instructions are handled as well), sets the
	 * value in * 'is_probably_a_function_call_insn' to nonzero
	 * if the instruction decoded is (actually, looks like) a
	 * subroutine call instruction
	 * (and sets * 'is_probably_a_function_call_insn' to zero
	 * otherwise), and returns the size (in bytes) of the
	 * instruction decoded
	 *
	 * any of the parameters 'next_insn_addr' and
	 * 'is_probably_a_function_call_insn' can be null in case
	 * the respective information is not of interest to the
	 * caller
	 *
	 * \note	if this function succeeds, it returns a
	 *		positive number which is the length of
	 *		the decoded instruction in bytes;
	 *		if this function fails, it returns zero */
	int (* decode_insn)(struct gear_engine_context * ctx,
			ARM_CORE_WORD addr,
			ARM_CORE_WORD * next_insn_addr,
			bool * is_probably_a_function_call_insn);

	/*! prints disassembly text for the instruction residing at a given target memory address
	 *
	 * outputs disassembly text for the instruction at target
	 * core address 'addr', by using the function 'print_fn'
	 * to do the output; returns a positive number denoting
	 * the instruction length on success, or zero on failure
	 * (e.g. when the instruction mnemonic is invalid,
	 * or the given target memory address could not be accessed */
	int (* print_disassembled_insn)(struct gear_engine_context * ctx, ARM_CORE_WORD addr,
			int (*print_fn)(const char * format, ...));
};

