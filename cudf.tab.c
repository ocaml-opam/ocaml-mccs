/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         cudfparse
#define yylex           cudflex
#define yyerror         cudferror
#define yydebug         cudfdebug
#define yynerrs         cudfnerrs

#define yylval          cudflval
#define yychar          cudfchar

/* Copy the first part of user declarations.  */
#line 10 "cudf.y" /* yacc.c:339  */


#include <stdio.h>
#include <string.h>
#include <cudf.h>
#include <cudf_hash_table.h>

extern int cudflineno;          /* number of the current line */
extern int cudflex (void);      /* lexical analyser */

/* Print out a CUDF problem error location */
void yyerror(const char *str)
{
        fprintf(stderr,"cudf syntax error (line: %d): %s\n", cudflineno, str);
}
 
int package_line = 0;    /* last line of a package declaration */       

CUDFVersionedPackage *current_package = (CUDFVersionedPackage *)NULL;  /* package under analysis */
int versioned_package_rank = 0;   /* last rank of a versionned package */
CUDFproblem *current_problem = (CUDFproblem *)NULL;  /* current problem */

CUDFVersionedPackageList all_packages;                       /* list of all versioned packages */
CUDFVirtualPackageList all_virtual_packages;                 /* list of all virtual packages */
CUDFVirtualPackageList version_installed_virtual_packages;   /* list of all virtual packages with at least one version installed */
CUDFVirtualPackageList all_virtual_packages_with_version;    /* list of all virtual packages with at least one */
CUDFVirtualPackageList uninstalled_virtual_packages_with_version;    /* list of all virtual packages with at least one */
CUDFVersionedPackageList installed_packages;                 /* list of all installed versioned packages */
CUDFVersionedPackageList uninstalled_packages;               /* list of all uninstalled versioned packages */
CUDFproblem *the_problem = (CUDFproblem *)NULL;              /* the CUDF problem to solve */

CUDFVpkgFormula *current_vpkgformula = (CUDFVpkgFormula *)NULL;  /* vpkgformula under analysis */
CUDFVpkgList *current_vpkglist = (CUDFVpkgList *)NULL;           /* vpkglist under analysis */
CUDFVpkg *current_vpkg = (CUDFVpkg *)NULL;                       /* vpkg under analysis */
CUDFPropertyType current_pt_type = pt_vpkgformula;               /* type of the user property under analysis */



CUDFVpkg *build_vpkg(const char *vpkgname, CUDFPackageOp op, CUDFVersion version) {  /* internal construction of a vpkg */
  return new CUDFVpkg(get_virtual_package(vpkgname), op, version);
}

void build_vpkgformula() {  /* internal construction of a vpkg formula */
  if (current_vpkgformula == (CUDFVpkgFormula *)NULL) {
    current_vpkgformula = new CUDFVpkgFormula;
  }
  current_vpkgformula->push_back(current_vpkglist);
  current_vpkglist = (CUDFVpkgList *)NULL;
}

void build_vpkglist(CUDFVpkg *avpkg) {  /* internal construction of a vpkglist */
  if (current_vpkglist == (CUDFVpkgList *)NULL) {
    current_vpkglist = new CUDFVpkgList;
  }
  current_vpkglist->push_back(avpkg);
}

// internal construction of a veqpkglist ... only used by provides
void build_veqpkglist(CUDFVpkg *avpkg) {
  CUDFVirtualPackage *vpackage = avpkg->virtual_package;
  CUDFVersion version = avpkg->version;

  if (current_vpkglist == (CUDFVpkgList *)NULL) {
    current_vpkglist = new CUDFVpkgList;
  }
  current_vpkglist->push_back(avpkg);
  switch(avpkg->op) {
  case op_none: 
    vpackage->providers.push_back(current_package); 
    break;
  case op_eq:
    if ((current_package->installed) && (version > vpackage->highest_installed_provider_version))
      vpackage->highest_installed_provider_version = version;
    {
      CUDFVersionedProviderListIterator ivpkgl = vpackage->versioned_providers.find(version);
      if (ivpkgl == vpackage->versioned_providers.end())
	vpackage->versioned_providers.insert(CUDFVersionedProviderList::value_type(version, CUDFProviderList(1, current_package)));
      else
	ivpkgl->second.push_back(current_package);
    }
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): veqpkglist is restricted to = operator.\n", cudflineno);
    exit(-1);
    break;
  }
}

