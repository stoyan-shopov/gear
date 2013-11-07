/*!
 *	\file	subprogram-access.h
 *	\brief	subprogram access header
 *	\author	shopov
 *
 *	\todo	support non-contiguous subprogram address ranges
 *	\todo	support subprogram block-scoped data types
 *
 *	Revision summary:
 *
 *	$Log: $
 */

#ifndef __SUBPROGRAM_ACCESS_H__
#define __SUBPROGRAM_ACCESS_H__

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

/*! compilation unit information holding data structure
 *
 * \note	the layout of this data structure is
 *		a bit bizarre, it is being done the way
 *		it is in order to save space... */
struct subprogram_data
{
	/*! the head of this structure */
	struct dwarf_head_struct	head;
	/*! various flags are held here */
	struct
	{
		/*! denotes that the subprogram is externally visible */
		bool		is_externally_visible : 1;
		/*! denotes that the subprogram is an abstract inline instance root
		 *
		 * if this is nonzero, then the dwarf_inline_code field below
		 * shall be valid and will contain the subprogram's die
		 * DW_AT_inline attribute value
		 *
		 * \note	the fields is_abstract_inline_instance_root
		 *		(this field) and is_concrete_inline_instance_root 
		 *		are mutually exclusive (both can't be true at the
		 *		same time) */
		bool		is_abstract_inline_instance_root : 1;
		/*! denotes that the subprogram is a concrete inline instance root
		 *
		 * \note	the fields is_concrete_inline_instance_root
		 *		(this field) and is_abstract_inline_instance_root 
		 *		are mutually exclusive (both can't be true at the
		 *		same time)
		 *
		 * if this is nonzero, the call_file and call_line fields below
		 * will contain valid information */
		bool		is_concrete_inline_instance_root : 1;
		/*! denotes that frame base data for this subprogram is available
		 *
		 * this is nonzero if, and only if, the fb_location field below
		 * is valid and contains the frame base location for this subprogram;
		 * this can be zero e.g. for inline subprograms which are not expanded inline
		 * (i.e. abstract inline subprogram instance roots), and declarations */
		bool		is_frame_base_available : 1;
		/*! denotes that the entry_pc field (below) for this node is applicable and valid
		 *
		 * this is nonzero if, and only if, the entry_pc field below
		 * is valid and contains the entry point address of this subprogram;
		 * this can be zero e.g. for inline subprograms which are not expanded inline
		 * (i.e. abstract inline subprogram instance roots), and declarations */
		bool		is_entry_point_valid	: 1;
		/*! denotes if this data node describes a subprogram declaration
		 *
		 * if this field is nonzero, then this subprogram node
		 * describes a non-defining/incomplete subprogram die;
		 * as the dwarf standard, paragraphs 2.13.1 and
		 * 2.13.2 say:
		 *
		 * 2.13.1 Non-Defining Declarations Debugging information entries that represent non-defining or otherwise incomplete declarations of a program entity have a DW_AT_declaration attribute, whose value is a flag.
		 *
		 * 2.13.2 Declarations Completing Non-Defining Declarations Debugging information entries that represent a declaration that completes another (earlier) nondefining declaration, may have a DW_AT_specification attribute whose value is a reference to the debugging information entry representing the non-defining declaration. Debugging information entries with a DW_AT_specification attribute do not need to duplicate information provided by the debugging information entry referenced by that specification attribute.
		 *
		 * notable missing information from subrogram nodes
		 * describing (non-defining) subrogram
		 * declarations are the memory regions
		 * that the subprogram spans */
		bool	is_node_a_declaration	: 1;

