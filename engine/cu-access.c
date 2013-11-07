/*!
 *	\file	cu-access.c
 *	\brief	compile unit debug information entries processing and access code
 *	\author	shopov
 *
 *	\todo	add range support to compilation units;
 *		this is not even a contrived or a rare
 *		case, an easy to produce test case is
 *		to define functions in different sections
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
#include <stdarg.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "cu-access.h"
#include "srcfile.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "util.h"
#include "dobj-access.h"
#include "type-access.h"
#include "symtab.h"
#include "gprintf.h"


#include <windows.h>

/*
 *
 * local functions follow
 *
 */


/*
 *
 * compile unit type hash operations
 * these are just stubs for the time being...
 *
 *	\todo	actually implement this
 *
 */

/*! \todo document this */
struct cu_hash
{
	/*! the die offset for the described type */
	Dwarf_Off		die_offset;
	/*! pointer to the data type tree built for the type */
	struct cu_data		* cu_data;
	/*! link pointer for the hash table */
	struct cu_hash		* next;
};

/*!
 *	\fn	static struct cu_data * hash_get_cu(struct gear_engine_context * ctx, Dwarf_Off die_offset)
 *	\brief	retrieves a pointer to the compilation unit described by the die at
 *		offset ::die_offset, if present
 *
 *	\todo	actually implement this
 *
 *	\param	ctx	gear engine context data structure, used to access
 *			the compilation unit hash table
 *	\param	die_offset	the offset of the compilation unit of interest
 *	\return	pointer to the data for the compilation unit die at offset ::die_offset, or
 *		null, if the compilation unit is not available
 */

static struct cu_data * hash_get_cu(struct gear_engine_context * ctx, Dwarf_Off die_offset)
{
struct cu_hash * p;

	p = ctx->cus;

	while (p)
	{
		if (p->die_offset == die_offset)
			break;
		p = p->next;
	}
	return p ? p->cu_data : 0;
}

/*!
 *	\fn	static void hash_put_cu(struct gear_engine_context * ctx, Dwarf_Off die_offset, struct cu_data * cu_data)
 *	\brief	brief description
 *
 *	\todo	actually implement this
 *
 *	\param	ctx	gear engine context data structure, used to access
 *			the compilation unit hash table
 *	\param	die_offset	the offset of the compilation unit of interest
 *	\param	cu_data		the data node to put in the hash table
 *	\return	none
 */

static void hash_put_cu(struct gear_engine_context * ctx, Dwarf_Off die_offset, struct cu_data * cu_data)
{
struct cu_hash * p;

	if (hash_get_cu(ctx, die_offset))
		panic("data type already in hash table");
	if (!(p = malloc(sizeof * p)))
		panic("out of core");
	/* just put it in front of the list */
	p->cu_data = cu_data;
	p->die_offset = die_offset;
	p->next = ctx->cus;
	ctx->cus = p;

}

/*!
 *	\fn	static struct cu_data * get_cu_node(void)
 *	\brief	allocates memory for a cu_node structure and initializes it
 *
 *	\return	a pointer to the newly allocated cu_node data structure
 */

static struct cu_data * get_cu_node(void)
{
struct cu_data * p;

	if (!(p = (struct cu_data * )calloc(1, sizeof * p)))
		panic("out of core");
	return p;
}


/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	struct cu_data * cu_process(struct gear_engine_context * ctx, Dwarf_Unsigned cu_die_offset);
 *	\brief	compilation unit access routine
 *
 *	given an offset of a compilation unit in the .debug_info section, this function
 *	builds the tree of this compilation unit and returns the root of this tree
 *
 *	\param	ctx	context used to access debugging information
 *	\param	cu_die_offset	offset of the compilation unit die in the .debug_info section
 *	\return	pointer to the root of the debug information tree built for the compilation unit
 */

struct cu_data * cu_process(struct gear_engine_context * ctx, Dwarf_Unsigned cu_die_offset)
{
struct cu_data * p;
int i;
int res;
Dwarf_Error err;
Dwarf_Die cu_die, child_die, sib_die;
Dwarf_Half tagval;
struct subprogram_data **	subs;
/*! a list of variables in the cu */
struct dobj_data **		vars;
Dwarf_Attribute attr;
Dwarf_Bool flag;

	/* see if the cu die tree is already in the hash table */
	if ((p = hash_get_cu(ctx, cu_die_offset)))
		return p;

	/* make sure this is indeed a compilation unit die */
	/* read the cu die */
	res = dwarf_offdie(ctx->dbg, cu_die_offset, &cu_die, &err);
	if (res != DW_DLV_OK)
		panic("dwarf_offdie()");
	if (dwarf_tag(cu_die, &tagval, &err) != DW_DLV_OK)
		panic("dwarf_tag()");
	if (tagval != DW_TAG_compile_unit)
		panic("fatal: expected a compilation unit die");

