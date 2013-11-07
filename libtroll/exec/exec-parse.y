
%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_exec"
%parse-param { struct exec_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }
%initial-action {/* yydebug_ = 1; */}

%destructor { free($$); } CSTR 

%initial-action		{ *parse_head = troll_allocate_record(TROLL_RECORD_EXEC); htroll->is_parse_error = false; htroll->lexpos = 0; }

%{
#include <libtroll.h>
%}

%union
{
	enum TARGET_CORE_STATE_ENUM target_state;
	char * cstr;
	ARM_CORE_WORD addr;
}

%{
/* pull-in the lexer declaration */	
#include "exec-common.h"
#include "exec-lex.h"

static void yyerror(struct exec_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token NEW_TARGET_STATE
%token STATE_RUNNING
%token STATE_STOPPED
%token HALT_ADDR
%token HALT_COMP_UNIT
%token HALT_SUBPROGRAM
%token HALT_SRCFILE
%token HALT_SRCLINE_NR
%token NUM CSTR


%type <addr>	NUM
%type <cstr>	CSTR

%start start_sym

%%

start_sym
	: '[' exec_state_record_list ']'
	;

exec_state_record_list
	: exec_state_record ','
	| exec_state_record_list exec_state_record ','
	;


exec_state_record
	: NEW_TARGET_STATE '=' target_state
	| HALT_COMP_UNIT '=' CSTR { (*parse_head)->flags.is_comp_unit_valid = 1;
       		(*parse_head)->comp_unit_name = $3;	}
	| HALT_SUBPROGRAM '=' CSTR	{ (*parse_head)->flags.is_subprogram_valid = 1;
       		(*parse_head)->subprogram_name = $3;	}
	| HALT_SRCFILE '=' CSTR		{ (*parse_head)->flags.is_srcfile_valid = 1;
       		(*parse_head)->srcfile_name = $3;	}
	| HALT_SRCLINE_NR '=' NUM	{ (*parse_head)->flags.is_srcline_valid = 1;
       		(*parse_head)->srcline = $3;	}
	;

target_state
	: STATE_RUNNING	{
		(*parse_head)->target_state = TARGET_CORE_STATE_RUNNING;
	}
	| STATE_STOPPED ',' HALT_ADDR '=' NUM	{
		(*parse_head)->target_state = TARGET_CORE_STATE_HALTED;
		(*parse_head)->flags.is_halt_addr_valid = 1;
		(*parse_head)->halt_addr = $5;
	}
	;

%%


static void yyerror(struct exec_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}

void exec_destroy(struct exec_type_struct * node)
{
	if (!node)
		return;
	if (node->flags.is_comp_unit_valid)
		free(node->comp_unit_name);
	if (node->flags.is_subprogram_valid)
		free(node->subprogram_name);
	if (node->flags.is_srcfile_valid)
		free(node->srcfile_name);
	free(node);
}

struct exec_type_struct * exec_clone(struct exec_type_struct * src)
{
struct exec_type_struct * p;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);

	p->flags = src->flags;
	p->target_state = src->target_state;
	p->halt_addr = src->halt_addr;
	p->srcline = src->srcline;

	p->comp_unit_name = 0;
	p->subprogram_name = 0;
	p->srcfile_name = 0;
	if (src->flags.is_comp_unit_valid)
		p->comp_unit_name = strdup(src->comp_unit_name);
	if (src->flags.is_subprogram_valid)
		p->subprogram_name = strdup(src->subprogram_name);
	if (src->flags.is_srcfile_valid)
		p->srcfile_name = strdup(src->srcfile_name);

	return p;
}

