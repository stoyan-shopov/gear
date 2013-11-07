/*!
 *	\file	dwarf-common.h
 *	\brief	common dwarf access file inclusions
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * common inclusion section follows
 *
 */

#ifndef __DWARF_COMMON_H__
#define __DWARF_COMMON_H__

#include "../../../dwarf-20120410/libdwarf/dwarf.h"
#include "../../../dwarf-20120410/libdwarf/libdwarf.h"

#include <gelf.h>
#include <stdbool.h>

/*
 *
 * exported types follow
 *
 */

/*! common debug information tree data node
 *
 * this structure should be put in the very beginning
 * of each and any data structure describing
 * some entity in the debug information tree node (e.g.
 * a compilation unit node, a subprogram node, a
 * data type node, etc); it is needed mainly to make
 * debug information tree navigation possible
 *
 * basically, this data structure identifies the type
 * of each node in the debug information tree
 * built for a given file
 *
 * it is extremely important that this is the first member
 * of any and all data structures used to build the
 * debugging information tree; this is so that a pointer
 * to a node in the debug information tree can always
 * be safely dereferenced to a structure of type
 * ::dwarf_head_struct so that the type of the information
 * the pointer points to can be precisely determined;
 * this is used in debug information tree navigation */
struct dwarf_head_struct
{
	/*! a pointer to the parent of the data node container
	 *
	 * if null, then the container dwarf debug information
	 * node does not have a parent; this is used for debug
	 * information tree navigation 
	 *
	 * this is currently used for debug information tree
	 * navigation for the purposes of scope resolution
	 * of program identifiers (in e.g. expression evaluation),
	 * so some debug information tree nodes do not set this
	 * to a valid value - e.g. non-root type tree nodes;
	 * these ones are never used for debug information tree
	 * navigation for such purposes
	 *
	 * \todo	if it is ever appropriate for such
	 *	debug information tree nodes to contain valid
	 *	parent pointers - rework the appropriate code;
	 *	this may not be actually trivial (type information
	 *	is not always a 'tree' because of special cases
	 *	such as aggregates (e.g. struct-s) containing as members
	 *	pointers to themselves; see comments about struct/union
	 *	processing in type_access.c, function type_process()) */
	struct dwarf_head_struct *	parent;
	/*! the dwarf tag of a debug information node
	 *
	 * this is used for identifying the type of a data
	 * node in the debug information tree 
	 *
	 * this is the value of the dwarf tag (as specified
	 * in the dwarf3 standard) of the node that
	 * contains this structure; used to determine the
	 * type of the container data structure and to
	 * properly access its data fields 
	 *
	 * not all dwarf tags have a corresponding data
	 * structure and different tags may correspond
	 * to the same container type (e.g. DW_TAG_variable
	 * and DW_TAG_formal_parameter each have a
	 * corresponding 'struct dobj_data' declared
	 * in dobj_access.h 
	 *
	 * to denote an invalid container type, and
	 * to be used for catching errors, when the container
	 * is somehow invalid or inappropriate/uninitialized,
	 * this should be initialized to the invalid
	 * 0 value */
	Dwarf_Half	tag;
	/*!	\todo		remove this, debugging only */
	Dwarf_Off		die_offset;
};

/*
 *
 * exported function prototypes follow
 *
 */

void dwarf_util_set_parent(void * child, void * parent);


/*! \todo debugging/statistics only, remove this... */
extern struct type_access_stats
{
	int	nr_base_type_nodes;
	int	nr_typedef_nodes;
	int	nr_tqual_nodes;
	int	nr_arr_type_nodes;
	int	nr_ptr_nodes;
	int	nr_struct_nodes;
	int	nr_union_nodes;
	int	nr_member_nodes;
	int	nr_enumerator_nodes;
	int	nr_enumeration_nodes;

	int	nr_symtab_nodes;
	int	nr_dobj_nodes;

	int	nr_subprogram_nodes;
	int	nr_lexblocks;
	int	nr_subprogram_prototype_nodes;
}
type_access_stats;


#endif /* __DWARF_COMMON_H__ */

