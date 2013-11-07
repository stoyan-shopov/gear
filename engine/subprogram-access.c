/*!
 *	\file	subprogram-access.c
 *	\brief	code for handling dwarf subprogram type nodes
 *	\author	shopov
 *
 *	\todo	handle the is_external flag
 *	\todo	support subprogram block-scoped data types
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
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "util.h"
#include "dobj-access.h"
#include "type-access.h"
#include "lexblock-access.h"
#include "dwarf-ranges.h"
#include "dwarf-util.h"

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static struct subprogram_data * get_sub_node(void)
 *	\brief	allocates memory for a subprogram_node structure and initializes it
 *
 *	\return	a pointer to the newly allocated subprogram_node data structure
 */

static struct subprogram_data * get_sub_node(void)
{
struct subprogram_data * p;

	if (!(p = (struct subprogram_data * ) calloc(1, sizeof * p)))
		panic("out of core");
	return p;
}

/*!
 *	\fn	static void sub_link_inlined_subprograms_frame_bases(struct subprogram_data * top_level_subprogram)
 *	\brief	updates the 'fb_location' fields of subprograms expanded inline within the 'top_level_subprogram' subprogram node passed
 *
 *	normally, subprograms expanded inline within another subprogram
 *	do not have a DW_AT_frame_base attribute specifying their
 *	frame bases - because these expanded subprograms actually do not
 *	possess dedicated stackframes, but instead use the frame base
 *	of the topmost subprogram that they have been inline-expanded in;
 *	however, for performing various tasks throughout the gear engine
 *	(such as scope resolution, address location computations, and
 *	other computations that need the frame base information)
 *	it is more convenient and efficient to stash the dwarf frame base
 *	data of topmost subprograms within their (descendant) inline-expanded
 *	subprograms (instead of looking up this frame base information each time
 *	it is needed) - this function does exactly this - it stashes
 *	the frame base data of a topmost subprogram within all of
 *	its descendant inline-expanded subprograms
 *
 *	\param	top_level_subprogram	the toplevel subprogram, for which to
 *					copy the 'fb_location' field into all
 *					of its descendant inline-expanded
 *					subprograms
 *	\return	none */
static void sub_link_inlined_subprograms_frame_bases(struct subprogram_data * top_level_subprogram)
{
	void link_subprograms(struct subprogram_data * dest)
	{

		void link_lexblocks(struct lexblock_data * dest)
		{
			struct subprogram_data * p;
			struct lexblock_data * l;

			for (p = dest->inlined_subprograms; p; p = p->sib_ptr)
			{
				link_subprograms(p);
			}
			for (l = dest->lexblocks; l; l = l->sib_ptr)
			{
				link_lexblocks(l);
			}
		}


		struct subprogram_data * p;
		struct lexblock_data * l;

		dest->is_frame_base_available = top_level_subprogram->is_frame_base_available;
		dest->fb_location = top_level_subprogram->fb_location;

		for (p = dest->inlined_subprograms; p; p = p->sib_ptr)
		{
			link_subprograms(p);
		}
		for (l = dest->lexblocks; l; l = l->sib_ptr)
		{
			link_lexblocks(l);
		}
	}
	link_subprograms(top_level_subprogram);
}


/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	struct subprogram_data * subprogram_process(struct gear_engine_context * ctx, Dwarf_Die die);
 *	\brief	subprogram debug information processing routine
 *
 *	given a subprogram die (be it tagged either DW_TAG_subprogram,
 *	or DW_TAG_subroutine_type), this routine builds the
 *	debugging information tree of the subroutine
 *
 *	\note	this function handles building the dubug information
 *		trees for both DW_TAG_subprogram dies and
 *		DW_TAG_subroutine_type dies
 *
 *	\param	ctx	context used to access the debuggee
 *	\param	die	die of the subprogram
 *	\return	pointer to the root of the debug information tree built for the subprogram
 */
