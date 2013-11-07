
//%skeleton "lalr1.cc"
%debug
%name-prefix="clang_parser_"
%locations
%pure-parser
%error-verbose
%parse-param	{ void * x }
%lex-param	{ void * x }

%initial-action
{
	yydebug = 0;
}

%{


#include <stdlib.h>
#include <stdio.h>
#include "clang.h"
#if __CXX__
using namespace std;
#include <iostream>
#else
#include "parser.tab.h"
#endif

static struct xlat_unit_parse_node * get_xlat_node(void)
{
	return (struct xlat_unit_parse_node *) calloc(1, sizeof * get_xlat_node());
}

#define MAX_LAST_LIST_NODE_STACK_DEPTH		64
/*! \todo	document these */
/*! \todo	stack is empty ascending - maybe change this */
static struct xlat_unit_parse_node * last_list_node_stack[MAX_LAST_LIST_NODE_STACK_DEPTH];
static int last_list_node_stack_ptr = 0;

/* returns 0 on error */
static struct xlat_unit_parse_node * merge_decl_specs(struct xlat_unit_parse_node * n1, struct xlat_unit_parse_node * n2);

/*static */struct xlat_unit_parse_node * parse_head;
static void yyerror(const YYLTYPE * loc, void * x, const char * msg);
%}

%union
{
	struct xlat_unit_parse_node	* parse_node;
	const char * ident;
	const char * string_literal;
	/*const void * const_data;*/
	long int const_val;
	enum EXPR_NODE_ENUM	expr_detail;
}

%{
#include "clang-parser-common.h"
%}

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type <parse_node> compound_statement declaration_list statement_list
%type <parse_node> expression_statement selection_statement labeled_statement
%type <parse_node> iteration_statement jump_statement translation_unit
%type <parse_node> external_declaration function_definition
%type <parse_node> declaration statement expression
%type <parse_node> primary_expression postfix_expression
%type <parse_node> argument_expression_list assignment_expression
%type <parse_node> conditional_expression logical_or_expression
%type <parse_node> logical_and_expression inclusive_or_expression
%type <parse_node> exclusive_or_expression and_expression
%type <parse_node> equality_expression relational_expression
%type <parse_node> shift_expression additive_expression
%type <parse_node> multiplicative_expression cast_expression
%type <parse_node> unary_expression
%type <parse_node> type_name

%type <parse_node> storage_class_specifier type_specifier type_qualifier
%type <parse_node> declaration_specifiers specifier_qualifier_list
%type <parse_node> type_qualifier_list
%type <parse_node> direct_declarator declarator init_declarator
%type <parse_node> init_declarator_list initializer

%type <expr_detail>	assignment_operator unary_operator
%type <ident>	IDENTIFIER
%type <string_literal>	STRING_LITERAL
%type <const_val>	CONSTANT

%start translation_unit
%%

primary_expression
	: IDENTIFIER
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_IDENT;
			$$->expr_node.id = $1;
		}
	| CONSTANT
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_CONSTANT;
			$$->expr_node.const_val = $1;
		}
	| STRING_LITERAL
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_STR_LITERAL;
			$$->expr_node.string_literal = $1;
		}
	| '(' expression ')'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_PARENTHESIZED_EXPR;
			$$->expr_node.expr[0] = $2;
		}
	;

postfix_expression
	: primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_ARR_SUBSCRIPT;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| postfix_expression '(' ')'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_FUNCTION_CALL;
			$$->expr_node.expr[0] = $1;
		}
	| postfix_expression '(' argument_expression_list ')'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_FUNCTION_CALL;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		}
	| postfix_expression '.' IDENTIFIER
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_DOT_MEMBER_SELECT;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.id = $3;
		}
	| postfix_expression PTR_OP IDENTIFIER
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_PTR_MEMBER_SELECT;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.id = $3;
		}
	| postfix_expression INC_OP
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_POST_INC;
			$$->expr_node.expr[0] = $1;
		}
	| postfix_expression DEC_OP
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_POST_DEC;
			$$->expr_node.expr[0] = $1;
		}
	;

