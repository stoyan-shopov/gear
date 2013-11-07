%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_disasm"
%parse-param { struct disasm_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { yyscan_t yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }

%destructor { free($$); } CSTR 
%destructor { free($$->text); free($$); } disasm_text_line src_text_line

%initial-action		{ *parse_head = troll_allocate_record(TROLL_RECORD_DISASSEMBLY_LIST); htroll->is_parse_error = false; htroll->lexpos = 0; }
%{
#include "disasm-types.h"
%}

%union
{
	char * cstr;
	unsigned long	num;
	struct disasm_text_node * t;
	struct
	{
		struct disasm_text_node * t;
		ARM_CORE_WORD		last_addr;
	}
	x;
}

%{
/* pull-in the lexer declaration */	
#include "disasm-common.h"
#include "disasm-lex.h"

static void yyerror(struct disasm_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token NUM CSTR
%token ADDR_KW
%token DISASM_LIST_KW
%token INSTRUCTION_COUNT_KW
%token SRCLINE_NR_KW
%token SRCFILE_KW
%token PC
%token DISASM_RANGE_KW
%token DISASM_KW


%type <cstr>	CSTR
%type <num>	NUM
%type <t>	disasm_text_line
%type <t>	src_text_line
%type <x>	dis_list

%start start_sym

%%

start_sym
	: '[' PC '=' NUM ',' DISASM_LIST_KW '=' '(' dis_list ')' ',' DISASM_RANGE_KW '=' '[' NUM ',' NUM ']' ',' INSTRUCTION_COUNT_KW '=' NUM ']' {
		(*parse_head)->pc = $4;
		(*parse_head)->start_addr = $15;
		(*parse_head)->first_addr_past_disassembly = $17;
		(*parse_head)->instruction_count = $22;
	}
	;

dis_list
	:	disasm_text_line	{ $$.t = $1; $$.last_addr = $1->num.addr; (*parse_head)->disasm_list = $1; }
	|	dis_list disasm_text_line { $$.t = $2; $$.last_addr = $2->num.addr; $1.t->next = $2; }
	|	src_text_line { $$.t = $1; (*parse_head)->disasm_list = $1; }
	|	dis_list src_text_line { $$.t = $2; $$.last_addr = $1.last_addr; $1.t->next = $2; }
	;

disasm_text_line
	:	ADDR_KW '=' NUM ',' DISASM_KW '=' CSTR ','	{
		$$ = calloc(1, sizeof * $$);
		$$->dis_type = DIS_TYPE_DISASSEMBLY;
		$$->text = $7;
		$$->num.addr = $3;
	}
src_text_line
	:	SRCLINE_NR_KW '=' NUM ',' SRCFILE_KW '=' CSTR ','	{
		$$ = calloc(1, sizeof * $$);
		$$->dis_type = DIS_TYPE_SRC;
		$$->text = $7;
		$$->num.line_nr = $3;
	}
	;

%%


static void yyerror(struct disasm_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
}

void disasm_destroy(struct disasm_type_struct * node)
{
struct disasm_text_node * p, * x;

	if (!node)
		return;

	p = node->disasm_list;
	while (p)
	{
		x = p;
		p = p->next;
		if (x->text)
			free(x->text);
		free(x);
	}
	free(node);
}

struct disasm_type_struct * disasm_clone(struct disasm_type_struct * src)
{
struct disasm_text_node * dis_node, ** new_node;
struct disasm_type_struct * p;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	troll_clone_parse_type_common_struct(&p->head, &src->head);

	p->pc = src->pc;
	p->start_addr = src->start_addr;
	p->first_addr_past_disassembly = src->first_addr_past_disassembly;

	new_node = &p->disasm_list;
	dis_node = src->disasm_list;
	while (dis_node)
	{
		*new_node = calloc(1, sizeof ** new_node);
		if (dis_node->text)
			(**new_node).text = strdup(dis_node->text);
		(**new_node).dis_type = dis_node->dis_type;
		(**new_node).num = dis_node->num;
		new_node = &(**new_node).next;
		dis_node = dis_node->next;
	}
	*new_node = 0;

	return p;
}

