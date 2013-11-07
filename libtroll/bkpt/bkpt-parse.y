%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_bkpt"
%parse-param { struct bkpt_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }

%destructor { free($$); } CSTR 
%destructor { if ($$.f.is_srcfile_valid) free($$.srcfile); } bkpt_desc bkpt_attr

%initial-action		{ * parse_head = 0; htroll->is_parse_error = false; htroll->lexpos = 0; }

%{
#include "bkpt-types.h"
%}

%union
{
	char * cstr;
	enum BKPT_RECORD_ENUM	btype;
	unsigned long	num;
	struct bkpt_type_struct bkpt, * last;
}

%{
/* pull-in the lexer declaration */
#include "bkpt-common.h"
#include "bkpt-lex.h"

static void yyerror(struct bkpt_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token NUM CSTR

%token ADDR_KW SRCFILE_KW SRCLINE_KW IS_AT_START_OF_STATEMENT_KW
%token BREAKPOINT_INSERTED_KW BREAKPOINT_REMOVED_KW BREAKPOINT_LIST_KW

%type <cstr>	CSTR
%type <num>	NUM
%type <btype>	bkpt_record_type
%type <bkpt>	bkpt_desc
%type <bkpt>	bkpt_attr
%type <last>	bkpt_desc_list

%start start_sym

%%

start_sym
	: bkpt_record_type '=' '(' bkpt_desc_list ')' { (*parse_head)->bkpt_record_type = $1; }
	| BREAKPOINT_LIST_KW
	{
		*parse_head = troll_allocate_record(TROLL_RECORD_BREAKPOINT);
		(*parse_head)->bkpt_record_type = BKPT_RECORD_BKPT_LIST;
		(*parse_head)->f.is_empty = 1;
	}
	| BREAKPOINT_INSERTED_KW
	{
		*parse_head = troll_allocate_record(TROLL_RECORD_BREAKPOINT);
		(*parse_head)->bkpt_record_type = BKPT_RECORD_BKPT_ADDED;
		(*parse_head)->f.is_empty = 1;
	}
	| BREAKPOINT_REMOVED_KW
	{
		*parse_head = troll_allocate_record(TROLL_RECORD_BREAKPOINT);
		(*parse_head)->bkpt_record_type = BKPT_RECORD_BKPT_REMOVED;
		(*parse_head)->f.is_empty = 1;
	}
	;

bkpt_record_type
	: BREAKPOINT_INSERTED_KW	{ $$ = BKPT_RECORD_BKPT_ADDED; }
	| BREAKPOINT_REMOVED_KW		{ $$ = BKPT_RECORD_BKPT_REMOVED; }
	| BREAKPOINT_LIST_KW		{ $$ = BKPT_RECORD_BKPT_LIST; }
	;

bkpt_desc_list
		: '[' bkpt_desc ']' ','
		{
			struct parse_type_common_struct p;
			$$ = *parse_head = troll_allocate_record(TROLL_RECORD_BREAKPOINT); 
			p = (*parse_head)->head;
			*$$ = $2;
			(*parse_head)->head = p;
		}
		| bkpt_desc_list '[' bkpt_desc ']' ','
		{
			$$ = $$->next = troll_allocate_record(TROLL_RECORD_BREAKPOINT); 
			*$$ = $3;
		}
		;
bkpt_desc
	: bkpt_attr ',' { $$ = $1; }
	| bkpt_desc bkpt_attr ','
	{
		$$ = $1;
		if ($2.f.is_addr_valid)
			$$.f.is_addr_valid = 1, $$.addr = $2.addr;
		if ($2.f.is_srcfile_valid)
			$$.f.is_srcfile_valid = 1, $$.srcfile = $2.srcfile;
		if ($2.f.is_srcline_valid)
			$$.f.is_srcline_valid = 1, $$.srcline = $2.srcline;
		if ($2.f.is_stmt_flag_valid)
			$$.f.is_stmt_flag_valid = 1, $$.is_at_start_of_stmt = $2.is_at_start_of_stmt;

	}
	;

bkpt_attr
	: ADDR_KW '=' NUM { $$ = (struct bkpt_type_struct) { .addr = $3, .f.is_addr_valid = 1 }; }
	| SRCFILE_KW '=' CSTR { $$ = (struct bkpt_type_struct) { .srcfile = $3, .f.is_srcfile_valid = 1 }; }
	| SRCLINE_KW '=' NUM { $$ = (struct bkpt_type_struct) { .srcline = $3, .f.is_srcline_valid = 1 }; }
	| IS_AT_START_OF_STATEMENT_KW '=' NUM { $$ = (struct bkpt_type_struct) { .is_at_start_of_stmt = ($3 ? true : false), .f.is_stmt_flag_valid = 1 }; }
	;

%%

static void yyerror(struct bkpt_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}

void bkpt_destroy(struct bkpt_type_struct * node)
{
struct bkpt_type_struct * p;
	if (!node)
		return;
	p = node;
	while (p)
	{
		if (p->srcfile)
			free(p->srcfile);
		node = p;
		p = p->next;
		free(node);
	}
}

struct bkpt_type_struct * bkpt_clone(struct bkpt_type_struct * src)
{
struct bkpt_type_struct * p, * t;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);

	t = p;
	do
	{
		t->f.is_empty = src->f.is_empty;
		t->f.is_addr_valid = src->f.is_addr_valid;
		t->f.is_srcfile_valid = src->f.is_srcfile_valid;
		t->f.is_srcline_valid = src->f.is_srcline_valid;
		t->f.is_stmt_flag_valid = src->f.is_stmt_flag_valid;

		t->addr = src->addr;
		t->is_at_start_of_stmt = src->is_at_start_of_stmt;
		if (src->srcfile)
			t->srcfile = strdup(src->srcfile);
		else
			t->srcfile = 0;
		t->srcline = src->srcline;
		if (src->next)
		{
			t->next = calloc(1, sizeof * t);
		}
		t = t->next;
		src = src->next;
	}
	while (src);

	return p;
}


