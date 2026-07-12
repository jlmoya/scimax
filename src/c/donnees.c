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

//   macOS/2027 port (Task 12): creerSym()/creerSym2()/recupResult() used to
//   hand-poke a "sym" mlist directly into the raw pre-2011 flat-stack memory
//   (a precomputed header template `tabSym`, filled in with the Maxima
//   subprocess's response bytes translated through a custom encoding table
//   `taba2s`). That memory model and the create/read primitives it needed
//   (CreateVar/C2F(createdata)/stk/istk/Lstk) no longer exist in this Scilab
//   build (removed 2015, commit ab2ff6e836a) -- see gestionVar.c for the
//   send-direction rewrite and the same background.
//
//   This is a from-scratch reimplementation of creerSym (build a "sym" mlist
//   at the next free stack position, i.e. Rhs+1, from an already-decoded C
//   string) and recupResult (read the Maxima subprocess's response and turn
//   it into that mlist) against modern api_scilab, using createNamedMList
//   for Syms's name-injection case. The wire format itself is unchanged and
//   was re-derived from src/lisp/scimax.lisp's `$_` printer rather than from
//   the old C reader: a scalar (non-matrix/list/set) Maxima result is framed
//   as "<BO>E\n<total>\n<raw text, total-3 bytes>", where <raw text> is
//   printed verbatim (princ) with no escaping, followed by the interpreter's
//   own "\n<EO>\n" prompt (6 bytes) once it's ready for the next command.
//   Matrix/list/set Maxima results use a different, multi-line framing this
//   port does not implement (documented gap, see
//   docs/design/toolbox-verification.md); recupResult() reports a clean
//   error for that case instead of mis-parsing raw bytes as if they were the
//   scalar framing.

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define __USE_DEPRECATED_STACK_FUNCTIONS__ 1
#include "api_scilab.h"
#include "maxsci1.h"
#include "maxsci.h"

int creerSym (char *, char **, int, int, char);
int creerSymNamed (const char *, const char *, char);
int recupResult (int);
int detecteErreurs (void);
void gererQuestion (void);
extern void maxkill (void);

int
creerSym (char *stro, char **tabstro, int m, int n, char type)
{
  int *piAddr = NULL;
  int pos = Rhs + 1;
  char typeStr[2];
  const char *names[3];
  const char *typeP = typeStr;

  typeStr[0] = type;
  typeStr[1] = '\0';
  names[0] = tabS[0];
  names[1] = tabS[1];
  names[2] = tabS[2];

  if (__sm_err (createMList (pvApiCtx, pos, 3, &piAddr)))
    return -1;
  if (__sm_err (createMatrixOfStringInList (pvApiCtx, pos, piAddr, 1, 1, 3, (char**) names)))
    return -1;
  if (__sm_err (createMatrixOfStringInList (pvApiCtx, pos, piAddr, 2, 1, 1, (char**) &typeP)))
    return -1;

  if (tabstro == NULL)
    {
      const char *rp = (stro != NULL) ? stro : "";
      if (__sm_err (createMatrixOfStringInList (pvApiCtx, pos, piAddr, 3, 1, 1, (char**) &rp)))
	return -1;
    }
  else
    {
      if (__sm_err (createMatrixOfStringInList (pvApiCtx, pos, piAddr, 3, m, n, (char**) tabstro)))
	return -1;
    }
  return 0;
}

/* Used only by sci_Syms.c: create a "sym" whose rep is its own name, bound
   directly to that NAME in the caller's scope (the modern equivalent of the
   old CreateVar(pos,...)+PutVar(pos,name) pair -- this is what makes
   "Syms x y z" declare x, y and z directly rather than returning them). */
int
creerSymNamed (const char *name, const char *stro, char type)
{
  int *piAddr = NULL;
  char typeStr[2];
  const char *names[3];
  const char *typeP = typeStr;
  const char *rp = (stro != NULL) ? stro : "";

  typeStr[0] = type;
  typeStr[1] = '\0';
  names[0] = tabS[0];
  names[1] = tabS[1];
  names[2] = tabS[2];

  if (__sm_err (createNamedMList (pvApiCtx, name, 3, &piAddr)))
    return -1;
  if (__sm_err (createMatrixOfStringInNamedList (pvApiCtx, name, piAddr, 1, 1, 3, (char**) names)))
    return -1;
  if (__sm_err (createMatrixOfStringInNamedList (pvApiCtx, name, piAddr, 2, 1, 1, (char**) &typeP)))
    return -1;
  if (__sm_err (createMatrixOfStringInNamedList (pvApiCtx, name, piAddr, 3, 1, 1, (char**) &rp)))
    return -1;
  return 0;
}

int
recupResult (int pos)
{
  unsigned char tp;
  int total, len, k;
  char *result;

  k = detecteErreurs ();
  if (k == -1 || k == 1)
    return k;

  tp = buf[4];

  VIDEOS; total = atoi (buf);
  /* total==3 <=> the Maxima-side $_ printer's (length ch) was 0: an empty
     result. Mirrors the original's "read 2 more (blank) lines and bail". */
  if (total == 3)
    {
      VIDEOS;
      VIDEOS;
      return 1;
    }

  if (tp != 'E')
    {
      Scierror (9999, "SciMax (macOS port): matrix/list/set Maxima results are not supported by this build\r\n");
      maxkill ();
      return -1;
    }

  len = total - 3;
  result = malloc ((size_t) len + 1);
  if (result == NULL)
    {
      Scierror (9999, "SciMax: out of memory\r\n");
      return -1;
    }
  for (k = 0; k < len; k++)
    result[k] = (char) Getc (os);
  result[len] = '\0';

  /* drain the trailing "\n<EO>\n" prompt marker so the pipe is clean for
     the next command (unchanged wire framing, see main-prompt in
     scimax.lisp) */
  for (k = 0; k < 6; k++)
    Getc (os);

  k = creerSym (result, NULL, 1, 1, 'M');
  free (result);
  return k;
}

void CANCEL (void)
{
  Putc ('$', is);
  Putc ('\n', is);
  fflush (is);
  do VIDEOS;
  while (!iseo (buf));
}

int detecteErreurs (void)
{
  char a, b, c, d, e;

  do VIDEOS;
  while (!isbo (buf) && (a=!isbe (buf)) && (b=!isbq (buf)) && (c=!isbs (buf)) && (d=!isbc (buf)) && !iseo (buf) && (e=!isbd (buf)));

  if (!a)
    {
      Scierror (9999, "Maxima error :\n");
      while ((VIDEOS, !isee (buf)))
	sciprint (buf);
      VIDEOS;
      return -1;
    }
  if (!b)
    {
      gererQuestion ();
      quest_mode = 1;
      return 1;
    }
  if (!c)
    {
      Scierror (9999, "A serious error occured\r\n");
      maxkill ();
      return -1;
    }
  if (!d)
    {
      sciprint ("Creating function in Maxima...\r\n");
      while ((VIDEOS, !isbo (buf)));
      return 0;
    }
  if (!e)
    {
      max_is_ok = 1;
      Scierror (9999, "\nMaxima started but it could not load scimax or scimath.\r\nGo to the directory src/lisp, read the README file and try to fix the problem.\r\nSend me an email to report the bug.\r\n\n");
      maxkill ();
      return -1;
    }
  return 0;
}

void gererQuestion (void)
{
  sciprint ("\n? Maxima Question\n");
  sciprint ("   * To answer use the function answer('your answer')\n");
  sciprint ("   * If you don't want to answer use the function noanswer\n\n");
  while ((VIDEOS, !iseq (buf)))
    sciprint("%s", buf);
}
