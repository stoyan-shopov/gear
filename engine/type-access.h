/*!
 *	\file	type-access.h
 *	\brief	data type information access
 *	\author	shopov
 *
 *	\todo	document how the data type information is stored and processed
 *	\todo	handle anonymous struct/union members
 *
 *	\todo	maybe handle arrays differently; right now,
 *		they tend to need some special case
 *		handling because they are a special case
 *		as mandated by the dwarf representation of arrays
 *		which is currently used virtually verbatim
 *		by the debug information tree building code;
 *		the need for special case handling of arrays
 *		stems from the fact that multidimensional
 *		arrays are not represented as arrays of arrays
 *		(of arrays, etc.) as thought of in c, and this
 *		leads to awkward handling at some places
 *
 *	\todo	clean up the names in the dtype_data data structure - make them shorter
 *
 *	Revision summary:
 *
 *	$Log: $
 */

#ifndef __TYPE_ACCESS_H__
#define __TYPE_ACCESS_H__

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

/*! basic c language type classification enumeration 
 *
 * for details, refer to the iso/iec 9899 c language
 * standard (technical corrigendum 2 (tc2) was used
 * when writing this); this document, as of time of
 * this writing, is freely available
 *
 * quoting the forementioned document:
 *
 * The meaning of a value stored in an object or returned by a function is determined by the
 * type of the expression used to access it. (An identifier declared to be an object is the
 * simplest such expression; the type is specified in the declaration of the identifier.) Types
 * are partitioned into object types (types that fully describe objects), function types (types
 * that describe functions), and incomplete types (types that describe objects but lack
 * information needed to determine their sizes).
 *
 * end of quotation
 *
 * in brief, object types fall in two broad categories:
 *	- scalar types
 *	- aggregate types
 * furthermore, scalar types are divided into:
 *	- arithmetic types
 *	- pointer types
 * and aggregate types are divided into:
 *	- structure/unions (struct/union types) - note that, technically,
 *		unions are not regarded as aggregates (because they
 *		can contain only a single element at a time) by the
 *		c standard, but they are called aggregate here nonetheless
 *	- arrays (array types)
 * arithmetic types are furthermore subdivided into:
 *	- integer types
 *	- real types
 *	- complex types
 * with integer types being:
 *	- signed
 *	- unsigned
 *
 * also, the arithmetic types (along with the
 * special 'char' type distinguished by the
 * iso/iec 9899 standard) are also referred to
 * as the 'basic types'; in the gear, these are
 * often called also the base types and a 'char'
 * type is currently not distinguished as a special
 * one - but it must be stressed that 'char'
 * type signedness is implementation specific
 * (and as an example, it is by default unsigned
 * for gcc for the arm)
 *
 * \note	as seen by the comments above,
 *		the type classification in the gear
 *		does not correspond in all aspects
 *		to what is stated in the
 *		iso/iec 9899 c language standard; yet,
 *		in practice, the standard classification
 *		and the classification in the gear
 *		are virtually equivalent - and very much
 *		please correct me on anything that does not
 *		fit this view
 *
 * summarizing object type hierarchy:
 *	- scalar
 *		- pointer
 *		- arithmetic
 *			- integer
 *				- signed
 *				- unsigned
 *			- real
 *			- complex
 *	- aggregate
 *		- structures/unions
 *		- arrays
 *
 * in this enumeration, only the type classes
 * actually used/of interest to the gear in the
 * classification above are enumerated
 *
 *	\todo	handle complex, floating and boolean types here
 *	\todo	maybe precompute this and store it in a
 *		field in the ::dtype_data data structure for types
 */
enum CTYPE_ENUM
{
	/*! invalid type encoding, used for catching errors */
	CTYPE_INVALID = 0,
	/*! a scalar type */
	CTYPE_SCALAR,
	/*! an arithmetic type */
	CTYPE_ARITHMETIC,
	/*! a pointer type */
	CTYPE_POINTER,
	/*! an integer type */
	CTYPE_INTEGER,
	/*! floating-point numeric type */
	CTYPE_FLOAT,
	/*! unsigned integer type */
	CTYPE_UNSIGNED,
	/*! signed integer type */
	CTYPE_SIGNED,
	/*! aggregate type (structure/union/class or array) */
	CTYPE_AGGREGATE,
	/*! a struct or union type, or a cxx class type */
	CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS,
	/*! a subroutine type */
	CTYPE_SUBROUTINE,
};


