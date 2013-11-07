/*!
 * \file	symtab.h
 * \brief	arm gear symbol table handling
 * \author	shopov
 *
 * enumerated here are all program entities that possess the concept of
 * 'name' (i.e., identifiers) that the debugger bothers to care about;
 * also read below the rationale for choosing which identifiers to put
 * in the symbol table, and which not
 *
 * c has 4 namespaces: labels, structure and union
 * tags, structure and union members, and all else;
 * amongst these:
 *	- labels - these are not accounted for in
 *	the symbol table
 *	- structure, union and enumeration tags - of these
 *	accounted for are structure, union and enumeration tags
 *	declared at file scope only -
 *	these are accounted for by SYM_TYPE symbol
 *	table entries entries; structure, union
 *	and enumeration tags at block scopes are not accounted for
 *	- structure and union members, and enumeration
 *	constants in enumerations not at file scope - these are
 *	not accounted for
 *	- all else - of these, data objects with static
 *	storage duration are accounted for by SYM_DATA_OBJECT
 *	symbol table entries, type definitions (typedefs)
 *	at file scope are accounted for by SYM_TYPE symbol
 *	table entries, functions are accounted for by
 *	SYM_SUBROUTINE symbol table entries, enumeration
 *	tags at file scope are accounted
 *	for by SYM_TYPE symbol table entries, enumeration
 *	constants (enumerators) in enumerations at file
 *	scope are accounted for by
 *	SYM_ENUM_CONSTANT symbol table entries
 *
 * also, identifiers in c have 4 scopes:
 *	- function - labels are the only identifiers with
 *	function scope; these (as already said) are not
 *	accounted for in the symbol table
 *	- function prototype - these are also not accounted
 *	for in the symbol table
 *	- block - these are generally not accounted for
 *	in the symbol table, unless the identifier denotes
 *	a data object with static storage duration
 *	- file - these are accounted for
 *
 * in summary, identifiers accounted for in the symbol table are:
 *	- identifiers for data objects with static storage duration, and
 *	- structure, union and enumeration tags, type definitions,
 *	all of which are declared at file scope, enumeration
 *	constants in enumerations at file scope
 *	
 * identifiers not accounted for in the symbol table are:
 *	- function labels
 *	- data objects declared at block scope which do not
 *	have static storage duration, function parameters
 *	- structure, union and enumeration tags, type definitions,
 *	all of which are not declared at file scope,
 *	enumeration constants in enumerations not at file scope
 *
 * also see the comments about the SYM_CLASS_ENUM enumeration below
 *
 * rationale (for supporting some identifiers, and not supporting
 * others) - the idea of putting the identifiers of the types
 * already mentioned above in the symbol table, is that the symbol
 * table should supply only symbols at a, so to say, global program
 * scope - that is, identifiers which are valid throughout the whole life of
 * a program, and the interpretation of which either:
 *	- does not depend on the program excecution context details
 *	(i.e. which function is currently executing) - this is the case
 *	when an identifier is unique, or
 *	- depends on a minimal specification of context - e.g. a source
 *	file or a compilation unit - the case when an identifier is
 *	not unique throughout different program execution contexts
 *
 *	\note	as an aside, note that, under the above assumptions,
 *		it is legitimate to use in a debugger expression
 *		unique identifiers with internal linkages from different
 *		compilation units (e.g. c variables with 'static' storage
 *		class specifiers in different c source files), whereas
 *		such use from within a c program is illegitimate
 *
 * that is to say, the symbol table is expected to hold only the most
 * useful identifiers to be used when evaluating expressions; if identifiers
 * not covered by the symbol table are needed to be used in an expression, they
 * should be further qualified to distinguish in what execution context
 * they are to be used (e.g.: "file:line-nr:id", "file:function:id", "
 * "function:line-nr:id", or even something like "core-addr-expression:id", 
 * etc), in which case specialized identifier scope resolution should come
 * in effect, which is most easily and effectively handled by scanning the
 * suprogram debug information data structures, without the use of the
 * symbol table; note however that such
 * qualifications shall be rarely needed or useful, because, probably,
 * the most common occasion where such qualifications will be needed, will
 * be when examining program state when a program halts; however, in this case,
 * identifier scope resolution is automatically performed by the gear engine
 * based on program execution context - i.e. the program counter value
 * (from which the executing function and line number can usually be
 * inferred) at which the program is halted
 *
 *
 * Revision summary:
 *
 * $Log: $
 */

/*
 *
 * exported types follow
 *
 */

/*! the symbol class enumeration
 *
 * see the comments at the start of this file for information
 * on what symbols are stored in the symbol table, and what not;
 * documented here is the payload pointed to by the symbol
 * pointer (anonymous union) in the sym_struct below */
enum SYM_CLASS_ENUM
{
	/*! invalid symbol class, used to catch bugs */
	SYM_INVALID = 0,
	/*! a data object(a variable)
	*
	* payload is dobj */
	SYM_DATA_OBJECT,
	/*! a subprogram
	*
	* payload is subprogram */
	SYM_SUBROUTINE,
	/*! a type description
	 *
	 * payload is dtype */
	SYM_TYPE,
	/*! an enumeration constant (enumerator) description
	 *
	 * payload is enum_const_data - a dtype_data pointer to the enumeration
	 * constant data type node (the enumerator) */
	SYM_ENUM_CONSTANT,
};

/*! data structure describing a single program symbol */
struct sym_struct
{
	/*! the symbol class, see ::SYM_CLASS_ENUM for details */
	enum SYM_CLASS_ENUM	symclass;
	/*! pointer to payload, interpretation depends on the symbol class, see SYM_CLASS_ENUM for details */
	union
	{
		/*! a pointer used to access SYM_DATA_OBJECT entries */
		struct dobj_data	* dobj;
		/*! a pointer used to access SYM_SUBROUTINE entries */
		struct subprogram_data	* subprogram;
		/*! a pointer used to access SYM_TYPE entries */
		struct dtype_data	* dtype;
		/*! a pointer used to access SYM_ENUM_CONSTANT entries */
		struct dtype_data	* enum_const_data;
	};
	/*! pointer to the next symbol in the list of symbols with a common name */
	struct sym_struct	* next;
};

/*
 *
 * exported function prototypes follow
 *
 */


void init_symtab(struct gear_engine_context * ctx);
struct sym_struct * symtab_find_sym(struct gear_engine_context * ctx, const char * name);
void symtab_store_sym(struct gear_engine_context * ctx, const char * name, enum SYM_CLASS_ENUM symclass, void * payload);

