/*!
 *	\file	dwarf-util.c
 *	\brief	dwarf debug information processing utility routines
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
#include "util.h"

#include "dwarf-common.h"
#include "dwarf-util.h"

/*! a table holding a dwarf tag - tag symbol name (a string) correspondence
 *
 * can be used for retrieving the symbollic name (a string) corresponding
 * to a dwarf tag value, and vice versa; the table is indexed by tag value */
const static char * const dwarf_tag_to_string_name_table[] =
{
#define STRINGIFY(str)	#str
	[DW_TAG_array_type]			= STRINGIFY(DW_TAG_array_type),
	[DW_TAG_class_type]			= STRINGIFY(DW_TAG_class_type),
	[DW_TAG_entry_point]			= STRINGIFY(DW_TAG_entry_point),
	[DW_TAG_enumeration_type]		= STRINGIFY(DW_TAG_enumeration_type),
	[DW_TAG_formal_parameter]		= STRINGIFY(DW_TAG_formal_parameter),
	[DW_TAG_imported_declaration]		= STRINGIFY(DW_TAG_imported_declaration),
	[DW_TAG_label]				= STRINGIFY(DW_TAG_label),
	[DW_TAG_lexical_block]			= STRINGIFY(DW_TAG_lexical_block),
	[DW_TAG_member]				= STRINGIFY(DW_TAG_member),
	[DW_TAG_pointer_type]			= STRINGIFY(DW_TAG_pointer_type),
	[DW_TAG_reference_type]			= STRINGIFY(DW_TAG_reference_type),
	[DW_TAG_compile_unit]			= STRINGIFY(DW_TAG_compile_unit),
	[DW_TAG_string_type]			= STRINGIFY(DW_TAG_string_type),
	[DW_TAG_structure_type]			= STRINGIFY(DW_TAG_structure_type),
	[DW_TAG_subroutine_type]		= STRINGIFY(DW_TAG_subroutine_type),
	[DW_TAG_typedef]			= STRINGIFY(DW_TAG_typedef),
	[DW_TAG_union_type]			= STRINGIFY(DW_TAG_union_type),
	[DW_TAG_unspecified_parameters]		= STRINGIFY(DW_TAG_unspecified_parameters),
	[DW_TAG_variant]			= STRINGIFY(DW_TAG_variant),
	[DW_TAG_common_block]			= STRINGIFY(DW_TAG_common_block),
	[DW_TAG_common_inclusion]		= STRINGIFY(DW_TAG_common_inclusion),
	[DW_TAG_inheritance]			= STRINGIFY(DW_TAG_inheritance),
	[DW_TAG_inlined_subroutine]		= STRINGIFY(DW_TAG_inlined_subroutine),
	[DW_TAG_module]				= STRINGIFY(DW_TAG_module),
	[DW_TAG_ptr_to_member_type]		= STRINGIFY(DW_TAG_ptr_to_member_type),
	[DW_TAG_set_type]			= STRINGIFY(DW_TAG_set_type),
	[DW_TAG_subrange_type]			= STRINGIFY(DW_TAG_subrange_type),
	[DW_TAG_with_stmt]			= STRINGIFY(DW_TAG_with_stmt),
	[DW_TAG_access_declaration]		= STRINGIFY(DW_TAG_access_declaration),
	[DW_TAG_base_type]			= STRINGIFY(DW_TAG_base_type),
	[DW_TAG_catch_block]			= STRINGIFY(DW_TAG_catch_block),
	[DW_TAG_const_type]			= STRINGIFY(DW_TAG_const_type),
	[DW_TAG_constant]			= STRINGIFY(DW_TAG_constant),
	[DW_TAG_enumerator]			= STRINGIFY(DW_TAG_enumerator),
	[DW_TAG_file_type]			= STRINGIFY(DW_TAG_file_type),
	[DW_TAG_friend]				= STRINGIFY(DW_TAG_friend),
	[DW_TAG_namelist]			= STRINGIFY(DW_TAG_namelist),
	[DW_TAG_namelist_item]			= STRINGIFY(DW_TAG_namelist_item),
	[DW_TAG_namelist_items]			= STRINGIFY(DW_TAG_namelist_items),
	[DW_TAG_packed_type]			= STRINGIFY(DW_TAG_packed_type),
	[DW_TAG_subprogram]			= STRINGIFY(DW_TAG_subprogram),
	[DW_TAG_template_type_parameter]	= STRINGIFY(DW_TAG_template_type_parameter),
	[DW_TAG_template_type_param]		= STRINGIFY(DW_TAG_template_type_param),
	[DW_TAG_template_value_parameter]	= STRINGIFY(DW_TAG_template_value_parameter),
	[DW_TAG_template_value_param]		= STRINGIFY(DW_TAG_template_value_param),
	[DW_TAG_thrown_type]			= STRINGIFY(DW_TAG_thrown_type),
	[DW_TAG_try_block]			= STRINGIFY(DW_TAG_try_block),
	[DW_TAG_variant_part]			= STRINGIFY(DW_TAG_variant_part),
	[DW_TAG_variable]			= STRINGIFY(DW_TAG_variable),
	[DW_TAG_volatile_type]			= STRINGIFY(DW_TAG_volatile_type),
	[DW_TAG_dwarf_procedure]		= STRINGIFY(DW_TAG_dwarf_procedure),
	[DW_TAG_restrict_type]			= STRINGIFY(DW_TAG_restrict_type),
	[DW_TAG_interface_type]			= STRINGIFY(DW_TAG_interface_type),
	[DW_TAG_namespace]			= STRINGIFY(DW_TAG_namespace),
	[DW_TAG_imported_module]		= STRINGIFY(DW_TAG_imported_module),
	[DW_TAG_unspecified_type]		= STRINGIFY(DW_TAG_unspecified_type),
	[DW_TAG_partial_unit]			= STRINGIFY(DW_TAG_partial_unit),
	[DW_TAG_imported_unit]			= STRINGIFY(DW_TAG_imported_unit),
	[DW_TAG_mutable_type]			= STRINGIFY(DW_TAG_mutable_type),
	[DW_TAG_condition]			= STRINGIFY(DW_TAG_condition),
	[DW_TAG_shared_type]			= STRINGIFY(DW_TAG_shared_type),
#undef STRINGIFY
};


