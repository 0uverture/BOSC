/*

   Opgave 3

   pipe.c

 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "pipe.h"

/* create a pipe between two new processes, executing the programs
   specified by filename1 and filename2 with the arguments in argv1
   and argv2 and wait for termination */
int pipecmd(char *filename1, char *argv1[], char *filename2, char *argv2[], int background) {
  int fd[2];
  if (pipe(fd) < 0) {
  printf("Error when creating pipe.\n");
  	return -1;
  }
  pid_t pid1 = fork();
  if (pid1 == 0) {
    // Switch out stdinput with pipe input
    close(0); // Close stdin
    dup(fd[0]); // Duplicate read-end of pipe (takes over stdin "spot")
    close(fd[1]); // Close write-end of pipe
    // Execute process
    int status = execvp(filename1, argv1);
    printf("Status of pid1: %d\n", status);
    // Clean up open pipe end
    close(fd[0]); // Close read-end of pipe (process finished)
    exit(status);
  } else if (pid1 > 0) {
	pid_t pid2 = fork();
	if (pid2 == 0) {
		// Switch out stdoutput with pipe output
		close(1); // Close stdout
		dup(fd[1]); // Duplicate write-end of pipe (takes over stdout "spot")
		close(fd[0]); // Close read-end of pipe
		// Execute process
		int status = execvp(filename2, argv2);
		printf("Status of pid2: %d\n", status);
		// Clean up open pipe end
		close(fd[1]); // Close write-end of pipe (process finished)
		exit(status);
	} else if (pid2 > 0) {
		close(fd[0]);
		close(fd[1]);
		if (!background) {
			// Wait for processes
			waitpid(pid1, NULL, 0);
			waitpid(pid2, NULL, 0);
		}
	} else {
		return -1;
	}
} else {
	return -1;
}
}
