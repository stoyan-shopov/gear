
%{
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "c-parse-types.h"

extern char yytext[];
extern int column;

yyerror(s)
char *s;
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}

struct expr * parse_head;

int main(int argc, char * argv[])
{
	return yyparse();
}

#define ALLOC_NODE(x)	(x *) calloc(1, sizeof(x))

%}

%defines "c-tokens.h"

%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR_T SHORT_T INT_T LONG_T SIGNED UNSIGNED FLOAT_T DOUBLE_T CONST_T VOLATILE VOID_T
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start expression
%destructor	{ parse_head = $$; } expression

%union
{
	const char	* str;
	struct { enum CONST_KIND kind; union { signed long long iconst; long double flconst; }; } const_info;
	struct primary_expr	* primary_expr;
	struct expr		* expr;
	enum EXPR_OP		assignment_op;
	struct cond_expr	* cond_expr;
	struct unary_expr	* unary_expr;
	struct cast_expr	* cast_expr;
	enum UNARY_OP		unary_op;
	struct postfix_expr	* postfix_expr;
	struct arg_expr_list	* args;
	struct logical_or_expr	* logical_or_expr;
	struct logical_and_expr	* logical_and_expr;
	struct incl_or_expr	* incl_or_expr;
	struct xor_expr		* xor_expr;
	struct and_expr		* and_expr;
	struct eq_expr		* eq_expr;
	struct rel_expr		* rel_expr;
	struct shift_expr	* shift_expr;
	struct additive_expr		* additive_expr;
	struct mult_expr	* mult_expr;


	struct type_name	* type_name;
	struct type_spec_list	* type_spec_list;
	struct type_spec	type_spec;
	struct abstract_decl	* abstract_decl;
	struct pointer_spec	* pointer_spec;
	struct direct_abstract_decl	* direct_abstract_decl;
};

%token	<str>			IDENTIFIER
%token	<const_info>		CONSTANT
%token	<str>			STRING_LITERAL
%type	<primary_expr>		primary_expression
%type	<expr>			expression
%type	<assignment_op>		assignment_operator
%type	<cond_expr>		conditional_expression
%type	<unary_expr>		unary_expression
%type	<cast_expr>		cast_expression
%type	<unary_op>		unary_operator
%type	<postfix_expr>		postfix_expression
%type	<args>			argument_expression_list
%type	<logical_or_expr>	logical_or_expression
%type	<logical_and_expr>	logical_and_expression
%type	<incl_or_expr>		inclusive_or_expression
%type	<xor_expr>		exclusive_or_expression
%type	<and_expr>		and_expression
%type	<eq_expr>		equality_expression
%type	<rel_expr>		relational_expression
%type	<shift_expr>		shift_expression
%type	<additive_expr>		additive_expression
%type	<mult_expr>		multiplicative_expression
%type	<type_name>		type_name
%type	<type_spec_list>	specifier_qualifier_list
%type	<type_spec>		type_specifier
%type	<abstract_decl>		abstract_declarator
%type	<pointer_spec>		pointer
%type	<direct_abstract_decl>		direct_abstract_declarator

%%

primary_expression
	: IDENTIFIER { $$ = ALLOC_NODE(struct primary_expr); $$->ident = strdup($1); }
	| CONSTANT { $$ = ALLOC_NODE(struct primary_expr); $$->const_kind = $1.kind; if ($1.kind == CONST_INT) $$->iconst = $1.iconst; else $$->flconst = $1.flconst; }
	| STRING_LITERAL { $$ = ALLOC_NODE(struct primary_expr); $$->str_literal = $1; }
	| '(' expression ')' { $$ = ALLOC_NODE(struct primary_expr); $$->expr = $2; }
	;

postfix_expression
	: primary_expression					{ $$ = ALLOC_NODE(struct postfix_expr); $$->primary_expr = $1; }
	| postfix_expression '[' expression ']'	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->expr = $3; $$->op = ARR_SUBSCRIPT; }
	| postfix_expression '(' ')'	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->op = FUNC_CALL; }
	| postfix_expression '(' argument_expression_list ')'	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->args = $3 ; $$->op = FUNC_CALL; }
	| postfix_expression '.' IDENTIFIER	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->ident = strdup($3) ; $$->op = STRUCT_MEMBER_ACCESS; }
	| postfix_expression PTR_OP IDENTIFIER	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->ident = strdup($3) ; $$->op = STRUCT_MEMBER_PTR_ACCESS; }
	| postfix_expression INC_OP	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->op = POSTFIX_INC_OP; }
	| postfix_expression DEC_OP	{ $$ = ALLOC_NODE(struct postfix_expr); $$->postfix_expr = $1; $$->op = POSTFIX_DEC_OP; }
	;

