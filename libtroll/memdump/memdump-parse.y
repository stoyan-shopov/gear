%error-verbose
%debug
%defines
%define api.pure
%name-prefix = "yy_memdump"
%parse-param { struct memdump_type_struct ** parse_head }
%parse-param { Troll htroll }
%parse-param { void * yyscanner }
%lex-param { Troll htroll }
%lex-param { yyscan_t yyscanner }
%initial-action {/* yydebug_ = 1; */}

%destructor { free($$.buf); } intarr 

%initial-action		{ *parse_head = troll_allocate_record(TROLL_RECORD_MEMDUMP); htroll->is_parse_error = false; htroll->lexpos = 0; }

%{
#include <stdint.h>	
%}

%union
{
	unsigned long	num;
	struct
	{
		int idx, len;
		uint32_t	* buf;
	} intarr;
}

%{
/* pull-in the lexer declaration */	
#include "memdump-common.h"
#include "memdump-lex.h"
static void yyerror(struct memdump_type_struct ** parse_head, Troll unused2, yyscan_t unused3, const char * errmsg);
%}

%token NUM CSTR
%token NR_DATA_ITEMS_KW START_ADDR_KW BYTE_WIDTH_KW DATA_LIST_KW


%type <num>	NUM
%type <intarr>	intarr

%start start_sym

%%

start_sym
	: memdump_record
	;

memdump_record	:	'[' NR_DATA_ITEMS_KW '=' NUM ',' BYTE_WIDTH_KW '=' NUM ',' START_ADDR_KW '=' NUM ',' DATA_LIST_KW '=' '(' intarr ')' ',' ']'
		{
			(*parse_head)->buf_len = $17.idx;
			(*parse_head)->buf = $17.buf;
			if ($8 != 4)
			{
				yyerror(parse_head, htroll, 0, "memdump byte size must be equal to 4");
				YYERROR;
			}
			if ($4 != $17.idx)
			{
				yyerror(parse_head, htroll, 0, "bad memdump size (different from what was actually read)");
				YYERROR;
			}
			(*parse_head)->start_addr = $12;
		}
		;


intarr		:
		NUM ','		{ $$.buf = calloc($$.len = 16, sizeof * $$.buf); $$.buf[0] = $1; $$.idx = 1; }
		| intarr NUM ','
			{
				$$ = $1;
				if ($$.idx == $$.len)
					$$.buf = realloc($$.buf, ($$.len *= 2) * sizeof * $$.buf);
				$$.buf[$$.idx++] = $2;
			}
		;

%%

static void yyerror(struct memdump_type_struct ** parse_head, Troll htroll, yyscan_t unused3, const char * errmsg)
{
	htroll->is_parse_error = true;
	troll_deallocate_record(*parse_head);
	*parse_head = 0;
	printf("parse error:\n");
	printf("%s\n", errmsg);
	printf("lexpos is %i\n", htroll->lexpos);
}

void memdump_destroy(struct memdump_type_struct * node)
{
	if (!node)
		return;
	if (node->buf)
		free(node->buf);
	free(node);
}

struct memdump_type_struct * memdump_clone(struct memdump_type_struct * src)
{
struct memdump_type_struct * p;

	if (!src)
		return 0;
	if (!(p = calloc(1, sizeof * p)))
		return 0;
	* p = * src;
	troll_clone_parse_type_common_struct(&p->head, &src->head);

	if (src->buf)
	{
		if (!(p->buf = calloc(p->buf_len, sizeof * p->buf)))
		{
			free(p);
			return 0;
		}
		memcpy(p->buf, src->buf, p->buf_len * sizeof * p->buf);
	}

	return p;
}