// get package version
CUDFVersion getversion(const char *svalue) {
  CUDFVersion version = 0;
  if (svalue[0] != '-') {
      sscanf(svalue, "%llu", &version);
      if (version >= 0) return version; // WARNING : should be restricted to > 0
  }
  fprintf(stderr, "Package version must be a <posint>.\n");
  exit(-1);
}

// list of the user required properties
vector<CUDFProperty *> required_properties;

// post process a versioned package declaration
void package_postprocess() {
  if (current_package != (CUDFVersionedPackage *)NULL) {
    CUDFVirtualPackage *vpackage = current_package->virtual_package;

    if (vpackage->all_versions.find(current_package) == vpackage->all_versions.end())
      vpackage->all_versions.insert(current_package);
    else {
      fprintf(stderr, "CUDF error (line %d): (package, version) must be unique (see package %s).\n", package_line, current_package->name);
      exit(-1);
    }
    if (current_package->installed) {
      installed_packages.push_back(current_package);
      if (vpackage->highest_installed == (CUDFVersionedPackage *)NULL)
	vpackage->highest_installed = current_package;
      else if (current_package->version > vpackage->highest_installed->version)
	vpackage->highest_installed = current_package;
      if (current_package->provides != (CUDFVpkgList *)NULL)
	for (CUDFVpkgListIterator iop = current_package->provides->begin(); iop != current_package->provides->end(); iop++)
	  if ((*iop)->op == op_eq) {
	    if ((*iop)->virtual_package->highest_installed_provider_version < (*iop)->version)
	      (*iop)->virtual_package->highest_installed_provider_version = (*iop)->version;
	  }
    } else
      uninstalled_packages.push_back(current_package);
    if (current_package->version > vpackage->highest_version)
      vpackage->highest_version = current_package->version;
    for (vector<CUDFProperty *>::iterator prop = required_properties.begin(); prop != required_properties.end(); prop++) {
      bool hasprop = false;
      for (CUDFPropertyValueListIterator pval = current_package->properties.begin(); pval != current_package->properties.end(); pval++)
	if ((*pval)->property == *prop) { hasprop = true; break; }
      if (! hasprop) {
	fprintf(stderr, "CUDF error (line %d): package (%s, %llu) lacks property %s.\n",
		package_line, current_package->name, current_package->version, (*prop)->name);
	exit(-1);
      }
    }
	
  }
}

//#define YYMAXDEPTH 800000
//#define YYMAXDEPTH 80

// list of enum under analysis
CUDFEnums *enuml;

// build an enum
void build_enum(char *estr) {
  char *the_enum;

  if ((the_enum = (char *)malloc(strlen(estr)+1)) == (char *)NULL) {
    fprintf(stderr, "CUDF error: can not alloc memory for enum %s.\n", estr);
    exit(-1);
  }
  strcpy(the_enum, estr);

  enuml->push_back(the_enum);
}

// get an enum from its name in an enum list
char *get_enum(CUDFEnums *e, char *estr) {
  for (CUDFEnumsIterator ei = e->begin(); ei != e->end(); ei++)
    if (strcmp((*ei), estr) == 0) return (*ei);
  return (char *)NULL;
}

// get a property ident type from a user property name
extern int pidenttype(char *pname);
int pidenttype(char *pname) {
  CUDFPropertiesIterator p = properties.find(string(pname));

  if (p == properties.end()) 
    return pt_none;

  return p->second->type_id;
}