argument_expression_list
	: assignment_expression
		{
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			$$ = last_list_node_stack[last_list_node_stack_ptr++] = $1;
		}
	| argument_expression_list ',' assignment_expression
		{
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= $3;
			$$ = $1;
		}
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| INC_OP unary_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_PRE_INC;
			$$->expr_node.expr[0] = $2;
		}
	| DEC_OP unary_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_PRE_DEC;
			$$->expr_node.expr[0] = $2;
		}
	| unary_operator cast_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = $1;
			$$->expr_node.expr[0] = $2;
		}
	| SIZEOF unary_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_SIZEOF;
			$$->expr_node.expr[0] = $2;
		}
	| SIZEOF '(' type_name ')'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_SIZEOF;
			$$->expr_node.type_name = $3;
		}
	;

unary_operator
	: '&'	{ $$ = EXPR_NODE_UNARY_AMPERSAND; }
	| '*'	{ $$ = EXPR_NODE_UNARY_INDIRECT; }
	| '+'	{ $$ = EXPR_NODE_UNARY_PLUS; }
	| '-'	{ $$ = EXPR_NODE_UNARY_MINUS; }
	| '~'	{ $$ = EXPR_NODE_UNARY_BITWISE_NOT; }
	| '!'	{ $$ = EXPR_NODE_UNARY_LOGICAL_NOT; }
	;

cast_expression
	: unary_expression { $$ = $1; }
	| '(' type_name ')' cast_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_MUL;
			$$->expr_node.expr[0] = $4;
			$$->expr_node.type_name = $2;
		}
	;

multiplicative_expression
	: cast_expression { $$ = $1; }
	| multiplicative_expression '*' cast_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_MUL;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| multiplicative_expression '/' cast_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_DIV;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| multiplicative_expression '%' cast_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_MOD;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_ADD;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| additive_expression '-' multiplicative_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_SUB;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

shift_expression
	: additive_expression { $$ = $1 }
	| shift_expression LEFT_OP additive_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_LEFT_SHIFT;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| shift_expression RIGHT_OP additive_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_RIGHT_SHIFT;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_LESS;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| relational_expression '>' shift_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_GREATER;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| relational_expression LE_OP shift_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_LESS_OR_EQUAL;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| relational_expression GE_OP shift_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_GREATER_OR_EQUAL;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_EQUAL;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	| equality_expression NE_OP relational_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_NOT_EQUAL;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

and_expression
	: equality_expression { $$ = $1; }
	| and_expression '&' equality_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_BITWISE_AND;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression '^' and_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_XOR;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

inclusive_or_expression
	: exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_BITWISE_OR;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_LOGICAL_AND;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_LOGICAL_OR;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = $2;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

assignment_operator
	: '='		{ $$ = EXPR_NODE_ASSIGN; }
	| MUL_ASSIGN	{ $$ = EXPR_NODE_MUL_ASSIGN; }
	| DIV_ASSIGN	{ $$ = EXPR_NODE_DIV_ASSIGN; }
	| MOD_ASSIGN	{ $$ = EXPR_NODE_MOD_ASSIGN; }
	| ADD_ASSIGN	{ $$ = EXPR_NODE_ADD_ASSIGN; }
	| SUB_ASSIGN	{ $$ = EXPR_NODE_SUB_ASSIGN; }
	| LEFT_ASSIGN	{ $$ = EXPR_NODE_LEFT_ASSIGN; }
	| RIGHT_ASSIGN	{ $$ = EXPR_NODE_RIGHT_ASSIGN; }
	| AND_ASSIGN	{ $$ = EXPR_NODE_AND_ASSIGN; }
	| XOR_ASSIGN	{ $$ = EXPR_NODE_XOR_ASSIGN; }
	| OR_ASSIGN	{ $$ = EXPR_NODE_OR_ASSIGN; }
	;

