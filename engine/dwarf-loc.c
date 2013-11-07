/*!
 * \file	dwarf-loc.c
 * \brief	dwarf location description manipulation
 * \author	shopov
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

#include "dwarf-common.h"
#include "target-defs.h"
#include "engine-err.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "util.h"

/*
 *
 * exported functions follow follow
 *
 */


/*!
 *	\fn	enum GEAR_ENGINE_ERR_ENUM dwarf_loc_eval_loc_from_list(struct gear_engine_context * ctx, ARM_CORE_WORD * result, bool * is_result_a_register, enum DWARF_EXPR_INFO_ENUM * eval_info, struct dwarf_location * loclist, ARM_CORE_WORD addr, ARM_CORE_WORD cu_base_addr, Dwarf_Unsigned fbreg);
 *	\brief	evaluate a dwarf location expression
 *
 *	given an address addr, and a location list loclist, this function
 *	locates the appropriate dwarf expression containing location information
 *	for the address, evaluates the expression, sets the variable pointed to
 *	by is_result_in_register to nonzero if the location expression evaluates
 *	to a register number and to zero otherwise, sets the variable pointed to
 *	by result to the value of the expression and returns zero; if the
 *	function fails for any reason, a nonzero value is returned; in the calculation
 *	of the dwarf expression, any references to the frame base (DW_OP_fbreg dwarf
 *	opcodes) are satisfied by the supplied fbreg parameter value; as this value
 *	must be computed by a call to this very routine as well, when calculating
 *	a dwarf expression for the frame base, DW_OP_fbreg opcodes would be illegal,
 *	but currently this is not checked; the compilation unit base address
 *	cu_base_addr is needed to adjust the hi and low pc values in the dwarf location
 *	list ranges, in the case there are no base address selection entries in the list -
 *	and such are currently totally unsupported; as the dwarf 3 specification says:
 *	"In the case of a compilation unit where all of the machine code is contained
 *	in a single contiguous section, no base address selection entry is needed."
 *
 *	\todo	implement frame base calculation sanity checks
 *	\todo	implement error reporting
 *	\todo	resolve the case with base address selection entries
 *	\todo	maybe fix frame base address handling
 *	\todo	maybe make the comparisons below more descriptive
 *		by using symbol names (0xffffffff comparisons below)
 *
 *	\todo	currently, only 32 bit addresses are handled here;
 *		see also the notes below in the location list scanning loop;
 *		this will have to be fixed if address widths other
 *		than 32 bit ones must be supported
 *
 *	\param	ctx	context used to access debugging information
 *			about the debuggee
 *	\param	result	the result is stored here; cannot be null
 *	\param	is_result_a_register	set to true if the dwarf expression evaluates to
 *		a register number, set to false otherwise; can be null
 *	\param	eval_info	used to convey information about the
 *			expression evaluation, see the DWARF_EXPR_INFO_ENUM
 *			for details; can be null
 *	\param	loclist		the location list to evaluate from; cannot be null
 *	\param	addr		the address to evaluate for
 *	\param	cu_base_addr	the base address of the compilation unit in which the
 *				object to locate is referenced, needed to adjust
 *				hi and lo pc values in location list entries
 *	\param	fbreg		value to be used in DW_OP_fbreg opcodes in the dwarf expression
 *
 *	\return	GEAR_ERR_NO_ERROR on success, GEAR_ERR_GENERIC_ERROR on failure
 *	\todo	more specific error codes may become needed
 */

