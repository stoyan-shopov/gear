
/* %option c++ */
%option header-file="regview-lex.h"
%option prefix="yy_regview"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "regview-parse.tab.h"
#include "regview-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

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
"="		{ yycount(htroll, yyleng); return '='; }
"("		{ yycount(htroll, yyleng); return '('; }
")"		{ yycount(htroll, yyleng); return ')'; }
","		{ yycount(htroll, yyleng); return ','; }

[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return *yytext; }

