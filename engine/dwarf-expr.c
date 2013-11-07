/*!
 * \file	dwarf-expr.c
 * \brief	dwarf expression evaluator
 * \author	shopov
 *
 *
 *	\todo		bear in mind the warning below
 *	\warning	reported is that gcc (in violation with the dwarf 3
 *			standard) emits DW_OP_regX opcodes in part of more
 *			complex expressions (while such opcodes must be used only
 *			in dwarf location expressions, and then - without combining
 *			them with any other opcodes), and the intention is to make
 *			this (deprecated) use of the DW_OP_regX opcode a compact
 *			replacement of the (correct  by the standard in this case)
 *			use of the DW_OP_bregX 0 opcode; that is, the nonconformance
 *			of gcc with the standard is, that when the DW_OP_regX is used
 *			alone in a location expression (the correct use by the standard) -
 *			the interpretation of this is "register X" (correct by the
 *			standard), but when this is used as a part of a more complex
 *			expression, containing opcodes other than the DW_OP_regX
 *			(incorrect use by the standard), that should be interpreted
 *			(incorrectly by the standard) as "DW_OP_bregX 0" - i.e.
 *			"the address contained in register X (to which value zero is
 *			added)" 
 *			bear this in mind and provide workarounds when necessary
 *
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

#include "dwarf-common.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "engine-err.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "gprintf.h"
#include "util.h"


/*
 *
 * local definitions follow
 *
 */

/*! the demaximum stack size, as the dwarf expression stack is currently statically allocated
 *
 * \todo	make this dynamically allocated (thread safety) */
#define STACK_SIZE	128

/*
 *
 * local data follows
 *
 */

/*! the dwarf expression stack, used for evaluating dwarf expressions
 *
 * even though this is declared as unsigned, it is at places necessary
 * to interpret data here as being signed
 * the dwarf4 standard (section 2.5) says:
DWARF expressions describe how to compute a value or name a location during debugging of a program. They are expressed in terms of DWARF operations that operate on a stack of values.
 * and below in section 2.5.1:
Each element of the stack is the size of an address on the target machine.
 */
static ARM_CORE_WORD	dwstack[STACK_SIZE];
/*! the index of the first free element to use on the stack (an empty ascending convention)
 *
 * the stack grows up */
static int stacktop;

/*
 *
 * local functions follow
 *
 */

/*!
 * \fn		static void dw_push(ARM_CORE_WORD val)
 * \brief	pushes a value onto the dwarf stack
 *
 * \param	val	the value to push onto the stack
 * \return	none
 */

static void dw_push(ARM_CORE_WORD val)
{
	if (stacktop == STACK_SIZE)
		panic("dwarf stack full");
	dwstack[stacktop++] = val;

}

/*!
 * \fn		static void dw_pop(void)
 * \brief	pops a value onto the dwarf stack
 *
 * \param	val	the value to push onto the stack
 * \return	none
 */

static ARM_CORE_WORD dw_pop(void)
{
	if (stacktop == 0)
		panic("dwarf stack empty");
	return dwstack[--stacktop];

}

/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	ARM_CORE_WORD dwarf_expr_eval(struct gear_engine_context * ctx, Dwarf_Locdesc * locdesc, Dwarf_Unsigned fbreg, bool * is_result_a_register, enum DWARF_EXPR_INFO_ENUM * eval_info);
 *	\brief	given a location description, evaluates the dwarf expression for the location
 *
 *	\note	when evaluating the location of a local variable, it
 *		is common that it resides in the stack and a frame base
 *		register value is needed; in this case, first should be
 *		computed the value of the frame base register
 *		for the subprogram appropriate, and this value should
 *		be then passed as a parameter when evaluating the
 *		location of the local variable; when evaluating the
 *		location expression for the frame base register itself,
 *		zero should be passed for the fbreg parameter; in any case,
 *		the various evaluation status variables returned by
 *		this function (e.g. is_result_a_register, eval_info)
 *		can be used by the caller to detect anomalies arised
 *		during an expression evaluation (such as needing a
 *		frame base register when evaluating an expression
 *		for a frame base register itself, etc.)
 *		
 *
 *	\param	ctx	context used to access debugging information
 *			about the debuggee
 *	\param	locdesc	a pointer to the location expression to evaluate;
 *			cannot be null
 *	\param	fbreg	the frame base register value to use when evaluating
 *			the expression; if unavailable, should be zero;
 *			also, read the note above
 *	\param	is_result_a_register	set to true if the result of
 *			evaluating the expression is a register number,
 *			otherwise (the result is a memory location) set
 *			to false; can be null
 *	\param	eval_info	used to convey information about the
 *			expression evaluation, see the DWARF_EXPR_INFO_ENUM
 *			for details; can be null
 *	\return	the value of the expression, must always fit into the
 *		result type, which is currently not the case for
 *		all result types
 *
 *	\todo	maybe revise the returned result type
 *
 *	\todo	duplicated below is an important warning that appears at the
 *		start of this file, and which is related to dwarf debugging
 *		information output of gcc
 *	\todo		bear in mind the warning below
 *	\warning	reported is that gcc (in violation with the dwarf 3
 *			standard) emits DW_OP_regX opcodes in part of more
 *			complex expressions (while such opcodes must be used only
 *			in dwarf location expressions, and then - without combining
 *			them with any other opcodes), and the intention is to make
 *			this (deprecated) use of the DW_OP_regX opcode a compact
 *			replacement of the (correct  by the standard in this case)
 *			use of the DW_OP_bregX 0 opcode; that is, the nonconformance
 *			of gcc with the standard is, that when the DW_OP_regX is used
 *			alone in a location expression (the correct use by the standard) -
 *			the interpretation of this is "register X" (correct by the
 *			standard), but when this is used as a part of a more complex
 *			expression, containing opcodes other than the DW_OP_regX
 *			(incorrect use by the standard), that should be interpreted
 *			(incorrectly by the standard) as "DW_OP_bregX 0" - i.e.
 *			"the address contained in register X (to which value zero is
 *			added)" 
 *			bear this in mind and provide workarounds when necessary
 */
