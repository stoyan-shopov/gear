/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SIZEOF = 258,
     PTR_OP = 259,
     INC_OP = 260,
     DEC_OP = 261,
     LEFT_OP = 262,
     RIGHT_OP = 263,
     LE_OP = 264,
     GE_OP = 265,
     EQ_OP = 266,
     NE_OP = 267,
     AND_OP = 268,
     OR_OP = 269,
     MUL_ASSIGN = 270,
     DIV_ASSIGN = 271,
     MOD_ASSIGN = 272,
     ADD_ASSIGN = 273,
     SUB_ASSIGN = 274,
     LEFT_ASSIGN = 275,
     RIGHT_ASSIGN = 276,
     AND_ASSIGN = 277,
     XOR_ASSIGN = 278,
     OR_ASSIGN = 279,
     TYPE_NAME = 280,
     TYPEDEF = 281,
     EXTERN = 282,
     STATIC = 283,
     AUTO = 284,
     REGISTER = 285,
     CHAR_T = 286,
     SHORT_T = 287,
     INT_T = 288,
     LONG_T = 289,
     SIGNED = 290,
     UNSIGNED = 291,
     FLOAT_T = 292,
     DOUBLE_T = 293,
     CONST_T = 294,
     VOLATILE = 295,
     VOID_T = 296,
     STRUCT = 297,
     UNION = 298,
     ENUM = 299,
     ELLIPSIS = 300,
     CASE = 301,
     DEFAULT = 302,
     IF = 303,
     ELSE = 304,
     SWITCH = 305,
     WHILE = 306,
     DO = 307,
     FOR = 308,
     GOTO = 309,
     CONTINUE = 310,
     BREAK = 311,
     RETURN = 312,
     IDENTIFIER = 313,
     CONSTANT = 314,
     STRING_LITERAL = 315
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 48 "c-parse.y"

	const char	* str;
	struct { enum CONST_KIND kind; union { signed long long iconst; long double flconst; }; } const_info;
	struct primary_expr	* primary_expr;
	struct expr		* expr;
	enum EXPR_OP		assignment_op;
	struct cond_expr	* cond_expr;
	struct unary_expr	* unary_expr;
	struct cast_expr	* cast_expr;
	enum UNARY_OP		unary_op;
	struct postfix_expr	* postfix_expr;
	struct arg_expr_list	* args;
	struct logical_or_expr	* logical_or_expr;
	struct logical_and_expr	* logical_and_expr;
	struct incl_or_expr	* incl_or_expr;
	struct xor_expr		* xor_expr;
	struct and_expr		* and_expr;
	struct eq_expr		* eq_expr;
	struct rel_expr		* rel_expr;
	struct shift_expr	* shift_expr;
	struct additive_expr		* additive_expr;
	struct mult_expr	* mult_expr;


	struct type_name	* type_name;
	struct type_spec_list	* type_spec_list;
	struct type_spec	type_spec;
	struct abstract_decl	* abstract_decl;
	struct pointer_spec	* pointer_spec;
	struct direct_abstract_decl	* direct_abstract_decl;



/* Line 1685 of yacc.c  */
#line 145 "c-tokens.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


