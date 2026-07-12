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

//   macOS/2027 port (Task 12): defmf() is out of scope for this port -- not
//   needed by the toolbox's smoke test (a plain maxevalf() round-trip) and
//   its caller (sci_defmf.c) leaned on more raw pre-2011 flat-stack
//   primitives (CreateVarFromPtr/SciString) than were worth reimplementing
//   against modern api_scilab within this task's time-box. Documented gap:
//   see docs/design/toolbox-verification.md.

int
defmf (maxname, maxcode, m, n, scicode, sciname, scifun)
     char *maxname, *maxcode, **scicode, *sciname, **scifun;
     int m, n;
{
  (void) maxname;
  (void) maxcode;
  (void) m;
  (void) n;
  (void) sciname;
  *scicode = (char *) 0;
  *scifun = (char *) 0;
  return -1;
}