// get a CUDF type from a type name
CUDFPropertyType gettype(char *ident) {
  /* enum CUDFPropertyType { pt_bool, pt_int, pt_nat, pt_posint, pt_enum, pt_string, */
  int length = strlen(ident);

  if (length >= 1)
    switch (ident[0]) {
    case 'b':
      if ((length == 4) && (ident[1] == 'o') && (ident[2] == 'o') && (ident[3] == 'l') && (ident[4] == '\0')) return pt_bool; 
    case 'e':
      if ((length == 4) && (ident[1] == 'n') && (ident[2] == 'u') && (ident[3] == 'm') && (ident[4] == '\0')) return pt_enum; 
    case 'i':
      if ((length == 3) && (ident[1] == 'n') && (ident[2] == 't') && (ident[3] == '\0')) return pt_int; 
    case 'n':
      if ((length == 3) && (ident[1] == 'a') && (ident[2] == 't') && (ident[3] == '\0')) return pt_nat; 
    case 'p':
      if ((length == 6) && (ident[1] == 'o') && (ident[2] == 's') && (ident[3] == 'i') && (ident[4] == 'n') && (ident[5] == 't') && (ident[6] == '\0')) 
	return pt_posint; 
    case 's':
      if ((length == 6) && (ident[1] == 't') && (ident[2] == 'r') && (ident[3] == 'i') && (ident[4] == 'n') && (ident[5] == 'g') && (ident[6] == '\0')) 
	return pt_string; 
    case 'v':
      if (length >= 4) {
	if (ident[1] == 'p') {
	  if ((ident[2] == 'k') && (ident[3] == 'g'))
	    switch (ident[4]) {
	    case '\0': 
	      return pt_vpkg;
	    case 'l': 
	      if ((length == 8) && (ident[5] == 'i') && (ident[6] == 's') && (ident[7] == 't') && (ident[8] == '\0')) return pt_vpkglist;
	    case 'f': 
	      if ((length == 11) && (ident[5] == 'o') && (ident[6] == 'r') && (ident[7] == 'm')  && (ident[8] == 'u')  
		  && (ident[9] == 'l')  && (ident[10] == 'a') && (ident[11] == '\0')) return pt_vpkgformula;
	    }
	} else if (ident[1] == 'e') {
	  if ((ident[2] == 'q') && (ident[3] == 'p') && (length >= 6) && (ident[4] == 'k') && (ident[5] == 'g'))
	    switch (ident[6]) {
	    case '\0': 
	      return pt_veqpkg;
	    case 'l': 
	      if ((length == 10) && (ident[7] == 'i') && (ident[8] == 's') && (ident[9] == 't') && (ident[10] == '\0')) return pt_veqpkglist;
	    }
	}
      }
    }

  fprintf(stderr, "CUDF error (line %d): property type awaited \"%s\" (%d).\n", cudflineno, ident, length);
  exit(-1);
}

extern char astring[];


#line 308 "cudf.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cudfdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PREAMBLE = 258,
    PROPERTYDEF = 259,
    PACKAGE = 260,
    VERSION = 261,
    DEPENDS = 262,
    CONFLICTS = 263,
    PROVIDES = 264,
    INSTALLED = 265,
    KEEP = 266,
    KEEPVERSION = 267,
    KEEPPACKAGE = 268,
    KEEPFEATURE = 269,
    KEEPNONE = 270,
    EQ = 271,
    NEQ = 272,
    SUP = 273,
    SUPEQ = 274,
    INF = 275,
    INFEQ = 276,
    TRUE = 277,
    FALSE = 278,
    VTRUE = 279,
    VFALSE = 280,
    PROBLEM = 281,
    INSTALL = 282,
    WASINSTALLED = 283,
    REMOVE = 284,
    UPGRADE = 285,
    PIDENT_BOOL = 286,
    PIDENT_INT = 287,
    PIDENT_NAT = 288,
    PIDENT_POSINT = 289,
    PIDENT_ENUM = 290,
    PIDENT_STRING = 291,
    PIDENT_VPKG = 292,
    PIDENT_VEQPKG = 293,
    PIDENT_VPKGLIST = 294,
    PIDENT_VEQPKGLIST = 295,
    PIDENT_VPKGFORMULA = 296,
    INTEGER = 297,
    IDENT = 298,
    PIDENT = 299,
    STRING = 300
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 245 "cudf.y" /* yacc.c:355  */

  //  CUDFVersion value;
  CUDFVpkg *avpkg;
  char str[256];

#line 397 "cudf.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE cudflval;

int cudfparse (void);



/* Copy the second part of user declarations.  */

