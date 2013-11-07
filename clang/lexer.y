
%option noyywrap

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include "parser.tab.h"
#include "clang-parser-common.h"

using namespace clang_parser;

enum parser::token::yytokentype check_type();

static void count(location * yylloc);
static void comment();

#define yyterminate() return (clang_parser::parser::token::yytokentype) YY_NULL;
%}

%%
"/*"			{ comment(); }

"auto"			{ count(yylloc); return(parser::token::AUTO); }
"break"			{ count(yylloc); return(parser::token::BREAK); }
"case"			{ count(yylloc); return(parser::token::CASE); }
"char"			{ count(yylloc); return(parser::token::CHAR); }
"const"			{ count(yylloc); return(parser::token::CONST); }
"continue"		{ count(yylloc); return(parser::token::CONTINUE); }
"default"		{ count(yylloc); return(parser::token::DEFAULT); }
"do"			{ count(yylloc); return(parser::token::DO); }
"double"		{ count(yylloc); return(parser::token::DOUBLE); }
"else"			{ count(yylloc); return(parser::token::ELSE); }
"enum"			{ count(yylloc); return(parser::token::ENUM); }
"extern"		{ count(yylloc); return(parser::token::EXTERN); }
"float"			{ count(yylloc); return(parser::token::FLOAT); }
"for"			{ count(yylloc); return(parser::token::FOR); }
"goto"			{ count(yylloc); return(parser::token::GOTO); }
"if"			{ count(yylloc); return(parser::token::IF); }
"int"			{ count(yylloc); return(parser::token::INT); }
"long"			{ count(yylloc); return(parser::token::LONG); }
"register"		{ count(yylloc); return(parser::token::REGISTER); }
"return"		{ count(yylloc); return(parser::token::RETURN); }
"short"			{ count(yylloc); return(parser::token::SHORT); }
"signed"		{ count(yylloc); return(parser::token::SIGNED); }
"sizeof"		{ count(yylloc); return(parser::token::SIZEOF); }
"static"		{ count(yylloc); return(parser::token::STATIC); }
"struct"		{ count(yylloc); return(parser::token::STRUCT); }
"switch"		{ count(yylloc); return(parser::token::SWITCH); }
"typedef"		{ count(yylloc); return(parser::token::TYPEDEF); }
"union"			{ count(yylloc); return(parser::token::UNION); }
"unsigned"		{ count(yylloc); return(parser::token::UNSIGNED); }
"void"			{ count(yylloc); return(parser::token::VOID); }
"volatile"		{ count(yylloc); return(parser::token::VOLATILE); }
"while"			{ count(yylloc); return(parser::token::WHILE); }

{L}({L}|{D})*		{ count(yylloc); return(check_type()); }

0[xX]{H}+{IS}?		{ count(yylloc); return(parser::token::CONSTANT); }
0{D}+{IS}?		{ count(yylloc); return(parser::token::CONSTANT); }
{D}+{IS}?		{ count(yylloc); return(parser::token::CONSTANT); }
L?'(\\.|[^\\'])+'	{ count(yylloc); return(parser::token::CONSTANT); }

{D}+{E}{FS}?		{ count(yylloc); return(parser::token::CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ count(yylloc); return(parser::token::CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ count(yylloc); return(parser::token::CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ count(yylloc); return(parser::token::STRING_LITERAL); }

"..."			{ count(yylloc); return(parser::token::ELLIPSIS); }
">>="			{ count(yylloc); return(parser::token::RIGHT_ASSIGN); }
"<<="			{ count(yylloc); return(parser::token::LEFT_ASSIGN); }
"+="			{ count(yylloc); return(parser::token::ADD_ASSIGN); }
"-="			{ count(yylloc); return(parser::token::SUB_ASSIGN); }
"*="			{ count(yylloc); return(parser::token::MUL_ASSIGN); }
"/="			{ count(yylloc); return(parser::token::DIV_ASSIGN); }
"%="			{ count(yylloc); return(parser::token::MOD_ASSIGN); }
"&="			{ count(yylloc); return(parser::token::AND_ASSIGN); }
"^="			{ count(yylloc); return(parser::token::XOR_ASSIGN); }
"|="			{ count(yylloc); return(parser::token::OR_ASSIGN); }
">>"			{ count(yylloc); return(parser::token::RIGHT_OP); }
"<<"			{ count(yylloc); return(parser::token::LEFT_OP); }
"++"			{ count(yylloc); return(parser::token::INC_OP); }
"--"			{ count(yylloc); return(parser::token::DEC_OP); }
"->"			{ count(yylloc); return(parser::token::PTR_OP); }
"&&"			{ count(yylloc); return(parser::token::AND_OP); }
"||"			{ count(yylloc); return(parser::token::OR_OP); }
"<="			{ count(yylloc); return(parser::token::LE_OP); }
">="			{ count(yylloc); return(parser::token::GE_OP); }
"=="			{ count(yylloc); return(parser::token::EQ_OP); }
"!="			{ count(yylloc); return(parser::token::NE_OP); }
";"			{ count(yylloc); return((parser::token::yytokentype)';'); }
("{"|"<%")		{ count(yylloc); return((parser::token::yytokentype)'{'); }
("}"|"%>")		{ count(yylloc); return((parser::token::yytokentype)'}'); }
","			{ count(yylloc); return((parser::token::yytokentype)','); }
":"			{ count(yylloc); return((parser::token::yytokentype)':'); }
"="			{ count(yylloc); return((parser::token::yytokentype)'='); }
"("			{ count(yylloc); return((parser::token::yytokentype)'('); }
")"			{ count(yylloc); return((parser::token::yytokentype)')'); }
("["|"<:")		{ count(yylloc); return((parser::token::yytokentype)'['); }
("]"|":>")		{ count(yylloc); return((parser::token::yytokentype)']'); }
"."			{ count(yylloc); return((parser::token::yytokentype)'.'); }
"&"			{ count(yylloc); return((parser::token::yytokentype)'&'); }
"!"			{ count(yylloc); return((parser::token::yytokentype)'!'); }
"~"			{ count(yylloc); return((parser::token::yytokentype)'~'); }
"-"			{ count(yylloc); return((parser::token::yytokentype)'-'); }
"+"			{ count(yylloc); return((parser::token::yytokentype)'+'); }
"*"			{ count(yylloc); return((parser::token::yytokentype)'*'); }
"/"			{ count(yylloc); return((parser::token::yytokentype)'/'); }
"%"			{ count(yylloc); return((parser::token::yytokentype)'%'); }
"<"			{ count(yylloc); return((parser::token::yytokentype)'<'); }
">"			{ count(yylloc); return((parser::token::yytokentype)'>'); }
"^"			{ count(yylloc); return((parser::token::yytokentype)'^'); }
"|"			{ count(yylloc); return((parser::token::yytokentype)'|'); }
"?"			{ count(yylloc); return((parser::token::yytokentype)'?'); }

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

static void count(location * yylloc)
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			yylloc->begin.column = 0;
		else if (yytext[i] == '\t')
			yylloc->begin.column += 8 - (yylloc->begin.column % 8);
		else
			yylloc->begin.column++;

	ECHO;
}


enum parser::token::yytokentype check_type()
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

	return(parser::token::IDENTIFIER);
}

