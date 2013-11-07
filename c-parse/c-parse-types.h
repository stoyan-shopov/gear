#ifndef __C_PARSE_TYPES_H__
#define __C_PARSE_TYPES_H__

enum CONST_KIND
{
	CONST_INVALID	= 0,
	CONST_INT,
	CONST_FLOAT,
};

struct primary_expr
{
	/*
	   primary_expression
	   : IDENTIFIER
	   | CONSTANT
	   | STRING_LITERAL
	   | '(' expression ')'
	   ;
	   */
	const char	* ident;
	enum CONST_KIND const_kind;
	union
	{
		signed long long	iconst;
		long double		flconst;
	};
	const char	* str_literal;
	struct expr	* expr;
};

enum POSTFIX_OP
{
	INVALID_POSTFIX_OP	= 0,
	ARR_SUBSCRIPT,
	FUNC_CALL,
	STRUCT_MEMBER_ACCESS,
	STRUCT_MEMBER_PTR_ACCESS,
	POSTFIX_INC_OP,
	POSTFIX_DEC_OP,
};

struct postfix_expr
{
	/*
	   postfix_expression
	   : primary_expression
	   | postfix_expression '[' expression ']'
	   | postfix_expression '(' ')'
	   | postfix_expression '(' argument_expression_list ')'
	   | postfix_expression '.' IDENTIFIER
	   | postfix_expression PTR_OP IDENTIFIER
	   | postfix_expression INC_OP
	   | postfix_expression DEC_OP
	   ;
	   */
	enum POSTFIX_OP op;
	struct primary_expr	* primary_expr;
	struct postfix_expr	* postfix_expr;
	struct expr		* expr;
	struct arg_expr_list	* args;
	const char		* ident;
};

struct arg_expr_list
{
	/*
	   argument_expression_list
	   : expression
	   | argument_expression_list ',' expression
	   ;
	   */
	struct expr		* expr;
	struct arg_expr_list	* next;
};

enum UNARY_OP
{
	INVALID_UNARY_OP	= 0,
	PREFIX_INC_OP,
	PREFIX_DEC_OP,
	SIZEOF_OP,
	UN_AMPERSAND,
	UN_ASTERISK,
	UN_PLUS,
	UN_MINUS,
	UN_TILDE,
	UN_EXCLAM,
};

struct unary_expr
{
	/*
	   unary_expression
	   : postfix_expression
	   | INC_OP unary_expression
	   | DEC_OP unary_expression
	   | unary_operator cast_expression
	   | SIZEOF unary_expression
	   | SIZEOF '(' type_name ')'
	   ;

	   unary_operator
	   : '&'
	   | '*'
	   | '+'
	   | '-'
	   | '~'
	   | '!'
	   ;
	   */
	struct postfix_expr	* postfix_expr;
	struct unary_expr	* unary_expr;
	struct cast_expr	* cast_expr;
	struct type_name	* type_name;
	enum UNARY_OP op;
};

struct cast_expr
{
	/*
	   cast_expression
	   : unary_expression
	   | '(' type_name ')' cast_expression
	   ;
	   */
	struct unary_expr	* unary_expr;
	struct type_name	* type_name;
	struct cast_expr	* cast_expr;
};

enum MULT_OP
{
	INVALID_MULT_OP		= 0,
	MULT_TIMES,
	MULT_DIV,
	MULT_MOD,
};

struct mult_expr
{
	/*
	   multiplicative_expression
	   : cast_expression
	   | multiplicative_expression '*' cast_expression
	   | multiplicative_expression '/' cast_expression
	   | multiplicative_expression '%' cast_expression
	   ;
	   */
	enum MULT_OP op;
	struct cast_expr	* cast_expr;
	struct mult_expr	* mult_expr;
};


enum ADDITIVE_OP
{
	INVALID_ADDITIVE_OP	= 0,
	ADDITIVE_PLUS,
	ADDITIVE_MINUS,
};


struct additive_expr
{
	/*
	   additive_expression
	   : multiplicative_expression
	   | additive_expression '+' multiplicative_expression
	   | additive_expression '-' multiplicative_expression
	   ;
	   */
	enum ADDITIVE_OP op;
	struct mult_expr	* mult_expr;
	struct additive_expr	* additive_expr;
};