/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	const char * const dwarf_util_get_tag_name(Dwarf_Half tag)
 *	\brief	given a dwarf tag value, returns it symbollic name (as a string)
 *
 *	\param	tag	dwarf tag value
 *	\return	symbollic name (a string) of parameter 'tag'
 */

const char * const dwarf_util_get_tag_name(Dwarf_Half tag)
{
	if (tag >= sizeof dwarf_tag_to_string_name_table / sizeof * dwarf_tag_to_string_name_table)
		return 0;
	return dwarf_tag_to_string_name_table[tag];
}

/*!
 *	\fn	void dwarf_util_set_parent(void * child, void * parent)
 *	\brief	used to initialize relation information between debug information tree nodes
 *
 *	this is used to initialize the parent pointer field in a
 *	dwarf_head_struct header structure contained in some
 *	dwarf debug information tree node; if child is null, does
 *	nothing
 *
 *	\note	this may be moved to a macro
 *
 *	\param	child	a pointer to the node that is an immediate
 *		successor of node 'parent' in the debug information tree;
 *		must be castable to struct dwarf_head_struct; if null
 *		the function has no effect
 *	\param	parent	a pointer to the immediate predecessor of node
 *		'child' in the debug information tree;
 *		must be castable to struct dwarf_head_struct;
 *		can be null
 *	\return	none
 *
 */
void dwarf_util_set_parent(void * child, void * parent)
{
	if (!child)
		return;
	((struct dwarf_head_struct *)child)->parent = parent;
}



