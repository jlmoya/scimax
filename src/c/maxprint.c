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
#include <stdio.h>
#include "maxsci1.h"
#include "maxsci.h"

// macOS/2027 port (Task 12): getColumnsSize() (console-width query) is
// another pre-2011 console API this build no longer ships (adv-cli/-nb runs
// headless anyway, so there is no real terminal width to query). Maxima's
// "linel" (line length) just controls its own pretty-printing wrap column;
// a fixed reasonable width is a fine substitute.
#define MAXCOLS 80

extern int detecteErreurs (void);
extern void envoiDonnees (void);
extern void CANCEL (void);
extern int gestionVar (int);

int maxprint (void)
{
  int n;

  G_nb.vars = 0;
  G_nb.appels = 0;

  fprintf (is, "linel:%i$___(", MAXCOLS);

  n = gestionVar (*Lstk (Top));

  if (n == -1)
    {
      CANCEL ();
      return -1;
    }
  
  Putc (')', is);
  Putc ('$', is);
  Putc ('\n', is);
  fflush (is);

  n = detecteErreurs ();
  if (n == -1 || n == 1)
    return n;
  
  while ((VIDEOS, iseo (buf) == 0 && !isbs (buf))) /* isbs: EOF escape (maxsci1.h) */
   if(!isbo(buf) )  sciprint ("%s", buf);
  
  return 0;
}
