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

//   macOS/2027 port (Task 12): the original walked a Scilab `list(...)`
//   argument element-by-element via C2F(getilist), a raw pre-2011
//   flat-stack primitive removed from Scilab core in 2015 (see
//   gestionVar.c/donnees.c for the general background, and maxevalf.c for
//   the equivalent non-list entry point this port DOES support). maxevalfl()
//   is out of scope for this port -- not needed by the toolbox's smoke test.
//   Documented gap: see docs/design/toolbox-verification.md.

int
maxevalfl (pos, stri, m)
     int pos, m;
     char *stri;
{
  (void) pos;
  (void) stri;
  (void) m;
  return -1;
}