/*!
 *	\fn	enum ENUM_DWARF_TAG_CATEGORY dwarf_util_get_tag_category(Dwarf_Half tag)
 *	\brief	for a given dwarf tag, returns the category to which the tag belongs
 *
 *	see enum ENUM_DWARF_TAG_CATEGORY for category details; this routine
 *	is mainly useful at a couple of places where generic processing
 *	needs to determine the type of a dwarf node in a debugging
 *	information tree generally pointed to by an untyped (void *)
 *	pointer, and subsequently to properly cast the pointer to
 *	an appropriate data type and do some processing on it; e.g.,
 *	if we have a "void *" pointer to a node in the debugging information
 *	tree which is categorized as a DWARF_TAG_CATEGORY_SUBPROGRAM,
 *	then it is safe to cast the pointer to a "struct subprogram_data *"
 *	(the type is declared in file subprogram-access.h) and subsequently
 *	perform various needed processings on data pointed to by
 *	this pointer
 *
 *	\param	tag	the dwarf tag value of the node in the
 *			debugging information tree, as specified
 *			by the dwarf standard (definitions reside
 *			in the dwarflib supplied dwarf.h file)
 *	\return	the category of the supplied tag, as specified by
 *		enum ENUM_DWARF_TAG_CATEGORY
 */
enum ENUM_DWARF_TAG_CATEGORY dwarf_util_get_tag_category(Dwarf_Half tag)
{
	switch (tag)
	{
		case DW_TAG_array_type:
		case DW_TAG_enumeration_type:
		case DW_TAG_pointer_type:
		case DW_TAG_structure_type:
		case DW_TAG_union_type:
		case DW_TAG_base_type:
		case DW_TAG_const_type:
		case DW_TAG_volatile_type:
			return DWARF_TAG_CATEGORY_DATA_TYPE;

		case DW_TAG_class_type:
		case DW_TAG_entry_point:
		case DW_TAG_imported_declaration:
		case DW_TAG_label:
		case DW_TAG_member:
		case DW_TAG_reference_type:
		case DW_TAG_string_type:
		case DW_TAG_subroutine_type:
		case DW_TAG_unspecified_parameters:
		case DW_TAG_variant:
		case DW_TAG_common_block:
		case DW_TAG_common_inclusion:
		case DW_TAG_inheritance:
		case DW_TAG_inlined_subroutine:
		case DW_TAG_module:
		case DW_TAG_ptr_to_member_type:
		case DW_TAG_set_type:
		case DW_TAG_subrange_type:
		case DW_TAG_with_stmt:
		case DW_TAG_access_declaration:
		case DW_TAG_catch_block:
		case DW_TAG_constant:
		case DW_TAG_enumerator:
		case DW_TAG_file_type:
		case DW_TAG_friend:
		case DW_TAG_namelist:
		case DW_TAG_namelist_item:
		case DW_TAG_packed_type:
		case DW_TAG_template_type_parameter:
		case DW_TAG_template_value_parameter:
		case DW_TAG_thrown_type:
		case DW_TAG_try_block:
		case DW_TAG_variant_part:
		case DW_TAG_dwarf_procedure:
		case DW_TAG_restrict_type:
		case DW_TAG_interface_type:
		case DW_TAG_namespace:
		case DW_TAG_imported_module:
		case DW_TAG_unspecified_type:
		case DW_TAG_partial_unit:
		case DW_TAG_imported_unit:
		case DW_TAG_mutable_type:
		case DW_TAG_condition:
		case DW_TAG_shared_type:
		case DW_TAG_lo_user:
			gprintf("unhandled die tag %s\n", dwarf_util_get_tag_name(tag));
			panic("");
			break;

		case DW_TAG_compile_unit:
			return DWARF_TAG_CATEGORY_COMPILATION_UNIT;

		case DW_TAG_lexical_block:
			return DWARF_TAG_CATEGORY_LEXBLOCK;

		case DW_TAG_formal_parameter:
		case DW_TAG_variable:
			return DWARF_TAG_CATEGORY_DATA_OBJECT;

		case DW_TAG_subprogram:
			return DWARF_TAG_CATEGORY_SUBPROGRAM;

		case DW_TAG_typedef:
			panic("");
			break;
		default:
			panic("");
			break;
	}
}


/*! \todo	move this to where it belongs */
struct type_access_stats type_access_stats;

