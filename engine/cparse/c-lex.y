

http://www.lysator.liu.se/c/


%option noyywrap
%option reentrant

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{

static void count(void);
static int check_type(void);

int c_lex_column;

/*! lexer input character reading macro
 *
 * \todo	this advances the lexer buffer (c_lang_lexbuf), which
 *		will become an issue if the lexer buffer is needed
 *		outside of here */ 
#define YY_INPUT(buf, result, max_size) \
{\
int i;\
	/* it should not be possible that this ever gets invoked, thus
	 * the infinite loop here */\
	while (1);\
	/*i = 0;*/\
	/*while (i < max_size && **c_lang_lexbuf)*/\
	/*	buf[i++] = **c_lang_lexbuf++;*/\
	/*result = i ? i : YY_NULL;*/\
}
%}

%%
"/*"			{ panic("");
			{
				char c, c1;

			loop:
				while ((c = input(yyscanner)) != '*' && c != 0)
					putchar(c);

				if ((c1 = input(yyscanner)) != '/' && c != 0)
				{
					unput(c1);
					goto loop;
				}

				if (c != 0)
					putchar(c1);
			}
			}


"auto"			{ count(); return(AUTO); }
"break"			{ count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); return(CHAR); }
"const"			{ count(); return(CONST); }
"continue"		{ count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"enum"			{ count(); return(ENUM); }
"extern"		{ count(); return(EXTERN); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"return"		{ count(); return(RETURN); }
"short"			{ count(); return(SHORT); }
"signed"		{ count(); return(SIGNED); }
"sizeof"		{ count(); return(SIZEOF); }
"static"		{ count(); return(STATIC); }
"struct"		{ count(); return(STRUCT); }
"switch"		{ count(); return(SWITCH); }
"typedef"		{ count(); return(TYPEDEF); }
"union"			{ count(); return(UNION); }
"unsigned"		{ count(); return(UNSIGNED); }
"void"			{ count(); return(VOID); }
"volatile"		{ count(); return(VOLATILE); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*		{
		count();
		/*! \todo	process conversion errors here */
		c_lang_lval->name = strdup(yytext);

		return check_type(); 
	}


${L}({L}|{D})*		{
		/* an extension to make possible to use register
		 * values in expressions; a register is specified
		 * as $reg_name; if reg_name is a recognized register
		 * name, the register is read and the lexer returns
		 * a CONSTANT token is returned to the parser;
		 * if the register is not recognized, a '$' token
		 * is returned, which will cause the parser to report
		 * an error
		 *
		 *	\todo	here, pc is represented as register 16,
		 *		cpsr - as register 17, and spsr as
		 *		register 18, and banked register
		 *		access is currently unsupported;
		 *		maybe fix these when the arm rdi based
		 *		code is wiped away
		 *		
		 *
		 *	\note	the list of recognized registers is:
		 *		r0-r15, pc - as an alias of r15,
		 *		lr - as an alias of r14, sp - as
		 *		an alias for r13, cpsr and spsr
		 *
		 *	\todo	maybe represent registers in a somewhat
		 *		different way (not as CONSTANT-s), so
		 *		that assignment to registers becomes
		 *		possible, should be a somewhat special
		 *		case as registers do not really have
		 *		an address
		 *	\todo	right now, when a register is not
		 *		recognized, the function panics; fix this
		 *	\todo	currently, register values are always
		 *		treated as signed (and therefore, sign
		 *		extension is performed in the conversions);
		 *		this may turn out to be not always correct
		 *	\todo	also support floating point registers
		 *		when the time has come
		 */
		int reg_nr, i;
		reg_nr = 0;
		ARM_CORE_WORD reg;

		count();
		/* change all letters to lower case */
		for (i = 0; i < yyleng; i++)
			yytext[i] = tolower(yytext[i]);

		/* here, yyleng >= 2 */
		if (yytext[1] == 'r')
		{
			/* check for a number specified register */
			if (yyleng < 3 || yyleng > 4)
			       panic("");
			reg_nr = yytext[2];
			if (!isdigit(reg_nr))
				panic("");
			reg_nr -= '0';
			if (yytext[3])
			{
				if (!isdigit(yytext[3]))
					panic("");
				reg_nr *= 10;
				reg_nr += yytext[3] - '0';
			}
		}
		else if (yytext[1] == 'p')
		{
			/* look for 'pc' */
			if (yyleng != 3 || yytext[2] != 'c')
				panic("");
			reg_nr = 15;
		}
		else if (yytext[1] == 'l')
		{
			/* look for 'lr' */
			if (yyleng != 3 || yytext[2] != 'r')
				panic("");
			reg_nr = 14;
		}
		else if (yytext[1] == 's')
		{
			/* look for either 'sp' or 'spsr' */
			if (yyleng == 3)
			{
				/* look for 'sp' */
				if (yytext[2] != 'p')
					panic("");
				reg_nr = 13;
			}
			else if (yyleng == 5)
			{
				/* look for 'spsr' */
				if (yytext[2] != 'p'
						|| yytext[3] != 's'
						|| yytext[4] != 'r')
					reg_nr = 18;
			}
			else
				panic(""); 
		}
		else if (yytext[1] == 'c')
		{
			/* look for 'cpsr' */
			if (yyleng == 6 && (yytext[2] == 'p'
						|| yytext[3] != 's'
						|| yytext[4] != 'r'))
					reg_nr = 17;
			else
				panic(""); 
		}
		/*! \todo	handle mode correctly */
		if (state->ctx->cc->core_reg_read(state->ctx, 0, 1 << reg_nr, &reg)
			!= GEAR_ERR_NO_ERROR)
			panic("");
		c_lang_lval->num = (unsigned long)(signed int) reg;
		return CONSTANT;
	}


0[xX]{H}+{IS}?		{
		count();
		/*! \todo	process conversion errors here */
		c_lang_lval->num = strtoull(yytext, 0, 0);
		return(CONSTANT); 
	}
0{D}+{IS}?		{
		count();
		/*! \todo	process conversion errors here */
		c_lang_lval->num = strtoull(yytext, 0, 0);
		return(CONSTANT); 
	}
{D}+{IS}?		{ 
		count();
		/*! \todo	process conversion errors here */
		c_lang_lval->num = strtoull(yytext, 0, 0);
		return(CONSTANT); 
	}
L?'(\\.|[^\\'])+'	{ panic(""); count(); return(CONSTANT); }

{D}+{E}{FS}?		{ panic(""); count(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ panic(""); count(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ panic(""); count(); return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ panic(""); count(); return(STRING_LITERAL); }

"..."			{ count(); return(ELLIPSIS); }
">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_SHIFT_OP); }
"<<"			{ count(); return(LEFT_SHIFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ /* ignore bad characters */ }

%%


/*! \todo	fix this */
static void count()
{
	/*
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			c_lex_column = 0;
		else if (yytext[i] == '\t')
			c_lex_column += 8 - (c_lex_column % 8);
		else
			c_lex_column++;
	*/
	/* suppress lexer output */
	/* ECHO; */
}


static int check_type()
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

	return(IDENTIFIER);
}