#line 414 "cudf.tab.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   125

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  99
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  142

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   300

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    46,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    47,     2,    48,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    49,     2,     2,     2,     2,     2,
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
      45
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   267,   267,   268,   269,   270,   273,   274,   276,   277,
     280,   280,   282,   283,   290,   297,   309,   317,   327,   342,
     356,   376,   377,   386,   387,   390,   393,   402,   403,   406,
     406,   406,   406,   406,   406,   406,   406,   409,   409,   412,
     412,   415,   415,   415,   418,   420,   420,   430,   430,   440,
     440,   450,   452,   454,   456,   458,   463,   479,   495,   513,
     535,   557,   579,   595,   611,   627,   627,   644,   644,   661,
     679,   685,   685,   695,   695,   705,   705,   715,   724,   733,
     742,   747,   748,   749,   751,   752,   754,   755,   757,   758,
     760,   761,   762,   763,   764,   765,   768,   769,   771,   772
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PREAMBLE", "PROPERTYDEF", "PACKAGE",
  "VERSION", "DEPENDS", "CONFLICTS", "PROVIDES", "INSTALLED", "KEEP",
  "KEEPVERSION", "KEEPPACKAGE", "KEEPFEATURE", "KEEPNONE", "EQ", "NEQ",
  "SUP", "SUPEQ", "INF", "INFEQ", "TRUE", "FALSE", "VTRUE", "VFALSE",
  "PROBLEM", "INSTALL", "WASINSTALLED", "REMOVE", "UPGRADE", "PIDENT_BOOL",
  "PIDENT_INT", "PIDENT_NAT", "PIDENT_POSINT", "PIDENT_ENUM",
  "PIDENT_STRING", "PIDENT_VPKG", "PIDENT_VEQPKG", "PIDENT_VPKGLIST",
  "PIDENT_VEQPKGLIST", "PIDENT_VPKGFORMULA", "INTEGER", "IDENT", "PIDENT",
  "STRING", "','", "'['", "']'", "'|'", "$accept", "cudf", "preambles",
  "property_definitions", "typedecls", "typedecl", "enums",
  "package_declarations", "package_declaration", "package_version",
  "package_options", "package_option", "problem_declaration",
  "problem_actions", "problem_action", "version", "depends", "conflicts",
  "provides", "installed", "wasinstalled", "keep", "property", "problem",
  "install", "remove", "upgrade", "keepvalue", "vpkgformula",
  "avpkgformula", "vpkgor", "veqpkg", "vpkg", "vpkglist", "veqpkglist", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,    44,    91,    93,   124
};
# endif

#define YYPACT_NINF -58

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-58)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       0,    -3,   -58,    18,    25,   -58,   -16,   -17,    24,   -58,
      27,   -58,   -58,    31,     3,   -58,   -58,    32,    32,    32,
     -16,   -58,   -58,   -58,   -58,    27,    -9,   -17,   -58,   -58,
     -58,    14,    44,   -58,   -58,    35,    35,    35,   -58,    -4,
      37,   -58,    36,   -15,    32,    42,    17,    22,    47,    49,
      46,    48,    50,    43,    51,    32,    42,    32,    42,   -15,
     -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,    52,
      53,    55,    56,    57,    58,    32,    34,   -58,    20,   -58,
     -58,   -58,   -58,    45,    40,   -58,    35,    77,   -58,    59,
     -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,
     -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,    35,    59,
     -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,    54,    60,
      61,    62,   -58,    63,    64,    85,    32,    32,    42,   -58,
     -58,   -58,   -58,   -58,   -58,    65,    40,   -58,   -58,    70,
      66,   -58
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,    70,     0,     2,     4,    37,     0,     0,     7,
       8,    10,     1,     0,     3,    23,    27,    71,    73,    75,
      38,    39,    41,    42,    43,     9,    12,     0,    26,    24,
       5,    25,    88,    90,    96,    72,    74,    76,    40,     0,
       0,    11,     0,    45,    47,    49,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    67,     0,
      28,    29,    30,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,    44,
      81,    82,    46,    83,    84,    86,    48,    88,    98,    50,
      51,    52,    77,    78,    79,    80,    55,    53,    54,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    68,
      69,    89,    91,    92,    93,    94,    95,    97,     0,     0,
       0,     0,    19,     0,     0,    17,     0,     0,     0,    13,
      14,    15,    16,    20,    22,     0,    85,    87,    99,     0,
       0,    18
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -58,   -58,   -58,   -58,    96,    79,   -58,   -58,    90,   -58,
     -58,   -58,   101,   -58,    97,   -58,   -58,   -58,   -58,   -58,
     -58,   -58,   -58,   -58,   -58,   -58,   -58,   -58,   -57,   -58,
     -10,   -41,   -43,   -13,    67
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     9,    10,    11,    78,    14,    15,    16,
      31,    60,     5,    20,    21,    61,    62,    63,    64,    65,
      66,    67,    68,     6,    22,    23,    24,    96,    82,    83,
      84,    33,    34,    35,    89
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      85,     7,   110,     1,    88,    36,    37,    39,    13,    80,
      81,    17,   106,    18,    19,   107,    85,    88,    12,   123,
      42,    43,    44,    45,    46,    47,     2,     8,    32,     2,
      13,    86,   117,    85,    92,    93,    94,    95,    40,    90,
      91,     8,    48,    76,   108,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,   118,   119,    80,    81,
      69,    70,    71,    72,    73,    74,   124,    26,   125,    97,
      98,    99,   100,    27,    28,    32,   120,    32,    79,   121,
      77,    75,   122,    85,   137,    87,   104,   138,   101,   127,
     102,   126,   103,    69,   111,   112,   105,   113,   114,   115,
     116,   135,   129,    25,    29,   128,    41,   134,   130,   131,
     132,   133,   139,   140,   141,    30,   136,    38,     0,     0,
       0,     0,     0,     0,     0,   109
};

