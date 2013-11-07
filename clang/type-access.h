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
	/*! unsigned integer type */
	CTYPE_UNSIGNED,
	/*! signed integer type */
	CTYPE_SIGNED,
	/*! aggregate type (structure/union or array) */
	CTYPE_AGGREGATE,
	/*! a struct or union type */
	CTYPE_STRUCT_OR_UNION,
	/*! a subroutine type */
	CTYPE_SUBROUTINE,
};


/*! basic (dwarf debug information representation mandated) data type class enumeration
 *
 * used in conjunction with ::dtype_data; these
 * closely follow the dwarf debug information representation
 * (and really correspond to dwarf standard tags)
 *
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
	/*! an 'enum' enumeration type */
	DTYPE_CLASS_ENUMERATION,
	/*! an enumeration constant (enumerator) */
	DTYPE_CLASS_ENUMERATOR,
	/*! a member of an aggregate type (currently, struct or union) */
	DTYPE_CLASS_MEMBER,
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
	/*! reference counter for this type
	 *
	 * \todo	this was intended to be used in conjunction
	 * 		with the hashing code, when building the type trees,
	 *		but is currently not used and may be scheduled
	 *		for removal */
	unsigned int	ref_cnt;
	/*! basic type class of this type - see the ::DTYPE_CLASS_ENUM enumerator for details */
	enum DTYPE_CLASS_ENUM	dtype_class;
	/*! size of the data described by the node (if relevant), in bytes 
	 *
	 * \todo	maybe change this to the more appropriate 'byte_size'
	 */
	unsigned int		data_size;
	/*! the name of the type entity, if appropriate and/or available
	 * 
	 * applicable in cases of:
	 *	- an aggregate type name (struct/union)
	 *	- an enumeration name
	 *	- an enumerator (enumeration constant) name
	 *	- a typedef definition
	 */
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
		/*! aggregate type member data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_MEMBER */
		struct
		{
			/*! pointer to the type of an aggregate type member */
			struct dtype_data *	member_type;
			/*! pointer to the sibling of the member */
			struct dtype_data *	sib_ptr;
			/*! aggregate type location description - offset of a member field from the base of an aggregate type
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
		}
		member_data;

		/*! struct/union type data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_STRUCT or DTYPE_CLASS_UNION */
		struct
		{
			/*! the head of a list of members for the struct */
			struct dtype_data *	members;
		}
		struct_data;

		/*! enumeration type data
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_ENUMERATION */
		struct
		{
			/*! the head of a list of enumeration constants (enumerators) for an enumeration */
			struct dtype_data *	members;
		}
		enum_data;

		/*! enumeration constant (enumerator) data; the name is held in the ::name field above
		 *
		 * applicable when the dtype_class field above
		 * equals DTYPE_CLASS_ENUMERATOR */
		struct
		{
			/*! pointer to the sibling of the enumeration constant */
			struct dtype_data *	sib_ptr;
			/*! the constant value correpsonding to the symbollic enumeration constant name */
			int			enum_const;
		}
		enumerator_data;

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
void type_dump_data_mi(struct dtype_data * type, void * data);
int dtype_access_sizeof(struct dwarf_head_struct * item);
int dtype_access_are_types_compatible(struct dtype_data * t1, struct dtype_data * t2);
bool dtype_access_is_ctype(enum CTYPE_ENUM what_type, struct dtype_data * type);
struct dtype_data * dtype_access_get_deref_type(struct dtype_data * type);

