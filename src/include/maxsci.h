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

#define SMFLOAT 1
#define SMFIXNUM8 2
#define SMFIXNUMU8 3
#define SMFIXNUM16 4
#define SMFIXNUMU16 5
#define SMFIXNUM32 6
#define SMFIXNUMU32 7
#define SMCOMPLEX 8
#define SMPOLYNOMF 9
#define SMPOLYNOMC 10
#define SMSTRING 11

#define MAXVARS 512

typedef struct {
  unsigned char type;
  long taille;
  int m;
  int n;
  int ptr;
} Info;

typedef struct {
  int vars;
  int appels; 
} Nb;

/* macOS/2027 port (Task 12): this header is included by ~10 .c files, each
 * of which used to get its OWN definition of these three globals -- legal
 * under old "common symbol" linking (-fcommon, historically clang/gcc's
 * default), but clang defaults to -fno-common now, which makes every one
 * of those extra definitions a hard "duplicate symbol" link error. Apply
 * the same single-definition-in-maxinit.c pattern maxsci1.h already uses
 * for its own globals (is/os/max_is_ok/...): maxinit.c #define GLOBAL
 * before including this header.
 */
#ifdef GLOBAL
Info G_tabvar[MAXVARS];
Nb G_nb;
void * G_tabptr[MAXVARS];
#else
extern Info G_tabvar[MAXVARS];
extern Nb G_nb;
extern void * G_tabptr[MAXVARS];
#endif
