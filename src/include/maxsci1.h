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

#include <stdio.h>
#include <string.h>
/* macOS/2027 port (Task 12): Scierror()/sciprint() used to come in
   transitively via stack-c.h (removed from core in 2015); every caller in
   this toolbox relies on maxsci1.h for its other shared globals, so declare
   them once here rather than in each of the ~18 files that call them. */
#include "sciprint.h"
#include "Scierror.h"

//#define DBG 1
#ifdef DBG
#define VIDEOS (fgets(buf,BUFSIZE,os),fprintf(stdout,"%s",buf))
#else
#define VIDEOS fgets(buf,BUFSIZE,os)
#endif

#define isbd(ch) (*((int*)ch)==*((int*)"<BD>"))
#define isbo(ch) (*((int*)ch)==*((int*)"<BO>"))
#define iseo(ch) (*((int*)ch)==*((int*)"<EO>"))
#define isbe(ch) (*((int*)ch)==*((int*)"<BE>"))
#define isee(ch) (*((int*)ch)==*((int*)"<EE>"))
#define isbs(ch) (*((int*)ch)==*((int*)"<BS>"))
#define isbc(ch) (*((int*)ch)==*((int*)"<BC>"))
#define isbq(ch) (*((int*)ch)==*((int*)"<BQ>"))
#define iseq(ch) (*((int*)ch)==*((int*)"<EQ>"))

#ifndef _MSC_VER
#define Putc(_ch,_fp) putc_unlocked(_ch,_fp)
#define Getc(_fp) getc_unlocked(_fp)
#else
#define Putc(_ch,_fp) putc(_ch,_fp)
#define Getc(_fp) getc(_fp)
#endif

#define BUFSIZE 256

#ifdef GLOBAL
unsigned char max_is_ok=0;

#ifndef _MSC_VER
pid_t pid;
#endif
FILE *is,*os;
char buf[BUFSIZE];
unsigned char quest_mode=0;
void *pvApiCtx=NULL;
#else
extern unsigned char max_is_ok;
#ifndef _MSC_VER
extern pid_t pid;
#endif
extern FILE *is,*os,*ds;
extern char buf[BUFSIZE];
extern unsigned char quest_mode;
extern void *pvApiCtx;
#endif

#define SD "c"
#define SMD "S"
#define L "l"
#define ML "M"

static int zero=0,un=1,deux=2,trois=3,quatre=4,cinq=5;
const static char* tabS[3]={"sym","t","rep"};
const static char* M = "M";

/* macOS/2027 port note (Task 12): the pre-2011 raw flat-stack API this
 * toolbox was written against (Lstk/istk/cstk/GetRhsVar/CreateVar, and the
 * old core/includes/stack-c.h + stack1.h/2.h/3.h that declared them) was
 * removed from Scilab core in commit ab2ff6e836a (2015). There is no direct
 * replacement: gestionVar()/creerSym()/recupResult() in gestionVar.c and
 * donnees.c were rewritten from scratch against the modern api_scilab
 * position-based accessors instead of walking raw stack memory.
 *
 * Every remaining call site in this toolbox that references Top/Lstk does so
 * only to mean "the position of the i-th already-read input argument" (never
 * to grow the stack mid-call before reading it), and for every one of those
 * call sites Top is used interchangeably with Rhs at function entry. Top is
 * therefore simply aliased to Rhs (itself an api_stack_common.h macro), and
 * Lstk(i) is repurposed to just hand back the position index i itself
 * (rather than a raw memory address) so existing call sites like
 * "lr = *Lstk(i); gestionVar(lr);" keep working unchanged against the new
 * position-based gestionVar().
 */
#define Top Rhs
static int __sm_lstk_scratch;
static int *Lstk(int i) { __sm_lstk_scratch = i; return &__sm_lstk_scratch; }

/* Most api_scilab accessors/constructors return a SciErr struct (whose
   .iErr field is nonzero on failure), not a plain int -- this collapses
   that to a plain truthy int so call sites can chain checks with ||/&&,
   e.g. "if (__sm_err(getVarType(...)) || __sm_err(getMatrixOfDouble(...)))". */
static int __sm_err(SciErr e) { return e.iErr != 0; }