		/*! dwarf DW_AT_inline attribute value, if a subprogram entry has such an attribute
		 *
		 * this is valid only if, the is_abstract_inline_instance_root
		 * flag above is non zero (indicating that a DW_AT_inline
		 * attribute is present for a subprogram debug information
		 * entry) - this can be one of:
		 * 	- DW_INL_not_inlined - subprogram neither declared
		 * 	as inline, nor inlined by the compiler
		 *	- DW_INL_inlined - subprogram not declared as
		 *	inline, but nonetheless inlined by the compiler
		 *	- DW_INL_declared_not_inlined - subprogram
		 *	declared as inline, yet not inlined by the
		 *	compiler
		 *	- DW_INL_declared_inlined - subprogram declared
		 *	as inline, and indeed inlined by the compiler */
		unsigned int	dwarf_inline_code : 2;

	};
	/*! source code coordinates information
	 *
	 * below are two (anonymous) mutually exclusive data structures:
	 *	- the first one contains the source code coordinates
	 *	of where in the source code the call to a subprogram
	 *	that is expanded inline (this subprogram) occurred;
	 *	this data structure is applicable only when the
	 *	'is_concrete_inline_instance_root' flag above
	 *	is nonzero - otherwise:
	 *	- the second one contains the source code coordinates
	 *	where this subprogram actually got defined -
	 *	this data structure is applicable only when the
	 *	'is_concrete_inline_instance_root' flag above is zero */
	union
	{
		/*! source code file coordinates of where the call to an inlined subprogram occurred
		 * 
		 * applicable only when the is_concrete_inline_instance_root flag
		 * above is nonzero */
		struct
		{
			/*! number of the source code file where the call to a concrete inline expansion of a subroutine occurred
			 *
			 * applicable only when the is_concrete_inline_instance_root flag
			 * above is nonzero; as the dwarf(now version 4) standard says:

			 An inlined subroutine entry may also have DW_AT_call_file, DW_AT_call_line and DW_AT_call_column attributes, each of whose value is an integer constant. These attributes represent the source file, source line number, and source column number, respectively, of the first character of the statement or expression that caused the inline expansion. The call file, call line, and call column attributes are interpreted in the same way as the declaration file, declaration line, and declaration column attributes, respectively (see Section 2.14).

and:

The value of the DW_AT_decl_file attribute corresponds to a file number from the line number information table for the compilation unit containing the debugging information entry and represents the source file in which the declaration appeared (see Section ). The value 0 indicates that no source file has been specified.

			 * \note	if this is zero, then this information
			 *		is unavailable
			 */
			Dwarf_Unsigned call_file;
			/*! number of the source code line where the call to a concrete inline expansion of a subroutine occurred
			 *
			 * applicable only when the is_concrete_inline_instance_root flag
			 * above is nonzero; as the dwarf(now version 4) standard says:

			 An inlined subroutine entry may also have DW_AT_call_file, DW_AT_call_line and DW_AT_call_column attributes, each of whose value is an integer constant. These attributes represent the source file, source line number, and source column number, respectively, of the first character of the statement or expression that caused the inline expansion. The call file, call line, and call column attributes are interpreted in the same way as the declaration file, declaration line, and declaration column attributes, respectively (see Section 2.14).

and:

The value of the DW_AT_decl_line attribute represents the source line number at which the first character of the identifier of the declared object appears. The value 0 indicates that no source line has been specified.

			 * \note	if this is zero, then this information
			 *		is unavailable
			 */
			Dwarf_Unsigned call_line;
		};
		/*! source code file declaration coordinates for a subprogram
		 *
		 * applicable only when the is_concrete_inline_instance_root flag
		 * above is zero */
		struct
		{
			/*! source code file number
			 *
			 * source code file number of the declaration of this
			 * subprogram, as recorded in the
			 * line number information table corresponding to the
			 * compilation unit containing this subprogram
			 *
			 * if this equals zero, then no file number has been specified */
			Dwarf_Unsigned		srcfile_nr;
			/*! source code line number
			 *
			 * source code line number of the declaration of this
			 * subprogram, as recorded in the
			 * line number information table corresponding to the
			 * compilation unit containing this subprogram
			 *
			 * if this equals zero, then no line number has been specified */
			Dwarf_Unsigned		srcline_nr;
		};
	}
	;

	/*! the table of address range(s) (if any) that this subprogram die covers */
	struct dwarf_ranges_struct * addr_ranges;
	/*! the entry point program counter address (if applicable) of this function
	 *
	 * this field is applicable if and only if the subprogram described
	 * by this node physically occupies memory (i.e., is not a
	 * declaration or an abstract inline instance root entry);
	 * this field is applicable and valid if and only if the
	 * is_entry_point_valid field above is nonzero */
	Dwarf_Addr	entry_pc;
	/*! name of the subroutine */
	char *		name;
	/*! return type for the subroutine */
	struct dtype_data *	type;
	/*! list of formal parameters */
	struct dobj_data *	params;
	/*! list of local variables */
	struct dobj_data *	vars;
	/*! list of nested lexical blocks */
	struct lexblock_data *	lexblocks;
	/*! used to build a list of sibling subroutines
	 *
	 * currently, this can be:
	 *	- a list of the top-level subprograms
	 *	in a compilation unit
	 *	- or the list of subprograms inlined in
	 *	another subprogram, or inside a lexical block
	 *	of a subprogram
	 *	- or the list of member functions of a class
	 *
	 * \todo	maybe change the name of this field to "next"
	 *		for better readability 
	 */
	struct subprogram_data *	sib_ptr;
	/*! a list of subprograms inlined in this subprogram
	 *
	 *	\todo	also support nested subprograms */
	struct subprogram_data *	inlined_subprograms;
	/*! frame base location information
	 *
	 * this field contains valid data only if the
	 * is_frame_base_available flag above is nonzero */
	struct dwarf_location		fb_location;
	/*! cxx linkage name (if present)
	 *
	 * this is valid only when non-null, in that case this containing
	 * node should describe some cxx struct/class member function;
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
struct subprogram_data * subprogram_process(struct gear_engine_context * ctx, Dwarf_Die die);

#endif /* __SUBPROGRAM_ACCESS_H__ */

