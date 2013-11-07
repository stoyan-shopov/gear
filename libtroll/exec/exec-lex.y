
/* %option c++ */
%option header-file="exec-lex.h"
%option prefix="yy_exec"
%option noyywrap
%option reentrant

%{
#include <stdio.h>
#include <string.h>

#include <libtroll.h>
#include "panic.h"
#include "exec-parse.tab.h"
#include "exec-common.h"

static void yycount(Troll htroll, int len)
{
	htroll->lexpos += len;
}

%}


%%

"NEW_TARGET_STATE"	{ yycount(htroll, yyleng); return NEW_TARGET_STATE; }
"RUNNING"		{ yycount(htroll, yyleng); yylval->target_state = TARGET_CORE_STATE_RUNNING;
				return STATE_RUNNING; }
"STOPPED"		{ yycount(htroll, yyleng); yylval->target_state = TARGET_CORE_STATE_HALTED;
				return STATE_STOPPED; }
"HALT_ADDR"		{ yycount(htroll, yyleng); return HALT_ADDR; }
"HALT_COMP_UNIT"	{ yycount(htroll, yyleng); return HALT_COMP_UNIT; }
"HALT_SUBPROGRAM"	{ yycount(htroll, yyleng); return HALT_SUBPROGRAM; }
"HALT_SRCFILE"		{ yycount(htroll, yyleng); return HALT_SRCFILE; }
"HALT_SRCLINE_NR"	{ yycount(htroll, yyleng); return HALT_SRCLINE_NR; }

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
"["		{ yycount(htroll, yyleng); return '['; }
"]"		{ yycount(htroll, yyleng); return ']'; }
[ \t\v\f]		{ yycount(htroll, yyleng); }
.		{ yycount(htroll, yyleng); printf("invalid character: %c\n", *yytext); return * yytext; }

