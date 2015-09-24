/* 

   Opgave 1

   forback.c

 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "forback.h"
#include "redirect.h"


/* start the program specified by filename with the arguments in argv 
   in a new process and wait for termination */
int foregroundcmd(char *filename, char *argv[], char *infilename, char *outfilename)
{
  pid_t pid = fork();
  if (pid == 0) { /* child */
    redirect_stdinandout(infilename, outfilename);
    
  	printf("F: Child replacing process...\n");
  	execvp(filename, argv);
  } else { /* parent */
  	printf("F: Waiting for child process, hopefully...\n");
  	waitpid(pid, NULL, 0); // Arguments...
  	printf("F: Done waiting for child.\n");
  }
}

/* start the program specified by filename with the arguments in argv 
   in a new process */
int backgroundcmd(char *filename, char *argv[], char *infilename, char *outfilename)
{
  pid_t pid = fork();
  if (pid == 0) { /* child */
    redirect_stdinandout(infilename, outfilename);

    printf("B: Child replacing process...\n");
    execvp(filename, argv);
  } else { /* parent */
    printf("B: Not waiting for child.\n");
  }
}