/*! basic (dwarf debug information representation mandated) data type class enumeration
 *
 * used in conjunction with ::dtype_data; these
 * closely follow the dwarf debug information representation
 * (and really correspond to dwarf standard tags)
 *
 * \todo	the names got stupid when DTYPE_CLASS_CLASS was added,
 *		maybe change the names below
 * \todo	sort these in a more consistent way */
enum DTYPE_CLASS_ENUM
{
	/*! invalid type class - used for catching errors */
	DTYPE_CLASS_INVALID = 0,
	/*! base type - one not described by means of other types
	 *
	 * one of the DW_ATE_* described types in the dwarf standard */
	DTYPE_CLASS_BASE_TYPE,
	/*! a pointer type
	 * 
	 * a pointer; the ::ptr_type in the ::gendata field of the ::dtype_data pointer below points to
	 * the type that the pointer points to
	 * \todo	handle void pointers */
	DTYPE_CLASS_PTR,
	/*! an array */
	DTYPE_CLASS_ARRAY,
	/*! an array subrange
	 * 
	 * an array subrange, this can be heavily optimised as only the ::upper_bound
	 * field in dtype_data.gendata.arr_subrange_data is currently used;
	 */
	DTYPE_CLASS_ARRAY_SUBRANGE,
	/*! a 'struct' structure type */
	DTYPE_CLASS_STRUCT,
	/*! a 'union' structure type */
	DTYPE_CLASS_UNION,
	/*! a 'class' */
	DTYPE_CLASS_CLASS,
	/*! an 'enum' enumeration type */
	DTYPE_CLASS_ENUMERATION,
	/*! a member of an aggregate type (currently - struct, union or a class) */
	DTYPE_CLASS_MEMBER,
	/*! a list of base classes that have a common superclass */
	DTYPE_CLASS_BASE_CLASS_LIST,
	/*! a 'typedef' declared type */
	DTYPE_CLASS_TYPEDEF,
	/*! a subroutine type */
	DTYPE_CLASS_SUBROUTINE,
	/*! a type qualifier
	 *
	 * the available type qualifiers are enumerated
	 * in the TYPE_QUALIFIER_ENUM enumeration below */
	DTYPE_CLASS_TYPE_QUALIFIER,
};

/*! a type qualifier enumeration */
enum TYPE_QUALIFIER_ENUM
{
	/*! invalid type qualifier - used for catching errors */
	TYPE_QUALIFIER_INVALID = 0,
	/*! a c 'const' type qualifier */
	TYPE_QUALIFIER_CONST,
	/*! a c 'restrict' type qualifier */
	TYPE_QUALIFIER_RESTRICT,
	/*! a c 'volatile' type qualifier */
	TYPE_QUALIFIER_VOLATILE,
};


/*! data type information holding data structure */
struct dtype_data
{
	/*! the head of this structure */
	struct dwarf_head_struct	head;

