/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 2 "c-parse.y"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "c-parse-types.h"

extern char yytext[];
extern int column;

yyerror(s)
char *s;
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}

struct expr * parse_head;

int main(int argc, char * argv[])
{
	return yyparse();
}

#define ALLOC_NODE(x)	(x *) calloc(1, sizeof(x))



/* Line 189 of yacc.c  */
#line 101 "c-parse.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 231 "c-parse.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 243 "c-parse.tab.c"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  95
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   284

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  82
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  28
/* YYNRULES -- Number of rules.  */
#define YYNRULES  108
/* YYNRULES -- Number of states.  */
#define YYNSTATES  160

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
       2,     2,     2,     2,     2,     2,     2,     2,    80,     2,
      75,    81,    76,    79,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,    64,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    78,     2,    71,     2,     2,     2,
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
     209,   211,   213,   215,   217,   219,   221,   223,   225,   227,
     229,   231,   233,   236,   239,   242,   244,   247,   249,   252,
     254,   256,   258,   260,   263,   266,   270,   272,   275,   277,
     280,   282,   284,   287,   291,   294,   298,   302,   307
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
     100,     0,    -1,    58,    -1,    59,    -1,    60,    -1,    61,
     100,    62,    -1,    83,    -1,    84,    63,   100,    64,    -1,
      84,    61,    62,    -1,    84,    61,    85,    62,    -1,    84,
      65,    58,    -1,    84,     4,    58,    -1,    84,     5,    -1,
      84,     6,    -1,   100,    -1,    85,    66,   100,    -1,    84,
      -1,     5,    86,    -1,     6,    86,    -1,    87,    88,    -1,
       3,    86,    -1,     3,    61,   107,    62,    -1,    67,    -1,
      68,    -1,    69,    -1,    70,    -1,    71,    -1,    72,    -1,
      86,    -1,    61,   107,    62,    88,    -1,    88,    -1,    89,
      68,    88,    -1,    89,    73,    88,    -1,    89,    74,    88,
      -1,    89,    -1,    90,    69,    89,    -1,    90,    70,    89,
      -1,    90,    -1,    91,     7,    90,    -1,    91,     8,    90,
      -1,    91,    -1,    92,    75,    91,    -1,    92,    76,    91,
      -1,    92,     9,    91,    -1,    92,    10,    91,    -1,    92,
      -1,    93,    11,    92,    -1,    93,    12,    92,    -1,    93,
      -1,    94,    67,    93,    -1,    94,    -1,    95,    77,    94,
      -1,    95,    -1,    96,    78,    95,    -1,    96,    -1,    97,
      13,    96,    -1,    97,    -1,    98,    14,    97,    -1,    98,
      -1,    98,    79,   100,    80,    99,    -1,    99,    -1,    86,
     101,   100,    -1,    81,    -1,    15,    -1,    16,    -1,    17,
      -1,    18,    -1,    19,    -1,    20,    -1,    21,    -1,    22,
      -1,    23,    -1,    24,    -1,    41,    -1,    31,    -1,    32,
      -1,    33,    -1,    34,    -1,    37,    -1,    38,    -1,    35,
      -1,    36,    -1,    42,    58,    -1,    43,    58,    -1,    44,
      58,    -1,    25,    -1,   102,   103,    -1,   102,    -1,   104,
     103,    -1,   104,    -1,    39,    -1,    40,    -1,    68,    -1,
      68,   106,    -1,    68,   105,    -1,    68,   106,   105,    -1,
     104,    -1,   106,   104,    -1,   103,    -1,   103,   108,    -1,
     105,    -1,   109,    -1,   105,   109,    -1,    61,   108,    62,
      -1,    63,    64,    -1,    63,    99,    64,    -1,   109,    63,
      64,    -1,   109,    63,    99,    64,    -1,    61,    62,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   112,   112,   113,   114,   115,   119,   120,   121,   122,
     123,   124,   125,   126,   130,   131,   135,   136,   137,   138,
     139,   140,   144,   145,   146,   147,   148,   149,   153,   154,
     159,   160,   161,   162,   166,   167,   168,   172,   173,   174,
     178,   179,   180,   181,   182,   186,   187,   188,   192,   193,
     197,   198,   202,   203,   207,   208,   212,   213,   217,   218,
     222,   223,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   257,   258,   259,   260,
     264,   265,   269,   270,   271,   272,   276,   277,   281,   282,
     286,   287,   288,   292,   293,   294,   295,   296,   297
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP",
  "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP",
  "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN",
  "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN",
  "OR_ASSIGN", "TYPE_NAME", "TYPEDEF", "EXTERN", "STATIC", "AUTO",
  "REGISTER", "CHAR_T", "SHORT_T", "INT_T", "LONG_T", "SIGNED", "UNSIGNED",
  "FLOAT_T", "DOUBLE_T", "CONST_T", "VOLATILE", "VOID_T", "STRUCT",
  "UNION", "ENUM", "ELLIPSIS", "CASE", "DEFAULT", "IF", "ELSE", "SWITCH",
  "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN",
  "IDENTIFIER", "CONSTANT", "STRING_LITERAL", "'('", "')'", "'['", "']'",
  "'.'", "','", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'", "'%'",
  "'<'", "'>'", "'^'", "'|'", "'?'", "':'", "'='", "$accept",
  "primary_expression", "postfix_expression", "argument_expression_list",
  "unary_expression", "unary_operator", "cast_expression",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_or_expression",
  "conditional_expression", "expression", "assignment_operator",
  "type_specifier", "specifier_qualifier_list", "type_qualifier",
  "pointer", "type_qualifier_list", "type_name", "abstract_declarator",
  "direct_abstract_declarator", 0
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
      58,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    82,    83,    83,    83,    83,    84,    84,    84,    84,
      84,    84,    84,    84,    85,    85,    86,    86,    86,    86,
      86,    86,    87,    87,    87,    87,    87,    87,    88,    88,
      89,    89,    89,    89,    90,    90,    90,    91,    91,    91,
      92,    92,    92,    92,    92,    93,    93,    93,    94,    94,
      95,    95,    96,    96,    97,    97,    98,    98,    99,    99,
     100,   100,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   103,   103,   103,   103,
     104,   104,   105,   105,   105,   105,   106,   106,   107,   107,
     108,   108,   108,   109,   109,   109,   109,   109,   109
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     1,     2,     1,     2,     1,
       1,     1,     1,     2,     2,     3,     1,     2,     1,     2,
       1,     1,     2,     3,     2,     3,     3,     4,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     2,     3,     4,     0,    22,    23,
      24,    25,    26,    27,     6,    16,    28,     0,    30,    34,
      37,    40,    45,    48,    50,    52,    54,    56,    58,    60,
       0,     0,    20,     0,    17,    18,    85,    74,    75,    76,
      77,    80,    81,    78,    79,    90,    91,    73,     0,     0,
       0,     0,    87,    98,    89,     0,     0,    12,    13,     0,
       0,     0,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    62,     0,    28,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     0,    82,    83,    84,
       5,    86,     0,     0,    92,   100,    99,   101,    88,     0,
      11,     8,     0,    14,     0,    10,    61,    31,    32,    33,
      35,    36,    38,    39,    43,    44,    41,    42,    46,    47,
      49,    51,    53,    55,    57,     0,    21,   108,     0,   104,
       0,    96,    94,    93,   102,     0,    29,     9,     0,     7,
       0,   103,   105,    97,    95,   106,     0,    15,    59,   107
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    14,    15,   112,    74,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    51,    73,
      52,    53,    54,   105,   143,    55,   106,   107
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -100
static const yytype_int16 yypact[] =
{
     156,   177,   192,   192,  -100,  -100,  -100,    47,  -100,  -100,
    -100,  -100,  -100,  -100,  -100,   145,    -6,   156,  -100,   -32,
     -24,    88,    17,    92,   -42,   -40,   -35,    35,    -8,  -100,
      54,    47,  -100,   156,  -100,  -100,  -100,  -100,  -100,  -100,
    -100,  -100,  -100,  -100,  -100,  -100,  -100,  -100,     7,    12,
      16,    14,   240,   -29,   240,    49,    39,  -100,  -100,    96,
     156,    63,  -100,  -100,  -100,  -100,  -100,  -100,  -100,  -100,
    -100,  -100,  -100,   156,  -100,  -100,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,  -100,    64,  -100,  -100,  -100,
    -100,  -100,   -33,   117,   -19,   -39,  -100,    65,  -100,   156,
    -100,  -100,   -15,  -100,    66,  -100,  -100,  -100,  -100,  -100,
     -32,   -32,   -24,   -24,    88,    88,    88,    88,    17,    17,
      92,   -42,   -40,   -35,    35,    51,  -100,  -100,    67,  -100,
      68,  -100,  -100,   -19,    65,   132,  -100,  -100,   156,  -100,
     156,  -100,  -100,  -100,  -100,  -100,    69,  -100,  -100,  -100
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -100,  -100,  -100,  -100,     0,  -100,    -9,    30,    31,   -28,
      37,    38,    44,    45,    48,    46,  -100,   -84,     4,  -100,
    -100,   -14,   -99,   -81,  -100,   110,    40,    41
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      16,    32,    34,    35,    30,   141,    93,    16,    75,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,   140,
      45,    46,   102,   142,   103,    89,    83,    84,   102,   137,
     103,    16,   102,    16,   103,   104,    76,    90,   101,   104,
     108,    77,    78,    91,   153,    79,    80,   147,    92,   104,
       1,   148,     2,     3,    95,   124,   125,   126,   127,    16,
      16,   156,   154,   113,   114,    97,   158,   117,   118,   119,
      98,    94,    36,    16,    99,    72,   100,   116,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    85,    86,    16,    81,    82,   110,   135,     1,
     146,     2,     3,    87,    88,     4,     5,     6,     7,   120,
     121,   109,   122,   123,     8,     9,    10,    11,    12,    13,
       1,   115,     2,     3,   128,   129,   136,   130,   145,   151,
     149,   150,   152,   159,   131,     1,   132,     2,     3,   134,
     133,    96,   138,     0,     0,     0,   144,     0,    16,    56,
      57,    58,   157,     0,     4,     5,     6,     7,   111,     1,
       0,     2,     3,     8,     9,    10,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     0,
       1,   139,     2,     3,     8,     9,    10,    11,    12,    13,
       4,     5,     6,     7,     0,     1,   155,     2,     3,     8,
       9,    10,    11,    12,    13,     0,    59,     0,    60,     0,
      61,     0,     0,     0,     4,     5,     6,     7,     0,     0,
       0,     0,     0,     8,     9,    10,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     4,     5,     6,    31,     0,
       0,     0,     0,     0,     8,     9,    10,    11,    12,    13,
       4,     5,     6,    33,     0,     0,     0,     0,     0,     8,
       9,    10,    11,    12,    13,    36,     0,     0,     0,     0,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50
};

