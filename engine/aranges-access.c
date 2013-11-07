/*!
 * \file	aranges-access.c
 * \brief	code for dealing with the .debug_aranges section
 * \author	shopov
 *
 *	basically, this is a helper for quickly locating an execution context
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

#include <stdlib.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "cu-access.h"
#include "subprogram-access.h"
#include "lexblock-access.h"
#include "util.h"
#include "aranges-access.h"
#include "dwarf-ranges.h"

/*
 *
 * local data types follows
 *
 */

/*! compilation unit address range descriptor structure */
struct cu_arange_desc_struct
{
	/*! the offset of the die for the compilation unit for the given descriptor */
	Dwarf_Off	cu_die_offset;
	/*! start core address for the address range for this descriptor */
	ARM_CORE_WORD		addr;
	/*! size of this address range, in bytes */
	ARM_CORE_WORD		len;
};

/*! address range data structure for an executable
 *
 * this contains a preprocessed view of the address ranges
 * of an executable as described in the .debug_aranges section
 * of the executable; the address range table is built with the
 * help of the .debug_aranges helper routines in libdwarf */ 
struct cu_aranges
{
	/*! address range table, an array of compilation unit address range descriptors */
	struct cu_arange_desc_struct		* range_tab;
	/*! the number of elements in the range_tab array above */
	int					range_cnt;
};



/*
 *
 * local functions follow
 *
 */


/*!
 *	\fn	static void find_subprogram_and_lexblock_for_addr(struct gear_engine_context * ctx, struct subprogram_data * subp, int addr, struct cu_data * cu, struct subprogram_data ** ret_subp, struct lexblock_data ** ret_lexblock)
 *	\brief	for a given subprogram node and a given address of interest, returns the most deeply nested lexical block (if any) and inline-expanded suprogram (if any) nodes containing the given address
 *
 *	\note	the basic mechanism of operation of this function
 *		is outlined in the comments below in the function
 *
 *	\param	ctx	gear engine context to work in
 *	\param	subp	the subprogram of interest in which to scan
 *			for lexical blocks and inline-expanded
 *			subprograms containing the given address
 *	\param	addr	address of interest for which to scan this
 *			subprogram for descendant lexical blocks and
 *			inline-expanded subprograms
 *	\param	cu	compilation unit containing the given
 *			subprogram, needed for retrieving the
 *			value of the compilation unit base address
 *			used in address range computations
 *	\param	ret_subp	a pointer to a variable in which
 *				to store the most deeply nested
 *				inline-expanded subprogram node of
 *				this subprogram,
 *				containing the given address
 *	\param	ret_lexblock	a pointer to a variable in which
 *				to store the most deeply nested
 *				lexical block node of
 *				this subprogram,
 *				containing the given address
 *	\return	none */
static void find_subprogram_and_lexblock_for_addr(struct gear_engine_context * ctx,
		struct subprogram_data * subp,
		int addr,
		struct cu_data * cu,
		struct subprogram_data ** ret_subp,
		struct lexblock_data ** ret_lexblock)
{
#if 0
	void scan_subp(struct subprogram_data * subp)
	{

		void scan_lexblock(struct lexblock_data * lexblock)

		{
			struct lexblock_data * l;
			struct subprogram_data * s;

			if (dwarf_ranges_is_in_range(ctx, lexblock->addr_ranges, addr,
						cu->default_cu_base_address))
				* ret_lexblock = lexblock;
			for (l = lexblock->lexblocks; l; l = l->sib_ptr)
				scan_lexblock(l);
			for (s = lexblock->inlined_subprograms; s; s = s->sib_ptr)
				scan_subp(s);

		}

		struct lexblock_data * l;
		struct subprogram_data * s;

		gprintf("%s(): scanning subprogram node %s\n", __func__, subp->name);
		if (dwarf_ranges_is_in_range(ctx, subp->addr_ranges, addr,
					cu->default_cu_base_address))
			* ret_subp = subp;
		for (l = subp->lexblocks; l; l = l->sib_ptr)
			scan_lexblock(l);
		for (s = subp->inlined_subprograms; s; s = s->sib_ptr)
			scan_subp(s);

	}
	* ret_subp = 0;
	* ret_lexblock = 0;

	if (subp)
		scan_subp(subp);

#else

