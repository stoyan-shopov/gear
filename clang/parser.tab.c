/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse clang_parser_parse
#define yylex   clang_parser_lex
#define yyerror clang_parser_error
#define yylval  clang_parser_lval
#define yychar  clang_parser_char
#define yydebug clang_parser_debug
#define yynerrs clang_parser_nerrs
#define yylloc clang_parser_lloc

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     CONSTANT = 259,
     STRING_LITERAL = 260,
     SIZEOF = 261,
     PTR_OP = 262,
     INC_OP = 263,
     DEC_OP = 264,
     LEFT_OP = 265,
     RIGHT_OP = 266,
     LE_OP = 267,
     GE_OP = 268,
     EQ_OP = 269,
     NE_OP = 270,
     AND_OP = 271,
     OR_OP = 272,
     MUL_ASSIGN = 273,
     DIV_ASSIGN = 274,
     MOD_ASSIGN = 275,
     ADD_ASSIGN = 276,
     SUB_ASSIGN = 277,
     LEFT_ASSIGN = 278,
     RIGHT_ASSIGN = 279,
     AND_ASSIGN = 280,
     XOR_ASSIGN = 281,
     OR_ASSIGN = 282,
     TYPE_NAME = 283,
     TYPEDEF = 284,
     EXTERN = 285,
     STATIC = 286,
     AUTO = 287,
     REGISTER = 288,
     CHAR = 289,
     SHORT = 290,
     INT = 291,
     LONG = 292,
     SIGNED = 293,
     UNSIGNED = 294,
     FLOAT = 295,
     DOUBLE = 296,
     CONST = 297,
     VOLATILE = 298,
     VOID = 299,
     STRUCT = 300,
     UNION = 301,
     ENUM = 302,
     ELLIPSIS = 303,
     CASE = 304,
     DEFAULT = 305,
     IF = 306,
     ELSE = 307,
     SWITCH = 308,
     WHILE = 309,
     DO = 310,
     FOR = 311,
     GOTO = 312,
     CONTINUE = 313,
     BREAK = 314,
     RETURN = 315
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define CONSTANT 259
#define STRING_LITERAL 260
#define SIZEOF 261
#define PTR_OP 262
#define INC_OP 263
#define DEC_OP 264
#define LEFT_OP 265
#define RIGHT_OP 266
#define LE_OP 267
#define GE_OP 268
#define EQ_OP 269
#define NE_OP 270
#define AND_OP 271
#define OR_OP 272
#define MUL_ASSIGN 273
#define DIV_ASSIGN 274
#define MOD_ASSIGN 275
#define ADD_ASSIGN 276
#define SUB_ASSIGN 277
#define LEFT_ASSIGN 278
#define RIGHT_ASSIGN 279
#define AND_ASSIGN 280
#define XOR_ASSIGN 281
#define OR_ASSIGN 282
#define TYPE_NAME 283
#define TYPEDEF 284
#define EXTERN 285
#define STATIC 286
#define AUTO 287
#define REGISTER 288
#define CHAR 289
#define SHORT 290
#define INT 291
#define LONG 292
#define SIGNED 293
#define UNSIGNED 294
#define FLOAT 295
#define DOUBLE 296
#define CONST 297
#define VOLATILE 298
#define VOID 299
#define STRUCT 300
#define UNION 301
#define ENUM 302
#define ELLIPSIS 303
#define CASE 304
#define DEFAULT 305
#define IF 306
#define ELSE 307
#define SWITCH 308
#define WHILE 309
#define DO 310
#define FOR 311
#define GOTO 312
#define CONTINUE 313
#define BREAK 314
#define RETURN 315




/* Copy the first part of user declarations.  */
#line 16 "parser.y"



#include <stdlib.h>
#include <stdio.h>
#include "clang.h"
#if __CXX__
using namespace std;
#include <iostream>
#else
#include "parser.tab.h"
#endif

static struct xlat_unit_parse_node * get_xlat_node(void)
{
	return (struct xlat_unit_parse_node *) calloc(1, sizeof * get_xlat_node());
}

#define MAX_LAST_LIST_NODE_STACK_DEPTH		64
/*! \todo	document these */
/*! \todo	stack is empty ascending - maybe change this */
static struct xlat_unit_parse_node * last_list_node_stack[MAX_LAST_LIST_NODE_STACK_DEPTH];
static int last_list_node_stack_ptr = 0;

/* returns 0 on error */
static struct xlat_unit_parse_node * merge_decl_specs(struct xlat_unit_parse_node * n1, struct xlat_unit_parse_node * n2);

/*static */struct xlat_unit_parse_node * parse_head;
static void yyerror(const YYLTYPE * loc, void * x, const char * msg);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 48 "parser.y"
{
	struct xlat_unit_parse_node	* parse_node;
	const char * ident;
	const char * string_literal;
	/*const void * const_data;*/
	long int const_val;
	enum EXPR_NODE_ENUM	expr_detail;
}
/* Line 187 of yacc.c.  */
#line 264 "parser.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */
#line 57 "parser.y"

#include "clang-parser-common.h"