argument_expression_list
	: expression	{ $$ = ALLOC_NODE(struct arg_expr_list); $$->expr = $1; }
	| argument_expression_list ',' expression	{ $$ = ALLOC_NODE(struct arg_expr_list); $$->expr = $3; $$->next = $1; }
	;

unary_expression
	: postfix_expression			{ $$ = ALLOC_NODE(struct unary_expr); $$->postfix_expr = $1; }
	| INC_OP unary_expression		{ $$ = ALLOC_NODE(struct unary_expr); $$->unary_expr = $2; $$->op = PREFIX_INC_OP; }
	| DEC_OP unary_expression		{ $$ = ALLOC_NODE(struct unary_expr); $$->unary_expr = $2; $$->op = PREFIX_DEC_OP; }
	| unary_operator cast_expression	{ $$ = ALLOC_NODE(struct unary_expr); $$->cast_expr = $2; $$->op = $1; }
	| SIZEOF unary_expression		{ $$ = ALLOC_NODE(struct unary_expr); $$->unary_expr = $2; $$->op = SIZEOF_OP; }
	| SIZEOF '(' type_name ')'		{ $$ = ALLOC_NODE(struct unary_expr); $$->type_name = $3; $$->op = SIZEOF_OP; }
	;

unary_operator
	: '&'	{ $$ = UN_AMPERSAND; }
	| '*'	{ $$ = UN_ASTERISK; }
	| '+'	{ $$ = UN_PLUS; }
	| '-'	{ $$ = UN_MINUS; }
	| '~'	{ $$ = UN_TILDE; }
	| '!'	{ $$ = UN_EXCLAM; }
	;

cast_expression
	: unary_expression			{ $$ = ALLOC_NODE(struct cast_expr); $$->unary_expr = $1; }
	| '(' type_name ')' cast_expression	{ $$ = ALLOC_NODE(struct cast_expr); $$->type_name = $2; $$->cast_expr = $4; }

	;

multiplicative_expression
	: cast_expression	{ $$ = ALLOC_NODE(struct mult_expr); $$->cast_expr = $1; }
	| multiplicative_expression '*' cast_expression	{ $$ = ALLOC_NODE(struct mult_expr); $$->mult_expr = $1; $$->op = MULT_TIMES; $$->cast_expr = $3; }
	| multiplicative_expression '/' cast_expression	{ $$ = ALLOC_NODE(struct mult_expr); $$->mult_expr = $1; $$->op = MULT_DIV; $$->cast_expr = $3; }
	| multiplicative_expression '%' cast_expression	{ $$ = ALLOC_NODE(struct mult_expr); $$->mult_expr = $1; $$->op = MULT_MOD; $$->cast_expr = $3; }
	;

additive_expression
	: multiplicative_expression	{ $$ = ALLOC_NODE(struct additive_expr); $$->mult_expr = $1; }
	| additive_expression '+' multiplicative_expression	{ $$ = ALLOC_NODE(struct additive_expr); $$->op = ADDITIVE_PLUS; $$->additive_expr = $1; $$->mult_expr = $3; }
	| additive_expression '-' multiplicative_expression	{ $$ = ALLOC_NODE(struct additive_expr); $$->op = ADDITIVE_MINUS; $$->additive_expr = $1; $$->mult_expr = $3; }
	;

shift_expression
	: additive_expression	{ $$ = ALLOC_NODE(struct shift_expr); $$->additive_expr = $1; }
	| shift_expression LEFT_OP additive_expression	{ $$ = ALLOC_NODE(struct shift_expr); $$->shift_expr = $1; $$->op = SHIFT_LEFT; $$->additive_expr = $3; }
	| shift_expression RIGHT_OP additive_expression	{ $$ = ALLOC_NODE(struct shift_expr); $$->shift_expr = $1; $$->op = SHIFT_RIGHT; $$->additive_expr = $3; }
	;

