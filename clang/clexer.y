
%option noyywrap

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "clang.h"
#include "parser.tab.h"
#include "clang-parser-common.h"

#if __CXX__
using namespace clang_parser;

static enum /*parser::token::*//*yytokentype*/int
#else
#define yyinput input
static int
#endif
check_type();

static void count(
#if __CXX__
location
#else
YYLTYPE
#endif
* yylloc);
static void comment();

static void convert_constant(YYSTYPE * yylval);

#if __CXX__
#define yyterminate() return (clang_parser::parser::token::yytokentype) YY_NULL
#else
#define yyterminate() return YY_NULL
#endif
%}

%%
"/*"			{ comment(); }

"auto"			{ count(yylloc); return(/*parser::token::*/AUTO); }
"break"			{ count(yylloc); return(/*parser::token::*/BREAK); }
"case"			{ count(yylloc); return(/*parser::token::*/CASE); }
"char"			{ count(yylloc); return(/*parser::token::*/CHAR); }
"const"			{ count(yylloc); return(/*parser::token::*/CONST); }
"continue"		{ count(yylloc); return(/*parser::token::*/CONTINUE); }
"default"		{ count(yylloc); return(/*parser::token::*/DEFAULT); }
"do"			{ count(yylloc); return(/*parser::token::*/DO); }
"double"		{ count(yylloc); return(/*parser::token::*/DOUBLE); }
"else"			{ count(yylloc); return(/*parser::token::*/ELSE); }
"enum"			{ count(yylloc); return(/*parser::token::*/ENUM); }
"extern"		{ count(yylloc); return(/*parser::token::*/EXTERN); }
"float"			{ count(yylloc); return(/*parser::token::*/FLOAT); }
"for"			{ count(yylloc); return(/*parser::token::*/FOR); }
"goto"			{ count(yylloc); return(/*parser::token::*/GOTO); }
"if"			{ count(yylloc); return(/*parser::token::*/IF); }
"int"			{ count(yylloc); return(/*parser::token::*/INT); }
"long"			{ count(yylloc); return(/*parser::token::*/LONG); }
"register"		{ count(yylloc); return(/*parser::token::*/REGISTER); }
"return"		{ count(yylloc); return(/*parser::token::*/RETURN); }
"short"			{ count(yylloc); return(/*parser::token::*/SHORT); }
"signed"		{ count(yylloc); return(/*parser::token::*/SIGNED); }
"sizeof"		{ count(yylloc); return(/*parser::token::*/SIZEOF); }
"static"		{ count(yylloc); return(/*parser::token::*/STATIC); }
"struct"		{ count(yylloc); return(/*parser::token::*/STRUCT); }
"switch"		{ count(yylloc); return(/*parser::token::*/SWITCH); }
"typedef"		{ count(yylloc); return(/*parser::token::*/TYPEDEF); }
"union"			{ count(yylloc); return(/*parser::token::*/UNION); }
"unsigned"		{ count(yylloc); return(/*parser::token::*/UNSIGNED); }
"void"			{ count(yylloc); return(/*parser::token::*/VOID); }
"volatile"		{ count(yylloc); return(/*parser::token::*/VOLATILE); }
"while"			{ count(yylloc); return(/*parser::token::*/WHILE); }

{L}({L}|{D})*		{ count(yylloc); yylval->ident = strdup(yytext); return(check_type()); }

0[xX]{H}+{IS}?		{ convert_constant(yylval); count(yylloc); return(/*parser::token::*/CONSTANT); }
0{D}+{IS}?		{ convert_constant(yylval); count(yylloc); return(/*parser::token::*/CONSTANT); }
{D}+{IS}?		{ convert_constant(yylval); count(yylloc); return(/*parser::token::*/CONSTANT); }
L?'(\\.|[^\\'])+'	{ panic(""); count(yylloc); return(/*parser::token::*/CONSTANT); }

