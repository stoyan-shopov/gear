/*!
 *	\file	dwarf-util.h
 *	\brief	dwarf-util.c header file
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * exported data types follow
 *
 */

/*! an enumeration to roughly categorize the different dwarf tags
 *
 * this one is used at several places to roughly determine what
 * entity a dwarf die represents and how this entity should be
 * handled - for example, data objects dies (which can be variables,
 * formal parameters) should be handled by utility code from
 * dobj-access.c, type dies (e.g. structures, unions, base types, etc.)
 * should be handled by code in type-access.c, and so on */
enum ENUM_DWARF_TAG_CATEGORY
{
	/*! an invalid category - used for catching errors */
	DWARF_TAG_CATEGORY_INVALID = 0,
	/*! data object category - this could be a variable, or a formal parameter
	 *
	 * a void pointer (to a debugging information tree node) containing
	 * a dwarf tag value (in its dwarf_head_struct) that falls
	 * in this category may be safely cast to type
	 * struct dobj_data (see file dobj-access.h for details) */
	DWARF_TAG_CATEGORY_DATA_OBJECT,
	/*! data type category
	 *
	 * a void pointer (to a debugging information tree node) containing
	 * a dwarf tag value (in its dwarf_head_struct) that falls
	 * in this category may be safely cast to type
	 * struct dtype_data (see file type-access.h for details) */
	DWARF_TAG_CATEGORY_DATA_TYPE,
	/*! subprogram category
	 *
	 * a void pointer (to a debugging information tree node) containing
	 * a dwarf tag value (in its dwarf_head_struct) that falls
	 * in this category may be safely cast to type
	 * struct subprogram_data (see file subprogram-access.h for details) */
	DWARF_TAG_CATEGORY_SUBPROGRAM,
	/*! lexical block category
	 *
	 * a void pointer (to a debugging information tree node) containing
	 * a dwarf tag value (in its dwarf_head_struct) that falls
	 * in this category may be safely cast to type
	 * struct lexblock_data (see file lexblock-access.h for details) */
	DWARF_TAG_CATEGORY_LEXBLOCK,
	/*! compilation unit category
	 *
	 * a void pointer (to a debugging information tree node) containing
	 * a dwarf tag value (in its dwarf_head_struct) that falls
	 * in this category may be safely cast to type
	 * struct cu_data (see file cu-access.h for details) */
	DWARF_TAG_CATEGORY_COMPILATION_UNIT,
};

/*
 *
 * exported function prototypes follow
 *
 */

const char * const dwarf_util_get_tag_name(Dwarf_Half tag);
enum ENUM_DWARF_TAG_CATEGORY dwarf_util_get_tag_category(Dwarf_Half tag);