enum GEAR_ENGINE_ERR_ENUM dwarf_loc_eval_loc_from_list(struct gear_engine_context * ctx,
		ARM_CORE_WORD * result, bool * is_result_a_register,
		enum DWARF_EXPR_INFO_ENUM * eval_info,
		struct dwarf_location * loclist, ARM_CORE_WORD addr,
	       	ARM_CORE_WORD cu_base_addr, Dwarf_Unsigned fbreg)
{
Dwarf_Locdesc * p;
int i;

	/* sanity checks */
	if (!result || !loclist || !ctx->cc)
		panic("");
	gprintf("shopov, is the dw_at_entry_pc case handled properly?\n");
	gprintf("shopov, is the base address selection case handled properly?\n");

	gprintf("%s(): addr is 0x%08x\n", __func__, addr);
	/* search the location list for the address */
	p = loclist->llbuf[0];
	/* see if the location is valid for the scope of the object it is
	 * associated with */
	if (p->ld_lopc == 0 && p->ld_hipc == 0)
		panic("dwarf_loc_eval_loc_from_list(): case not handled");
	i = 0;
	gprintf("%s(): cu_base_addr 0x%08x\n", __func__, (unsigned int) cu_base_addr);
	while (i < loclist->listlen)
	{
		/* \note	here are a few special cases
		 *		in which we compare addresses against
	         *		the largest target address value, which
		 *	 	for a 32 bit machine (as is currently
		 *		the case for arm targets) equals
		 *		0xffffffff, yet the Dwarf_Addr
		 *		data type used in the calculations
		 *		is defined as 'unsigned long long',
		 *		and as the libdwarf documentation
		 *		states, in cases when a single
		 *		location description (as opposed
		 *		to a location list) is used to
		 *		describe the location of an object,
		 *		the call to dwarf_loclist_n() (in
		 *		libdwarf) results in (quoting the
		 *		libdwarf consumer library interface 
		 *		document, revision 1.55, 04 july 2007,
		 *		distributed along with the libdwarf
		 *		source code):
		 * If the attribute is a location description (DW_FORM_block2 or DW_FORM_block4) then some of the
		 * Dwarf_Locdesc values of the single Dwarf_Locdesc record are set to sensible but arbitrary values.
		 * Specifically, ld_lopc is set to 0 and ld_hipc is set to all-bits-on. And *listlen is set to 1.
		 *
		 *		which means ld_lopc and ld_hipc are
		 *		set to 0xffffffffffffffff on
		 *		a x86 machine (as long long-s are 64
		 *		bits wide there) and comparing them
		 *		to the largest address value for an 32 bit
		 *		(e.g. arm) machine, which is 0xffffffff,
		 *		doesnt work; the problem here is solved
		 *		by not testing for equality, but rather
		 *		using greater-or-equal comparisons
		 *		(with respect to the largest 32 bit
		 *		address value 0xffffffff) in the code
		 *		below (in case anyone wondered why
		 *		the comparisons below are such, this
		 *		is the explanation) */
		gprintf("%s(): loclist entry %i, low_addr 0x%08x, hi_addr 0x%08x\n",
				__func__,
				i,
				(unsigned int) p->ld_lopc + cu_base_addr,
				(unsigned int) p->ld_hipc + cu_base_addr);

		/* stay alert for base address selection entries
		 * \todo	verify that this works by finding a testcase */
		if (p->ld_lopc /*==*/ >= 0xffffffff)
			panic("base address selection entry sensed in dwarf location list, doesnt know what to do");
		/* a special case test - see if the location description
		 * spans the whole address range */
		if (p->ld_lopc == 0 && p->ld_hipc /*==*/ >= 0xffffffff)
			break;
		/* regular cases follow */
		if (p->ld_lopc + cu_base_addr <= addr
			&& addr < p->ld_hipc + cu_base_addr)
			/* dwarf expression found */
			break;
		p = loclist->llbuf[++i];
	}

	if (i == loclist->listlen)
	{
		gprintf("expression out of range\n");
		return GEAR_ERR_GENERIC_ERROR;
	}
	*result = dwarf_expr_eval(ctx, loclist->llbuf[i], fbreg, is_result_a_register, eval_info);
	return GEAR_ERR_NO_ERROR;
}

/*!
 *	\fn	bool dwarf_loc_are_locations_the_same(struct gear_engine_context * ctx, struct dwarf_location * loc1, struct dwarf_location * loc2);
 *	\brief	determines if two location descriptions are the same
 *
 *	\param	ctx	context to work in
 *	\param	loc1	first location description data structure
 *	\param	loc2	second location description data structure
 *	\return	true, if the location descriptions passed are the same,
 *		false otherwise */
bool dwarf_loc_are_locations_the_same(struct gear_engine_context * ctx, struct dwarf_location * loc1, struct dwarf_location * loc2)
{
int i;
Dwarf_Locdesc * l1, * l2;
	if (loc1->listlen != loc2->listlen)
		return false;
	for (i = 0; i < loc1->listlen; i ++)
	{
		l1 = loc1->llbuf[i];
		l2 = loc2->llbuf[i];
		if (l1->ld_lopc != l2->ld_lopc
				|| l1->ld_hipc != l2->ld_hipc
				|| l1->ld_cents != l2->ld_cents)
			return false;
		if (memcmp(l1->ld_s, l2->ld_s, l1->ld_cents * sizeof * l1->ld_s))
			return false;
	}
	return true;
}

