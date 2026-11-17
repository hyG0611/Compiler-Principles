/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "lrparser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ast.h"
#include "lrparser.tab.h"

int yylex(void);
void yyerror(const char *s);

past ast_root = NULL;
static size_t pending_min_literals;


/* 复制字符串，避免依赖非标准的 strdup */
static char *copy_text(const char *s)
{
    size_t len;
    char *p;

    if (s == NULL)
        return NULL;

    len = strlen(s);
    p = (char *)malloc(len + 1);

    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(p, s, len + 1);
    return p;
}

/* 构造二元运算节点 */
static past make_binary(int op, past left, past right)
{
    past node = newAstNode(BINARY_OPERATOR);
    node->ivalue = op;
    node->left = left;
    node->right = right;
    return node;
}

/* 构造一元运算节点 */
static past make_unary(int op, past child)
{
    if (op == Y_SUB) {
        past literal = child;
        while (literal->nodeType == PAREN_EXPR) literal = literal->left;
        if (literal->nodeType == INT_MIN_MAGNITUDE) {
            literal->nodeType = INTEGER_LITERAL;
            literal->ivalue = INT_MIN;
            --pending_min_literals;
            return child;
        }
    }
    past node = newAstNode(UNARY_OPERATOR);
    node->ivalue = op;
    node->left = child;
    return node;
}

/* 构造变量声明节点 */
static past make_var_decl(char *name, past dims, past init)
{
    past var = newAstNode(VAR_DECL);
    var->svalue = copy_text(name);
    /* Array dimensions belong to the declared type, not to the AST children
       shown for a VarDecl. Keep the initializer as its first AST child. */
    var->left = init;
    var->right = dims;
    return var;
}

/* 构造声明语句节点 */
static past make_decl_stmt(past declarations, int is_const)
{
    past node = newAstNode(DECL_STMT);
    node->ivalue = is_const;
    node->left = declarations;
    return node;
}

/* 构造函数节点 */
static past make_func(char *name, past params, past body)
{
    past node = newAstNode(FUNCTION_DECL);
    node->svalue = copy_text(name);
    node->left = params;
    node->right = body;
    return node;
}

/* 构造函数参数节点 */
static past make_param(char *name, past dimensions)
{
    past node = newAstNode(PARM_DECL);
    node->svalue = copy_text(name);
    if (dimensions) dimensions->right = NULL;
    node->right = dimensions;
    return node;
}

/* 构造初始化列表 */
static past make_init_list(past values)
{
    past node = newAstNode(INIT_LIST_EXPR);
    node->left = values;
    return node;
}

/* 构造复合语句 */
static past make_block(past items)
{
    past node = newAstNode(COMPOUND_STMT);
    node->left = items;
    return node;
}

/* 构造函数调用 */
static past make_call(char *name, past args)
{
    past id = newID(name);
    past node = newAstNode(CALL_EXPR);
    node->left = id;
    node->right = args;
    return node;
}

