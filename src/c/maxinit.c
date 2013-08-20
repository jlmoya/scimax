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
#include "stack-c.h"
#ifndef _MSC_VER
#include <unistd.h>
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

#define GLOBAL
#include "maxsci1.h"
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
      
  if (!max_is_ok)
    {
      sciprint ("Launching Maxima :\n");
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
	  scimax = malloc (strlen (path) + 21 + 1);
	  maxima_init = malloc (strlen (path) + 29 + 1);
	  sprintf (scimax, "%s/src/lisp/loader.lisp", path);
	  sprintf (maxima_init, "%s/maxima-init/maxima-init.lisp", path);
	  if (execlp ("maxima", "maxima", 
		      "-p", scimax,
		      "-p", maxima_init,
		      "--disable-readline", 
		      "--very-quiet", NULL) == -1)
	    {
	      free (scimax);
	      free (maxima_init);
	      Scierror (9999, "Error in launching Maxima\r\n");
	      echec = 1;
	      return 1;
	    }
	  free (scimax);
	  free (maxima_init);
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
	  if (detecteErreurs () == -1)
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
