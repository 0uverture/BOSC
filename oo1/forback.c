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


/* start the program specified by filename with the arguments in argv 
   in a new process and wait for termination */
int foregroundcmd(char *filename, char *argv[])
{
  pid_t pid = fork();
  if (pid == 0) { /* child */
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
int backgroundcmd(char *filename, char *argv[])
{
  printf("B: Replacing process with given process...\n");
  execvp(filename, argv);
}
