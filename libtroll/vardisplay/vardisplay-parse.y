/*! \todo	clean this up; the memory management and the
 *		parsing is braindamaged (see e.g. the breakpoint
 *		parser for an example of how to reorganize this) */

%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_vardisplay"
%parse-param { struct vardisplay_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }
%initial-action {/* yydebug_ = 1; */}

%destructor { free($$); } CSTR 
%destructor { dealloc_var_typedef_fields($$); free($$); } type_desc_list 
%destructor { dealloc_var_typedef_fields(&$$); } type_attr type_attr_list arr_dim_list
%destructor { int i; for (i = 0; i < $$.idx; i++) free($$.vardata[i]); free($$.vardata); } value_list

%initial-action		{ * parse_head = 0; htroll->is_parse_error = false; htroll->lexpos = 0; }

%{
#include "vardisplay-types.h"
%}

%union
{
	struct var_typedef	type;
	struct var_typedef	* ptype;
	char * cstr;
	unsigned long	num;
	struct
	{
		int	size;
		int	idx;
		const char ** vardata;
	}
	vardata;
}


%{
/* pull-in the lexer declaration */
#include "vardisplay-common.h"
#include "vardisplay-lex.h"
#include "panic.h"

static void yyerror(struct vardisplay_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);
static void dealloc_var_typedef_fields(struct var_typedef * p)
{
	if (p->name)
		free(p->name);
	if (p->type_name)
		free(p->type_name);
	if (p->rawtype)
		free(p->rawtype);
	if (p->childlist)
	{
		dealloc_var_typedef_fields(p->childlist);
		free(p->childlist);
	}
	if (p->next)
	{
		dealloc_var_typedef_fields(p->next);
		free(p->next);
	}
}

static int compute_nr_atoms(struct var_typedef * p)
{
int i, j;
struct var_typedef * t;

	if (!p)
		return 0;
	i = compute_nr_atoms(p->childlist);
	if (!i)
		i = 1;
	for (j = 0; j < p->len_upper_bounds; j ++)
		i *= p->arr_upper_bounds[j] + 1;
	return (p->nr_atoms = i) + compute_nr_atoms(p->next);
}

static void merge_type_nodes(struct var_typedef * dest, struct var_typedef * src);
%}

%token ARRDIMS TYPENAME CHILDLIST NAME CSTR DEREF_POINT NUM VAR_NR RAWTYPE VALUE_LIST


%type <cstr>	CSTR
%type <num>	NUM

%type <type>	type_attr
%type <type>	type_attr_list
%type <ptype>	type_desc_list
%type <type>	arr_dim_list
%type <vardata>	value_list

%start type_description

%%


type_description
	: '['	type_attr_list ']'	{
		* parse_head = troll_allocate_record(TROLL_RECORD_VARDISPLAY);
		(*parse_head)->vartype = malloc(sizeof *(*parse_head)->vartype);
		*(*parse_head)->vartype = $2;
	}
	| '['	type_attr_list VALUE_LIST '=' '(' value_list ')' ',' ']'	{
		int t;

		* parse_head = troll_allocate_record(TROLL_RECORD_VARDISPLAY);
		(*parse_head)->vartype = malloc(sizeof *(*parse_head)->vartype);
		*(*parse_head)->vartype = $2;
		(*parse_head)->valarray = $6.vardata;
		if ((t = compute_nr_atoms((*parse_head)->vartype)) != $6.idx)
		{
			printf("expected number of atoms: %i\n", t);
			printf("number of atoms received from the gear engine: %i\n", $6.idx);
			panic("");
		}
	}
	;

value_list
	: CSTR ',' { $$.vardata = malloc(($$.size = 16) * sizeof * $$.vardata); $$.vardata[0] = $1; $$.idx = 1; }
	| value_list CSTR ','{
		$$ = $1;
		if ($$.idx == $$.size)
			$$.vardata = realloc($$.vardata, ($$.size *= 2) * sizeof * $$.vardata);
		$$.vardata[$$.idx ++] = $2;
	}
	;

type_attr_list
	: type_attr ','	{
		$$ = $1;
	}
	| type_attr_list type_attr ',' {
		$$ = $1;
		merge_type_nodes(&$$, &$2);
	}
	;