static const yytype_int16 yycheck[] =
{
      43,     4,    59,     3,    45,    18,    19,    16,     5,    24,
      25,    27,    55,    29,    30,    56,    59,    58,     0,    76,
       6,     7,     8,     9,    10,    11,    26,    44,    43,    26,
       5,    44,    75,    76,    12,    13,    14,    15,    47,    22,
      23,    44,    28,    47,    57,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    22,    23,    24,    25,
      16,    17,    18,    19,    20,    21,    46,    43,    48,    22,
      23,    22,    23,    46,    43,    43,    42,    43,    42,    45,
      43,    46,    48,   126,   127,    43,    43,   128,    42,    49,
      42,    46,    42,    16,    42,    42,    45,    42,    42,    42,
      42,    16,    48,     7,    14,    46,    27,    43,    48,    48,
      48,    48,    47,    43,    48,    14,   126,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    26,    51,    52,    62,    73,     4,    44,    53,
      54,    55,     0,     5,    57,    58,    59,    27,    29,    30,
      63,    64,    74,    75,    76,    54,    43,    46,    43,    58,
      62,    60,    43,    81,    82,    83,    83,    83,    64,    16,
      47,    55,     6,     7,     8,     9,    10,    11,    28,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      61,    65,    66,    67,    68,    69,    70,    71,    72,    16,
      17,    18,    19,    20,    21,    46,    47,    43,    56,    42,
      24,    25,    78,    79,    80,    82,    83,    43,    81,    84,
      22,    23,    12,    13,    14,    15,    77,    22,    23,    22,
      23,    42,    42,    42,    43,    45,    82,    81,    83,    84,
      78,    42,    42,    42,    42,    42,    42,    82,    22,    23,
      42,    45,    48,    78,    46,    48,    46,    49,    46,    48,
      48,    48,    48,    48,    43,    16,    80,    82,    81,    47,
      43,    48
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    51,    51,    51,    52,    52,    53,    53,
      54,    54,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    56,    56,    57,    57,    58,    59,    60,    60,    61,
      61,    61,    61,    61,    61,    61,    61,    62,    62,    63,
      63,    64,    64,    64,    65,    66,    66,    67,    67,    68,
      68,    69,    69,    70,    70,    71,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      73,    74,    74,    75,    75,    76,    76,    77,    77,    77,
      77,    78,    78,    78,    79,    79,    80,    80,    81,    81,
      82,    82,    82,    82,    82,    82,    83,    83,    84,    84
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     3,     0,     2,     1,     2,
       1,     3,     2,     6,     6,     6,     6,     5,     9,     5,
       6,     1,     3,     1,     2,     2,     2,     0,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       2,     1,     1,     1,     2,     1,     2,     1,     2,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     2,     1,     2,     2,
       1,     1,     2,     1,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     3,     3,     3,     3,     3,     1,     3,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
        case 12:
#line 282 "cudf.y" /* yacc.c:1646  */
    { CUDFProperty *prop = new CUDFProperty((yyvsp[-1].str), gettype((yyvsp[0].str))); properties[string((yyvsp[-1].str))] = prop; required_properties.push_back(prop);  }
#line 1601 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 283 "cudf.y" /* yacc.c:1646  */
    { if (gettype((yyvsp[-4].str)) == pt_bool) 
	                                    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_bool, 1); 
	                                  else {
					    fprintf(stderr, "CUDF error (line %d): property value requires a boolean typed property (%s).\n", cudflineno, (yyvsp[-4].str));
					    exit(-1);              
					  }
                                        }
#line 1613 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 290 "cudf.y" /* yacc.c:1646  */
    { if (gettype((yyvsp[-4].str)) == pt_bool) 
	                                    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_bool, 0); 
	                                  else {
					    fprintf(stderr, "CUDF error (line %d): property value requires a boolean typed property (%s).\n", cudflineno, (yyvsp[-4].str));
					    exit(-1);              
					  }
                                        }