enum SHIFT_OP
{
	INVALID_SHIFT_OP	= 0,
	SHIFT_LEFT,
	SHIFT_RIGHT,
};

struct shift_expr
{
	/*
	   shift_expression
	   : additive_expression
	   | shift_expression LEFT_OP additive_expression
	   | shift_expression RIGHT_OP additive_expression
	   ;
	   */
	enum SHIFT_OP op;
	struct additive_expr	* additive_expr;
	struct shift_expr	* shift_expr;
};

enum REL_OP
{
	INVALID_REL_OP		= 0,
	REL_LESS,
	REL_GREATER,
	REL_LESS_OR_EQUAL,
	REL_GREATER_OR_EQUAL,

};

struct rel_expr
{
	/*
	   relational_expression
	   : shift_expression
	   | relational_expression '<' shift_expression
	   | relational_expression '>' shift_expression
	   | relational_expression LE_OP shift_expression
	   | relational_expression GE_OP shift_expression
	   ;
	   */
	enum REL_OP op;
	struct shift_expr	* shift_expr;
	struct rel_expr		* rel_expr;
};

enum EQ_OP
{
	INVALID_EQ_OP	= 0,
	EQ_EQUAL,
	EQ_NOT_EQUAL,
};

struct eq_expr
{
	/*
	   equality_expression
	   : relational_expression
	   | equality_expression EQ_OP relational_expression
	   | equality_expression NE_OP relational_expression
	   ;
	   */
	enum EQ_OP op;
	struct rel_expr		* rel_expr;
	struct eq_expr		* eq_expr;
};

enum AND_OP
{
	INVALID_AND_OP	= 0,
	AND_OP_AMPERSAND,
};

struct and_expr
{
	/*
	   and_expression
	   : equality_expression
	   | and_expression '&' equality_expression
	   ;
	   */
	enum AND_OP op;
	struct eq_expr		* eq_expr;
	struct and_expr		* and_expr;
};

enum XOR_EXPR_OP
{
	INVALID_XOR_OP	= 0,
	XOR_OP,
};

struct xor_expr
{
	/*
	   exclusive_or_expression
	   : and_expression
	   | exclusive_or_expression '^' and_expression
	   ;
	   */
	enum XOR_EXPR_OP op;
	struct and_expr		* and_expr;
	struct xor_expr		* xor_expr;
};

enum INCL_EXPR_OP
{
	INVALID_INCL_OR_OP,
	INCL_OR_OP,
};

struct incl_or_expr
{
	/*
	   inclusive_or_expression
	   : exclusive_or_expression
	   | inclusive_or_expression '|' exclusive_or_expression
	   ;
	   */
	enum INCL_EXPR_OP op;
	struct xor_expr		* xor_expr;
	struct incl_or_expr	* incl_or_expr;
	struct incl_or_op	* incl_or_op;
};

enum LOGICAL_AND_OP_EXPR
{
	INVALID_LOGICAL_AND_OP	= 0,
	LOGICAL_AND_OP
};

struct logical_and_expr
{
	/*
	   logical_and_expression
	   : inclusive_or_expression
	   | logical_and_expression AND_OP inclusive_or_expression
	   ;
	   */
	enum LOGICAL_AND_OP_EXPR op;
	struct incl_or_expr	* incl_or_expr;
	struct logical_and_expr	* logical_and_expr;
};


enum LOGICAL_OR_OP_EXPR
{
	INVALID_LOGICAL_OR_OP	= 0,
	LOGICAL_OR_OP,
};

struct logical_or_expr
{
	/*
	   logical_or_expression
	   : logical_and_expression
	   | logical_or_expression OR_OP logical_and_expression
	   ;
	   */
	enum LOGICAL_OR_OP_EXPR op;
	struct logical_and_expr	* logical_and_expr;
	struct logical_or_expr	* logical_or_expr;
};

enum COND_EXPR_OP
{
	INVALID_COND_EXPR_OP	= 0,
	LOGICAL_TERNARY_OP,
};

struct cond_expr
{
	/*
	   conditional_expression
	   : logical_or_expression
	   | logical_or_expression '?' expression ':' conditional_expression
	   ;
	   */
	enum COND_EXPR_OP op;
	struct logical_or_expr	* logical_or_expr;
	struct expr		* expr;
	struct cond_expr	* cond_expr;
};