{D}+{E}{FS}?		{ panic(""); count(yylloc); return(/*parser::token::*/CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ panic(""); count(yylloc); return(/*parser::token::*/CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ panic(""); count(yylloc); return(/*parser::token::*/CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ panic(""); count(yylloc); return(/*parser::token::*/STRING_LITERAL); }

"..."			{ count(yylloc); return(/*parser::token::*/ELLIPSIS); }
">>="			{ count(yylloc); return(/*parser::token::*/RIGHT_ASSIGN); }
"<<="			{ count(yylloc); return(/*parser::token::*/LEFT_ASSIGN); }
"+="			{ count(yylloc); return(/*parser::token::*/ADD_ASSIGN); }
"-="			{ count(yylloc); return(/*parser::token::*/SUB_ASSIGN); }
"*="			{ count(yylloc); return(/*parser::token::*/MUL_ASSIGN); }
"/="			{ count(yylloc); return(/*parser::token::*/DIV_ASSIGN); }
"%="			{ count(yylloc); return(/*parser::token::*/MOD_ASSIGN); }
"&="			{ count(yylloc); return(/*parser::token::*/AND_ASSIGN); }
"^="			{ count(yylloc); return(/*parser::token::*/XOR_ASSIGN); }
"|="			{ count(yylloc); return(/*parser::token::*/OR_ASSIGN); }
">>"			{ count(yylloc); return(/*parser::token::*/RIGHT_OP); }
"<<"			{ count(yylloc); return(/*parser::token::*/LEFT_OP); }
"++"			{ count(yylloc); return(/*parser::token::*/INC_OP); }
"--"			{ count(yylloc); return(/*parser::token::*/DEC_OP); }
"->"			{ count(yylloc); return(/*parser::token::*/PTR_OP); }
"&&"			{ count(yylloc); return(/*parser::token::*/AND_OP); }
"||"			{ count(yylloc); return(/*parser::token::*/OR_OP); }
"<="			{ count(yylloc); return(/*parser::token::*/LE_OP); }
">="			{ count(yylloc); return(/*parser::token::*/GE_OP); }
"=="			{ count(yylloc); return(/*parser::token::*/EQ_OP); }
"!="			{ count(yylloc); return(/*parser::token::*/NE_OP); }
";"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)';'); }
("{"|"<%")		{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'{'); }
("}"|"%>")		{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'}'); }
","			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)','); }
":"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)':'); }
"="			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'='); }
"("			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'('); }
")"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)')'); }
("["|"<:")		{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'['); }
("]"|":>")		{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)']'); }
"."			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'.'); }
"&"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'&'); }
"!"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'!'); }
"~"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'~'); }
"-"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'-'); }
"+"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'+'); }
"*"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'*'); }
"/"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'/'); }
"%"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'%'); }
"<"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'<'); }
">"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'>'); }
"^"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'^'); }
"|"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'|'); }
"?"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'?'); }
"@"			{ count(yylloc); return((/*parser::token::*//*yytokentype*/int)'@'); }

[ \t\v\n\f]		{ count(yylloc); }
.			{ /* ignore bad characters */ }

%%

/*
yywrap()
{
	return(1);
}
*/

static void comment()
{
	char c, c1;

loop:
	while ((c = yyinput()) != '*' && c != 0)
		putchar(c);

	if ((c1 = yyinput()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

	if (c != 0)
		putchar(c1);
}

/*
static int column = 0;
*/

static void count(
#if __CXX__
location
#else
YYLTYPE
#endif
* yylloc)
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
#if __CXX__
			yylloc->begin.column = 0;
#else
			yylloc->first_column = 0;
#endif
		else if (yytext[i] == '\t')
#if __CXX__
			yylloc->begin.column += 8 - (yylloc->begin.column % 8);
#else
			yylloc->first_column += 8 - (yylloc->first_column % 8);
#endif
		else
#if __CXX__
			yylloc->begin.column++;
#else
			yylloc->first_column++;
#endif

	ECHO;
}


#if __CXX__
enum /*parser::token::*//*yytokentype*/int check_type()
#else
int
#endif
check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

/*
*	it actually will only return IDENTIFIER
*/

#if __CXX__
	return(/*parser::token::*/IDENTIFIER);
#else
	return(IDENTIFIER);
#endif
}

static void convert_constant(YYSTYPE * yylval)
{
long int l;

	errno = 0;
	l = strtol(yytext, 0, 0);
	if (errno)
		panic("");
	yylval->const_val = l;
}
