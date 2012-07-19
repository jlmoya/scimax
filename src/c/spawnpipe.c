#pragma comment(lib, "User32.lib")
#pragma comment(lib, "advapi32.lib")

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h> 


extern int  Scierror(int iv,const char *fmt,...);


#include<windows.h>
void CreateTwoSideHandle(HANDLE* childInput,HANDLE* childOutput,HANDLE* childError,HANDLE* parentRead,HANDLE* parentWrite );
char *build_command(char **argv, int quote); 


/* Spawns a new process with argument list 'argv'
 * and redirects its standard output to 'istream'
 * , tandard input to 'ostream'
 * Returns a handle of the newly created process
 * or -1 if an error occurs
 */
int SpawnPipe(char *argv[], void **istream, void **ostream)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE parentRead, childWrite , childError, childRead, parentWrite;

	int fd,ofd, create;
	char *command = 0;

	command = build_command(argv, 1);

	if (!command)
		return -1;

	/* Set the length of the security attribute structure and allow
	   the handle to be inherited by child processes */



 CreateTwoSideHandle( &childRead, &childWrite, &childError, &parentRead, &parentWrite );
	GetStartupInfo(&si);

	/* Sets the standard output handle for the process to the
	   handle specified in hStdOutput */
	si.dwFlags = STARTF_USESTDHANDLES;

	si.hStdInput =  childRead;
	si.hStdOutput = childWrite;
	si.hStdError  = childError;
	
	create = CreateProcess(
		NULL,				// The full path of app to launch
		command,					// Command line parameters
		NULL,					// Default process security attributes
		NULL,					// Default thread security attributes
		TRUE,					// Inherit handles from the parent
		0,						// Normal priority
		NULL,					// Use the same environment as the parent
		NULL,					// Use app's directory as current
		&si,					// Startup Information
		&pi);					// Process information stored upon return
		
	/* Close the handle that we're not going to use */
	CloseHandle(childRead);
  CloseHandle(childWrite);
	CloseHandle(childError); 
	
	if (!create)
	{
		Scierror(9999,"Error in CreateProcess");
		CloseHandle(parentRead);
		return 0;
	}

	/* Associates a file descriptor with the stdout pipe */
	 fd = _open_osfhandle((intptr_t)parentRead, _O_BINARY);


	if (!fd)
	{
		Scierror(9999,"Error in  _open_osfhandle for parent read");
		return 0;
	}

	ofd = _open_osfhandle((intptr_t)parentWrite, _O_TEXT);
		if (!ofd)
	{
		Scierror(9999,"Error in  _open_osfhandle for parent write");
		return 0;
	}
	/* Open the pipe istream using its file descriptor */
	*istream = _fdopen( fd, "rb");
	if(!(*istream))
	{
		Scierror(9999,"Error in  fdopen for parent read");
		_close(fd);
		return 0;
	}

	*ostream = _fdopen(ofd, "w");
	if(!(*ostream))
	{
		Scierror(9999,"Error in  fdopen for parent write");
		_close(ofd);
		return 0;
	}

	if (command)
		free(command);

	return (int)pi.hProcess;
}



/* Adds double quotes around 'src' and concatenates the resulting
 * string to 'dest', returns a pointer to 'dest'
 */
static void *strqcat(char *dest, const char *src)
{
	strcat(dest, "\"");
	strcat(dest, src);
	strcat(dest, "\"");

	return dest;
}

/* Copies the string 'src' to 'dest' enclosing it with
 * double quotes (") and returns a pointer to 'dest'
 */
static void *strqcpy(char *dest, const char *src)
{
	strcpy(dest, "\"");
	strcat(dest, src);
	strcat(dest, "\"");

	return dest;
}

/* Converts the given argument array to a string, with each
 * argument separated by space character. Note: this function
 * is not complete.
 *
 * If parameter 'quote' is 1, each argument will be enclosed
 * in double quotes (")
 *
 * The function returns a pointer to the string; it is
 * programmer'sresponsibility to free the allocated memory.
 */
char *build_command(char **argv)
{
	char *cmd;
	int i, length = 0;

	for(i = 0; argv[i]; i++) {
	 length += 3;	/* two quotation marks and a space */
	 length += (int)strlen(argv[i]);
	}

	cmd = (char *)malloc(length);
	strqcpy(cmd, argv[0]);

	for(i = 1; argv[i]; i++) {
		strcat(cmd, " ");
		strqcat(cmd, argv[i]);
  }

	return cmd;
}

  


void CreateTwoSideHandle(HANDLE* childInput,HANDLE* childOutput,HANDLE* childError,HANDLE* parentRead,HANDLE* parentWrite )
{
      HANDLE parentReadTmp;
      HANDLE parentWriteTmp;
      SECURITY_ATTRIBUTES sa;


      // Set up the security attributes struct.
      sa.nLength= sizeof(SECURITY_ATTRIBUTES);
      sa.lpSecurityDescriptor = NULL;
      sa.bInheritHandle = TRUE;


      // Create the child output pipe.
      if (!CreatePipe(&parentReadTmp,childOutput,&sa,0))
         Scierror(9999,"Error in createing childOutput pipe");


      // Create a duplicate of the output write handle for the std error
      // write handle. This is necessary in case the child application
      // closes one of its std output handles.
      if (!DuplicateHandle(GetCurrentProcess(),*childOutput,
                           GetCurrentProcess(),childError,0,
                           TRUE,DUPLICATE_SAME_ACCESS))
         Scierror(9999,"Error in duplicate childOutput handle");


      // Create the child input pipe.
      if (!CreatePipe(childInput,&parentWriteTmp,&sa,0))
         Scierror(9999,"Error in createing childInput pipe");


      // Create new output read handle and the input write handles. Set
      // the Properties to FALSE. Otherwise, the child inherits the
      // properties and, as a result, non-closeable handles to the pipes
      // are created.
      if (!DuplicateHandle(GetCurrentProcess(),parentReadTmp,
                           GetCurrentProcess(),
                           parentRead, // Address of new handle.
                           0,FALSE, // Make it uninheritable.
                           DUPLICATE_SAME_ACCESS))
         Scierror(9999,"DupliateHandle");

      if (!DuplicateHandle(GetCurrentProcess(),parentWriteTmp,
                           GetCurrentProcess(),
                           parentWrite, // Address of new handle.
                           0,FALSE, // Make it uninheritable.
                           DUPLICATE_SAME_ACCESS))
      Scierror(9999,"Error in duplicate parentWrite handle ");


      // Close inheritable copies of the handles you do not want to be
      // inherited.
      if (!CloseHandle(parentReadTmp)) Scierror(9999,"Error in closeing parentReadTmp handle");
      if (!CloseHandle(parentWriteTmp)) Scierror(9999,"Error in closeing  parentWriteTmp handle)");
}
