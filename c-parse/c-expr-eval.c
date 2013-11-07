#include <stdio.h>
#include <stdlib.h>

#include "c-parse-types.h"

#include "type-access.h"

#define panic(msg) do { printf("%s:%i\t%s():\t%s\n", __FILE__, __LINE__, __func__, msg); exit(1); } while(0)

static struct gear_engine_context * ctx;
typedef uint32_t CORE_WORD;

struct eval_node
{
	enum
	{
		OBJ_INVALID	= 0,
		OBJ_DTYPE,
		OBJ_SUBPROGRAM,
		OBJ_DOBJ,
	}
	objkind;
	union
	{
		struct dtype_data dtype;
		struct dobj_data dobj;
		struct subprogram_data subp;
	};
	struct
	{
		bool	is_addr_applicable	: 1;
		bool	is_val_applicable	: 1;
		bool	is_addr_computed	: 1;
		bool	is_val_computed		: 1;
	};
	uint32_t	addr;
	uint8_t		* data;
	union
	{
		int8_t		i8;
		uint8_t		u8;
		int16_t		i16;
		uint16_t	u16;
		int32_t		i32;
		uint32_t	u32;
		int64_t		i64;
		uint64_t	u64;
		float		f;
		double		d;
		long double	ld;
	};

};


/*
 * 
 * expression value evaluation routine prototypes
 *
 */

static struct eval_node eval_primary_expr(struct primary_expr * p);
static struct eval_node eval_postfix_expr(struct postfix_expr * p);
static struct eval_node eval_arg_expr_list(struct arg_expr_list * p);
static struct eval_node eval_unary_expr(struct unary_expr * p);
static struct eval_node eval_cast_expr(struct cast_expr * p);
static struct eval_node eval_mult_expr(struct mult_expr * p);
static struct eval_node eval_additive_expr(struct additive_expr * p);
static struct eval_node eval_shift_expr(struct shift_expr * p);
static struct eval_node eval_rel_expr(struct rel_expr * p);
static struct eval_node eval_eq_expr(struct eq_expr * p);
static struct eval_node eval_and_expr(struct and_expr * p);
static struct eval_node eval_xor_expr(struct xor_expr * p);
static struct eval_node eval_incl_or_expr(struct incl_or_expr * p);
static struct eval_node eval_logical_and_expr(struct logical_and_expr * p);
static struct eval_node eval_logical_or_expr(struct logical_or_expr * p);
static struct eval_node eval_cond_expr(struct cond_expr * p);
static struct eval_node eval_expr(struct expr * p);
static struct eval_node eval_type_spec(struct type_spec * p);
static struct eval_node eval_type_spec_list(struct type_spec_list * p);
static struct eval_node eval_pointer_spec(struct pointer_spec * p);
static struct eval_node eval_type_name(struct type_name * p);
static struct eval_node eval_abstract_decl(struct abstract_decl * p);
static struct eval_node eval_direct_abstract_decl(struct direct_abstract_decl * p);;


/*
 * 
 * expression type evaluation routine prototypes
 *
 */

static struct eval_node eval_type_primary_expr(struct primary_expr * p);
static struct eval_node eval_type_postfix_expr(struct postfix_expr * p);
static struct eval_node eval_type_arg_expr_list(struct arg_expr_list * p);
static struct eval_node eval_type_unary_expr(struct unary_expr * p);
static struct eval_node eval_type_cast_expr(struct cast_expr * p);
static struct eval_node eval_type_mult_expr(struct mult_expr * p);
static struct eval_node eval_type_additive_expr(struct additive_expr * p);
static struct eval_node eval_type_shift_expr(struct shift_expr * p);
static struct eval_node eval_type_rel_expr(struct rel_expr * p);
static struct eval_node eval_type_eq_expr(struct eq_expr * p);
static struct eval_node eval_type_and_expr(struct and_expr * p);
static struct eval_node eval_type_xor_expr(struct xor_expr * p);
static struct eval_node eval_type_incl_or_expr(struct incl_or_expr * p);
static struct eval_node eval_type_logical_and_expr(struct logical_and_expr * p);
static struct eval_node eval_type_logical_or_expr(struct logical_or_expr * p);
static struct eval_node eval_type_cond_expr(struct cond_expr * p);
static struct eval_node eval_type_expr(struct expr * p);
static struct eval_node eval_type_type_spec(struct type_spec * p);
static struct eval_node eval_type_type_spec_list(struct type_spec_list * p);
static struct eval_node eval_type_pointer_spec(struct pointer_spec * p);
static struct eval_node eval_type_type_name(struct type_name * p);
static struct eval_node eval_type_abstract_decl(struct abstract_decl * p);
static struct eval_node eval_type_direct_abstract_decl(struct direct_abstract_decl * p);;