struct subprogram_data * subprogram_process(struct gear_engine_context * ctx, Dwarf_Die die)
{
struct subprogram_data * p;
int res;
Dwarf_Error err;
Dwarf_Bool flag;
Dwarf_Attribute attr;
Dwarf_Die type_die, child_die;
struct dobj_data **	vars;
struct dobj_data **	params;
struct subprogram_data **	inlined_subprograms;
struct lexblock_data ** lexblocks;
Dwarf_Half tagval;

	/* see if this is indeed a subprogram die */
	if (dwarf_tag(die, &tagval, &err) != DW_DLV_OK)
		panic("dwarf_tag()");
	if (tagval != DW_TAG_subprogram
			&& tagval != DW_TAG_subroutine_type
			&& tagval != DW_TAG_inlined_subroutine)
		panic("invalid subprogram die tag");

	/* check for unsupported attributes */
	if (dwarf_hasattr(die, DW_AT_const_value, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported attribute found");
	if (dwarf_hasattr(die, DW_AT_prototyped, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		//panic("unsupported data object attribute found");
		gprintf("must be fixed: unsupported data object attribute found");
	if (dwarf_hasattr(die, DW_AT_pure, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");
	if (dwarf_hasattr(die, DW_AT_return_addr, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");

	if (dwarf_hasattr(die, DW_AT_static_link, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");

	/* get core for the new node */
	p = get_sub_node();

	p->head.tag = tagval;
	/* first of all, see if this subprogram die is a (concrete) inline
	 * expansion of an (abstract) inline subroutine
	 *
	 * *or*
	 *
	 * this is a (concrete) out-of-line expansion of an (abstract)
	 * inline subroutine
	 *
	 * there is a subtle, but significant, difference between
	 * these two cases; in both of them, the subprogram die
	 * *will* have a DW_AT_abstract_origin
	 * attribute, there are, however, notable differences:
	 *	- in the case of a (concrete) inline expansion of an (abstract)
	 *	inlined subroutine, this very now processed subprogram
	 *	die *will* also possess an DW_AT_inline attribute, and its
	 *	tag *will* equal DW_TAG_inlined_subroutine; see the dwarf
	 *	3 standard document, paragraph 3.3.8.2 for details
	 *
	 *	whereas,
	 *
	 *	- in the case of a (concrete) out-of-line expansion of
	 *	an (abstract) inline subroutine, this very now processed
	 *	sibrogram die *is not* obliged to possess a DW_AT_inline
	 *	attribute, and - this is the important thing - its tag
	 *	*is not* obliged to equal DW_TAG_inlined_subroutine
	 *	(and will, indeed, in practice, equal simply a
	 *	DW_TAG_subprogram - this is confirmed by both practice,
	 *	and examples in the dwarf 3 standard document); see
	 *	the dwarf 3 standard document, paragraph 3.3.8.4
	 *	for details
	 *
	 *	all this said, in the statement below, done is this:
	 *	- if this very now processed subprogram node possesses
	 *	the attribute DW_AT_abstract_origin (which is true in both
	 *	of the cases described above), the fields of the abstractly
	 *	originated die referenced by this very now processed die
	 *	are copied (verbatim) from the abstractly originated die
	 *	into this very now processed die, and in the process of doing
	 *	this, neither the tag of this very now processed die, nor
	 *	any other (owned by whomever) dwarf attributes; these
	 *	actions are, however, not appropriate for both of the cases
	 *	described above
	 *
	 *	so:
	 *
	 *	then, here, the fields of this very now processed
	 *	subprogram die node - which are sensitive to the context
	 *	of the cases described above, are reverted to safe values;
	 *	these fields are the ones that tell whether/how inlining
	 *	is in effect
	 *
	 *	to sum this up: read the source, it should be clear,
	 *	i rant so much, because this is important, be ware of the
	 *	artefacts described herein; in short, here i(sgs) do not
	 *	discriminate between the cases above, but rather try
	 *	to play safe... */
	if (dwarf_hasattr(die, DW_AT_abstract_origin, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Off offset;
		Dwarf_Die abstract_die;
		struct subprogram_data * abstract_node;

		/* if source code function call coordinates are
		 * present, retrieve them */
		if (dwarf_hasattr(die, DW_AT_call_file, &flag, &err) != DW_DLV_OK)
			panic("dwarf_hasattr()");
		if (flag)
		{
			if (dwarf_attr(die, DW_AT_call_file, &attr, &err) != DW_DLV_OK)
				panic("dwarf_attr()");
			if (dwarf_formudata(attr, &p->call_file, &err) != DW_DLV_OK)
				panic("dwarf_formudata()");
			dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		}
		if (dwarf_hasattr(die, DW_AT_call_line, &flag, &err) != DW_DLV_OK)
			panic("dwarf_hasattr()");
		if (flag)
		{
			if (dwarf_attr(die, DW_AT_call_line, &attr, &err) != DW_DLV_OK)
				panic("dwarf_attr()");
			if (dwarf_formudata(attr, &p->call_line, &err) != DW_DLV_OK)
				panic("dwarf_formudata()");
			dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		}

		/* retrieve the associated abstract instance
		 * die offset */
		if (dwarf_attr(die, DW_AT_abstract_origin, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
			panic("dwarf_global_formref()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (dwarf_offdie(ctx->dbg, offset, &abstract_die, &err) != DW_DLV_OK)
			panic("dwarf_offdie()");
		if (!(abstract_node = subprogram_process(ctx, abstract_die)))
			panic("");
		dwarf_dealloc(ctx->dbg, abstract_die, DW_DLA_DIE);
		/* copy the fields of interest */
		/*! \todo	this is incorrect/incomplete - other fields
		 *		must be fixed up as well (e.g. the formal
		 *		parameters, etc. */
		*p = *abstract_node;

		p->params = 0;
		p->vars = 0;
		p->lexblocks = 0;
		p->inlined_subprograms = 0;

		/* fix up fields */
		p->is_abstract_inline_instance_root
			= 0;

	}
	if (dwarf_hasattr(die, DW_AT_MIPS_linkage_name, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_MIPS_linkage_name, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the flag's value */
		if (dwarf_formstring(attr, &p->linkage_name, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}
	/* the dwarf standard, paragraph 2.13.2 says:
	 *
	 * 2.13.2 Declarations Completing Non-Defining Declarations
	 * Debugging information entries that represent a
	 * declaration that completes another (earlier)
	 * non-defining declaration, may have a DW_AT_specification
	 * attribute whose value is a reference to the debugging
	 * information entry representing the non-defining declaration.
	 * Debugging information entries with a DW_AT_specification attribute
	 * do not need to duplicate information provided by the debugging
	 * information entry referenced by that specification attribute.
	 *
	 * do just that here - see if the current node completes
	 * another (non-defining) declaration, and if so - copy all
	 * of the fields of the other (non-defining) declaration
	 * into this new node, and then process the attributes
	 * of this new node, thus complementing and/or overriding
	 * the fields copied from the other (non-defining) declaration */

	/* see if the subprogram die completes another,
	 * non-defining declaration */
	if (dwarf_hasattr(die, DW_AT_specification, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Off offset;
		Dwarf_Die decl_die;
		struct subprogram_data * decl_node;
		/* sanity check */
		if (dwarf_hasattr(die, DW_AT_declaration, &flag, &err) != DW_DLV_OK)
			panic("dwarf_hasattr()");
		if (flag)
			panic("");

		if (dwarf_attr(die, DW_AT_specification, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the associated non-defining declaration
		 * die offset */
		if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
			panic("dwarf_global_formref()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (dwarf_offdie(ctx->dbg, offset, &decl_die, &err) != DW_DLV_OK)
			panic("dwarf_offdie()");
		if (!(decl_node = subprogram_process(ctx, decl_die)))
			panic("");
		dwarf_dealloc(ctx->dbg, decl_die, DW_DLA_DIE);
		/* see if the referenced die is well-formed */
		if (!decl_node->is_node_a_declaration
				|| decl_node->head.tag != tagval)
			panic("");
		/* ok, copy the fields of the associated
		 * non-defining declaration into the current node */
		*p = *decl_node;
		/* only this field should be fixed */
		p->is_node_a_declaration = false;
	}

	/* see if this is an abstract inline subprogram instance root */
	if (dwarf_hasattr(die, DW_AT_inline, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Unsigned	inline_code;

		if (dwarf_attr(die, DW_AT_inline, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formudata(attr, &inline_code, &err) != DW_DLV_OK)
			panic("dwarf_formudata()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		p->dwarf_inline_code = inline_code;
		p->is_abstract_inline_instance_root = true;
	}
	/* validate inline flags */
	if (p->is_abstract_inline_instance_root
			&& p->is_concrete_inline_instance_root)
		panic("");
	/* see if this is a declaration */
	if (dwarf_hasattr(die, DW_AT_declaration, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_declaration, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the flag's value */
		if (dwarf_formflag(attr, &flag, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (flag)
			p->is_node_a_declaration = true;
	}

	/* store the address range(s) covered by this subprogram */
	p->addr_ranges = dwarf_ranges_get_ranges(ctx, die);

	/* determine declaration coordinates */
	/* extract source code file number */
	if (dwarf_hasattr(die, DW_AT_decl_file, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_decl_file, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formudata(attr, &p->srcfile_nr, &err) != DW_DLV_OK)
			panic("dwarf_formudata()");
	}
	/* extract source code line number */
	if (dwarf_hasattr(die, DW_AT_decl_line, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_decl_line, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formudata(attr, &p->srcline_nr, &err) != DW_DLV_OK)
			panic("dwarf_formudata()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}

	/* store frame base location information for the subroutine */
	if (dwarf_hasattr(die, DW_AT_frame_base, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_frame_base, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the attribute's value */
		if (dwarf_loclist_n(attr, &p->fb_location.llbuf, &p->fb_location.listlen, &err) != DW_DLV_OK)
		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			printf("dwarf_loclist_n()");
			printf("\nABCDEFGXXX\ndwarf_loclist_n()");
		}
		else
		{
			p->is_frame_base_available = true;
			gprintf("frame base is available\n");
		}
	}
	/* record the entry point address (if available) for this subprogram */
	if (dwarf_lowpc(die, &p->entry_pc, &err) == DW_DLV_OK)
		p->is_entry_point_valid = 1;

	/* however, if this die has a DW_AT_entry_pc, make its
	 * value override, because the dwarf4 standard says:

Any debugging information entry describing an entity that has a range of code addresses, which includes compilation units, module initialization, subroutines, ordinary blocks, try/catch blocks, and the like, may have a DW_AT_entry_pc attribute to indicate the first executable instruction within that range of addresses. The value of the DW_AT_entry_pc attribute is a relocated address. If no DW_AT_entry_pc attribute is present, then the entry address is assumed to be the same as the value of the DW_AT_low_pc attribute, if present; otherwise, the entry address is unknown.

	 */
	if (dwarf_hasattr(die, DW_AT_entry_pc, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_entry_pc, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formaddr(attr, &p->entry_pc, &err) != DW_DLV_OK)
			panic("dwarf_formaddr()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		p->is_entry_point_valid = 1;
	}

	/* make sure a frame base is present in cases when it should be */
	if (tagval !=	/* a frame base must be present if no type-only entries
			 * are being handled... */ DW_TAG_subroutine_type
			/* ... and this is not a declaration */
			&& !p->is_node_a_declaration
			&& (/* ... and if not inlined subroutines are being processed... */
				(p->is_abstract_inline_instance_root && p->is_frame_base_available)
			/* inline-expanded subprograms do not normally have frame
			 * base information, because they use the frame
			 * base information of their toplevel containing
			 * subprogram (i.e. the one that they have
			 * been inline-expanded into), and this frame
			 * base information will later be copied to
			 * them (below) by the sub_link_inlined_subprograms_frame_bases()
			 * funtion

			   || (!p->is_abstract_inline_instance_root && !p->is_frame_base_available)

			 */
				))
	{
{ Dwarf_Off x; if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic(""); printf("cu relative offset: %i\n", (int) x); }
	gprintf("frame base is: %s\n", p->is_frame_base_available ? "available":"unavailable");
	gprintf("abstract inline root: %s\n", p->is_abstract_inline_instance_root ? "true":"false");
	gprintf("concrete inline root: %s\n", p->is_concrete_inline_instance_root ? "true":"false");
		panic("");
	}

	/* see if this is an externally visible symbol */
	if (dwarf_hasattr(die, DW_AT_external, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_external, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formflag(attr, &flag, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (flag)
			p->is_externally_visible = true;
	}

	/* retrieve the subprogram return type */
	if (1 && dtype_access_get_type_die(ctx, die, &type_die))
	{
		/* the subprogram does return a result of some type */
		p->type = type_process(ctx, type_die);
		dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);
	}

	/* retrieve the subprograms name */
	if (dwarf_diename(die, &p->name, &err) != DW_DLV_OK)
	{
		//panic("dwarf_diename()");
		gprintf("skipping panic(), downgraded from panic(): dwarf_diename()");
	}

	/* process children of the subprogram die */
	params = &p->params;
	vars = &p->vars;
	lexblocks = &p->lexblocks;
	inlined_subprograms = &p->inlined_subprograms;
	res = dwarf_child(die, &child_die, &err);

	/* process the children (if any) of the suprogram die */
	if (res == DW_DLV_ERROR)
		panic("dwarf_child()");
	else if (res != DW_DLV_NO_ENTRY)
	{
		do
		{
			Dwarf_Die	sib_die;
			void * pchild;

			pchild = 0;
			if (dwarf_tag(child_die, &tagval, &err) != DW_DLV_OK)
				panic("dwarf_tag()");
			switch (tagval)
			{
				case DW_TAG_variable:
					if (0) break;
					pchild = *vars = dobj_process(ctx, child_die);
					vars = &((*vars)->sib_ptr);
					break;
				case DW_TAG_formal_parameter:
					if (0) break;
					pchild = *params = dobj_process(ctx, child_die);
					params = &((*params)->sib_ptr);
					break;
				case DW_TAG_lexical_block:
					if (0) break;
					pchild = *lexblocks = lexblock_process(ctx, child_die);
					lexblocks = &((*lexblocks)->sib_ptr);
					break;
				case DW_TAG_inlined_subroutine:
					if (0) break;
					pchild = *inlined_subprograms = subprogram_process(ctx, child_die);
					inlined_subprograms = &((*inlined_subprograms)->sib_ptr);
					break;
				case DW_TAG_label:
					if (0) break;
					printf("XXXXXXXXXXXXXX\n");
					printf("downgraded from panic: handle subprogram DW_TAG_unspecified_parameters tag\n");
					break;
				case DW_TAG_unspecified_parameters:
					if (0) break;
					printf("XXXXXXXXXXXXXX\n");
					printf("downgraded from panic: handle subprogram DW_TAG_unspecified_parameters tag\n");
					break;
				default:
					gprintf("%s(): downgraded from panic(), skipping panic() now: invalid subprogram child die: ", __func__);
					printf("unsupported tag %s\n", dwarf_util_get_tag_name(tagval));
					if (0) panic("");
					break;
			}
			dwarf_util_set_parent(pchild, p);

			if ((res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err)) == DW_DLV_ERROR)
				panic("dwarf_siblingof()");
			dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
			child_die = sib_die;
		}
		while (res == DW_DLV_OK);
		/* copy frame base information to any contained
		 * inline-expanded subprograms */
		/*! \todo	if this is just a declaration,
		 *		this is not needed, should anything
		 *		special be done here? */
		if (p->head.tag == DW_TAG_subprogram)
			sub_link_inlined_subprograms_frame_bases(p);
	}

	if (p->is_node_a_declaration)
		type_access_stats.nr_subprogram_prototype_nodes ++;
	else	
		type_access_stats.nr_subprogram_nodes ++;

	return p;
}

