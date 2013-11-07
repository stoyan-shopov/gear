
#include <stdbool.h>

struct xlat_unit_parse_node
{
	/* a generict list 'next' pointer - interpretation depends on
	 *
	 * context (e.g., this could be a declaration list, a statement
	 * list, etc.) */
	struct xlat_unit_parse_node * next;
	enum
	{
		PARSE_NODE_INVALID = 0,
		PARSE_NODE_EXPR,
		PARSE_NODE_TYPE_NAME,
		/* a declaration will normally
		 * have an associated declaration specifier
		 * list, and a list of declarators
		 * (which may themselves contain initializers) */
		PARSE_NODE_DECLARATION,
		PARSE_NODE_DECL_SPEC,
		PARSE_NODE_DECLARATOR,
		/* a function definition */
		PARSE_NODE_FUNCDEF,
		/* statement types */
		PARSE_NODE_LABELED_STMT,
		PARSE_NODE_COMPOUND_STMT,
		/*PARSE_NODE_EXPR_STMT,*/
		PARSE_NODE_SELECT_STMT,
		PARSE_NODE_ITERATE_STMT,
		PARSE_NODE_JUMP_STMT,
	}
	type;

	union
	{
		/* a declaration */
		struct decl_node
		{
			struct xlat_unit_parse_node	* decl_spec;
			struct xlat_unit_parse_node	* declarator_list;
		}
		declaration;

		struct declarator_node
		{
			enum
			{
				DECLARATOR_TYPE_INVALID = 0,
				DECLARATOR_TYPE_IDENT,
			}
			type;
			union
			{
				const char * id;
			};
			struct xlat_unit_parse_node	* decl_spec;
			/*! \todo	this doesnt really belong here,
			 *		it is redundant and must be pulled
			 *		somewhere out of here (maybe introduce
			 *		a new init_declarator node as present
			 *		in the grammar) */
			struct xlat_unit_parse_node	* initializer;
		}
		declarator;

		struct decl_spec_node
		{
			/* storage class related flags */
			union
			{
				struct
				{
					bool	is_typedef	: 1;
					bool	is_extern	: 1;
					bool	is_static	: 1;
					bool	is_auto		: 1;
					bool	is_register	: 1;
				};
				int	storage_class_flags;
			};
			/* type qualifier related flags */
			union
			{
				struct
				{
					bool	is_const	: 1;
					bool	is_volatile	: 1;
				};
				int	type_qualifier_flags;
			};
			/* type specifier related flags */
			union
			{
				/*!\todo	this cant represent long long - add it */
				struct
				{
					bool	is_void		: 1;
					bool	is_char		: 1;
					bool	is_short	: 1;
					bool	is_int		: 1;
					bool	is_long		: 1;
					bool	is_float	: 1;
					bool	is_double	: 1;
					bool	is_signed	: 1;
					bool	is_unsigned	: 1;
					bool	is_struct_or_union_spec	: 1;
					bool	is_enum_spec	: 1;
					bool	is_type_name	: 1;
				};
				int	type_specifier_flags;
			};
			/* this field is applicable only if either of
			 * the is_struct_or_union_spec, is_enum_spec,
			 * is_type_name flags above is true, this gives
			 * the details about the structure-or-union,
			 * the enumeration, or the type name
			 *
			 * note that the grammar allows any of the three to be
			 * true siumltaneously (which would make this field ambiguous),
			 * but since this is not really useful, it is flagged as
			 * a semantic error by the parser, so this field should
			 * never be ambiguous in the interpretation/execution
			 * phase (that is, no more than one of the above flags
			 * shall be true for any declaration in a successfully
			 * parsed program) */
			struct xlat_unit_parse_node * detail;
		}
		decl_spec;

		struct
		{
			enum
			{
				LABELED_STMT_INVALID = 0,
				/* a 'switch' labeled statement - either
				 * a 'case', or a 'default'
				 * labeled statement */
				LABELED_STMT_SWITCH,
				/* an 'ordinary' labeled statement -
				 * i.e., one that can be the target of
				 * a 'goto' jump statement */
				LABELED_STMT_LABEL,
			}
			type;
			union
			{
				char	* label_name;
				/* if 0, this is a 'default'
				 * labeled statement, otherwise
				 * this is the controlling expression
				 * for a 'case' labeled statement */
				struct xlat_unit_parse_node	* expr;
			};
			/* the statement itself */
			struct xlat_unit_parse_node * stmt;
		}
		labeled_statement;

		struct compound_stmt
		{
			/* declaration list, can be empty */
			struct xlat_unit_parse_node	* decl_list;
			/* statement list, can be empty */
			struct xlat_unit_parse_node	* stmt_list;
		}
		compound_stmt;

		struct select_stmt
		{
			enum
			{
				SELECT_STMT_INVALID = 0,
				SELECT_STMT_IF,
				SELECT_STMT_IF_ELSE,
				SELECT_STMT_SWITCH,
			}
			type;
			/* statement control expression */
			struct xlat_unit_parse_node	* expr;
			struct xlat_unit_parse_node	* stmt;
			struct xlat_unit_parse_node	* else_stmt;
		}
		select_stmt;

		struct iterate_stmt
		{
			enum
			{
				ITER_STMT_INVALID = 0,
				ITER_STMT_WHILE,
				ITER_STMT_DO_WHILE,
				ITER_STMT_FOR_EXPR_EXPR,
				ITER_STMT_FOR_EXPR_EXPR_EXPR,
			}
			type;
			struct xlat_unit_parse_node	* stmt;
			struct xlat_unit_parse_node	* expr;
			/* 'for' loop controlling expression */
			struct xlat_unit_parse_node	* for_expr_ctrl;
			/* 'for' loop post-expression */
			struct xlat_unit_parse_node	* for_expr_post;
		}
		iterate_stmt;

		struct jump_stmt
		{
			enum
			{
				JUMP_STMT_INVALID = 0,
				JUMP_STMT_GOTO,
				JUMP_STMT_CONTINUE,
				JUMP_STMT_BREAK,
				JUMP_STMT_RETURN,
			}
			type;
			union
			{
				/* goto label string */
				const char * id;
				/* expression data; if 0, return nothing */
				struct xlat_unit_parse_node	* expr;
			}
			details;
		}
		jump_stmt;

		struct expr_node
		{
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
				EXPR_NODE_COMMA,
			}
			type;

			struct
			{
				struct xlat_unit_parse_node * expr[2];
				union
				{
					/* this is for the ternary conditional expression */
					struct xlat_unit_parse_node * cond_expr;
					/* this holds the type name for type cast
					 * and sizeof expressions */
					struct xlat_unit_parse_node * type_name;
					const char * id;
					const char * string_literal;
					/*const void * const_val;*/
					long int const_val;
				};
			};
		}
		expr_node;
	};
};