	/*! various flags; which of them are actually applicable, depends on the data object class */
	struct
	{
		/*! basic type class of this type - see the ::DTYPE_CLASS_ENUM enumerator for details */
		enum DTYPE_CLASS_ENUM	dtype_class : 5;
		/*! denotes if this data node describes a data object declaration
		 *
		 * if this field is nonzero, then this data type node
		 * describes a non-defining/incomplete data type;
		 * as the dwarf standard, paragraph 2.13.1 and
		 * paragraph 2.13.2 say:
		 *
		 * 2.13.1 Non-Defining Declarations Debugging information entries that represent non-defining or otherwise incomplete declarations of a program entity have a DW_AT_declaration attribute, whose value is a flag.
		 * 2.13.2 Declarations Completing Non-Defining Declarations Debugging information entries that represent a declaration that completes another (earlier) nondefining declaration, may have a DW_AT_specification attribute whose value is a reference to the debugging information entry representing the non-defining declaration. Debugging information entries with a DW_AT_specification attribute do not need to duplicate information provided by the debugging information entry referenced by that specification attribute.
		 *
		 * notable missing information from data type nodes
		 * describing (non-defining) data type
		 * declarations are location descriptions 
		 * and byte sizes */
		bool	is_node_a_declaration	: 1;
		/*! when 1, denotes that this node describes a static cxx struct/class data member (variable)
		 *
		 * note that this implies that the dtype_class field for
		 * this node must equal DTYPE_CLASS_MEMBER; for
		 * details on this flag's purpose and interpretation,
		 * read the comments about the member_data.member_location
		 * field below */ 
		bool	is_node_a_static_cxx_struct_member	: 1;
	};
	/*! reference counter for this type
	 *
	 * \todo	this was intended to be used in conjunction
	 * 		with the hashing code, when building the type trees,
	 *		but is currently not used and may be scheduled
	 *		for removal */
	//unsigned int	ref_cnt;
	/*! size of the data described by the node (if relevant), in bytes 
	 *
	 * \todo	maybe change this to the more appropriate 'byte_size'
	 */
	unsigned int		data_size;
	/*! the name of the type entity, if appropriate and/or available */
	char *		name;
	/*! generic data describing this data node, interpretation depends on the value of the dtype_class field above */
	union
	{
		/*! array type data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_ARRAY */
		struct
		{
			/*! the head of a list of array subrange type nodes */
			struct dtype_data *	subranges;
			/*! the type of the elements of the array */
			struct dtype_data *	element_type;
		}
		arr_data;
		/*! array subrange type data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_ARRAY_SUBRANGE */
		struct
		{
			/*! pointer to the next subrange sibling node for the described array */
			struct dtype_data *	sib_ptr;
			/*! the upper bound (index) covered by this subrange entry, inclusive
			 *
			 * in the case of c flexible array members - this
			 * entry is set to UINT_MAX
			 *
			 * \todo	this field is currently not validated */
			unsigned int		upper_bound;
			/*! the type of the elements of the array
			 *
			 * this duplicates the element_type in the arr_data
			 * for the parent of this node (whose dtype_class
			 * is DTYPE_CLASS_ARRAY) - this facilitates some
			 * special case processings regarding arrays
			 *
			 * \todo	another way to do this would be
			 *		to use the parent field in the
			 *		dwarf_head_struct field to
			 *		retrieve the parent, and then
			 *		the element type, but i(sgs) dont
			 *		think this has any benefits; settle
			 *		this down
			 */
			struct dtype_data	* element_type;
		}
		arr_subrange_data;
		/*! base type encoding, one of the DW_ATE_* described types in the dwarf standard
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_BASE_TYPE */
		int	base_type_encoding;

		/*! a list of base classes corresponding to a common superclass
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_BASE_CLASS_LIST
		 *
		 * \todo	this is virtually the same as the member_data
		 *		field below, only the names differ, maybe merge
		 *		these */
		struct
		{
			/*! a pointer to the actual type of this base class */
			struct dtype_data *	class_type;
			/*! a pointer to the next base class in the list */
			struct dtype_data *	sib_ptr;
			/*! base class location description within the derived superclass
			 *
			 * the value of this field, as the dwarf standard says,
			 * describes the location of the beginning of the
			 * inherited type relative to the beginning address
			 * of the derived class
			 *
			 * \todo	currently (as is also the case for
			 *		structure members), this field only
			 *		supports a constant offset value,
			 *		there is currently no support for
			 *		generic location descriptions
			 *
			 * \note	a value of UINT_MAX in this field
			 *		denotes that the location information
			 *		is unknown/invalid */
			unsigned int		location_in_derived_class;
		}
		inheritance_data;

		/*! aggregate type member data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_MEMBER
		 *
		 * \todo	this is virtually the same as the inheritance_data
		 *		field above, only the names differ, maybe merge
		 *		these */
		struct
		{
			/*! pointer to the type of an aggregate type member */
			struct dtype_data *	member_type;
			/*! pointer to the sibling of the member */
			struct dtype_data *	sib_ptr;
			/*! aggregate type location description
			 * 
			 * note that this value has two different interpretations
			 * depending on the value of the is_node_a_declaration
			 * flag above - when this flag is zero, this location
			 * value is the offset of the data member field from
			 * the base of the appropriate aggregate type
			 * (struct or class), and when this flag is
			 * nonzero, this location value is the absolute
			 * address in target memory of the data member field
			 *
			 * a value of UINT_MAX in this field means
			 * there is no location description associated
			 * with this aggregate data member - this is
			 * the usual case with c union data members;
			 * when the debug information tree for a data
			 * type is built, it is the builder's responsibility
			 * (usually this is function type_process() in module
			 * type-access.c) to make sure that:
			 * 	- for c union members this field is
			 * 	correctly 'fixed up' when the debug
			 * 	information subtree for the member
			 * 	is built - this means overwriting
			 * 	this field with zero if it equals
			 * 	UINT_MAX, and
			 * 	- for c structure data members
			 * 	this field contains a value that
			 * 	is different than UINT_MAX
			 */
			unsigned int		member_location;
			/*! aggregate member size in bits (if appropriate)
			 *
			 * this corresponds to a dwarf DW_AT_bit_size
			 * attribute, and is applicable only when nonzero
			 * (that is, if this field is zero, then no
			 * DW_AT_bit_size attribute is present) */
			unsigned int		bit_size;
			/*! aggregate member offset in bits (if appropriate)
			 *
			 * this corresponds to a dwarf DW_AT_bit_offset
			 * attribute, and is applicable only when nonzero
			 * (that is, if this field is zero, then no
			 * DW_AT_bit_offset attribute is present,
			 * or if present, it is zero) */
			unsigned int		bit_offset;
		}
		member_data;

