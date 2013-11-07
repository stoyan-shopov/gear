
/* %option c++ */
%option header-file="memdump-lex.h"
%option prefix="yy_memdump"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "memdump-types.h"
#include "memdump-parse.tab.h"
#include "memdump-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"NR_DATA_ITEMS"		{ yycount(htroll, yyleng); return NR_DATA_ITEMS_KW; }
"BYTE_WIDTH"		{ yycount(htroll, yyleng); return BYTE_WIDTH_KW; }
"DATA_LIST"		{ yycount(htroll, yyleng); return DATA_LIST_KW; }
"START_ADDR"		{ yycount(htroll, yyleng); return START_ADDR_KW; }

0[xX][a-fA-F0-9]+	{ yycount(htroll, yyleng); yylval->num = strtoul(yytext, 0, 0);
		return NUM; }
[0-9]+		{ yycount(htroll, yyleng); yylval->num = strtoul(yytext, 0, 0);
		return NUM; }
"="		{ yycount(htroll, yyleng); return '='; }
"["		{ yycount(htroll, yyleng); return '['; }
"]"		{ yycount(htroll, yyleng); return ']'; }
"("		{ yycount(htroll, yyleng); return '('; }
")"		{ yycount(htroll, yyleng); return ')'; }
","		{ yycount(htroll, yyleng); return ','; }

[ \t\v\f]		{ }
.		{  yycount(htroll, yyleng); printf("invalid character: %c(0x%02x)\n", *yytext, *yytext); return * yytext; }

