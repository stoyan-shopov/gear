
%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_stackframe"
%parse-param { struct stackframe_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }
%initial-action {/* yydebug_ = 1; */}

%destructor { free($$); } CSTR 
%destructor {
	if ($$.flags.is_comp_unit_valid)
		free($$.comp_unit_name);
	if ($$.flags.is_subprogram_valid)
		free($$.subprogram_name);
	if ($$.flags.is_srcfile_valid)
		free($$.srcfile_name);
	} stackframe_record stackframe_desc_list

%initial-action		{ * parse_head = 0; htroll->is_parse_error = false; htroll->lexpos = 0; }

%{
#include "stackframe-types.h"	
%}

%union
{
	char * cstr;
	ARM_CORE_WORD addr;
	struct stackframe_struct node;
	struct stackframe_struct * last_node;
}

%{
/* pull-in the lexer declaration */	
#include "panic.h"
#include "stackframe-common.h"
#include "stackframe-lex.h"

static void yyerror(struct stackframe_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token PC_ADDR
%token COMP_UNIT
%token SUBPROGRAM
%token SRCFILE
%token SRCLINE_NR
%token SELECTED_FRAME
%token NUM CSTR

%type <addr>	NUM
%type <cstr>	CSTR
%type <node>	stackframe_desc_list
%type <node>	stackframe_record
%type <last_node>	stackframe_list

%start start_sym

%%

start_sym
	: '(' stackframe_list ')'
	| SELECTED_FRAME '=' NUM
		{
			*parse_head = troll_allocate_record(TROLL_RECORD_STACKFRAME); 
			(*parse_head)->record_type = STACKFRAME_RECORD_TYPE_SELECTED_FRAME_NR;
			(*parse_head)->selected_frame_nr = $3;
		}
	;


stackframe_list
		: '[' stackframe_desc_list ']' ',' {
				*parse_head = troll_allocate_record(TROLL_RECORD_STACKFRAME); 
				(*parse_head)->record_type = STACKFRAME_RECORD_TYPE_BACKTRACE;
				*($$ = (*parse_head)->youngest_frame = malloc(sizeof(struct stackframe_struct))) = $2;
				(*parse_head)->frame_cnt = 1;
			}
		| stackframe_list '[' stackframe_desc_list ']' ',' {
				* ($1->older = malloc(sizeof(struct stackframe_struct))) = $3;
				$$ = $1->older;
				$$->younger = $1;
				(*parse_head)->oldest_frame = $$;
				(*parse_head)->frame_cnt++;
			}
		;

stackframe_desc_list
	: stackframe_record ',' { $$ = $1; }
	| stackframe_desc_list stackframe_record ',' {
		$$ = $1;
		if ($2.flags.is_comp_unit_valid)
			$$.flags.is_comp_unit_valid = 1, $$.comp_unit_name = $2.comp_unit_name;
		else if ($2.flags.is_pc_addr_valid)
			$$.flags.is_pc_addr_valid = 1, $$.pc_addr = $2.pc_addr;
		else if ($2.flags.is_subprogram_valid)
			$$.flags.is_subprogram_valid = 1, $$.subprogram_name = $2.subprogram_name;
		else if ($2.flags.is_srcfile_valid)
			$$.flags.is_srcfile_valid = 1, $$.srcfile_name = $2.srcfile_name;
		else if ($2.flags.is_srcline_valid)
			$$.flags.is_srcline_valid = 1, $$.srcline = $2.srcline;
	}
	;


stackframe_record
	: COMP_UNIT '=' CSTR { $$ = (struct stackframe_struct) { .flags.is_comp_unit_valid = 1, .comp_unit_name = $3}; }
	| PC_ADDR '=' NUM { $$ = (struct stackframe_struct) { .flags.is_pc_addr_valid = 1, .pc_addr = $3}; }
	| SUBPROGRAM '=' CSTR { $$ = (struct stackframe_struct) { .flags.is_subprogram_valid = 1, .subprogram_name = $3}; }
	| SRCFILE '=' CSTR { $$ = (struct stackframe_struct) { .flags.is_srcfile_valid = 1, .srcfile_name = $3}; }
	| SRCLINE_NR '=' NUM { $$ = (struct stackframe_struct) { .flags.is_srcline_valid = 1, .srcline = $3}; }
	;

%%


static void yyerror(struct stackframe_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}

void stackframe_destroy(struct stackframe_type_struct * node)
{
	struct stackframe_struct * p, * t;

	if (!node)
		return;

	if (node->record_type == STACKFRAME_RECORD_TYPE_BACKTRACE)
	{
		/* destroy the frame list */
		p = node->youngest_frame;
		while (p)
		{
			t = p->older;
			if (p->flags.is_comp_unit_valid)
				free(p->comp_unit_name);
			if (p->flags.is_subprogram_valid)
				free(p->subprogram_name);
			if (p->flags.is_srcfile_valid)
				free(p->srcfile_name);
			free(p);
			p = t;
		}
	}
	else if (node->record_type == STACKFRAME_RECORD_TYPE_SELECTED_FRAME_NR)
	{/* nothing to do */}
	else
		panic("");
	free(node);
}

struct stackframe_type_struct * stackframe_clone(struct stackframe_type_struct * src)
{
struct stackframe_type_struct * p;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);
	{
	struct stackframe_struct * f, ** fn, * younger;

		p->record_type = src->record_type;
		if (src->record_type == STACKFRAME_RECORD_TYPE_BACKTRACE)
		{
			f = src->youngest_frame;
			fn = &p->youngest_frame;
			p->frame_cnt = src->frame_cnt;
			younger = 0;
			while (f)
			{
				*fn = calloc(1, sizeof ** fn);
				p->oldest_frame = *fn;
				(*fn)->flags = f->flags;
				(*fn)->pc_addr = f->pc_addr;
				(*fn)->srcline = f->srcline;

				(*fn)->comp_unit_name = 0;
				(*fn)->subprogram_name = 0;
				(*fn)->srcfile_name = 0;
				if (f->flags.is_comp_unit_valid)
					(*fn)->comp_unit_name = strdup(f->comp_unit_name);
				
				if (f->flags.is_subprogram_valid)
					(*fn)->subprogram_name = strdup(f->subprogram_name);
				if (f->flags.is_srcfile_valid)
					(*fn)->srcfile_name = strdup(f->srcfile_name);
				/* link nodes */
				(*fn)->younger = younger;
				if (younger)
					younger->older = *fn;
				younger = *fn;
				fn = &(*fn)->older;
				/* clone next frame */
				f = f->older;
			}
			/* terminate the list */
			*fn = 0;
		}
		else if (src->record_type == STACKFRAME_RECORD_TYPE_SELECTED_FRAME_NR)
		{
			p->selected_frame_nr = src->selected_frame_nr;
		}
		else
			panic("");
	}

	return p;
}