expression
	: assignment_expression { $$ = $1; }
	| expression ',' assignment_expression
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			$$->expr_node.type = EXPR_NODE_COMMA;
			$$->expr_node.expr[0] = $1;
			$$->expr_node.expr[1] = $3;
		}
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
		{
			/* this is really useful only for defining
			 * a new structure/union/enumeration */
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECLARATION;
			$$->declaration.decl_spec = $1;
		}
	| declaration_specifiers init_declarator_list ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECLARATION;
			$$->declaration.decl_spec = $1;
			$$->declaration.declarator_list = $2;
		}
	;

declaration_specifiers
	: storage_class_specifier { $$ = $1; }
	| declaration_specifiers storage_class_specifier { if (!($$ = merge_decl_specs($1, $2))) panic("abort the parse here (YYERROR)\n"); }
	| type_specifier { $$ = $1; }
	| declaration_specifiers type_specifier { if (!($$ = merge_decl_specs($1, $2))) panic("abort the parse here (YYERROR)\n"); }
	| type_qualifier { $$ = $1; }
	| declaration_specifiers type_qualifier { if (!($$ = merge_decl_specs($1, $2))) panic("abort the parse here (YYERROR)\n"); }
	;

init_declarator_list
	: init_declarator { $$ = $1; }
	| init_declarator_list ',' init_declarator { panic(""); }
	;

init_declarator
	: declarator { $$ = $1; }
	| declarator '=' initializer { $$ = $1; $$->declarator.initializer = $3; }
	;

storage_class_specifier
	: TYPEDEF { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_typedef = true; }
	| EXTERN { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_extern = true; }
	| STATIC { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_static = true; }
	| AUTO { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_auto = true; }
	| REGISTER { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_register = true; }
	;

type_specifier
	: VOID { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_void = true; }
	| CHAR { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_char = true; }
	| SHORT { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_short = true; }
	| INT { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_int = true; }
	| LONG { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_long = true; }
	| FLOAT { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_float = true; }
	| DOUBLE { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_double = true; }
	| SIGNED { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_signed = true; }
	| UNSIGNED { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_unsigned = true; }
	| struct_or_union_specifier { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_struct_or_union_spec = true; }
	| enum_specifier { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_enum_spec = true; }
	| TYPE_NAME { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_type_name = true; }
	;

type_qualifier
	: CONST { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_const = true; }
	| VOLATILE { ($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; $$->decl_spec.is_volatile = true; }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: specifier_qualifier_list type_specifier { if (!($$ = merge_decl_specs($1, $2))) panic("abort the parse here (YYERROR)\n"); }
	| type_specifier { $$ = $1; }
	| specifier_qualifier_list type_qualifier { if (!($$ = merge_decl_specs($1, $2))) panic("abort the parse here (YYERROR)\n"); }
	| type_qualifier { $$ = $1; }
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

declarator
	: pointer direct_declarator { panic(""); }
	| direct_declarator { $$ = $1 }
	;

direct_declarator
	: IDENTIFIER
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECLARATOR;
			$$->declarator.type = DECLARATOR_TYPE_IDENT;
			$$->declarator.id = $1;
		}
	| '(' declarator ')' { panic(""); }
	| direct_declarator '[' constant_expression ']' { panic(""); }
	| direct_declarator '[' ']' { panic(""); }
	| direct_declarator '(' parameter_type_list ')' { panic(""); }
	| direct_declarator '(' identifier_list ')' { panic(""); }
	| direct_declarator '(' ')' { panic(""); }
	;

pointer
	: '*'
	| '*' type_qualifier_list
	| '*' pointer
	| '*' type_qualifier_list pointer
	;

type_qualifier_list
	: type_qualifier { $$ = $1; }
	| type_qualifier_list type_qualifier { if (!($$ = merge_decl_specs($1, $2))) panic("abort the parse here (YYERROR)\n"); }
	;


parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list { panic(""); }
	| specifier_qualifier_list abstract_declarator { panic(""); }
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
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expression
	| '{' initializer_list '}' { panic(""); }
	| '{' initializer_list ',' '}' { panic(""); }
	;

