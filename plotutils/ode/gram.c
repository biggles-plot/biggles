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
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     IDENT = 259,
     SEP = 260,
     ABS = 261,
     SQRT = 262,
     EXP = 263,
     LOG = 264,
     LOG10 = 265,
     SIN = 266,
     COS = 267,
     TAN = 268,
     ASIN = 269,
     ACOS = 270,
     ATAN = 271,
     SINH = 272,
     COSH = 273,
     TANH = 274,
     ASINH = 275,
     ACOSH = 276,
     ATANH = 277,
     FLOOR = 278,
     CEIL = 279,
     J0 = 280,
     J1 = 281,
     Y0 = 282,
     Y1 = 283,
     LGAMMA = 284,
     GAMMA = 285,
     ERF = 286,
     ERFC = 287,
     INVERF = 288,
     NORM = 289,
     INVNORM = 290,
     IGAMMA = 291,
     IBETA = 292,
     EVERY = 293,
     FROM = 294,
     PRINT = 295,
     STEP = 296,
     EXAM = 297,
     UMINUS = 298
   };
#endif
/* Tokens.  */
#define NUMBER 258
#define IDENT 259
#define SEP 260
#define ABS 261
#define SQRT 262
#define EXP 263
#define LOG 264
#define LOG10 265
#define SIN 266
#define COS 267
#define TAN 268
#define ASIN 269
#define ACOS 270
#define ATAN 271
#define SINH 272
#define COSH 273
#define TANH 274
#define ASINH 275
#define ACOSH 276
#define ATANH 277
#define FLOOR 278
#define CEIL 279
#define J0 280
#define J1 281
#define Y0 282
#define Y1 283
#define LGAMMA 284
#define GAMMA 285
#define ERF 286
#define ERFC 287
#define INVERF 288
#define NORM 289
#define INVNORM 290
#define IGAMMA 291
#define IBETA 292
#define EVERY 293
#define FROM 294
#define PRINT 295
#define STEP 296
#define EXAM 297
#define UMINUS 298




/* Copy the first part of user declarations.  */
#line 1 "gram.y"

/* Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1999, 2005, 2008, Free Software 
 * Foundation, Inc.
 */
/*
 * Grammar for ode:
 * Most things are self-explanatory.
 * When you're done with a lexptr-type object
 * you should free it with lfree.  They are
 * used for passing constants around while parsing
 * (computing the value of) a cexpr.  The macros
 * for evaluating operators and functions are the
 * most important thing to be familiar with before
 * toying with the semantics.
 */
#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * Value is true iff operands pass ONECON.
 */
#define TWOCON(x,y) (ONECON(x) && ONECON(y))
#define THREECON(x,y,z) (ONECON(x) && ONECON(y) && ONECON(z))
/*
 * Value must be a (struct expr *).  Returns true if its arg, when
 * evaluated, would put a constant on the stack.
 */
#define ONECON(x) (x->ex_oper == O_CONST && x->ex_next == NULL)
/*
 * Performs ordinary binary arithmetic, when there are two constants in an
 * expr (`op' is a C operator that includes an assignment, e.g., +=).
 */
#define COMBINE(x,y,r,op) {x->ex_value op y->ex_value; efree(y); r = x;}
/*
 * Generates stack code for a binary operation, as for a dyadic operator in
 * an expression.  E.g., op = O_PLUS.
 */
#define BINARY(x,y,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(y,ep);\
                concat(r=x,y);}
/*
 * Generates stack code for a ternary operation, as for a triadic operator in
 * an expression.  E.g., op = O_IBETA.
 */
#define TERNARY(x,y,z,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(z,ep);\
                concat(y,z);\
                concat(r=x,y);}
/*
 * Performs ordinary unary arithmetic, when there is a constant in an expr.
 * "-" seems to work as a monadic operator.
 */
#define CONFUNC(x,r,f) {x->ex_value = f(x->ex_value); r = x;}
/*
 * Generates stack code for a unary operation, as for a monadic operator in
 * an expression.
 */
#define UNARY(oprnd,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(r=oprnd,ep);}
/*
 * Performs binary arithmetic in a cexpr (`op' is a C operator that
 * includes an assignment, e.g. +=).
 */
#define CEXOP(x,y,r,op) {x->lx_u.lxu_value op y->lx_u.lxu_value;\
                lfree(y);\
                r = x;}
/*
 * Performs unary arithmetic in a cexpr.
 */
#define CEXFUNC(x,r,f) {x->lx_u.lxu_value = f(x->lx_u.lxu_value); r=x;}

/*
 * A hook for future upgrades in error reporting
 */
static  char    *errmess = NULL;