/* Line 216 of yacc.c.  */
#line 292 "parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  93
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1429

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  64
/* YYNRULES -- Number of rules.  */
#define YYNRULES  213
/* YYNRULES -- Number of states.  */
#define YYNSTATES  353

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   315

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    72,     2,     2,     2,    74,    67,     2,
      61,    62,    68,    69,    66,    70,    65,    73,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    80,    82,
      75,    81,    76,    79,    85,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,    64,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    83,    78,    84,    71,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    13,    15,    20,    24,
      29,    33,    37,    40,    43,    45,    49,    51,    54,    57,
      60,    63,    68,    70,    72,    74,    76,    78,    80,    82,
      87,    89,    93,    97,   101,   103,   107,   111,   113,   117,
     121,   123,   127,   131,   135,   139,   141,   145,   149,   151,
     155,   157,   161,   163,   167,   169,   173,   175,   179,   181,
     187,   189,   193,   195,   197,   199,   201,   203,   205,   207,
     209,   211,   213,   215,   217,   221,   223,   226,   230,   232,
     235,   237,   240,   242,   245,   247,   251,   253,   257,   259,
     261,   263,   265,   267,   269,   271,   273,   275,   277,   279,
     281,   283,   285,   287,   289,   291,   293,   295,   301,   306,
     309,   311,   313,   315,   318,   322,   325,   327,   330,   332,
     334,   338,   340,   343,   347,   352,   358,   361,   363,   367,
     369,   373,   376,   378,   380,   384,   389,   393,   398,   403,
     407,   409,   412,   415,   419,   421,   424,   426,   430,   432,
     436,   439,   442,   444,   446,   450,   452,   455,   457,   459,
     462,   466,   469,   473,   477,   482,   485,   489,   493,   498,
     500,   504,   509,   511,   515,   517,   519,   521,   523,   525,
     527,   531,   536,   540,   543,   547,   551,   556,   558,   561,
     563,   566,   568,   571,   577,   585,   591,   597,   605,   612,
     620,   624,   627,   630,   633,   637,   639,   642,   644,   646,
     650,   655,   659,   663
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     147,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    61,
     106,    62,    -1,    87,    -1,    88,    63,   106,    64,    -1,
      88,    61,    62,    -1,    88,    61,    89,    62,    -1,    88,
      65,     3,    -1,    88,     7,     3,    -1,    88,     8,    -1,
      88,     9,    -1,   104,    -1,    89,    66,   104,    -1,    88,
      -1,     8,    90,    -1,     9,    90,    -1,    91,    92,    -1,
       6,    90,    -1,     6,    61,   133,    62,    -1,    67,    -1,
      68,    -1,    69,    -1,    70,    -1,    71,    -1,    72,    -1,
      90,    -1,    61,   133,    62,    92,    -1,    92,    -1,    93,
      68,    92,    -1,    93,    73,    92,    -1,    93,    74,    92,
      -1,    93,    -1,    94,    69,    93,    -1,    94,    70,    93,
      -1,    94,    -1,    95,    10,    94,    -1,    95,    11,    94,
      -1,    95,    -1,    96,    75,    95,    -1,    96,    76,    95,
      -1,    96,    12,    95,    -1,    96,    13,    95,    -1,    96,
      -1,    97,    14,    96,    -1,    97,    15,    96,    -1,    97,
      -1,    98,    67,    97,    -1,    98,    -1,    99,    77,    98,
      -1,    99,    -1,   100,    78,    99,    -1,   100,    -1,   101,
      16,   100,    -1,   101,    -1,   102,    17,   101,    -1,   102,
      -1,   102,    79,   106,    80,   103,    -1,   103,    -1,    90,
     105,   104,    -1,    81,    -1,    18,    -1,    19,    -1,    20,
      -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,
      -1,    26,    -1,    27,    -1,   104,    -1,   106,    66,   104,
      -1,   103,    -1,   109,    82,    -1,   109,   110,    82,    -1,
     112,    -1,   109,   112,    -1,   113,    -1,   109,   113,    -1,
     114,    -1,   109,   114,    -1,   111,    -1,   110,    66,   111,
      -1,   125,    -1,   125,    81,   136,    -1,    29,    -1,    30,
      -1,    31,    -1,    32,    -1,    33,    -1,    44,    -1,    34,
      -1,    35,    -1,    36,    -1,    37,    -1,    40,    -1,    41,
      -1,    38,    -1,    39,    -1,   115,    -1,   122,    -1,    28,
      -1,    42,    -1,    43,    -1,   116,     3,    83,   117,    84,
      -1,   116,    83,   117,    84,    -1,   116,     3,    -1,    45,
      -1,    46,    -1,   118,    -1,   117,   118,    -1,   119,   120,
      82,    -1,   119,   113,    -1,   113,    -1,   119,   114,    -1,
     114,    -1,   121,    -1,   120,    66,   121,    -1,   125,    -1,
      80,   107,    -1,   125,    80,   107,    -1,    47,    83,   123,
      84,    -1,    47,     3,    83,   123,    84,    -1,    47,     3,
      -1,   124,    -1,   123,    66,   124,    -1,     3,    -1,     3,
      81,   107,    -1,   127,   126,    -1,   126,    -1,     3,    -1,
      61,   125,    62,    -1,   126,    63,   107,    64,    -1,   126,
      63,    64,    -1,   126,    61,   129,    62,    -1,   126,    61,
     132,    62,    -1,   126,    61,    62,    -1,    68,    -1,    68,
     128,    -1,    68,   127,    -1,    68,   128,   127,    -1,   114,
      -1,   128,   114,    -1,   130,    -1,   130,    66,    48,    -1,
     131,    -1,   130,    66,   131,    -1,   109,   125,    -1,   109,
     134,    -1,   109,    -1,     3,    -1,   132,    66,     3,    -1,
     119,    -1,   119,   134,    -1,   127,    -1,   135,    -1,   127,
     135,    -1,    61,   134,    62,    -1,    63,    64,    -1,    63,
     107,    64,    -1,   135,    63,    64,    -1,   135,    63,   107,
      64,    -1,    61,    62,    -1,    61,   129,    62,    -1,   135,
      61,    62,    -1,   135,    61,   129,    62,    -1,   104,    -1,
      83,   137,    84,    -1,    83,   137,    66,    84,    -1,   136,
      -1,   137,    66,   136,    -1,   139,    -1,   140,    -1,   143,
      -1,   144,    -1,   145,    -1,   146,    -1,     3,    80,   138,
      -1,    49,   107,    80,   138,    -1,    50,    80,   138,    -1,
      83,    84,    -1,    83,   142,    84,    -1,    83,   141,    84,
      -1,    83,   141,   142,    84,    -1,   108,    -1,   141,   108,
      -1,   138,    -1,   142,   138,    -1,    82,    -1,   106,    82,
      -1,    51,    61,   106,    62,   138,    -1,    51,    61,   106,
      62,   138,    52,   138,    -1,    53,    61,   106,    62,   138,
      -1,    54,    61,   106,    62,   138,    -1,    55,   138,    54,
      61,   106,    62,    82,    -1,    56,    61,   143,   143,    62,
     138,    -1,    56,    61,   143,   143,   106,    62,   138,    -1,
      57,     3,    82,    -1,    58,    82,    -1,    59,    82,    -1,
      60,    82,    -1,    60,   106,    82,    -1,   148,    -1,   147,
     148,    -1,   149,    -1,   108,    -1,    85,   106,    85,    -1,
     109,   125,   141,   140,    -1,   109,   125,   140,    -1,   125,
     141,   140,    -1,   125,   140,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   104,   104,   110,   116,   122,   131,   132,   139,   145,
     154,   161,   168,   174,   183,   189,   199,   200,   206,   212,
     218,   224,   233,   234,   235,   236,   237,   238,   242,   243,
     253,   254,   261,   268,   278,   279,   286,   296,   297,   304,
     314,   315,   322,   329,   336,   346,   347,   354,   364,   365,
     375,   376,   386,   387,   397,   398,   408,   409,   419,   420,
     424,   425,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   449,   450,   460,   464,   471,   480,   481,
     482,   483,   484,   485,   489,   490,   494,   495,   499,   500,
     501,   502,   503,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   522,   523,   527,   528,   529,
     533,   534,   538,   539,   543,   547,   548,   549,   550,   554,
     555,   559,   560,   561,   565,   566,   567,   571,   572,   576,
     577,   581,   582,   586,   592,   593,   594,   595,   596,   597,
     601,   602,   603,   604,   608,   609,   614,   615,   619,   620,
     624,   625,   626,   630,   631,   635,   636,   640,   641,   642,
     646,   647,   648,   649,   650,   651,   652,   653,   654,   658,
     659,   660,   664,   665,   669,   670,   671,   672,   673,   674,
     678,   679,   680,   684,   688,   695,   702,   715,   721,   731,
     737,   747,   748,   752,   759,   767,   777,   784,   791,   799,
     811,   817,   822,   827,   832,   841,   847,   857,   858,   859,
     863,   864,   865,   866
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "CONSTANT",
  "STRING_LITERAL", "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP",
  "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP",
  "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN",
  "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN",
  "TYPE_NAME", "TYPEDEF", "EXTERN", "STATIC", "AUTO", "REGISTER", "CHAR",
  "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED", "FLOAT", "DOUBLE", "CONST",
  "VOLATILE", "VOID", "STRUCT", "UNION", "ENUM", "ELLIPSIS", "CASE",
  "DEFAULT", "IF", "ELSE", "SWITCH", "WHILE", "DO", "FOR", "GOTO",
  "CONTINUE", "BREAK", "RETURN", "'('", "')'", "'['", "']'", "'.'", "','",
  "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'", "'%'", "'<'", "'>'",
  "'^'", "'|'", "'?'", "':'", "'='", "';'", "'{'", "'}'", "'@'", "$accept",
  "primary_expression", "postfix_expression", "argument_expression_list",
  "unary_expression", "unary_operator", "cast_expression",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_or_expression",
  "conditional_expression", "assignment_expression", "assignment_operator",
  "expression", "constant_expression", "declaration",
  "declaration_specifiers", "init_declarator_list", "init_declarator",
  "storage_class_specifier", "type_specifier", "type_qualifier",
  "struct_or_union_specifier", "struct_or_union",
  "struct_declaration_list", "struct_declaration",
  "specifier_qualifier_list", "struct_declarator_list",
  "struct_declarator", "enum_specifier", "enumerator_list", "enumerator",
  "declarator", "direct_declarator", "pointer", "type_qualifier_list",
  "parameter_type_list", "parameter_list", "parameter_declaration",
  "identifier_list", "type_name", "abstract_declarator",
  "direct_abstract_declarator", "initializer", "initializer_list",
  "statement", "labeled_statement", "compound_statement",
  "declaration_list", "statement_list", "expression_statement",
  "selection_statement", "iteration_statement", "jump_statement",
  "translation_unit", "external_declaration", "function_definition", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,    40,    41,    91,    93,    46,    44,    38,    42,    43,
      45,   126,    33,    47,    37,    60,    62,    94,   124,    63,
      58,    61,    59,   123,   125,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    86,    87,    87,    87,    87,    88,    88,    88,    88,
      88,    88,    88,    88,    89,    89,    90,    90,    90,    90,
      90,    90,    91,    91,    91,    91,    91,    91,    92,    92,
      93,    93,    93,    93,    94,    94,    94,    95,    95,    95,
      96,    96,    96,    96,    96,    97,    97,    97,    98,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   103,   103,
     104,   104,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   106,   106,   107,   108,   108,   109,   109,
     109,   109,   109,   109,   110,   110,   111,   111,   112,   112,
     112,   112,   112,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   114,   114,   115,   115,   115,
     116,   116,   117,   117,   118,   119,   119,   119,   119,   120,
     120,   121,   121,   121,   122,   122,   122,   123,   123,   124,
     124,   125,   125,   126,   126,   126,   126,   126,   126,   126,
     127,   127,   127,   127,   128,   128,   129,   129,   130,   130,
     131,   131,   131,   132,   132,   133,   133,   134,   134,   134,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   136,
     136,   136,   137,   137,   138,   138,   138,   138,   138,   138,
     139,   139,   139,   140,   140,   140,   140,   141,   141,   142,
     142,   143,   143,   144,   144,   144,   145,   145,   145,   145,
     146,   146,   146,   146,   146,   147,   147,   148,   148,   148,
     149,   149,   149,   149
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     1,     4,     3,     4,
       3,     3,     2,     2,     1,     3,     1,     2,     2,     2,
       2,     4,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     5,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     2,     3,     1,     2,
       1,     2,     1,     2,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     5,     4,     2,
       1,     1,     1,     2,     3,     2,     1,     2,     1,     1,
       3,     1,     2,     3,     4,     5,     2,     1,     3,     1,
       3,     2,     1,     1,     3,     4,     3,     4,     4,     3,
       1,     2,     2,     3,     1,     2,     1,     3,     1,     3,
       2,     2,     1,     1,     3,     1,     2,     1,     1,     2,
       3,     2,     3,     3,     4,     2,     3,     3,     4,     1,
       3,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       3,     4,     3,     2,     3,     3,     4,     1,     2,     1,
       2,     1,     2,     5,     7,     5,     5,     7,     6,     7,
       3,     2,     2,     2,     3,     1,     2,     1,     1,     3,
       4,     3,     3,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   133,   104,    88,    89,    90,    91,    92,    94,    95,
      96,    97,   100,   101,    98,    99,   105,   106,    93,   110,
     111,     0,     0,   140,     0,   208,     0,    78,    80,    82,
     102,     0,   103,     0,   132,     0,     0,   205,   207,   126,
       0,     0,   144,   142,   141,     2,     3,     4,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     6,    16,
      28,     0,    30,    34,    37,    40,    45,    48,    50,    52,
      54,    56,    58,    60,    73,     0,    76,     0,    84,    79,
      81,    83,    86,   109,     0,     0,   187,     0,   213,     0,
       0,     0,   131,     1,   206,     0,   129,     0,   127,   134,
     145,   143,     0,    20,     0,    17,    18,     0,   116,   118,
     155,     0,     0,    12,    13,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    62,     0,
      28,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,     0,    77,     0,   211,     0,     0,     0,
     112,     0,     2,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   191,   183,     0,   189,   174,   175,
       0,     0,   176,   177,   178,   179,    86,   188,   212,   153,
     139,   152,     0,   146,   148,     0,   136,    75,     0,     0,
       0,     0,   124,     0,     5,     0,     0,   115,   117,   157,
     156,   158,     0,    11,     8,     0,    14,     0,    10,    61,
      31,    32,    33,    35,    36,    38,    39,    43,    44,    41,
      42,    46,    47,    49,    51,    53,    55,    57,     0,    74,
      85,     0,   169,    87,   210,     0,   108,   113,     0,     0,
     119,   121,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   201,   202,   203,     0,   192,   185,     0,   184,   190,
       0,   150,   157,   151,   137,     0,   138,     0,   135,   125,
     130,   128,    21,   165,     0,     0,   161,     0,   159,     0,
       0,    29,     9,     0,     7,     0,   172,     0,   107,   122,
       0,   114,     0,   180,     0,   182,     0,     0,     0,     0,
       0,   200,   204,   186,   147,   149,   154,   166,   160,   162,
     167,     0,   163,     0,    15,    59,     0,   170,   120,   123,
     181,     0,     0,     0,     0,     0,   168,   164,   171,   173,
     193,   195,   196,     0,     0,     0,     0,     0,   198,     0,
     194,   197,   199
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    58,    59,   215,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,   129,
     176,   198,    86,    87,    77,    78,    27,    28,    29,    30,
      31,   159,   160,   161,   249,   250,    32,    97,    98,    33,
      34,    35,    44,   284,   193,   194,   195,   111,   285,   211,
     243,   297,   177,   178,   179,    89,   181,   182,   183,   184,
     185,    36,    37,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -216