initializer_list
	: initializer
	| initializer_list ',' initializer
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement { $$ = 0; }
	| CASE constant_expression ':' statement { $$ = 0; }
	| DEFAULT ':' statement { $$ = 0; }
	;

compound_statement
	: '{' '}'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
		}
	| '{' statement_list '}'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			$$->compound_stmt.stmt_list = $2;
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		}
	| '{' declaration_list '}'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			$$->compound_stmt.decl_list = $2;
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		}
	| '{' declaration_list statement_list '}'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			$$->compound_stmt.decl_list = $2;
			$$->compound_stmt.stmt_list = $3;
			if (last_list_node_stack_ptr-- == 0)
				panic("");
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		}
	;

declaration_list
	: declaration
		{
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			$$ = last_list_node_stack[last_list_node_stack_ptr++] = $1;
		}
	| declaration_list declaration
		{
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= $2;
			$$ = $1;
		}
	;

statement_list
	: statement
		{
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			$$ = last_list_node_stack[last_list_node_stack_ptr++] = $1;
		}
	| statement_list statement
		{
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= $2;
			$$ = $1;
		}
	;

expression_statement
	: ';' { $$ = 0; }
	| expression ';' { $$ = $1; }
	;

selection_statement
	: IF '(' expression ')' statement
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			$$->select_stmt.type = /*xlat_unit_parse_node::*//*select_stmt::*/SELECT_STMT_IF;
			$$->select_stmt.expr = $3;
			$$->select_stmt.stmt = $5;
		}
	| IF '(' expression ')' statement ELSE statement
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			$$->select_stmt.type = /*xlat_unit_parse_node::*//*select_stmt::*/SELECT_STMT_IF_ELSE;
			$$->select_stmt.expr = $3;
			$$->select_stmt.stmt = $5;
			$$->select_stmt.else_stmt = $7;
		}
	| SWITCH '(' expression ')' statement
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			$$->select_stmt.type = /*xlat_unit_parse_node::*//*select_stmt::*/SELECT_STMT_SWITCH;
			$$->select_stmt.expr = $3;
			$$->select_stmt.stmt = $5;
		}
	;

iteration_statement
	: WHILE '(' expression ')' statement
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			$$->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_WHILE;
			$$->iterate_stmt.expr = $3;
			$$->iterate_stmt.stmt = $5;
		}
	| DO statement WHILE '(' expression ')' ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			$$->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_DO_WHILE;
			$$->iterate_stmt.expr = $5;
			$$->iterate_stmt.stmt = $2;
		}
	| FOR '(' expression_statement expression_statement ')' statement
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			$$->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_FOR_EXPR_EXPR;
			$$->iterate_stmt.expr = $3;
			$$->iterate_stmt.stmt = $6;
			$$->iterate_stmt.for_expr_ctrl = $4;
		}
	| FOR '(' expression_statement expression_statement expression ')' statement
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			$$->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_FOR_EXPR_EXPR_EXPR;
			$$->iterate_stmt.expr = $3;
			$$->iterate_stmt.stmt = $7;
			$$->iterate_stmt.for_expr_ctrl = $4;
			$$->iterate_stmt.for_expr_post = $5;
		}
	;

