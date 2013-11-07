/*!
 *	\file	scope.c
 *	\brief	debug information symbol scope related routines
 *	\author	shopov
 *
 *	\todo	properly handle subroutine block-scoped types
 *	\todo	handle types; currently types are not handled at all
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

#include "dwarf-common.h"
#include "target-defs.h"
#include "engine-err.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "dobj-access.h"
#include "cu-access.h"
#include "lexblock-access.h"
#include "subprogram-access.h"
#include "symtab.h"
#include "util.h"
#include "scope.h"
#include "aranges-access.h"
#include "dwarf-util.h"

/*
 *
 * local functions follow
 *
 */ 


/*!
 *	\deprecated	this function is deprecated, it handles just data object names
 *			and not identifiers in general (e.g. type names) - transform
 *			this to a general purpose identifier scope resolution function
 *
 *	\fn	static struct dwarf_head_struct * deprecated_lookup_global_scope(struct gear_engine_context * ctx, const char * name, const struct scope_resolution_flags flags)
 *	\brief	performs identifier lookup using the global scope only
 *
 *	\todo	properly handle duplicate (non-unique) symbols in the symbol table
 *
 *	\param	ctx	context to work in
 *	\param	name	identifier name to look up
 *	\param	flags	various flags directing the course of identifier
 *			scope resolution
 *	\return	a pointer to the data object data node for the supplied
 *		identifier (if one is found), null otherwise */
static struct dwarf_head_struct * deprecated_lookup_global_scope(struct gear_engine_context * ctx, const char * name, const struct scope_resolution_flags flags)
{
struct sym_struct * sym;
	/* all of the global symbols must be in the
	 * engine symbol table, so just look it up */
	sym = symtab_find_sym(ctx, name);
	if (!sym)
		return 0;
	/*! \todo	properly determine (based on flag
	 *		values) what kind of symbol is
	 *		expected and properly handle
	 *		the case with multiple symbols
	 *		found */
	if (sym->next || (sym->symclass != SYM_DATA_OBJECT && sym->symclass != SYM_SUBROUTINE))
	{
		panic("");
	}
	else
		return sym->dobj;
	panic("");
}


/*
 *
 * exported functions follow
 *
 */ 

/*!
 *	\deprecated	this function is deprecated, it handles just data object names
 *			and not identifiers in general (e.g. type names) - transform
 *			this to a general purpose identifier scope resolution function
 *
 *	\fn	struct dwarf_head_struct * deprecated_scope_locate_dobj(struct gear_engine_context * ctx, const char * name, const struct scope_resolution_flags flags)
 *	\brief	locates a data object by performing proper identifier scope resolution
 *
 *	\param	ctx	context to work in
 *	\param	name	the identifier of the data object to be found
 *	\param	flags	various flags directing the course of identifier
 *			scope resolution
 *	\return	a pointer to the data object data node for the supplied
 *		identifier (if one is found), null otherwise */
struct dwarf_head_struct * deprecated_scope_locate_dobj(struct gear_engine_context * ctx, const char * name, const struct scope_resolution_flags flags)
{
struct subprogram_data * subp;
struct lexblock_data * lexblock;
struct dobj_data * dobj;
struct cu_data * cu;
void * lookup_node;
ARM_CORE_WORD pc;

	/* sanity checks */
	if (!name)
		panic("");
	/* see if only the global scope is requested to be looked up;
	 * this can be the case when the debugged program is not
	 * being run, so there is no other scope to look up other
	 * than the global one */
	if (flags.global_scope_only)
	{
		return deprecated_lookup_global_scope(ctx, name, flags);
	}
	/* no global scope only requested for searching - start
	 * drilling inside out from the current scope */
	/*! \todo	handle mode correctly */
	if (ctx->cc->core_reg_read(ctx,
				0,
				1 << ctx->tdesc->get_target_pc_reg_nr(ctx),
				&pc) != GEAR_ERR_NO_ERROR)
	{
		/* error accessing the target - proceed to searching the
		 * global scope only */
		return deprecated_lookup_global_scope(ctx, name,
				(struct scope_resolution_flags) { .global_scope_only = 1, });
	}