bool erritem;


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

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 85 "gram.y"
{
        struct  lex     *lexptr;
        struct  expr    *exprptr;
        struct  prt     *prtptr;
        int     simple;
}
/* Line 187 of yacc.c.  */
#line 274 "gram.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 287 "gram.c"

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
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
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
#define YYFINAL  52
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   876

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  10
/* YYNRULES -- Number of rules.  */
#define YYNRULES  104
/* YYNRULES -- Number of states.  */
#define YYNSTATES  346

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   298

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,     2,     2,     2,     2,     2,    50,
      55,    56,    46,    44,    51,    45,     2,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    43,     2,    54,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    52,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    49
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    15,    18,    24,    30,
      36,    44,    48,    50,    54,    57,    58,    60,    62,    64,
      66,    67,    70,    71,    74,    78,    82,    86,    90,    94,
      98,   103,   108,   113,   118,   123,   128,   133,   138,   143,
     148,   153,   158,   163,   168,   173,   178,   183,   188,   193,
     198,   203,   208,   213,   218,   223,   228,   233,   238,   243,
     248,   255,   264,   267,   269,   273,   277,   281,   285,   289,
     293,   298,   303,   308,   313,   318,   323,   328,   333,   338,
     343,   348,   353,   358,   363,   368,   373,   378,   383,   388,
     393,   398,   403,   408,   413,   418,   423,   428,   433,   438,
     443,   450,   459,   462,   464
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      58,     0,    -1,    59,    -1,    58,    59,    -1,     5,    -1,
       4,    43,    66,     5,    -1,     1,     5,    -1,     4,    50,
      43,    66,     5,    -1,    40,    60,    63,    64,     5,    -1,
      41,    65,    51,    65,     5,    -1,    41,    65,    51,    65,
      51,    65,     5,    -1,    42,     4,     5,    -1,    61,    -1,
      60,    51,    61,    -1,     4,    62,    -1,    -1,    50,    -1,
      52,    -1,    53,    -1,    54,    -1,    -1,    38,    65,    -1,
      -1,    39,    65,    -1,    55,    65,    56,    -1,    65,    44,
      65,    -1,    65,    45,    65,    -1,    65,    46,    65,    -1,
      65,    47,    65,    -1,    65,    48,    65,    -1,     7,    55,
      65,    56,    -1,     6,    55,    65,    56,    -1,     8,    55,
      65,    56,    -1,     9,    55,    65,    56,    -1,    10,    55,
      65,    56,    -1,    11,    55,    65,    56,    -1,    12,    55,
      65,    56,    -1,    13,    55,    65,    56,    -1,    20,    55,
      65,    56,    -1,    21,    55,    65,    56,    -1,    22,    55,
      65,    56,    -1,    14,    55,    65,    56,    -1,    15,    55,
      65,    56,    -1,    16,    55,    65,    56,    -1,    17,    55,
      65,    56,    -1,    18,    55,    65,    56,    -1,    19,    55,
      65,    56,    -1,    23,    55,    65,    56,    -1,    24,    55,
      65,    56,    -1,    25,    55,    65,    56,    -1,    26,    55,
      65,    56,    -1,    27,    55,    65,    56,    -1,    28,    55,
      65,    56,    -1,    32,    55,    65,    56,    -1,    31,    55,
      65,    56,    -1,    33,    55,    65,    56,    -1,    29,    55,
      65,    56,    -1,    30,    55,    65,    56,    -1,    34,    55,
      65,    56,    -1,    35,    55,    65,    56,    -1,    36,    55,
      65,    51,    65,    56,    -1,    37,    55,    65,    51,    65,
      51,    65,    56,    -1,    45,    65,    -1,     3,    -1,    55,
      66,    56,    -1,    66,    44,    66,    -1,    66,    45,    66,
      -1,    66,    46,    66,    -1,    66,    47,    66,    -1,    66,
      48,    66,    -1,     7,    55,    66,    56,    -1,     6,    55,
      66,    56,    -1,     8,    55,    66,    56,    -1,     9,    55,
      66,    56,    -1,    10,    55,    66,    56,    -1,    11,    55,
      66,    56,    -1,    12,    55,    66,    56,    -1,    13,    55,
      66,    56,    -1,    20,    55,    66,    56,    -1,    21,    55,
      66,    56,    -1,    22,    55,    66,    56,    -1,    14,    55,
      66,    56,    -1,    15,    55,    66,    56,    -1,    16,    55,
      66,    56,    -1,    17,    55,    66,    56,    -1,    18,    55,
      66,    56,    -1,    19,    55,    66,    56,    -1,    23,    55,
      66,    56,    -1,    24,    55,    66,    56,    -1,    25,    55,
      66,    56,    -1,    26,    55,    66,    56,    -1,    27,    55,
      66,    56,    -1,    28,    55,    66,    56,    -1,    29,    55,
      66,    56,    -1,    30,    55,    66,    56,    -1,    32,    55,
      66,    56,    -1,    31,    55,    66,    56,    -1,    33,    55,
      66,    56,    -1,    34,    55,    66,    56,    -1,    35,    55,
      66,    56,    -1,    36,    55,    66,    51,    66,    56,    -1,
      37,    55,    66,    51,    66,    51,    66,    56,    -1,    45,
      66,    -1,     3,    -1,     4,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   110,   110,   111,   114,   116,   127,   139,   168,   175,
     187,   207,   247,   252,   262,   275,   276,   278,   283,   288,
     296,   297,   306,   307,   315,   319,   323,   327,   331,   335,
     342,   346,   352,   356,   360,   364,   368,   372,   376,   380,
     384,   388,   392,   396,   400,   404,   408,   412,   416,   420,
     424,   428,   432,   436,   440,   444,   448,   452,   456,   460,
     464,   471,   479,   483,   487,   489,   496,   503,   510,   523,
     610,   617,   628,   635,   642,   649,   656,   663,   670,   677,
     684,   691,   698,   705,   712,   719,   726,   733,   740,   747,
     754,   761,   768,   775,   782,   789,   796,   803,   810,   817,
     824,   836,   849,   856,   863
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUMBER", "IDENT", "SEP", "ABS", "SQRT",
  "EXP", "LOG", "LOG10", "SIN", "COS", "TAN", "ASIN", "ACOS", "ATAN",
  "SINH", "COSH", "TANH", "ASINH", "ACOSH", "ATANH", "FLOOR", "CEIL", "J0",
  "J1", "Y0", "Y1", "LGAMMA", "GAMMA", "ERF", "ERFC", "INVERF", "NORM",
  "INVNORM", "IGAMMA", "IBETA", "EVERY", "FROM", "PRINT", "STEP", "EXAM",
  "'='", "'+'", "'-'", "'*'", "'/'", "'^'", "UMINUS", "'''", "','", "'~'",
  "'!'", "'?'", "'('", "')'", "$accept", "program", "stat", "prtlist",
  "prtitem", "prttag", "optevery", "optfrom", "cexpr", "expr", 0
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
     295,   296,   297,    61,    43,    45,    42,    47,    94,   298,
      39,    44,   126,    33,    63,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    58,    59,    59,    59,    59,    59,    59,
      59,    59,    60,    60,    61,    62,    62,    62,    62,    62,
      63,    63,    64,    64,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     4,     2,     5,     5,     5,
       7,     3,     1,     3,     2,     0,     1,     1,     1,     1,
       0,     2,     0,     2,     3,     3,     3,     3,     3,     3,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       6,     8,     2,     1,     3,     3,     3,     3,     3,     3,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       6,     8,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     4,     0,     0,     0,     0,     2,     6,
       0,     0,    15,    20,    12,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     3,   103,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    16,    17,    18,    19,    14,     0,     0,    22,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    62,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   102,     0,     5,     0,     0,     0,     0,
       0,     0,    21,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    64,    65,    66,
      67,    68,    69,     7,    23,     8,    31,    30,    32,    33,
      34,    35,    36,    37,    41,    42,    43,    44,    45,    46,
      38,    39,    40,    47,    48,    49,    50,    51,    52,    56,
      57,    54,    53,    55,    58,    59,     0,     0,     9,     0,
      71,    70,    72,    73,    74,    75,    76,    77,    81,    82,
      83,    84,    85,    86,    78,    79,    80,    87,    88,    89,
      90,    91,    92,    93,    94,    96,    95,    97,    98,    99,
       0,     0,     0,     0,     0,     0,     0,    60,     0,    10,
     100,     0,     0,     0,    61,   101
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,    13,    14,    96,    99,   185,    50,    90
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -50
static const yytype_int16 yypact[] =
{
     233,    -2,   -22,   -50,    12,   181,    44,   228,   -50,   -50,
     131,     8,   -16,   -36,   -50,   -50,    29,    30,   114,   115,
     122,   123,   124,   126,   128,   130,   170,   172,   176,   180,
     184,   190,   191,   192,   195,   203,   204,   206,   207,   216,
     217,   234,   236,   238,   251,   254,   255,   256,   181,   181,
     209,   177,   -50,   -50,   -50,   -50,   269,   272,   273,   274,
     287,   290,   291,   292,   305,   308,   309,   310,   323,   326,
     327,   328,   341,   344,   345,   346,   359,   362,   363,   364,
     377,   380,   381,   382,   395,   398,   399,   400,   131,   131,
     127,   131,   -50,   -50,   -50,   -50,   -50,   181,    12,    94,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   -50,   -39,   181,   181,   181,   181,   181,   181,
     -50,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   -50,   -34,   -50,   131,   131,   131,   131,
     131,   175,   828,   -50,   181,   225,   -21,   -15,   196,   232,
     239,   252,   257,   270,   275,   288,   293,   306,   311,   324,
     329,   342,   347,   360,   365,   378,   383,   396,   401,   414,
     419,   432,   437,   450,   455,   468,   780,   788,   -50,   -28,
     -28,   246,   246,   246,    -1,   473,   486,   491,   504,   509,
     522,   527,   540,   545,   558,   563,   576,   581,   594,   599,
     612,   617,   630,   635,   648,   653,   666,   671,   684,   689,
     702,   707,   720,   725,   738,   796,   804,   -50,    78,    78,
     259,   259,   259,   -50,   828,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   181,   181,   -50,   181,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,   -50,
     131,   131,   743,   812,   219,   756,   820,   -50,   181,   -50,
     -50,   131,   761,   774,   -50,   -50
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -50,   -50,   318,   -50,   214,   -50,   -50,   -50,   -48,   -49
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     132,   133,    97,     9,   298,   134,   135,   136,   137,   138,
     176,   177,   178,   179,   180,    98,    12,   218,   136,   137,
     138,    10,   257,   134,   135,   136,   137,   138,    11,   134,
     135,   136,   137,   138,    92,   266,    93,    94,    95,   173,
     174,   267,   181,   134,   135,   136,   137,   138,    51,   182,
     299,    91,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   100,   101,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   178,   179,   180,   258,   259,   260,
     261,   262,   175,   184,    54,    55,   264,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,   102,
     103,   176,   177,   178,   179,   180,    88,   104,   105,   106,
     263,   107,   140,   108,    15,   109,    89,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   176,
     177,   178,   179,   180,   339,   110,    48,   111,    52,     1,
     265,   112,     2,     3,     1,   113,    49,     2,     3,   114,
     134,   135,   136,   137,   138,   115,   116,   117,   332,   333,
     118,   334,   268,   134,   135,   136,   137,   138,   119,   120,
     139,   121,   122,   134,   135,   136,   137,   138,     4,     5,
       6,   123,   124,     4,     5,     6,   134,   135,   136,   137,
     138,   335,   336,   134,   135,   136,   137,   138,   269,   125,
     342,   126,   343,   127,   138,   270,   134,   135,   136,   137,
     138,   134,   135,   136,   137,   138,   128,   180,   271,   129,
     130,   131,   183,   272,   134,   135,   136,   137,   138,   134,
     135,   136,   137,   138,   141,    53,   273,   142,   143,   144,
       0,   274,   134,   135,   136,   137,   138,   134,   135,   136,
     137,   138,   145,     0,   275,   146,   147,   148,     0,   276,
     134,   135,   136,   137,   138,   134,   135,   136,   137,   138,
     149,     0,   277,   150,   151,   152,     0,   278,   134,   135,
     136,   137,   138,   134,   135,   136,   137,   138,   153,     0,
     279,   154,   155,   156,     0,   280,   134,   135,   136,   137,
     138,   134,   135,   136,   137,   138,   157,     0,   281,   158,
     159,   160,     0,   282,   134,   135,   136,   137,   138,   134,
     135,   136,   137,   138,   161,     0,   283,   162,   163,   164,
       0,   284,   134,   135,   136,   137,   138,   134,   135,   136,
     137,   138,   165,     0,   285,   166,   167,   168,     0,   286,
     134,   135,   136,   137,   138,   134,   135,   136,   137,   138,
     169,     0,   287,   170,   171,   172,     0,   288,   134,   135,
     136,   137,   138,   134,   135,   136,   137,   138,     0,     0,
     289,     0,     0,     0,     0,   290,   134,   135,   136,   137,
     138,   134,   135,   136,   137,   138,     0,     0,   291,     0,
       0,     0,     0,   292,   134,   135,   136,   137,   138,   134,
     135,   136,   137,   138,     0,     0,   293,     0,     0,     0,
       0,   294,   134,   135,   136,   137,   138,   176,   177,   178,
     179,   180,     0,     0,   295,     0,     0,     0,     0,   300,
     176,   177,   178,   179,   180,   176,   177,   178,   179,   180,
       0,     0,   301,     0,     0,     0,     0,   302,   176,   177,
     178,   179,   180,   176,   177,   178,   179,   180,     0,     0,
     303,     0,     0,     0,     0,   304,   176,   177,   178,   179,
     180,   176,   177,   178,   179,   180,     0,     0,   305,     0,
       0,     0,     0,   306,   176,   177,   178,   179,   180,   176,
     177,   178,   179,   180,     0,     0,   307,     0,     0,     0,
       0,   308,   176,   177,   178,   179,   180,   176,   177,   178,
     179,   180,     0,     0,   309,     0,     0,     0,     0,   310,
     176,   177,   178,   179,   180,   176,   177,   178,   179,   180,
       0,     0,   311,     0,     0,     0,     0,   312,   176,   177,
     178,   179,   180,   176,   177,   178,   179,   180,     0,     0,
     313,     0,     0,     0,     0,   314,   176,   177,   178,   179,
     180,   176,   177,   178,   179,   180,     0,     0,   315,     0,
       0,     0,     0,   316,   176,   177,   178,   179,   180,   176,
     177,   178,   179,   180,     0,     0,   317,     0,     0,     0,
       0,   318,   176,   177,   178,   179,   180,   176,   177,   178,
     179,   180,     0,     0,   319,     0,     0,     0,     0,   320,
     176,   177,   178,   179,   180,   176,   177,   178,   179,   180,
       0,     0,   321,     0,     0,     0,     0,   322,   176,   177,
     178,   179,   180,   176,   177,   178,   179,   180,     0,     0,
     323,     0,     0,     0,     0,   324,   176,   177,   178,   179,
     180,   176,   177,   178,   179,   180,     0,     0,   325,     0,
       0,     0,     0,   326,   176,   177,   178,   179,   180,   176,
     177,   178,   179,   180,     0,     0,   327,     0,     0,     0,
       0,   328,   176,   177,   178,   179,   180,   134,   135,   136,
     137,   138,     0,     0,   329,     0,     0,     0,     0,   337,
     176,   177,   178,   179,   180,   134,   135,   136,   137,   138,
       0,     0,   340,     0,     0,     0,     0,   344,   176,   177,
     178,   179,   180,     0,   134,   135,   136,   137,   138,     0,
     345,   296,   134,   135,   136,   137,   138,     0,     0,   297,
     176,   177,   178,   179,   180,     0,     0,   330,   176,   177,
     178,   179,   180,     0,     0,   331,   134,   135,   136,   137,
     138,     0,     0,   338,   176,   177,   178,   179,   180,     0,
       0,   341,   134,   135,   136,   137,   138
};