#line 1625 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 297 "cudf.y" /* yacc.c:1646  */
    {  CUDFPropertyType pt = gettype((yyvsp[-4].str));
	                                     if (pt == pt_int)  
					       properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_int, atoi((yyvsp[-1].str))); 
					     else if (pt == pt_posint)  
					       properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_posint, atoi((yyvsp[-1].str))); 
					     else if (pt == pt_nat)  
					       properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_nat, atoi((yyvsp[-1].str))); 
					     else {
					       fprintf(stderr, "CUDF error (line %d): property value requires an integer typed property (%s).\n", cudflineno, (yyvsp[-4].str));
					       exit(-1);              
					     }
                                           }
#line 1642 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 309 "cudf.y" /* yacc.c:1646  */
    {  if (gettype((yyvsp[-4].str)) == pt_string) 
	                                      properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_string, astring); 
	                                    else {
					      fprintf(stderr, "CUDF error (line %d): property value requires a string typed property (%s).\n", cudflineno, (yyvsp[-4].str));
					      exit(-1);              
					    }
                                          }
#line 1654 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 317 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFProperty *prop = new CUDFProperty((yyvsp[-4].str), pt_enum, enuml); 
  if (gettype((yyvsp[-3].str)) != pt_enum) {
    fprintf(stderr, "CUDF error (line %d): this must be an enum type (%s).\n", cudflineno, (yyvsp[-3].str));
    exit(-1);              
  }
  properties[string((yyvsp[-4].str))] = prop; 
  required_properties.push_back(prop); 
}
#line 1668 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 327 "cudf.y" /* yacc.c:1646  */
    { 
  if (gettype((yyvsp[-7].str)) == pt_enum) 
    properties[string((yyvsp[-8].str))] = new CUDFProperty((yyvsp[-8].str), pt_enum, enuml, (yyvsp[-1].str)); 
  else {
    fprintf(stderr, "CUDF error (line %d): property value requires an enum type (%s).\n", cudflineno, (yyvsp[-7].str));
    exit(-1);              
  }
}
#line 1681 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 342 "cudf.y" /* yacc.c:1646  */
    {  // No type checking is done here ...
  switch(gettype((yyvsp[-3].str))) {
  case pt_vpkglist:
    properties[string((yyvsp[-4].str))] = new CUDFProperty((yyvsp[-4].str), pt_vpkglist, new CUDFVpkgList); 
    break;
  case pt_veqpkglist:
    properties[string((yyvsp[-4].str))] = new CUDFProperty((yyvsp[-4].str), pt_veqpkglist, new CUDFVpkgList); 
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): property value requires a default value (%s).\n", cudflineno, (yyvsp[-3].str));
    exit(-1);              
  }
}
#line 1699 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 356 "cudf.y" /* yacc.c:1646  */
    {  // No type checking is done here ...
  CUDFPropertyType pt = gettype((yyvsp[-4].str));
  if (pt == pt_vpkg) {
    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_vpkg, current_vpkgformula[0][0]); 
  } else if (pt == pt_veqpkg)  
    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_veqpkg, current_vpkgformula[0][0]); 
  else if (pt == pt_vpkglist)  
    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_vpkglist, current_vpkgformula->front()); 
  else if (pt == pt_veqpkglist)  
    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_veqpkglist, current_vpkgformula->front()); 
  else if (pt == pt_vpkgformula)  
    properties[string((yyvsp[-5].str))] = new CUDFProperty((yyvsp[-5].str), pt_vpkgformula, current_vpkgformula); 
  else {
    fprintf(stderr, "CUDF error (line %d): property value requires a (sub)vpkgformula typed property (%s).\n", cudflineno, (yyvsp[-4].str));
    exit(-1);              
  }
  current_vpkgformula = (CUDFVpkgFormula *)NULL; 
}
#line 1722 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 376 "cudf.y" /* yacc.c:1646  */
    { enuml = new CUDFEnums; build_enum((yyvsp[0].str)); }
#line 1728 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 377 "cudf.y" /* yacc.c:1646  */
    { build_enum((yyvsp[0].str)); }
#line 1734 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 393 "cudf.y" /* yacc.c:1646  */
    { 
  package_postprocess();
  package_line = cudflineno;
  CUDFVirtualPackage *virtual_package = get_virtual_package((yyvsp[0].str));
  current_package = new CUDFVersionedPackage((yyvsp[0].str), versioned_package_rank++);
  all_packages.push_back(current_package);
  current_package->virtual_package = virtual_package;
}
#line 1747 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 418 "cudf.y" /* yacc.c:1646  */
    {  current_package->set_version(getversion((yyvsp[0].str))); }