#line 204 "lrparser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lrparser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_Y_INT = 3,                      /* Y_INT  */
  YYSYMBOL_Y_FLOAT = 4,                    /* Y_FLOAT  */
  YYSYMBOL_Y_VOID = 5,                     /* Y_VOID  */
  YYSYMBOL_Y_CONST = 6,                    /* Y_CONST  */
  YYSYMBOL_Y_IF = 7,                       /* Y_IF  */
  YYSYMBOL_Y_ELSE = 8,                     /* Y_ELSE  */
  YYSYMBOL_Y_WHILE = 9,                    /* Y_WHILE  */
  YYSYMBOL_Y_BREAK = 10,                   /* Y_BREAK  */
  YYSYMBOL_Y_CONTINUE = 11,                /* Y_CONTINUE  */
  YYSYMBOL_Y_RETURN = 12,                  /* Y_RETURN  */
  YYSYMBOL_Y_ID = 13,                      /* Y_ID  */
  YYSYMBOL_num_INT = 14,                   /* num_INT  */
  YYSYMBOL_num_FLOAT = 15,                 /* num_FLOAT  */
  YYSYMBOL_Y_ADD = 16,                     /* Y_ADD  */
  YYSYMBOL_Y_SUB = 17,                     /* Y_SUB  */
  YYSYMBOL_Y_MUL = 18,                     /* Y_MUL  */
  YYSYMBOL_Y_DIV = 19,                     /* Y_DIV  */
  YYSYMBOL_Y_MODULO = 20,                  /* Y_MODULO  */
  YYSYMBOL_Y_LESS = 21,                    /* Y_LESS  */
  YYSYMBOL_Y_LESSEQ = 22,                  /* Y_LESSEQ  */
  YYSYMBOL_Y_GREAT = 23,                   /* Y_GREAT  */
  YYSYMBOL_Y_GREATEQ = 24,                 /* Y_GREATEQ  */
  YYSYMBOL_Y_NOTEQ = 25,                   /* Y_NOTEQ  */
  YYSYMBOL_Y_EQ = 26,                      /* Y_EQ  */
  YYSYMBOL_Y_NOT = 27,                     /* Y_NOT  */
  YYSYMBOL_Y_AND = 28,                     /* Y_AND  */
  YYSYMBOL_Y_OR = 29,                      /* Y_OR  */
  YYSYMBOL_Y_ASSIGN = 30,                  /* Y_ASSIGN  */
  YYSYMBOL_Y_LPAR = 31,                    /* Y_LPAR  */
  YYSYMBOL_Y_RPAR = 32,                    /* Y_RPAR  */
  YYSYMBOL_Y_LBRACKET = 33,                /* Y_LBRACKET  */
  YYSYMBOL_Y_RBRACKET = 34,                /* Y_RBRACKET  */
  YYSYMBOL_Y_LSQUARE = 35,                 /* Y_LSQUARE  */
  YYSYMBOL_Y_RSQUARE = 36,                 /* Y_RSQUARE  */
  YYSYMBOL_Y_COMMA = 37,                   /* Y_COMMA  */
  YYSYMBOL_Y_SEMICOLON = 38,               /* Y_SEMICOLON  */
  YYSYMBOL_LOWER_THAN_ELSE = 39,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_num_INT_MIN_MAG = 40,           /* num_INT_MIN_MAG  */
  YYSYMBOL_YYACCEPT = 41,                  /* $accept  */
  YYSYMBOL_TranslationUnit = 42,           /* TranslationUnit  */
  YYSYMBOL_CompUnits = 43,                 /* CompUnits  */
  YYSYMBOL_CompUnit = 44,                  /* CompUnit  */
  YYSYMBOL_Decl = 45,                      /* Decl  */
  YYSYMBOL_ConstDecl = 46,                 /* ConstDecl  */
  YYSYMBOL_ConstDefs = 47,                 /* ConstDefs  */
  YYSYMBOL_ConstDef = 48,                  /* ConstDef  */
  YYSYMBOL_ConstExps = 49,                 /* ConstExps  */
  YYSYMBOL_ConstInitVal = 50,              /* ConstInitVal  */
  YYSYMBOL_ConstInitVals = 51,             /* ConstInitVals  */
  YYSYMBOL_VarDecl = 52,                   /* VarDecl  */
  YYSYMBOL_VarDecls = 53,                  /* VarDecls  */
  YYSYMBOL_VarDef = 54,                    /* VarDef  */
  YYSYMBOL_InitVal = 55,                   /* InitVal  */
  YYSYMBOL_InitVals = 56,                  /* InitVals  */
  YYSYMBOL_FuncDef = 57,                   /* FuncDef  */
  YYSYMBOL_FuncParams = 58,                /* FuncParams  */
  YYSYMBOL_FuncParam = 59,                 /* FuncParam  */
  YYSYMBOL_Block = 60,                     /* Block  */
  YYSYMBOL_BlockItems = 61,                /* BlockItems  */
  YYSYMBOL_BlockItem = 62,                 /* BlockItem  */
  YYSYMBOL_Stmt = 63,                      /* Stmt  */
  YYSYMBOL_Exp = 64,                       /* Exp  */
  YYSYMBOL_Cond = 65,                      /* Cond  */
  YYSYMBOL_LVal = 66,                      /* LVal  */
  YYSYMBOL_ArraySubscripts = 67,           /* ArraySubscripts  */
  YYSYMBOL_PrimaryExp = 68,                /* PrimaryExp  */
  YYSYMBOL_UnaryExp = 69,                  /* UnaryExp  */
  YYSYMBOL_FuncCall = 70,                  /* FuncCall  */
  YYSYMBOL_CallParams = 71,                /* CallParams  */
  YYSYMBOL_MulExp = 72,                    /* MulExp  */
  YYSYMBOL_AddExp = 73,                    /* AddExp  */
  YYSYMBOL_RelExp = 74,                    /* RelExp  */
  YYSYMBOL_EqExp = 75,                     /* EqExp  */
  YYSYMBOL_LAndExp = 76,                   /* LAndExp  */
  YYSYMBOL_LOrExp = 77,                    /* LOrExp  */
  YYSYMBOL_Type = 78                       /* Type  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  15
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   310

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  102
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  190

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   295


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   187,   187,   197,   202,   210,   216,   223,   227,   236,
     244,   248,   255,   259,   268,   273,   282,   286,   290,   297,
     301,   310,   318,   322,   329,   333,   337,   341,   348,   352,
     356,   363,   367,   376,   380,   384,   388,   393,   398,   406,
     410,   417,   421,   425,   429,   440,   444,   451,   455,   462,
     468,   477,   481,   485,   489,   493,   499,   505,   512,   516,
     520,   524,   534,   541,   548,   552,   571,   576,   585,   590,
     594,   598,   603,   610,   614,   618,   622,   626,   633,   637,
     644,   648,   655,   659,   663,   667,   674,   678,   682,   689,
     693,   697,   701,   705,   712,   716,   720,   727,   731,   738,
     742,   749,   750
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "Y_INT", "Y_FLOAT",
  "Y_VOID", "Y_CONST", "Y_IF", "Y_ELSE", "Y_WHILE", "Y_BREAK",
  "Y_CONTINUE", "Y_RETURN", "Y_ID", "num_INT", "num_FLOAT", "Y_ADD",
  "Y_SUB", "Y_MUL", "Y_DIV", "Y_MODULO", "Y_LESS", "Y_LESSEQ", "Y_GREAT",
  "Y_GREATEQ", "Y_NOTEQ", "Y_EQ", "Y_NOT", "Y_AND", "Y_OR", "Y_ASSIGN",
  "Y_LPAR", "Y_RPAR", "Y_LBRACKET", "Y_RBRACKET", "Y_LSQUARE", "Y_RSQUARE",
  "Y_COMMA", "Y_SEMICOLON", "LOWER_THAN_ELSE", "num_INT_MIN_MAG",
  "$accept", "TranslationUnit", "CompUnits", "CompUnit", "Decl",
  "ConstDecl", "ConstDefs", "ConstDef", "ConstExps", "ConstInitVal",
  "ConstInitVals", "VarDecl", "VarDecls", "VarDef", "InitVal", "InitVals",
  "FuncDef", "FuncParams", "FuncParam", "Block", "BlockItems", "BlockItem",
  "Stmt", "Exp", "Cond", "LVal", "ArraySubscripts", "PrimaryExp",
  "UnaryExp", "FuncCall", "CallParams", "MulExp", "AddExp", "RelExp",
  "EqExp", "LAndExp", "LOrExp", "Type", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-124)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     133,  -124,  -124,    33,     9,    73,   133,  -124,  -124,  -124,
    -124,  -124,    71,    75,    91,  -124,  -124,     7,   -13,  -124,
      15,    13,    29,  -124,   206,    25,    18,    53,   101,  -124,
      88,    90,    54,  -124,   127,   214,    60,    91,  -124,    -8,
    -124,  -124,    18,    18,    18,    18,    94,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,   154,    77,   141,   117,   121,   116,
     120,    90,    92,   134,   206,    18,    96,  -124,    90,    65,
    -124,    90,     9,   146,   184,  -124,  -124,   214,  -124,   235,
      18,   159,  -124,  -124,  -124,   170,  -124,  -124,     2,    18,
      18,    18,    18,    18,    18,    18,    18,    18,    18,    18,
      18,    18,    90,  -124,    90,  -124,  -124,   140,  -124,   173,
     174,   168,   172,   242,  -124,  -124,  -124,  -124,   144,  -124,
    -124,   175,   178,   101,  -124,  -124,   270,   159,  -124,  -124,
      79,  -124,  -124,  -124,    98,   189,    18,  -124,  -124,   206,
    -124,  -124,  -124,   154,   154,    77,    77,    77,    77,   141,
     141,   117,   121,  -124,  -124,  -124,    18,    18,  -124,  -124,
    -124,   188,  -124,  -124,  -124,    18,   177,  -124,   214,  -124,
      18,  -124,   196,  -124,  -124,   202,   203,  -124,   198,   159,
    -124,  -124,  -124,   176,   176,  -124,   230,  -124,   176,  -124
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,   101,   102,     0,     0,     0,     2,     3,     5,     7,
       8,     6,     0,     0,     0,     1,     4,    24,     0,    22,
       0,     0,     0,    10,     0,     0,     0,    26,     0,    21,
       0,     0,     0,    39,     0,     0,     0,     0,     9,    64,
      70,    72,     0,     0,     0,     0,     0,    71,    25,    28,
      69,    73,    82,    74,    86,    89,    94,    97,    99,    62,
       0,     0,     0,     0,     0,     0,    24,    23,     0,     0,
      36,     0,     0,    41,     0,    12,    16,     0,    11,     0,
       0,    65,    75,    76,    77,     0,    29,    31,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,    14,    27,     0,    37,     0,
       0,     0,     0,     0,    45,    52,    49,    54,     0,    47,
      50,     0,    69,     0,    38,    40,     0,    42,    17,    19,
       0,    13,    78,    80,     0,     0,     0,    68,    30,     0,
      83,    84,    85,    87,    88,    90,    91,    92,    93,    96,
      95,    98,   100,    34,    35,    15,     0,     0,    58,    59,
      60,     0,    46,    48,    53,     0,    43,    18,     0,    79,
       0,    66,     0,    32,    63,     0,     0,    61,     0,    44,
      20,    81,    67,     0,     0,    51,    56,    55,     0,    57
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -124,  -124,  -124,   234,   -59,  -124,  -124,   205,   222,   -68,
    -124,  -124,  -124,   216,   -38,  -124,  -124,   228,   191,   -17,
    -124,   142,  -123,   -24,   104,   -66,   -69,  -124,   -27,  -124,
    -124,    87,    72,    97,   164,   167,  -124,     1
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     5,     6,     7,     8,     9,    22,    23,    27,    75,
     130,    10,    18,    19,    48,    88,    11,    32,    33,   117,
     118,   119,   120,   121,   175,    50,    81,    51,    52,    53,
     134,    54,    55,    56,    57,    58,    59,    34
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      49,    12,    63,   122,   127,    14,   129,    12,    87,   131,
     116,    76,     1,     2,    70,    82,    83,    84,     1,     2,
      30,    85,    49,    79,    28,    29,   106,    80,     1,     2,
      60,    39,    40,    41,    42,    43,   138,    24,    25,   139,
      49,   107,    26,    35,   103,    44,    13,    31,    26,    45,
      76,   108,   122,    76,   124,   133,   135,    61,    47,   116,
     186,   187,   140,   141,   142,   189,    37,    38,     1,     2,
     123,     4,   109,    15,   110,   111,   112,   113,    39,    40,
      41,    42,    43,    64,    17,   153,    71,   154,    65,   161,
      77,    72,    44,    92,    93,    65,    45,   179,    69,   114,
     180,   173,   135,   115,    21,    47,    20,    39,    40,    41,
      42,    43,   172,   167,    66,    49,   168,   122,   122,   123,
      68,    44,   122,    69,   104,    45,    24,    46,    86,    72,
     169,    26,   174,   174,    47,   170,     1,     2,     3,     4,
      73,   178,    98,    99,    76,   101,   181,     1,     2,   100,
       4,   109,   102,   110,   111,   112,   113,    39,    40,    41,
      42,    43,    94,    95,    96,    97,   145,   146,   147,   148,
     105,    44,    89,    90,    91,    45,   155,    69,   162,   143,
     144,   126,   115,   109,    47,   110,   111,   112,   113,    39,
      40,    41,    42,    43,   136,   149,   150,    39,    40,    41,
      42,    43,   137,    44,   156,   157,   158,    45,   165,    69,
     159,    44,    80,   164,   115,    45,    47,    74,   128,    39,
      40,    41,    42,    43,    47,   171,   177,    39,    40,    41,
      42,    43,   182,    44,   183,   184,   185,    45,   188,    46,
      16,    44,    78,    36,    67,    45,    47,    74,    39,    40,
      41,    42,    43,    62,    47,    39,    40,    41,    42,    43,
     163,   176,    44,   125,   151,     0,    45,   132,   152,    44,
       0,     0,     0,    45,     0,    47,     0,     0,     0,     0,
     160,     0,    47,    39,    40,    41,    42,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
       0,    45,     0,     0,     0,     0,   166,     0,     0,     0,
      47
};

static const yytype_int16 yycheck[] =
{
      24,     0,    26,    69,    73,     4,    74,     6,    46,    77,
      69,    35,     3,     4,    31,    42,    43,    44,     3,     4,
       5,    45,    46,    31,    37,    38,    64,    35,     3,     4,
       5,    13,    14,    15,    16,    17,    34,    30,    31,    37,
      64,    65,    35,    30,    61,    27,    13,    32,    35,    31,
      74,    68,   118,    77,    71,    79,    80,    32,    40,   118,
     183,   184,    89,    90,    91,   188,    37,    38,     3,     4,
      69,     6,     7,     0,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    30,    13,   102,    32,   104,    35,   113,
      30,    37,    27,    16,    17,    35,    31,   166,    33,    34,
     168,   139,   126,    38,    13,    40,    31,    13,    14,    15,
      16,    17,   136,    34,    13,   139,    37,   183,   184,   118,
      32,    27,   188,    33,    32,    31,    30,    33,    34,    37,
      32,    35,   156,   157,    40,    37,     3,     4,     5,     6,
      13,   165,    25,    26,   168,    29,   170,     3,     4,    28,
       6,     7,    32,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    21,    22,    23,    24,    94,    95,    96,    97,
      36,    27,    18,    19,    20,    31,    36,    33,    34,    92,
      93,    35,    38,     7,    40,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    35,    98,    99,    13,    14,    15,
      16,    17,    32,    27,    31,    31,    38,    31,    30,    33,
      38,    27,    35,    38,    38,    31,    40,    33,    34,    13,
      14,    15,    16,    17,    40,    36,    38,    13,    14,    15,
      16,    17,    36,    27,    32,    32,    38,    31,     8,    33,
       6,    27,    37,    21,    28,    31,    40,    33,    13,    14,
      15,    16,    17,    25,    40,    13,    14,    15,    16,    17,
     118,   157,    27,    72,   100,    -1,    31,    32,   101,    27,
      -1,    -1,    -1,    31,    -1,    40,    -1,    -1,    -1,    -1,
      38,    -1,    40,    13,    14,    15,    16,    17,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,    42,    43,    44,    45,    46,
      52,    57,    78,    13,    78,     0,    44,    13,    53,    54,
      31,    13,    47,    48,    30,    31,    35,    49,    37,    38,
       5,    32,    58,    59,    78,    30,    49,    37,    38,    13,
      14,    15,    16,    17,    27,    31,    33,    40,    55,    64,
      66,    68,    69,    70,    72,    73,    74,    75,    76,    77,
       5,    32,    58,    64,    30,    35,    13,    54,    32,    33,
      60,    32,    37,    13,    33,    50,    64,    30,    48,    31,
      35,    67,    69,    69,    69,    64,    34,    55,    56,    18,
      19,    20,    16,    17,    21,    22,    23,    24,    25,    26,
      28,    29,    32,    60,    32,    36,    55,    64,    60,     7,
       9,    10,    11,    12,    34,    38,    45,    60,    61,    62,
      63,    64,    66,    78,    60,    59,    35,    67,    34,    50,
      51,    50,    32,    64,    71,    64,    35,    32,    34,    37,
      69,    69,    69,    72,    72,    73,    73,    73,    73,    74,
      74,    75,    76,    60,    60,    36,    31,    31,    38,    38,
      38,    64,    34,    62,    38,    30,    36,    34,    37,    32,
      37,    36,    64,    55,    64,    65,    65,    38,    64,    67,
      50,    64,    36,    32,    32,    38,    63,    63,     8,    63
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    41,    42,    43,    43,    44,    44,    45,    45,    46,
      47,    47,    48,    48,    49,    49,    50,    50,    50,    51,
      51,    52,    53,    53,    54,    54,    54,    54,    55,    55,
      55,    56,    56,    57,    57,    57,    57,    57,    57,    58,
      58,    59,    59,    59,    59,    60,    60,    61,    61,    62,
      62,    63,    63,    63,    63,    63,    63,    63,    63,    63,
      63,    63,    64,    65,    66,    66,    67,    67,    68,    68,
      68,    68,    68,    69,    69,    69,    69,    69,    70,    70,
      71,    71,    72,    72,    72,    72,    73,    73,    73,    74,
      74,    74,    74,    74,    75,    75,    75,    76,    76,    77,
      77,    78,    78
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     4,
       1,     3,     3,     4,     3,     4,     1,     2,     3,     1,
       3,     3,     1,     3,     1,     3,     2,     4,     1,     2,
       3,     1,     3,     5,     6,     6,     5,     6,     6,     1,
       3,     2,     3,     4,     5,     2,     3,     1,     2,     1,
       1,     4,     1,     2,     1,     5,     5,     7,     2,     2,
       2,     3,     1,     1,     1,     2,     3,     4,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     3,     4,
       1,     3,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     1,
       3,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */


/* User initialization code.  */
#line 181 "lrparser.y"
{ pending_min_literals = 0; ast_root = NULL; }

#line 1227 "lrparser.tab.c"

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* TranslationUnit: CompUnits  */
#line 188 "lrparser.y"
      {
          if (pending_min_literals != 0) {
              yyerror("decimal integer literal exceeds the signed 32-bit range");
              YYABORT;
          }
      }
#line 1435 "lrparser.tab.c"
    break;

  case 3: /* CompUnits: CompUnit  */
#line 198 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
          ast_root = (yyval.node);
      }
#line 1444 "lrparser.tab.c"
    break;

  case 4: /* CompUnits: CompUnits CompUnit  */
#line 203 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-1].node), (yyvsp[0].node));
          ast_root = (yyval.node);
      }
#line 1453 "lrparser.tab.c"
    break;

  case 5: /* CompUnit: Decl  */
#line 211 "lrparser.y"
      {
          /* At translation-unit scope Clang exposes each VarDecl directly;
             DECL_STMT is used only for declarations inside a compound stmt. */
          (yyval.node) = (yyvsp[0].node) != NULL ? (yyvsp[0].node)->left : NULL;
      }
#line 1463 "lrparser.tab.c"
    break;

  case 6: /* CompUnit: FuncDef  */