jump_statement
	: GOTO IDENTIFIER ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			$$->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_GOTO;
			$$->jump_stmt.details.id = $2;
		}
	| CONTINUE ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			$$->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_CONTINUE;
		}
	| BREAK ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			$$->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_BREAK;
		}
	| RETURN ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			$$->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_RETURN;
		}
	| RETURN expression ';'
		{
			($$ = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			$$->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_RETURN;
			$$->jump_stmt.details.expr = $2;
		}
	;

translation_unit
	: external_declaration
		{
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			parse_head = $$ = last_list_node_stack[last_list_node_stack_ptr++] = $1;
		}
	| translation_unit external_declaration
		{
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= $2;
			parse_head = $$ = $1;
		}
	;

external_declaration
	: function_definition { $$ = $1; }
	| declaration { $$ = $1; }
	| '@' expression '@' { $$ = $2; }
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement { $$ = 0; }
	| declaration_specifiers declarator compound_statement { $$ = 0; }
	| declarator declaration_list compound_statement { $$ = 0; }
	| declarator compound_statement { $$ = 0; }
	;

%%
#include <stdio.h>

extern char yytext[];
extern int column;

#if __CXX__
void clang_parser::parser::error(const location_type * loc, const std::string& msg)
#else
static void yyerror(const YYLTYPE * loc, void * x, const char * msg)
#endif
{
	fflush(stdout);
#if __CXX__
	printf("\n%*s\n%*s", loc->begin.column, "^", loc->begin.column, "");
	cout << msg << '\n';
#else
	printf("\n%*s\n%*s\n", loc->first_column, "^", loc->first_column, msg);
#endif
}

static struct xlat_unit_parse_node * merge_decl_specs(struct xlat_unit_parse_node * n1, struct xlat_unit_parse_node * n2)
{
struct decl_spec_node tmp, tmp1;
int i;

	if (n1->type != PARSE_NODE_DECL_SPEC
		|| n2->type != PARSE_NODE_DECL_SPEC)
		panic("");
	/* check for semantic inconsistencies */
	if (n1->decl_spec.storage_class_flags & n2->decl_spec.storage_class_flags
		|| n1->decl_spec.type_qualifier_flags & n2->decl_spec.type_qualifier_flags
		|| n1->decl_spec.type_specifier_flags & n2->decl_spec.type_specifier_flags
		|| n1->decl_spec.detail && n2->decl_spec.detail)
	{
		printf("error: bad type declaration specifier list\n");
		goto error;
	}
	tmp = n1->decl_spec;

	tmp.storage_class_flags |= n2->decl_spec.storage_class_flags;
	tmp1 = tmp;
	if (i = tmp1.storage_class_flags)
	{
		/* see if more than one storage class flags have
		 * been requested */
		/* set the first nonzero bit to zero, then see if more
		 * nonzero bits remain */
		if ((((i - 1) & ~i) + 1) ^ i)
		{
			printf("error: too many storage class specifiers\n");
			goto error;
		}
	}
	/* nothing to check about the type qualifier flags */
	tmp.type_qualifier_flags |= n2->decl_spec.type_qualifier_flags;
	tmp.type_specifier_flags |= n2->decl_spec.type_specifier_flags;
	tmp1 = tmp;
	if (i = tmp1.type_specifier_flags)
	{
		/* see if the requested type specifier flags are incompatible */
		if (tmp1.is_void
			|| tmp1.is_float
			|| /*!\todo this excludes long double - add it */tmp1.is_double
			|| tmp1.is_struct_or_union_spec
			|| tmp1.is_enum_spec
			|| tmp1.is_type_name)
		{
			/* set the first nonzero bit to zero, then see if more
			 * nonzero bits remain */
			if ((((i - 1) & ~i) + 1) ^ i)
			{
				printf("error: incompatible type specifiers requested\n");
				goto error;
			}
		}
		else
		{
			if ((tmp1.is_signed && tmp1.is_unsigned)
				|| (tmp1.is_long && tmp1.is_short))
			{
				printf("error: incompatible type specifiers requested\n");
				goto error;
			}
			tmp1.is_signed = tmp1.is_unsigned
				= tmp1.is_long = tmp1.is_short = false;

			/* set the first nonzero bit to zero, then see if more
			 * nonzero bits remain */
			if ((((i - 1) & ~i) + 1) ^ i)
			{
				printf("error: incompatible type specifiers requested\n");
				goto error;
			}
		}
	}
	if (n1->decl_spec.detail && n2->decl_spec.detail)
	{
		printf("error: incompatible type specifiers requested\n");
		goto error;
	}
	if (n1->decl_spec.detail)
		tmp.detail = n1->decl_spec.detail;
	else
		tmp.detail = n2->decl_spec.detail;
	/* no error */
	n1->decl_spec = tmp;
	free(n2);
	return n1;

error:
	free(n1);
	free(n2);
	return 0;
}

