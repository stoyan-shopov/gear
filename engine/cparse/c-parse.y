/*!
 *	\file	c-parse.y
 *	\brief	c expression parser bison source file
 *	\author	shopov
 *
 *	\note	throughout the comments in this file, the parsing
 *		function names 'yyparse()' and 'c_lang_parse()',
 *		as well as the lexical token scanning function
 *		names 'yylex()' and 'c_lang_lex()', are used
 *		interchangeably
 *
 *	this is the main source file responsible for the c language
 *	expression parsing and evaluation; the expression grammar
 *	supported here is a subset of the c language expression
 *	grammar; i(sgs) used as a base for this the yacc grammar
 *	i got from http://www.lysator.liu.se/c/
 *	(then again, i might be wrong and could have gotten it
 *	from somewhere else; doesnt really matter that much)
 *
 *	http://www.lysator.liu.se/c/ is a wonderful page of resources
 *	for the c language and other c related stuff;
 *	i(sgs) got the base source for the lexer from there as well
 *
 *	as written above, not all of the c language expression
 *	grammar is supported here; notable omissions are increment/
 *	decrement operators; assignment operators are unsupported
 *	right now, but will most probably be supported in the next
 *	major gear release; all of the mentioned operators
 *	have side effects and this is the reason not to support these
 *	in this first release of the gear; it is not really a problem
 *	to support these, but i(sgs) think it would be wiser to
 *	keep things not too complex for the first release of the gear;
 *	relational operators are not handled right now as well
 *
 *	\todo	fix the comments above when support for side-effects
 *		inflicting operators is added
 *
 *	the c expression parsing logic is needed at various places
 *	such as supporting automatically displayed expression values
 *	when debugging a program, determining values when the
 *	user enters commands, etc.
 *
 *	\todo	currently, an expression is not preprocessed in
 *		any way, it is instead reparsed each time its value
 *		is needed; this is obviously quite bad; some simple
 *		expression preprocessing/precompiling must be
 *		implemented (e.g. building a simple parse tree(abstract
 *		syntax tree), which can be processed whenever
 *		an expression value is needed instead of
 *		reparsing everything from scratch)
 *
 *	in the parser, when an error occurs, resources are deallocated
 *	via the "%destructor" bison construct, however destructors are
 *	invoked by the generated parser, when symbols are being
 *	discarded, and symbols being discarded are of 4 kinds
 *	(quoting the bison manual):
 *
 *		symbols popped during the first phase of error recovery,
 *		incoming terminals during the second phase of error recovery,
 *		the current look-ahead and the entire stack (except the current right-hand side symbols) when the parser returns immediately, and
 *		the start symbol, when the parser succeeds.
 *	(The parser can return immediately because of an explicit call to
 *	YYABORT or YYACCEPT, or failed error recovery, or memory exhaustion.
 *	Right-hand side (in the original - "size" - apparently a typographical error)
 *	symbols of a rule that explicitly triggers a syntax
 *	error via YYERROR are not discarded automatically. As a rule of thumb,
 *	destructors are invoked only when user actions cannot manage the memory. )
 *
 *	memory should not be deallocated in the last case only - this
 *	is the case of a normal parse, where the 'parse_head' variable
 *	contains the value of the start token; in the other three cases
 *	the allocated resources must be freed, these other cases rely
 *	on the proper setting of the 'is_parser_aborted' variable
 *	which *must* be reset to zero prior to entry in the parser
 *	function, and gets set to nonzero on two occassions - invoking the
 *	error reporting function (yyerror() - here, c_lang_error()) - this happens
 *	on a parse error, or aborting the parser via YYABORT - this
 *	happens on a semantical error with an otherwise syntactically
 *	correct construct; whenever the generated parser invoked
 *	destructor code is executed, it checks if the 'is_parser_aborted'
 *	flag is nonzero - and if so - frees allocated resources
 *
 *	\todo	in any case, the above must be checked with valgrind
 *
 *	\note	the rules in the parser grammar are coded to invoke
 *		an appropriate function which performs a calculation on
 *		its parameter(s) and returns a result (e.g. addition,
 *		multiplication, logical operations, etc.); for the sake
 *		of efficiency, it is often the case that some
 *		of the parameters, passed to the functions that do
 *		the calculations, gets overwritten by the result
 *		and the other parameter(s) are discarded (deallocated)
 *		by the calculation function; this is why resources
 *		should not be deallocated by code in the rules
 *		of the grammar; obviously, it is also unsafe for
 *		code in the rules section of the grammar to reference
 *		any variables, which have already been used as parameters
 *		for a calculation function, because they should
 *		have been deallocated and are therefore invalid;
 *		in practice, it is quite improbable that one such
 *		variable would ever be needed after the
 *		calculations referencing it have been done
 *	\note	as a convention, the validity of the parameters to
 *		functions invoked from within the rules section
 *		of the grammar is not checked in the rules section and
 *		*must* be done by the functions themselves;
 *		also, as a convention, all functions invoked from within the
 *		rules section of the expression grammar return pointers
 *		to data structures of type exp_data_node (detailed below)
 *		and in case of an error - when parsing should be aborted -
 *		a null pointer must be returned, so that the parsing function (yyparse())
 *		knows parsing should be aborted by invoking the YYABORT()
 *		macro; it is the invoked function's responsibility, which
 *		detects the error and returns a null pointer, to set the
 *		is_parser_aborted flag to true and to adequately
 *		flag the error condition by setting the errcode and parse_msg
 *		members of the parser_state data structure prior to returning
 *		null to the parsing function generated from the rules section
 *		of the expression grammar (yyparse()), which then invokes YYABORT()
 *
 *	\todo	clean up the c_lang_error function
 *	\todo	this module may benefit from a simple local memory
 *		management scheme such as maintaining a pool of free and
 *		allocated exp_data_node lists
 *	\todo	clean up the sanity checks in many of the routines here;
 *		some of these routines were historically in a separate
 *		module, but are now here, which voids many of the checks
 *	\todo	maybe use the flex option-header to generate a header
 *		file and use it - it is cleaner; i(sgs) was not aware
 *		of this option until recently...
 *	\todo	handle floating point types; currently they are not handled
 *		at all, and quite some tweaking must be done to support them
 *		(they are arithmetic types, and as such, are also scalar types)
 *	\note	the resolve_id() and fetch_data() routines in this module are
 *		the only ones in this module that are allowed to access target
 *		resources (via a struct core_control target interface)
 *	\todo	the resolve_id() and fetch_data() routines in this module are
 *		the only ones in this module that are allowed to access target
 *		resources (via a struct core_control target interface) - but
 *		right now they are quite broken - they do not take in account
 *		various expression evaluation flags and it is currently
 *		impossible to perform any expression evaluation without
 *		a live target - this is bad, bad, bad...
 *
 *	Revision summary:
 *
 *	$Log: $
 */


%debug

%name-prefix="c_lang_"
%parse-param	{ struct parser_state * state }
%parse-param	{ yyscan_t yyscanner }
%lex-param	{ struct parser_state * state }
%lex-param	{ yyscan_t yyscanner }
%pure-parser

/* bison data type declarations */
%union
{
	struct	exp_data_node	* expval;
	/* c operator code */
	/*! c expression operator enumeration
	 *
	 * \note	not all operators are listed here, some unused/unneeded ones are missing
	 */
	enum C_EXP_OPERATOR_ENUM
	{
		/*! invalid value, used for catching errors */
		C_EXP_OPERATOR_INVALID = 0,
		/*! unary operator '&' */
		UNARY_GETADDR,
		/*! indirection (unary '*') */
		UNARY_DEREF,
		/*! unary '+' */
		UNARY_PLUS,
		/*! unary '-' */
		UNARY_MINUS,
		/*! operator '~' */
		UNARY_BITWISE_NOT,
		/*! operator '!' */
		UNARY_LOGICAL_NOT,
		/*! binary operator '&' */
		BITWISE_AND,
		/*! binary inclusive or '|' */
		BITWISE_OR,
		/*! binary exclusive or '^' */
		BITWISE_XOR,
		/*! binary logical and '&&' */
		LOGICAL_AND,
		/*! binary logical inclusive or '||' */
		LOGICAL_OR,
	}
	opcode;
	/* name of an identifier */
	char	* name;
	/* value of a numeric constant */
	unsigned long long num;
}


%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_SHIFT_OP RIGHT_SHIFT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

/* token type declarations */
%type <num>	CONSTANT
%type <name>	IDENTIFIER


/* nonterminal symbols types */
%type <expval>	expression
%type <expval>	primary_expression
%type <expval>	postfix_expression
%type <expval>	unary_expression
%type <expval>	cast_expression
%type <expval>	multiplicative_expression
%type <expval>	additive_expression
%type <expval>	shift_expression
%type <expval>	relational_expression
%type <expval>	equality_expression
%type <expval>	and_expression
%type <expval>	exclusive_or_expression
%type <expval>	inclusive_or_expression
%type <expval>	logical_and_expression

%destructor {
	/* if an error has not been flagged, do nothing */
	if (state->is_parser_aborted)
	{
		destroy_exp_node(state, $$);
	}
} expression primary_expression postfix_expression
	unary_expression cast_expression multiplicative_expression
	additive_expression shift_expression relational_expression
	equality_expression and_expression exclusive_or_expression
	inclusive_or_expression logical_and_expression

%destructor {
	/* if an error has not been flagged, do nothing */
	if (state->is_parser_aborted)
	{
		free($$);
	}
	} IDENTIFIER


%type <opcode>	unary_operator

%start expression


