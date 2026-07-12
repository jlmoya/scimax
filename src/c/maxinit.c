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
#ifndef _MSC_VER
#include <unistd.h>
#include <util.h>
#else
void C2F(getenvc)(int *ierr,char *var,char *buf,int *buflen,int *iflag);
int SpawnPipe(char *argv[], void **istream, void **ostream);
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define GLOBAL
#include "maxsci1.h"
#include "maxsci.h"
#include "sciprint.h"
#include "Scierror.h"

extern int detecteErreurs (void);
extern int recupResult (int);
extern void envoiDonnees (void);
extern void gererQuestion (void);
extern void maxkill (void);

int maxinit ()
{
	#ifndef _MSC_VER
  int pipesm[2];
  int pipems[2];
  unsigned char echec = 0;
  char * scimax, * maxima_init;
  char * path = getenv ("SCIMAX_TOOLBOX_PATH");
  /* macOS/2027 port (Task 12): execlp("maxima",...) (bare name, PATH search)
     reliably failed here with "Error in launching Maxima" in this
     fork()ed-from-the-JVM child, even though getenv("PATH") in the PARENT
     Scilab process visibly includes /opt/homebrew/bin -- use the absolute
     path builder.sce already resolved via `which maxima` and recorded in
     MAXIMA_EXE_PATH (set by etc/SciMax.start) instead of trusting this
     child's own PATH resolution; execlp() still falls back to a PATH
     search if MAXIMA_EXE_PATH is unset. */
  char * maxima_exe = getenv ("MAXIMA_EXE_PATH");
  if (maxima_exe == NULL || maxima_exe[0] == '\0')
    maxima_exe = "maxima";
      
  if (!max_is_ok)
    {
      sciprint ("Launching Maxima :\n");
      /* macOS/2027 port (Task 12): a PTY was tried here instead of a plain
	 pipe (this Maxima/SBCL fully-buffers its stdout on a pipe instead of
	 line-buffering, delaying the handshake response by 40+ seconds --
	 see below); openpty()'s slave, dup2()'d over stdin/stdout/stderr,
	 reproducibly broke the PARENT Scilab session's own console
	 ("Cannot access to the term attributes: Operation not supported by
	 device", confirmed by isolated reproduction) even though the pty
	 slave lives only in the forked child. Reverted to the plain pipe;
	 the buffering delay is real but bounded (see the sciprint below and
	 the smoke file/report note) rather than trading it for a broken
	 session. */
      if (pipe (pipesm) || pipe (pipems))
	{
	  Scierror (9999, "Error in creating pipe\r\n");
	  return 1;
	}
      pid = fork ();
      if (pid == (pid_t)0)
	{
	  dup2 (pipems[1], STDOUT_FILENO);
	  dup2 (pipesm[0], STDIN_FILENO);
	  dup2 (pipems[1], STDERR_FILENO);
	  close (pipems[0]);
	  close (pipesm[1]);
	  /* macOS/2027 port (Task 12): the original preloaded loader.lisp via
	     -p. Confirmed by isolated reproduction outside Scilab: with THIS
	     Maxima (5.49, SBCL) and a PIPED (non-TTY) stdin -- exactly this
	     process's setup, above -- any use of -p at all (one file or two)
	     crashes Maxima's own startup into its low-level Lisp debugger
	     ("Unknown &KEY argument: #<SYNONYM-STREAM :SYMBOL SB-SYS:*STDIN*
	     ...>") before it ever reads a command from the pipe; loading the
	     exact same file via a plain load(...) call over the pipe, after
	     Maxima finishes its own normal (non-'-p') startup, does not. So
	     -p is dropped entirely here, and loader.lisp is loaded as the
	     first step of the SAME post-fork stdin command that already
	     loads maxima-init.mac/linearalgebra/nchrpl/mathml below --
	     load() transparently dispatches on extension (.lisp vs .mac),
	     exactly like the existing maxima-init.mac load already did. */
	  if (execlp (maxima_exe, "maxima",
		      "--disable-readline",
		      "--very-quiet", NULL) == -1)
	    {
	      Scierror (9999, "Error in launching Maxima\r\n");
	      echec = 1;
	      return 1;
	    }
	}
      else if (pid < (pid_t)0) 
	{
	  Scierror (9999, "Error in forking\r\n");
	  return 1;
	}
      else if (!echec)
	{
	  close (pipems[1]);
	  close (pipesm[0]);
	  is = fdopen (pipesm[1], "w");
	  os = fdopen (pipems[0], "r");
	  /* macOS/2027 port (Task 12): the original called detecteErreurs()
	     here first (waiting for a ready signal) before sending this
	     command. With -p gone (see above), Maxima hasn't loaded
	     scimax.lisp yet at this point in the stream, so main-prompt is
	     still Maxima's own stock prompt, not the <BO>/<BE>/... markers
	     detecteErreurs() looks for -- calling it here would misparse
	     that stock prompt. Matches the working isolated reproduction,
	     which sent this combined command immediately after Maxima
	     started, with no separate wait-for-ready step; recupResult()
	     below already calls detecteErreurs() itself as its first step
	     against THIS command's actual response, once main-prompt has
	     been redefined by the load() of loader.lisp/scimax.lisp that is
	     now this command's own first clause (replacing -p). load()
	     dispatches on extension, so a raw .lisp file works exactly like
	     the existing maxima-init.mac load beside it. */
	  fprintf (is, "_((load(\"%s/src/lisp/loader.lisp\"),file_search_maxima:append(file_search_maxima,[\"%s/maxima_init\"]),load(\"%s/maxima-init/maxima-init.mac\"),load(\"%s/maxima-init/maxima-init.lisp\"),load(linearalgebra),load(nchrpl),load(mathml)))$\n", path, path, path, path);
	  fflush (is);
	  if (recupResult (1) == -1)
	    {
	      max_is_ok = 1;
	      sciprint ("Maybe you should get the package maxima-share\n"); 
	      maxkill ();
	    }
	  else
 	    {
	      max_is_ok = 1;
	      sciprint ("OK\n");
	    }
	}
    }
  else
    {
      Scierror (9999, "Maxima has already been started\r\n");
      return 1;
    }
    #else 
  unsigned char echec = 0;
  char * scimax, * maxima_init;
  char   path[256] ,maxima_exe[256];
  char *argv[9];  
  int pid;

	int ierr,iflag=0,l1buf=256;	
	C2F(getenvc)(&ierr,"SCIMAX_TOOLBOX_PATH",path,&l1buf,&iflag);
	if ( ierr== 1) 
	{
		  Scierror (9999,"SCIMAX_TOOLBOX_PATH not defined.\n");
			      return 1;
	}
	  
	C2F(getenvc)(&ierr,"MAXIMA_EXE_PATH",maxima_exe,&l1buf,&iflag);
	if ( ierr== 1) 
	{
		  Scierror (9999,"MAXIMA_EXE_PATH not defined\n");
		      return 1;
	}
	  
  if (!max_is_ok)
    {
    sciprint ("Launching Maxima :\n");
	  scimax = malloc (strlen (path) + 21 + 1);
	  maxima_init = malloc (strlen (path) + 29 + 1);
    sprintf (scimax, "%s/src/lisp/loader.lisp", path);
 	  sprintf (maxima_init, "%s/maxima-init/maxima-init.lisp", path);
 
	 argv[0] = maxima_exe;
	 argv[1] =  "-p"; 
	 argv[2] =  scimax ; 
	 argv[3] =  "-p"; 
	 argv[4] =  maxima_init ; 
	 argv[5] =  "--disable-readline"; 
	 argv[6] =   "--very-quiet";
	 argv[7] =   NULL;
	 
    pid = SpawnPipe(argv, (void **)&os, (void **)&is) ;
	  free (scimax);
	  free (maxima_init);
	if (!pid || detecteErreurs () == -1)
	    {
	      Scierror (9999, "Error in launching Maxima\r\n");
	      echec = 1;
	      return 1;
	    }
	  fprintf (is, "_((file_search_maxima:append(file_search_maxima,[\"%s/maxima_init\"]),load(\"%s/maxima-init/maxima-init.mac\"),load(linearalgebra),load(nchrpl),load(mathml)))$\n", path, path);
	  fflush (is);
	  if (recupResult (1) == -1)
	    {
	      max_is_ok = 1;
	      sciprint ("Maybe you should get the package maxima-share\n"); 
	      maxkill ();
	    }
	  else
 	    {
	      max_is_ok = 1;
	      sciprint ("OK\n");
	    }
    }
  else
    {
      Scierror (9999, "Maxima has already been started\r\n");
      return 1;
    }
    #endif
  return 0;
}
