//    <SciMax, a Scilab toolbox to connect Maxima.>
//    Copyright (C) <2009>  <Calixte DENIZET>

//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.

//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.


//   Contact : Calixte DENIZET <calixte.denizet@ac-rennes.fr>

//   macOS/2027 port (Task 12): see gestionVar.c/donnees.c for the general
//   background (raw flat-stack API removed from core in 2015). This file's
//   own raw-memory use was determineOp()'s scalar-vs-matrix size probe
//   (istk(iadr(*Lstk(...)))), reimplemented below via api_scilab dimension
//   queries. C2F(intersci) bookkeeping was dropped -- write-only, never read
//   back anywhere in this toolbox.

#define __USE_DEPRECATED_STACK_FUNCTIONS__ 1
#include "api_scilab.h"
#include "sci_types.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "maxsci1.h"
#include "maxsci.h"
#include "operateurs.h"

extern int gestionVar (int);
extern void envoiDonnees (void);
extern void CANCEL (void);
extern int recupResult (int);

/* macOS/2027 port (Task 12): bare C99 "inline" (no "static") does not
 * guarantee an out-of-line definition gets emitted anywhere -- if the single
 * call site below isn't actually inlined (e.g. no optimization), the call
 * is left referencing an external "_determineOp" symbol that was never
 * defined, and addinter's dlopen() fails at load time with "symbol not
 * found in flat namespace" (confirmed by isolated reproduction). "static"
 * sidesteps the whole ambiguity: always a real, locally-linked function.
 */
#define INLINE static

INLINE void determineOp (char);
static int sm_opSize (int);

int
maxevalop (pos, stri)
     int pos;
     char *stri;
{
  int k, lr;

  G_nb.vars = 0;
  G_nb.appels = 0;

  Putc ('_', is);
  Putc ('(', is);

  lr = *Lstk (Top - 1);
  k = gestionVar (lr);

  if (k == -1)
    {
      CANCEL ();
      Scierror (9999, "The type of the variable 1 is not managed by SciMax\r\n");
      return -1;
    }

  determineOp (stri[9] - 65);

  lr = *Lstk (Top);
  k = gestionVar (lr);

  if (k == -1)
    {
      CANCEL ();
      Scierror (9999, "The type of the variable 2 is not managed by SciMax\r\n");
      return -1;
    }

  Putc (')', is);
  Putc ('$', is);
  Putc ('\n', is);
  fflush (is);

  return recupResult (pos);
}

/* Element count of the operand at input position `pos`: for this toolbox's
   own "sym" mlist, the size of its 'rep' field (a scalar sym has a 1x1 rep);
   otherwise the size of the value itself. Replaces the original's raw
   istk(iadr(...)) header walk -- same "is this a scalar (size 1) or a
   matrix" question, asked via api_scilab instead of raw memory. */
static int
sm_opSize (int pos)
{
  int *piAddr = NULL;
  int type = -1;
  int m = 0, n = 0;

  if (__sm_err (getVarAddressFromPosition (pvApiCtx, pos, &piAddr)) || piAddr == NULL)
    return 1;
  if (__sm_err (getVarType (pvApiCtx, piAddr, &type)))
    return 1;
  if (type == sci_mlist)
    {
      int *pRep = NULL;
      if (__sm_err (getListItemAddress (pvApiCtx, piAddr, 3, &pRep)) || __sm_err (getVarDimension (pvApiCtx, pRep, &m, &n)))
	return 1;
      return m * n;
    }
  if (__sm_err (getVarDimension (pvApiCtx, piAddr, &m, &n)))
    return 1;
  return m * n;
}

INLINE void
determineOp (op)
     char op;
{
  int a = sm_opSize (Top - 1) - 1;
  int b = sm_opSize (Top) - 1;

  switch (op)
    {
    case __ADD :
      Putc ('+', is);
      return;
    case __POW :
      Putc ('^', is);
      if (a) Putc ('^', is);
      return;
    case __MULT :
      if (a && b) Putc ('.', is);
      else Putc ('*', is);
      return;
    case __DIV :
      if (b)
	{
	  Putc (!a ? '*' : '.', is);
	  Putc ('i', is);
	  Putc ('n', is);
	  Putc ('v', is);
	}
      else Putc ('/', is);
      return;
    case __SUB :
      Putc ('-', is);
      return;
    case __BACKSLASH :
      if (a)
	{
	  Putc ('v', is);
	  Putc ('n', is);
	  Putc ('i', is);
	  Putc (!b ? '*' : '.', is);
	}
      else
	{
	  Putc ('_', is);
	  Putc ('b', is);
	  Putc ('_', is);
	}
      return;
    case __PMULT :
      Putc ('*', is);
      return;
    case __PDIV :
      Putc ('/', is);
      return;
    case __PBACKSLASH :
      Putc ('_', is);
      Putc ('b', is);
      Putc ('_', is);
      return;
    case __PPOW :
      if (!b) Putc ('^', is);
      else
	{
	  Putc ('_', is);
	  Putc ('p', is);
	  Putc ('_', is);
	}
      return;
    case __FEEDBACK :
      Putc ('_', is);
      Putc ('f', is);
      Putc ('_', is);
      return;
    case __KRONPROD :
      Putc ('_', is);
      Putc ('k', is);
      Putc ('_', is);
      return;
    case __LOW :
      Putc ('<', is);
      return;
    case __LEQ :
      Putc ('<', is);
      Putc ('=', is);
      return;
    case __GREAT :
      Putc ('>', is);
      return;
    case __GEQ :
      Putc ('>', is);
      Putc ('=', is);
      return;
    case __NEQ :
      Putc ('#', is);
      return;
    case __EQ :
      Putc ('=', is);
      return;
    case __AFFECT :
      Putc (':', is);
      return;
    }
}