type_attr
	: NAME '=' CSTR		{
		memset(&$$, 0, sizeof $$);
		$$.name = $3;
	}
	| ARRDIMS '=' arr_dim_list	{
		$$ = $3;
	}
	| DEREF_POINT	{
		memset(&$$, 0, sizeof $$);
		$$.is_deref_point = true;
	}
	| TYPENAME '=' CSTR	{
		memset(&$$, 0, sizeof $$);
		$$.type_name = $3;
	}
	| RAWTYPE '=' CSTR	{
		memset(&$$, 0, sizeof $$);
		$$.rawtype = $3;
	}
	| CHILDLIST '=' '(' type_desc_list ')'	{
		memset(&$$, 0, sizeof $$);
		$$.childlist = $4;
	}
	;

type_desc_list
	: '['	type_attr_list ']' ','	{
		$$ = malloc(sizeof * $$);
		*$$ = $2;
	}
	| '['	type_attr_list ']' ',' type_desc_list	{
		/*! \todo	change this to left recursion; the depth
		  *		here is not expected to be too large, but anyway... */
		$$ = malloc(sizeof * $$);
		*$$ = $2;
		$$->next = $5;
	}
	;

arr_dim_list
	: NUM '$'	{
		memset(&$$, 0, sizeof $$);
		$$.len_upper_bounds = 1;
		*$$.arr_upper_bounds = $1;
	}
	| arr_dim_list NUM '$'	{
		$$ = $1;
		if ($$.len_upper_bounds == MAX_ARR_BOUNDS)
			panic("");
		$$.arr_upper_bounds[$$.len_upper_bounds++] = $2;
	}
	;
%%

static void merge_type_nodes(struct var_typedef * dest, struct var_typedef * src)
{
	if (!(dest && src))
		panic("");

	/* a special case */
	if (dest->len_upper_bounds || src->len_upper_bounds)
	{
		if (dest->len_upper_bounds && src->len_upper_bounds)
			panic("");
		if (src->len_upper_bounds)
		{
			dest->len_upper_bounds = src->len_upper_bounds;
			memcpy(dest->arr_upper_bounds, src->arr_upper_bounds, sizeof dest->arr_upper_bounds);
		}
	}

	if (dest->is_deref_point && src->is_deref_point)
		panic("");
	dest->is_deref_point = dest->is_deref_point || src->is_deref_point;

	if (dest->name && src->name)
		panic("");

	if (src->name)
	{
		dest->name = src->name;
	}

	/* bloody special case; right now, the rawtype record generation
	 * in the engine is broken for arrays - in this case two rawtype
         * records are generated of which only the second one is valid */
	if (dest->rawtype && src->rawtype)
	{
		free(dest->rawtype);
		dest->rawtype = src->rawtype;
	}
	else if (src->rawtype)
	{
		dest->rawtype = src->rawtype;
	}

	if (dest->type_name && src->type_name)
		panic("");
	if (src->type_name)
	{
		dest->type_name = src->type_name;
	}

	if (dest->childlist && src->childlist)
		panic("");
	if (src->childlist)
	{
		dest->childlist = src->childlist;
	}

	if (dest->next && src->next)
		panic("");
	if (src->next)
	{
		dest->next = src->next;
	}
}

static void yyerror(struct vardisplay_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	* parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}

static struct var_typedef * clone_var_typedef(struct var_typedef * src)
{
struct var_typedef * p;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;

	if (src->name)
		p->name = strdup(src->name);
	if (src->type_name)
		p->type_name = strdup(src->type_name);
	if (src->rawtype)
		p->rawtype = strdup(src->rawtype);
	p->is_deref_point = src->is_deref_point;
	p->len_upper_bounds = src->len_upper_bounds;
	p->nr_atoms = src->nr_atoms;
	memcpy(p->arr_upper_bounds, src->arr_upper_bounds, sizeof p->arr_upper_bounds);

	p->childlist = clone_var_typedef(src->childlist);
	p->next = clone_var_typedef(src->next);

	return p;

}

void vardisplay_destroy(struct vardisplay_type_struct * node)
{
int i;	
	if (!node)
		return;
	for (i = 0; i < node->vartype->nr_atoms; i++)
		free(node->valarray[i]);
	if (node->valarray)
		free(node->valarray);
	dealloc_var_typedef_fields(node->vartype);
	free(node->vartype);
	free(node);
}

struct vardisplay_type_struct * vardisplay_clone(struct vardisplay_type_struct * src)
{
struct vardisplay_type_struct * p;
int i;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);
	p->vartype = clone_var_typedef(src->vartype);
	if (src->valarray)
	{
		p->valarray = malloc(p->vartype->nr_atoms * sizeof * p->valarray);
		for (i = 0; i < p->vartype->nr_atoms; i++)
			p->valarray[i] = strdup(src->valarray[i]);
	}

	return p;
}

