/* 

   Opgave 2

   redirect.c

 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "redirect.h"

/* start the program specified by filename with the arguments in argv 
   in a new process that has its stdin redirected to infilename and
   wait for termination */
int redirect_stdincmd(char *filename, char *argv[], char *infilename)
{
	pid_t pid = fork();
	if (pid == 0) { /* child process*/
		/* manipulate the file descriptor of the child process */
		int fid = open(infilename, O_RDONLY);
		/* Replace stdin of the child process with fid */
		close(0); /* 0 = stdin */
		dup(fid);
		/* Close fid */
		close(fid);
		execvp(filename, argv);
	}
	else {          /* parent process */
		waitpid(pid, NULL, 0);
		printf("redirect_stdINcmd done.\n");
	}
  	return 0;
}

/* start the program specified by filename with the arguments in argv 
   in a new process that has its stdout redirected to outfilename and 
   wait for termination */
int redirect_stdoutcmd(char *filename, char *argv[], char *outfilename)
{
	pid_t pid = fork();
	if (pid == 0) { /* child process*/
		/* manipulate the file descriptor of the child process */
		int fid = open(outfilename, O_WRONLY);
		/* Replace stdin of the child process with fid */
		close(1); /* 1 = stdout */
		dup(fid);
		/* Close fid */
		close(fid);
		execvp(filename, argv);
	}
	else {          /* parent process */
		waitpid(pid, NULL, 0);
		printf("redirect_stdOUTcmd done.\n");
	}
  	return 0;
}
