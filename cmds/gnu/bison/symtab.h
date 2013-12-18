/* Definitions for symtab.c and callers, part of bison,
   Copyright (C) 1984 Bob Corbett and Free Software Foundation, Inc.

BISON is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the BISON General Public License for full details.

Everyone is granted permission to copy, modify and redistribute BISON,
but only under the conditions described in the BISON General Public
License.  A copy of this license is supposed to have been given to you
along with BISON so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  

 $Header: /hsrc/cmds/gnu/bison/RCS/symtab.h,v 1.1 1990/08/28 11:39:40 james Exp $ */

#define	TABSIZE	1009


/*  symbol classes  */

#define SUNKNOWN 0
#define STOKEN	 1
#define SNTERM	 2


typedef
  struct bucket
    {
      struct bucket *link;
      struct bucket *next;
      char *tag;
      char *type_name;
      short value;
      short prec;
      short assoc;
      short user_token_number;
      char class;
    }
  bucket;


extern bucket **symtab;
extern bucket *firstsymbol;

extern bucket *getsym();
