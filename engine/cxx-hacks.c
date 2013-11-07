/*!	\todo	document this */

#include <stdarg.h>
/* for elf_hash()... */
#include <libelf.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "type-access.h"
#include "dwarf-loc.h"
#include "engine-err.h"
#include "subprogram-access.h"
#include "dobj-access.h"
#include "util.h"
#include "gprintf.h"
#include "miprintf.h"
#include "symtab.h"
#include "cxx-hacks.h"

/*! \todo document this */
static struct cxx_linkage_hash_node
{
	/*! pointer to the payload; exactly which is applicable depends on the struct dwarf_head_struct.tag */
	union
	{
		struct dwarf_head_struct	* head;
		struct dtype_data		* dtype;
		//struct subprogram_data		* sub;
		struct dobj_data		* dobj;
	};
	const char	* linkage_name;
	/*! link pointer for the hash table */
	struct type_hash_node	* next;
}
* cxx_linkage_hashtab
[
//#define HTAB_SIZE	39989
/*! \todo	debug only */
#define HTAB_SIZE	1
HTAB_SIZE
];

static void merge_var_def_to_var_decl(struct gear_engine_context * ctx, struct dtype_data * decl, struct dobj_data * def)
{
ARM_CORE_WORD res;
enum DWARF_EXPR_INFO_ENUM eval_info;

	/* sanity checks */
	if (decl->dtype_class != DTYPE_CLASS_MEMBER
			|| !decl->is_node_a_declaration
			|| def->is_node_a_declaration
			|| (!def->is_location_valid && !def->is_constant_value)
			|| def->dobj_class != DOBJ_CLASS_VAR
			|| !def->is_externally_visible
			)
	{
		panic("");
	}
	if (def->is_constant_value)
	{
		gprintf("%s(): handle constant values here...\n", __func__);
		return;
	}
	/* promote this declaration to a definition and store location
	 * information */
	decl->is_node_a_declaration = 0;

	if (dwarf_loc_eval_loc_from_list(ctx,
				&res,
				0,
				&eval_info,
				&def->location,
				0,
				0,
				0) != GEAR_ERR_NO_ERROR)
		panic("");

	if (eval_info != DW_EXPR_IS_CONSTANT)
		panic("");
	decl->member_data.member_location = res;
}

void cxx_hacks_add_linkage_node(struct gear_engine_context * ctx, struct dwarf_head_struct * h)
{
const char * linkage_name;
struct cxx_linkage_hash_node * hnode;
struct dobj_data * dobj, * new_dobj;
int i;

	new_dobj = 0;
	switch (h->tag)
	{
		case DW_TAG_member:
			/* h points to a dtype_data node */
			linkage_name = ((struct dtype_data *) h)->linkage_name;
			break;
		case DW_TAG_variable:
			/* h points to a dobj_data node */
			linkage_name = ((struct dobj_data *) h)->linkage_name;
			new_dobj = (struct dobj_data *) h;
			break;
		default:
			panic("");
	}
	if (!linkage_name)
		panic("");
	/* data objects are expected to be unique, if at all present... */
	dobj = 0;
	for (hnode = cxx_linkage_hashtab[i = elf_hash(linkage_name) % HTAB_SIZE]; hnode; hnode = hnode->next)
	{
		/* if the node already is in the hash table, do nothing */
		if (hnode->head == h)
			return;
		if (!strcmp(linkage_name, hnode->linkage_name) && hnode->head->tag == DW_TAG_variable)
		{
			if (dobj)
				/*!	\todo	multiple definitions... is this at all possible... */
				panic("");
			dobj = hnode->dobj;
		}
	}
	/* data objects are expected to be unique, if at all present... */
	if (dobj && new_dobj)
	{
		/* possible, if the compiler has optimized the
		 * data objects to compile-time known constants
		 * which consume no memory (i.e. have attributes
		 * DW_AT_const_value) */
		/* see if the data objects are actually (equal) constants
		 * which do not live in memory */
		if (dobj->is_constant_value
				&& new_dobj->is_constant_value
				&& dobj->const_val == new_dobj->const_val)
			/* the data objects are the same, no need to do
			 * anything else */
			return;
		/* otherwise, see if both data objects have the same
		 * locations */
		else if (dobj->is_location_valid
				&& new_dobj->is_location_valid
				&& dwarf_loc_are_locations_the_same(ctx, &dobj->location, &new_dobj->location)
				)
			/* the data objects have the same locations,
			 * no need to do anything else */
			return;
		gprintf("multiple symbols with the same linkage name: %s\n", linkage_name);
		gprintf("old dobj die at 0x%08x\n", (int) dobj->head.die_offset);
		gprintf("new dobj die at 0x%08x\n", (int) new_dobj->head.die_offset);
		panic("");
	}
	if (!(hnode = calloc(1, sizeof * hnode)))
		panic("");
	hnode->next = cxx_linkage_hashtab[i];
	cxx_linkage_hashtab[i] = hnode;
	hnode->head = h;
	hnode->linkage_name = linkage_name;

	if (new_dobj)
	{
		for (hnode = cxx_linkage_hashtab[i]; hnode; hnode = hnode->next)
			if (!strcmp(linkage_name, hnode->linkage_name) && hnode->head->tag == DW_TAG_member)
				merge_var_def_to_var_decl(ctx, hnode->dtype, new_dobj);
	}
	else if (dobj)
		merge_var_def_to_var_decl(ctx, h, dobj);
}

struct dtype_data * cxx_hacks_find_class_def(struct gear_engine_context * ctx, const char * class_name)
{
}

