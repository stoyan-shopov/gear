/*!
 * \file	dwarf-expr.h
 * \brief	dwarf expression evaluator header file
 * \author	shopov
 *
 *
 * Revision summary:
 *
 * $Log: $
 */

#ifndef __DWARF_EXPR_H__
#define __DWARF_EXPR_H__


/*
 *
 * exported types follow
 *
 */

/*! enumeration giving information about a dwarf expression evaluation requirementes
 *
 * this is returned as a result of evaluating a dwarf expression
 * via dwarf_expr_eval(), it provides information about what resources
 * were needed for evaluating a given expression; this is here mainly
 * to convey information whether an expression is a constant one (and
 * therefore does not require valid target context to evaluate, i.e.
 * can be evaluated without a real target available), or an expression
 * requres resources (e.g. registers) to compute and is therefore
 * dependent on having a valid target context
 *
 * \warning	order is important here, ordering of these
 *		is used in dwarf_expr_eval
 */ 
enum DWARF_EXPR_INFO_ENUM
{
	/*! invalid value, do not use */
	DW_EXPR_INFO_INVALID = 0,
	/*! the expression is a constant, i.e. does not require register values to compute */
	DW_EXPR_IS_CONSTANT,
	/*! the value computed has no location in the dwarf-described program (and therefore has no address), but is rather a known constant value
	 *
	 * this type of value can correspond to the new in dwarf4
	 * DW_OP_stack_value opcode, and denotes that the object
	 * whose location is being computed does not really
	 * exist in the program being described and therefore
	 * has no address associated with it, but nevertheless
	 * the value of this object is known and the value computed
	 * by the dwarf location description for this object is
	 * actually the value of this object, rather than its address;
	 * an example - some tests were run under gcc 4.5.0, where
	 * a function called another function, with a parameter which
	 * was a pointer; in the function call this pointer was
	 * set to point to a static storage duration array - with
	 * optimizations turned on, gcc 4.5.0 actually inlined the
	 * function being called, and furthermore optimized the
	 * pointer parameter to not exist in the target (in memory
	 * or registers), but to rather be initialized from
	 * the constant address of the statically allocated array */
	DW_EXPR_LOCATION_IS_CONSTANT_BUT_NOT_ADDR,
	/*! the expression needs target memory access to evaluate */
	DW_EXPR_NEEDS_TARGET_MEM_ACCESS,
	/*! the expression needs registers to evaluate */
	DW_EXPR_NEEDS_REGS,
	/*! the expression needs a frame base register to evaluate 
	 *
	 * this naturally implies DW_EXPR_NEEDS_REGS */
	DW_EXPR_NEEDS_FBREG,
};

/*
 *
 * exported function prototypes follow
 *
 */

ARM_CORE_WORD dwarf_expr_eval(struct gear_engine_context * ctx,
		Dwarf_Locdesc * locdesc, Dwarf_Unsigned fbreg,
		bool * is_result_a_register, enum DWARF_EXPR_INFO_ENUM * eval_info);
     
#endif /* __DWARF_EXPR_H__ */

