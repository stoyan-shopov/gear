
/* %option c++ */
%option header-file="srcinfo-lex.h"
%option prefix="yy_srcinfo"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include "panic.h"
#include "srcinfo-types.h"
#include "srcinfo-parse.tab.h"
#include "srcinfo-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

\"([^\"]|"\\\"")*\"	{
		yycount(htroll, yyleng);
		yytext[yyleng - 1] = '\0';
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		yylval->cstr = cstr;
		return CSTR; }
"COMPILATION_UNITS"	{ yycount(htroll, yyleng); return COMPILATION_UNITS_KW; }
"CU_NAME"	{ yycount(htroll, yyleng); return CU_NAME_KW; }
"CU_COMPILATION_DIR"	{ yycount(htroll, yyleng); return CU_COMPILATION_DIR_KW; }
"SOURCE_FILES_DATA"	{ yycount(htroll, yyleng); return SOURCE_FILES_DATA_KW; }
"SRC_FILE_NAME"	{ yycount(htroll, yyleng); return SRC_FILE_NAME_KW; }
"INCLUSIVE_SPANNED_ADDR_RANGE"	{ yycount(htroll, yyleng); return INCLUSIVE_SPANNED_ADDR_RANGE_KW; }
"SPANNED_ADDR_RANGES"		{ yycount(htroll, yyleng); return SPANNED_ADDR_RANGES_KW; }
"BKPT_LINES_BMAP"		{ yycount(htroll, yyleng); return BKPT_LINES_BMAP_KW; }
"SUBPROGRAM_LIST"	{ yycount(htroll, yyleng); return SUBPROGRAM_LIST_KW; }
"SUBPROGRAM_NAME"	{ yycount(htroll, yyleng); return SUBPROGRAM_NAME_KW; }
"DECL_LINE"		{ yycount(htroll, yyleng); return DECL_LINE_KW; }
"SRCLINE_LIST"		{ yycount(htroll, yyleng); return SRCLINE_LIST_KW; }

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

[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{  yycount(htroll, yyleng); printf("invalid character: %c(0x%02x)\n", *yytext, *yytext); return * yytext; }