enum EXPR_OP
{
	INVALID_EXPR_OP	= 0,
	SIMPLE_ASSIGN,
	MUL_ASSIGN_OP,
	DIV_ASSIGN_OP,
	MOD_ASSIGN_OP,
	ADD_ASSIGN_OP,
	SUB_ASSIGN_OP,
	LEFT_ASSIGN_OP,
	RIGHT_ASSIGN_OP,
	AND_ASSIGN_OP,
	XOR_ASSIGN_OP,
	OR_ASSIGN_OP,
};

struct expr
{
	/*
	   expression
	   : conditional_expression
	   | unary_expression assignment_operator expression
	   ;

	   assignment_operator
	   : '='
	   | MUL_ASSIGN
	   | DIV_ASSIGN
	   | MOD_ASSIGN
	   | ADD_ASSIGN
	   | SUB_ASSIGN
	   | LEFT_ASSIGN
	   | RIGHT_ASSIGN
	   | AND_ASSIGN
	   | XOR_ASSIGN
	   | OR_ASSIGN
	   ;
	   */
	enum EXPR_OP op;
	struct cond_expr	* cond_expr;
	struct expr		* expr;
	struct unary_expr		* unary_expr;
};


struct type_spec
{
	/*
	   type_specifier
	   : VOID
	   | CHAR
	   | SHORT
	   | INT
	   | LONG
	   | FLOAT
	   | DOUBLE
	   | SIGNED
	   | UNSIGNED
	   | struct_or_union_specifier
	   | enum_specifier
	   | TYPE_NAME
	   ;
	   */
	enum TYPE_SPEC_ENUM
	{
		INVALID_TYPE_SPEC	= 0,
		T_VOID,
		T_CHAR,
		T_SHORT,
		T_INT,
		T_LONG,
		T_FLOAT,
		T_DOUBLE,
		T_SIGNED,
		T_UNSIGNED,
		T_STRUCT_SPEC,
		T_UNION_SPEC,
		T_ENUM_SPEC,
		T_TYPE_NAME,
	}
	tspec;
	/* enum/struct/union name */
	const char	* name;
};

struct type_spec_list
{
	/*
	   specifier_qualifier_list
	   : type_specifier specifier_qualifier_list
	   | type_specifier
	   | type_qualifier specifier_qualifier_list
	   | type_qualifier
	   ;

	   type_qualifier
	   : CONST
	   | VOLATILE
	   ;
	   */
	struct type_spec	type_spec;
	struct type_spec_list	* next;
};

struct pointer_spec
{
	/*
	   pointer
	   : '*'
	   | '*' type_qualifier_list
	   | '*' pointer
	   | '*' type_qualifier_list pointer
	   ;

	   type_qualifier_list
	   : type_qualifier
	   | type_qualifier_list type_qualifier
	   ;
	   */
	/* number of '* * * ...'-s in this pointer 'train'*/
	int	nr_refs;
};

struct type_name
{
	/*
	   type_name
	   : specifier_qualifier_list
	   | specifier_qualifier_list abstract_declarator
	   ;
	   */
	struct type_spec_list	* type_spec;
	struct abstract_decl	* abstract_decl;
};

struct abstract_decl
{
	/*
	   abstract_declarator
	   : pointer
	   | direct_abstract_declarator
	   | pointer direct_abstract_declarator
	   ;
	   */
	struct pointer_spec	* pointer_spec;
	struct direct_abstract_decl	* direct_abstract_decl;
};

struct direct_abstract_decl
{
	/*
	   direct_abstract_declarator
	   : '(' abstract_declarator ')'
	   | '[' ']'
	   | '[' conditional_expression ']'
	   | direct_abstract_declarator '[' ']'
	   | direct_abstract_declarator '[' conditional_expression ']'
	   | '(' ')'
	   ;
	   */
	enum DIRECT_ABSTRACT_DECL_TYPE
	{
		INVALID_DIRECT_ABSTRACT_DECL	= 0,
		DAD_PARENS,
		DAD_SQBRACKETS,
	}
	kind;
	struct abstract_decl		* abstract_decl;
	struct direct_abstract_decl	* direct_abstract_decl;
	struct cond_expr		* cond_expr;
};

#include "c-tokens.h"

#endif /* __C_PARSE_TYPES_H__ */

