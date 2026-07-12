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
#include <string.h>
#include "maxsci1.h"

extern int maxinit (void);

int
sci_maxinit (fname, _pvApiCtx)
     char *fname;
     void *_pvApiCtx;
{
  int m, n, path;

  pvApiCtx = _pvApiCtx;
  CheckRhs (0, 0) ;
  /* macOS/2027 port (Task 12): the demo/docs call this bare ("maxinit()",
     0 outputs, matching LhsVar(1)=0 below); CheckLhs(1,1) rejected that
     with "Wrong number of output argument(s): 1 expected" (confirmed by
     the smoke test), so widen to allow 0. */
  CheckLhs (0, 1) ;

  LhsVar (1) = 0;
  return maxinit ();
}