	if (!(lookup_node = aranges_get_lexblock_for_addr(ctx, pc)))
		lookup_node = aranges_get_subp_for_addr(ctx, pc);

	 /* it doesnt make much sense to look
	  * at file (compilation unit) scope any further - even if
	  * the program counter is in the bounds of a compilation unit,
	  * it has apparently hit some padding or data and and is probably
	  * invalid anyway - so just look up the global scope and return */
	if (!lookup_node)
		return deprecated_lookup_global_scope(ctx, name, flags);

	/* ok, start drilling inside out - lookup the supplied identifier
	 * in the following scope order (provided the program counter is
	 * within the bounds of a subroutine/lexical block with debugging
	 * information available):
	 *	- the current lexical block and subsequently any
	 *		enclosing lexical blocks (this includes
	 *		variables and types local to a function and
	 *		any nested lexical blocks)
	 *	- function parameters
	 *	- data objects with file scope and internal linkage for
	 *		the compilation unit containing the program counter
	 *		(static data objects local to the compilation unit)
	 *	- data objects with external linkage (the global scope)
	 */
	while (lookup_node)
	{
		switch (dwarf_util_get_tag_category(((struct dwarf_head_struct *)lookup_node)->tag))
		{
			case DWARF_TAG_CATEGORY_LEXBLOCK:
				/* lookup variables declared within this
				 * lexical block (if any) */
				lexblock = (struct lexblock_data *) lookup_node;
				dobj = lexblock->vars;
				while (dobj)
				{
					if (!dobj->name)
						gprintf("%s(): no name for dobj at 0x%08x\n", __func__, (int) dobj->head.die_offset);
					if (dobj->name && !strcmp(name, dobj->name))
					{
						return dobj;
					}
					dobj = dobj->sib_ptr;
				}
							
				break;

			case DWARF_TAG_CATEGORY_SUBPROGRAM:
				/* first, lookup variables declared
				 * within this subroutine (if any) */
				subp = (struct subprogram_data *) lookup_node;
				dobj = subp->vars;
				while (dobj)
				{
					if (!strcmp(name, dobj->name))
					{
						return dobj;
					}
					dobj = dobj->sib_ptr;
				}
				/* second, lookup the parameters
				 * of this subroutine (if any) */
				dobj = subp->params;
				while (dobj)
				{
					if (!dobj->name)
						gprintf("%s(): no name for dobj at 0x%08x\n", __func__, (int) dobj->head.die_offset);
					if (dobj->name && !strcmp(name, dobj->name))
					{
						return dobj;
					}
					dobj = dobj->sib_ptr;
				}
				break;

			case DWARF_TAG_CATEGORY_COMPILATION_UNIT:
				/* first, lookup file scope variables
				 * for the compilation unit (if any) */
				cu = (struct cu_data *) lookup_node;
				dobj = cu->vars;
				while (dobj)
				{
					if (!strcmp(name, dobj->name))
					{
						return dobj;
					}
					dobj = dobj->sib_ptr;
				}
				/* second, lookup subroutines
				 * in the compilation unit (if any) */
				subp = cu->subs;
				while (subp)
				{
					if (!strcmp(name, subp->name))
					{
						return subp;
					}
					subp = subp->sib_ptr;
				}
				/*! \todo	lookup types here */
				break;

			default:
				panic("");
		}
		/* symbol not found on the current iteration - move to
		 * the nodes parent and continue searching */
		lookup_node = ((struct dwarf_head_struct *)lookup_node)->parent;
	}
	/* symbol lookup failed; as a last resort - lookup the global
	 * scope */
	return deprecated_lookup_global_scope(ctx, name, flags);
}