relational_expression
	: shift_expression	{ $$ = ALLOC_NODE(struct rel_expr); $$->shift_expr = $1; }
	| relational_expression '<' shift_expression	{ $$ = ALLOC_NODE(struct rel_expr); $$->rel_expr = $1; $$->op = REL_LESS; $$->shift_expr = $3; }
	| relational_expression '>' shift_expression	{ $$ = ALLOC_NODE(struct rel_expr); $$->rel_expr = $1; $$->op = REL_GREATER; $$->shift_expr = $3; }
	| relational_expression LE_OP shift_expression	{ $$ = ALLOC_NODE(struct rel_expr); $$->rel_expr = $1; $$->op = REL_LESS_OR_EQUAL; $$->shift_expr = $3; }
	| relational_expression GE_OP shift_expression	{ $$ = ALLOC_NODE(struct rel_expr); $$->rel_expr = $1; $$->op = REL_GREATER_OR_EQUAL; $$->shift_expr = $3; }
	;

equality_expression
	: relational_expression		{ $$ = ALLOC_NODE(struct eq_expr); $$->rel_expr = $1; }
	| equality_expression EQ_OP relational_expression	{ $$ = ALLOC_NODE(struct eq_expr); $$->eq_expr = $1; $$->op = EQ_EQUAL; $$->rel_expr = $3; }
	| equality_expression NE_OP relational_expression	{ $$ = ALLOC_NODE(struct eq_expr); $$->eq_expr = $1; $$->op = EQ_NOT_EQUAL; $$->rel_expr = $3; }
	;

and_expression
	: equality_expression	{ $$ = ALLOC_NODE(struct and_expr); $$->eq_expr = $1; }
	| and_expression '&' equality_expression	{ $$ = ALLOC_NODE(struct and_expr); $$->and_expr = $1; $$->op = AND_OP_AMPERSAND; $$->eq_expr = $3; }
	;

exclusive_or_expression
	: and_expression	{ $$ = ALLOC_NODE(struct xor_expr); $$->and_expr = $1; }
	| exclusive_or_expression '^' and_expression	{ $$ = ALLOC_NODE(struct xor_expr); $$->xor_expr = $1; $$->op = XOR_OP; $$->and_expr = $3; }
	;

inclusive_or_expression
	: exclusive_or_expression	{ $$ = ALLOC_NODE(struct incl_or_expr); $$->xor_expr = $1; }
	| inclusive_or_expression '|' exclusive_or_expression	{ $$ = ALLOC_NODE(struct incl_or_expr); $$->incl_or_expr = $1; $$->op = INCL_OR_OP; $$->xor_expr = $3; }
	;

logical_and_expression
	: inclusive_or_expression	{ $$ = ALLOC_NODE(struct logical_and_expr); $$->incl_or_expr = $1; }
	| logical_and_expression AND_OP inclusive_or_expression	{ $$ = ALLOC_NODE(struct logical_and_expr); $$->logical_and_expr = $1; $$->op = LOGICAL_AND_OP; $$->incl_or_expr = $3; }
	;

logical_or_expression
	: logical_and_expression	{ $$ = ALLOC_NODE(struct logical_or_expr); $$->logical_and_expr = $1; }
	| logical_or_expression OR_OP logical_and_expression	{ $$ = ALLOC_NODE(struct logical_or_expr); $$->logical_or_expr = $1; $$->op = LOGICAL_OR_OP; $$->logical_and_expr = $3; }
	;

conditional_expression
	: logical_or_expression	{ $$ = ALLOC_NODE(struct cond_expr); $$->logical_or_expr = $1; }
	| logical_or_expression '?' expression ':' conditional_expression	{ $$ = ALLOC_NODE(struct cond_expr); $$->logical_or_expr = $1; $$->op = LOGICAL_TERNARY_OP; $$->expr = $3; $$->cond_expr = $5; }
	;

expression
	: conditional_expression { $$ = ALLOC_NODE(struct expr); $$->cond_expr = $1; }
	| unary_expression assignment_operator expression { $$ = ALLOC_NODE(struct expr); $$->unary_expr = $1; $$->op = $2; $$->expr = $3; }
	;

