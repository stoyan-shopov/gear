/*!
 *	\file	dobj-access.c
 *	\brief	data objects entries processing and access code
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
#include "util.h"
#include "type-access.h"
#include "core-access.h" 
#include "dwarf-expr.h" 
#include "dwarf-loc.h"
#include "dobj-access.h"

#include "cxx-hacks.h"

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static struct dobj_data * get_dobj_node(void)
 *	\brief	allocates memory for a dobj_data structure and initializes it
 *
 *	\return	a pointer to the newly allocated dobj_data data structure
 */

static struct dobj_data * get_dobj_node(void)
{
#if 0
struct dobj_data * p;

	if (!(p = (struct dobj_data * ) calloc(1, sizeof * p)))
		panic("out of core");
	return p;
#else
/*! \todo	fix these */
static struct dobj_data * dobj_cache = 0;
static int dcache_size = 0;
	
	if (!dcache_size)
	{
		if (!(dobj_cache = calloc(dcache_size = 256, sizeof * dobj_cache)))
			panic("");
	}
	dcache_size --;
	return dobj_cache ++;

#endif	
}
/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	struct dobj_data * dobj_process(struct gear_engine_context * ctx, Dwarf_Die die);
 *	\brief	given a dwarf die, builds the information tree for the data object described by the die
 *
 *	\param	ctx	context used for access to the debugging information
 *			of the debuggee
 *	\param	die	the die of interest
 *	\return	pointer to the root of the debug information tree built for the
 *		data object of the die
 */
struct dobj_data * dobj_process(struct gear_engine_context * ctx, Dwarf_Die die)
{
struct dobj_data * p;
int res;
Dwarf_Half tagval;
Dwarf_Error err;
Dwarf_Die type_die;
Dwarf_Attribute attr;
Dwarf_Bool flag;

	/*! \todo	debug only, remove when done */
Dwarf_Off die_offset;
	if (dwarf_dieoffset(die, &die_offset, &err) != DW_DLV_OK)
		panic("dwarf_dieoffset()");

	/* get the tag of the die */
	if (dwarf_tag(die, &tagval, &err) != DW_DLV_OK)
		panic("dwarf_tag()");
	/* validate the tag of the die - make sure
	 * it is indeed a data object die */
	switch (tagval)
	{
		case DW_TAG_formal_parameter:
		case DW_TAG_member:
		case DW_TAG_variable:
			/* tag ok */
			break;
		default:
			gprintf("unsupported dwarf tag: %s\n", dwarf_util_get_tag_name(tagval));
			panic("");
	}