static const yytype_int16 yycheck[] =
{
       0,     1,     2,     3,     0,   104,    14,     7,    17,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,   103,
      39,    40,    61,   104,    63,    67,     9,    10,    61,    62,
      63,    31,    61,    33,    63,    68,    68,    77,    52,    68,
      54,    73,    74,    78,   143,    69,    70,    62,    13,    68,
       3,    66,     5,     6,     0,    83,    84,    85,    86,    59,
      60,   145,   143,    59,    60,    58,   150,    76,    77,    78,
      58,    79,    25,    73,    58,    81,    62,    73,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    75,    76,    94,     7,     8,    58,    94,     3,
     109,     5,     6,    11,    12,    58,    59,    60,    61,    79,
      80,    62,    81,    82,    67,    68,    69,    70,    71,    72,
       3,    58,     5,     6,    87,    88,    62,    89,    63,    62,
      64,    80,    64,    64,    90,     3,    91,     5,     6,    93,
      92,    31,   102,    -1,    -1,    -1,   105,    -1,   148,     4,
       5,     6,   148,    -1,    58,    59,    60,    61,    62,     3,
      -1,     5,     6,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,    -1,
       3,    64,     5,     6,    67,    68,    69,    70,    71,    72,
      58,    59,    60,    61,    -1,     3,    64,     5,     6,    67,
      68,    69,    70,    71,    72,    -1,    61,    -1,    63,    -1,
      65,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,    -1,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      58,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     5,     6,    58,    59,    60,    61,    67,    68,
      69,    70,    71,    72,    83,    84,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    61,    86,    61,    86,    86,    25,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,   100,   102,   103,   104,   107,     4,     5,     6,    61,
      63,    65,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    81,   101,    86,    88,    68,    73,    74,    69,
      70,     7,     8,     9,    10,    75,    76,    11,    12,    67,
      77,    78,    13,    14,    79,     0,   107,    58,    58,    58,
      62,   103,    61,    63,    68,   105,   108,   109,   103,    62,
      58,    62,    85,   100,   100,    58,   100,    88,    88,    88,
      89,    89,    90,    90,    91,    91,    91,    91,    92,    92,
      93,    94,    95,    96,    97,   100,    62,    62,   108,    64,
      99,   104,   105,   106,   109,    63,    88,    62,    66,    64,
      80,    62,    64,   104,   105,    64,    99,   100,    99,    64
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
      yyerror (YY_("syntax error: cannot back up")); \
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
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
		  Type, Value); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 100: /* "expression" */