/*
 * 
 * expression value evaluation routines
 *
 */


static struct eval_node eval_primary_expr(struct primary_expr * p)
{
	/*
	   primary_expression
	   : IDENTIFIER
	   | CONSTANT
	   | STRING_LITERAL
	   | '(' expression ')'
	   ;
	   */
	struct eval_node x;
	if (p->ident)
		x.x = 0xdaeba;
	else if (p->str_literal)
		x.x = 0xdeadbeef;
	else if (p->expr)
		x = eval_expr(p->expr);
	else
		x.x = p->constant;

	return x;
}

static struct eval_node eval_postfix_expr(struct postfix_expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_POSTFIX_OP:
			x = eval_primary_expr(p->primary_expr);
			break;
		case ARR_SUBSCRIPT:
		case FUNC_CALL:
		case STRUCT_MEMBER_ACCESS:
		case STRUCT_MEMBER_PTR_ACCESS:
		case POSTFIX_INC_OP:
		case POSTFIX_DEC_OP:
			panic("");
	}

	return x;
}

static struct eval_node eval_arg_expr_list(struct arg_expr_list * p)
{
	/*
	   argument_expression_list
	   : expression
	   | argument_expression_list ',' expression
	   ;
	   */
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_unary_expr(struct unary_expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_POSTFIX_OP:
			x = eval_postfix_expr(p->postfix_expr);
			break;
		case ARR_SUBSCRIPT:
		case FUNC_CALL:
		case STRUCT_MEMBER_ACCESS:
		case STRUCT_MEMBER_PTR_ACCESS:
		case POSTFIX_INC_OP:
		case POSTFIX_DEC_OP:
			panic("");
			break;
	}

	return x;
}

static struct eval_node eval_cast_expr(struct cast_expr * p)
{
	/*
	   cast_expression
	   : unary_expression
	   | '(' type_name ')' cast_expression
	   ;
	   */
	struct eval_node x;
	if (p->unary_expr)
		x = eval_unary_expr(p->unary_expr);
	else
		panic("");

	return x;
}