		/*! struct/union/class type data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_STRUCT, DTYPE_CLASS_UNION 
		 * or DTYPE_CLASS_CLASS
		 *
		 * \todo	maybe change the name because of classes */
		struct
		{
			/*! a list of inherited base classes, if any
			 *
			 * each node in this list corresponds to a die
			 * owned by the class described by this dtype_data
			 * node with the tag DW_TAG_inheritance, and
			 * a DW_AT_type type attribute containing
			 * a reference to the base class type die */
			struct dtype_data *	inherited_bases;
			/*! a list of member functions in this class */
			struct subprogram_data	* member_functions;
			/*! the head of a list of members for the struct/union/class */
			struct dtype_data *	data_members;
		}
		struct_data;

		/*! enumeration type data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_ENUMERATION */
		struct
		{
			/*! an array holding the enumeration constants (enumerators) for this enumeration */
			struct
			{
				/*! the enumerator symbollic name */
				const char	* name;
				/*! the constant value correpsonding to the symbollic enumeration constant name */
				int	enum_const;
			}
			* enumerators;
			/*! the number of enumerator entries in the 'enumerators' array above */
			int nr_enumerators;
		}
		enum_data;

		/*! the type pointed to by a DTYPE_CLASS_PTR node
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_PTR */
		struct dtype_data *	ptr_type;

		/*! typedef entry specific data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_TYPEDEF */
		struct
		{
			/*! the type described by a typedef entry (DTYPE_TYPEDEF_ENTRY) */
			struct dtype_data *	type;
		}
		typedef_data;
		/*! type qualifier specific data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_TYPE_QUALIFIER */
		struct
		{
			/*! the specific type qualifier described by this entry */
			enum TYPE_QUALIFIER_ENUM qualifier_id;
			/*! the actual type data for this qualified type */
			struct dtype_data *	type;
		}
		type_qualifier_data;
		/*! subroutine data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_SUBROUTINE */
		struct
		{
			/*! the subroutine node describing this subroutine type */
			struct subprogram_data *	sub;
		}
		subroutine_data;
	};
#if 1
	/*! cxx linkage name (if present) 
	 *
	 * this is valid only when non-null, in that case this containing
	 * node should describe some cxx struct/class member (function
	 * or variable); this linkage name is currently used as a quick
	 * (and maybe not very reliable) helper to facilitate reading
	 * cxx struct/class debug information, which may be dispersed
	 * through compilation units and has to be stitched together */
	const char * linkage_name;
#endif	
};

/*
 *
 * exported function prototypes follow
 *
 */
int dtype_access_get_type_die(struct gear_engine_context * ctx, Dwarf_Die die, Dwarf_Die * type_die);
struct dtype_data * type_process(struct gear_engine_context * ctx, Dwarf_Die die);
void type_data_dump(struct dtype_data * type, void * data, char * name, int first_atom, int last_atom);
void type_dump_mi(struct dtype_data * type, const char * name, bool is_in_deref);
void type_dump_data_mi(struct gear_engine_context * ctx, struct dtype_data * type, void * data);
int dtype_access_sizeof(struct dwarf_head_struct * item);
bool dtype_access_are_types_compatible(struct dtype_data * t1, struct dtype_data * t2);
bool dtype_access_is_ctype(enum CTYPE_ENUM what_type, struct dtype_data * type);
struct dtype_data * dtype_access_get_deref_type(struct dtype_data * type);
struct dtype_data * dtype_access_get_unqualified_base_type(struct dtype_data * type);

void init_types(struct gear_engine_context * ctx);

#endif /* __TYPE_ACCESS_H__ */