%{
/*
 *
 * include section follows
 *
 */

#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "target-description.h"
#include "engine-err.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "cu-access.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "dobj-access.h"
#include "aranges-access.h"
#include "type-access.h"
#include "util.h"
#include "c-expression.h"
#include "gear-constants.h"
#include "dwarf-util.h"
#include "scope.h"

#include "c-parse.tab.h"
#include "c-lex-decl.h"

/*! parser state data structure, used to make the parser fully reentrant
 *
 * this data structure holds various data items needed by the parser to:
 *	- pass parameters to the parser (input variables)
 *	- maintain parser state (state variables)
 *	- return result data (output variables)
 * (these three sections are outlined in the coments below)
 * a pointer to such a data structure is passed as a parameter to the
 * parser; memory for the data structure must be allocated by callers of the
 * parser, and the input variables section must be properly initialized
 * prior to invoking the parser; after return from the parser, only the
 * output variables section of the data structure should be considered
 * valid - all other fileds must not be used as, by convention, the parser
 * is free to modify them */
struct parser_state
{
	/*
	 * input variables section
         */

	/*! engine context to work into */
	struct gear_engine_context	* ctx;
	/*! various flags controlling the parser operation */
	struct
	{
		/*! the parser should perform syntax analysis on its input
		 *
		 * when set, this flag causes the parser to perform a very rough
		 * syntax analysis of the input expression, so rough that
		 * scope is not taken in account, and not even data type
		 * resolution is performed - the primary goal of this
		 * analysis is to rule out expressions which are obviously
		 * invalid (e.g., "3+++", "%!*", etc.)
		 *
		 * \note	the convention is, when in this mode,
		 *		no flags in the expression data
		 *		nodes to ever get set, which facilitates
		 *		the catching of errors; see also the
		 *		comments for the flags field in
		 *		struct exp_data_node
		 *
		 * \todo	this is currently coded in the parser
		 *		but the interface is not exposed
		 *		(in functions such as 
		 *		c_expression_eval_and_print()) as
		 *		it has not been needed yet; expose
		 *		the interface if/when it is needed
		 * \todo	this is currently not really used
		 *		and is not tested at all
		 */
		int	is_syntax_checked	: 1;
		/*! the parser should perform type evaluation on its input
		 *
		 * when set, this flag causes the parser to perform a syntax
		 * analysis on the input expression (this flag therefore
		 * implies the is_syntax_checked flag above) and evaluate
		 * the expression for its type only taking in account symbol scope;
		 * this means that the result of the evaluation of the
		 * expression is its resulting type, and not its value;
		 * in the course of the type evaluation, the debugged
		 * target is never accessed, which means the target
		 * may be unavailable when performing type evaluation
		 * of an expression (target resources such as memory
		 * and register contents in this case are not needed);
		 * this flag is here to support "whatis -e "expression""
		 * type queries from the user
		 *
		 * \note	the convention is, when in this mode,
		 *		no flags in the expression data nodes,
		 *		other than the is_addr_applicable flag,
		 *		to ever get set, which facilitates
		 *		the catching of errors; see also the
		 *		comments for the flags field in
		 *		struct exp_data_node
		 *
		 * \note	when set, this flag implies the is_syntax_checked
		 *		flag above
		 * \todo	this is currently not really used
		 *		and is not tested at all; it is always
		 *		assumed that an expression is being
		 *		evaluated at least for its type (makes sense);
		 *		this is currently redundant - maybe remove it
		 *		altogether
		 */
		int	is_eval_type	: 1;
		/* the parser should perform a full-blown expression evaluation of its input
		 *
	         * when set, this flag causes the parser to perform
		 * a syntax analysis on the input expression and
		 * evaluate the expression for its value (in addition
		 * to its type - which means this flag implies the
		 * is_eval_type flag above); in the course of evaluating
		 * the expression, symbol scoping is taken in account and
		 * target memory is accessed to fetch the data object
		 * values needed for the evaluation of the expression value;
		 * this obviously means that the target must be available
		 * for the evaluation to take place (in contrast to the
		 * cases for the flags above)
		 *
		 * \note	when set, this flag implies the
		 *		is_eval_type flag above */
		int	is_eval_value	: 1;
	}
	flags;
	/*
	 * state variables section
         */

	/*! parser abortion flag
	 *
	 * this is used to help deallocating resources when aborting a parse;
	 * this *must* be reset to zero prior to invoking the parser function;
	 * also see the parser memory management comments at the start of this
	 * file, and the comments above the errcode member below; it is an
	 * error that this be set (true) and yyparse() returns zero
	 * (designating parsing success), and vice versa (this is cleared
	 * (false) and yyparse() returns() nonzero, designating parsing failure) */
	bool is_parser_aborted;

	/*! target resources (memory/register) error flag
	 *
	 * this is set to 'true' whenever accessing the target fails and
	 * is an indication that any value computed/fetched is meaningless;
	 * this is a sticky flag that must be set to 'false' prior to deploying
	 * the parser, and is set to 'true' whenever accessing the target
	 * fails, it never gets reset to 'false' during the operation of the
	 * parser */
	bool is_target_access_failed;

	/*! error code returned by the parser
	 *
	 * this *must* be set to GEAR_ERR_NO_ERROR prior to invoking
	 * the parser; it gets set to an error code different than
	 * GEAR_ERR_NO_ERROR whenever the parser is aborted due to an
	 * error, therefore, this is different from GEAR_ERR_NO_ERROR
	 * if, and only if, the is_parser_aborted flag is true (that is,
	 * whenever yyparse() returns nonzero, designating parsing failure ) */
	enum GEAR_ENGINE_ERR_ENUM	errcode;

	/*! a message to print when dumping the result of an expression evaluation in machine interface format
	 *
	 * this should be set to null prior to invoking the parser function
	 * (yyparse()), and may be set to some string, signifying the result
	 * of an expression evaluation (e.g. the cause of an error when evaluating
	 * an expression failed - for example "syntax error"); whenever set to non
	 * null, this message string *must* be deallocated (via free()) when no longer
	 * of use */
	char	* parse_msg;

	/*
	 * output variables section
         */
	/*! the value of the start token, after	the parser function returns to its caller
	 *
	 * this is valid only after a successfull parse (the parser function
	 * returns a zero value), it is otherwise invalid and must be ignored */
	struct exp_data_node	* parse_head;
};

#include "lex.c_lang.c"

/*
 *
 * local types follow
 *
 */


/*! expression data node data structure used in expression evaluation and validation
 *
 * when an expression is being evaluated or validated, instances of this
 * are used to incrementally evaluate or validate the expression; on a
 * lowest level, these are created and initialized by the lexer for
 * lexical tokens such as constants and recognized variable names
 *
 * you can think of this as a subexpression of an expression being
 * processed incrementally, in a bottom-up fashion corresponding to
 * the parse tree that the parser generates
 *
 * an expression data node will normally have a type (unless the
 * type is 'void', but this is not really useful), an associated
 * value (again, unless the type of the data node is 'void'), and
 * may have a location description of the object described by the
 * data node in the target being debugged, if the object occupies
 * physical resources in the target (these resources can be memory
 * and/or register(s))
 *
 * in the case when the data node has a value, this value may or may
 * not be computed and made available at the time a node is built;
 * if the value is marked as invalid (see the flags field below),
 * the value must be fetched on demand explicitly issued by code
 * that needs the data node value in order to continue processing
 * of the expression
 *
 *
 * some peculiarities are present here for efficiencys sake, they
 * are documented below
 *
 * \todo	properly order the fields below
 * \todo	some of these may be redundant - revise this
 *		data structure when the code is mature and
 *		has been used for a while
 * \todo	the usage of the data field below is plain evil;
 *		this must be optimized (small data must be put
 *		directly in the val union, and not thru the 'data'
 *		field)
 */
struct exp_data_node
{
	/*! next node pointer, used for building lists of expression nodes
	 *
	 * \note	this is curently not used anywhere
	 */
	struct exp_data_node	* next;
	/*! data type enumerator this node describes
	 *
	 * \note	of the types below, currently
	 *	used are only the long long types (signed and
	 *	unsigned); of them, actually only
	 *	the signed version is used
	 *
	 *	rationale	the types here were
	 *	declared to correspond to c basic data types
	 *	and to be used that way, but this is not
	 *	really useful (and then again there are
	 *	the integer promotion rules), so actually
	 *	only the largest signed integer type
	 *	really gets used; as signed and unsigned overflow
	 *	and underflow are silently ignored,
	 *	it would make sense to promote a signed integer
	 *	to an unsigned on, e.g., positive addition overflow,
	 *	and this would really make any difference
	 *	in subsequent right shifts and divisions;
	 *	i dont think it is worth the trouble to
	 *	handle signed and unsigned versions of
	 *	the integers, as if this really is needed
	 *	(meaning one is exposed to the risk of
	 *	overflowing a long long),
	 *	then something is probably wrong */
	enum
	{
		/*! invalid data type */
		DATA_TYPE_INVALID = 0,
		/* these below are the basic types
		 * mentioned by the c standard; they play a
	         * special role not only because they are
		 * very common, but because interpretation
		 * on the value field below is optimized for
		 * directly containing a value of a basic type
		 *
		 * \note	the enumerators order below matter, as
		 * they are sorted by integer promotion rank;
		 * also, as enumerated, the signed integer
		 * types occuppy even position in the enumeration
		 * space, and the corresponding unsigned versions
		 * occupy odd positions, one greater from the
		 * corresponding signed versions; thus, a quick
		 * comparison of types can be accomplished by
		 * right shifting and comparing the corresponding
		 * enumerator values, and signedness can be
		 * determined by inspecting the lowest order
		 * bit of the enumerator value */

		/*! signed char type */
		BASE_TYPE_SIGNED_CHAR = 2,
		/*! unsigned char type */
		BASE_TYPE_UCHAR = 3,
		/*! signed short type */
		BASE_TYPE_SHORT = 4,
		/*! unsigned short type */
		BASE_TYPE_USHORT = 5,
		/*! signed int type */
		BASE_TYPE_INT = 6,
		/*! unsigned int type */
		BASE_TYPE_UINT = 7,
		/*! signed long type */
		BASE_TYPE_LONG = 8,
		/*! unsigned long type */
		BASE_TYPE_ULONG = 9,
		/*! signed long long type */
		BASE_TYPE_LONG_LONG = 10,
		/*! unsigned long long type */
		BASE_TYPE_ULONG_LONG = 11,
		/*! an identifier that has not yet been mapped on a concrete instance variable
		 *
		 * the purpose of this is to stash an identifier when
		 * parsing an expression until the object referenced
		 * by the identifier name is absolutely needed to
		 * procced with an expression evaluation (e.g. an array
		 * subscripting, a pointer dereference, an aggregate data
		 * type member access, etc.); an expression data node
		 * of this type has nothing more here that is applicable
		 * except the idname field below; the idname really
		 * serves only the purpose of holding an identifier name
		 * until its resolution and is then discarded (invalidated);
		 * when the object specified by the identifier is needed
		 * for the expression evaluation, the gear symbol tables
		 * are scanned and (if the identifier is successfully
		 * resolved) the expression node is transformed
		 * into a NON_BASE_TYPE_OBJECT node (see below), and all
		 * remaining fields of this exp_data_node structure
		 * are initialized accordingly; if the identifier is
		 * not found in the gear symbol tables, parsing is
		 * usually aborted */
		UNRESOLVED_ID,
		/*! non-basic type, applicable and valid is the data_obj field below
		 *
		 * this identifies some object in the debugged program, that
		 * is, something that occupies physical target resources
		 * (e.g. memory and/or registers)
		 */
		NON_BASE_TYPE_OBJECT,
		/*! a subprogram object, applicable and valid is the subprogram_data field below */
		SUBPROGRAM_OBJECT,
	}
	type;
	/*! this union contains context-dependent data, the context is defined by the 'type' field above
	 *
	 * there are really only two cases to consider here */
	union
	{
		/*! a data structure containing fields applicable and valid for nodes of type NON_BASE_TYPE_OBJECT */
		struct
		{
			/*! various flags controlling interpetation of other fields here
			 *
			 * \note	the convention is, when the parser is
			 *		in syntax checking or type evaluation mode,
			 *		no flags in the expression data nodes
			 *		other than the is_addr_applicable flag,
			 *		to ever get set, which facilitates
			 *		the catching of errors; see also the
			 *		comments for the flags field in
			 *		struct parser_state
			 */
			struct
			{
				/*! denotes if a value applies for this data node
				 *
				 * for a non-void type, this should be true;
				 * if true, the val field below applies (is valid) */
				int	is_val_applicable	: 1;
				/*! denotes if the value for this node has been fetched
				 *
				 * for a data node with is_val_applicable set to true,
				 * denotes if the data value is already available in the
				 * val field below (true), or is not available and
				 * must be explicitly fetched (false) by anyone needing
				 * the value of this data node; this is here primary
				 * to avoid unnecessary communication for data fetching
				 * from the target */
				int	is_val_fetched		: 1;
				/*! address applicable flag
				 *
				 * denotes if this data node describes a data object
				 * for which the notion of address (physical location
				 * in the target) applies; address here means the
				 * loc field below is valid and usable (it cannot
				 * be invalid as with the is_val_fetched flag for
				 * the data value above)
				 *
				 * address here means a physical address and/or
				 * a set of registers in the target; see the
				 * ::loc field below for details */
				int	is_addr_applicable	: 1;
				/*! \todo	a quick hack - fix this */
				int	is_addr_a_reg_nr	: 1;
			}
			flags;
			/*! \todo	define this properly, this is a hack now - fix it
			 * \todo	this must account for register storage, should
			 *		really be a location list */
			union
			{
				ARM_CORE_WORD		addr;
				/*! \todo	a quick hack - fix this */
				ARM_CORE_WORD		reg_nr;
			};
			/*! the type of the object referenced by the expression node
			 *
			 * \todo	handle arrays; right now, they are not handled
			 *		and they are going to need some special case
			 *		handling because they are a special case
			 *		as mandated by the dwarf representation of arrays
			 *		which is currently used virtually verbatim
			 *		by the debug information tree building code;
			 *		the need for special case handling of arrays
			 *		stems from the fact that multidimensional
			 *		arrays are not represented as arrays of arrays
			 *		(of arrays, etc.) as thought of in c, and this
			 *		tends to call for some special case handling
			 *		at times */
			struct dtype_data	* type;
		}
		data_obj;
		/*! a field applicable and valid for nodes of type SUBPROGRAM_OBJECT */
		struct subprogram_data	* subprogram_data;
	};
	/*! data value storage
	 *
	 * \todo	fix comments, right here they are outrageously wrong
	 * in case of a basic type, the union supplies
	 * the data value directly; otherwise (type
	 * is NON_BASE_TYPE_OBJECT), the val pointer points
	 * to the data type of the object described by the node */
	union
	{
		/*! signed char value */
		signed char	scval;
		/*! unsigned char value */
		unsigned char	ucval;
		/*! signed int value */
		int		sival;
		/*! unsigned int value */
		unsigned int	uival;
		/*! signed long value */
		long		slval;
		/*! unsigned long value */
		unsigned long	ulval;
		/*! signed long long value */
		long long	sllval;
		/*! unsigned long long value */
		unsigned long long ullval;
		/*! stores the name of a non yet resolved identifier
		 *
		 * this is only applicable in the case of expression nodes
		 * describing an unresolved identifier; see
		 * the comments above for UNRESOLVED_ID */
		char		* idname;
		/*! \todo	define this */
		void		* data;
	}
	val;
};


/*
 *
 * local function prototypes follow
 *
 */

/* parser needed error reporting function */
static void c_lang_error(struct parser_state * state, yyscan_t yyscanner, char * msg);
/* forward declarations - needed by the parser function */
static struct exp_data_node * c_exp_add(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_sub(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_unary_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * exp_node);
static struct exp_data_node * c_exp_bitwise_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_logical_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_mul(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_div(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_mod(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_shift(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2, bool is_left_shift);
static struct exp_data_node * c_exp_arr_subscript(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2);
static struct exp_data_node * c_exp_member_select(struct parser_state * state, struct exp_data_node * op1, char * member_name, bool is_ptr_member_select);
static void destroy_exp_node(struct parser_state * state, struct exp_data_node * exp_node);
static struct exp_data_node * get_exp_data_node(struct parser_state * state);

%}


%%

primary_expression
	: IDENTIFIER	{
		if (!state->flags.is_syntax_checked)
		{
			struct exp_data_node * p;
			p = get_exp_data_node(state);
			p->type = UNRESOLVED_ID;
			p->val.idname = $1;
			$$ = p;
		}
		}
	| CONSTANT	{
		if (!state->flags.is_syntax_checked)
		{
			struct exp_data_node * p;
			p = get_exp_data_node(state);
			if (state->flags.is_eval_value)
			{
				p->data_obj.flags.is_val_applicable = 1;
				p->data_obj.flags.is_val_fetched = 1;
				p->val.sllval = $1;
			}
			p->type = BASE_TYPE_LONG_LONG;
			$$ = p;
		}
		}
	| STRING_LITERAL	{
			panic("");
		}
	| '(' expression ')'	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $2;
		}
		}
	;

postfix_expression
	: primary_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| postfix_expression '[' expression ']'	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_arr_subscript(state, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
/*
	| postfix_expression '(' ')'
*/
	| postfix_expression '.' IDENTIFIER	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_member_select(state, $1, $3, false)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	| postfix_expression PTR_OP IDENTIFIER	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_member_select(state, $1, $3, true)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
/*
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
*/
	;


unary_expression
	: postfix_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
/*
	| INC_OP unary_expression
	| DEC_OP unary_expression
*/
	| unary_operator cast_expression	{

		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_unary_op(state, $1, $2)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}

		}
	| SIZEOF unary_expression	{
			panic("");
		}
	| SIZEOF '(' type_name ')'	{
			panic("");
		}
	;

unary_operator
	: '&'	{ if (!state->flags.is_syntax_checked) { $$ = UNARY_GETADDR; } }
	| '*'	{ if (!state->flags.is_syntax_checked) { $$ = UNARY_DEREF; } }
	| '+'	{ if (!state->flags.is_syntax_checked) { $$ = UNARY_PLUS; } }
	| '-'	{ if (!state->flags.is_syntax_checked) { $$ = UNARY_MINUS; } }
	| '~'	{ if (!state->flags.is_syntax_checked) { $$ = UNARY_BITWISE_NOT; } }
	| '!'	{ if (!state->flags.is_syntax_checked) { $$ = UNARY_LOGICAL_NOT; } }
	;

cast_expression
	: unary_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| '(' type_name ')' cast_expression	{
			panic("");
		}
	;

multiplicative_expression
	: cast_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| multiplicative_expression '*' cast_expression	{

		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_mul(state, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}

		}
	| multiplicative_expression '/' cast_expression{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_div(state, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	| multiplicative_expression '%' cast_expression{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_mod(state, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

additive_expression
	: multiplicative_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| additive_expression '+' multiplicative_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_add(state, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}

		}
	| additive_expression '-' multiplicative_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_sub(state, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

shift_expression
	: additive_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| shift_expression LEFT_SHIFT_OP additive_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_shift(state, $1, $3, true)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	| shift_expression RIGHT_SHIFT_OP additive_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_shift(state, $1, $3, false)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

relational_expression
	: shift_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| relational_expression '<' shift_expression	{
			panic("");
		}
	| relational_expression '>' shift_expression	{
			panic("");
		}
	| relational_expression LE_OP shift_expression	{
			panic("");
		}
	| relational_expression GE_OP shift_expression	{
			panic("");
		}
	;

equality_expression
	: relational_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| equality_expression EQ_OP relational_expression	{
			panic("");
		}
	| equality_expression NE_OP relational_expression	{
			panic("");
		}
	;

and_expression
	: equality_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| and_expression '&' equality_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_bitwise_op(state, BITWISE_AND, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

exclusive_or_expression
	: and_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| exclusive_or_expression '^' and_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_bitwise_op(state, BITWISE_XOR, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

inclusive_or_expression
	: exclusive_or_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| inclusive_or_expression '|' exclusive_or_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_bitwise_op(state, BITWISE_OR, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

logical_and_expression
	: inclusive_or_expression	{
		if (!state->flags.is_syntax_checked)
		{
			$$ = $1;
		}
		}
	| logical_and_expression AND_OP inclusive_or_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_logical_op(state, LOGICAL_AND, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;


expression
	: logical_and_expression	{
		if (!state->flags.is_syntax_checked)
		{
			state->parse_head = $$ = $1;
		}
		}
	| expression OR_OP logical_and_expression	{
		if (!state->flags.is_syntax_checked)
		{
			if (!($$ = c_exp_logical_op(state, LOGICAL_OR, $1, $3)))
			{
				/* error occurred - abort the parser */
				YYABORT;
			}
		}
		}
	;

constant_expression
	: expression
	;

/*
declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers
	| type_specifier
	| type_specifier declaration_specifiers
	| type_qualifier
	| type_qualifier declaration_specifiers
	;
*/
/*
storage_class_specifier
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;
*/

type_specifier
	: VOID	{
			panic("");
		}
	| CHAR	{
			panic("");
		}
	| SHORT	{
			panic("");
		}
	| INT	{
			panic("");
		}
	| LONG	{
			panic("");
		}
	| FLOAT	{
			panic("");
		}
	| DOUBLE	{
			panic("");
		}
	| SIGNED	{
			panic("");
		}
	| UNSIGNED	{
			panic("");
		}
	| struct_or_union_specifier	{
			panic("");
		}
	| enum_specifier	{
			panic("");
		}
	| TYPE_NAME	{
			panic("");
		}
	;

struct_or_union_specifier
/*
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	*/
	/*|*/: struct_or_union IDENTIFIER	{
			panic("");
		}
	;

struct_or_union
	: STRUCT
	| UNION
	;
/*
struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;
*/

/*
struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;
*/

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
/*
	| type_qualifier specifier_qualifier_list
	| type_qualifier
*/
	;
/*
struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;
*/
/*
struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;
*/

enum_specifier
/*
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
*/
	/*|*/: ENUM IDENTIFIER
	{
		panic("");
	}
	;
/*
enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;
*/
/*
enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;
*/

/*
type_qualifier
	: CONST
	| VOLATILE
	;
*/

/*
declarator
	: pointer direct_declarator
	| direct_declarator
	;
*/
/*
direct_declarator
	: IDENTIFIER
	| '(' declarator ')'
	| direct_declarator '[' constant_expression ']'
	| direct_declarator '[' ']'
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
	;
*/

pointer
	: '*'
/*
	| '*' type_qualifier_list
*/
	| '*' pointer
/*
	| '*' type_qualifier_list pointer
*/
	;
/*
type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;
*/

/*
parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
	;
*/

/*
parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;
*/
/*
parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;
*/
/*
identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;
*/
type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expression ']'
/*
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
*/
	;

%%

/*
 *
 * user code section of the bison input file follows
 *
 */


/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static inline void clear_type_flags(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	a simple function to clear some flags of an expression data node
 *
 *	this function clears all flags in an expression data node other
 *	than the is_addr_applicable flag; this is mainly needed for
 *	catching errors when parsing an expression in order to obtain
 *	its resulting type
 *
 *	\note	this is an inline function instead of a macro, because
 *		macros are evil, and there wont be a difference with
 *		a decent compiler (such as the gcc) anyway
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	expression data node for which to clear the
 *				flags
 *	\return	none
 */
static inline void clear_type_flags(struct parser_state * state, struct exp_data_node * exp_node)
{
	exp_node->data_obj.flags.is_val_applicable = exp_node->data_obj.flags.is_val_fetched = 0;
}

/*!
 *	\fn	static inline void set_parse_abort_vars(struct parser_state * state, enum GEAR_ENGINE_ERR_ENUM errcode, char * errmsg)
 *	\brief	a helper function to set relevant parser state variables when aborting a parse due to an error
 *
 *	\param	state	parser state whose abort variables must be set
 *	\param	errcode	error code to set as a parse abort reason
 *	\param	errmsg	error message string to set as a parse abort message; can be null
 *	\return	none */
static inline void set_parse_abort_vars(struct parser_state * state, enum GEAR_ENGINE_ERR_ENUM errcode, char * errmsg)
{
	state->is_parser_aborted = true;
	state->errcode = errcode;
	if (errmsg)
		state->parse_msg = strdup(errmsg);
	else
		state->parse_msg = 0;
}

/*!
 *	\fn	static enum GEAR_ENGINE_ERR_ENUM resolve_id(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	resolves the location of an identifier used in an expression
 *
 *	this function resolves the type and location of an identifier used
 *	in an expression (properly initializes the type and location fields
 *	of the expression data node passed as a parameter)
 *	by scanning the debug information symbol tables taking scope
 *	in an account
 *
 *	\note	in case of an error, this routine does *not* invoke
 *		destroy_exp_node() on the exp_node expression node
 *		passed - this is a responsibility of the caller
 *
 *	\todo	this function is currently largely inclomplete, only
 *		data objects are handled right now
 *	\todo	this routine needs a major cleanup and maybe should
 *		be rewritten
 *	\todo	handle expression evaluation flags so that it is possible
 *		to work without a live target present
 *
 *	\note	this, and the fetch_data(), routines are the only routines
 *		in this module that access
 *		target resources, and it is best to keep things this way
 *		because it would be much easier to handle target-access
 *		related errors at just these places - rather than spread
 *		target-access related error handling like plague
 *		in the module
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	a pointer to an expression data node of the
 *			identifier to be resolved, must be of type
 *			UNRESOLVED_ID and the idname field must be
 *			properly initialized; cant be null
 *	\return	GEAR_ERR_NO_ERROR on success, GEAR_ERR_EXPR_SEMANTIC_ERROR (and the
 *		parser abortion variables will have been properly set) in case of an error,
 *		GEAR_ERR_TARGET_ACCESS_ERROR if resolving the identifier requires
 *		target resources that are inaccessible
 */
static enum GEAR_ENGINE_ERR_ENUM resolve_id(struct parser_state * state, struct exp_data_node * exp_node)
{
ARM_CORE_WORD addr;
bool is_result_in_reg;
enum DWARF_EXPR_INFO_ENUM eval_info;
struct dwarf_head_struct * dwhead;
struct dobj_data * dobj;
struct cu_data * cu;
struct subprogram_data * subp;
/* program counter and frame base registers are needed for evaluation of various data */
ARM_CORE_WORD fbreg, cubase;
ARM_CORE_WORD pc;

	/* sanity checks */
	if (exp_node->type == DATA_TYPE_INVALID)
		panic("");
	if (exp_node->type != UNRESOLVED_ID)
		return GEAR_ERR_NO_ERROR;
	if (!exp_node->val.idname)
		panic("");

	/*! \todo	set flags properly here */
	dwhead = deprecated_scope_locate_dobj(state->ctx, exp_node->val.idname,
			(struct scope_resolution_flags) { 0 } );
	if (!dwhead)
	{
		/*! \todo	print the identifier here as well */
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "unresolved identifier");
		return GEAR_ERR_EXPR_SEMANTIC_ERROR;
	}
	/* determine and properly handle the type of the symbol just found */
	switch (dwarf_util_get_tag_category(dwhead->tag))
	{
		case DWARF_TAG_CATEGORY_SUBPROGRAM:
			exp_node->type = SUBPROGRAM_OBJECT;
			exp_node->subprogram_data = (struct subprogram_data *) dwhead;
			break;
		case DWARF_TAG_CATEGORY_DATA_OBJECT:
		{
			dobj = (struct dobj_data *) dwhead;
			if (dobj->is_node_a_declaration || dobj->type->is_node_a_declaration)
			{
				set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "attempted to resolve a type declaration");
				return GEAR_ERR_EXPR_SEMANTIC_ERROR;
			}

			free(exp_node->val.idname);
			/* zero the val union - some error catching code relies on this
			 * (namely, checking the val.data field in c_exp_ptr_fixup()) */
			memset(&exp_node->val, 0, sizeof exp_node->val);
			exp_node->type = NON_BASE_TYPE_OBJECT;

			/* locate the compilation unit containing this data object - the
			 * compilation unit starting address in memory is needed for
			 * computing the data object address below */
			/*! \todo	this is incorrect - expression evaluation flags
			 *		are not handled at all here, target resources
			 *		are accessed at all times - this is extremely vile */
			/*! \todo	this here is *wrong* - used is the compilation
			 *		unit that the target is currently stopped into,
			 *		instead of (correctly) using the compilation
			 *		unit containing the data object */
			do
			{
				dwhead = dwhead->parent;
				if (!dwhead)
					panic("");
			}
			while (dwhead->tag != DW_TAG_compile_unit);
			cubase = ((struct cu_data *) dwhead)->default_cu_base_address;

			fbreg = 0;

			/* retrieve frame base address */
			if (state->ctx->cc->core_reg_read(state->ctx,
						0,
						1 << state->ctx->tdesc->get_target_pc_reg_nr(state->ctx),
						&pc) != GEAR_ERR_NO_ERROR)
			{
				/* target access error */
				state->is_target_access_failed = true;

				/*! \todo	this mysterious -2 is to
				 *		force the program counter to
				 *		some value that is presumably
				 *		outside any valid program address;
				 *		-1 could be problematic because
				 *		it is a special case at some
				 *		places in the dwarf standard...
				 *		0 is almost surely a valid
				 *		program address for embedded targets */
				pc = -2;
			}
			else
			{
				cu = aranges_get_cu_for_addr(state->ctx, pc);
				subp = aranges_get_subp_for_addr(state->ctx, pc);
				/* compute frame base register value */
				if (cu)
					cubase = cu->default_cu_base_address;

				if (subp)
				{
					if (!subp->is_frame_base_available)
						panic("");
					if (dwarf_loc_eval_loc_from_list(state->ctx,
								&fbreg,
								&is_result_in_reg,
								&eval_info,
								&subp->fb_location,
								pc,
								cubase,
								0) != GEAR_ERR_NO_ERROR)
						panic("");
					if (eval_info == DW_EXPR_INFO_INVALID || eval_info >= DW_EXPR_NEEDS_FBREG)
						panic("");
				}
			}

			exp_node->data_obj.type = dobj->type;

			/* retrieve object address */
			if (!dobj->is_location_valid)
			{
				void * buf;
				int size;
				/* no location information is present about
				 * this data object - however, it may still
				 * be possible to access it if it has been
				 * reduced to a constant value */
				if (!dobj->is_constant_value)
					panic("");
				/*! \todo	fix the size checks when
				 *		dwarf block forms are supported */
				size = dtype_access_sizeof((struct dwarf_head_struct *) exp_node->data_obj.type);
				if (size > sizeof dobj->const_val)
					panic("");
				exp_node->data_obj.flags.is_val_applicable = 1;
				exp_node->data_obj.flags.is_addr_applicable = 0;
				if (!(buf = calloc(1, size)))
					panic("");
				exp_node->val.data = buf;
				exp_node->data_obj.flags.is_val_fetched = 1;
				memcpy(buf, &dobj->const_val, size);
				break;
			}

			/* location information is present - process it */
			if (dwarf_loc_eval_loc_from_list(state->ctx,
				&addr,
				&is_result_in_reg,
				&eval_info,
				&dobj->location,
				pc,
				cubase,
				fbreg) != GEAR_ERR_NO_ERROR)
					panic("error retrieving object address");

			/* validate address computed */
			if (eval_info == DW_EXPR_INFO_INVALID || eval_info > DW_EXPR_NEEDS_FBREG)
				panic("");
			if (state->is_target_access_failed)
			{
					if (eval_info >= DW_EXPR_NEEDS_TARGET_MEM_ACCESS)
					{
						set_parse_abort_vars(state, GEAR_ERR_TARGET_ACCESS_ERROR, "target access error");
						return GEAR_ERR_TARGET_ACCESS_ERROR;
					}
					/* target access not needed after all - reset error flag */
					state->is_target_access_failed = 0;
			}

			exp_node->data_obj.flags.is_val_applicable = exp_node->data_obj.flags.is_addr_applicable = 1;
			/*! \todo	a quick hack - fix this */
			if (is_result_in_reg)
			{
				int nr_target_core_regs;
				exp_node->data_obj.flags.is_addr_a_reg_nr = 1;
				if (!(nr_target_core_regs
						= state->ctx->tdesc->get_nr_target_core_regs(state->ctx)))
					panic("");
				if (addr >= nr_target_core_regs)
					panic("");
				exp_node->data_obj.reg_nr = addr;
			}
			else
			{
				if (eval_info != DW_EXPR_LOCATION_IS_CONSTANT_BUT_NOT_ADDR)
				{
					exp_node->data_obj.addr = addr;
				}
				else
				{
				int size;
				void * buf;
					/* the data object processed does not have address,
					 * but its value is nonetheless known - and actually
					 * it has been returned in the 'addr' parameter
					 * in the dwarf_loc_eval_loc_from_list() call above */
					exp_node->data_obj.flags.is_addr_applicable = 0;

					size = dtype_access_sizeof((struct dwarf_head_struct *) exp_node->data_obj.type);
					if (size > sizeof(ARM_CORE_WORD))
						panic("");
					if (!(buf = calloc(1, size)))
						panic("");
					exp_node->val.data = buf;
					exp_node->data_obj.flags.is_val_fetched = 1;
					memcpy(buf, &addr, size);
				}
			}
			/* all seems ok */
		}
			break;
		default:
			set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "unsupported symbol type");
			return GEAR_ERR_EXPR_SEMANTIC_ERROR;
			break;
	}

	return GEAR_ERR_NO_ERROR;
}

/*!
 *	\fn	static void fetch_data(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	retrieves the data bytes for a given expression node
 *
 *	\note	in order for this to work, the passed expression data node
 *		must describe a non base type object, which must be 'real'
 *		in the sense that it occupies physical target resources
 *		(e.g. memory and/or register(s)) - as opposed to expression
 *		data nodes that contain only type information and therefore
 *		the notions of 'location' and 'value' do not make sense
 *
 *	\note	this, and the resolve_id(), routines are the only routines
 *		in this module that access
 *		target resources, and it is best to keep things this way
 *		because it would be much easier to handle target-access
 *		related errors at just these places - rather than spread
 *		target-access related error handling like plague
 *		in the module
 *
 *	\todo	define error codes and code the error handling
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	a pointer to an expression data node for the data
 *			object of interest
 *	\return	currently none, but error codes must be defined
 */
static void fetch_data(struct parser_state * state, struct exp_data_node * exp_node)
{
int size;
int nbytes;
void * buf;

	/* sanity checks */
	if (exp_node->type == DATA_TYPE_INVALID
			|| exp_node->type == SUBPROGRAM_OBJECT)
		return;
	/* in case the expression is parsed for something other than
	 * obtaining its value, this routine should never get called;
         * make sure this is the case */	 
	if (!state->flags.is_eval_value)
		return;

	/* see if data has already been fetched */
	if (exp_node->data_obj.flags.is_val_applicable &&
		       exp_node->data_obj.flags.is_val_fetched)
		/* yes - do nothing */
		return;

	/* validate expression node */
	if (exp_node->type != NON_BASE_TYPE_OBJECT || !exp_node->data_obj.flags.is_val_applicable
			|| !exp_node->data_obj.flags.is_addr_applicable
			|| exp_node->data_obj.flags.is_val_fetched
			|| exp_node->data_obj.type->is_node_a_declaration)
	{
		gprintf("exp_node->type != NON_BASE_TYPE_OBJECT ? %s\n", (exp_node->type != NON_BASE_TYPE_OBJECT) ? "yes" : "no");
		gprintf("exp_node->data_obj.flags.is_val_applicable ? %s\n", (exp_node->data_obj.flags.is_val_applicable) ? "yes" : "no");
		gprintf("exp_node->data_obj.flags.is_addr_applicable ? %s\n", (exp_node->data_obj.flags.is_addr_applicable) ? "yes" : "no");
		gprintf("exp_node->data_obj.flags.is_val_fetched ? %s\n", (exp_node->data_obj.flags.is_val_fetched) ? "yes" : "no");
		gprintf("exp_node->data_obj.type->is_node_a_declaration ? %s\n", (exp_node->data_obj.type->is_node_a_declaration) ? "yes" : "no");

		if (exp_node->data_obj.type->is_node_a_declaration
				&& exp_node->data_obj.type->dtype_class == DTYPE_CLASS_CLASS)
		{
			/* attempt to 'fix-up' this data object by looking for
			 * an appropriate class definition... */
		}
		return;
	}
	size = dtype_access_sizeof((struct dwarf_head_struct *) exp_node->data_obj.type);
	if (!(buf = calloc(1, size)))
		panic("");
	/*! \note	this is currently the only place where
	 *		target resources are accessed */ 
	/*! \todo	a quick hack - fix this */
	if (exp_node->data_obj.flags.is_addr_a_reg_nr)
	{
		ARM_CORE_WORD reg;
		int nr_target_core_regs, target_reg_nr;
		if (!(nr_target_core_regs
				= state->ctx->tdesc->get_nr_target_core_regs(state->ctx)))
			panic("");
		target_reg_nr = exp_node->data_obj.reg_nr;
		if (state->ctx->tdesc->translate_dwarf_reg_nr_to_target_reg_nr(state->ctx, &target_reg_nr)
				!= GEAR_ERR_NO_ERROR)
			panic("");
		if (target_reg_nr >= nr_target_core_regs)
			panic("");
		if (state->ctx->cc->core_reg_read(state->ctx, 0, 1 << target_reg_nr, &reg) != GEAR_ERR_NO_ERROR)
		{
			/* target access error */
			state->is_target_access_failed = true;
			return;
		}
		if (size > sizeof(ARM_CORE_WORD))
			panic("");
		/*! \todo	this is braindamaged... */
		memcpy(buf, &reg, size);
	}
	else
	{
		nbytes = size;
		if (state->ctx->cc->core_mem_read(state->ctx, buf, exp_node->data_obj.addr, &nbytes) != GEAR_ERR_NO_ERROR
				|| nbytes != size)
		{
			/* target access error */
			state->is_target_access_failed = true;
			return;
		}
	}
	exp_node->val.data = buf;
	exp_node->data_obj.flags.is_val_fetched = 1;
}

/*!
 *	\fn	static bool is_val_ctype_int(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	determines if an expression node describes an integer base type object
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	pointer to the expression node for the data object
 *			of interest
 *	\return	true, if the data object described by exp_node is of an
 *		integer base type, false otherwise
 */
static bool is_val_ctype_int(struct parser_state * state, struct exp_data_node * exp_node)
{
	/* sanity checks */
	if (exp_node->type == DATA_TYPE_INVALID
			|| exp_node->type == UNRESOLVED_ID
			|| exp_node->type == SUBPROGRAM_OBJECT)
		return false;
	if (exp_node->type < UNRESOLVED_ID)
		return true;
	return dtype_access_is_ctype(CTYPE_INTEGER, exp_node->data_obj.type);
}

/*!
 *	\fn	static signed long long get_ctype_int(struct parser_state * state, struct exp_data_node * exp_node, bool * is_signed_int)
 *	\brief	given an expression node for an integer type object, retrieves the data object value
 *
 *	\todo	design error reporting here
 *
 *	\note	the value of the target integer object, if not one of the
 *		parser built in types, must have been fetched prior
 *		to invoking this routine - this routine will not attempt to
 *		fetch the target integer by accessing target resources
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	pointer to the expression node for the data object
 *			of interest
 *	\param	is_signed_int	a pointer to where to store information about
 *			whether the accessed integral type has sign or not
 *			(if set to true, the integral type is a signed one,
 *			otherwise it is unsigned); this is useful at a couple
 *			of places where the sign of an integral type could make a
 *			difference (e.g., when performing a right shift, this
 *			is used to determine if the shift should be an arithmetic or
 *			a logic one); if the accessed integral type is unsigned,
 *			it is appropriate to cast the result returned from
 *			this function to an unsigned type; this parameter can
 *			be null, in which case this information is not stored
 *			
 *	\return	the value of the integer type data object described by exp_node
 */
static signed long long get_ctype_int(struct parser_state * state, struct exp_data_node * exp_node, bool * is_signed_int)
{
void * val;
signed long long res;

	if (!is_val_ctype_int(state, exp_node))
		panic("");
	/* it is expected that the integer value has already been fetched */
	if (!exp_node->data_obj.flags.is_val_applicable || !exp_node->data_obj.flags.is_val_fetched)
		panic("");
	if (exp_node->type != NON_BASE_TYPE_OBJECT)
	{
		/* read the comments about the type member in the 
		 * exp_data_node structure for details on the sign encoding */
		if (is_signed_int)
			*is_signed_int = (exp_node->type & 1) ? false : true;
		/* assume it is one of the internal types */
		/*! \todo	currently only the sllval is used,
		 *		this is not actually a problem, but
		 *		may need revising at some time */
		res = exp_node->val.sllval;
	}
	else
	{
		val = exp_node->val.data;
		switch (exp_node->data_obj.type->base_type_encoding)
		{
			case DW_ATE_unsigned:
			case DW_ATE_unsigned_char:
				if (is_signed_int)
					*is_signed_int = false;
				switch (exp_node->data_obj.type->data_size)
				{
					case 1:
						res = *(unsigned char *)val;
						break;
					case 2:
						res = *(unsigned short *)val;
						break;
					case 4:
						res = *(unsigned int *)val;
						break;
					case 8:
						res = *(unsigned long long *)val;
						break;
					default:
						panic("");
				}
				break;
			case DW_ATE_signed:
			case DW_ATE_signed_char:
				if (is_signed_int)
					*is_signed_int = true;
				switch (exp_node->data_obj.type->data_size)
				{
					case 1:
						res = *(signed char *)val;
						break;
					case 2:
						res = *(signed short *)val;
						break;
					case 4:
						res = *(signed int *)val;
						break;
					case 8:
						res = *(signed long long *)val;
						break;
					default:
						panic("");
				}
				break;
			default:
				panic("");
		}
	}
	return res;
}

/*!
 *	\fn	static int is_val_ctype_ptr(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	determines if an expression node describes a pointer type object
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	pointer to the expression node for the data object
 *			of interest
 *	\return	nonzero, if the data object described by exp_node is of a
 *		pointer type, zero otherwise
 */
static int is_val_ctype_ptr(struct parser_state * state, struct exp_data_node * exp_node)
{
	/* sanity checks */
	if (exp_node->type == DATA_TYPE_INVALID
			|| exp_node->type == UNRESOLVED_ID
			|| exp_node->type == SUBPROGRAM_OBJECT)
		return 0;
	if (exp_node->type < UNRESOLVED_ID)
		return 0;
	return dtype_access_is_ctype(CTYPE_POINTER, exp_node->data_obj.type);
}

/*!
 *	\fn	static int is_val_ctype_scalar(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	determines if an expression node describes a scalar type object
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	pointer to the expression node for the data object
 *			of interest
 *	\return	nonzero, if the data object described by exp_node is of a
 *		scalar type, zero otherwise
 */
static int is_val_ctype_scalar(struct parser_state * state, struct exp_data_node * exp_node)
{
	/* sanity checks */
	if (exp_node->type == DATA_TYPE_INVALID
			|| exp_node->type == UNRESOLVED_ID
			|| exp_node->type == SUBPROGRAM_OBJECT)
		return 0;
	if (exp_node->type < UNRESOLVED_ID)
		return 1;
	return dtype_access_is_ctype(CTYPE_SCALAR, exp_node->data_obj.type);
}

/*!
 *	\fn	static int is_val_ctype_struct_or_union(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	determines if an expression node describes an object of a struct or union type
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	pointer to the expression node for the data object
 *			of interest
 *	\return	nonzero, if the data object described by exp_node is of a
 *		struct or union type, zero otherwise
 */
static int is_val_ctype_struct_or_union(struct parser_state * state, struct exp_data_node * exp_node)
{
	/* sanity checks */
	if (exp_node->type == DATA_TYPE_INVALID
			|| exp_node->type == UNRESOLVED_ID
			|| exp_node->type == SUBPROGRAM_OBJECT)
		return 0;
	if (exp_node->type < UNRESOLVED_ID)
		return false;
	return dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, dtype_access_get_unqualified_base_type(exp_node->data_obj.type));
}


/*!
 *	\fn	static struct exp_data_node * get_exp_data_node(struct parser_state * state)
 *	\brief	allocates and returns a new exp_data_node structure
 *
 *	this is a helper routine used by the expression parsing and evaluation code
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\return	a pointer to a newly allocated exp_data_node data structure
 */
static struct exp_data_node * get_exp_data_node(struct parser_state * state)
{
struct exp_data_node * p;

	p = calloc(1, sizeof(*p));
	if (!p)
		panic("out of core");
	return p;

}


/*!
 *	\fn	static void destroy_exp_node(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	deallocates expression data nodes which are no longer needed
 *
 *	see also the comments about memory management in the parser
 *	at the start of this file; this function is invoked on two
 *	occasions - by the bison generated code - when discarding
 *	symbols from the parser stack (the %destructor bison construct),
 *	and by the calculation functions - when they no more need
 *	an operand
 *
 *	\todo	handle deallocation of the artifically grown type tree
 *		nodes - the two special cases in exp_unary_op() and
 *		c_exp_ptr_fixup()
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	a pointer to the expression data node to
 *			deallocate
 *	\return	none
 */
static void destroy_exp_node(struct parser_state * state, struct exp_data_node * exp_node)
{
	/* determine what must be deallocated */
	switch (exp_node->type)
	{
		case UNRESOLVED_ID:
			/* deallocate the name string only */
			free(exp_node->val.idname);
			break;
		case SUBPROGRAM_OBJECT:
			/* nothing to do */
			break;
		case NON_BASE_TYPE_OBJECT:
			/* examine flags */
			if (exp_node->data_obj.flags.is_val_applicable &&
					exp_node->data_obj.flags.is_val_fetched)
				free(exp_node->val.data);
			/* deallocate memory for type data in case it
			 * has been allocated at one of the two special
			 * cases (in c_exp_ptr_fixup() and c_exp_unary_op();
			 * how to make sure this is indeed the case, i.e.
			 * type data memory has been allocated in this module
			 * and has not been retrieved from the type-access
			 * module? simple - inspect the tag field in the
			 * dwarf_head_struct at the start of the data
			 * type entry, and if it is zero (which is not
			 * used in the dwarf standard as a valid tag
			 * value), then assume this entry was allocated
			 * in this module, and so it must be freed */
			if (!exp_node->data_obj.type->head.tag)
				free(exp_node->data_obj.type);
			break;
		default:
			/* a base type - deallocate nothing */
			;
	}
	free(exp_node);
}

/*!
 *	\fn	static void c_exp_ptr_fixup(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	downgrades a pointer-type expression data node
 *
 *	this function, given an expression data node with a pointer type,
 *	removes the node's lvalue qualification, if any; this basically
 *	means the expression data node has its is_addr_applicable field
 *	zeroed, so that the pointer now has only value, but no storage;
 *	the need for this arises from usage of pointers in address
 *	arithmetic expressions, where they can be no more assigned to;
 *	there are two cases to consider regarding the value of this
 *	new pointer type - these two cases apply only when an
 *	expression is evaluated for its value:
 *		- if the pointer to downgrade is a "genuine" one (i.e.,
 *		not an array), its value - if not fetched - is fetched
 *		from target core
 *		- if the pointer to downgrade is an array, the new pointer
 *		value equals the address of the array, which must
 *		already be available following identifier resolution,
 *		(this is checked here), and so target core is never
 *		accessed in this case
 *	in any case, the type of the pointer after the downgrading is
 *	assigned to the type field of the exp_node parameter
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	exp_node	a pointer to an expression data node to
 *			be downgraded; the node must be of a pointer
 *			type (a "genuine" pointer, or an array)
 *	\return	none
 *	\todo	define and properly handle errors here
 */
static void c_exp_ptr_fixup(struct parser_state * state, struct exp_data_node * exp_node)
{
struct dtype_data * t;
struct dtype_data * ptype;

	/* sanity checks */
	if (!is_val_ctype_ptr(state, exp_node))
		panic("");
	/* first of all, see if the pointer has already been "fixed up"
	 * (this can be the case in, e.g., an expression like:
         * "ptr + 1 + 2", where "ptr" is some pointer - 
	 * in this case the expression is evaluated as:
	 * "(ptr + 1) + 2", and the resulting pointer from
	 * evaluating the subexpression "(ptr + 1)" is already
	 * a "fixed up" pointer) */
	if (!exp_node->data_obj.flags.is_addr_applicable)
	{
		/* make sure the pointer is a "fixed up" one
		 * by checking its head tag */
		if (exp_node->data_obj.type->head.tag)
			panic("");
		if (state->flags.is_eval_value)
		{
			if (!exp_node->data_obj.flags.is_val_applicable ||
					!exp_node->data_obj.flags.is_val_fetched)
				panic("");
		}
		/* nothing more to do */
		return;
	}

	t = exp_node->data_obj.type;

	if (t->dtype_class == DTYPE_CLASS_PTR)
	{
		if (state->flags.is_eval_value)
		{
			/* fetch the pointer value */
			fetch_data(state, exp_node);
		}
	}
	else
	{
		/* assume this is an array or an array subrange dwarf type node */
		if (state->flags.is_eval_value)
		{
			if (exp_node->data_obj.flags.is_val_fetched)
				/* normally, the data field of the expression data node
				 * should simply be deallocated here; this used to
				 * be the case, however this should not be possible
				 * anymore - hence the panic here; if this gets ever
				 * triggerred, investigate the causes, as it should
				 * not be possible the data above to have been fetched */
				panic("");
			exp_node->data_obj.flags.is_val_fetched = exp_node->data_obj.flags.is_val_applicable = 1;
			/* data for the expression node should not have been fetched,
			 * just check this here */
			if (exp_node->val.data)
				panic("");
			if (!(exp_node->val.data = malloc(sizeof(ARM_CORE_WORD))))
				panic("");
			/*! \todo	a quick hack - fix this */
			if (exp_node->data_obj.flags.is_addr_a_reg_nr)
				panic("");
			*(ARM_CORE_WORD *) exp_node->val.data = exp_node->data_obj.addr;
		}
	}
	/* this expression node is no more an lvalue */
	/*! \note	do not move this code, because
	 *		the call above to fetch_data()
	 *		relies on this flag being set */ 
	exp_node->data_obj.flags.is_addr_applicable = 0;

	/* special case
	 * this is a special case; this is
	 * the one of two cases handled in this module
	 * in which the type tree built from
	 * the dwarf debug information is allowed
	 * to artificially grow by adding type
	 * nodes describing pointers (the other case
	 * is handled in exp_unary_op(), when processing
	 * the unary '&' operator); these artificial
	 * nodes are built here, by the expression parsing
	 * code and are referenced only by the expression
	 * evaluation code; they are also deallocated
	 * here; no one outside here is able to see these
	 * artificial nodes, as they are referred to only
	 * within this module (pointers to them are visible
	 * nowhere outside of this module) */
	if (!(ptype = calloc(1, sizeof * ptype)))
		panic("");
	ptype->dtype_class = DTYPE_CLASS_PTR;
	ptype->ptr_type = dtype_access_get_deref_type(exp_node->data_obj.type);

	exp_node->data_obj.type = ptype;
}


/*!
 *	\fn	static struct exp_data_node * c_exp_sub(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	performs a subtraction operation
 *
 *	this function subtracts op2 from op1; after return from
 *	this function, memory pointed to
 *	by op1 and op2 is invalid and op1 and op2 must not be used any more;
 *	this can be used for address arithmetic calculations; in the
 *	case where two pointers are subtracted, the compatibility
 *	of the pointer types is verified
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	first operand for the subtraction operator;
 *			can be of a numeric or a non-void pointer type
 *	\param	op2	second operand for the subtraction operator;
 *			can be of a numeric or a non-void pointer type
 *	\return	the result of the subtraction; can be of numeric or
 *		pointer type (in the case of address arithmetic
 *		calculations)
 *	\todo	define and properly handle errors here
 */
static struct exp_data_node * c_exp_sub(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
{
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}

	if (is_val_ctype_int(state, op1))
	{
		if (is_val_ctype_int(state, op2))
		{
			res = get_exp_data_node(state);
			res->type = BASE_TYPE_ULONG_LONG;
			if (state->flags.is_eval_value)
			{
				fetch_data(state, op1);
				fetch_data(state, op2);

				res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
				res->val.sllval = get_ctype_int(state, op1, 0) -
					get_ctype_int(state, op2, 0);
			}
			/* discard operands */
			destroy_exp_node(state, op1);
			destroy_exp_node(state, op2);
		}
		else
			goto type_error;
		return res;
	}
	else if (is_val_ctype_ptr(state, op1))
	{
		if (is_val_ctype_int(state, op2))
		{
			res = op1;
			c_exp_ptr_fixup(state, res);

			if (state->flags.is_eval_value)
			{
				fetch_data(state, op2);
				*(ARM_CORE_WORD *) (res->val.data) -=
					dtype_access_sizeof((struct dwarf_head_struct *)dtype_access_get_deref_type(res->data_obj.type))
					* get_ctype_int(state, op2, 0);
			}
			else
				clear_type_flags(state, res);
			/* discard second operand */
			destroy_exp_node(state, op2);
		}
		else if (is_val_ctype_ptr(state, op2))
		{
			signed long long sll, dsize;
			if (!dtype_access_are_types_compatible(op1->data_obj.type, op2->data_obj.type))
				goto type_error;

			res = get_exp_data_node(state);
			res->type = BASE_TYPE_ULONG_LONG;
			if (state->flags.is_eval_value)
			{
				c_exp_ptr_fixup(state, op1);
				c_exp_ptr_fixup(state, op2);

				res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
				sll = *(signed long long *) op1->val.data -
					*(signed long long *) op2->val.data;
				dsize = dtype_access_sizeof((struct dwarf_head_struct *) dtype_access_get_deref_type(op1->data_obj.type));
				if (!dsize)
					panic("");
				if (sll % dsize)
				{
					destroy_exp_node(state, op1);
					destroy_exp_node(state, op2);
					set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "pointers bad");
					return 0;
				}
				sll /= dsize;
				res->val.sllval = sll;
			}
			/* discard operands */
			destroy_exp_node(state, op1);
			destroy_exp_node(state, op2);
		}
		else
			goto type_error;
		return res;
	}
	else
	{
type_error:
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
}


/*!
 *	\fn	static struct exp_data_node * c_exp_add(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	performs an addition operation
 *
 *	this function adds op2 to op1; after return from this function,
 *	memory pointed to by op1 and op2 is invalid and op1 and op2
 *	must not be used any more; this can be used for address arithmetic
 *	calculations - in this case only one of the operands can be a pointer
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	first operand for the addition operator;
 *			can be of a numeric or a non-void pointer type
 *	\param	op2	second operand for the addition operator;
 *			can be of a numeric or a non-void pointer type
 *	\return	the result of the addition; can be of numeric or
 *		pointer type (in the case of address arithmetic
 *		calculations)
 *	\todo	define and properly handle errors here
 */
static struct exp_data_node * c_exp_add(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
{
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}

	if (is_val_ctype_int(state, op1))
	{
		if (is_val_ctype_int(state, op2))
		{
			res = get_exp_data_node(state);
			res->type = BASE_TYPE_ULONG_LONG;

			if (state->flags.is_eval_value)
			{
				fetch_data(state, op1);
				fetch_data(state, op2);

				res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
				res->val.sllval = get_ctype_int(state, op1, 0) +
					get_ctype_int(state, op2, 0);
			}
			/* discard operands */
			destroy_exp_node(state, op1);
			destroy_exp_node(state, op2);
		}
		else if (is_val_ctype_ptr(state, op2))
		{
handle_ptr_addition:
			res = op2;

			c_exp_ptr_fixup(state, res);
			if (state->flags.is_eval_value)
			{
				fetch_data(state, op1);
				*(ARM_CORE_WORD *) (res->val.data) +=
					dtype_access_sizeof((struct dwarf_head_struct *)dtype_access_get_deref_type(res->data_obj.type))
					* get_ctype_int(state, op1, 0);
			}
			else
				clear_type_flags(state, res);
			/* discard op1 */
			destroy_exp_node(state, op1);
		}
		else
			goto type_error;
		return res;
	}
	else if (is_val_ctype_ptr(state, op1))
	{
		/* swap operands and jump above */
		if (is_val_ctype_int(state, op2))
		{
			struct exp_data_node * t;
			t = op1;
			op1 = op2;
			op2 = t;
			goto handle_ptr_addition;
		}
		else
			goto type_error;
	}
	else
	{
type_error:
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
}

/*!
 *	\fn	static struct exp_data_node * c_exp_mul(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	performs a multiplication operation
 *
 *	this function multiplies op1 and op2; after return from this
 *	function, memory pointed to by op1 and op2 is invalid and
 *	op1 and op2 must not be used any more
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	first operand for the addition operator;
 *			must be of a numeric type
 *	\param	op2	second operand for the addition operator;
 *			must be of a numeric type
 *	\return	the result of the multiplication
 *	\todo	define and properly handle errors here
 */
static struct exp_data_node * c_exp_mul(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
{
signed long long sll;
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}

	if (!is_val_ctype_int(state, op1) || !is_val_ctype_int(state, op2))
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
	res = get_exp_data_node(state);
	res->type = BASE_TYPE_LONG_LONG;
	if (state->flags.is_eval_value)
	{
		fetch_data(state, op1);
		fetch_data(state, op2);

		sll = get_ctype_int(state, op1, 0) * get_ctype_int(state, op2, 0);

		res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
		res->val.sllval = sll;
	}
	/* discard operands */
	destroy_exp_node(state, op1);
	destroy_exp_node(state, op2);
	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_div(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	performs a division operation
 *
 *	this function divides op1 by op2; after return from this
 *	function, memory pointed to by op1 and op2 is invalid and
 *	op1 and op2 must not be used any more; division by zero is
 *	not tolerated
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	divident
 *	\param	op2	divisor, cant be zero
 *	\return	the result of the division
 *	\todo	define and properly handle errors here
 */
static struct exp_data_node * c_exp_div(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
{
signed long long sll;
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}

	if (!is_val_ctype_int(state, op1) || !is_val_ctype_int(state, op2))
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
	res = get_exp_data_node(state);
	res->type = BASE_TYPE_LONG_LONG;

	if (state->flags.is_eval_value)
	{
		sll = get_ctype_int(state, op2, 0);
		if (!sll)
		{
			destroy_exp_node(state, op1);
			destroy_exp_node(state, op2);
			free(res);
			set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "divide by zero");
			return 0;
		}
		sll = get_ctype_int(state, op1, 0) / sll;

		res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
		res->val.sllval = sll;
	}
	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_mod(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	performs a modulo operation
 *
 *	this function computes op1 modulo op2; after return from
 *	this function, memory pointed to by op1 and op2 is invalid
 *	and op1 and op2 must not be used any more; op2 cant be zero
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	divident
 *	\param	op2	divisor, cant be zero
 *	\return	the result of the division
 *	\todo	define and properly handle errors here
 */
static struct exp_data_node * c_exp_mod(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
{
signed long long sll;
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}

	if (!is_val_ctype_int(state, op1) || !is_val_ctype_int(state, op2))
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
	res = get_exp_data_node(state);
	res->type = BASE_TYPE_LONG_LONG;

	if (state->flags.is_eval_value)
	{
		sll = get_ctype_int(state, op2, 0);
		if (!sll)
		{
			destroy_exp_node(state, op1);
			destroy_exp_node(state, op2);
			free(res);
			set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "divide by zero");
			return 0;
		}
		sll = get_ctype_int(state, op1, 0) % sll;

		res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
		res->val.sllval = sll;
	}
	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_shift(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2, bool is_left_shift)
 *	\brief	performs a left or right shift operation
 *
 *	this function computes op1 shifted left or right by op2 places;
 *	after return from the function, memory pointed to
 *	by op1 and op2 is invalid and op1 and op2 must not be used any more;
 *	a shift to the right is an arithmetic one (rather than a
 *	logical one) whenever the shift value is of a signed target
 *	integral type, or is of a built-in signed integral type
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	value to be shifted, must be of integral type
 *	\param	op2	shift amount, must be of integral type
 *	\param	is_left_shift	determines the shift direction,
 *			if true - shifts left, otherwise - shifts right
 *	\return	the result of the shift operation
 *	\todo	define and properly handle errors here
 */
static struct exp_data_node * c_exp_shift(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2, bool is_left_shift)
{
signed long long sll;
int shift_amount;
struct exp_data_node * res;
bool is_signed_int;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}

	if (!is_val_ctype_int(state, op1) || !is_val_ctype_int(state, op2))
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
	res = get_exp_data_node(state);
	res->type = BASE_TYPE_LONG_LONG;

	if (state->flags.is_eval_value)
	{
		fetch_data(state, op1);
		fetch_data(state, op2);

		sll = get_ctype_int(state, op1, &is_signed_int);
		/* if shift_amount is negative, or is larger than or equal
		 * to the width of the promoted left operand, the c standard says
		 * that the behavior is undefined, so dont bother to be too smart
		 * here */
		shift_amount = get_ctype_int(state, op2, 0);
		if (is_left_shift)
			sll <<= shift_amount;
		else if (is_signed_int)
			/* arithmetic shift right */
			sll >>= shift_amount;
		else
			/* logical shift right */
			sll = (unsigned long long) sll >> shift_amount;

		res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
		res->val.sllval = sll;
	}
	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_arr_subscript(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	performs an array subscription operation
 *
 *	this function performs an array subscription operation by
 *	computing the result address, but *does not* fetch the value
 *	located at the address computed; this is the proper behavior
 *	as the value may never be needed and may indeed point to invalid
 *	memory; an example:
 *		&arr[2]; equivalent to: arr + 2; the value arr[2] is never
 *	needed
 *	as the c language standard says, one of the operands of the
 *	array subscript operator must have type 'pointer to -type-',
 *	and the other must have a numeric type, and the result will
 *	have type '-type-'; this means these sample
 *	expressions are valid and equivalent: 'ptr[2]', '2[ptr]'
 *
 *	after return from the function, memory pointed to
 *	by op1 and op2 is invalid and op1 and op2 must not be used any more
 *
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	first operand for the array subscript operator
 *	\param	op2	second operand for the array subscript operator
 *	\return	the result of the array subscript operator; this will
 *		be an expression data node of type the dereferenced type
 *		of the pointer operand, will have its address properly
 *		initialized, but its value will not have been fetched, as
 *		it may never be actually needed (as in, e.g., as mentioned
 *		above, in expressions such as: &ptr[2])
 */
static struct exp_data_node * c_exp_arr_subscript(struct parser_state * state, struct exp_data_node * op1, struct exp_data_node * op2)
{
struct exp_data_node * res;

	/* sanity checks */
	if (op1->type == DATA_TYPE_INVALID || op2->type == DATA_TYPE_INVALID)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad operands to array subscript");
		return 0;
	}
	res = c_exp_add(state, op1, op2);
	if (res == 0)
	{
		/* error computing expression */
		return 0;
	}
	/* make sure we indeed have a pointer as a result */
	if (!is_val_ctype_ptr(state, res))
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}
	/* shortcut to the dereference operator */
	return c_exp_unary_op(state, UNARY_DEREF, res);
}

/*!
 *	\fn	static struct exp_data_node * c_exp_member_select(struct parser_state * state, struct exp_data_node * op1, char * member_name, bool is_ptr_member_select)
 *	\brief	computes the result of the c member select operators (operators '.' and '->')
 *
 *	after return from the function, memory pointed to
 *	by op1 and op2 is invalid and op1 and op2 must not be used any more
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	op1	first operand for the array subscript operator; must
 *			have a structure or union type (when the is_ptr_member_select
 *			parameter is false), or a pointer to a structure type (
 *			when the is_ptr_member_select parameter is true)
 *			or a union type
 *	\param	member_name	second operand for the array subscript operator; must
 *			be an identifier denoting the member's name
 *	\param	is_ptr_member_select	determines if the operator is a '.' member
 *			select (false), or a '->' member select (true) operator
 *	\return	the result of the member select operator */
static struct exp_data_node * c_exp_member_select(struct parser_state * state, struct exp_data_node * op1, char * member_name, bool is_ptr_member_select)
{
struct exp_data_node * res;
struct dtype_data * m;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		return 0;
	}

	if (is_ptr_member_select)
	{
		/* a '->' operator */
		if (!is_val_ctype_ptr(state, op1))
		{
			destroy_exp_node(state, op1);
			set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad type");
			return 0;
		}
		/* c_exp_unary_op() below will destroy op1,
		 * so no need to manually destroy it here */
		res = c_exp_unary_op(state, UNARY_DEREF, op1);
		if (!res)
			return 0;
	}
	else
	{
		/* a '.' operator */
		res = get_exp_data_node(state);
		*res = *op1;
		destroy_exp_node(state, op1);
	}
	if (!is_val_ctype_struct_or_union(state, res))
	{
		destroy_exp_node(state, res);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad type");
		return 0;
	}
	/* scan the list of members */
	m = dtype_access_get_unqualified_base_type(res->data_obj.type)->struct_data.data_members;
	while (m)
	{
		/*! \todo	properly handle anonymous members here */
		if (/* skip over anonymous members */m->name
				&& !strcmp(m->name, member_name))
			break;
		m = m->member_data.sib_ptr;
	}
	if (!m)
	{
		/* member not found */
		destroy_exp_node(state, res);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "strct/union member not found");
		return 0;
	}
	res->data_obj.type = m->member_data.member_type;
	if (state->flags.is_eval_value)
	{
		if (!res->data_obj.flags.is_addr_applicable || !res->data_obj.flags.is_val_applicable
				|| res->data_obj.flags.is_val_fetched)
			panic("");
		/*! \todo	a quick hack - fix this */
		if (res->data_obj.flags.is_addr_a_reg_nr)
			panic("");
		/* compute the address of the member within the struct/union */
		if (m->is_node_a_static_cxx_struct_member)
			panic("");
		if (m->member_data.member_location == UINT_MAX)
			panic("");
		res->data_obj.addr += m->member_data.member_location;
	}

	free(member_name);

	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_unary_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * exp_node)
 *	\brief	handle the c '&', '*', '!', '~', '+', '-' unary operators in an expression
 *
 *	after return from the function, memory pointed to by exp_node
 *	is invalid and exp_node must not be used any more
 *
 *	\note	the result of a dereference operator will
 *		be an expression data node of type the dereferenced type
 *		of the pointer operand, will have its address properly
 *		initialized, but its value will not have been fetched, as
 *		it may never be actually needed (as in, e.g., as mentioned
 *		above, in expressions such as: "&ptr[2]", "&*(ptr + 2)",
 *		"sizeof * ptr", etc.)
 *		an implementation detail - when evaluating an expression,
 *		the array subscript operator invokes this after computing
 *		a pointer value
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	opc	operator opcode; see enum C_EXP_OPERATOR_ENUM for details
 *	\param	exp_node	operand for the unary operator
 *	\return	the result of the unary operator
 */
static struct exp_data_node * c_exp_unary_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * exp_node)
{
signed long long sll;
struct exp_data_node * res;

	/* in case of an indirection (dereference - unary asterisk)
	 * or a get address operator (unary ampersand) 
	 * the value of the operand should not be fetched */
	if (resolve_id(state, exp_node) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, exp_node);
		return 0;
	}

	switch (opc)
	{
		/*! \note	you may want to read the notes about the
		 *		dereference operator in the comments
		 *		about this function */
		case UNARY_DEREF:
		{
			if (!is_val_ctype_ptr(state, exp_node))
			{
				destroy_exp_node(state, exp_node);
				set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "not a pointer type");
				return 0;
			}

			c_exp_ptr_fixup(state, exp_node);
			res = get_exp_data_node(state);
			*res = *exp_node;
			res->data_obj.type = dtype_access_get_deref_type(exp_node->data_obj.type);

			if (state->flags.is_eval_value)
			{
				res->data_obj.flags.is_val_fetched = 0;
				/* clear the is_addr_a_reg_nr, if the pointer
				 * has been fetched from a register */
				res->data_obj.flags.is_addr_a_reg_nr = 0;
				res->data_obj.addr = *(ARM_CORE_WORD *) res->val.data;
				res->val.data = 0;
			}
			else
				clear_type_flags(state, res);
			res->data_obj.flags.is_addr_applicable = 1;
			destroy_exp_node(state, exp_node);
			return res;
		}
		case UNARY_GETADDR:
		{
			/* special case
			 * this is a special case; this is
		         * the one of two cases handled in this module
			 * in which the type tree built from
			 * the dwarf debug information is allowed
			 * to artificially grow by adding type
			 * nodes describing pointers (the other case
			 * is handled in c_exp_ptr_fixup()); these artificial
			 * nodes are built here, by the expression parsing
			 * code and are referenced only by the expression
			 * evaluation code; they are also deallocated
			 * here; no one outside here is able to see these
			 * artificial nodes, as they are referred to only
			 * within this module (pointers to them are visible
			 * nowhere outside of this module) */
			struct dtype_data * ptype;

			if (!exp_node->data_obj.flags.is_addr_applicable)
			{
				set_parse_abort_vars(state,
						GEAR_ERR_EXPR_SEMANTIC_ERROR,
						"object referenced does not have address (probably is a compile-time computed constant)");
				destroy_exp_node(state, exp_node);
				return 0;
			}

			/*! \todo	a quick hack - fix this */
			if (exp_node->data_obj.flags.is_addr_a_reg_nr)
				panic("");

			if (!(ptype = calloc(1, sizeof * ptype)))
				panic("");
			res = get_exp_data_node(state);
			ptype->dtype_class = DTYPE_CLASS_PTR;
			ptype->ptr_type = exp_node->data_obj.type;

			res->type = NON_BASE_TYPE_OBJECT;
			if (state->flags.is_eval_value)
			{
				res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
				/*! \todo	this is evil, fix it */
				if (!(res->val.data = malloc(sizeof(ARM_CORE_WORD))))
					panic("");
				/*! \todo	a quick hack - fix this */
				if (exp_node->data_obj.flags.is_addr_a_reg_nr)
					panic("");
				*(ARM_CORE_WORD *) res->val.data = exp_node->data_obj.addr;
			}
			res->data_obj.type = ptype;
			/* discard operand */
			destroy_exp_node(state, exp_node);
			return res;
		}
	}

	res = get_exp_data_node(state);
	if (state->flags.is_eval_value)
	{
		/* perform actual computations only if the expression
		 * is evaluated for its value */  
		fetch_data(state, exp_node);

		switch (opc)
		{
			case UNARY_PLUS:
				if (!is_val_ctype_int(state, exp_node))
				{
					destroy_exp_node(state, exp_node);
					destroy_exp_node(state, res);
					set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad type");
					return 0;
				}
				sll = get_ctype_int(state, exp_node, 0);
				res->val.sllval = sll;
				break;
			case UNARY_MINUS:
				if (!is_val_ctype_int(state, exp_node))
				{
					destroy_exp_node(state, exp_node);
					destroy_exp_node(state, res);
					set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad type");
					return 0;
				}
				sll = get_ctype_int(state, exp_node, 0);
				res->val.sllval = -sll;
				break;
			case UNARY_BITWISE_NOT:
				if (!is_val_ctype_int(state, exp_node))
				{
					destroy_exp_node(state, exp_node);
					destroy_exp_node(state, res);
					set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad type");
					return 0;
				}
				sll = get_ctype_int(state, exp_node, 0);
				res->val.sllval = ~sll;
				break;
			case UNARY_LOGICAL_NOT:
				if (is_val_ctype_int(state, exp_node))
				{
					sll = get_ctype_int(state, exp_node, 0);
				}
				else if (is_val_ctype_ptr(state, exp_node))
				{
					if (*(ARM_CORE_WORD *) exp_node->val.data)
						sll = 1;
					else
						sll = 0;
				}
				else
				{
					destroy_exp_node(state, exp_node);
					destroy_exp_node(state, res);
					set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "bad type");
					return 0;
				}
				res->val.sllval = sll ? 0 : 1;
				break;
		}

		res->data_obj.flags.is_val_fetched = res->data_obj.flags.is_val_applicable = 1;

	}

	res->data_obj.flags.is_addr_applicable = 0;
	res->type = BASE_TYPE_LONG_LONG;
	/* discard operand */
	destroy_exp_node(state, exp_node);
	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_bitwise_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	handle binary bitwise operators ('&', '|', '^') in an expression
 *
 *	after return from the function, memory pointed to
 *	by op1 and op2 is invalid and op1 and op2 must not be used any more
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	opc	operator opcode; see enum C_EXP_OPERATOR_ENUM for details
 *	\param	op1	first operand; must be of integer type
 *	\param	op2	second operand; must be of integer type
 *	\return	the result of the binary bitwise operator
 */
static struct exp_data_node * c_exp_bitwise_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * op1, struct exp_data_node * op2)
{
long long l1, l2;
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}
	
	if (!is_val_ctype_int(state, op1) || !is_val_ctype_int(state, op2))
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}

	if (state->flags.is_eval_value)
	{
		fetch_data(state, op1);
		fetch_data(state, op2);
		l1 = get_ctype_int(state, op1, 0);
		l2 = get_ctype_int(state, op2, 0);
		switch (opc)
		{
			case BITWISE_AND:
				l1 &= l2;
				break;
			case BITWISE_OR:
				l1 |= l2;
				break;
			case BITWISE_XOR:
				l1 ^= l2;
				break;
			default:
				panic("");
		}
	}
	else
	{
		/* expression type evaluation */
		switch (opc)
		{
			case BITWISE_AND:
			case BITWISE_OR:
			case BITWISE_XOR:
				break;
			default:
				panic("");
		}
	}

	res = get_exp_data_node(state);
	if (state->flags.is_eval_value)
		res->val.sllval = l1;
	res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
	res->type = BASE_TYPE_LONG_LONG;
	return res;
}

/*!
 *	\fn	static struct exp_data_node * c_exp_logical_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * op1, struct exp_data_node * op2)
 *	\brief	handle binary logical operators ('&&', '||') in an expression
 *
 *	after return from the function, memory pointed to
 *	by op1 and op2 is invalid and op1 and op2 must not be used any more
 *
 *	\note	short circuiting is supported
 *
 *	\param	state	current parser state (see struct parser_state for details)
 *	\param	opc	operator opcode; see enum C_EXP_OPERATOR_ENUM for details
 *	\param	op1	first operand; must be of scalar type
 *	\param	op2	second operand; must be of scalar type
 *	\return	the result of the binary logical operator
 */
static struct exp_data_node * c_exp_logical_op(struct parser_state * state, enum C_EXP_OPERATOR_ENUM opc, struct exp_data_node * op1, struct exp_data_node * op2)
{
long long l1;
struct exp_data_node * res;

	if (resolve_id(state, op1) != GEAR_ERR_NO_ERROR
			|| resolve_id(state, op2) != GEAR_ERR_NO_ERROR)
	{
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		return 0;
	}
	
	if (!is_val_ctype_scalar(state, op1) || !is_val_ctype_scalar(state, op2))
	{
type_error:		
		destroy_exp_node(state, op1);
		destroy_exp_node(state, op2);
		set_parse_abort_vars(state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "incompatible types");
		return 0;
	}

	if (state->flags.is_eval_value)
	{
		fetch_data(state, op1);
		fetch_data(state, op2);
		if (is_val_ctype_int(state, op1))
		{
			l1 = get_ctype_int(state, op1, 0);
		}
		else if (is_val_ctype_ptr(state, op1))
		{
			/* the object referenced should be of pointer type */
			if (*(ARM_CORE_WORD *) op1->val.data)
				l1 = 1;
			else
				l1 = 0;
		}
		else
			goto type_error;
		if (l1)
			l1 = 1;
		/* check for short circuiting */
		switch (opc)
		{
			case LOGICAL_AND:
				if (!l1)
					goto done;
				break;
			case LOGICAL_OR:
				if (l1)
					goto done;
				break;
			default:
				panic("");
		}

		if (is_val_ctype_int(state, op2))
		{
			l1 = get_ctype_int(state, op2, 0);
		}
		else if (is_val_ctype_ptr(state, op2))
		{
			/* the object referenced should be of pointer type */
			if (*(ARM_CORE_WORD *) op2->val.data)
				l1 = 1;
			else
				l1 = 0;
		}
		else
			goto type_error;
		if (l1)
			l1 = 1;
	}
	else
	{
		/* expression type evaluation */
		switch (opc)
		{
			case LOGICAL_AND:
			case LOGICAL_OR:
				break;
			default:
				panic("");
		}
	}

done:

	res = get_exp_data_node(state);
	if (state->flags.is_eval_value)
		res->val.sllval = l1;
	res->data_obj.flags.is_val_applicable = res->data_obj.flags.is_val_fetched = 1;
	res->type = BASE_TYPE_LONG_LONG;
	return res;
}


/*!
 *	\fn	static void c_lang_error(struct parser_state * state, yyscan_t yyscanner, char * msg)
 *	\brief	error function invoked by the bison generated yyparse() parsing function when a syntax error occurs
 *
 *	this is invoked when the bison parser detects a syntax error, it just
 *	sets up a generic "syntax error" error message and adjust the parser
 *	state properly for a parse abortion
 *
 *	\param	state	current parser state 
 *	\param	yyscanner	lexical scanner state, currently unused
 *	\param	msg	an error message passed by the parser
 *	\todo	this is currently not exported to a frontend, but it
 *		should be exported as it is more informative than the
 *		generic "syntax error" message currently passed to a frontend
 *	\return	none */
static void c_lang_error(struct parser_state * state, yyscan_t yyscanner, char * msg)
{
	gprintf("\n%*s\n%*s\n", c_lex_column, "^", c_lex_column, msg);
	state->is_parser_aborted = true;
	state->errcode = GEAR_ERR_EXPR_SYNTAX_ERROR;
	state->parse_msg = strdup(msg);
}

/*
 *
 * exported functions follow
 *
 */

/*!
 *	\fn	void c_expression_eval_and_print(struct gear_engine_context * ctx, const char * exp_str, struct expr_eval_flags flags)
 *	\brief	evaluate and print a c language expression
 *
 *	this evaluates and prints a c language expression, taking
 *	in account program scope
 *
 *	\todo	pass information about the desired scope to work into;
 *		currently, the evaluation is done in the global
 *		program scope
 *	\todo	move the initialization of the lexer state variables
 *		(the call to c_langlex_init() below) to some
 *		initialization code, and, respectively, supply
 *		some cleanup routine to be invoked on engine
 *		shutdown (to clean up the lexer variables - the
 *		call to c_langlex_destroy() below); this way
 *		the lexer variables will not be allocated and
 *		deallocated on each call to this routine, which
 *		is quite braindamaged
 *
 *	\param	ctx	context to work in
 *	\param	exp_str	the string of the expression to parse; must
 *			be nul terminated
 *	\param	flags	printing flags; for valid values, see the flag
 *			declaration structure in the header for this source file
 *	\return	none
 *	\todo	do not destroy the lexer on each parse - this is not necessary
 *	\todo	define and properly return error codes */
void c_expression_eval_and_print(struct gear_engine_context * ctx, const char * exp_str, struct expr_eval_flags flags)
{
YY_BUFFER_STATE lex_buf_state;
yyscan_t yyscanner;
struct parser_state state;
int parser_errcode;

	/* initialize parser state */
	memset(&state, 0, sizeof state);
	state.ctx = ctx;
	state.errcode = GEAR_ERR_NO_ERROR;

	if (!flags.print_type && !flags.print_val)
		panic("");
	if (flags.print_type)
		state.flags.is_eval_type = 1;
	if (flags.print_val)
		state.flags.is_eval_value = 1;

	if (c_langlex_init(&yyscanner))
		panic("");
	lex_buf_state = c_lang_scan_string(exp_str, yyscanner);
	parser_errcode = c_lang_parse(&state, yyscanner);
	c_lang_delete_buffer(lex_buf_state, yyscanner);
	if (c_langlex_destroy(yyscanner))
		panic("");

	/* handle errors */
	if (parser_errcode)
	{

dump_error_and_return:

		/* parse error occurred */
		if (state.is_parser_aborted == false)
			panic("");
		if (state.errcode == GEAR_ERR_NO_ERROR)
			panic("");
		/* just dump error code and message and return */
		miprintf("ERRCODE=[%i,\"%s\"],EXPRESSION_TYPE_DATA,", state.errcode,
				state.parse_msg ? state.parse_msg : "");
		/* terminate machine interface output record */
		miprintf("\n");
		if (state.parse_msg)
			free(state.parse_msg);
		return;
	}

	if (flags.print_val && state.is_target_access_failed)
	{
		set_parse_abort_vars(&state, GEAR_ERR_TARGET_ACCESS_ERROR, "error accessing the target");
		goto dump_error_and_return;
	}

	/* see if the result of the parse is an unresolved identifier */  
	if (state.parse_head->type == UNRESOLVED_ID
		&& resolve_id(&state, state.parse_head) != GEAR_ERR_NO_ERROR)
	{
		/* the identifier could not be resolved */
		destroy_exp_node(&state, state.parse_head);
		goto dump_error_and_return;
	}

	/* expression parsed successfully */
	/* dump expression result depending on its type */
	switch (state.parse_head->type)
	{
		case BASE_TYPE_SIGNED_CHAR:
		case BASE_TYPE_UCHAR:
		case BASE_TYPE_SHORT:
		case BASE_TYPE_USHORT:
		case BASE_TYPE_INT:
		case BASE_TYPE_UINT:
		case BASE_TYPE_LONG:
		case BASE_TYPE_ULONG:
		case BASE_TYPE_LONG_LONG:
		case BASE_TYPE_ULONG_LONG:

			if (flags.print_val
				&& (!state.parse_head->data_obj.flags.is_val_applicable
						|| !state.parse_head->data_obj.flags.is_val_fetched))
			{
				set_parse_abort_vars(&state, GEAR_ERR_GENERIC_ERROR, "error retrieving expression value");
				goto dump_error_and_return;
			}

			gprintf("gear_int %lli", state.parse_head->val.sllval);
			/* expression parsed successfully */
			miprintf("ERRCODE=[%i,\"%s\"],EXPRESSION_TYPE_DATA,", GEAR_ERR_NO_ERROR, state.parse_msg);
			if (state.parse_msg)
				free(state.parse_msg);
			miprintf("[");
			if (flags.print_type)
				miprintf("TYPENAME = \"#int\",");
			if (flags.print_val)
			{
				signed long long sll;
				miprintf("VALUE_LIST = (");
				sll = state.parse_head->val.sllval;
				miprintf("\"%lli (", sll);
				if (sll <= 0xff)
					miprintf("0x%02llx", sll);
				else if (sll <= 0xffff)
					miprintf("0x%04llx", sll);
				else if (sll <= 0xffffffff)
					miprintf("0x%08llx", sll);
				else
					miprintf("0x%016llx", sll);
				miprintf(")\",");
				miprintf("),");
			}
			break;
		case NON_BASE_TYPE_OBJECT:
			if (flags.print_val)
			{
				/* only fetch the data if it is indeed to
				 * be printed */
				fetch_data(&state, state.parse_head);
				if (flags.print_val && state.is_target_access_failed)
				{
					set_parse_abort_vars(&state, GEAR_ERR_TARGET_ACCESS_ERROR, "error accessing the target");
					goto dump_error_and_return;
				}
			}
			miprintf("ERRCODE=[%i,\"%s\"],EXPRESSION_TYPE_DATA,", GEAR_ERR_NO_ERROR, state.parse_msg);
			if (state.parse_msg)
				free(state.parse_msg);
			miprintf("[");
			if (flags.print_type)
				type_dump_mi(state.parse_head->data_obj.type, exp_str, false);
			if (flags.print_val)
			{
				miprintf("VALUE_LIST = (");
				type_dump_data_mi(ctx, state.parse_head->data_obj.type, state.parse_head->val.data);
				miprintf("),");
			}
			break;
		case SUBPROGRAM_OBJECT:
		{
			if (flags.print_val
				&& !state.parse_head->subprogram_data->is_entry_point_valid)
			{
				set_parse_abort_vars(&state, GEAR_ERR_GENERIC_ERROR, "subprogram entry point unavailable");
				goto dump_error_and_return;
			}
			miprintf("ERRCODE=[%i,\"%s\"],EXPRESSION_TYPE_DATA,", GEAR_ERR_NO_ERROR, state.parse_msg);
			if (state.parse_msg)
				free(state.parse_msg);
			miprintf("[");
			if (flags.print_type)
				type_dump_mi(	/* create a data type node
						 * just for the call */
						& (struct dtype_data)
						{
							.head = { .tag = 0, .parent = 0 },
							{ .dtype_class = DTYPE_CLASS_SUBROUTINE, .is_node_a_declaration = 0, },
							.data_size = 0,
							.name = 0,
							{ .subroutine_data.sub
								= state.parse_head->subprogram_data },
						},
						exp_str,
						false);
			if (flags.print_val)
			{
				miprintf("VALUE_LIST = (");
				miprintf("\"0x%08x\",", (unsigned int)state.parse_head->subprogram_data->entry_pc);
				miprintf("),");
			}
		}
			break;
		default:
			set_parse_abort_vars(&state, GEAR_ERR_EXPR_SEMANTIC_ERROR, "expression evaluated to an unknown type");
			goto dump_error_and_return;
	}
	miprintf("]");
	/* terminate machine interface output record */
	miprintf("\n");
	destroy_exp_node(&state, state.parse_head);
}

/*!
 *	\fn	bool c_expression_eval_int(struct gear_engine_context * ctx, const char * exp_str, signed long long * val)
 *	\brief	evaluate a c language expression, expecting the result to be an integer
 *
 *	this evaluates a c language expression, taking
 *	in account program scope; the result is expected to be an
 *	integer
 *
 *	\todo	pass information about the desired scope to work into;
 *		currently, the evaluation is done in the global
 *		program scope
 *	\todo	move the initialization of the lexer state variables
 *		(the call to c_langlex_init() below) to some
 *		initialization code, and, respectively, supply
 *		some cleanup routine to be invoked on engine
 *		shutdown (to clean up the lexer variables - the
 *		call to c_langlex_destroy() below); this way
 *		the lexer variables will not be allocated and
 *		deallocated on each call to this routine, which
 *		is quite braindamaged
 *
 *	\param	ctx	context to work in
 *	\param	exp_str	the string of the expression to parse; must
 *			be nul terminated
 *	\param	val	a pointer to where to store the value of the
 *			expression
 *	\return	true, if the expression passed successfully evaluated to
 *		an integer, false if some error occurred
 *	\todo	do not destroy the lexer on each parse - this is not necessary
 *	\todo	define and properly return error codes */
bool c_expression_eval_int(struct gear_engine_context * ctx, const char * exp_str, signed long long * val)
{
YY_BUFFER_STATE lex_buf_state;
yyscan_t yyscanner;
struct parser_state state;
int parser_errcode;
bool res;

	/* initialize parser state */
	memset(&state, 0, sizeof state);
	state.ctx = ctx;
	state.flags.is_eval_value = 1;
	state.errcode = GEAR_ERR_NO_ERROR;

	if (c_langlex_init(&yyscanner))
		panic("");
	lex_buf_state = c_lang_scan_string(exp_str, yyscanner);
	parser_errcode = c_lang_parse(&state, yyscanner);
	c_lang_delete_buffer(lex_buf_state, yyscanner);
	if (c_langlex_destroy(yyscanner))
		panic("");

	/* handle errors */
	if (parser_errcode)
	{
		return false;
	}

	/* dump expression result according to its type */
	res = true;
	switch (state.parse_head->type)
	{
		case BASE_TYPE_SIGNED_CHAR:
		case BASE_TYPE_UCHAR:
		case BASE_TYPE_SHORT:
		case BASE_TYPE_USHORT:
		case BASE_TYPE_INT:
		case BASE_TYPE_UINT:
		case BASE_TYPE_LONG:
		case BASE_TYPE_ULONG:
		case BASE_TYPE_LONG_LONG:
		case BASE_TYPE_ULONG_LONG:
			*val = state.parse_head->val.sllval;
			break;
		case UNRESOLVED_ID:
			if (resolve_id(&state, state.parse_head) != GEAR_ERR_NO_ERROR)
			{
				/* the identifier could not be resolved */
				res = false;
			}
			else
			{
				if (state.parse_head->type == SUBPROGRAM_OBJECT)
				{
					if (!state.parse_head->subprogram_data->is_entry_point_valid)
						panic("");
					* val = state.parse_head->subprogram_data->entry_pc;
				}
				else
					res = false;
			}
			break;
		default:
			res = false;
	}
	destroy_exp_node(&state, state.parse_head);
	return res;
}

