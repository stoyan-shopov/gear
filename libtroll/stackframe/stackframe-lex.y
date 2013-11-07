
/* %option c++ */
%option header-file="stackframe-lex.h"
%option prefix="yy_stackframe"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "stackframe-types.h"
#include "stackframe-parse.tab.h"
#include "stackframe-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"PC_ADDR"		{ yycount(htroll, yyleng); return PC_ADDR; }
"COMP_UNIT"	{ yycount(htroll, yyleng); return COMP_UNIT; }
"SUBPROGRAM"	{ yycount(htroll, yyleng); return SUBPROGRAM; }
"SRCFILE"		{ yycount(htroll, yyleng); return SRCFILE; }
"SRCLINE_NR"	{ yycount(htroll, yyleng); return SRCLINE_NR; }
"SELECTED_FRAME"	{ yycount(htroll, yyleng); return SELECTED_FRAME; }

0[xX][a-fA-F0-9]+	{ yycount(htroll, yyleng); yylval->addr = strtoul(yytext, 0, 0);
		return NUM; }
[0-9]+		{ yycount(htroll, yyleng); yylval->addr = strtoul(yytext, 0, 0);
		return NUM; }
\"([^\"]|"\\\"")*\"	{ yycount(htroll, yyleng);
		yytext[yyleng - 1] = 0;
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		yylval->cstr = cstr;
		return CSTR; }


"="		{ yycount(htroll, yyleng); return '='; }
","		{ yycount(htroll, yyleng); return ','; }
"("		{ yycount(htroll, yyleng); return '('; }
")"		{ yycount(htroll, yyleng); return ')'; }
"["		{ yycount(htroll, yyleng); return '['; }
"]"		{ yycount(htroll, yyleng); return ']'; }
[ \t\v\f]	{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return * yytext; }

