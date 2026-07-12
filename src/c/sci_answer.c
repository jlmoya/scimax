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

#define __USE_DEPRECATED_STACK_FUNCTIONS__ 1
#include "api_scilab.h"
#include "maxsci1.h"

extern int recupResult (int);

int
sci_answer (fname, _pvApiCtx)
     char *fname;
     void *_pvApiCtx;
{
  int *piAddr = NULL;
  char *answerStr = NULL;

  pvApiCtx = _pvApiCtx;
  if (max_is_ok == 0)
    {
      Scierror (9999, "Maxima has not been started : use maxinit\n");
      return -1;
    }
  if (quest_mode == 0)
    {
      Scierror (9999, "Maxima didn't ask you a question\n");
      return -1;
    }

  CheckLhs (1, 1);
  CheckRhs (1, 1);

  if (__sm_err (getVarAddressFromPosition (pvApiCtx, 1, &piAddr)) || getAllocatedSingleString (pvApiCtx, piAddr, &answerStr))
    {
      Scierror (9999, "SciMax: expecting a string answer\n");
      return -1;
    }

  fprintf (is, "%s$\n", answerStr);
  fflush (is);
  freeAllocatedSingleString (answerStr);

  if (recupResult (1) == 1)
    {
      LhsVar (1) = 0;
      return 0;
    }
  
  quest_mode = 0;

  LhsVar (1) = Rhs + 1;
  return 0;
}
  
