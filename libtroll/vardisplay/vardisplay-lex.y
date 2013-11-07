
/* %option c++ */
%option header-file="vardisplay-lex.h"
%option prefix="yy_vardisplay"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "vardisplay-types.h"
#include "vardisplay-parse.tab.h"
#include "vardisplay-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"TYPENAME"	{ yycount(htroll, yyleng); return TYPENAME; }
"RAWTYPE"	{ yycount(htroll, yyleng); return RAWTYPE; }
"CHILDLIST"	{ yycount(htroll, yyleng); return CHILDLIST; }
"NAME"		{ yycount(htroll, yyleng); return NAME; }
"ARRDIMS"	{ yycount(htroll, yyleng); return ARRDIMS; }
[0-9]+		{ yycount(htroll, yyleng); yylval->num = strtol(yytext, 0, 0);
		return NUM; }
\"([^\"]|"\\\"")*\"	{
		int c;
		yycount(htroll, yyleng); 
		yytext[yyleng - 1] = 0;
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		yylval->cstr = cstr;
		return CSTR; }
"DEREF_POINT"	{ yycount(htroll, yyleng); return DEREF_POINT; }
"VALUE_LIST"	{ yycount(htroll, yyleng); return VALUE_LIST; }
","		{ yycount(htroll, yyleng); return ','; }
"("		{ yycount(htroll, yyleng); return '('; }
")"		{ yycount(htroll, yyleng); return ')'; }
"["		{ yycount(htroll, yyleng); return '['; }
"]"		{ yycount(htroll, yyleng); return ']'; }
"="		{ yycount(htroll, yyleng); return '='; }
"$"		{ yycount(htroll, yyleng); return '$'; }

[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return * yytext; }