static const yytype_int16 yypact[] =
{
     355,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,    35,    11,    -8,   953,  -216,   718,  -216,  -216,  -216,
    -216,    42,  -216,  1213,    73,    25,   296,  -216,  -216,   -67,
      38,   -14,  -216,  -216,    -8,  -216,  -216,  -216,   974,  1023,
    1023,   859,  -216,  -216,  -216,  -216,  -216,  -216,  -216,   157,
     387,   953,  -216,    80,    81,   133,     7,   210,    -9,    85,
     -15,   141,    40,  -216,  -216,   -43,  -216,   -23,  -216,  -216,
    -216,  -216,  1193,   102,  1167,   416,  -216,   718,  -216,  1213,
     546,   365,    73,  -216,  -216,    38,   111,   -45,  -216,  -216,
    -216,  -216,   859,  -216,   953,  -216,  -216,   -11,  -216,  -216,
    1305,   140,   196,  -216,  -216,   814,   953,   202,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,   953,
    -216,  -216,   953,   953,   953,   953,   953,   953,   953,   953,
     953,   953,   953,   953,   953,   953,   953,   953,   953,   953,
     953,   953,  -216,    11,  -216,   662,  -216,  1213,  1167,  1116,
    -216,   812,   128,   953,   130,   151,   153,   156,   628,   158,
     242,   164,   165,   734,  -216,  -216,   -12,  -216,  -216,  -216,
     486,   556,  -216,  -216,  -216,  -216,   167,  -216,  -216,  -216,
    -216,  1080,   187,   186,  -216,    -1,  -216,  -216,   189,   -37,
     953,    38,  -216,   193,  -216,  1269,   904,  -216,  -216,   123,
    -216,   135,   953,  -216,  -216,    94,  -216,   137,  -216,  -216,
    -216,  -216,  -216,    80,    80,    81,    81,   133,   133,   133,
     133,     7,     7,   210,    -9,    85,   -15,   141,   -30,  -216,
    -216,   662,  -216,  -216,  -216,  1136,  -216,  -216,   953,    63,
    -216,   177,   628,   178,   628,   953,   953,   953,   205,   763,
     179,  -216,  -216,  -216,    65,  -216,  -216,   592,  -216,  -216,
    1035,  -216,    74,  -216,  -216,  1381,  -216,   257,  -216,  -216,
    -216,  -216,  -216,  -216,   200,   203,  -216,   204,   135,  1346,
     929,  -216,  -216,   953,  -216,   953,  -216,   -35,  -216,  -216,
       5,  -216,   953,  -216,   628,  -216,    97,   108,   114,   206,
     763,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,   207,  -216,   209,  -216,  -216,   283,  -216,  -216,  -216,
    -216,   628,   628,   628,   953,   941,  -216,  -216,  -216,  -216,
     212,  -216,  -216,   115,   628,   121,   628,   184,  -216,   628,
    -216,  -216,  -216
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -216,  -216,  -216,  -216,   -39,  -216,   -44,   103,    92,    95,
     100,   125,   131,   127,   134,   136,  -216,   -79,  -111,  -216,
     -24,   -48,    33,     1,  -216,   126,   -19,    36,    30,  -216,
    -216,   139,  -129,   -27,  -216,   -17,  -216,   198,    83,   -20,
     -32,     2,  -216,   -85,  -216,    19,  -216,   199,   -78,  -196,
    -215,  -216,   -26,  -216,   -18,    93,   118,  -131,  -216,  -216,
    -216,  -216,   259,  -216
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      75,    26,    41,    92,   216,   192,    82,    79,     1,   103,
     105,   106,   197,   288,     1,    88,    95,   131,   219,   139,
     140,   201,   130,   151,   110,    43,   296,   107,     1,   201,
     247,   326,   210,    25,    16,    17,   151,    26,    39,   202,
     239,    96,   152,   153,   242,    83,   101,   279,    99,   327,
     295,   204,   130,    42,   151,   151,    81,   149,   145,   154,
      23,   276,    80,   147,   156,   277,    22,   186,    79,    25,
     265,   188,    22,    23,   100,   110,   288,     1,   107,    23,
     107,   109,   141,   142,   197,   248,    22,   108,   220,   221,
     222,   191,   217,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   339,   209,   273,   109,   253,   247,    81,    40,   150,
     108,   197,   187,    80,   130,    84,   238,   197,   310,   300,
     242,   151,   109,   186,    90,   270,    91,   206,   108,   244,
     208,   251,   258,   137,   138,   301,   207,   312,   132,   264,
     135,   136,   280,   133,   134,   269,   292,   148,   287,   331,
     293,   130,   146,   151,   112,   113,   114,   130,   291,   197,
     332,   271,    79,   130,   151,   157,   333,   347,   180,   335,
     151,   151,   324,   349,   205,   158,   206,   151,   109,   109,
     187,   208,   200,   272,   108,   108,   289,   207,   290,   213,
     299,   294,   212,   151,   321,   218,   191,   209,   252,   130,
     254,   197,   255,   187,   256,   242,   325,   257,   115,   259,
     116,    81,   117,   197,   143,   144,   303,    80,   305,   225,
     226,   306,   307,   308,   227,   228,   229,   230,   223,   224,
      92,   269,   323,   231,   232,   260,   261,   262,   155,   274,
      41,   130,   275,   278,   329,   282,   130,   302,   304,   309,
     316,   311,   317,   130,   346,   318,   351,   334,   319,   336,
     233,   191,   272,   337,   235,   109,   191,   234,   330,   240,
     251,   108,   236,   328,   281,   237,    45,    46,    47,    48,
     191,    49,    50,   199,   315,    94,    93,   245,   267,     1,
       0,   203,     0,     0,     0,   340,   341,   342,     0,     0,
     343,   345,     0,     0,     0,     0,     0,     0,   348,     0,
     350,     0,     0,   352,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    51,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,     0,    22,     1,     0,
       0,     0,     0,     0,    23,     0,   241,   338,    45,    46,
      47,    48,     0,    49,    50,     0,     0,     0,     0,     0,
       0,    24,     0,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,     0,     0,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,     0,    22,     0,     0,   162,
      46,    47,    48,    23,    49,    50,    51,     0,     0,   196,
       0,     0,    52,    53,    54,    55,    56,    57,     0,     0,
      24,     0,     0,     0,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     0,   163,   164,   165,   128,   166,
     167,   168,   169,   170,   171,   172,   173,    51,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,   162,
      46,    47,    48,     0,    49,    50,     0,     0,   174,    85,
     175,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     0,   163,   164,   165,     0,   166,
     167,   168,   169,   170,   171,   172,   173,    51,     0,   189,
       0,     0,     0,    52,    53,    54,    55,    56,    57,   162,
      46,    47,    48,     0,    49,    50,     0,     0,   174,    85,
     266,     0,     0,     0,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     0,   162,    46,    47,    48,     0,
      49,    50,     0,     0,     0,   163,   164,   165,   190,   166,
     167,   168,   169,   170,   171,   172,   173,    51,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,     0,
       0,   162,    46,    47,    48,     0,    49,    50,   174,    85,
     268,   163,   164,   165,     0,   166,   167,   168,   169,   170,
     171,   172,   173,    51,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    45,    46,    47,    48,     0,
      49,    50,     0,     0,   174,    85,   313,   163,   164,   165,
       0,   166,   167,   168,   169,   170,   171,   172,   173,    51,
       0,     0,     0,     0,     0,    52,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     0,    51,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,     0,     0,    45,    46,    47,
      48,     0,    49,    50,     0,   241,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    45,    46,    47,    48,
       0,    49,    50,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,     0,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
      76,    52,    53,    54,    55,    56,    57,     0,     0,     0,
       0,     0,     0,     0,     0,     1,   263,    45,    46,    47,
      48,     0,    49,    50,    51,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,     0,     0,     0,     0,
       2,     0,     0,     0,     0,   174,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
       0,     0,    45,    46,    47,    48,     0,    49,    50,     0,
       0,     0,     0,    22,     0,    51,   214,     0,     0,     0,
      23,    52,    53,    54,    55,    56,    57,     2,     0,     0,
       0,     0,   248,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    45,    46,    47,
      48,     0,    49,    50,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    45,    46,    47,    48,     0,    49,    50,     0,
       0,     0,     0,     0,    45,    46,    47,    48,     0,    49,
      50,     0,     0,     0,     0,     0,    45,    46,    47,    48,
       0,    49,    50,     0,     0,    51,     0,     0,   286,     0,
       0,    52,    53,    54,    55,    56,    57,    45,    46,    47,
      48,     0,    49,    50,     0,     0,     0,     0,     0,     0,
      51,     0,     0,   322,     0,     0,    52,    53,    54,    55,
      56,    57,    51,   344,     0,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    51,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    45,    46,    47,    48,
       0,    49,    50,     0,     0,   102,     0,     0,     1,     0,
       0,    52,    53,    54,    55,    56,    57,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,     1,   104,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,   270,   283,   206,     0,
       0,     0,     0,    23,     0,     0,     0,     0,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,     0,   206,     2,     0,     0,     0,    23,     0,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     2,     0,     0,     0,     0,     0,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     0,     0,     0,     0,
     246,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,     0,     0,     0,     0,     0,
     298,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   155,     0,    85,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     205,   283,   206,     2,     0,     0,     0,    23,     0,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   205,     0,   206,     0,
       0,     0,     0,    23,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   320,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,   314
};