/* Line 1009 of yacc.c  */
#line 45 "c-parse.y"
	{ parse_head = (yyvaluep->expr); };

/* Line 1009 of yacc.c  */
#line 1342 "c-parse.tab.c"
	break;

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
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

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

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1464 of yacc.c  */
#line 112 "c-parse.y"
    { (yyval.primary_expr) = ALLOC_NODE(struct primary_expr); (yyval.primary_expr)->ident = strdup((yyvsp[(1) - (1)].str)); ;}
    break;

  case 3:

/* Line 1464 of yacc.c  */
#line 113 "c-parse.y"
    { (yyval.primary_expr) = ALLOC_NODE(struct primary_expr); (yyval.primary_expr)->const_kind = (yyvsp[(1) - (1)].const_info).kind; if ((yyvsp[(1) - (1)].const_info).kind == CONST_INT) (yyval.primary_expr)->iconst = (yyvsp[(1) - (1)].const_info).iconst; else (yyval.primary_expr)->flconst = (yyvsp[(1) - (1)].const_info).flconst; ;}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 114 "c-parse.y"
    { (yyval.primary_expr) = ALLOC_NODE(struct primary_expr); (yyval.primary_expr)->str_literal = (yyvsp[(1) - (1)].str); ;}
    break;

  case 5:

/* Line 1464 of yacc.c  */
#line 115 "c-parse.y"
    { (yyval.primary_expr) = ALLOC_NODE(struct primary_expr); (yyval.primary_expr)->expr = (yyvsp[(2) - (3)].expr); ;}
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 119 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->primary_expr = (yyvsp[(1) - (1)].primary_expr); ;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 120 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (4)].postfix_expr); (yyval.postfix_expr)->expr = (yyvsp[(3) - (4)].expr); (yyval.postfix_expr)->op = ARR_SUBSCRIPT; ;}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 121 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (3)].postfix_expr); (yyval.postfix_expr)->op = FUNC_CALL; ;}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 122 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (4)].postfix_expr); (yyval.postfix_expr)->args = (yyvsp[(3) - (4)].args) ; (yyval.postfix_expr)->op = FUNC_CALL; ;}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 123 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (3)].postfix_expr); (yyval.postfix_expr)->ident = strdup((yyvsp[(3) - (3)].str)) ; (yyval.postfix_expr)->op = STRUCT_MEMBER_ACCESS; ;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 124 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (3)].postfix_expr); (yyval.postfix_expr)->ident = strdup((yyvsp[(3) - (3)].str)) ; (yyval.postfix_expr)->op = STRUCT_MEMBER_PTR_ACCESS; ;}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 125 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (2)].postfix_expr); (yyval.postfix_expr)->op = POSTFIX_INC_OP; ;}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 126 "c-parse.y"
    { (yyval.postfix_expr) = ALLOC_NODE(struct postfix_expr); (yyval.postfix_expr)->postfix_expr = (yyvsp[(1) - (2)].postfix_expr); (yyval.postfix_expr)->op = POSTFIX_DEC_OP; ;}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 130 "c-parse.y"
    { (yyval.args) = ALLOC_NODE(struct arg_expr_list); (yyval.args)->expr = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 131 "c-parse.y"
    { (yyval.args) = ALLOC_NODE(struct arg_expr_list); (yyval.args)->expr = (yyvsp[(3) - (3)].expr); (yyval.args)->next = (yyvsp[(1) - (3)].args); ;}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 135 "c-parse.y"
    { (yyval.unary_expr) = ALLOC_NODE(struct unary_expr); (yyval.unary_expr)->postfix_expr = (yyvsp[(1) - (1)].postfix_expr); ;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 136 "c-parse.y"
    { (yyval.unary_expr) = ALLOC_NODE(struct unary_expr); (yyval.unary_expr)->unary_expr = (yyvsp[(2) - (2)].unary_expr); (yyval.unary_expr)->op = PREFIX_INC_OP; ;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 137 "c-parse.y"
    { (yyval.unary_expr) = ALLOC_NODE(struct unary_expr); (yyval.unary_expr)->unary_expr = (yyvsp[(2) - (2)].unary_expr); (yyval.unary_expr)->op = PREFIX_DEC_OP; ;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 138 "c-parse.y"
    { (yyval.unary_expr) = ALLOC_NODE(struct unary_expr); (yyval.unary_expr)->cast_expr = (yyvsp[(2) - (2)].cast_expr); (yyval.unary_expr)->op = (yyvsp[(1) - (2)].unary_op); ;}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 139 "c-parse.y"
    { (yyval.unary_expr) = ALLOC_NODE(struct unary_expr); (yyval.unary_expr)->unary_expr = (yyvsp[(2) - (2)].unary_expr); (yyval.unary_expr)->op = SIZEOF_OP; ;}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 140 "c-parse.y"
    { (yyval.unary_expr) = ALLOC_NODE(struct unary_expr); (yyval.unary_expr)->type_name = (yyvsp[(3) - (4)].type_name); (yyval.unary_expr)->op = SIZEOF_OP; ;}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 144 "c-parse.y"
    { (yyval.unary_op) = UN_AMPERSAND; ;}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 145 "c-parse.y"
    { (yyval.unary_op) = UN_ASTERISK; ;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 146 "c-parse.y"
    { (yyval.unary_op) = UN_PLUS; ;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 147 "c-parse.y"
    { (yyval.unary_op) = UN_MINUS; ;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 148 "c-parse.y"
    { (yyval.unary_op) = UN_TILDE; ;}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 149 "c-parse.y"
    { (yyval.unary_op) = UN_EXCLAM; ;}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 153 "c-parse.y"
    { (yyval.cast_expr) = ALLOC_NODE(struct cast_expr); (yyval.cast_expr)->unary_expr = (yyvsp[(1) - (1)].unary_expr); ;}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 154 "c-parse.y"
    { (yyval.cast_expr) = ALLOC_NODE(struct cast_expr); (yyval.cast_expr)->type_name = (yyvsp[(2) - (4)].type_name); (yyval.cast_expr)->cast_expr = (yyvsp[(4) - (4)].cast_expr); ;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 159 "c-parse.y"
    { (yyval.mult_expr) = ALLOC_NODE(struct mult_expr); (yyval.mult_expr)->cast_expr = (yyvsp[(1) - (1)].cast_expr); ;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 160 "c-parse.y"
    { (yyval.mult_expr) = ALLOC_NODE(struct mult_expr); (yyval.mult_expr)->mult_expr = (yyvsp[(1) - (3)].mult_expr); (yyval.mult_expr)->op = MULT_TIMES; (yyval.mult_expr)->cast_expr = (yyvsp[(3) - (3)].cast_expr); ;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 161 "c-parse.y"
    { (yyval.mult_expr) = ALLOC_NODE(struct mult_expr); (yyval.mult_expr)->mult_expr = (yyvsp[(1) - (3)].mult_expr); (yyval.mult_expr)->op = MULT_DIV; (yyval.mult_expr)->cast_expr = (yyvsp[(3) - (3)].cast_expr); ;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 162 "c-parse.y"
    { (yyval.mult_expr) = ALLOC_NODE(struct mult_expr); (yyval.mult_expr)->mult_expr = (yyvsp[(1) - (3)].mult_expr); (yyval.mult_expr)->op = MULT_MOD; (yyval.mult_expr)->cast_expr = (yyvsp[(3) - (3)].cast_expr); ;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 166 "c-parse.y"
    { (yyval.additive_expr) = ALLOC_NODE(struct additive_expr); (yyval.additive_expr)->mult_expr = (yyvsp[(1) - (1)].mult_expr); ;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 167 "c-parse.y"
    { (yyval.additive_expr) = ALLOC_NODE(struct additive_expr); (yyval.additive_expr)->op = ADDITIVE_PLUS; (yyval.additive_expr)->additive_expr = (yyvsp[(1) - (3)].additive_expr); (yyval.additive_expr)->mult_expr = (yyvsp[(3) - (3)].mult_expr); ;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 168 "c-parse.y"
    { (yyval.additive_expr) = ALLOC_NODE(struct additive_expr); (yyval.additive_expr)->op = ADDITIVE_MINUS; (yyval.additive_expr)->additive_expr = (yyvsp[(1) - (3)].additive_expr); (yyval.additive_expr)->mult_expr = (yyvsp[(3) - (3)].mult_expr); ;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 172 "c-parse.y"
    { (yyval.shift_expr) = ALLOC_NODE(struct shift_expr); (yyval.shift_expr)->additive_expr = (yyvsp[(1) - (1)].additive_expr); ;}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 173 "c-parse.y"
    { (yyval.shift_expr) = ALLOC_NODE(struct shift_expr); (yyval.shift_expr)->shift_expr = (yyvsp[(1) - (3)].shift_expr); (yyval.shift_expr)->op = SHIFT_LEFT; (yyval.shift_expr)->additive_expr = (yyvsp[(3) - (3)].additive_expr); ;}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 174 "c-parse.y"
    { (yyval.shift_expr) = ALLOC_NODE(struct shift_expr); (yyval.shift_expr)->shift_expr = (yyvsp[(1) - (3)].shift_expr); (yyval.shift_expr)->op = SHIFT_RIGHT; (yyval.shift_expr)->additive_expr = (yyvsp[(3) - (3)].additive_expr); ;}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 178 "c-parse.y"
    { (yyval.rel_expr) = ALLOC_NODE(struct rel_expr); (yyval.rel_expr)->shift_expr = (yyvsp[(1) - (1)].shift_expr); ;}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 179 "c-parse.y"
    { (yyval.rel_expr) = ALLOC_NODE(struct rel_expr); (yyval.rel_expr)->rel_expr = (yyvsp[(1) - (3)].rel_expr); (yyval.rel_expr)->op = REL_LESS; (yyval.rel_expr)->shift_expr = (yyvsp[(3) - (3)].shift_expr); ;}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 180 "c-parse.y"
    { (yyval.rel_expr) = ALLOC_NODE(struct rel_expr); (yyval.rel_expr)->rel_expr = (yyvsp[(1) - (3)].rel_expr); (yyval.rel_expr)->op = REL_GREATER; (yyval.rel_expr)->shift_expr = (yyvsp[(3) - (3)].shift_expr); ;}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 181 "c-parse.y"
    { (yyval.rel_expr) = ALLOC_NODE(struct rel_expr); (yyval.rel_expr)->rel_expr = (yyvsp[(1) - (3)].rel_expr); (yyval.rel_expr)->op = REL_LESS_OR_EQUAL; (yyval.rel_expr)->shift_expr = (yyvsp[(3) - (3)].shift_expr); ;}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 182 "c-parse.y"
    { (yyval.rel_expr) = ALLOC_NODE(struct rel_expr); (yyval.rel_expr)->rel_expr = (yyvsp[(1) - (3)].rel_expr); (yyval.rel_expr)->op = REL_GREATER_OR_EQUAL; (yyval.rel_expr)->shift_expr = (yyvsp[(3) - (3)].shift_expr); ;}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 186 "c-parse.y"
    { (yyval.eq_expr) = ALLOC_NODE(struct eq_expr); (yyval.eq_expr)->rel_expr = (yyvsp[(1) - (1)].rel_expr); ;}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 187 "c-parse.y"
    { (yyval.eq_expr) = ALLOC_NODE(struct eq_expr); (yyval.eq_expr)->eq_expr = (yyvsp[(1) - (3)].eq_expr); (yyval.eq_expr)->op = EQ_EQUAL; (yyval.eq_expr)->rel_expr = (yyvsp[(3) - (3)].rel_expr); ;}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 188 "c-parse.y"
    { (yyval.eq_expr) = ALLOC_NODE(struct eq_expr); (yyval.eq_expr)->eq_expr = (yyvsp[(1) - (3)].eq_expr); (yyval.eq_expr)->op = EQ_NOT_EQUAL; (yyval.eq_expr)->rel_expr = (yyvsp[(3) - (3)].rel_expr); ;}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 192 "c-parse.y"
    { (yyval.and_expr) = ALLOC_NODE(struct and_expr); (yyval.and_expr)->eq_expr = (yyvsp[(1) - (1)].eq_expr); ;}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 193 "c-parse.y"
    { (yyval.and_expr) = ALLOC_NODE(struct and_expr); (yyval.and_expr)->and_expr = (yyvsp[(1) - (3)].and_expr); (yyval.and_expr)->op = AND_OP_AMPERSAND; (yyval.and_expr)->eq_expr = (yyvsp[(3) - (3)].eq_expr); ;}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 197 "c-parse.y"
    { (yyval.xor_expr) = ALLOC_NODE(struct xor_expr); (yyval.xor_expr)->and_expr = (yyvsp[(1) - (1)].and_expr); ;}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 198 "c-parse.y"
    { (yyval.xor_expr) = ALLOC_NODE(struct xor_expr); (yyval.xor_expr)->xor_expr = (yyvsp[(1) - (3)].xor_expr); (yyval.xor_expr)->op = XOR_OP; (yyval.xor_expr)->and_expr = (yyvsp[(3) - (3)].and_expr); ;}
    break;

  case 52:

/* Line 1464 of yacc.c  */
#line 202 "c-parse.y"
    { (yyval.incl_or_expr) = ALLOC_NODE(struct incl_or_expr); (yyval.incl_or_expr)->xor_expr = (yyvsp[(1) - (1)].xor_expr); ;}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 203 "c-parse.y"
    { (yyval.incl_or_expr) = ALLOC_NODE(struct incl_or_expr); (yyval.incl_or_expr)->incl_or_expr = (yyvsp[(1) - (3)].incl_or_expr); (yyval.incl_or_expr)->op = INCL_OR_OP; (yyval.incl_or_expr)->xor_expr = (yyvsp[(3) - (3)].xor_expr); ;}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 207 "c-parse.y"
    { (yyval.logical_and_expr) = ALLOC_NODE(struct logical_and_expr); (yyval.logical_and_expr)->incl_or_expr = (yyvsp[(1) - (1)].incl_or_expr); ;}
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 208 "c-parse.y"
    { (yyval.logical_and_expr) = ALLOC_NODE(struct logical_and_expr); (yyval.logical_and_expr)->logical_and_expr = (yyvsp[(1) - (3)].logical_and_expr); (yyval.logical_and_expr)->op = LOGICAL_AND_OP; (yyval.logical_and_expr)->incl_or_expr = (yyvsp[(3) - (3)].incl_or_expr); ;}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 212 "c-parse.y"
    { (yyval.logical_or_expr) = ALLOC_NODE(struct logical_or_expr); (yyval.logical_or_expr)->logical_and_expr = (yyvsp[(1) - (1)].logical_and_expr); ;}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 213 "c-parse.y"
    { (yyval.logical_or_expr) = ALLOC_NODE(struct logical_or_expr); (yyval.logical_or_expr)->logical_or_expr = (yyvsp[(1) - (3)].logical_or_expr); (yyval.logical_or_expr)->op = LOGICAL_OR_OP; (yyval.logical_or_expr)->logical_and_expr = (yyvsp[(3) - (3)].logical_and_expr); ;}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 217 "c-parse.y"
    { (yyval.cond_expr) = ALLOC_NODE(struct cond_expr); (yyval.cond_expr)->logical_or_expr = (yyvsp[(1) - (1)].logical_or_expr); ;}
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 218 "c-parse.y"
    { (yyval.cond_expr) = ALLOC_NODE(struct cond_expr); (yyval.cond_expr)->logical_or_expr = (yyvsp[(1) - (5)].logical_or_expr); (yyval.cond_expr)->op = LOGICAL_TERNARY_OP; (yyval.cond_expr)->expr = (yyvsp[(3) - (5)].expr); (yyval.cond_expr)->cond_expr = (yyvsp[(5) - (5)].cond_expr); ;}
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 222 "c-parse.y"
    { (yyval.expr) = ALLOC_NODE(struct expr); (yyval.expr)->cond_expr = (yyvsp[(1) - (1)].cond_expr); ;}
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 223 "c-parse.y"
    { (yyval.expr) = ALLOC_NODE(struct expr); (yyval.expr)->unary_expr = (yyvsp[(1) - (3)].unary_expr); (yyval.expr)->op = (yyvsp[(2) - (3)].assignment_op); (yyval.expr)->expr = (yyvsp[(3) - (3)].expr); ;}
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 227 "c-parse.y"
    { (yyval.assignment_op) = SIMPLE_ASSIGN; ;}
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 228 "c-parse.y"
    { (yyval.assignment_op) = MUL_ASSIGN_OP; ;}
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 229 "c-parse.y"
    { (yyval.assignment_op) = DIV_ASSIGN_OP; ;}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 230 "c-parse.y"
    { (yyval.assignment_op) = MOD_ASSIGN_OP; ;}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 231 "c-parse.y"
    { (yyval.assignment_op) = ADD_ASSIGN_OP; ;}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 232 "c-parse.y"
    { (yyval.assignment_op) = SUB_ASSIGN_OP; ;}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 233 "c-parse.y"
    { (yyval.assignment_op) = LEFT_ASSIGN_OP; ;}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 234 "c-parse.y"
    { (yyval.assignment_op) = RIGHT_ASSIGN_OP; ;}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 235 "c-parse.y"
    { (yyval.assignment_op) = AND_ASSIGN_OP; ;}
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 236 "c-parse.y"
    { (yyval.assignment_op) = XOR_ASSIGN_OP; ;}
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 237 "c-parse.y"
    { (yyval.assignment_op) = OR_ASSIGN_OP; ;}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 241 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_VOID; ;}
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 242 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_CHAR; ;}
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 243 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_SHORT; ;}
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 244 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_INT; ;}
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 245 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_LONG; ;}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 246 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_FLOAT; ;}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 247 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_DOUBLE; ;}
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 248 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_SIGNED; ;}
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 249 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_UNSIGNED; ;}
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 250 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_STRUCT_SPEC; (yyval.type_spec).name = strdup((yyvsp[(2) - (2)].str)); ;}
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 251 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_UNION_SPEC; (yyval.type_spec).name = strdup((yyvsp[(2) - (2)].str)); ;}
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 252 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_ENUM_SPEC; (yyval.type_spec).name = strdup((yyvsp[(2) - (2)].str)); ;}
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 253 "c-parse.y"
    { /*$$ = ALLOC_NODE(struct type_spec);*/ (yyval.type_spec).tspec = T_TYPE_NAME; ;}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 257 "c-parse.y"
    { (yyval.type_spec_list) = ALLOC_NODE(struct type_spec_list); (yyval.type_spec_list)->type_spec = (yyvsp[(1) - (2)].type_spec); (yyval.type_spec_list)->next = (yyvsp[(2) - (2)].type_spec_list); ;}
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 258 "c-parse.y"
    { (yyval.type_spec_list) = ALLOC_NODE(struct type_spec_list); (yyval.type_spec_list)->type_spec = (yyvsp[(1) - (1)].type_spec); ;}
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 259 "c-parse.y"
    { (yyval.type_spec_list) = (yyvsp[(2) - (2)].type_spec_list); ;}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 260 "c-parse.y"
    { (yyval.type_spec_list) = ALLOC_NODE(struct type_spec_list); ;}
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 269 "c-parse.y"
    { (yyval.pointer_spec) = ALLOC_NODE(struct pointer_spec); (yyval.pointer_spec)->nr_refs = 1; ;}
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 270 "c-parse.y"
    { (yyval.pointer_spec) = ALLOC_NODE(struct pointer_spec); (yyval.pointer_spec)->nr_refs = 1; ;}
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 271 "c-parse.y"
    { (yyval.pointer_spec)->nr_refs ++; ;}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 272 "c-parse.y"
    { (yyval.pointer_spec)->nr_refs ++; ;}
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 281 "c-parse.y"
    { (yyval.type_name) = ALLOC_NODE(struct type_name); (yyval.type_name)->type_spec = (yyvsp[(1) - (1)].type_spec_list); ;}
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 282 "c-parse.y"
    { (yyval.type_name) = ALLOC_NODE(struct type_name); (yyval.type_name)->type_spec = (yyvsp[(1) - (2)].type_spec_list); (yyval.type_name)->abstract_decl = (yyvsp[(2) - (2)].abstract_decl); ;}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 286 "c-parse.y"
    { (yyval.abstract_decl) = ALLOC_NODE(struct abstract_decl); (yyval.abstract_decl)->pointer_spec = (yyvsp[(1) - (1)].pointer_spec); ;}
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 287 "c-parse.y"
    { (yyval.abstract_decl) = ALLOC_NODE(struct abstract_decl); (yyval.abstract_decl)->direct_abstract_decl = (yyvsp[(1) - (1)].direct_abstract_decl); ;}
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 288 "c-parse.y"
    { (yyval.abstract_decl) = ALLOC_NODE(struct abstract_decl); (yyval.abstract_decl)->pointer_spec = (yyvsp[(1) - (2)].pointer_spec); (yyval.abstract_decl)->direct_abstract_decl = (yyvsp[(2) - (2)].direct_abstract_decl); ;}
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 292 "c-parse.y"
    { (yyval.direct_abstract_decl) = ALLOC_NODE(struct direct_abstract_decl); (yyval.direct_abstract_decl)->kind = DAD_PARENS; (yyval.direct_abstract_decl)->abstract_decl = (yyvsp[(2) - (3)].abstract_decl); ;}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 293 "c-parse.y"
    { (yyval.direct_abstract_decl) = ALLOC_NODE(struct direct_abstract_decl); (yyval.direct_abstract_decl)->kind = DAD_SQBRACKETS; ;}
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 294 "c-parse.y"
    { (yyval.direct_abstract_decl) = ALLOC_NODE(struct direct_abstract_decl); (yyval.direct_abstract_decl)->kind = DAD_SQBRACKETS; ;}
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 295 "c-parse.y"
    { (yyval.direct_abstract_decl) = ALLOC_NODE(struct direct_abstract_decl); (yyval.direct_abstract_decl)->kind = DAD_SQBRACKETS; ;}
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 296 "c-parse.y"
    { (yyval.direct_abstract_decl) = ALLOC_NODE(struct direct_abstract_decl); (yyval.direct_abstract_decl)->kind = DAD_SQBRACKETS; ;}
    break;

  case 108:

/* Line 1464 of yacc.c  */
#line 297 "c-parse.y"
    { (yyval.direct_abstract_decl) = ALLOC_NODE(struct direct_abstract_decl); (yyval.direct_abstract_decl)->kind = DAD_PARENS; ;}
    break;



/* Line 1464 of yacc.c  */
#line 2368 "c-parse.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

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
      yyerror (YY_("syntax error"));
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
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
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



/* Line 1684 of yacc.c  */
#line 300 "c-parse.y"