static const yytype_int16 yycheck[] =
{
      48,    49,    38,     5,     5,    44,    45,    46,    47,    48,
      44,    45,    46,    47,    48,    51,     4,    56,    46,    47,
      48,    43,    56,    44,    45,    46,    47,    48,    50,    44,
      45,    46,    47,    48,    50,    56,    52,    53,    54,    88,
      89,    56,    91,    44,    45,    46,    47,    48,     4,    97,
      51,    43,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,    55,    55,   134,   135,   136,   137,
     138,   139,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,    46,    47,    48,   176,   177,   178,
     179,   180,     5,    39,     3,     4,   184,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    55,
      55,    44,    45,    46,    47,    48,    45,    55,    55,    55,
       5,    55,     5,    55,     3,    55,    55,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    44,
      45,    46,    47,    48,     5,    55,    45,    55,     0,     1,
       5,    55,     4,     5,     1,    55,    55,     4,     5,    55,
      44,    45,    46,    47,    48,    55,    55,    55,   296,   297,
      55,   299,    56,    44,    45,    46,    47,    48,    55,    55,
      51,    55,    55,    44,    45,    46,    47,    48,    40,    41,
      42,    55,    55,    40,    41,    42,    44,    45,    46,    47,
      48,   330,   331,    44,    45,    46,    47,    48,    56,    55,
     338,    55,   341,    55,    48,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    55,    48,    56,    55,
      55,    55,    98,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    55,     7,    56,    55,    55,    55,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    55,    -1,    56,    55,    55,    55,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      55,    -1,    56,    55,    55,    55,    -1,    56,    44,    45,
      46,    47,    48,    44,    45,    46,    47,    48,    55,    -1,
      56,    55,    55,    55,    -1,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    55,    -1,    56,    55,
      55,    55,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    55,    -1,    56,    55,    55,    55,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    55,    -1,    56,    55,    55,    55,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      55,    -1,    56,    55,    55,    55,    -1,    56,    44,    45,
      46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
      46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
      46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
      46,    47,    48,    44,    45,    46,    47,    48,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    56,    44,    45,    46,    47,
      48,    44,    45,    46,    47,    48,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    56,    44,    45,    46,    47,    48,    44,
      45,    46,    47,    48,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    56,    44,    45,    46,    47,    48,    44,    45,    46,
      47,    48,    -1,    -1,    56,    -1,    -1,    -1,    -1,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    56,    44,    45,
      46,    47,    48,    -1,    44,    45,    46,    47,    48,    -1,
      56,    51,    44,    45,    46,    47,    48,    -1,    -1,    51,
      44,    45,    46,    47,    48,    -1,    -1,    51,    44,    45,
      46,    47,    48,    -1,    -1,    51,    44,    45,    46,    47,
      48,    -1,    -1,    51,    44,    45,    46,    47,    48,    -1,
      -1,    51,    44,    45,    46,    47,    48
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     5,    40,    41,    42,    58,    59,     5,
      43,    50,     4,    60,    61,     3,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    45,    55,
      65,     4,     0,    59,     3,     4,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    45,    55,
      66,    43,    50,    52,    53,    54,    62,    38,    51,    63,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    65,    65,    44,    45,    46,    47,    48,    51,
       5,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    66,    66,     5,    44,    45,    46,    47,
      48,    66,    65,    61,    39,    64,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    56,    65,
      65,    65,    65,    65,    65,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    56,    66,    66,
      66,    66,    66,     5,    65,     5,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    51,    51,     5,    51,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      51,    51,    65,    65,    65,    66,    66,    56,    51,     5,
      56,    51,    65,    66,    56,    56
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
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
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



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



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
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
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



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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
        case 4:
#line 115 "gram.y"
    { lfree((yyvsp[(1) - (1)].lexptr)); }
    break;

  case 5:
#line 117 "gram.y"
    {
			  struct sym *sp;
			  
			  sp = lookup((yyvsp[(1) - (4)].lexptr)->lx_u.lxu_name);
			  sp->sy_value = eval((yyvsp[(3) - (4)].exprptr));
			  sp->sy_flags |= SF_INIT;
			  lfree((yyvsp[(1) - (4)].lexptr));
			  efree((yyvsp[(3) - (4)].exprptr));
			  lfree((yyvsp[(4) - (4)].lexptr));
                        }
    break;

  case 6:
#line 128 "gram.y"
    {
			  if (errmess == NULL)
			    errmess = "syntax error";
			  fprintf (stderr, "%s:%s:%d: %s\n", 
				   progname, filename,
				   ((yyvsp[(2) - (2)].lexptr)->lx_lino), errmess);
			  errmess = NULL;
			  lfree((yyvsp[(2) - (2)].lexptr));
			  yyerrok;
			  yyclearin;
                        }
    break;

  case 7:
#line 140 "gram.y"
    {
			  struct sym *sp;
			  struct prt *pp, *qp;
			  
			  sp = lookup((yyvsp[(1) - (5)].lexptr)->lx_u.lxu_name);
			  efree(sp->sy_expr);
			  sp->sy_expr = (yyvsp[(4) - (5)].exprptr);
			  sp->sy_flags |= SF_ISEQN;
			  if (!sawprint) 
			    {
			      for (pp=pqueue; pp!=NULL; pp=pp->pr_link)
				if (pp->pr_sym == sp)
				  goto found;
			      pp = palloc();
			      pp->pr_sym = sp;
			      if (pqueue == NULL)
				pqueue = pp;
			      else 
				{
				  for (qp=pqueue; qp->pr_link!=NULL; )
				    qp = qp->pr_link;
				  qp->pr_link = pp;
                                }
			    }
			found:
			  lfree((yyvsp[(1) - (5)].lexptr));
			  lfree((yyvsp[(5) - (5)].lexptr));
                        }
    break;

  case 8:
#line 169 "gram.y"
    {
			  sawprint = true;
			  prerr = erritem;
			  erritem = false;
			  lfree((yyvsp[(5) - (5)].lexptr));
                        }
    break;

  case 9:
#line 176 "gram.y"
    {
			  lfree((yyvsp[(5) - (5)].lexptr));
			  tstart = (yyvsp[(2) - (5)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(2) - (5)].lexptr));
			  tstop = (yyvsp[(4) - (5)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(4) - (5)].lexptr));
			  if (!conflag)
			    startstep();
			  solve();
			  sawstep = true;
                        }
    break;

  case 10:
#line 188 "gram.y"
    {
			  double savstep;
			  bool savconflag;
			  
			  lfree((yyvsp[(7) - (7)].lexptr));
			  tstart = (yyvsp[(2) - (7)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(2) - (7)].lexptr));
			  tstop = (yyvsp[(4) - (7)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(4) - (7)].lexptr));
			  savstep = tstep;
			  tstep = (yyvsp[(6) - (7)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(6) - (7)].lexptr));
			  savconflag = conflag;
			  conflag = true;
			  solve();
			  tstep = savstep;
			  conflag = savconflag;
			  sawstep = true;
                        }
    break;

  case 11:
#line 208 "gram.y"
    {
			  struct sym *sp;
			  
			  lfree((yyvsp[(3) - (3)].lexptr));
			  sp = lookup((yyvsp[(2) - (3)].lexptr)->lx_u.lxu_name);
			  lfree((yyvsp[(2) - (3)].lexptr));
			  printf ("\"%.*s\" is ",NAMMAX,sp->sy_name);
			  switch (sp->sy_flags & SF_DEPV)
			    {
			    case SF_DEPV:
			    case SF_ISEQN:
			      printf ("a dynamic variable\n");
			      break;
			    case SF_INIT:
			      printf ("an initialized constant\n");
			      break;
			    case 0:
			      printf ("an uninitialized constant\n");
			      break;
			    default:
			      panicn ("impossible (%d) in EXAM action",
				      sp->sy_flags);
			    }
			  printf ("value:");
			  prval (sp->sy_value);
			  printf ("\nprime:");
			  prval (sp->sy_prime);
			  printf ("\nsserr:");
			  prval (sp->sy_sserr);
			  printf ("\naberr:");
			  prval (sp->sy_aberr);
			  printf ("\nacerr:");
			  prval (sp->sy_acerr);
			  putchar ('\n');
			  prexq(sp->sy_expr);
			  fflush(stdout);
                        }
    break;

  case 12:
#line 248 "gram.y"
    {
			  pfree(pqueue);
			  pqueue = (yyvsp[(1) - (1)].prtptr);
                        }
    break;

  case 13:
#line 253 "gram.y"
    {
			  struct prt *pp;
			  
			  for (pp=pqueue; pp->pr_link!=NULL; pp=pp->pr_link)
			    ;
			  pp->pr_link = (yyvsp[(3) - (3)].prtptr);
                        }
    break;

  case 14:
#line 263 "gram.y"
    {
			  struct prt *pp;
			  
			  pp = palloc();
			  pp->pr_sym = lookup((yyvsp[(1) - (2)].lexptr)->lx_u.lxu_name);
			  pp->pr_which = (ent_cell)((yyvsp[(2) - (2)].simple));
			  lfree((yyvsp[(1) - (2)].lexptr));
			  (yyval.prtptr) = pp;
                        }
    break;

  case 15:
#line 275 "gram.y"
    { (yyval.simple) = P_VALUE; }
    break;

  case 16:
#line 277 "gram.y"
    { (yyval.simple) = P_PRIME; }
    break;

  case 17:
#line 279 "gram.y"
    {
			  (yyval.simple) = P_ACERR;
			  erritem = true;
                        }
    break;

  case 18:
#line 284 "gram.y"
    {
			  (yyval.simple) = P_ABERR;
			  erritem = true;
                        }
    break;

  case 19:
#line 289 "gram.y"
    {
			  (yyval.simple) = P_SSERR;
			  erritem = true;
                        }
    break;

  case 20:
#line 296 "gram.y"
    { sawevery = false; }
    break;

  case 21:
#line 298 "gram.y"
    {
                        sawevery = true;
                        tevery = IROUND((yyvsp[(2) - (2)].lexptr)->lx_u.lxu_value);
                        lfree((yyvsp[(2) - (2)].lexptr));
                        }
    break;

  case 22:
#line 306 "gram.y"
    { sawfrom = false; }
    break;

  case 23:
#line 308 "gram.y"
    {
			  sawfrom = true;
			  tfrom = (yyvsp[(2) - (2)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(2) - (2)].lexptr));
                        }
    break;

  case 24:
#line 316 "gram.y"
    {
			  (yyval.lexptr) = (yyvsp[(2) - (3)].lexptr);
                        }
    break;

  case 25:
