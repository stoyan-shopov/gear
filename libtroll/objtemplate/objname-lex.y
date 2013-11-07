
/* %option c++ */
%option header-file="objname-lex.h"
%option prefix="yy_objprefix"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "objname-parse.tab.h"
#include "objname-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"KEYWORD"	{ yycount(htroll, yyleng); return KEYWORD; }
"ERROR"		{ yycount(htroll, yyleng); return ERROR; }
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
"$"		{ yycount(htroll, yyleng); return '$'; }

[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return * yytext; }

