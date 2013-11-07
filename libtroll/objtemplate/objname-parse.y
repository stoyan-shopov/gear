%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_objprefix"
%parse-param { struct objname_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }
%initial-action {/* yydebug_ = 1; */}

%destructor { free($$); } CSTR 

%initial-action		{ *parse_head = troll_allocate_record(TROLL_RECORD_INVALID); htroll->is_parse_error = false; htroll->lexpos = 0; }

%union
{
	char * cstr;
	unsigned long	num;
}

%{
/* pull-in the lexer declaration */	
#include "objname-common.h"
#include "objname-lex.h"

static void yyerror(struct objname_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token NUM CSTR
%token KEYWORD
%token ERROR


%type <cstr>	CSTR
%type <num>	NUM

%start start_sym

%%

start_sym
	: CSTR
	;

%%


static void yyerror(struct objname_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}

void objname_destroy(struct objname_type_struct * node)
{
	if (!node)
		return;
	free(node);
}

struct objname_type_struct * objname_clone(struct objname_type_struct * src)
{
struct objname_type_struct * p;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);

	return p;
}