static struct eval_node eval_mult_expr(struct mult_expr * p)
{
	/*
	   multiplicative_expression
	   : cast_expression
	   | multiplicative_expression '*' cast_expression
	   | multiplicative_expression '/' cast_expression
	   | multiplicative_expression '%' cast_expression
	   ;
	   */
	struct eval_node x;
	switch (p->op)
	{
		default:
		case INVALID_MULT_OP:
			x = eval_cast_expr(p->cast_expr);
			break;
		case MULT_TIMES:
			x.x = eval_mult_expr(p->mult_expr).x * eval_cast_expr(p->cast_expr).x;
			break;
		case MULT_DIV:
			x.x = eval_mult_expr(p->mult_expr).x / eval_cast_expr(p->cast_expr).x;
			break;
		case MULT_MOD:
			x.x = eval_mult_expr(p->mult_expr).x % eval_cast_expr(p->cast_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_additive_expr(struct additive_expr * p)
{
	struct eval_node x;
	/*
	   additive_expression
	   : multiplicative_expression
	   | additive_expression '+' multiplicative_expression
	   | additive_expression '-' multiplicative_expression
	   ;
	   */
	switch (p->op)
	{
		default:
		case INVALID_ADDITIVE_OP:
			x = eval_mult_expr(p->mult_expr);
			break;
		case ADDITIVE_PLUS:
			x.x = eval_additive_expr(p->additive_expr).x + eval_mult_expr(p->mult_expr).x;
			break;
		case ADDITIVE_MINUS:
			x.x = eval_additive_expr(p->additive_expr).x - eval_mult_expr(p->mult_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_shift_expr(struct shift_expr * p)
{
	/*
	   shift_expression
	   : additive_expression
	   | shift_expression LEFT_OP additive_expression
	   | shift_expression RIGHT_OP additive_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_SHIFT_OP:
			x = eval_additive_expr(p->additive_expr);
			break;
		case SHIFT_LEFT:
			x.x = eval_shift_expr(p->shift_expr).x << eval_additive_expr(p->additive_expr).x;
			break;
		case SHIFT_RIGHT:
			x.x = eval_shift_expr(p->shift_expr).x >> eval_additive_expr(p->additive_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_rel_expr(struct rel_expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_REL_OP:
			x = eval_shift_expr(p->shift_expr);
			break;
		case REL_LESS:
			x.x = (eval_rel_expr(p->rel_expr).x < eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
		case REL_GREATER:
			x.x = (eval_rel_expr(p->rel_expr).x > eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
		case REL_LESS_OR_EQUAL:
			x.x = (eval_rel_expr(p->rel_expr).x <= eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
		case REL_GREATER_OR_EQUAL:
			x.x = (eval_rel_expr(p->rel_expr).x >= eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
	}

	return x;
}

static struct eval_node eval_eq_expr(struct eq_expr * p)
{
	/*
	   equality_expression
	   : relational_expression
	   | equality_expression EQ_OP relational_expression
	   | equality_expression NE_OP relational_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_EQ_OP:
			x = eval_rel_expr(p->rel_expr);
			break;
		case EQ_EQUAL:
			x.x = (eval_eq_expr(p->eq_expr).x == eval_rel_expr(p->rel_expr).x) ? 1 : 0;
			break;
		case EQ_NOT_EQUAL:
			x.x = (eval_eq_expr(p->eq_expr).x != eval_rel_expr(p->rel_expr).x) ? 1 : 0;
			break;
	}

	return x;
}

static struct eval_node eval_and_expr(struct and_expr * p)
{
	/*
	   and_expression
	   : equality_expression
	   | and_expression '&' equality_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_AND_OP:
			x = eval_eq_expr(p->eq_expr);
			break;
		case AND_OP_AMPERSAND:
			x.x = eval_and_expr(p->and_expr).x & eval_eq_expr(p->eq_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_xor_expr(struct xor_expr * p)
{
	/*
	   exclusive_or_expression
	   : and_expression
	   | exclusive_or_expression '^' and_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_XOR_OP:
			x = eval_and_expr(p->and_expr);
			break;
		case XOR_OP:
			x.x = eval_xor_expr(p->xor_expr).x ^ eval_and_expr(p->and_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_incl_or_expr(struct incl_or_expr * p)
{
	/*
	   inclusive_or_expression
	   : exclusive_or_expression
	   | inclusive_or_expression '|' exclusive_or_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_INCL_OR_OP:
			x = eval_xor_expr(p->xor_expr);
			break;
		case INCL_OR_OP:
			x.x = eval_incl_or_expr(p->incl_or_expr).x | eval_xor_expr(p->xor_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_logical_and_expr(struct logical_and_expr * p)
{
	/*
	   logical_and_expression
	   : inclusive_or_expression
	   | logical_and_expression AND_OP inclusive_or_expression
	   ;
	   */

	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_LOGICAL_AND_OP:
			x = eval_incl_or_expr(p->incl_or_expr);
			break;
		case LOGICAL_AND_OP:
			x.x = eval_logical_and_expr(p->logical_and_expr).x && eval_incl_or_expr(p->incl_or_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_logical_or_expr(struct logical_or_expr * p)
{
	/*
	   logical_or_expression
	   : logical_and_expression
	   | logical_or_expression OR_OP logical_and_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_LOGICAL_OR_OP:
			x = eval_logical_and_expr(p->logical_and_expr);
			break;
		case LOGICAL_OR_OP:
			x.x = eval_logical_or_expr(p->logical_or_expr).x && eval_logical_and_expr(p->logical_and_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_cond_expr(struct cond_expr * p)
{
	/*
	   conditional_expression
	   : logical_or_expression
	   | logical_or_expression '?' expression ':' conditional_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_COND_EXPR_OP:
			x = eval_logical_or_expr(p->logical_or_expr);
			break;
		case LOGICAL_OR_OP:
			x.x = eval_logical_or_expr(p->logical_or_expr).x ? eval_expr(p->expr).x : eval_cond_expr(p->cond_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_expr(struct expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_EXPR_OP:
			x = eval_cond_expr(p->cond_expr);
			break;
		case SIMPLE_ASSIGN:
		case MUL_ASSIGN_OP:
		case DIV_ASSIGN_OP:
		case MOD_ASSIGN_OP:
		case ADD_ASSIGN_OP:
		case SUB_ASSIGN_OP:
		case LEFT_ASSIGN_OP:
		case RIGHT_ASSIGN_OP:
		case AND_ASSIGN_OP:
		case XOR_ASSIGN_OP:
		case OR_ASSIGN_OP:
			panic("");
	}


	return x;
}

static struct eval_node eval_type_spec(struct type_spec * p)
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
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_spec_list(struct type_spec_list * p)
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
	struct eval_node x;
	
	panic("");

	return x;
}

static struct eval_node eval_pointer_spec(struct pointer_spec * p)
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
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_name(struct type_name * p)
{
	/*
	   type_name
	   : specifier_qualifier_list
	   | specifier_qualifier_list abstract_declarator
	   ;
	   */
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_abstract_decl(struct abstract_decl * p)
{
	/*
	   abstract_declarator
	   : pointer
	   | direct_abstract_declarator
	   | pointer direct_abstract_declarator
	   ;
	   */
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_direct_abstract_decl(struct direct_abstract_decl * p)
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
	struct eval_node x;

	panic("");

	return x;
}


/*
 * 
 * expression value evaluation routines
 *
 */
static struct eval_node eval_type_primary_expr(struct primary_expr * p)
{
	/*
	   primary_expression
	   : IDENTIFIER
	   | CONSTANT
	   | STRING_LITERAL
	   | '(' expression ')'
	   ;
	   */
	struct eval_node x;
	if (p->ident)
		x.x = 0xdaeba;
	else if (p->str_literal)
		x.x = 0xdeadbeef;
	else if (p->expr)
		x = eval_expr(p->expr);
	else
		x.x = p->constant;

	return x;
}

static struct eval_node eval_type_postfix_expr(struct postfix_expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_POSTFIX_OP:
			x = eval_primary_expr(p->primary_expr);
			break;
		case ARR_SUBSCRIPT:
		case FUNC_CALL:
		case STRUCT_MEMBER_ACCESS:
		case STRUCT_MEMBER_PTR_ACCESS:
		case POSTFIX_INC_OP:
		case POSTFIX_DEC_OP:
			panic("");
	}

	return x;
}

static struct eval_node eval_type_arg_expr_list(struct arg_expr_list * p)
{
	/*
	   argument_expression_list
	   : expression
	   | argument_expression_list ',' expression
	   ;
	   */
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_unary_expr(struct unary_expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_POSTFIX_OP:
			x = eval_postfix_expr(p->postfix_expr);
			break;
		case ARR_SUBSCRIPT:
		case FUNC_CALL:
		case STRUCT_MEMBER_ACCESS:
		case STRUCT_MEMBER_PTR_ACCESS:
		case POSTFIX_INC_OP:
		case POSTFIX_DEC_OP:
			panic("");
			break;
	}

	return x;
}

static struct eval_node eval_type_cast_expr(struct cast_expr * p)
{
	/*
	   cast_expression
	   : unary_expression
	   | '(' type_name ')' cast_expression
	   ;
	   */
	struct eval_node x;
	if (p->unary_expr)
		x = eval_unary_expr(p->unary_expr);
	else
		panic("");

	return x;
}

static struct eval_node eval_type_mult_expr(struct mult_expr * p)
{
	/*
	   multiplicative_expression
	   : cast_expression
	   | multiplicative_expression '*' cast_expression
	   | multiplicative_expression '/' cast_expression
	   | multiplicative_expression '%' cast_expression
	   ;
	   */
	struct eval_node x;
	switch (p->op)
	{
		default:
		case INVALID_MULT_OP:
			x = eval_cast_expr(p->cast_expr);
			break;
		case MULT_TIMES:
			x.x = eval_mult_expr(p->mult_expr).x * eval_cast_expr(p->cast_expr).x;
			break;
		case MULT_DIV:
			x.x = eval_mult_expr(p->mult_expr).x / eval_cast_expr(p->cast_expr).x;
			break;
		case MULT_MOD:
			x.x = eval_mult_expr(p->mult_expr).x % eval_cast_expr(p->cast_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_additive_expr(struct additive_expr * p)
{
	struct eval_node x;
	/*
	   additive_expression
	   : multiplicative_expression
	   | additive_expression '+' multiplicative_expression
	   | additive_expression '-' multiplicative_expression
	   ;
	   */
	switch (p->op)
	{
		default:
		case INVALID_ADDITIVE_OP:
			x = eval_mult_expr(p->mult_expr);
			break;
		case ADDITIVE_PLUS:
			x.x = eval_additive_expr(p->additive_expr).x + eval_mult_expr(p->mult_expr).x;
			break;
		case ADDITIVE_MINUS:
			x.x = eval_additive_expr(p->additive_expr).x - eval_mult_expr(p->mult_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_shift_expr(struct shift_expr * p)
{
	/*
	   shift_expression
	   : additive_expression
	   | shift_expression LEFT_OP additive_expression
	   | shift_expression RIGHT_OP additive_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_SHIFT_OP:
			x = eval_additive_expr(p->additive_expr);
			break;
		case SHIFT_LEFT:
			x.x = eval_shift_expr(p->shift_expr).x << eval_additive_expr(p->additive_expr).x;
			break;
		case SHIFT_RIGHT:
			x.x = eval_shift_expr(p->shift_expr).x >> eval_additive_expr(p->additive_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_rel_expr(struct rel_expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_REL_OP:
			x = eval_shift_expr(p->shift_expr);
			break;
		case REL_LESS:
			x.x = (eval_rel_expr(p->rel_expr).x < eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
		case REL_GREATER:
			x.x = (eval_rel_expr(p->rel_expr).x > eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
		case REL_LESS_OR_EQUAL:
			x.x = (eval_rel_expr(p->rel_expr).x <= eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
		case REL_GREATER_OR_EQUAL:
			x.x = (eval_rel_expr(p->rel_expr).x >= eval_shift_expr(p->shift_expr).x) ? 1 : 0;
			break;
	}

	return x;
}

static struct eval_node eval_type_eq_expr(struct eq_expr * p)
{
	/*
	   equality_expression
	   : relational_expression
	   | equality_expression EQ_OP relational_expression
	   | equality_expression NE_OP relational_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_EQ_OP:
			x = eval_rel_expr(p->rel_expr);
			break;
		case EQ_EQUAL:
			x.x = (eval_eq_expr(p->eq_expr).x == eval_rel_expr(p->rel_expr).x) ? 1 : 0;
			break;
		case EQ_NOT_EQUAL:
			x.x = (eval_eq_expr(p->eq_expr).x != eval_rel_expr(p->rel_expr).x) ? 1 : 0;
			break;
	}

	return x;
}

static struct eval_node eval_type_and_expr(struct and_expr * p)
{
	/*
	   and_expression
	   : equality_expression
	   | and_expression '&' equality_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_AND_OP:
			x = eval_eq_expr(p->eq_expr);
			break;
		case AND_OP_AMPERSAND:
			x.x = eval_and_expr(p->and_expr).x & eval_eq_expr(p->eq_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_xor_expr(struct xor_expr * p)
{
	/*
	   exclusive_or_expression
	   : and_expression
	   | exclusive_or_expression '^' and_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_XOR_OP:
			x = eval_and_expr(p->and_expr);
			break;
		case XOR_OP:
			x.x = eval_xor_expr(p->xor_expr).x ^ eval_and_expr(p->and_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_incl_or_expr(struct incl_or_expr * p)
{
	/*
	   inclusive_or_expression
	   : exclusive_or_expression
	   | inclusive_or_expression '|' exclusive_or_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_INCL_OR_OP:
			x = eval_xor_expr(p->xor_expr);
			break;
		case INCL_OR_OP:
			x.x = eval_incl_or_expr(p->incl_or_expr).x | eval_xor_expr(p->xor_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_logical_and_expr(struct logical_and_expr * p)
{
	/*
	   logical_and_expression
	   : inclusive_or_expression
	   | logical_and_expression AND_OP inclusive_or_expression
	   ;
	   */

	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_LOGICAL_AND_OP:
			x = eval_incl_or_expr(p->incl_or_expr);
			break;
		case LOGICAL_AND_OP:
			x.x = eval_logical_and_expr(p->logical_and_expr).x && eval_incl_or_expr(p->incl_or_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_logical_or_expr(struct logical_or_expr * p)
{
	/*
	   logical_or_expression
	   : logical_and_expression
	   | logical_or_expression OR_OP logical_and_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_LOGICAL_OR_OP:
			x = eval_logical_and_expr(p->logical_and_expr);
			break;
		case LOGICAL_OR_OP:
			x.x = eval_logical_or_expr(p->logical_or_expr).x && eval_logical_and_expr(p->logical_and_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_cond_expr(struct cond_expr * p)
{
	/*
	   conditional_expression
	   : logical_or_expression
	   | logical_or_expression '?' expression ':' conditional_expression
	   ;
	   */
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_COND_EXPR_OP:
			x = eval_logical_or_expr(p->logical_or_expr);
			break;
		case LOGICAL_OR_OP:
			x.x = eval_logical_or_expr(p->logical_or_expr).x ? eval_expr(p->expr).x : eval_cond_expr(p->cond_expr).x;
			break;
	}

	return x;
}

static struct eval_node eval_type_expr(struct expr * p)
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
	struct eval_node x;

	switch (p->op)
	{
		default:
		case INVALID_EXPR_OP:
			x = eval_cond_expr(p->cond_expr);
			break;
		case SIMPLE_ASSIGN:
		case MUL_ASSIGN_OP:
		case DIV_ASSIGN_OP:
		case MOD_ASSIGN_OP:
		case ADD_ASSIGN_OP:
		case SUB_ASSIGN_OP:
		case LEFT_ASSIGN_OP:
		case RIGHT_ASSIGN_OP:
		case AND_ASSIGN_OP:
		case XOR_ASSIGN_OP:
		case OR_ASSIGN_OP:
			panic("");
	}


	return x;
}

static struct eval_node eval_type_type_spec(struct type_spec * p)
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
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_type_spec_list(struct type_spec_list * p)
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
	struct eval_node x;
	
	panic("");

	return x;
}

static struct eval_node eval_type_pointer_spec(struct pointer_spec * p)
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
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_type_name(struct type_name * p)
{
	/*
	   type_name
	   : specifier_qualifier_list
	   | specifier_qualifier_list abstract_declarator
	   ;
	   */
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_abstract_decl(struct abstract_decl * p)
{
	/*
	   abstract_declarator
	   : pointer
	   | direct_abstract_declarator
	   | pointer direct_abstract_declarator
	   ;
	   */
	struct eval_node x;

	panic("");

	return x;
}

static struct eval_node eval_type_direct_abstract_decl(struct direct_abstract_decl * p)
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
	struct eval_node x;

	panic("");

	return x;
}
















































/*!
 *	\fn	static enum GEAR_ENGINE_ERR_ENUM resolve_id(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	resolves the location of an identifier used in an expression
 *
 *	this function resolves the type and location of an identifier used
 *	in an expression (properly initializes the type and location fields
 *	of the expression data node passed as a parameter)
 *	by scanning the debug information symbol tables taking scope
 *	in account
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
 *	\fn	static enum GEAR_ENGINE_ERR_ENUM resolve_id(struct parser_state * state, struct exp_data_node * exp_node)
 *	\brief	resolves the location of an identifier used in an expression
 *
 *	this function resolves the type and location of an identifier used
 *	in an expression (properly initializes the type and location fields
 *	of the expression data node passed as a parameter)
 *	by scanning the debug information symbol tables taking scope
 *	in account
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
static struct eval_node resolve_id(const char * id)
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
struct eval_node x;

	memset(&x, 0, sizeof x);

	/*! \todo	set flags properly here */
	dwhead = deprecated_scope_locate_dobj(ctx, id, (struct scope_resolution_flags) { 0 } );
	if (!dwhead)
	{
		sabort("identifier not found");
	}
	/* determine and properly handle the type of the symbol just found */
	switch (dwarf_util_get_tag_category(dwhead->tag))
	{
		case DWARF_TAG_CATEGORY_SUBPROGRAM:
			x.objkind = OBJ_SUBPROGRAM;
			x.subp = * (struct subprogram_data *) dwhead;
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

void c_expr_eval(const char * expr, struct gear_engine_context * ctx)
{

}


resolve_id - locate type, subprogram, or data object; if found - determine if an address and a value are applicable; compute the address, but do not fetch the value
fetch_data - check that address is available, fetch data, take basic types in account