static const yytype_int16 yycheck[] =
{
      24,     0,    22,    35,   115,    90,    26,    26,     3,    48,
      49,    50,    91,   209,     3,    33,    83,    61,   129,    12,
      13,    66,    61,    66,    51,    23,   241,    51,     3,    66,
     159,    66,   110,     0,    42,    43,    66,    36,     3,    84,
     151,     3,    85,    66,   155,     3,    44,    84,    62,    84,
      80,    62,    91,    23,    66,    66,    26,    17,    67,    82,
      68,    62,    26,    78,    82,    66,    61,    87,    87,    36,
      82,    89,    61,    68,    44,   102,   272,     3,   102,    68,
     104,    51,    75,    76,   163,    80,    61,    51,   132,   133,
     134,    90,   116,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   326,   110,   191,    84,   163,   245,    87,    83,    79,
      84,   200,    89,    87,   163,    83,   150,   206,   259,    66,
     241,    66,   102,   153,    61,    61,    63,    63,   102,   157,
     110,   161,   168,    10,    11,    82,   110,    82,    68,   173,
      69,    70,   200,    73,    74,   181,    62,    16,   206,    62,
      66,   200,    77,    66,     7,     8,     9,   206,   212,   248,
      62,   191,   191,   212,    66,    82,    62,    62,    85,   310,
      66,    66,   293,    62,    61,    83,    63,    66,   158,   159,
     157,   161,    81,   191,   158,   159,    61,   161,    63,     3,
     248,    64,    62,    66,   289,     3,   205,   205,    80,   248,
      80,   290,    61,   180,    61,   326,   295,    61,    61,    61,
      63,   191,    65,   302,    14,    15,   252,   191,   254,   137,
     138,   255,   256,   257,   139,   140,   141,   142,   135,   136,
     272,   267,   290,   143,   144,     3,    82,    82,    81,    62,
     270,   290,    66,    64,   302,    62,   295,    80,    80,    54,
       3,    82,    62,   302,    52,    62,    82,    61,    64,    62,
     145,   270,   270,    64,   147,   245,   275,   146,   304,   153,
     300,   245,   148,   300,   201,   149,     3,     4,     5,     6,
     289,     8,     9,    95,   275,    36,     0,   158,   180,     3,
      -1,   102,    -1,    -1,    -1,   331,   332,   333,    -1,    -1,
     334,   335,    -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,
     346,    -1,    -1,   349,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    61,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    61,     3,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    83,    84,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    61,    -1,    -1,     3,
       4,     5,     6,    68,     8,     9,    61,    -1,    -1,    64,
      -1,    -1,    67,    68,    69,    70,    71,    72,    -1,    -1,
      85,    -1,    -1,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    51,    81,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    -1,     3,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    82,    83,
      84,    -1,    -1,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    49,    50,    51,    62,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    82,    83,
      84,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    82,    83,    84,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      82,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,    -1,    61,    -1,    -1,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    -1,    83,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    61,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,
      82,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,    82,     3,     4,     5,
       6,    -1,     8,     9,    61,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    82,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    -1,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    61,    -1,    61,    62,    -1,    -1,    -1,
      68,    67,    68,    69,    70,    71,    72,    28,    -1,    -1,
      -1,    -1,    80,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    -1,    -1,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    61,    -1,    -1,    64,    -1,
      -1,    67,    68,    69,    70,    71,    72,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    -1,    -1,    64,    -1,    -1,    67,    68,    69,    70,
      71,    72,    61,    62,    -1,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    61,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    61,    -1,    -1,     3,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     3,    61,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    61,    62,    63,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    -1,    63,    28,    -1,    -1,    -1,    68,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    28,    -1,    -1,    -1,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      84,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,
      84,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    83,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      61,    62,    63,    28,    -1,    -1,    -1,    68,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,    63,    -1,
      -1,    -1,    -1,    68,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    61,    68,    85,   108,   109,   112,   113,   114,
     115,   116,   122,   125,   126,   127,   147,   148,   149,     3,
      83,   125,   114,   127,   128,     3,     4,     5,     6,     8,
       9,    61,    67,    68,    69,    70,    71,    72,    87,    88,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   106,    82,   110,   111,   112,
     113,   114,   125,     3,    83,    83,   108,   109,   140,   141,
      61,    63,   126,     0,   148,    83,     3,   123,   124,    62,
     114,   127,    61,    90,    61,    90,    90,   106,   113,   114,
     119,   133,     7,     8,     9,    61,    63,    65,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    81,   105,
      90,    92,    68,    73,    74,    69,    70,    10,    11,    12,
      13,    75,    76,    14,    15,    67,    77,    78,    16,    17,
      79,    66,    85,    66,    82,    81,   140,   141,    83,   117,
     118,   119,     3,    49,    50,    51,    53,    54,    55,    56,
      57,    58,    59,    60,    82,    84,   106,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   125,   108,   140,     3,
      62,   109,   129,   130,   131,   132,    64,   103,   107,   123,
      81,    66,    84,   133,    62,    61,    63,   113,   114,   127,
     134,   135,    62,     3,    62,    89,   104,   106,     3,   104,
      92,    92,    92,    93,    93,    94,    94,    95,    95,    95,
      95,    96,    96,    97,    98,    99,   100,   101,   106,   104,
     111,    83,   104,   136,   140,   117,    84,   118,    80,   120,
     121,   125,    80,   107,    80,    61,    61,    61,   138,    61,
       3,    82,    82,    82,   106,    82,    84,   142,    84,   138,
      61,   125,   127,   134,    62,    66,    62,    66,    64,    84,
     107,   124,    62,    62,   129,   134,    64,   107,   135,    61,
      63,    92,    62,    66,    64,    80,   136,   137,    84,   107,
      66,    82,    80,   138,    80,   138,   106,   106,   106,    54,
     143,    82,    82,    84,    48,   131,     3,    62,    62,    64,
      62,   129,    64,   107,   104,   103,    66,    84,   121,   107,
     138,    62,    62,    62,    61,   143,    62,    64,    84,   136,
     138,   138,   138,   106,    62,   106,    52,    62,   138,    62,
     138,    82,   138
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, x, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc, x)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, x); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void * x)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, x)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    void * x;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (x);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void * x)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, x)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    void * x;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, x);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void * x)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, x)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    void * x;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , x);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, x); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void * x)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, x)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    void * x;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (x);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void * x);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void * x)
#else
int
yyparse (x)
    void * x;