	struct subprogram_data * s, * saved_subp;
	struct lexblock_data * l, * saved_lexblock;

	if (!subp || !dwarf_ranges_is_in_range(ctx, subp->addr_ranges, addr,
				cu->default_cu_base_address))
	{
		* ret_subp = 0;
		* ret_lexblock = 0;
		return;
	}

	saved_subp = subp;
	saved_lexblock = 0;
	/* this subprogram may have inline-expanded subprogram nodes,
	 * some of which actually contain the address of interest;
	 * dive in this subprogram looking for any such
	 * inline-expanded subprograms
	 *
	 * as these subprograms may actually be not direct children
	 * of this subprogram node, but rather children of lexical
	 * blocks belonging to this subprogram node, also handle
	 * any lexical blocks contained within this subprogram node
	 *
	 * the loop below consists of four basic steps; starting from within
	 * an address-matching subprogram node:
	 *	(1) scanning immediate descendant inline-expanded
	 *	subprogram nodes, if a match has been found, repeat
	 *	the scan, until no new match has been found; then,
	 *	in the last matching inline-expanded subprogram perform:
	 *		(2) scanning immediate descendant lexical block nodes,
	 *		in the last matching lexical block perform:
	 *			(3) scanning of immediate descendant
	 *			lexical block nodes, and - as in step 1 -
	 *			repeat the scan until no new match has
	 *			been found; then, in the last
	 *			matching node perform:
	 *				(4) scanning of immediate descendant
	 *				inline-expanded subprogram nodes
	 *
	 * so at the end of the last (fourth) step, what is obtained
	 * is an inline-expanded subprogram node, to which these
	 * four steps are successively applied until no further
	 * progress is possible - no further progress is possible
	 * if at the end of the second step no lexical block has
	 * been found, or at the end of the fourth step no subprogram
	 * node has been found */
	do
	{
		gprintf("%s(): scanning subprogram %s\n", __func__, saved_subp->name);
		/* first, scan direct inline-expanded subprogram descendants
		 * of the last subprogram matched, and repeat the scan on
		 * the matched subprograms until no more matches are found */
		do
		{
			for (s = saved_subp->inlined_subprograms; s; s = s->sib_ptr)
			{
				gprintf("%s(): scanning child subprogram %s\n", __func__, s->name);
				if (dwarf_ranges_is_in_range(ctx, s->addr_ranges, addr,
							cu->default_cu_base_address))
				{
					/* match found */
					gprintf("%s(): matched child subprogram %s\n", __func__, s->name);
					saved_subp = s;
					break;
				}
			}
		}
		while (s);

		/* second, scan the last matching subprogram's lexical blocks (if any) */
		for (l = saved_subp->lexblocks; l; l = l->sib_ptr)
			if (dwarf_ranges_is_in_range(ctx, l->addr_ranges, addr,
						cu->default_cu_base_address))
			{
				/* match found */
				saved_lexblock = l;
				break;
			}

		if (!l)
			/* no further progress possible */
		{
			/* shopov 02082010: however, there are some quirks here...
			 * i(sgs) could not really find in the dwarf4 standard
			 * any explicit notion of the intuitively reasonable
			 * assumption that any dwarf die-s that have code
			 * address range information associated with them
			 * (be it a DW_AT_low_pc/DW_AT_high_pc pair or a
			 * DW_AT_ranges attribute), and which contain other
			 * die-s that also have code address information,
			 * should actually have all of their children die-s'
			 * code address ranges contained within their - i.e.
			 * the common parent's - code address ranges; in short,
			 * i could not find any notion in the dwarf4 standard
			 * that a die that has code address ranges will contain
			 * in these ranges all of the ranges of any children
			 * die-s that also have address ranges...
			 *
			 * some torture tests were ran with gcc 4.5.0 on this
			 * code sample:

			 volatile int x;

			 .....


			 int fhack(int (*fptr_1)(int ccc))
			 {
			 return x + x - 10 * fptr_1(x++ + ++x);
			 }

			 static inline int f0(void)
			 {
			 if (x == 0)
			 {
			 __label__ failure;
			 volatile int * xxx = &x;
			 inline int fff(int i)
			 {
			 if (!i)
			 goto failure;
			 return x + i;
			 }
			 return fff(*xxx) * fff(10 + xxx[1]) / fff(x - 1) - fhack(fff);
failure:
return 10;
}
else
{
volatile int * yyy = &x;
if (!x && *yyy)
return *yyy;
else
return 100;
}
}

			 * now, the dwarf generated by gcc had a
			 * DW_AT_ranges attribute generated for the
			 * lexical block containing the nested fff()
			 * function, and this lexical block's address
			 * range did not contain the address range
			 * generated for fff(), therefore the tests
			 * in these function will fail and the fff()
			 * function will not be reached in the scans
			 * in this function... however - one thing
			 * is very important - the function is nested,
			 * but *not* inlined - so i(sgs) am not at all
			 * sure what should be done in this case...
			 *
			 * as i(sgs) could not find any formal indication
			 * that what gcc does is non-conforming with the
			 * standard, i(sgs) add here a few more tests
			 * to hopefully handle cases as the one described
			 * above better... */
		/*! \todo	not actually done right now, code this... */
		break;
		}
/* third, scan direct lexical blocks of the last
 * lexical block matched, and repeat the scan until
 * no more matches are found */
do
{
	for (l = saved_lexblock->lexblocks; l; l = l->sib_ptr)
		if (dwarf_ranges_is_in_range(ctx, l->addr_ranges, addr,
					cu->default_cu_base_address))
		{
			/* match found */
			saved_lexblock = l;
			break;
		}
}
while (l);

	for (s = saved_lexblock->inlined_subprograms; s; s = s->sib_ptr)
if (dwarf_ranges_is_in_range(ctx, s->addr_ranges, addr,
			cu->default_cu_base_address))
{
	saved_subp = s;
	break;
}
}
while (s);

* ret_subp = saved_subp;
* ret_lexblock = saved_lexblock;
#endif
}

