%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_srcinfo"
%parse-param { struct srcinfo_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }
%initial-action {/* yydebug_ = 1; */}

%initial-action		{ *parse_head = troll_allocate_record(TROLL_RECORD_SRCINFO); htroll->is_parse_error = false; htroll->lexpos = 0; }

%{
#include "srcinfo-types.h"
%}

%destructor { free($$.bmap_buf); } int_array
%destructor { free($$); } CSTR
%destructor { int i; for (i = 0; i < $$.idx; i ++) free($$.subs[i].name); free($$.subs); } subprogram_list
%destructor { int i; for (i = 0; i < $$.idx; i++)
	{ if ($$.comp_units[i].name) free($$.comp_units[i].name);
	if ($$.comp_units[i].compdir) free($$.comp_units[i].compdir); }
	free($$.comp_units); } comp_unit_list
%destructor { free($$.ap); } num_pairs_list
%destructor {
int i;
if ($$.subprogram_arr)
{
	for (i = 0; i < $$.subprogram_arr_len; i++)
		free($$.subprogram_arr[i].name);
	free($$.subprogram_arr);	
}

if ($$.srcaddr_pairs) free($$.srcaddr_pairs);
if ($$.bkpt_bmap) free($$.bkpt_bmap);
if ($$.srcname) free($$.srcname);
} src_data_node src_data_node_list

%union
{
	char		* cstr;
	struct
	{
/* currently, the gear engine supports a maximum of 65536 source code lines */
#define MAX_SRCLINE_BMAP_SIZE		2048		
		int		(* bmap_buf)[MAX_SRCLINE_BMAP_SIZE];
		int		idx;
	}
	srcline_bmap;
	struct
	{
		struct subprogram_type_struct * subs;
		int len;
		int idx;
	}
	subarr;
	struct
	{
		struct cu_info_struct * comp_units;
		int len;
		int idx;
	}
	comp_unit_arr;
	struct
	{
		struct srcline_addr_pair_struct * ap;
		int aplen;
		int idx;
	}
	address_pairs;
	/*! \todo	currently, address ranges are not handled properly - only
	 *		the first address range of a range list is recorded; fix this */
	struct
	{
		ARM_CORE_WORD	low_addr;
		ARM_CORE_WORD	hi_addr;
	}
	addr_range;
	unsigned long	num;
	struct srclist_type_struct node;
	struct srclist_type_struct * last_src;
}

%{
/* pull-in the lexer declaration */	
#include "panic.h"
#include "srcinfo-common.h"
#include "srcinfo-lex.h"

static void yyerror(struct srcinfo_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);

%}

%token NUM CSTR
%token SRC_FILE_NAME_KW
%token INCLUSIVE_SPANNED_ADDR_RANGE_KW
%token SPANNED_ADDR_RANGES_KW
%token BKPT_LINES_BMAP_KW
%token SUBPROGRAM_LIST_KW
%token SUBPROGRAM_NAME_KW
%token NR_SRCLINES_KW
%token SRCLINE_LIST_KW
%token DECL_LINE_KW
%token COMPILATION_UNITS_KW
%token CU_NAME_KW
%token CU_COMPILATION_DIR_KW
%token SOURCE_FILES_DATA_KW


%type <cstr>	CSTR
%type <num>	NUM
%type <node>	src_data_node_list
%type <node>	src_data_node
%type <srcline_bmap>	int_array
%type <address_pairs>	num_pairs_list
%type <comp_unit_arr>	comp_unit_list
%type <subarr>	subprogram_list
%type <last_src>	srcfile_list

%start start_sym

%%

start_sym
		: '[' COMPILATION_UNITS_KW '=' '(' comp_unit_list ')' ',' SOURCE_FILES_DATA_KW '=' '(' srcfile_list ')' ',' ']'
			{
				(*parse_head)->nr_comp_units = $5.idx;
				(*parse_head)->comp_units = $5.comp_units;
			}
		;

srcfile_list
	: '[' src_data_node_list ']' ','
		{
			(*parse_head)->srclist = $$ = calloc(1, sizeof * $$);
			*$$ = $2;
		}
	| srcfile_list '[' src_data_node_list ']' ','
		{
			$$ = calloc(1, sizeof * $$);
			*$$ = $3;
			$1->next = $$;
		}
	;