#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif


  /* User initialization code.  */
#line 12 "parser.y"
{
	yydebug = 0;
}
/* Line 1069 of yacc.c.  */
#line 1921 "parser.tab.c"
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 105 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_IDENT;
			(yyval.parse_node)->expr_node.id = (yyvsp[(1) - (1)].ident);
		;}
    break;

  case 3:
#line 111 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_CONSTANT;
			(yyval.parse_node)->expr_node.const_val = (yyvsp[(1) - (1)].const_val);
		;}
    break;

  case 4:
#line 117 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_STR_LITERAL;
			(yyval.parse_node)->expr_node.string_literal = (yyvsp[(1) - (1)].string_literal);
		;}
    break;

  case 5:
#line 123 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_PARENTHESIZED_EXPR;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(2) - (3)].parse_node);
		;}
    break;

  case 6:
#line 131 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 7:
#line 133 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_ARR_SUBSCRIPT;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (4)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (4)].parse_node);
		;}
    break;

  case 8:
#line 140 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_FUNCTION_CALL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
		;}
    break;

  case 9:
#line 146 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_FUNCTION_CALL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (4)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (4)].parse_node);
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		;}
    break;

  case 10:
#line 155 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_DOT_MEMBER_SELECT;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.id = (yyvsp[(3) - (3)].ident);
		;}
    break;

  case 11:
#line 162 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_PTR_MEMBER_SELECT;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.id = (yyvsp[(3) - (3)].ident);
		;}
    break;

  case 12:
#line 169 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_POST_INC;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (2)].parse_node);
		;}
    break;

  case 13:
#line 175 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_POST_DEC;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (2)].parse_node);
		;}
    break;

  case 14:
#line 184 "parser.y"
    {
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			(yyval.parse_node) = last_list_node_stack[last_list_node_stack_ptr++] = (yyvsp[(1) - (1)].parse_node);
		;}
    break;

  case 15:
#line 190 "parser.y"
    {
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= (yyvsp[(3) - (3)].parse_node);
			(yyval.parse_node) = (yyvsp[(1) - (3)].parse_node);
		;}
    break;

  case 16:
#line 199 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 17:
#line 201 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_PRE_INC;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(2) - (2)].parse_node);
		;}
    break;

  case 18:
#line 207 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_PRE_DEC;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(2) - (2)].parse_node);
		;}
    break;

  case 19:
#line 213 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = (yyvsp[(1) - (2)].expr_detail);
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(2) - (2)].parse_node);
		;}
    break;

  case 20:
#line 219 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_SIZEOF;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(2) - (2)].parse_node);
		;}
    break;

  case 21:
#line 225 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_SIZEOF;
			(yyval.parse_node)->expr_node.type_name = (yyvsp[(3) - (4)].parse_node);
		;}
    break;

  case 22:
#line 233 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_UNARY_AMPERSAND; ;}
    break;

  case 23:
#line 234 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_UNARY_INDIRECT; ;}
    break;

  case 24:
#line 235 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_UNARY_PLUS; ;}
    break;

  case 25:
#line 236 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_UNARY_MINUS; ;}
    break;

  case 26:
#line 237 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_UNARY_BITWISE_NOT; ;}
    break;

  case 27:
#line 238 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_UNARY_LOGICAL_NOT; ;}
    break;

  case 28:
#line 242 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 29:
#line 244 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_MUL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(4) - (4)].parse_node);
			(yyval.parse_node)->expr_node.type_name = (yyvsp[(2) - (4)].parse_node);
		;}
    break;

  case 30:
#line 253 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 31:
#line 255 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_MUL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 32:
#line 262 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_DIV;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 33:
#line 269 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_MOD;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 34:
#line 278 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 35:
#line 280 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_ADD;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 36:
#line 287 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_SUB;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 37:
#line 296 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node) ;}
    break;

  case 38:
#line 298 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_LEFT_SHIFT;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 39:
#line 305 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_RIGHT_SHIFT;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 40:
#line 314 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 41:
#line 316 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_LESS;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 42:
#line 323 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_GREATER;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 43:
#line 330 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_LESS_OR_EQUAL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 44:
#line 337 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_GREATER_OR_EQUAL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 45:
#line 346 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 46:
#line 348 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_EQUAL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 47:
#line 355 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_NOT_EQUAL;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 48:
#line 364 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 49:
#line 366 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_BITWISE_AND;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 50:
#line 375 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 51:
#line 377 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_XOR;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 52:
#line 386 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 53:
#line 388 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_BITWISE_OR;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 54:
#line 397 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 55:
#line 399 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_LOGICAL_AND;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 56:
#line 408 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 57:
#line 410 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_LOGICAL_OR;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 58:
#line 419 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 60:
#line 424 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 61:
#line 426 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = (yyvsp[(2) - (3)].expr_detail);
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 62:
#line 435 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_ASSIGN; ;}
    break;

  case 63:
#line 436 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_MUL_ASSIGN; ;}
    break;

  case 64:
#line 437 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_DIV_ASSIGN; ;}
    break;

  case 65:
#line 438 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_MOD_ASSIGN; ;}
    break;

  case 66:
#line 439 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_ADD_ASSIGN; ;}
    break;

  case 67:
#line 440 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_SUB_ASSIGN; ;}
    break;

  case 68:
#line 441 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_LEFT_ASSIGN; ;}
    break;

  case 69:
#line 442 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_RIGHT_ASSIGN; ;}
    break;

  case 70:
#line 443 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_AND_ASSIGN; ;}
    break;

  case 71:
#line 444 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_XOR_ASSIGN; ;}
    break;

  case 72:
#line 445 "parser.y"
    { (yyval.expr_detail) = EXPR_NODE_OR_ASSIGN; ;}
    break;

  case 73:
#line 449 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 74:
#line 451 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_EXPR;
			(yyval.parse_node)->expr_node.type = EXPR_NODE_COMMA;
			(yyval.parse_node)->expr_node.expr[0] = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->expr_node.expr[1] = (yyvsp[(3) - (3)].parse_node);
		;}
    break;

  case 76:
#line 465 "parser.y"
    {
			/* this is really useful only for defining
			 * a new structure/union/enumeration */
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECLARATION;
			(yyval.parse_node)->declaration.decl_spec = (yyvsp[(1) - (2)].parse_node);
		;}
    break;

  case 77:
#line 472 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECLARATION;
			(yyval.parse_node)->declaration.decl_spec = (yyvsp[(1) - (3)].parse_node);
			(yyval.parse_node)->declaration.declarator_list = (yyvsp[(2) - (3)].parse_node);
		;}
    break;

  case 78:
#line 480 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 79:
#line 481 "parser.y"
    { if (!((yyval.parse_node) = merge_decl_specs((yyvsp[(1) - (2)].parse_node), (yyvsp[(2) - (2)].parse_node)))) panic("abort the parse here (YYERROR)\n"); ;}
    break;

  case 80:
#line 482 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 81:
#line 483 "parser.y"
    { if (!((yyval.parse_node) = merge_decl_specs((yyvsp[(1) - (2)].parse_node), (yyvsp[(2) - (2)].parse_node)))) panic("abort the parse here (YYERROR)\n"); ;}
    break;

  case 82:
#line 484 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 83:
#line 485 "parser.y"
    { if (!((yyval.parse_node) = merge_decl_specs((yyvsp[(1) - (2)].parse_node), (yyvsp[(2) - (2)].parse_node)))) panic("abort the parse here (YYERROR)\n"); ;}
    break;

  case 84:
#line 489 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 85:
#line 490 "parser.y"
    { panic(""); ;}
    break;

  case 86:
#line 494 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 87:
#line 495 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (3)].parse_node); (yyval.parse_node)->declarator.initializer = (yyvsp[(3) - (3)].parse_node); ;}
    break;

  case 88:
#line 499 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_typedef = true; ;}
    break;

  case 89:
#line 500 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_extern = true; ;}
    break;

  case 90:
#line 501 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_static = true; ;}
    break;

  case 91:
#line 502 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_auto = true; ;}
    break;

  case 92:
#line 503 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_register = true; ;}
    break;

  case 93:
#line 507 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_void = true; ;}
    break;

  case 94:
#line 508 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_char = true; ;}
    break;

  case 95:
#line 509 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_short = true; ;}
    break;

  case 96:
#line 510 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_int = true; ;}
    break;

  case 97:
#line 511 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_long = true; ;}
    break;

  case 98:
#line 512 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_float = true; ;}
    break;

  case 99:
#line 513 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_double = true; ;}
    break;

  case 100:
#line 514 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_signed = true; ;}
    break;

  case 101:
#line 515 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_unsigned = true; ;}
    break;

  case 102:
#line 516 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_struct_or_union_spec = true; ;}
    break;

  case 103:
#line 517 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_enum_spec = true; ;}
    break;

  case 104:
#line 518 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_type_name = true; ;}
    break;

  case 105:
#line 522 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_const = true; ;}
    break;

  case 106:
#line 523 "parser.y"
    { ((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECL_SPEC; (yyval.parse_node)->decl_spec.is_volatile = true; ;}
    break;

  case 115:
#line 547 "parser.y"
    { if (!((yyval.parse_node) = merge_decl_specs((yyvsp[(1) - (2)].parse_node), (yyvsp[(2) - (2)].parse_node)))) panic("abort the parse here (YYERROR)\n"); ;}
    break;

  case 116:
#line 548 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 117:
#line 549 "parser.y"
    { if (!((yyval.parse_node) = merge_decl_specs((yyvsp[(1) - (2)].parse_node), (yyvsp[(2) - (2)].parse_node)))) panic("abort the parse here (YYERROR)\n"); ;}
    break;

  case 118:
#line 550 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 131:
#line 581 "parser.y"
    { panic(""); ;}
    break;

  case 132:
#line 582 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node) ;}
    break;

  case 133:
#line 587 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_DECLARATOR;
			(yyval.parse_node)->declarator.type = DECLARATOR_TYPE_IDENT;
			(yyval.parse_node)->declarator.id = (yyvsp[(1) - (1)].ident);
		;}
    break;

  case 134:
#line 592 "parser.y"
    { panic(""); ;}
    break;

  case 135:
#line 593 "parser.y"
    { panic(""); ;}
    break;

  case 136:
#line 594 "parser.y"
    { panic(""); ;}
    break;

  case 137:
#line 595 "parser.y"
    { panic(""); ;}
    break;

  case 138:
#line 596 "parser.y"
    { panic(""); ;}
    break;

  case 139:
#line 597 "parser.y"
    { panic(""); ;}
    break;

  case 144:
#line 608 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 145:
#line 609 "parser.y"
    { if (!((yyval.parse_node) = merge_decl_specs((yyvsp[(1) - (2)].parse_node), (yyvsp[(2) - (2)].parse_node)))) panic("abort the parse here (YYERROR)\n"); ;}
    break;

  case 155:
#line 635 "parser.y"
    { panic(""); ;}
    break;

  case 156:
#line 636 "parser.y"
    { panic(""); ;}
    break;

  case 170:
#line 659 "parser.y"
    { panic(""); ;}
    break;

  case 171:
#line 660 "parser.y"
    { panic(""); ;}
    break;

  case 180:
#line 678 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 181:
#line 679 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 182:
#line 680 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 183:
#line 685 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
		;}
    break;

  case 184:
#line 689 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			(yyval.parse_node)->compound_stmt.stmt_list = (yyvsp[(2) - (3)].parse_node);
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		;}
    break;

  case 185:
#line 696 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			(yyval.parse_node)->compound_stmt.decl_list = (yyvsp[(2) - (3)].parse_node);
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		;}
    break;

  case 186:
#line 703 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			(yyval.parse_node)->compound_stmt.decl_list = (yyvsp[(2) - (4)].parse_node);
			(yyval.parse_node)->compound_stmt.stmt_list = (yyvsp[(3) - (4)].parse_node);
			if (last_list_node_stack_ptr-- == 0)
				panic("");
			if (last_list_node_stack_ptr-- == 0)
				panic("");
		;}
    break;

  case 187:
#line 716 "parser.y"
    {
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			(yyval.parse_node) = last_list_node_stack[last_list_node_stack_ptr++] = (yyvsp[(1) - (1)].parse_node);
		;}
    break;

  case 188:
#line 722 "parser.y"
    {
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= (yyvsp[(2) - (2)].parse_node);
			(yyval.parse_node) = (yyvsp[(1) - (2)].parse_node);
		;}
    break;

  case 189:
#line 732 "parser.y"
    {
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			(yyval.parse_node) = last_list_node_stack[last_list_node_stack_ptr++] = (yyvsp[(1) - (1)].parse_node);
		;}
    break;

  case 190:
#line 738 "parser.y"
    {
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= (yyvsp[(2) - (2)].parse_node);
			(yyval.parse_node) = (yyvsp[(1) - (2)].parse_node);
		;}
    break;

  case 191:
#line 747 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 192:
#line 748 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (2)].parse_node); ;}
    break;

  case 193:
#line 753 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			(yyval.parse_node)->select_stmt.type = /*xlat_unit_parse_node::*//*select_stmt::*/SELECT_STMT_IF;
			(yyval.parse_node)->select_stmt.expr = (yyvsp[(3) - (5)].parse_node);
			(yyval.parse_node)->select_stmt.stmt = (yyvsp[(5) - (5)].parse_node);
		;}
    break;

  case 194:
#line 760 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			(yyval.parse_node)->select_stmt.type = /*xlat_unit_parse_node::*//*select_stmt::*/SELECT_STMT_IF_ELSE;
			(yyval.parse_node)->select_stmt.expr = (yyvsp[(3) - (7)].parse_node);
			(yyval.parse_node)->select_stmt.stmt = (yyvsp[(5) - (7)].parse_node);
			(yyval.parse_node)->select_stmt.else_stmt = (yyvsp[(7) - (7)].parse_node);
		;}
    break;

  case 195:
#line 768 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_SELECT_STMT;
			(yyval.parse_node)->select_stmt.type = /*xlat_unit_parse_node::*//*select_stmt::*/SELECT_STMT_SWITCH;
			(yyval.parse_node)->select_stmt.expr = (yyvsp[(3) - (5)].parse_node);
			(yyval.parse_node)->select_stmt.stmt = (yyvsp[(5) - (5)].parse_node);
		;}
    break;

  case 196:
#line 778 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			(yyval.parse_node)->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_WHILE;
			(yyval.parse_node)->iterate_stmt.expr = (yyvsp[(3) - (5)].parse_node);
			(yyval.parse_node)->iterate_stmt.stmt = (yyvsp[(5) - (5)].parse_node);
		;}
    break;

  case 197:
#line 785 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			(yyval.parse_node)->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_DO_WHILE;
			(yyval.parse_node)->iterate_stmt.expr = (yyvsp[(5) - (7)].parse_node);
			(yyval.parse_node)->iterate_stmt.stmt = (yyvsp[(2) - (7)].parse_node);
		;}
    break;

  case 198:
#line 792 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			(yyval.parse_node)->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_FOR_EXPR_EXPR;
			(yyval.parse_node)->iterate_stmt.expr = (yyvsp[(3) - (6)].parse_node);
			(yyval.parse_node)->iterate_stmt.stmt = (yyvsp[(6) - (6)].parse_node);
			(yyval.parse_node)->iterate_stmt.for_expr_ctrl = (yyvsp[(4) - (6)].parse_node);
		;}
    break;

  case 199:
#line 800 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_ITERATE_STMT;
			(yyval.parse_node)->iterate_stmt.type = /*xlat_unit_parse_node::*//*iterate_stmt::*/ITER_STMT_FOR_EXPR_EXPR_EXPR;
			(yyval.parse_node)->iterate_stmt.expr = (yyvsp[(3) - (7)].parse_node);
			(yyval.parse_node)->iterate_stmt.stmt = (yyvsp[(7) - (7)].parse_node);
			(yyval.parse_node)->iterate_stmt.for_expr_ctrl = (yyvsp[(4) - (7)].parse_node);
			(yyval.parse_node)->iterate_stmt.for_expr_post = (yyvsp[(5) - (7)].parse_node);
		;}
    break;

  case 200:
#line 812 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			(yyval.parse_node)->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_GOTO;
			(yyval.parse_node)->jump_stmt.details.id = (yyvsp[(2) - (3)].ident);
		;}
    break;

  case 201:
#line 818 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			(yyval.parse_node)->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_CONTINUE;
		;}
    break;

  case 202:
#line 823 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			(yyval.parse_node)->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_BREAK;
		;}
    break;

  case 203:
#line 828 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			(yyval.parse_node)->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_RETURN;
		;}
    break;

  case 204:
#line 833 "parser.y"
    {
			((yyval.parse_node) = get_xlat_node())->type = /*xlat_unit_parse_node::*/PARSE_NODE_JUMP_STMT;
			(yyval.parse_node)->jump_stmt.type = /*xlat_unit_parse_node::*//*jump_stmt::*/JUMP_STMT_RETURN;
			(yyval.parse_node)->jump_stmt.details.expr = (yyvsp[(2) - (3)].parse_node);
		;}
    break;

  case 205:
#line 842 "parser.y"
    {
			if (last_list_node_stack_ptr == MAX_LAST_LIST_NODE_STACK_DEPTH)
				panic("");
			parse_head = (yyval.parse_node) = last_list_node_stack[last_list_node_stack_ptr++] = (yyvsp[(1) - (1)].parse_node);
		;}
    break;

  case 206:
#line 848 "parser.y"
    {
			last_list_node_stack[last_list_node_stack_ptr - 1]
				= last_list_node_stack[last_list_node_stack_ptr - 1]->next
				= (yyvsp[(2) - (2)].parse_node);
			parse_head = (yyval.parse_node) = (yyvsp[(1) - (2)].parse_node);
		;}
    break;

  case 207:
#line 857 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 208:
#line 858 "parser.y"
    { (yyval.parse_node) = (yyvsp[(1) - (1)].parse_node); ;}
    break;

  case 209:
#line 859 "parser.y"
    { (yyval.parse_node) = (yyvsp[(2) - (3)].parse_node); ;}
    break;

  case 210:
#line 863 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 211:
#line 864 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 212:
#line 865 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;

  case 213:
#line 866 "parser.y"
    { (yyval.parse_node) = 0; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3186 "parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, x, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (&yylloc, x, yymsg);
	  }
	else
	  {
	    yyerror (&yylloc, x, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, x);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, x);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, x, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, x);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, x);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 869 "parser.y"

#include <stdio.h>

extern char yytext[];
extern int column;

#if __CXX__
void clang_parser::parser::error(const location_type * loc, const std::string& msg)
#else
static void yyerror(const YYLTYPE * loc, void * x, const char * msg)
#endif
{
	fflush(stdout);
#if __CXX__
	printf("\n%*s\n%*s", loc->begin.column, "^", loc->begin.column, "");
	cout << msg << '\n';
#else
	printf("\n%*s\n%*s\n", loc->first_column, "^", loc->first_column, msg);
#endif
}

static struct xlat_unit_parse_node * merge_decl_specs(struct xlat_unit_parse_node * n1, struct xlat_unit_parse_node * n2)
{
struct decl_spec_node tmp, tmp1;
int i;

	if (n1->type != PARSE_NODE_DECL_SPEC
		|| n2->type != PARSE_NODE_DECL_SPEC)
		panic("");
	/* check for semantic inconsistencies */
	if (n1->decl_spec.storage_class_flags & n2->decl_spec.storage_class_flags
		|| n1->decl_spec.type_qualifier_flags & n2->decl_spec.type_qualifier_flags
		|| n1->decl_spec.type_specifier_flags & n2->decl_spec.type_specifier_flags
		|| n1->decl_spec.detail && n2->decl_spec.detail)
	{
		printf("error: bad type declaration specifier list\n");
		goto error;
	}
	tmp = n1->decl_spec;

	tmp.storage_class_flags |= n2->decl_spec.storage_class_flags;
	tmp1 = tmp;
	if (i = tmp1.storage_class_flags)
	{
		/* see if more than one storage class flags have
		 * been requested */
		/* set the first nonzero bit to zero, then see if more
		 * nonzero bits remain */
		if ((((i - 1) & ~i) + 1) ^ i)
		{
			printf("error: too many storage class specifiers\n");
			goto error;
		}
	}
	/* nothing to check about the type qualifier flags */
	tmp.type_qualifier_flags |= n2->decl_spec.type_qualifier_flags;
	tmp.type_specifier_flags |= n2->decl_spec.type_specifier_flags;
	tmp1 = tmp;
	if (i = tmp1.type_specifier_flags)
	{
		/* see if the requested type specifier flags are incompatible */
		if (tmp1.is_void
			|| tmp1.is_float
			|| /*!\todo this excludes long double - add it */tmp1.is_double
			|| tmp1.is_struct_or_union_spec
			|| tmp1.is_enum_spec
			|| tmp1.is_type_name)
		{
			/* set the first nonzero bit to zero, then see if more
			 * nonzero bits remain */
			if ((((i - 1) & ~i) + 1) ^ i)
			{
				printf("error: incompatible type specifiers requested\n");
				goto error;
			}
		}
		else
		{
			if ((tmp1.is_signed && tmp1.is_unsigned)
				|| (tmp1.is_long && tmp1.is_short))
			{
				printf("error: incompatible type specifiers requested\n");
				goto error;
			}
			tmp1.is_signed = tmp1.is_unsigned
				= tmp1.is_long = tmp1.is_short = false;

			/* set the first nonzero bit to zero, then see if more
			 * nonzero bits remain */
			if ((((i - 1) & ~i) + 1) ^ i)
			{
				printf("error: incompatible type specifiers requested\n");
				goto error;
			}
		}
	}
	if (n1->decl_spec.detail && n2->decl_spec.detail)
	{
		printf("error: incompatible type specifiers requested\n");
		goto error;
	}
	if (n1->decl_spec.detail)
		tmp.detail = n1->decl_spec.detail;
	else
		tmp.detail = n2->decl_spec.detail;
	/* no error */
	n1->decl_spec = tmp;
	free(n2);
	return n1;

error:
	free(n1);
	free(n2);
	return 0;
}