	/* allocate memory for the cu tree root */
	p = get_cu_node();

	/* validate the cu die */
	/* check for unsupported attributes */
	if (dwarf_hasattr(cu_die, DW_AT_macro_info, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		printf("warning: panic downgrade; unsupported data object attribute found");
	/* retrieve the address range(s) spanned by the compilation unit */
	/* the dwarf3 standard document, dated December 20, 2005,
	 * section 3.1.1 Normal and Partial Compilation Unit Entries,
	 * on page 38 says:
	 *
	 * Compilation unit entries may have the following attributes:
	 * 1. Either a DW_AT_low_pc and DW_AT_high_pc pair of attributes or a DW_AT_ranges attribute whose values encode the contiguous or non-contiguous address ranges, respectively, of the machine instructions generated for the compilation unit (see Section 2.17).
	 * A DW_AT_low_pc attribute may also be specified in combination with DW_AT_ranges to specify the default base address for use in location lists (see Section 2.6.6) and range lists (see Section 2.17.3).
	 *
	 * buld the address range(s) spanned by the compilation unit,
	 * determine the default base address mentioned above, and
	 * make some checks here */

	/* retrieve the address range(s) covered by this compilation unit */
	p->addr_ranges = dwarf_ranges_get_ranges(ctx, cu_die);

	/* determine the default base address for use in location
	 * lists and range lists - in all cases, this is the value
	 * of the DW_AT_low_pc attribute, which must be present */
	res = dwarf_lowpc(cu_die, &p->default_cu_base_address, &err);
	if (res != DW_DLV_OK)
	{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(cu_die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(cu_die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
		printf("dwarf_lowpc()");
	}
	if (dwarf_hasattr(cu_die, DW_AT_ranges, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		/* DW_AT_ranges attribute present -
		 * make sure only a DW_AT_low_pc attribute is present
		 * (already checked above), and a DW_AT_high_pc attribute
		 * is not present */
		if (dwarf_hasattr(cu_die, DW_AT_high_pc, &flag, &err) != DW_DLV_OK)
			panic("dwarf_hasattr()");
		if (flag)
			panic("");
	}
	else
	{
		/* DW_AT_ranges attribute not present -
		 * make sure both a DW_AT_low_pc attribute is present
		 * (already checked above), and a DW_AT_high_pc
		 * attribute is present */
		if (dwarf_hasattr(cu_die, DW_AT_high_pc, &flag, &err) != DW_DLV_OK)
			panic("dwarf_hasattr()");
		if (!flag)
			panic("");
	}

	/* retrieve the compilation directory for this compilation unit */
	if (dwarf_hasattr(cu_die, DW_AT_comp_dir, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(cu_die, DW_AT_comp_dir, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the attribute's value */
		if (dwarf_formstring(attr, &p->comp_dir, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}

	if (dwarf_hasattr(cu_die, DW_AT_entry_pc, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (0) gprintf("downgraded from panic(): unsupported data object attribute found - check for non-contiguos cu/dw_at_ranges/non-patched gcc (see alexandre oliva patches)");
		/*! \todo	read the comments from the gcc 4.4.3 sources below
		 *		(in the gprintf() call below); when gcc stops emitting
		 *		the improper DW_AT_entry_pc, deprecate it in the gear
		 *		as well... */ 
		if (0) gprintf("rationale: gcc 4.4.3 sources, dwarf2out.c, line 16712; scheduled for removal from the gear:\n\n\n"
      "/* We need to give .debug_loc and .debug_ranges an appropriate\n"
	 "\"base address\".  Use zero so that these addresses become\n"
	 "absolute.  Historically, we've emitted the unexpected\n"
	 "DW_AT_entry_pc instead of DW_AT_low_pc for this purpose\n."
	 "Emit both to give time for other tools to adapt.  *\n"
		       );
	}

	/* check for missing attributes */
	if (dwarf_hasattr(cu_die, DW_AT_stmt_list, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (!flag)
		panic("missing line number information for compilation unit");

	/* fill in data fields */
	p->head.tag = DW_TAG_compile_unit;

	/* retrieve the compilation unit name */
	res = dwarf_diename(cu_die, &p->name, &err);
	if (res != DW_DLV_OK)
		panic("dwarf_diename()");

	printf("processing compilation unit %s\n", p->name);

	/* retrieve line number information */
	if (dwarf_srclines(cu_die, &p->linebuf, &p->linecount, &err) != DW_DLV_OK)
		panic("dwarf_srclines");
	/* retrieve source files for this cu */
	if (dwarf_srcfiles(cu_die, &p->srcfiles, &p->srccount, &err) != DW_DLV_OK)
		panic("dwarf_srcfiles");

	res = dwarf_srclang(cu_die, &p->language, &err);
	if (res != DW_DLV_OK)
		panic("dwarf_srclang()");

	/* ok, proceed to processing the children of this compilation unit */
	res = dwarf_child(cu_die, &child_die, &err);
	switch (res)
	{
		case DW_DLV_OK:
			break;
		case DW_DLV_NO_ENTRY:
			gprintf("this compilation unit does not have children\n");
			dwarf_dealloc(ctx->dbg, cu_die, DW_DLA_DIE);
			return p;
			break;
		case DW_DLV_ERROR:
		default:
			panic("dwarf_child()");
	}

	subs = &p->subs;
	vars = &p->vars;

	while (1)
	{
		struct dwarf_head_struct * pchild;

		pchild = 0;
		if (dwarf_tag(child_die, &tagval, &err) != DW_DLV_OK)
			panic("dwarf_tag()");

		switch (tagval)
		{
			case DW_TAG_variable:
				{
				struct dobj_data * dobj;
				if (0) break;

					dobj = dobj_process(ctx, child_die);
					if (dobj->is_node_a_declaration)
						/* do not add declarations to
						 * the symbol table */
						break;
					pchild = *vars = dobj;
					/* thrust this into the symbol table */
					/* ... but only if the data object just
					 * processed has not already been added
					 * as a child somewhere else (this is
					 * the case, e.g., for static struct/class
					 * members) */
					if (!pchild->parent)
						symtab_store_sym(ctx, (*vars)->name, SYM_DATA_OBJECT, *vars);
					vars = &((*vars)->sib_ptr);
				}
				break;
			case DW_TAG_subprogram:
				{
				struct subprogram_data * subp;
				if (0) break;

					subp = subprogram_process(ctx, child_die);
					if (subp->is_node_a_declaration)
						/* do not add declarations to
						 * the symbol table */
						break;
					pchild = *subs = subp;
					symtab_store_sym(ctx, (*subs)->name, SYM_SUBROUTINE, *subs);
					subs = &((*subs)->sib_ptr);
				}
				break;
			case DW_TAG_base_type:
				if (1) break;
				gprintf("cu base type child found, skipping\n");
				gprintf("*********************this is wrong - handle this properly\n");
				break;

			case DW_TAG_array_type:
			case DW_TAG_subrange_type:
			case DW_TAG_enumeration_type:
			case DW_TAG_pointer_type:
			case DW_TAG_reference_type:
			case DW_TAG_typedef:
			case DW_TAG_string_type:
			case DW_TAG_class_type:
			case DW_TAG_subroutine_type:
			case DW_TAG_unspecified_parameters:
			case DW_TAG_variant:
			case DW_TAG_ptr_to_member_type:
			case DW_TAG_member:
			case DW_TAG_set_type:
			case DW_TAG_access_declaration:
			case DW_TAG_volatile_type:
			case DW_TAG_constant:
			case DW_TAG_file_type:
			case DW_TAG_inheritance:	
			case DW_TAG_template_type_parameter:	
				/*! \todo	code this properly */
				type_process(ctx, child_die);
				break;
			default:
				if (1) break;
				gprintf("unsupported dwarf die tag found: %s(%i)\n",
					dwarf_util_get_tag_name(tagval), tagval);
				gprintf("unknown cu child die tag - %s - skipping panic now - fix this\n",
						dwarf_util_get_tag_name(tagval));
				/*! \todo	code this properly */
				//pchild = type_process(ctx, child_die);
				break;
		}
		/* store the parent of the current child */
		if (pchild && !pchild->parent)
			dwarf_util_set_parent(pchild, p);

		/* get next child */
		res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err);
		dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
		child_die = sib_die;
		switch (res)
		{
			case DW_DLV_OK:
				break;
			case DW_DLV_NO_ENTRY:
				/* save cu entry into hash table and return */
				hash_put_cu(ctx, cu_die_offset, p);
				dwarf_dealloc(ctx->dbg, cu_die, DW_DLA_DIE);
				return p;
				break;
			case DW_DLV_ERROR:
			default:
				panic("dwarf_siblingof()");
		}
		if (0) {
		extern HWND hwnd;
		MSG msg;
		HDC hdc;
		PAINTSTRUCT ps;
		static int idx = 0;
		static char p[] = "|/-\\";
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				printf("msg: %i;", msg.message);
				if (msg.message == WM_PAINT)
					break;
			}
			if (!(hdc = BeginPaint(hwnd, &ps)))
				exit(1);
			TextOut(hdc, 0, 0, p + idx, 1);
			idx ++;
			idx &= 3;
			EndPaint(hwnd, &ps);

		}
	}
}

