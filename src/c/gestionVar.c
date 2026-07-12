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

//   macOS/2027 port (Task 12): gestionVar() used to walk the raw pre-2011
//   flat-stack memory layout directly (istk/stk header decoding, including a
//   hand-rolled MLIST header offset table for the "sym" type and a custom
//   character-encoding table for strings). That raw layout and the API that
//   exposed it (stack-c.h/stack1.h/2.h/3.h) no longer exist in this Scilab
//   build (removed 2015, commit ab2ff6e836a). This is a from-scratch
//   reimplementation of the SAME wire behaviour (send the Scilab value at
//   input position `pos` to the Maxima subprocess as Maxima-syntax text on
//   `is`) against the modern api_scilab position-based accessors.
//
//   Supported types: real (double) matrices, strings (spliced in as raw,
//   unquoted Maxima syntax -- this matches the original behaviour, e.g. how
//   maxevalf('diff', x1, ...) passes 'diff' as a bare Maxima function name),
//   and this toolbox's own "sym" mlist (mlist(['sym','t','rep'], t, rep) --
//   only the 'rep' field, itself already Maxima-syntax text, is spliced in).
//   Complex/int8-32/polynomial matrices are NOT supported by this port
//   (documented gap -- see docs/design/toolbox-verification.md); as with the
//   original code's own "type not managed" fallback, gestionVar() simply
//   returns -1 for them so callers report a clean error instead of
//   mis-marshaling.

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#define __USE_DEPRECATED_STACK_FUNCTIONS__ 1
#include "api_scilab.h"
#include "sci_types.h"
#include "maxsci1.h"
#include "maxsci.h"

int gestionVar (int);

#define printRealMat(type, mat, m, n, str1, str2, str3, str4)	\
  mn = m * n;							\
  								\
  if (mn == 1)							\
    {								\
      fprintf (is, str1, ((type)mat)[0]);			\
      break;							\
    }								\
  								\
  fprintf (is, "(Matrix(");					\
  								\
  for (i = 0; i < m - 1; i++)					\
    {								\
      Putc ('[', is);						\
      for (j = 0; j < n - 1; j++)				\
	{							\
	  fprintf (is, str2, ((type)mat)[j * m + i]);		\
	}							\
      fprintf (is, str3, ((type)mat)[(n - 1) * m + i]);		\
    }								\
  Putc ('[', is);						\
  for (j = 0; j < n - 1; j++)					\
    {								\
      fprintf (is, str2, ((type)mat)[j * m + m - 1]);		\
    }								\
  fprintf (is, str4, ((type)mat)[mn - 1])

int
gestionVar (int pos)
{
  int *piAddr = NULL;
  int type = -1;

  if (pos == 0)
    return -1;

  if (__sm_err (getVarAddressFromPosition (pvApiCtx, pos, &piAddr)) || piAddr == NULL)
    return -1;
  if (__sm_err (getVarType (pvApiCtx, piAddr, &type)))
    return -1;

  switch (type)
    {
    case sci_matrix:
      {
	int m = 0, n = 0, mn;
	double *data = NULL;
	register int i, j;

	if (__sm_err (getMatrixOfDouble (pvApiCtx, piAddr, &m, &n, &data)))
	  return -1;
	do
	  {
	    printRealMat (double*, data, m, n, "(%.17g)", "%.17g,", "%.17g],", "%.17g]))");
	  }
	while (0);
	return 0;
      }
    case sci_strings:
      {
	char *str = NULL;

	if (getAllocatedSingleString (pvApiCtx, piAddr, &str))
	  return -1;
	fprintf (is, "(%s)", str);
	freeAllocatedSingleString (str);
	return 0;
      }
    case sci_mlist:
      {
	/* Duck-typed: this toolbox is the only thing that puts mlists on
	   the stack, and its only mlist shape is sym = ['sym','t','rep'].
	   Item 3 ('rep') already holds Maxima-syntax text -- splice it in
	   verbatim, exactly like the original raw-header version did. */
	int *pRep = NULL;
	int nbItem = 0;
	char *rep = NULL;

	if (__sm_err (getListItemNumber (pvApiCtx, piAddr, &nbItem)) || nbItem < 3)
	  return -1;
	if (__sm_err (getListItemAddress (pvApiCtx, piAddr, 3, &pRep)))
	  return -1;
	if (getAllocatedSingleString (pvApiCtx, pRep, &rep))
	  return -1;
	fprintf (is, "(%s)", rep);
	freeAllocatedSingleString (rep);
	return 0;
      }
    default:
      return -1;
    }
}
