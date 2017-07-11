/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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

#ifndef YY_CUDF_CUDF_TAB_H_INCLUDED
# define YY_CUDF_CUDF_TAB_H_INCLUDED
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
#line 245 "cudf.y" /* yacc.c:1909  */

  //  CUDFVersion value;
  CUDFVpkg *avpkg;
  char str[256];

#line 106 "cudf.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE cudflval;

int cudfparse (void);

#endif /* !YY_CUDF_CUDF_TAB_H_INCLUDED  */