#line 1753 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 420 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_package->depends == (CUDFVpkgFormula *)NULL) {
    current_package->depends = current_vpkgformula; 
    current_vpkgformula = (CUDFVpkgFormula *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): depends declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}
#line 1767 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 430 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_package->conflicts == (CUDFVpkgList *)NULL) {
    current_package->conflicts = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): conflicts declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}
#line 1781 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 440 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_package->provides == (CUDFVpkgList *)NULL) {
    current_package->provides = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): provides declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}
#line 1795 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 450 "cudf.y" /* yacc.c:1646  */
    { current_package->installed = true; }
#line 1801 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 452 "cudf.y" /* yacc.c:1646  */
    { current_package->installed = false; }
#line 1807 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 454 "cudf.y" /* yacc.c:1646  */
    { current_package->wasinstalled = true; }
#line 1813 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 456 "cudf.y" /* yacc.c:1646  */
    { current_package->wasinstalled = false; }
#line 1819 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 463 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_bool:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, 1));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (true) for property %s.\n", cudflineno, (yyvsp[-1].str));
      exit(-1);
  }
}
#line 1839 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 479 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_bool:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, 0));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (false) for property %s.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 1859 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 495 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    print_properties(stdout, &properties);
    exit(-1);
  }
  int value = atoi((yyvsp[0].str));
  switch (p->second->type_id) {
  case pt_int:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%d) for property %s.\n", cudflineno, value, (yyvsp[-1].str));
      exit(-1);
  }
}
#line 1881 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 513 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    print_properties(stdout, &properties);
    exit(-1);
  }
  int value = atoi((yyvsp[0].str));
  switch (p->second->type_id) {
  case pt_nat:
    if (value < 0) {
      fprintf(stderr, "CUDF error (line %d): property %s (nat) requires values >= 0: %d.\n", cudflineno, (yyvsp[-1].str), value);
      exit(-1);
    }
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%d) for property %s.\n", cudflineno, value, (yyvsp[-1].str));
      exit(-1);
  }
}
#line 1907 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 535 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    print_properties(stdout, &properties);
    exit(-1);
  }
  int value = atoi((yyvsp[0].str));
  switch (p->second->type_id) {
  case pt_posint:
    if (value <= 0) {
      fprintf(stderr, "CUDF error (line %d): property %s (posint) requires values > 0: %d.\n", cudflineno, (yyvsp[-1].str), value);
      exit(-1);
    }
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%d) for property %s.\n", cudflineno, value, (yyvsp[-1].str));
      exit(-1);
  }
}
#line 1933 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 557 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  char *value;
  switch (p->second->type_id) {
  case pt_enum:
    value = get_enum(p->second->enuml, (yyvsp[0].str));
    if (value == (char *)NULL) {
      fprintf(stderr, "CUDF error (line %d): property %s (enum) can not take value %s.\n", cudflineno, (yyvsp[-1].str), (yyvsp[0].str));
      exit(-1);
    }
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%s) for property %s.\n", cudflineno, (yyvsp[0].str), (yyvsp[-1].str));
    exit(-1);
  }
}
#line 1959 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 579 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_string:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, astring));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%s) for property %s.\n", cudflineno, astring, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 1979 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 595 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_vpkg:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, (yyvsp[-1].str)));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value for property %s.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 1999 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 611 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_veqpkg:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, (yyvsp[-1].str)));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value for property %s.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 2019 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 627 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_vpkglist:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, current_vpkglist));
    current_vpkglist = (CUDFVpkgList *)NULL; 
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value for property %s.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 2040 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 644 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_veqpkglist:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, current_vpkglist));
    current_vpkglist = (CUDFVpkgList *)NULL; 
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value for property %s.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 2061 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 661 "cudf.y" /* yacc.c:1646  */
    { 
  CUDFPropertiesIterator p = properties.find(string((yyvsp[-1].str)));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_vpkgformula:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, current_vpkgformula));
    current_vpkgformula = (CUDFVpkgFormula *)NULL; 
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value for property %s.\n", cudflineno, (yyvsp[-1].str));
    exit(-1);
  }
}
#line 2082 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 679 "cudf.y" /* yacc.c:1646  */
    { 
  package_postprocess();
  if (the_problem == (CUDFproblem *)NULL)
    the_problem = current_problem = new CUDFproblem(); 
}
#line 2092 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 685 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_problem->install == (CUDFVpkgList *)NULL) {
    current_problem->install = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): install declared twice.\n", cudflineno);
    exit(-1);
  }
}
#line 2106 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 695 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_problem->remove == (CUDFVpkgList *)NULL) {
    current_problem->remove = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): remove declared twice.\n", cudflineno);
    exit(-1);
  }
}
#line 2120 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 705 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_problem->upgrade == (CUDFVpkgList *)NULL) {
    current_problem->upgrade = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): upgrade declared twice.\n", cudflineno);
    exit(-1);
  }
}
#line 2134 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 715 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_package->keep == keep_none) {
    current_package->keep = keep_version;
  } else {
    fprintf(stderr, "CUDF error (line %d): keep declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}
#line 2147 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 724 "cudf.y" /* yacc.c:1646  */
    { 
  if (current_package->keep == keep_none) {
    current_package->keep = keep_package;
  } else {
    fprintf(stderr, "CUDF error (line %d): keep declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}
#line 2160 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 733 "cudf.y" /* yacc.c:1646  */
    {
  if (current_package->keep == keep_none) {
    current_package->keep = keep_version;
  } else {
    fprintf(stderr, "CUDF error (line %d): keep declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}
#line 2173 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 742 "cudf.y" /* yacc.c:1646  */
    {
  current_package->keep = keep_none;
}
#line 2181 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 747 "cudf.y" /* yacc.c:1646  */
    { build_vpkglist(vpkg_true); build_vpkgformula(); }
#line 2187 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 748 "cudf.y" /* yacc.c:1646  */
    { build_vpkglist(vpkg_false); build_vpkgformula(); }
#line 2193 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 751 "cudf.y" /* yacc.c:1646  */
    { build_vpkgformula(); }
#line 2199 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 752 "cudf.y" /* yacc.c:1646  */
    { build_vpkgformula(); }
#line 2205 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 754 "cudf.y" /* yacc.c:1646  */
    { build_vpkglist((yyvsp[0].avpkg)); }
#line 2211 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 755 "cudf.y" /* yacc.c:1646  */
    { build_vpkglist((yyvsp[0].avpkg)); }
#line 2217 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 757 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[0].str), op_none, 0); }
#line 2223 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 758 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[-2].str), op_eq, getversion((yyvsp[0].str))); }
#line 2229 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 760 "cudf.y" /* yacc.c:1646  */
    { (yyval.avpkg) = (yyvsp[0].avpkg); }
#line 2235 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 761 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[-2].str), op_neq, getversion((yyvsp[0].str))); }
#line 2241 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 762 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[-2].str), op_sup, getversion((yyvsp[0].str))); }
#line 2247 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 763 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[-2].str), op_supeq, getversion((yyvsp[0].str))); }
#line 2253 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 764 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[-2].str), op_inf, getversion((yyvsp[0].str))); }
#line 2259 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 765 "cudf.y" /* yacc.c:1646  */
    { current_vpkg = (yyval.avpkg) = build_vpkg((yyvsp[-2].str), op_infeq, getversion((yyvsp[0].str)));}
#line 2265 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 768 "cudf.y" /* yacc.c:1646  */
    { build_vpkglist((yyvsp[0].avpkg)); }
#line 2271 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 769 "cudf.y" /* yacc.c:1646  */
    { build_vpkglist((yyvsp[0].avpkg)); }
#line 2277 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 771 "cudf.y" /* yacc.c:1646  */
    { build_veqpkglist((yyvsp[0].avpkg)); }
#line 2283 "cudf.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 772 "cudf.y" /* yacc.c:1646  */
    { build_veqpkglist((yyvsp[0].avpkg)); }
#line 2289 "cudf.tab.c" /* yacc.c:1646  */
    break;


#line 2293 "cudf.tab.c" /* yacc.c:1646  */
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
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
  return yyresult;
}
#line 776 "cudf.y" /* yacc.c:1906  */


extern FILE *cudfin;

// main parser function
int parse_cudf(FILE *input_file) {
  int retval;

  //  cudfdebug = 1; 
  cudfin = input_file; 
  retval = cudfparse(); 
  if (the_problem == (CUDFproblem *)NULL)
    the_problem = current_problem = new CUDFproblem(); 
  the_problem->properties = &properties;
  the_problem->all_packages = &all_packages;
  the_problem->installed_packages = &installed_packages;
  the_problem->uninstalled_packages = &uninstalled_packages;
  the_problem->all_virtual_packages = &all_virtual_packages;

  return retval;
}

