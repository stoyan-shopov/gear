

enum EXPR_NODE_ENUM
{
	EXPR_NODE_INVALID = 0,
	EXPR_NODE_IDENT,
	EXPR_NODE_CONSTANT,
	EXPR_NODE_STR_LITERAL,
	EXPR_NODE_PARENTHESIZED_EXPR,
	EXPR_NODE_ARR_SUBSCRIPT,
	EXPR_NODE_FUNCTION_CALL,
	EXPR_NODE_DOT_MEMBER_SELECT,
	EXPR_NODE_PTR_MEMBER_SELECT,
	EXPR_NODE_POST_INC,
	EXPR_NODE_POST_DEC,
	EXPR_NODE_ARG_EXPR_LIST,
	EXPR_NODE_PRE_INC,
	EXPR_NODE_PRE_DEC,
	EXPR_NODE_UNARY_AMPERSAND,
	EXPR_NODE_UNARY_INDIRECT,
	EXPR_NODE_UNARY_PLUS,
	EXPR_NODE_UNARY_MINUS,
	EXPR_NODE_UNARY_BITWISE_NOT,
	EXPR_NODE_UNARY_LOGICAL_NOT,
	EXPR_NODE_SIZEOF,
	EXPR_NODE_TYPE_CAST,
	EXPR_NODE_MUL,
	EXPR_NODE_DIV,
	EXPR_NODE_MOD,
	EXPR_NODE_ADD,
	EXPR_NODE_SUB,
	EXPR_NODE_LEFT_SHIFT,
	EXPR_NODE_RIGHT_SHIFT,
	EXPR_NODE_LESS,
	EXPR_NODE_GREATER,
	EXPR_NODE_LESS_OR_EQUAL,
	EXPR_NODE_GREATER_OR_EQUAL,
	EXPR_NODE_EQUAL,
	EXPR_NODE_NOT_EQUAL,
	EXPR_NODE_BITWISE_AND,
	EXPR_NODE_XOR,
	EXPR_NODE_BITWISE_OR,
	EXPR_NODE_LOGICAL_AND,
	EXPR_NODE_LOGICAL_OR,
	EXPR_NODE_TERNARY_COND,
	EXPR_NODE_ASSIGN,
	EXPR_NODE_MUL_ASSIGN,
	EXPR_NODE_DIV_ASSIGN,
	EXPR_NODE_MOD_ASSIGN,
	EXPR_NODE_ADD_ASSIGN,
	EXPR_NODE_SUB_ASSIGN,
	EXPR_NODE_LEFT_ASSIGN,
	EXPR_NODE_RIGHT_ASSIGN,
	EXPR_NODE_AND_ASSIGN,
	EXPR_NODE_XOR_ASSIGN,
	EXPR_NODE_OR_ASSIGN,
};

struct
{
	expr[2];
	union
	{
		/* this is for the ternary conditional expression */
		expr;
		char * id;
		char * string_literal;
		void * const_val;
	};
};

%%

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

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

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

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

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
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

expression
	: assignment_expression { $$ = 0; }
	| expression ',' assignment_expression { $$ = 0; }
	;



cond_expr
unary_expr assgn_op assign_expr
--------------------------
assign_expr
comma