src_data_node_list
		: src_data_node ',' { $$ = $1; }
		| src_data_node_list src_data_node ','
			{
				$$ = $1;
				if ($2.low_pc) $$.low_pc = $2.low_pc;
				if ($2.hi_pc) $$.hi_pc = $2.hi_pc;
				if ($2.subprogram_arr)
				{
					$$.subprogram_arr = $2.subprogram_arr;
					$$.subprogram_arr_len = $2.subprogram_arr_len;
				}
				if ($2.bmap_size)
				{
					$$.bmap_size = $2.bmap_size;
					$$.bkpt_bmap = $2.bkpt_bmap;
				}
				if ($2.srcaddr_len)
				{
					$$.srcaddr_len = $2.srcaddr_len;
					$$.srcaddr_pairs = $2.srcaddr_pairs;
				}
			}
		;



src_data_node
	: SRC_FILE_NAME_KW '=' CSTR
		{
			memset(&$$, 0, sizeof $$);
			$$.srcname = $3;
		}
	| INCLUSIVE_SPANNED_ADDR_RANGE_KW '=' '[' NUM NUM ']'
		{
			memset(&$$, 0, sizeof $$);
			$$.low_pc = $4;
			$$.hi_pc = $5;
		}
	| SPANNED_ADDR_RANGES_KW '=' '(' dummy_num_pairs_list ')'
		{
			/*! \todo	this is currently unused,
			 *		it is just syntactically
			 *		validated... */
			memset(&$$, 0, sizeof $$);
			/*
			free($4.ap);
			memset(&$$, 0, sizeof $$);
			*/
		}
	| SUBPROGRAM_LIST_KW '=' '(' ')'
		{
			memset(&$$, 0, sizeof $$);
		}
	| SUBPROGRAM_LIST_KW '=' '(' subprogram_list ')'
		{
			memset(&$$, 0, sizeof $$);
			$$.subprogram_arr = $4.subs;
			$$.subprogram_arr_len = $4.idx;
		}
	| BKPT_LINES_BMAP_KW '=' '(' int_array ')'
		{
			memset(&$$, 0, sizeof $$);
			$$.bmap_size = $4.idx;
			$$.bkpt_bmap = *$4.bmap_buf;
		}
	| SRCLINE_LIST_KW '=' '(' num_pairs_list ')'
		{
			memset(&$$, 0, sizeof $$);
			$$.srcaddr_pairs = $4.ap;
			$$.srcaddr_len = $4.idx;
		}
	;

subprogram_list
		: '[' SUBPROGRAM_NAME_KW '=' CSTR ',' DECL_LINE_KW '=' NUM ']' ','
			{
				$$.subs = calloc($$.len = 16, sizeof * $$.subs);
				$$.subs[0].name = $4;
				$$.subs[0].srcline_nr = $8;
				$$.idx = 1;
			}
		| subprogram_list '[' SUBPROGRAM_NAME_KW '=' CSTR ',' DECL_LINE_KW '=' NUM ']' ','
			{
				$$ = $1;
				if ($$.idx == $$.len)
					$$.subs = realloc($$.subs, ($$.len *= 2) * sizeof * $$.subs);
				$$.subs[$$.idx].name = $5;	
				$$.subs[$$.idx].srcline_nr = $9;	
				$$.idx ++;
			}
		;

dummy_num_pairs_list
		: NUM NUM ',' {}
		| dummy_num_pairs_list NUM NUM ',' {}
		;
num_pairs_list
		: NUM NUM NUM NUM ','
			{
				$$.ap = calloc($$.aplen = 256, sizeof * $$.ap);
				/* zeroth element reserved/unused */
				$$.ap[1].addr = $1;
				$$.ap[1].srcline_nr = $2;
				$$.ap[1].cuname_idx = $3;
				$$.ap[1].subarr_idx = $4;
				$$.idx = 2;
			}
		| num_pairs_list NUM NUM NUM NUM ','
			{
				$$ = $1;
				if ($$.idx == $$.aplen)
				{
					$$.ap = realloc($$.ap, $$.aplen * 2 * sizeof * $$.ap);
					memset($$.ap + $$.aplen, 0, $$.aplen * sizeof * $$.ap);
					$$.aplen *= 2;
				}
				$$.ap[$$.idx].addr = $2;
				$$.ap[$$.idx].srcline_nr = $3;
				$$.ap[$$.idx].cuname_idx = $4;
				$$.ap[$$.idx].subarr_idx = $5;
				$$.idx++;
			}
		;

int_array
	: NUM ','
		{
			$$.bmap_buf = calloc(MAX_SRCLINE_BMAP_SIZE, sizeof(int));
			(*$$.bmap_buf)[0] = $1;
			$$.idx = 1;
		}
	| int_array NUM ','
		{
			if ($1.idx >= MAX_SRCLINE_BMAP_SIZE)
				panic("");
			$$ = $1;
			(*$$.bmap_buf)[$$.idx ++] = $2;
		}
	;