#line 217 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1471 "lrparser.tab.c"
    break;

  case 7: /* Decl: ConstDecl  */
#line 224 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1479 "lrparser.tab.c"
    break;

  case 8: /* Decl: VarDecl  */
#line 228 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1487 "lrparser.tab.c"
    break;

  case 9: /* ConstDecl: Y_CONST Type ConstDefs Y_SEMICOLON  */
#line 237 "lrparser.y"
      {
          (yyval.node) = make_decl_stmt((yyvsp[-1].node), 1);
          for (past var = (yyvsp[-1].node); var; var = var->next) var->is_float = (yyvsp[-2].ivalue) == Y_FLOAT;
      }
#line 1496 "lrparser.tab.c"
    break;

  case 10: /* ConstDefs: ConstDef  */
#line 245 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1504 "lrparser.tab.c"
    break;

  case 11: /* ConstDefs: ConstDefs Y_COMMA ConstDef  */
#line 249 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-2].node), (yyvsp[0].node));
      }
#line 1512 "lrparser.tab.c"
    break;

  case 12: /* ConstDef: Y_ID Y_ASSIGN ConstInitVal  */
#line 256 "lrparser.y"
      {
          (yyval.node) = make_var_decl((yyvsp[-2].svalue), NULL, (yyvsp[0].node)); (yyval.node)->is_const = 1;
      }
