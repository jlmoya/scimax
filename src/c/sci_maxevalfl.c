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

//   macOS/2027 port (Task 12): the original read its args via
//   GetRhsVar(SD)/GetRhsVar(L) -- raw pre-2011 flat-stack primitives removed
//   from Scilab core in 2015 (see gestionVar.c/donnees.c for the general
//   background, and maxevalf.c/sci_maxevalf.c for the equivalent non-list
//   entry point this port DOES support). maxevalfl() is out of scope for
//   this port -- not needed by the toolbox's smoke test. Documented gap: see
//   docs/design/toolbox-verification.md.

#define __USE_DEPRECATED_STACK_FUNCTIONS__ 1
#include "api_scilab.h"
#include "maxsci1.h"

int
sci_maxevalfl (fname, _pvApiCtx)
     char *fname;
     void *_pvApiCtx;
{
  pvApiCtx = _pvApiCtx;
  Scierror (9999, "SciMax (macOS port): maxevalfl() is not supported in this build\r\n");
  return -1;
}