comp_unit_list
	: '[' CU_NAME_KW '=' CSTR ',' CU_COMPILATION_DIR_KW '=' CSTR ',' ']' ','
		{
			$$.comp_units = calloc($$.len = 16, sizeof * $$.comp_units);
			$$.comp_units[0].name = $4;
			$$.comp_units[0].compdir = $8;
			$$.idx = 1;

		}
	| comp_unit_list '[' CU_NAME_KW '=' CSTR ',' CU_COMPILATION_DIR_KW '=' CSTR ',' ']' ','
		{
			$$ = $1;
			if ($$.idx == $$.len)
			{
				$$.comp_units = realloc($$.comp_units, $$.len * 2 * sizeof * $$.comp_units);
				memset($$.comp_units + $$.len, 0, $$.len * sizeof * $$.comp_units);
				$$.len *= 2;
			}
			$$.comp_units[$$.idx].name = $5;
			$$.comp_units[$$.idx ++].compdir = $9;
		}
	;

%%


static void yyerror(struct srcinfo_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	if (*parse_head)
		troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error at position %i:\n", htroll->lexpos);
	printf("%s\n", errmsg);
}

void srcinfo_destroy(struct srcinfo_type_struct * node)
{
struct srclist_type_struct * p, * p1;
int i;

	if (!node)
		return;
	for (i = 0; i < node->nr_comp_units; i++)
	{
		free(node->comp_units[i].name);
		free(node->comp_units[i].compdir);
	}
	if (node->comp_units)
		free(node->comp_units);

	p = node->srclist;
	while (p)
	{
		if (p->srcname)
			free(p->srcname);
		if (p->bkpt_bmap)
			free(p->bkpt_bmap);
		if (p->srcaddr_pairs)
			free(p->srcaddr_pairs);
		if (p->subprogram_arr)
		{
			for (i = 0; i < p->subprogram_arr_len; i ++)
				free(p->subprogram_arr[i].name);
			free(p->subprogram_arr);
		}
		p1 = p;
		p = p->next;
		free(p1);
	}

	free(node);
}

struct srcinfo_type_struct * srcinfo_clone(struct srcinfo_type_struct * src)
{
struct srclist_type_struct * p;
struct srclist_type_struct * psrc;
struct srcinfo_type_struct * newnode;
int i;


	if (!src)
		return 0;
	if (!(newnode = calloc(1, sizeof * newnode)))
		return 0;
	troll_clone_parse_type_common_struct(&newnode->head, &src->head);

	if (i = src->nr_comp_units)
	{
		newnode->comp_units = calloc(i, sizeof * newnode->comp_units);
		newnode->nr_comp_units = i;
		for (i = 0; i < src->nr_comp_units; i++)
		{
			newnode->comp_units[i].name = strdup(src->comp_units[i].name);
			newnode->comp_units[i].compdir = strdup(src->comp_units[i].compdir);
		}
	}

	/* clone srclist */
	newnode->srclist = p = calloc(1, sizeof * p);
	psrc = src->srclist;
	do
	{
		p->srcname = strdup(psrc->srcname);
		p->bmap_size = psrc->bmap_size;
		p->low_pc = psrc->low_pc;
		p->hi_pc = psrc->hi_pc;
		p->bkpt_bmap = (unsigned int *) malloc(p->bmap_size * sizeof * p->bkpt_bmap);
		memcpy(p->bkpt_bmap, psrc->bkpt_bmap, p->bmap_size * sizeof * p->bkpt_bmap);
		p->srcaddr_len = psrc->srcaddr_len;
		if (psrc->srcaddr_pairs)
		{
			p->srcaddr_pairs = (struct srcline_addr_pair_struct *) malloc(p->srcaddr_len * sizeof * p->srcaddr_pairs);
			memcpy(p->srcaddr_pairs, psrc->srcaddr_pairs,
					p->srcaddr_len * sizeof * p->srcaddr_pairs);
		}
		else
			p->srcaddr_pairs = 0;
		/* copy subprogram list */
		if (psrc->subprogram_arr_len)
		{
			p->subprogram_arr = calloc(psrc->subprogram_arr_len, sizeof * p->subprogram_arr);
			p->subprogram_arr_len = psrc->subprogram_arr_len;
			for (i = 0; i < p->subprogram_arr_len; i ++)
				p->subprogram_arr[i].name = strdup(psrc->subprogram_arr[i].name),
				p->subprogram_arr[i].srcline_nr = psrc->subprogram_arr[i].srcline_nr;
		}

		psrc = psrc->next;
		if (psrc)
			p = p->next = calloc(1, sizeof * p);

	}
	while (psrc);

	return newnode;
}

