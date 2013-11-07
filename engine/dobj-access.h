/*!
 *	\file	dobj-access.h
 *	\brief	data objects access header
 *	\author	shopov
 *
 *
 *	Revision summary:
 *
 *	$Log: $
 */

#ifndef __DOBJ_ACCESS_H__
#define __DOBJ_ACCESS_H__

/*
 *
 * exported definitions follow
 *
 */

/*
 *
 * exported types follow
 *
 */

/*! the data object class type enumeration */
enum DOBJ_CLASS_ENUM
{
	/*! invalid data object class */
	DOBJ_CLASS_INVALID = 0,
	/*! variable class */
	DOBJ_CLASS_VAR,
	/*! formal parameter class */
	DOBJ_CLASS_FORMAL_PARAMETER,
};

/*! the data object information holding structure */
struct dobj_data
{
	/*! the head of this structure */
	struct dwarf_head_struct	head;
	/*! various flags; which of them are actually applicable, depends on the data object class */
	struct
	{
		/*! the class of the data object */
		enum DOBJ_CLASS_ENUM	dobj_class	: 2;

		/*!	denotes whether a data object has been artificially constructed by the compiler and is otherwise not present in the source code
		 *
		 * when nonzero, this flag denotes that this data object
		 * has been artificially constructed by the compiler
		 * and has no direct representation in the original
		 * source code; such data object nodes may be lacking
		 * some dwarf attributes (such as names) which are
		 * normally available for 'non-artificial' data objects */
		bool	is_artificial		: 1;
		/*! is the symbol externally visible
		 *
		 * \note	not applicable for formal parameters */
		bool	is_externally_visible	: 1;
		/*! denotes if the location record (below) is valid
		 *
		 * if this is nonzero if the location field below is valid;
		 * also read comments about the is_constant_value flag below;
		 * it is an error for a data object node to have both
		 * this flag and the is_constant_value flag below
		 * set - these flags are mutually exclusive */
		bool	is_location_valid	: 1;
		/*! denotes whether this data object describes a variable or formal parameter whose value is constant and not represented by an object in the target address space
		 *
		 * when nonzero, this flag denotes that this object
		 * describes a variable or formal parameter whose value
		 * is constant and is not represented by an object in
		 * the target program - this happens when the dwarf
		 * debug information generated for this data object
		 * possesses a DW_AT_const_value attribute, and it
		 * does *not* have a DW_AT_location attribute;
		 * it is therefore an error for a data object described
		 * by dwarf to have both a DW_AT_location attribute, and
		 * a DW_AT_const_value attribute - and it is therefore
		 * an error if both this flag is set and the 'is_location_valid'
		 * flag is also set - these flags are mutually exclusive;
		 * when this flag is set, then the 'const_val' field
		 * below is applicable and valid
		 *
		 * as the dwarf4 standard, section 4.1, paragraph 10 says:

10. A DW_AT_const_value attribute for an entry describing a variable or formal parameter whose value is constant and not represented by an object in the address space of the program, or an entry describing a named constant. (Note that such an entry does not have a location attribute.) The value of this attribute may be a string or any of the constant data or data block forms, as appropriate for the representation of the variable’s value. The value is the actual constant value of the variable, represented as it would be on the target architecture.
One way in which a formal parameter with a constant value and no location can arise is for a formal parameter of an inlined subprogram that corresponds to a constant actual parameter of a call that is inlined.

		 */
		bool	is_constant_value	: 1;
		/*! denotes if this data node describes a data object declaration
		 *
		 * if this field is nonzero, then this data node
		 * describes a non-defining/incomplete data object;
		 * as the dwarf standard, paragraph 2.13.1 and
		 * paragraph 2.13.2 say:
		 *
		 * 2.13.1 Non-Defining Declarations Debugging information entries that represent non-defining or otherwise incomplete declarations of a program entity have a DW_AT_declaration attribute, whose value is a flag.
		 * 2.13.2 Declarations Completing Non-Defining Declarations Debugging information entries that represent a declaration that completes another (earlier) nondefining declaration, may have a DW_AT_specification attribute whose value is a reference to the debugging information entry representing the non-defining declaration. Debugging information entries with a DW_AT_specification attribute do not need to duplicate information provided by the debugging information entry referenced by that specification attribute.
		 *
		 * notable missing information from data nodes
		 * describing (non-defining) data object
		 * declarations are location descriptions */
		bool	is_node_a_declaration	: 1;
	};
	/*! name of the data object */
	char *			name;
	/*! the type of the data object */
	struct dtype_data *	type;
	/*!
	 * \todo	maybe change the name of this field to "next"
	 *		for better readability
	 *
	 * a sibling pointer to another data object, available for generic use
	 * (semantics implied by context)
	 */
	struct dobj_data *	sib_ptr;
	/*! this union contains details about how to locate this data object
	 *
	 * basically, a data object is either:
	 *	- physically present in the target, and therefore
	 *	can be located at a certain place in the target
	 *	(e.g. in memory and/or registers), in which case
	 *	the 'is_location_valid' flag above is nonzero and
	 *	the 'location' field below applies,
	 *
	 * or
	 *
	 *	- the data object is not physically present in
	 *	the target, and if it has been reduced to a constant
	 *	which is known at compile time, the 'is_constant_value'
	 *	flag above is nonzero and the 'const_val' field below
	 *	applies
	 *
	 *	\note	the data fields below are mutually exclusive,
	 *		no more than one of them can be applicable and valid
	 *		at a time
	 */
	union
	{
		/*! location information
		 *
		 * this is valid only if the 'is_location_valid' flag above is nonzero */
		struct dwarf_location	location;
		/*! the constant value of a data object which has the 'is_constant_value' flag above set
		 *
		 * this is applicable and valid only if the 'is_constant_value'
		 * flag above is nonzro, read the comments above for
		 * this flag for more details
		 *
		 * \todo	maybe add some fields to discriminate
		 *		between signed/unsigned constants
		 *
		 * \todo	also support string constants
		 *		(DW_FORM_string and DW_FORM_strp)
		 * \todo	also support block constants
		 *		(DW_FORM_block1, DW_FORM_block2 ...) */
		Dwarf_Signed	const_val;
	};
	/*! cxx linkage name (if present)
	 *
	 * this is valid only when non-null, in that case this containing
	 * node should describe some cxx struct/class member variable;
	 * this linkage name is currently used as a quick
	 * (and maybe not very reliable) helper to facilitate reading
	 * cxx struct/class debug information, which may be dispersed
	 * through compilation units and has to be stitched together */
	const char * linkage_name;
};

/*
 *
 * exported function prototypes follow
 *
 */
struct dobj_data * dobj_process(struct gear_engine_context * ctx, Dwarf_Die die);

#endif /* __DOBJ_ACCESS_H__ */

