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
int pipecmd(char *filename1, char *argv1[], char *filename2, char *argv2[]) {
	int fd[2];
	if (pipe(fd) < 0) {
		printf("An error occurred while creating pipe\n");
		return -1;
	}
	pid_t pid1 = fork();
	if (pid1 == 0) {
		// Switch out stdinput with pipe input
		close(fd[1]);
		close(0);
		dup(fd[0]);
		// Do proces here
		int status = execvp(filename1, argv1);
		printf("Status of pid1: %d\n", status);
		// Cleanup
		close(fd[0]);
		exit(status);
	} else if (pid1 > 0) {
		pid_t pid2 = fork();
		if (pid2 == 0) {
			// Switch out stdoutput with pipe output
			close(fd[0]);
			close(1);
			dup(fd[1]);
			// Do proces here
			int status = execvp(filename2, argv2);
			printf("Status of pid2: %d\n", status);
			// Cleanup
			close(fd[1]);
			exit(status);
		} else if (pid2 > 0) {
			// Wait for both processes to finish
			int status;
			int pid1res = waitpid(pid1, &status, 0);
			int pid2res = waitpid(pid2, &status, 0);
			printf("Process 1 exit code: %d\n", pid1-pid1res);
			printf("Process 2 exit code: %d\n", pid2-pid2res);
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}
