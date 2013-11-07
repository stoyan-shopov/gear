/*!
 *	\file	lexblock-access.c
 *	\brief	lexical block entries processing and access code
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "util.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "dwarf-ranges.h"
#include "type-access.h"
#include "dobj-access.h"
#include "subprogram-access.h"
#include "lexblock-access.h"

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static struct lexblock_data * get_lexblock_node(void)
 *	\brief	allocates memory for a lexblock_data structure and initializes it
 *
 *	\return	a pointer to the newly allocated lexblock_data data structure
 */

static struct lexblock_data * get_lexblock_node(void)
{
struct lexblock_data * p;

	if (!(p = (struct lexblock_data * )malloc(sizeof(struct lexblock_data))))
		panic("out of core");
	memset(p, 0, sizeof(*p));
	return p;
}


/*
 *
 * exported functions follow
 *
 */


/*!
 *	\fn	struct lexblock_data * lexblock_process(struct gear_engine_context * ctx, Dwarf_Die die);
 *	given a dwarf die, builds the information tree for the lexical block described by the die
 *
 *	\param	ctx	context used to access debugging information about
 *			the debuggee
 *	\param	die	the die of interest
 *	\return	pointer to the root of the debug information tree built for the
 *		lexcical block of the die
 */
struct lexblock_data * lexblock_process(struct gear_engine_context * ctx, Dwarf_Die die)
{
struct lexblock_data * p;
Dwarf_Half tagval;
Dwarf_Error err;
Dwarf_Die child_die;
struct dobj_data **	vars;
struct lexblock_data ** lexblocks;
struct subprogram_data ** inlined_subprograms;
int res;
Dwarf_Bool flag;

	/* see if this is indeed a lexical block die */
	if (dwarf_tag(die, &tagval, &err) != DW_DLV_OK)
		panic("dwarf_tag()");
	if (tagval != DW_TAG_lexical_block)
		panic("invalid lexical block die tag");

	/* check for unsupported attributes */
	if (dwarf_hasattr(die, DW_AT_abstract_origin, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported attribute found");

	/* get core for the new node */
	p = get_lexblock_node();
	p->head.tag = tagval;

	/* retrieve the address range(s) (if any) covered by this lexical block */
	p->addr_ranges = dwarf_ranges_get_ranges(ctx, die);

	/* process children of the lexical block */
	vars = &p->vars;
	lexblocks = &p->lexblocks;
	inlined_subprograms = &p->inlined_subprograms;

	res = dwarf_child(die, &child_die, &err);

	if (res == DW_DLV_ERROR)
		panic("dwarf_child()");

	while (res != DW_DLV_NO_ENTRY)
	{
		Dwarf_Die sib_die;
		void * pchild;

		pchild = 0;
		if (dwarf_tag(child_die, &tagval, &err) != DW_DLV_OK)
			panic("dwarf_tag()");
		switch (tagval)
		{
			case DW_TAG_variable:
				pchild = *vars = dobj_process(ctx, child_die);
				vars = &((*vars)->sib_ptr);
				break;
			case DW_TAG_lexical_block:
				pchild = *lexblocks = lexblock_process(ctx, child_die);
				lexblocks = &((*lexblocks)->sib_ptr);
				break;
			case DW_TAG_label:
				/*! \todo	do labels need any special handling here??? */
				break;
			case DW_TAG_inlined_subroutine:
				pchild = *inlined_subprograms = subprogram_process(ctx, child_die);
				inlined_subprograms = &((*inlined_subprograms)->sib_ptr);
				break;
			case DW_TAG_subprogram:
				/* gcc (at least version 4.5.2) can generate function prototype
				 * dies for function calls in the source when there is no
				 * function prototype encountered in the source so far
				 * for the function being called; allow
				 * and ignore such dies */
				if (dwarf_hasattr(child_die, DW_AT_declaration, &flag, &err) != DW_DLV_OK)
					panic("dwarf_hasattr()");
				if (!flag)
					panic("unsupported lexblock die found");
				break;
			default:
				printf("invalid lexblock child die\n");
				printf("%s\n", dwarf_util_get_tag_name(tagval));
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
				gprintf("panic downgrade: lexblock_process()");
				break;
		}
		dwarf_util_set_parent(pchild, p);

		if ((res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err)) == DW_DLV_ERROR)
			panic("dwarf_child()");
		dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
		child_die = sib_die;
	}
	while (res == DW_DLV_OK);

type_access_stats.nr_lexblocks ++;

	return p;
}