/*
 *
 * exported functions follow
 *
 */


/*!
 *	\fn	struct cu_data * aranges_get_cu_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	locates which compilation unit contains a given address
 *
 *	given an address, retrieves the compilation unit that contains
 *	text for the given address; this one is used for execution context
 *	determination
 *
 *	\param	ctx	gear engine context
 *	\param	addr	the address of interest
 *	\return	pointer to the cu_data structure for the compilation unit containing the
 *		given address or null, if no such compilation unit is found
 */

struct cu_data * aranges_get_cu_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
struct cu_arange_desc_struct * p;
int i;

	p = ctx->cu_aranges->range_tab;
	i = ctx->cu_aranges->range_cnt;
	while (i)
	{
		if (p->addr <= addr && addr < p->addr + p->len)
			break;
		p++;
		i--;
	}
	if (!i)
		return 0;

	return cu_process(ctx, p->cu_die_offset);
}


/*!
 *	\fn	struct subprogram_data * aranges_get_subp_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	locates which subprogram contains a given address
 *
 *	given an address, retrieves the subprogram within the compilation
 *	unit that contains the address; this one is used for execution context
 *	determination
 *
 *	\param	ctx	gear engine context
 *	\param	addr	the address of interest
 *	\return	pointer to the subprogram_data structure for the function containing the
 *		given address or null, if no such function is found
 */
struct subprogram_data * aranges_get_subp_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
struct cu_data * cu;
struct subprogram_data * subp;
struct lexblock_data * dummy_lexblock;

	if (!(cu = aranges_get_cu_for_addr(ctx, addr)))
		/* compilation unit not found */
		return 0;
	/* ok, locate the subprogram within the compilation unit */
	subp = cu->subs;

	while (subp)
	{
		if (dwarf_ranges_is_in_range(ctx, subp->addr_ranges, addr,
					cu->default_cu_base_address))
			/* match found */
			break;
		subp = subp->sib_ptr;
	}
	return subp;

	find_subprogram_and_lexblock_for_addr(ctx, subp, addr, cu, &subp, &dummy_lexblock);
	return subp;
}

/*!
 *	\fn	struct lexblock_data * aranges_get_lexblock_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
 *	\brief	locates which subprogram contains a given address
 *
 *	given an address, retrieves the lexical block within
 *	the subroutine that contains the address; this one is used for
 *	execution context determination
 *
 *	\param	ctx	gear engine context
 *	\param	addr	the address of interest
 *	\return	pointer to the lexblock_data structure for the
 *		lexical block containing the given address or null,
 *		if no such lexical block is found
 */
