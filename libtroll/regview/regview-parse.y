%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_regview"
%parse-param { struct regview_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { yyscan_t yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }

%destructor { free($$); } CSTR 
%destructor { troll_deallocate_record($$); } reg_entry

%initial-action		{ *parse_head = 0; htroll->is_parse_error = false; htroll->lexpos = 0; }

%union
{
	char * cstr;
	unsigned long	num;
	struct regview_type_struct * reg_node;
}

%{
/* pull-in the lexer declaration */	
#include "regview-common.h"
#include "regview-lex.h"

static void yyerror(struct regview_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token NUM CSTR
%token KEYWORD
%token ERROR


%type <cstr>	CSTR
%type <num>	NUM
%type <reg_node>	reglist
%type <reg_node>	reg_entry

%start start_sym

%%

start_sym
	: '(' reglist ')'
	;

reglist
	: reg_entry	{ *parse_head = $$ = $1; }
	| reglist reg_entry
		{
			$$ = $1;
			while ($1->next)
				$1 = $1->next;
			$1->next = $2;
		}
	;

reg_entry
	: CSTR '=' NUM ','
		{
			$$ = troll_allocate_record(TROLL_RECORD_REGISTER_LIST);
			$$->name = $1;
			$$->val = $3;
		}
	;

%%


static void yyerror(struct regview_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}


void regview_destroy(struct regview_type_struct * node)
{
struct regview_type_struct * p, * t;

	if (!node)
		return;
	p = node;
	while (p)
	{
		t = p;
		p = p->next;
		if (t->name)
			free(t->name);
		free(t);
	}
}

struct regview_type_struct * regview_clone(struct regview_type_struct * src)
{
struct regview_type_struct * reg_node, * p, * h;

	if (!src)
		return 0;
	if (!(h = p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);

	reg_node = src;

	goto there;

	while (reg_node = reg_node->next)
	{
		p = p->next = calloc(1, sizeof * p);
there:
		p->val = reg_node->val;
		if (reg_node->name)
			p->name = strdup(reg_node->name);
		else
			p->name = 0;
	}
	p->next = 0;

	return h;
}

