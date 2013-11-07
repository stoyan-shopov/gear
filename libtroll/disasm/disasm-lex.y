
/* %option c++ */
%option header-file="disasm-lex.h"
%option prefix="yy_disasm"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "disasm-types.h"
#include "disasm-parse.tab.h"
#include "disasm-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"SRCLINE_NR"	{ yycount(htroll, yyleng); return SRCLINE_NR_KW; }
"SRCFILE_NAME"	{ yycount(htroll, yyleng); return SRCFILE_KW; }
"ADDR"		{ yycount(htroll, yyleng); return ADDR_KW; }
"DISASM"	{ yycount(htroll, yyleng); return DISASM_KW; }
"DISASM_LIST"	{ yycount(htroll, yyleng); return DISASM_LIST_KW; }
"INSTRUCTION_COUNT"	{ yycount(htroll, yyleng); return INSTRUCTION_COUNT_KW; }
"PC"		{ yycount(htroll, yyleng); return PC; }
"DISASM_RANGE"	{ yycount(htroll, yyleng); return DISASM_RANGE_KW; }

0[xX][a-fA-F0-9]+	{ yycount(htroll, yyleng); yylval->num = strtoul(yytext, 0, 0);
		return NUM; }
[0-9]+		{ yycount(htroll, yyleng); yylval->num = strtoul(yytext, 0, 0);
		return NUM; }
\"([^\"]|"\\\""|"\"\"")*\"	{ yycount(htroll, yyleng);
		yytext[yyleng - 1] = 0;
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		yylval->cstr = cstr;
		return CSTR; }
"["		{ yycount(htroll, yyleng); return '['; }
"]"		{ yycount(htroll, yyleng); return ']'; }
"("		{ yycount(htroll, yyleng); return '('; }
")"		{ yycount(htroll, yyleng); return ')'; }
","		{ yycount(htroll, yyleng); return ','; }
"="		{ yycount(htroll, yyleng); return '='; }

[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return * yytext; }

