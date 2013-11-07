
/* %option c++ */
%option header-file="bkpt-lex.h"
%option prefix="yy_bkpt"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "bkpt-types.h"
#include "panic.h"
#include "bkpt-parse.tab.h"
#include "bkpt-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"BREAKPOINT_INSERTED"	{ yycount(htroll, yyleng); return BREAKPOINT_INSERTED_KW; }
"BREAKPOINT_REMOVED"	{ yycount(htroll, yyleng); return BREAKPOINT_REMOVED_KW; }
"BREAKPOINT_LIST"	{ yycount(htroll, yyleng); return BREAKPOINT_LIST_KW; }
"ADDR"		{ yycount(htroll, yyleng); return ADDR_KW; }
"SRCFILE"	{ yycount(htroll, yyleng); return SRCFILE_KW; }
"SRCLINE"	{ yycount(htroll, yyleng); return SRCLINE_KW; }
"IS_AT_START_OF_STATEMENT"	{ yycount(htroll, yyleng); return IS_AT_START_OF_STATEMENT_KW; }

0[xX][a-fA-F0-9]+	{ yycount(htroll, yyleng); yylval->num = strtoul(yytext, 0, 0);
		return NUM; }
[0-9]+		{ yycount(htroll, yyleng); yylval->num = strtoul(yytext, 0, 0);
		return NUM; }
\"([^\"]|"\\\"")*\"	{ yycount(htroll, yyleng); 
		yytext[yyleng - 1] = '\0';
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		yylval->cstr = cstr;
		return CSTR; }
"("		{ yycount(htroll, yyleng); return '('; }
")"		{ yycount(htroll, yyleng); return ')'; }
"["		{ yycount(htroll, yyleng); return '['; }
"]"		{ yycount(htroll, yyleng); return ']'; }
","		{ yycount(htroll, yyleng); return ','; }
"="		{ yycount(htroll, yyleng); return '='; }

[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return *yytext; }

