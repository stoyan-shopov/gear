
ripped and reworked from http://www.lysator.liu.se/c/ANSI-C-grammar-l.html


%option reentrant
%option noyywrap
%option prefix="c_"
%option header-file="c-lex.h"
%option extra-type="char * *"

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include "c-lex-tokens.h"

static void c_lex_count_and_tokenize(enum C_LEX_TOKEN_ENUM token, yyscan_t scanner);
%}

%%

"//"			{	/*! \todo	check c standard preprocessor
				 *		behavior when there is an escaped
				 *		slash at the end of the line */
				int c, i;
				for (i = 1, c = yyinput(yyscanner); ; i++, c = yyinput(yyscanner))
				{
					if (c == '\\')
					{
						i++;
						if (!yyinput(yyscanner))
							break;
					}
					else if (c == '\n')
						break;
				}
				/* the flex manual dictates:
				 *
				 * Actions are free to modify `yyleng' except they should not do so if
				 * the action also includes use of `yymore()'... 
				 *
				 * which yymore() is here not used
				 */
				yyleng = c_get_leng(yyscanner) /* should equal two at all times */
						+ i;
				c_lex_count_and_tokenize(C_TOKEN_COMMENT, yyscanner);
			}

"/*"			{
			register int c, i;

				/* is_in_comment = true; */
				for ( i = 0 ; ; )
				{
					while ( (c = yyinput(yyscanner)) != '*' && c != EOF )
						/* eat up text of comment */
						i++;

					if ( c == '*' )
					{
						i++;
						while ( (c = yyinput(yyscanner)) == '*' )
							i++;
						i++;
						if ( c == '/' )
						{
							/* is_in_comment = false; */
							/* found the end */
							break;
						}
					}

					if ( c == EOF )
					{
						/* error( "EOF in comment" ); */
						break;
					}
				}
				/* the flex manual dictates:
				 *
				 * Actions are free to modify `yyleng' except they should not do so if
				 * the action also includes use of `yymore()'... 
				 *
				 * which yymore() is here not used
				 */
				yyleng += i;
				c_lex_count_and_tokenize(C_TOKEN_COMMENT, yyscanner);
			}


"auto"			|
"char"			|
"const"			|
"double"		|
"enum"			|
"extern"		|
"float"			|
"int"			|
"long"			|
"register"		|
"short"			|
"signed"		|
"static"		|
"struct"		|
"typedef"		|
"union"			|
"unsigned"		|
"void"			|
"volatile"		{ c_lex_count_and_tokenize(C_TOKEN_TYPE_RELATED_KEYWORD, yyscanner); }

"break"			|
"case"			|
"continue"		|
"default"		|
"do"			|
"else"			|
"for"			|
"goto"			|
"if"			|
"return"		|
"sizeof"		|
"switch"		|
"while"			{ c_lex_count_and_tokenize(C_TOKEN_KEYWORD, yyscanner); }

{L}({L}|{D})*		{ c_lex_count_and_tokenize(C_TOKEN_IDENTIFIER, yyscanner); }

L?'(\\.|[^\\'])+'	{ c_lex_count_and_tokenize(C_TOKEN_CHAR_CONSTANT, yyscanner); }

0[xX]{H}+{IS}?		|
0{D}+{IS}?		|
{D}+{IS}?		{ c_lex_count_and_tokenize(C_TOKEN_CONSTANT, yyscanner); }

{D}+{E}{FS}?		|
{D}*"."{D}+({E})?{FS}?	|
{D}+"."{D}*({E})?{FS}?	{ c_lex_count_and_tokenize(C_TOKEN_CONSTANT, yyscanner); }

L?\"			{
			register int c, i;

				for ( i = 0 ; ; )
				{
					while ( (c = yyinput(yyscanner)) != '"' && c != '\\' && c != EOF )
						/* eat up text of comment */
						i++;

					if ( c == '"' )
					{
						i++;
						break;
					}	
					else if (c == '\\')
					{
						/* escape next symbol */
						i ++;
						c = yyinput(yyscanner);
						if (c != EOF)
						{
							i ++;
							continue;
						}
					}

					/* c == EOF */
					{
						/* error( "EOF in string literal" ); */
						break;
					}
				}
				/* the flex manual dictates:
				 *
				 * Actions are free to modify `yyleng' except they should not do so if
				 * the action also includes use of `yymore()'... 
				 *
				 * which yymore() is here not used
				 */
				yyleng += i;
				c_lex_count_and_tokenize(C_TOKEN_STRING_LITERAL, yyscanner);
			}

"..."			|
">>="			|
"<<="			|
"+="			|
"-="			|
"*="			|
"/="			|
"%="			|
"&="			|
"^="			|
"|="			|
">>"			|
"<<"			|
"++"			|
"--"			|
"->"			|
"&&"			|
"||"			|
"<="			|
">="			|
"=="			|
"!="			|
";"			|
("{"|"<%")		|
("}"|"%>")		|
","			|
":"			|
"="			|
"("			|
")"			|
("["|"<:")		|
("]"|":>")		|
"."			|
"&"			|
"!"			|
"~"			|
"-"			|
"+"			|
"*"			|
"/"			|
"%"			|
"<"			|
">"			|
"^"			|
"|"			|
"?"			{ c_lex_count_and_tokenize(C_TOKEN_PUNCTUATOR, yyscanner); }

[ \t\v\n\f]		{ c_lex_count_and_tokenize(C_TOKEN_INVALID, yyscanner); }
.			{ c_lex_count_and_tokenize(C_TOKEN_INVALID, yyscanner); }

%%

/*
("#"[^"//""/*"]*)	{ is_in_preproc = true; c_lex_count_and_tokenize(C_TOKEN_PREPROCESSOR); return C_TOKEN_PREPROCESSOR; }
*/

static void c_lex_count_and_tokenize(enum C_LEX_TOKEN_ENUM token, yyscan_t scanner)
{
YY_EXTRA_TYPE p = c_get_extra(scanner);
int i;

	if (!(i = c_get_leng(scanner)))
	{
		printf("lexer fatal error\n");
		while(1);
	}
	memset(*p, token - C_TOKEN_BASE, i);
	*p += i;
}

