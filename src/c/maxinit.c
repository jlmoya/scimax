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
#include <signal.h>
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

  if (path == NULL || path[0] == '\0')
    {
      /* set by etc/SciMax.start; without it neither loader.lisp nor the
	 maxima-init files below can be located (and %s would be passed
	 NULL) */
      Scierror (9999, "SciMax: SCIMAX_TOOLBOX_PATH is not set\r\n");
      return 1;
    }

  if (!max_is_ok)
    {
      sciprint ("Launching Maxima :\n");
      /* macOS/2027 port (Task 12): the child talks over PLAIN PIPES, on
	 purpose. This Maxima/SBCL full-buffers its stdout when it is not a
	 tty, which used to strand every response (the whole handshake) in
	 the child's stdio buffer indefinitely; a PTY was tried to force
	 line buffering, and openpty()'s slave, dup2()'d over
	 stdin/stdout/stderr, reproducibly broke the PARENT Scilab
	 session's own console ("Cannot access to the term attributes:
	 Operation not supported by device", confirmed by isolated
	 reproduction) even though the pty slave lives only in the forked
	 child. That attempt was reverted, and the buffering is now solved
	 on the LISP side instead: src/lisp/loader.lisp installs a
	 main-prompt that (finish-output)s right after printing the
	 "\n<EO>\n" frame terminator, i.e. the subprocess flushes each
	 complete response at exactly the moment it is ready for the next
	 command. Nothing here may ever touch this process's own stdio or
	 controlling terminal. */
      if (access (maxima_exe, X_OK) != 0 && strchr (maxima_exe, '/') != NULL)
	{
	  /* Loud, parent-side gate: MAXIMA_EXE_PATH (etc/SciMax.start) is
	     an absolute path in practice, so a missing/renamed Maxima is
	     caught here before any fork -- "verified means runnable here".
	     A bare command name (unset MAXIMA_EXE_PATH) is left to the
	     child's execlp() PATH search below. */
	  Scierror (9999, "SciMax: Maxima executable '%s' not found or not executable\r\n", maxima_exe);
	  return 1;
	}
      {
	/* Same loud parent-side gate for loader.lisp: if it is missing,
	   the load() error below would come from Maxima's STOCK error
	   reporter (scimax.lisp's marker-emitting merror is not loaded
	   yet, nor is the <EO>-printing main-prompt), i.e. no marker line
	   would ever arrive and the step-1 drain would hang. */
	char ldr[1024];
	snprintf (ldr, sizeof ldr, "%s/src/lisp/loader.lisp", path);
	if (access (ldr, R_OK) != 0)
	  {
	    Scierror (9999, "SciMax: '%s' not found\r\n", ldr);
	    return 1;
	  }
      }
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
	     -p is dropped entirely here and loader.lisp is loaded over the
	     pipe instead, as its own dedicated first command -- see the
	     two-step handshake in the parent branch below. */
	  execlp (maxima_exe, "maxima",
		  "--disable-readline",
		  "--very-quiet", NULL);
	  /* exec only returns on failure. This is a FORKED CHILD of the
	     whole Scilab/JVM process: the old "Scierror(...); return 1"
	     here let a broken second Scilab limp on inside the fork.
	     _exit() immediately instead (no atexit/JVM teardown); the
	     parent's reader then sees EOF on the pipe, which the hardened
	     VIDEOS macro (maxsci1.h) turns into a loud "<BS>" protocol
	     error rather than a hang. */
	  _exit (127);
	}
      else if (pid < (pid_t)0)
	{
	  Scierror (9999, "Error in forking\r\n");
	  return 1;
	}
      else
	{
	  close (pipems[1]);
	  close (pipesm[0]);
	  is = fdopen (pipesm[1], "w");
	  os = fdopen (pipems[0], "r");
	  /* macOS/2027 port (Task 12): two-step handshake, restoring the
	     protocol structure upstream got from "-p loader.lisp" (which
	     this Maxima/SBCL crashes on when stdin is a pipe -- see the
	     comment at execlp above).

	     Step 1: load loader.lisp as a PLAIN maxima command, then wait
	     for the first "<EO>" prompt. This must not be part of the
	     framed "_((...))" command: $_ is only defmspec'd once
	     scimax.lisp is loaded, so a combined "_((load(loader),...))"
	     dispatches as an unknown-function noun form -- the loads run,
	     but no "<BO>E" frame is ever printed and the old
	     recupResult(1) handshake blocked forever waiting for one
	     (confirmed byte-level in an isolated pipe run). Once
	     loader.lisp is in, main-prompt prints "<EO>" and flushes
	     (see src/lisp/loader.lisp), so this drain terminates; the
	     load's own informational lines are skipped, "<BD>" (scimath/
	     scimax unloadable) and "<BS>" (EOF on the pipe, i.e. the
	     child died -- injected by VIDEOS in maxsci1.h) fail loudly.

	     Step 2: only now send the framed init command (share
	     packages + user init files) and parse its "<BO>E" response
	     with the stock recupResult(1), exactly like every later
	     command. */
	  fprintf (is, "load(\"%s/src/lisp/loader.lisp\")$\n", path);
	  fflush (is);
	  do VIDEOS;
	  while (!iseo (buf) && !isbd (buf) && !isbs (buf));
	  if (!iseo (buf))
	    {
	      Scierror (9999, isbd (buf)
			? "SciMax: Maxima started but could not load scimax/scimath (see src/lisp/README)\r\n"
			: "SciMax: Maxima exited during startup\r\n");
	      kill (pid, SIGKILL);
	      fclose (is);
	      fclose (os);
	      return 1;
	    }
	  fprintf (is, "_((file_search_maxima:append(file_search_maxima,[\"%s/maxima_init\"]),load(\"%s/maxima-init/maxima-init.mac\"),load(\"%s/maxima-init/maxima-init.lisp\"),load(linearalgebra),load(nchrpl),load(mathml)))$\n", path, path, path);
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