#line 1520 "lrparser.tab.c"
    break;

  case 13: /* ConstDef: Y_ID ConstExps Y_ASSIGN ConstInitVal  */
#line 260 "lrparser.y"
      {
          /* The accepted ConstDef action keeps the written brace structure;
             only VarDef performs scalar-run grouping. Bounds are type data. */
          (yyval.node) = make_var_decl((yyvsp[-3].svalue), (yyvsp[-2].node), (yyvsp[0].node)); (yyval.node)->is_const = 1;
      }
#line 1530 "lrparser.tab.c"
    break;

  case 14: /* ConstExps: Y_LSQUARE Exp Y_RSQUARE  */
#line 269 "lrparser.y"
      {
          (yyval.node) = newAstNode(ARRAY_SUBSCRIPT_EXPR);
          (yyval.node)->right = (yyvsp[-1].node);
      }
#line 1539 "lrparser.tab.c"
    break;

  case 15: /* ConstExps: ConstExps Y_LSQUARE Exp Y_RSQUARE  */
#line 274 "lrparser.y"
      {
          past dim = newAstNode(ARRAY_SUBSCRIPT_EXPR);
          dim->right = (yyvsp[-1].node);
          (yyval.node) = appendAst((yyvsp[-3].node), dim);
      }
#line 1549 "lrparser.tab.c"
    break;

  case 16: /* ConstInitVal: Exp  */
#line 283 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1557 "lrparser.tab.c"
    break;

  case 17: /* ConstInitVal: Y_LBRACKET Y_RBRACKET  */
#line 287 "lrparser.y"
      {
          (yyval.node) = make_init_list(NULL);
      }
#line 1565 "lrparser.tab.c"
    break;

  case 18: /* ConstInitVal: Y_LBRACKET ConstInitVals Y_RBRACKET  */
#line 291 "lrparser.y"
      {
          (yyval.node) = make_init_list((yyvsp[-1].node));
      }
#line 1573 "lrparser.tab.c"
    break;

  case 19: /* ConstInitVals: ConstInitVal  */
#line 298 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1581 "lrparser.tab.c"
    break;

  case 20: /* ConstInitVals: ConstInitVals Y_COMMA ConstInitVal  */
#line 302 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-2].node), (yyvsp[0].node));
      }
#line 1589 "lrparser.tab.c"
    break;

  case 21: /* VarDecl: Type VarDecls Y_SEMICOLON  */
#line 311 "lrparser.y"
      {
          (yyval.node) = make_decl_stmt((yyvsp[-1].node), 0);
          for (past var = (yyvsp[-1].node); var; var = var->next) var->is_float = (yyvsp[-2].ivalue) == Y_FLOAT;
      }
#line 1598 "lrparser.tab.c"
    break;

  case 22: /* VarDecls: VarDef  */
#line 319 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1606 "lrparser.tab.c"
    break;

  case 23: /* VarDecls: VarDecls Y_COMMA VarDef  */
#line 323 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-2].node), (yyvsp[0].node));
      }
#line 1614 "lrparser.tab.c"
    break;

  case 24: /* VarDef: Y_ID  */
#line 330 "lrparser.y"
      {
          (yyval.node) = make_var_decl((yyvsp[0].svalue), NULL, NULL);
      }
#line 1622 "lrparser.tab.c"
    break;

  case 25: /* VarDef: Y_ID Y_ASSIGN InitVal  */
#line 334 "lrparser.y"
      {
          (yyval.node) = make_var_decl((yyvsp[-2].svalue), NULL, (yyvsp[0].node));
      }
#line 1630 "lrparser.tab.c"
    break;

  case 26: /* VarDef: Y_ID ConstExps  */
#line 338 "lrparser.y"
      {
          (yyval.node) = make_var_decl((yyvsp[-1].svalue), (yyvsp[0].node), NULL);
      }
#line 1638 "lrparser.tab.c"
    break;

  case 27: /* VarDef: Y_ID ConstExps Y_ASSIGN InitVal  */