struct lexblock_data * aranges_get_lexblock_for_addr(struct gear_engine_context * ctx, ARM_CORE_WORD addr)
{
struct cu_data * cu;
struct subprogram_data * subp;
struct lexblock_data * lexblock;

	if (!(cu = aranges_get_cu_for_addr(ctx, addr)))
		/* compilation unit not found */
		return 0;
	if (!(subp = aranges_get_subp_for_addr(ctx, addr)))
		/* subprogram not found */
		return 0;
	find_subprogram_and_lexblock_for_addr(ctx, subp, addr, cu, &subp, &lexblock);

	return lexblock;
}

/*!
 *	\fn	void init_aranges(struct gear_engine_context * ctx)
 *	\brief 	initializes aranges data for an image file
 *
 *	\param	ctx	gear engine context
 *	\return	none
 */

void init_aranges(struct gear_engine_context * ctx)
{
int res;
Dwarf_Error err;
Dwarf_Arange	* aranges;
Dwarf_Signed	nr_aranges;
struct cu_aranges	* cu_aranges;
int i, j, k;

int cu_aranges_compare(struct cu_arange_desc_struct * arg1, struct cu_arange_desc_struct * arg2)
{
	if (arg1->addr == arg2->addr)
		return 0;
	if (arg1->addr < arg2->addr)
		return -1;
	return 1;
}

	res = dwarf_get_aranges(ctx->dbg, &aranges, &nr_aranges, &err);
	if (res == DW_DLV_ERROR)
		panic("dwarf_aranges");
	if (res == DW_DLV_NO_ENTRY)
		panic("image does not have a .debug_aranges section, doesnt know what to do");
	gprintf("%s(): aranges data successfully initialized, found a total of %i aranges\n",
			__func__, nr_aranges);
	/* get core for the address ranges table */
	if (!(cu_aranges = malloc(sizeof * cu_aranges)))
		panic("");
	if (!(cu_aranges->range_tab = malloc(nr_aranges * sizeof * cu_aranges->range_tab)))
		panic("");
	cu_aranges->range_cnt = nr_aranges;
	for (i = 0; i < nr_aranges; i++)
	{
		Dwarf_Addr start;
		Dwarf_Unsigned len;
		Dwarf_Off cu_die_offset;

		if (dwarf_get_arange_info(aranges[i], &start, &len,
					&cu_die_offset, &err) != DW_DLV_OK)
			panic("dwarf_get_arange_info()");
		cu_aranges->range_tab[i].cu_die_offset = cu_die_offset;
		cu_aranges->range_tab[i].addr = (ARM_CORE_WORD) start;
		cu_aranges->range_tab[i].len = (ARM_CORE_WORD) len;
		if (0) gprintf("%s(): cu_die %i, start 0x%08x, end 0x%08x\n", __func__,
			(int) cu_die_offset,
			(int) start,
			(int) start + len);
		/* deallocate storage allocated by libdwarf */
		dwarf_dealloc(ctx->dbg, aranges[i], DW_DLA_ARANGE);
	}
	/* deallocate storage allocated by libdwarf */
	dwarf_dealloc(ctx->dbg, aranges, DW_DLA_LIST);
	/* sort the list by starting core address, in ascending order */
	qsort(cu_aranges->range_tab, nr_aranges, sizeof * cu_aranges->range_tab, (int (*)(const void *, const void *)) cu_aranges_compare);
	/* use simple selection sort */
	/*!	\todo	the address ranges list will most commonly be already
	 *		sorted; maybe check for this, wont make much difference
	 *		anyway... */

#if 0
	for (i = 0; i < nr_aranges; i++)
	{
		for (k = j = i; j < nr_aranges; j++)
			if (cu_aranges->range_tab[j].addr < 
					cu_aranges->range_tab[k].addr)
				k = j;
		if (k != i)
		{
			struct cu_arange_desc_struct tmp;
			tmp = cu_aranges->range_tab[i];
			cu_aranges->range_tab[i] = cu_aranges->range_tab[k];
			cu_aranges->range_tab[k] = tmp;
		}
	}
#endif	
	ctx->cu_aranges = cu_aranges;
}