ARM_CORE_WORD dwarf_expr_eval(struct gear_engine_context * ctx,
		Dwarf_Locdesc * locdesc, Dwarf_Unsigned fbreg,
		bool * is_result_a_register, enum DWARF_EXPR_INFO_ENUM * eval_info)
{
Dwarf_Loc * p;
int i, j;
bool is_result_reg;
enum DWARF_EXPR_INFO_ENUM exp_info;

	/* sanity checks */
	if (!locdesc || !ctx->cc)
		panic("");
	/* initialize stack */
	stacktop = 0;

	p = locdesc->ld_s;
	i = locdesc->ld_cents;
	is_result_reg = false;
	exp_info = DW_EXPR_IS_CONSTANT;

	while (i--)
	{
		j = 31;
		if (is_result_reg)
			panic("invalid dwarf expression - result is a register and more operations found");
		switch (p->lr_atom)
		{
			/* from the dwarf4 standard:
DW_OP_deref The DW_OP_deref operation pops the top stack entry and treats it as an address. The value retrieved from that address is pushed. The size of the data retrieved from the dereferenced address is the size of an address on the target machine.
			 */
			case DW_OP_deref:
			{
			ARM_CORE_WORD t;
			unsigned nbytes;

				exp_info = DW_EXPR_NEEDS_TARGET_MEM_ACCESS;
				t = dw_pop();
				nbytes = sizeof(ARM_CORE_WORD);
				if (ctx->cc->core_mem_read(ctx, &t, t, &nbytes) != GEAR_ERR_NO_ERROR)
					panic("");
				if (nbytes != sizeof(ARM_CORE_WORD))
					panic("");

				dw_push(t);

				break;
			}
			/* literal encodings */
			case DW_OP_lit0:
				j--;
			case DW_OP_lit1:
				j--;
			case DW_OP_lit2:
				j--;
			case DW_OP_lit3:
				j--;
			case DW_OP_lit4:
				j--;
			case DW_OP_lit5:
				j--;
			case DW_OP_lit6:
				j--;
			case DW_OP_lit7:
				j--;
			case DW_OP_lit8:
				j--;
			case DW_OP_lit9:
				j--;
			case DW_OP_lit10:
				j--;
			case DW_OP_lit11:
				j--;
			case DW_OP_lit12:
				j--;
			case DW_OP_lit13:
				j--;
			case DW_OP_lit14:
				j--;
			case DW_OP_lit15:
				j--;
			case DW_OP_lit16:
				j--;
			case DW_OP_lit17:
				j--;
			case DW_OP_lit18:
				j--;
			case DW_OP_lit19:
				j--;
			case DW_OP_lit20:
				j--;
			case DW_OP_lit21:
				j--;
			case DW_OP_lit22:
				j--;
			case DW_OP_lit23:
				j--;
			case DW_OP_lit24:
				j--;
			case DW_OP_lit25:
				j--;
			case DW_OP_lit26:
				j--;
			case DW_OP_lit27:
				j--;
			case DW_OP_lit28:
				j--;
			case DW_OP_lit29:
				j--;
			case DW_OP_lit30:
				j--;
			case DW_OP_lit31:

				dw_push(j);
				break;

			case DW_OP_addr:
				dw_push(p->lr_number);
				break;
			case DW_OP_const1u:
			case DW_OP_const1s:
			case DW_OP_const2u:
			case DW_OP_const2s:
			case DW_OP_const4u:
			case DW_OP_const4s:
			case DW_OP_const8u:
			case DW_OP_const8s:
				dw_push(p->lr_number);
				break;
			case DW_OP_constu:
			case DW_OP_consts:
				panic("check dwarf opcode");
				break;

			case DW_OP_breg0:
				j--;
			case DW_OP_breg1:
				j--;
			case DW_OP_breg2:
				j--;
			case DW_OP_breg3:
				j--;
			case DW_OP_breg4:
				j--;
			case DW_OP_breg5:
				j--;
			case DW_OP_breg6:
				j--;
			case DW_OP_breg7:
				j--;
			case DW_OP_breg8:
				j--;
			case DW_OP_breg9:
				j--;
			case DW_OP_breg10:
				j--;
			case DW_OP_breg11:
				j--;
			case DW_OP_breg12:
				j--;
			case DW_OP_breg13:
				j--;
			case DW_OP_breg14:
				j--;
			case DW_OP_breg15:
				j--;
			case DW_OP_breg16:
				j--;
			case DW_OP_breg17:
				j--;
			case DW_OP_breg18:
				j--;
			case DW_OP_breg19:
				j--;
			case DW_OP_breg20:
				j--;
			case DW_OP_breg21:
				j--;
			case DW_OP_breg22:
				j--;
			case DW_OP_breg23:
				j--;
			case DW_OP_breg24:
				j--;
			case DW_OP_breg25:
				j--;
			case DW_OP_breg26:
				j--;
			case DW_OP_breg27:
				j--;
			case DW_OP_breg28:
				j--;
			case DW_OP_breg29:
				j--;
			case DW_OP_breg30:
				j--;
			case DW_OP_breg31:

			{
				ARM_CORE_WORD reg;

				if (j > ctx->tdesc->get_nr_target_core_regs(ctx))
					panic("unknown target register number requested in dwarf expression");
				if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx, &j)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				/*! \todo	handle mode correctly */
				if (ctx->cc->core_reg_read(ctx, 0, 1 << j, &reg)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				if (exp_info < DW_EXPR_NEEDS_REGS)
					exp_info = DW_EXPR_NEEDS_REGS;
				dw_push((signed)reg + p->lr_number);
				break;
			}
			case DW_OP_bregx:
			{
				ARM_CORE_WORD reg;

				j = p->lr_number;
				if (j > ctx->tdesc->get_nr_target_core_regs(ctx))
					panic("unknown target register number requested in dwarf expression");
				if (ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(ctx, &j)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				/*! \todo	handle mode correctly */
				if (ctx->cc->core_reg_read(ctx, 0, 1 << j, &reg)
						!= GEAR_ERR_NO_ERROR)
					panic("");
				if (exp_info < DW_EXPR_NEEDS_REGS)
					exp_info = DW_EXPR_NEEDS_REGS;
				dw_push((signed)reg + p->lr_number2);
				break;
			}
			case DW_OP_fbreg:
				dw_push(p->lr_number + fbreg);
				if (exp_info < DW_EXPR_NEEDS_FBREG)
					exp_info = DW_EXPR_NEEDS_FBREG;
				break;

			case DW_OP_reg0:
				j--;
			case DW_OP_reg1:
				j--;
			case DW_OP_reg2:
				j--;
			case DW_OP_reg3:
				j--;
			case DW_OP_reg4:
				j--;
			case DW_OP_reg5:
				j--;
			case DW_OP_reg6:
				j--;
			case DW_OP_reg7:
				j--;
			case DW_OP_reg8:
				j--;
			case DW_OP_reg9:
				j--;
			case DW_OP_reg10:
				j--;
			case DW_OP_reg11:
				j--;
			case DW_OP_reg12:
				j--;
			case DW_OP_reg13:
				j--;
			case DW_OP_reg14:
				j--;
			case DW_OP_reg15:
				j--;
			case DW_OP_reg16:
				j--;
			case DW_OP_reg17:
				j--;
			case DW_OP_reg18:
				j--;
			case DW_OP_reg19:
				j--;
			case DW_OP_reg20:
				j--;
			case DW_OP_reg21:
				j--;
			case DW_OP_reg22:
				j--;
			case DW_OP_reg23:
				j--;
			case DW_OP_reg24:
				j--;
			case DW_OP_reg25:
				j--;
			case DW_OP_reg26:
				j--;
			case DW_OP_reg27:
				j--;
			case DW_OP_reg28:
				j--;
			case DW_OP_reg29:
				j--;
			case DW_OP_reg30:
				j--;
			case DW_OP_reg31:
			{
				is_result_reg = true;
				dw_push(j);
				break;
			}
			case DW_OP_regx:
			{
				is_result_reg = true;
				dw_push(p->lr_number);
				break;
			}
			/* this one is new in dwarf4 */
			case DW_OP_stack_value:
				exp_info = DW_EXPR_LOCATION_IS_CONSTANT_BUT_NOT_ADDR;
				break;
			default:
				gprintf("unsupported dwarf expression opcode: %hhu\n", p->lr_atom);
				panic("");
				break;
		}

		/* skip to next opcode */
		p++;
	}

	if (stacktop != 1)
		panic("error evaluating dwarf expression");
	if (is_result_a_register)
		*is_result_a_register = is_result_reg;
	if (eval_info)
		*eval_info = exp_info;
	return dwstack[0];
}