#line 320 "gram.y"
    {
			  CEXOP((yyvsp[(1) - (3)].lexptr),(yyvsp[(3) - (3)].lexptr),(yyval.lexptr),+=)
                        }
    break;

  case 26:
#line 324 "gram.y"
    {
			  CEXOP((yyvsp[(1) - (3)].lexptr),(yyvsp[(3) - (3)].lexptr),(yyval.lexptr),-=)
                        }
    break;

  case 27:
#line 328 "gram.y"
    {
			  CEXOP((yyvsp[(1) - (3)].lexptr),(yyvsp[(3) - (3)].lexptr),(yyval.lexptr),*=)
                        }
    break;

  case 28:
#line 332 "gram.y"
    {
			  CEXOP((yyvsp[(1) - (3)].lexptr),(yyvsp[(3) - (3)].lexptr),(yyval.lexptr),/=)
                        }
    break;

  case 29:
#line 336 "gram.y"
    {
			  (yyvsp[(1) - (3)].lexptr)->lx_u.lxu_value =
			    pow((yyvsp[(1) - (3)].lexptr)->lx_u.lxu_value,(yyvsp[(3) - (3)].lexptr)->lx_u.lxu_value);
			  lfree((yyvsp[(3) - (3)].lexptr));
			  (yyval.lexptr) = (yyvsp[(1) - (3)].lexptr);
                        }
    break;

  case 30:
#line 343 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),sqrt)
                        }
    break;

  case 31:
#line 347 "gram.y"
    {
			  if ((yyvsp[(3) - (4)].lexptr)->lx_u.lxu_value < 0)
			    (yyvsp[(3) - (4)].lexptr)->lx_u.lxu_value = -((yyvsp[(3) - (4)].lexptr)->lx_u.lxu_value);
			  (yyval.lexptr) = (yyvsp[(3) - (4)].lexptr);
                        }
    break;

  case 32:
#line 353 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),exp)
                        }
    break;

  case 33:
#line 357 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),log)
                        }
    break;

  case 34:
#line 361 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),log10)
                        }
    break;

  case 35:
#line 365 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),sin)
                        }
    break;

  case 36:
#line 369 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),cos)
                        }
    break;

  case 37:
#line 373 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),tan)
                        }
    break;

  case 38:
#line 377 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),asinh)
                        }
    break;

  case 39:
#line 381 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),acosh)
                        }
    break;

  case 40:
#line 385 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),atanh)
                        }
    break;

  case 41:
#line 389 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),asin)
                        }
    break;

  case 42:
#line 393 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),acos)
                        }
    break;

  case 43:
#line 397 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),atan)
                        }
    break;

  case 44:
#line 401 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),sinh)
                        }
    break;

  case 45:
#line 405 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),cosh)
                        }
    break;

  case 46:
#line 409 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),tanh)
                        }
    break;

  case 47:
#line 413 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),floor)
                        }
    break;

  case 48:
#line 417 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),ceil)
                        }
    break;

  case 49:
#line 421 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),j0)
                        }
    break;

  case 50:
#line 425 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),j1)
                        }
    break;

  case 51:
#line 429 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),y0)
                        }
    break;

  case 52:
#line 433 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),y1)
                        }
    break;

  case 53:
#line 437 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),erfc)
                        }
    break;

  case 54:
#line 441 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),erf)
                        }
    break;

  case 55:
#line 445 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),inverf)
                        }
    break;

  case 56:
#line 449 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),F_LGAMMA)
                        }
    break;

  case 57:
#line 453 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),f_gamma)
                        }
    break;

  case 58:
#line 457 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),norm)
                        }
    break;

  case 59:
#line 461 "gram.y"
    {
			  CEXFUNC((yyvsp[(3) - (4)].lexptr),(yyval.lexptr),invnorm)
                        }
    break;

  case 60:
#line 465 "gram.y"
    {
			  (yyvsp[(3) - (6)].lexptr)->lx_u.lxu_value =
			    igamma((yyvsp[(3) - (6)].lexptr)->lx_u.lxu_value,(yyvsp[(5) - (6)].lexptr)->lx_u.lxu_value);
			  lfree((yyvsp[(5) - (6)].lexptr));
			  (yyval.lexptr) = (yyvsp[(3) - (6)].lexptr);
                        }
    break;

  case 61:
#line 472 "gram.y"
    {
			  (yyvsp[(3) - (8)].lexptr)->lx_u.lxu_value =
			    ibeta((yyvsp[(3) - (8)].lexptr)->lx_u.lxu_value,(yyvsp[(5) - (8)].lexptr)->lx_u.lxu_value,(yyvsp[(7) - (8)].lexptr)->lx_u.lxu_value);
			  lfree((yyvsp[(5) - (8)].lexptr));
			  lfree((yyvsp[(7) - (8)].lexptr));
			  (yyval.lexptr) = (yyvsp[(3) - (8)].lexptr);
                        }
    break;

  case 62:
#line 480 "gram.y"
    {
			  CEXFUNC((yyvsp[(2) - (2)].lexptr),(yyval.lexptr),-)
                        }
    break;

  case 63:
#line 484 "gram.y"
    { (yyval.lexptr) = (yyvsp[(1) - (1)].lexptr); }
    break;

  case 64:
#line 488 "gram.y"
    { (yyval.exprptr) = (yyvsp[(2) - (3)].exprptr); }
    break;

  case 65:
#line 490 "gram.y"
    {
			  if (TWOCON((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr)))
			    COMBINE((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),+=)
			  else
			    BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_PLUS);
                        }
    break;

  case 66:
#line 497 "gram.y"
    {
			  if (TWOCON((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr)))
			    COMBINE((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),-=)
			  else
			    BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_MINUS);
                        }
    break;

  case 67:
#line 504 "gram.y"
    {
			  if (TWOCON((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr)))
			    COMBINE((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),*=)
			  else
			    BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_MULT);
                        }
    break;

  case 68:
#line 511 "gram.y"
    {
			  if (TWOCON((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr)))
			    COMBINE((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),/=)
			  else if (ONECON((yyvsp[(3) - (3)].exprptr)) && (yyvsp[(3) - (3)].exprptr)->ex_value!=0.) 
			    {
			      /* division by constant */
			      (yyvsp[(3) - (3)].exprptr)->ex_value = 1./(yyvsp[(3) - (3)].exprptr)->ex_value;
			      BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_MULT);
			    } 
			  else
			    BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_DIV);
                        }
    break;

  case 69:
#line 524 "gram.y"
    {
			  double f;
			  bool invert = false;
			  
			  if (TWOCON((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr))) 
			    {
			      /* case const ^ const */
			      (yyvsp[(1) - (3)].exprptr)->ex_value = pow((yyvsp[(1) - (3)].exprptr)->ex_value,(yyvsp[(3) - (3)].exprptr)->ex_value);
			      efree((yyvsp[(3) - (3)].exprptr));
			    } 
			  else if (ONECON((yyvsp[(1) - (3)].exprptr))) 
			    {
			      if ((yyvsp[(1) - (3)].exprptr)->ex_value == 1.)
				{
				  /* case 1 ^ x */
				  efree((yyvsp[(3) - (3)].exprptr));
				  (yyval.exprptr) = (yyvsp[(1) - (3)].exprptr);
                                }
			      else
				goto other;
			    }
			  else if (!ONECON((yyvsp[(3) - (3)].exprptr)))
			    goto other;
			  else 
			    {
			      f = (yyvsp[(3) - (3)].exprptr)->ex_value;
			      if (f < 0.) 
				{
				  /*
				   * negative exponent means
				   * to append an invert cmd
				   */
				  f = -f;
				  invert = true;
                                }
			      if (f == 2.) 
				{
				  /* case x ^ 2 */
				  (yyvsp[(3) - (3)].exprptr)->ex_oper = O_SQAR;
				  concat((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr));
				  (yyval.exprptr) = (yyvsp[(1) - (3)].exprptr);
                                }
			      else if (f == 3.) 
				{
				  /* case x ^ 3 */
				  (yyvsp[(3) - (3)].exprptr)->ex_oper = O_CUBE;
				  concat((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr));
				  (yyval.exprptr) = (yyvsp[(1) - (3)].exprptr);
                                }
			      else if (f == 0.5) 
				{
				  /* case x ^ .5 */
				  (yyvsp[(3) - (3)].exprptr)->ex_oper = O_SQRT;
				  concat((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr));
				  (yyval.exprptr) = (yyvsp[(1) - (3)].exprptr);
                                }
			      else if (f == 1.5) 
				{
				  /* case x ^ 1.5 */
				  (yyvsp[(3) - (3)].exprptr)->ex_oper = O_CUBE;
				  BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_SQRT);
                                }
			      else if (f == 1.) 
				{
				  /* case x ^ 1 */
				  efree((yyvsp[(3) - (3)].exprptr));
				  (yyval.exprptr) = (yyvsp[(1) - (3)].exprptr);
                                }
			      else if (f == 0.) 
				{
				  /* case x ^ 0 */
				  efree((yyvsp[(1) - (3)].exprptr));
				  (yyvsp[(3) - (3)].exprptr)->ex_value = 1.;
				  (yyval.exprptr) = (yyvsp[(3) - (3)].exprptr);
                                } 
			      else 
				{
				other:
				  /* default */
				  invert = false;
				  BINARY((yyvsp[(1) - (3)].exprptr),(yyvsp[(3) - (3)].exprptr),(yyval.exprptr),O_POWER);
                                }
			      if (invert)
				UNARY((yyval.exprptr),(yyval.exprptr),O_INV)
			    }
                        }
    break;

  case 70:
#line 611 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),sqrt)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_SQRT);
                        }
    break;

  case 71:
#line 618 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr))) 
			    {
			      if ((yyvsp[(3) - (4)].exprptr)->ex_value < 0)
				(yyvsp[(3) - (4)].exprptr)->ex_value = -((yyvsp[(3) - (4)].exprptr)->ex_value);
			      (yyval.exprptr) = (yyvsp[(3) - (4)].exprptr);
			  } 
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ABS);
                        }
    break;

  case 72:
#line 629 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),exp)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_EXP);
                        }
    break;

  case 73:
#line 636 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),log)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_LOG);
                        }
    break;

  case 74:
#line 643 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),log10)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_LOG10);
                        }
    break;

  case 75:
#line 650 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),sin)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_SIN);
                        }
    break;

  case 76:
#line 657 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),cos)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_COS);
                        }
    break;

  case 77:
#line 664 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),tan)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_TAN);
                        }
    break;

  case 78:
#line 671 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),asinh)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ASINH);
                        }
    break;

  case 79:
#line 678 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),acosh)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ACOSH);
                        }
    break;

  case 80:
#line 685 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),atanh)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ATANH);
                        }
    break;

  case 81:
#line 692 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),asin)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ASIN);
                        }
    break;

  case 82:
#line 699 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),acos)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ACOS);
                        }
    break;

  case 83:
#line 706 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),atan)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ATAN);
                        }
    break;

  case 84:
#line 713 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),sinh)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_SINH);
                        }
    break;

  case 85:
#line 720 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),cosh)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_COSH);
                        }
    break;

  case 86:
#line 727 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),tanh)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_TANH);
                        }
    break;

  case 87:
#line 734 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),floor)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_FLOOR);
                        }
    break;

  case 88:
#line 741 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),ceil)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_CEIL);
                        }
    break;

  case 89:
#line 748 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),j0)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_J0);
                        }
    break;

  case 90:
#line 755 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),j1)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_J1);
                        }
    break;

  case 91:
#line 762 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),y0)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_Y0);
                        }
    break;

  case 92:
#line 769 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),y1)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_Y1);
                        }
    break;

  case 93:
#line 776 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),F_LGAMMA)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_LGAMMA);
                        }
    break;

  case 94:
#line 783 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),f_gamma)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_GAMMA);
                        }
    break;

  case 95:
#line 790 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),erfc)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ERFC);
                        }
    break;

  case 96:
#line 797 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),erf)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_ERF);
                        }
    break;

  case 97:
#line 804 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),inverf)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_INVERF);
                        }
    break;

  case 98:
#line 811 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),norm)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_NORM);
                        }
    break;

  case 99:
#line 818 "gram.y"
    {
			  if (ONECON((yyvsp[(3) - (4)].exprptr)))
			    CONFUNC((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),invnorm)
			  else
			    UNARY((yyvsp[(3) - (4)].exprptr),(yyval.exprptr),O_INVNORM);
                        }
    break;

  case 100:
#line 825 "gram.y"
    {
			  if (TWOCON((yyvsp[(3) - (6)].exprptr),(yyvsp[(5) - (6)].exprptr))) 
			    {
			      (yyvsp[(3) - (6)].exprptr)->ex_value = 
				igamma((yyvsp[(3) - (6)].exprptr)->ex_value,(yyvsp[(5) - (6)].exprptr)->ex_value);
			      efree((yyvsp[(5) - (6)].exprptr));
			      (yyval.exprptr) = (yyvsp[(3) - (6)].exprptr);
			    }
			  else 
			    BINARY((yyvsp[(3) - (6)].exprptr),(yyvsp[(5) - (6)].exprptr),(yyval.exprptr),O_IGAMMA);
		        }
    break;

  case 101:
#line 837 "gram.y"
    {
			  if (THREECON((yyvsp[(3) - (8)].exprptr),(yyvsp[(5) - (8)].exprptr),(yyvsp[(7) - (8)].exprptr))) 
			    {
			      (yyvsp[(3) - (8)].exprptr)->ex_value = 
				ibeta((yyvsp[(3) - (8)].exprptr)->ex_value,(yyvsp[(5) - (8)].exprptr)->ex_value,(yyvsp[(7) - (8)].exprptr)->ex_value);
			      efree((yyvsp[(5) - (8)].exprptr));
			      efree((yyvsp[(7) - (8)].exprptr));
			      (yyval.exprptr) = (yyvsp[(3) - (8)].exprptr);
			    }
			  else 
			    TERNARY((yyvsp[(3) - (8)].exprptr),(yyvsp[(5) - (8)].exprptr),(yyvsp[(7) - (8)].exprptr),(yyval.exprptr),O_IBETA);
		        }
    break;

  case 102:
#line 850 "gram.y"
    {
			  if (ONECON((yyvsp[(2) - (2)].exprptr)))
			    CONFUNC((yyvsp[(2) - (2)].exprptr),(yyval.exprptr),-)
			  else
			    UNARY((yyvsp[(2) - (2)].exprptr),(yyval.exprptr),O_NEG);
                        }
    break;

  case 103:
#line 857 "gram.y"
    {
			  (yyval.exprptr) = ealloc();
			  (yyval.exprptr)->ex_oper = O_CONST;
			  (yyval.exprptr)->ex_value = (yyvsp[(1) - (1)].lexptr)->lx_u.lxu_value;
			  lfree((yyvsp[(1) - (1)].lexptr));
                        }
    break;

  case 104:
#line 864 "gram.y"
    {
			  (yyval.exprptr) = ealloc();
			  (yyval.exprptr)->ex_oper = O_IDENT;
			  (yyval.exprptr)->ex_sym = lookup((yyvsp[(1) - (1)].lexptr)->lx_u.lxu_name);
			  lfree((yyvsp[(1) - (1)].lexptr));
                        }
    break;


/* Line 1267 of yacc.c.  */
#line 2888 "gram.c"
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
		      yytoken, &yylval);
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

  if (yyn == YYFINAL)
    YYACCEPT;

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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
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


#line 871 "gram.y"


int
yyerror (const char *s)
{
  return 0;
}

/*
 * tack two queues of stack code together
 * e1 is connected on the tail of e0
 * There is no good way to test for circular
 * lists, hence the silly count.
 */
void
concat (struct expr *e0, struct expr *e1)
{
  int count;
  
  if (e0 == NULL || e1 == NULL) 
    panic ("NULL expression queue");

  for (count = 0; e0->ex_next != NULL; e0 = e0->ex_next)
    if (++count > 10000) 
      panic ("circular expression queue");

  e0->ex_next = e1;
}

/*
 * print an expression queue
 * called when EXAMINE is invoked on a variable (see above)
 */
void
prexq (const struct expr *ep)
{
  const char *s;
  
  printf (" code:");
  if (ep == NULL)
    putchar ('\n');

  for (; ep != NULL; ep = ep->ex_next) 
    {
      switch (ep->ex_oper) 
	{
	case O_PLUS: s = "add"; break;
	case O_MINUS: s = "subtract"; break;
	case O_MULT: s = "multiply"; break;
	case O_DIV: s = "divide"; break;
	case O_POWER: s = "power"; break;
	case O_SQRT: s = "sqrt"; break;
	case O_EXP: s = "exp"; break;
	case O_LOG: s = "log"; break;
	case O_LOG10: s = "log10"; break;
	case O_SIN: s = "sin"; break;
	case O_COS: s = "cos"; break;
	case O_TAN: s = "cos"; break;
	case O_ASIN: s = "sin"; break;
	case O_ACOS: s = "cos"; break;
	case O_ATAN: s = "cos"; break;
	case O_NEG: s = "negate"; break;
	case O_ABS: s = "abs"; break;
	case O_SINH: s = "sinh"; break;
	case O_COSH: s = "cosh"; break;
	case O_TANH: s = "tanh"; break;
	case O_ASINH: s = "asinh"; break;
	case O_ACOSH: s = "acosh"; break;
	case O_ATANH: s = "atanh"; break;
	case O_SQAR: s = "square"; break;
	case O_CUBE: s = "cube"; break;
	case O_INV: s = "invert"; break;
	case O_FLOOR: s = "floor"; break;
	case O_CEIL: s = "ceil"; break;
	case O_J0: s = "besj0"; break;
	case O_J1: s = "besj1"; break;
	case O_Y0: s = "besy0"; break;
	case O_Y1: s = "besy1"; break;
	case O_ERF: s = "erf"; break;
	case O_ERFC: s = "erfc"; break;
	case O_INVERF: s = "inverf"; break;
	case O_LGAMMA: s = "lgamma"; break;
	case O_GAMMA: s = "gamma"; break;
	case O_NORM: s = "norm"; break;
	case O_INVNORM: s = "invnorm"; break;
	case O_IGAMMA: s = "igamma"; break;
	case O_IBETA: s = "ibeta"; break;
	case O_CONST:
	  printf ("\tpush ");
	  prval (ep->ex_value);
	  putchar ('\n');
	  s = NULL;
	  break;
	case O_IDENT:
	  printf ("\tpush \"%.*s\"\n",
		 NAMMAX, ep->ex_sym->sy_name);
	  s = NULL;
	  break;
	default: s = "unknown!";
	}

      if (s != NULL)
	printf ("\t%s\n",s);
    }
}