assignment_operator
	: '='		{ $$ = SIMPLE_ASSIGN; }
	| MUL_ASSIGN	{ $$ = MUL_ASSIGN_OP; }
	| DIV_ASSIGN	{ $$ = DIV_ASSIGN_OP; }
	| MOD_ASSIGN	{ $$ = MOD_ASSIGN_OP; }
	| ADD_ASSIGN	{ $$ = ADD_ASSIGN_OP; }
	| SUB_ASSIGN	{ $$ = SUB_ASSIGN_OP; }
	| LEFT_ASSIGN	{ $$ = LEFT_ASSIGN_OP; }
	| RIGHT_ASSIGN	{ $$ = RIGHT_ASSIGN_OP; }
	| AND_ASSIGN	{ $$ = AND_ASSIGN_OP; }
	| XOR_ASSIGN	{ $$ = XOR_ASSIGN_OP; }
	| OR_ASSIGN	{ $$ = OR_ASSIGN_OP; }
	;

type_specifier
	: VOID_T			{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_VOID; }
	| CHAR_T			{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_CHAR; }
	| SHORT_T			{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_SHORT; }
	| INT_T			{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_INT; }
	| LONG_T			{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_LONG; }
	| FLOAT_T			{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_FLOAT; }
	| DOUBLE_T		{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_DOUBLE; }
	| SIGNED		{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_SIGNED; }
	| UNSIGNED		{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_UNSIGNED; }
	| STRUCT IDENTIFIER	{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_STRUCT_SPEC; $$.name = strdup($2); }
	| UNION IDENTIFIER	{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_UNION_SPEC; $$.name = strdup($2); }
	| ENUM IDENTIFIER	{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_ENUM_SPEC; $$.name = strdup($2); }
	| TYPE_NAME		{ /*$$ = ALLOC_NODE(struct type_spec);*/ $$.tspec = T_TYPE_NAME; }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list	{ $$ = ALLOC_NODE(struct type_spec_list); $$->type_spec = $1; $$->next = $2; }
	| type_specifier				{ $$ = ALLOC_NODE(struct type_spec_list); $$->type_spec = $1; }
	| type_qualifier specifier_qualifier_list	{ $$ = $2; }
	| type_qualifier				{ $$ = ALLOC_NODE(struct type_spec_list); }
	;

type_qualifier
	: CONST_T
	| VOLATILE
	;

pointer
	: '*'	{ $$ = ALLOC_NODE(struct pointer_spec); $$->nr_refs = 1; }
	| '*' type_qualifier_list	{ $$ = ALLOC_NODE(struct pointer_spec); $$->nr_refs = 1; }
	| '*' pointer	{ $$->nr_refs ++; }
	| '*' type_qualifier_list pointer	{ $$->nr_refs ++; }
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;

type_name
	: specifier_qualifier_list	{ $$ = ALLOC_NODE(struct type_name); $$->type_spec = $1; }
	| specifier_qualifier_list abstract_declarator	{ $$ = ALLOC_NODE(struct type_name); $$->type_spec = $1; $$->abstract_decl = $2; }
	;

abstract_declarator
	: pointer				{ $$ = ALLOC_NODE(struct abstract_decl); $$->pointer_spec = $1; }
	| direct_abstract_declarator		{ $$ = ALLOC_NODE(struct abstract_decl); $$->direct_abstract_decl = $1; }
	| pointer direct_abstract_declarator	{ $$ = ALLOC_NODE(struct abstract_decl); $$->pointer_spec = $1; $$->direct_abstract_decl = $2; }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'	{ $$ = ALLOC_NODE(struct direct_abstract_decl); $$->kind = DAD_PARENS; $$->abstract_decl = $2; }
	| '[' ']'	{ $$ = ALLOC_NODE(struct direct_abstract_decl); $$->kind = DAD_SQBRACKETS; }
	| '[' conditional_expression ']'	{ $$ = ALLOC_NODE(struct direct_abstract_decl); $$->kind = DAD_SQBRACKETS; }
	| direct_abstract_declarator '[' ']'	{ $$ = ALLOC_NODE(struct direct_abstract_decl); $$->kind = DAD_SQBRACKETS; }
	| direct_abstract_declarator '[' conditional_expression ']'	{ $$ = ALLOC_NODE(struct direct_abstract_decl); $$->kind = DAD_SQBRACKETS; }
	| '(' ')'	{ $$ = ALLOC_NODE(struct direct_abstract_decl); $$->kind = DAD_PARENS; }
	;

%%