	/* check for unsupported attributes */
#if 0	
	if (dwarf_hasattr(die, DW_AT_specification, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
		panic("unsupported data object attribute found");
	}
#endif
	if (dwarf_hasattr(die, DW_AT_variable_parameter, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");
	if (dwarf_hasattr(die, DW_AT_is_optional, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");
	if (dwarf_hasattr(die, DW_AT_start_scope, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");
	if (dwarf_hasattr(die, DW_AT_endianity, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("unsupported data object attribute found");

	/* get core for the new node */
	p = get_dobj_node();
	/*! \todo	debug only, remove when done */
	p->head.die_offset = die_offset;

	p->head.tag = tagval;
#if 0

	if (dwarf_hasattr(die, DW_AT_abstract_origin, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
{ Dwarf_Off x; if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic(""); gprintf("cu relative offset: %i\n", (int) x); }
		gprintf("downgraded from panic(), skipping panic now: unsupported attribute found");
		/*! \todo	hack hack hack */
		p = get_dobj_node();
		p->head.die_offset = die_offset;
		return p;
	}
#endif

	if (dwarf_hasattr(die, DW_AT_MIPS_linkage_name, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_MIPS_linkage_name, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the attribute's value */
		if (dwarf_formstring(attr, &p->linkage_name, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}

	if (dwarf_hasattr(die, DW_AT_artificial, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		/* \todo	gcc 4.5.0 generated some very obscure artificial
		 *		variables in some of the tests performed...
		 *		i(sgs) really have no idea what to do with these
		 *		(and if they are of any use at all);
		 *		update - other than c++ this pointers
		 *		gcc 4.5.1 is also capable of generating
		 *		also anonymous artificial variables for
		 *		the upper array bounds of variable
		 *		length arrays... handle these as well
		 *		(should really be handled in type-access.c) */
		if (dwarf_attr(die, DW_AT_artificial, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the flag's value */
		if (dwarf_formflag(attr, &flag, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (flag)
			p->is_artificial = 1;
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

	/* see if the data object completes another,
	 * non-defining declaration */
	if (dwarf_hasattr(die, DW_AT_specification, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Off offset;
		Dwarf_Die decl_die;
		struct dobj_data * decl_node;
		Dwarf_Half decl_tag;

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
		if (!(decl_node = dobj_process(ctx, decl_die)))
			panic("");
		/* save the declaration tag for later use below - see
		 * the call to 'dwarf_util_set_parent()' below */
		if (dwarf_tag(decl_die, &decl_tag, &err) != DW_DLV_OK)
			panic("dwarf_tag()");

		dwarf_dealloc(ctx->dbg, decl_die, DW_DLA_DIE);
		/* see if the referenced die is well-formed */
		if (!decl_node->is_node_a_declaration
				/* possible for static cxx struct-type(class) members
				   || decl_node->head.tag != tagval */)
		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			panic("");
		}
		/* ok, copy the fields of the associated
		 * non-defining declaration into the current node */
		*p = *decl_node;
		/* only these fields should need fixing */
		p->is_node_a_declaration = false;
		p->head.tag = tagval;
		p->head.die_offset = die_offset;
		if (die_offset == 0)
			panic("");
		/* if the specification die for this die is
		 * a DW_TAG_member (meaning it is some struct/union/class
		 * member), set the parent of this die to be the
		 * specification die so that this data object
		 * does not accidentally end up in the global
		 * symbol table and therefore in the global
		 * namespace with its unqualified name
		 * (when processing compilation units in
		 * cu-access.c, function cu_process(), 
		 * data objects with null parents are
		 * being added to the global symbol table;
		 * example (cxx), if you have:
		 struct x
		 {
		 	static int var;
		 } x;
		 * without setting the parent here, the
		 * symbol 'var' will be added in the global
		 * symbol table by cu_process(), and this
		 * is incorrect */
		if (decl_tag == DW_TAG_member)
			dwarf_util_set_parent(p, decl_node);
	}

	/*! \todo	should this really be here... maybe move it somehwere above... */
	/* handle nodes of concrete inline instance trees corresponding
	 * to inline-expanded subprograms */
	if (dwarf_hasattr(die, DW_AT_abstract_origin, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Off offset;
		Dwarf_Die abstract_die;
		struct dobj_data * abstract_node;

		/* retrieve the associated abstract origin
		 * die offset */
		if (dwarf_attr(die, DW_AT_abstract_origin, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
			panic("dwarf_global_formref()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (dwarf_offdie(ctx->dbg, offset, &abstract_die, &err) != DW_DLV_OK)
			panic("dwarf_offdie()");
		if (!(abstract_node = dobj_process(ctx, abstract_die)))
			panic("");
		dwarf_dealloc(ctx->dbg, abstract_die, DW_DLA_DIE);
		/* copy the fields of interest */
		/*! \todo	this is incorrect/incomplete - other fields;
		 *		also, see how this should work along with
		 *		the DW_AT_specification case above, and
		 *		look in the dwarf4 standard if this is at
		 *		all possible */
		*p = *abstract_node;
		p->head.die_offset = die_offset;
		p->sib_ptr = 0;

		if (0) free(abstract_node);
	}


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
			p->is_node_a_declaration = 1;
	}

	/* fill the data object class */
	switch (tagval)
	{
		/* probably a static cxx struct-type(class) member */
		case DW_TAG_member:
		case DW_TAG_variable:
			p->dobj_class = DOBJ_CLASS_VAR;
			break;
		case DW_TAG_formal_parameter:
			p->dobj_class = DOBJ_CLASS_FORMAL_PARAMETER;
			break;
		default:
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			panic("invalid data object die tag");
			break;
	}
	/* retrieve the data object name */
	if (dwarf_hasattr(die, DW_AT_name, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (!flag)
	{
		/* a name attribute no present - however, it could have
		 * already been copied to this node in  case this is
		 * a data node that is completing another (non-defining)
		 * declaration, in which case the name field of this node
		 * will have a non-null value; otherwise, there is some
		 * error
		 *
		 * and it is also possible for the name to be absent
		 * altogether if a formal parameter node in a 
		 * function prototype (in, e.g., a subroutine type
		 * (that is, in a DW_TAG_subroutine_type)) is being
		 * processed
		 *
		 * \todo	it cannot really be verified at this
		 *		point if indeed a formal parameter
		 *		is being handled in the case outlined above */
		if (!p->name
				/* artificial data objects are allowed
				 * to remain unnamed */
				&& !(p->is_artificial
				/* the same for formal parameters in prototypes... */	
				|| p->dobj_class == DOBJ_CLASS_FORMAL_PARAMETER))

		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			/*! \todo	gcc 4.5.1 generates DW_TAG_variable
			 *		with nothing else for some automatic
			 *		variables in concrete inline instance roots...
			 *		investigate and fix this... */
			return p;
			panic("data object doesnt have a name");
		}
	}
	else
	{
		if (dwarf_diename(die, &p->name, &err) != DW_DLV_OK)
			panic("dwarf_diename()");

	}
	/* check for external visibility */
	if (dwarf_hasattr(die, DW_AT_external, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_external, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the flag's value */
		if (dwarf_formflag(attr, &flag, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (flag)
			p->is_externally_visible = 1;
	}

	/* retrieve the data object type */
	if (!dtype_access_get_type_die(ctx, die, &type_die))
	{
		/* a type attribute not present - however, it could have
		 * already been copied to this node in case this is
		 * a data node that is completing another (non-defining)
		 * declaration, or if this is a concrete inline instance
		 * tree node for which type information has been copied
		 * from its corresponding abstract inline instance tree node;
		 * in either case the dobj_type field of this node
		 * will have a non-null value; otherwise, there is some
		 * error */
		if (!p->type)
		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			gprintf("panic downgrade: error retrieving data object type");
			return p;
		}
	}
	else
	{
		p->type = type_process(ctx, type_die);
		dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);
	}

	/* ok, now store the location of the data object */
	if (dwarf_hasattr(die, DW_AT_location, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_location, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");

		if ((res = dwarf_loclist_n(attr, &p->location.llbuf, &p->location.listlen, &err)) != DW_DLV_OK
				/* it is possible that this list is simply an empty one */
				&& res != DW_DLV_NO_ENTRY
				)
		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			if (dwarf_errno(err) == DW_DLE_LOC_EXPR_BAD)
				printf("!!! location expression bad !!!\n");
			printf("PANIC() DONGRADE: dwarf_loclist_n()\n");
			printf("GCC 4.6.2 is known to generate some funny DW_OP_GNU_implicit_pointer opcodes - handle these\n");
		}
		else if (res != DW_DLV_NO_ENTRY)
			p->is_location_valid = true;
		if (p->name && !strcmp(p->name, "argc"))
		{
			int i;
			gprintf("argc detected\n");
			gprintf("location list size: %i\n", (int) p->location.listlen);
			for (i = 0; i < p->location.listlen; i++)
			{
				gprintf("low pc: 0x%08x, hi pc: 0x%08x\n",
						(int) p->location.llbuf[i]->ld_lopc,
						(int) p->location.llbuf[i]->ld_hipc);
			}
		}	
	}
	/* see if this is a constant object (that has no location);
	 * also read the comments about the 'is_constant_value' flag
	 * in file 'dobj-access.h' */
	if (dwarf_hasattr(die, DW_AT_const_value, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Half form;
		/* see what form this DW_AT_const_value attribute has
		 * (can be one of constant, block, string */
		if (dwarf_attr(die, DW_AT_const_value, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_whatform(attr, &form, &err) != DW_DLV_OK)
			panic("");
		switch (form)
		{
			/*! \todo	handle sign properly here */
			case DW_FORM_data1:
			case DW_FORM_data2:
			case DW_FORM_data4:
			case DW_FORM_data8:
			case DW_FORM_udata:
				if (dwarf_formudata(attr, &p->const_val, &err)
						!= DW_DLV_OK)
				{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
					panic("");
				}
				/* make sure the sizes of the data fetched
				 * from the DW_AT_const_value attribute and as
				 * recorded in the type information for
				 * this data object match */
				if (p->type == 0 
						|| sizeof p->const_val < dtype_access_sizeof(p->type))
					panic("");
				break;
			case DW_FORM_sdata:
				if (dwarf_formsdata(attr, &p->const_val, &err)
						!= DW_DLV_OK)
				{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
					panic("");
				}
				/* make sure the sizes of the data fetched
				 * from the DW_AT_const_value attribute and as
				 * recorded in the type information for
				 * this data object match */
				if (p->type == 0 
						|| sizeof p->const_val < dtype_access_sizeof(p->type))
					panic("");
				break;
			case DW_FORM_string:	
			case DW_FORM_strp:	
				gprintf("handle string constants\n");
				break;
			case DW_FORM_block1:
			case DW_FORM_block2:
				gprintf("handle block constants\n");
				break;
			default:
				gprintf("unsupported dwarf constant form: %i\n", (int) form);
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
				panic("");
		}
		p->is_constant_value = 1;

	}

	if (p->is_location_valid && p->is_constant_value)
	{
		/* strictly speaking (see dwarf 4 standard, section 4.1,
		 * paragraph 10), this is an incorrect situation,
		 * gcc 4.5.1 has been seen to actually generate
		 * a declaration die (possessing a DW_AT_declaration
		 * attribute), and then another die, which references
		 * the previous one via a DW_AT_specification attribute
		 * and has a DW_AT_location attribute... work around this
		 * here by wiping out the is_location_valid flag, as
		 * the constant value is most recent at this point in
		 * the code */
		/*! \todo	is anything else more appropriate here??? */
		p->is_location_valid = 0;
#if 0
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
		panic("");
#endif		
	}

type_access_stats.nr_dobj_nodes ++;

	if (p->linkage_name && !p->is_node_a_declaration && p->head.tag == DW_TAG_variable)
		cxx_hacks_add_linkage_node(ctx, p);
	return p;
}