#line 342 "lrparser.y"
      {
          (yyval.node) = make_var_decl((yyvsp[-3].svalue), (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 1646 "lrparser.tab.c"
    break;

  case 28: /* InitVal: Exp  */
#line 349 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1654 "lrparser.tab.c"
    break;

  case 29: /* InitVal: Y_LBRACKET Y_RBRACKET  */
#line 353 "lrparser.y"
      {
          (yyval.node) = make_init_list(NULL);
      }
#line 1662 "lrparser.tab.c"
    break;

  case 30: /* InitVal: Y_LBRACKET InitVals Y_RBRACKET  */
#line 357 "lrparser.y"
      {
          (yyval.node) = make_init_list((yyvsp[-1].node));
      }
#line 1670 "lrparser.tab.c"
    break;

  case 31: /* InitVals: InitVal  */
#line 364 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1678 "lrparser.tab.c"
    break;

  case 32: /* InitVals: InitVals Y_COMMA InitVal  */
#line 368 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-2].node), (yyvsp[0].node));
      }
#line 1686 "lrparser.tab.c"
    break;

  case 33: /* FuncDef: Type Y_ID Y_LPAR Y_RPAR Block  */
#line 377 "lrparser.y"
      {
          (yyval.node) = make_func((yyvsp[-3].svalue), NULL, (yyvsp[0].node)); (yyval.node)->is_float = (yyvsp[-4].ivalue) == Y_FLOAT;
      }
#line 1694 "lrparser.tab.c"
    break;

  case 34: /* FuncDef: Type Y_ID Y_LPAR Y_VOID Y_RPAR Block  */
#line 381 "lrparser.y"
      {
          (yyval.node) = make_func((yyvsp[-4].svalue), NULL, (yyvsp[0].node)); (yyval.node)->is_float = (yyvsp[-5].ivalue) == Y_FLOAT;
      }
#line 1702 "lrparser.tab.c"
    break;

  case 35: /* FuncDef: Type Y_ID Y_LPAR FuncParams Y_RPAR Block  */
#line 385 "lrparser.y"
      {
          (yyval.node) = make_func((yyvsp[-4].svalue), (yyvsp[-2].node), (yyvsp[0].node)); (yyval.node)->is_float = (yyvsp[-5].ivalue) == Y_FLOAT;
      }
#line 1710 "lrparser.tab.c"
    break;

  case 36: /* FuncDef: Y_VOID Y_ID Y_LPAR Y_RPAR Block  */
#line 389 "lrparser.y"
      {
          (yyval.node) = make_func((yyvsp[-3].svalue), NULL, (yyvsp[0].node));
          (yyval.node)->ivalue = Y_VOID;
      }
#line 1719 "lrparser.tab.c"
    break;

  case 37: /* FuncDef: Y_VOID Y_ID Y_LPAR Y_VOID Y_RPAR Block  */
#line 394 "lrparser.y"
      {
          (yyval.node) = make_func((yyvsp[-4].svalue), NULL, (yyvsp[0].node));
          (yyval.node)->ivalue = Y_VOID;
      }
#line 1728 "lrparser.tab.c"
    break;

  case 38: /* FuncDef: Y_VOID Y_ID Y_LPAR FuncParams Y_RPAR Block  */
#line 399 "lrparser.y"
      {
          (yyval.node) = make_func((yyvsp[-4].svalue), (yyvsp[-2].node), (yyvsp[0].node));
          (yyval.node)->ivalue = Y_VOID;
      }
#line 1737 "lrparser.tab.c"
    break;

  case 39: /* FuncParams: FuncParam  */
#line 407 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1745 "lrparser.tab.c"
    break;

  case 40: /* FuncParams: FuncParams Y_COMMA FuncParam  */
#line 411 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-2].node), (yyvsp[0].node));
      }
#line 1753 "lrparser.tab.c"
    break;

  case 41: /* FuncParam: Type Y_ID  */
#line 418 "lrparser.y"
      {
          (yyval.node) = make_param((yyvsp[0].svalue), NULL); (yyval.node)->is_float = (yyvsp[-1].ivalue) == Y_FLOAT;
      }
#line 1761 "lrparser.tab.c"
    break;

  case 42: /* FuncParam: Type Y_ID ArraySubscripts  */
#line 422 "lrparser.y"
      {
          (yyval.node) = make_param((yyvsp[-1].svalue), (yyvsp[0].node)); (yyval.node)->is_float = (yyvsp[-2].ivalue) == Y_FLOAT;
      }
#line 1769 "lrparser.tab.c"
    break;

  case 43: /* FuncParam: Type Y_ID Y_LSQUARE Y_RSQUARE  */
#line 426 "lrparser.y"
      {
          (yyval.node) = make_param((yyvsp[-2].svalue), newAstNode(ARRAY_SUBSCRIPT_EXPR)); (yyval.node)->is_float = (yyvsp[-3].ivalue) == Y_FLOAT;
      }
#line 1777 "lrparser.tab.c"
    break;

  case 44: /* FuncParam: Type Y_ID Y_LSQUARE Y_RSQUARE ArraySubscripts  */
#line 430 "lrparser.y"
      {
          past first = newAstNode(ARRAY_SUBSCRIPT_EXPR);
          first->next = (yyvsp[0].node);
          (yyval.node) = make_param((yyvsp[-3].svalue), first); (yyval.node)->is_float = (yyvsp[-4].ivalue) == Y_FLOAT;
      }
#line 1787 "lrparser.tab.c"
    break;

  case 45: /* Block: Y_LBRACKET Y_RBRACKET  */
#line 441 "lrparser.y"
      {
          (yyval.node) = make_block(NULL);
      }
#line 1795 "lrparser.tab.c"
    break;

  case 46: /* Block: Y_LBRACKET BlockItems Y_RBRACKET  */
#line 445 "lrparser.y"
      {
          (yyval.node) = make_block((yyvsp[-1].node));
      }
#line 1803 "lrparser.tab.c"
    break;

  case 47: /* BlockItems: BlockItem  */
#line 452 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1811 "lrparser.tab.c"
    break;

  case 48: /* BlockItems: BlockItems BlockItem  */
#line 456 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-1].node), (yyvsp[0].node));
      }
#line 1819 "lrparser.tab.c"
    break;

  case 49: /* BlockItem: Decl  */
#line 463 "lrparser.y"
      {
          /* The accepted lab implementation emits local const declarations
             directly, while ordinary local declarations retain DECL_STMT. */
          (yyval.node) = (yyvsp[0].node) != NULL && (yyvsp[0].node)->ivalue == 1 ? (yyvsp[0].node)->left : (yyvsp[0].node);
      }
#line 1829 "lrparser.tab.c"
    break;

  case 50: /* BlockItem: Stmt  */
#line 469 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1837 "lrparser.tab.c"
    break;

  case 51: /* Stmt: LVal Y_ASSIGN Exp Y_SEMICOLON  */
#line 478 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_ASSIGN, (yyvsp[-3].node), (yyvsp[-1].node));
      }
#line 1845 "lrparser.tab.c"
    break;

  case 52: /* Stmt: Y_SEMICOLON  */
#line 482 "lrparser.y"
      {
          (yyval.node) = newAstNode(NULL_STMT);
      }
#line 1853 "lrparser.tab.c"
    break;

  case 53: /* Stmt: Exp Y_SEMICOLON  */
#line 486 "lrparser.y"
      {
          (yyval.node) = (yyvsp[-1].node);
      }
#line 1861 "lrparser.tab.c"
    break;

  case 54: /* Stmt: Block  */
#line 490 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1869 "lrparser.tab.c"
    break;

  case 55: /* Stmt: Y_WHILE Y_LPAR Cond Y_RPAR Stmt  */
#line 494 "lrparser.y"
      {
          (yyval.node) = newAstNode(WHILE_STMT);
          (yyval.node)->left = (yyvsp[-2].node);
          (yyval.node)->right = (yyvsp[0].node);
      }
#line 1879 "lrparser.tab.c"
    break;

  case 56: /* Stmt: Y_IF Y_LPAR Cond Y_RPAR Stmt  */
#line 500 "lrparser.y"
      {
          (yyval.node) = newAstNode(IF_STMT);
          (yyval.node)->if_cond = (yyvsp[-2].node);
          (yyval.node)->left = (yyvsp[0].node);
      }
#line 1889 "lrparser.tab.c"
    break;

  case 57: /* Stmt: Y_IF Y_LPAR Cond Y_RPAR Stmt Y_ELSE Stmt  */
#line 506 "lrparser.y"
      {
          (yyval.node) = newAstNode(IF_STMT);
          (yyval.node)->if_cond = (yyvsp[-4].node);
          (yyval.node)->left = (yyvsp[-2].node);
          (yyval.node)->right = (yyvsp[0].node);
      }
#line 1900 "lrparser.tab.c"
    break;

  case 58: /* Stmt: Y_BREAK Y_SEMICOLON  */
#line 513 "lrparser.y"
      {
          (yyval.node) = newAstNode(BREAK_STMT);
      }
#line 1908 "lrparser.tab.c"
    break;

  case 59: /* Stmt: Y_CONTINUE Y_SEMICOLON  */
#line 517 "lrparser.y"
      {
          (yyval.node) = newAstNode(CONTINUE_STMT);
      }
#line 1916 "lrparser.tab.c"
    break;

  case 60: /* Stmt: Y_RETURN Y_SEMICOLON  */
#line 521 "lrparser.y"
      {
          (yyval.node) = newAstNode(RETURN_STMT);
      }
#line 1924 "lrparser.tab.c"
    break;

  case 61: /* Stmt: Y_RETURN Exp Y_SEMICOLON  */
#line 525 "lrparser.y"
      {
          (yyval.node) = newAstNode(RETURN_STMT);
          (yyval.node)->left = (yyvsp[-1].node);
      }
#line 1933 "lrparser.tab.c"
    break;

  case 62: /* Exp: LOrExp  */
#line 535 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1941 "lrparser.tab.c"
    break;

  case 63: /* Cond: Exp  */
#line 542 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 1949 "lrparser.tab.c"
    break;

  case 64: /* LVal: Y_ID  */
#line 549 "lrparser.y"
      {
          (yyval.node) = newID((yyvsp[0].svalue));
      }
#line 1957 "lrparser.tab.c"
    break;

  case 65: /* LVal: Y_ID ArraySubscripts  */
#line 553 "lrparser.y"
      {
          past base = newID((yyvsp[-1].svalue));
          past subs = (yyvsp[0].node);
          past cur = subs;

          while (cur != NULL) {
              past next = cur->next;
              cur->left = base;
              cur->next = NULL;
              base = cur;
              cur = next;
          }

          (yyval.node) = base;
      }
#line 1977 "lrparser.tab.c"
    break;

  case 66: /* ArraySubscripts: Y_LSQUARE Exp Y_RSQUARE  */
#line 572 "lrparser.y"
      {
          (yyval.node) = newAstNode(ARRAY_SUBSCRIPT_EXPR);
          (yyval.node)->right = (yyvsp[-1].node);
      }
#line 1986 "lrparser.tab.c"
    break;

  case 67: /* ArraySubscripts: ArraySubscripts Y_LSQUARE Exp Y_RSQUARE  */
#line 577 "lrparser.y"
      {
          past dim = newAstNode(ARRAY_SUBSCRIPT_EXPR);
          dim->right = (yyvsp[-1].node);
          (yyval.node) = appendAst((yyvsp[-3].node), dim);
      }
#line 1996 "lrparser.tab.c"
    break;

  case 68: /* PrimaryExp: Y_LPAR Exp Y_RPAR  */
#line 586 "lrparser.y"
      {
          (yyval.node) = newAstNode(PAREN_EXPR);
          (yyval.node)->left = (yyvsp[-1].node);
      }
#line 2005 "lrparser.tab.c"
    break;

  case 69: /* PrimaryExp: LVal  */
#line 591 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2013 "lrparser.tab.c"
    break;

  case 70: /* PrimaryExp: num_INT  */
#line 595 "lrparser.y"
      {
          (yyval.node) = newInt((yyvsp[0].ivalue));
      }
#line 2021 "lrparser.tab.c"
    break;

  case 71: /* PrimaryExp: num_INT_MIN_MAG  */
#line 599 "lrparser.y"
      {
          ++pending_min_literals;
          (yyval.node) = newAstNode(INT_MIN_MAGNITUDE);
      }
#line 2030 "lrparser.tab.c"
    break;

  case 72: /* PrimaryExp: num_FLOAT  */
#line 604 "lrparser.y"
      {
          (yyval.node) = newFloat((yyvsp[0].fvalue));
      }
#line 2038 "lrparser.tab.c"
    break;

  case 73: /* UnaryExp: PrimaryExp  */
#line 611 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2046 "lrparser.tab.c"
    break;

  case 74: /* UnaryExp: FuncCall  */
#line 615 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2054 "lrparser.tab.c"
    break;

  case 75: /* UnaryExp: Y_ADD UnaryExp  */
#line 619 "lrparser.y"
      {
          (yyval.node) = make_unary(Y_ADD, (yyvsp[0].node));
      }
#line 2062 "lrparser.tab.c"
    break;

  case 76: /* UnaryExp: Y_SUB UnaryExp  */
#line 623 "lrparser.y"
      {
          (yyval.node) = make_unary(Y_SUB, (yyvsp[0].node));
      }
#line 2070 "lrparser.tab.c"
    break;

  case 77: /* UnaryExp: Y_NOT UnaryExp  */
#line 627 "lrparser.y"
      {
          (yyval.node) = make_unary(Y_NOT, (yyvsp[0].node));
      }
#line 2078 "lrparser.tab.c"
    break;

  case 78: /* FuncCall: Y_ID Y_LPAR Y_RPAR  */
#line 634 "lrparser.y"
      {
          (yyval.node) = make_call((yyvsp[-2].svalue), NULL);
      }
#line 2086 "lrparser.tab.c"
    break;

  case 79: /* FuncCall: Y_ID Y_LPAR CallParams Y_RPAR  */
#line 638 "lrparser.y"
      {
          (yyval.node) = make_call((yyvsp[-3].svalue), (yyvsp[-1].node));
      }
#line 2094 "lrparser.tab.c"
    break;

  case 80: /* CallParams: Exp  */
#line 645 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2102 "lrparser.tab.c"
    break;

  case 81: /* CallParams: CallParams Y_COMMA Exp  */
#line 649 "lrparser.y"
      {
          (yyval.node) = appendAst((yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2110 "lrparser.tab.c"
    break;

  case 82: /* MulExp: UnaryExp  */
#line 656 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2118 "lrparser.tab.c"
    break;

  case 83: /* MulExp: MulExp Y_MUL UnaryExp  */
#line 660 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_MUL, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2126 "lrparser.tab.c"
    break;

  case 84: /* MulExp: MulExp Y_DIV UnaryExp  */
#line 664 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_DIV, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2134 "lrparser.tab.c"
    break;

  case 85: /* MulExp: MulExp Y_MODULO UnaryExp  */
#line 668 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_MODULO, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2142 "lrparser.tab.c"
    break;

  case 86: /* AddExp: MulExp  */
#line 675 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2150 "lrparser.tab.c"
    break;

  case 87: /* AddExp: AddExp Y_ADD MulExp  */
#line 679 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_ADD, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2158 "lrparser.tab.c"
    break;

  case 88: /* AddExp: AddExp Y_SUB MulExp  */
#line 683 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_SUB, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2166 "lrparser.tab.c"
    break;

  case 89: /* RelExp: AddExp  */
#line 690 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2174 "lrparser.tab.c"
    break;

  case 90: /* RelExp: RelExp Y_LESS AddExp  */
#line 694 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_LESS, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2182 "lrparser.tab.c"
    break;

  case 91: /* RelExp: RelExp Y_LESSEQ AddExp  */
#line 698 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_LESSEQ, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2190 "lrparser.tab.c"
    break;

  case 92: /* RelExp: RelExp Y_GREAT AddExp  */
#line 702 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_GREAT, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2198 "lrparser.tab.c"
    break;

  case 93: /* RelExp: RelExp Y_GREATEQ AddExp  */
#line 706 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_GREATEQ, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2206 "lrparser.tab.c"
    break;

  case 94: /* EqExp: RelExp  */
#line 713 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2214 "lrparser.tab.c"
    break;

  case 95: /* EqExp: EqExp Y_EQ RelExp  */
#line 717 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_EQ, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2222 "lrparser.tab.c"
    break;

  case 96: /* EqExp: EqExp Y_NOTEQ RelExp  */
#line 721 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_NOTEQ, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2230 "lrparser.tab.c"
    break;

  case 97: /* LAndExp: EqExp  */
#line 728 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2238 "lrparser.tab.c"
    break;

  case 98: /* LAndExp: LAndExp Y_AND EqExp  */
#line 732 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_AND, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2246 "lrparser.tab.c"
    break;

  case 99: /* LOrExp: LAndExp  */
#line 739 "lrparser.y"
      {
          (yyval.node) = (yyvsp[0].node);
      }
#line 2254 "lrparser.tab.c"
    break;

  case 100: /* LOrExp: LOrExp Y_OR LAndExp  */
#line 743 "lrparser.y"
      {
          (yyval.node) = make_binary(Y_OR, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2262 "lrparser.tab.c"
    break;

  case 101: /* Type: Y_INT  */
#line 749 "lrparser.y"
              { (yyval.ivalue) = Y_INT; }
#line 2268 "lrparser.tab.c"
    break;

  case 102: /* Type: Y_FLOAT  */
#line 750 "lrparser.y"
              { (yyval.ivalue) = Y_FLOAT; }
#line 2274 "lrparser.tab.c"
    break;


#line 2278 "lrparser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 753 "lrparser.y"


void yyerror(const char *s)
{
    fprintf(stderr, "Syntax error: %s\n", s);
}








