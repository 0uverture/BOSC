/* 

   bosh.c : BOSC shell 

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "redirect.h"
#include "parser.h"
#include "forback.h"
#include "pipe.h"
#include "print.h"

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100
#define COMMANDANDARGSMAX 256

void handler(int dummy)
{
}

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char *hostname)
{
  FILE *file;
  file = fopen("/proc/sys/kernel/hostname", "r");
  fscanf(file, "%s", hostname);
  fclose(file);
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd)
{
  Cmd *cmdlist = shellcmd->the_cmds;
  int background = shellcmd->background;
  char *infilename = shellcmd->rd_stdin;
  char *outfilename = shellcmd->rd_stdout;

  // Count amount of commands (pipe-components)
  Cmd *cmdlistCounter = cmdlist;
  int cmdAmount = 0;
  while(cmdlistCounter != NULL){
    cmdlistCounter = cmdlistCounter->next;
    cmdAmount++;
  }
  printf("Command amount: %d\n", cmdAmount);

  int fd[2]; // New pipe declared

  int in  = -1; // In being used at execution
  int out  = -1; // Out to be passed on
  int last_out; // Out to use
  if (outfilename != NULL) {
    last_out = open(outfilename, O_WRONLY|O_CREAT);
  }
  else {
    last_out = -1;
  }

  pid_t pids[cmdAmount];
  int i;
  for (i = 0; cmdlist != NULL; i++ ) {
    char **cmd = cmdlist->cmd; // Current command
    cmdlist = cmdlist->next; // Iteration

    // Init pipe if another command exists
    if (cmdlist != NULL) {
      // if true: pipe was used
      if (pipe(fd) < 0) {
        printf("Error when creating pipe.\n");
        return -1;
      }
      in = fd[0];
      out = fd[1];
    }
    else {
      // Last command (First input command - replace input if any inputfile provided)
      if (infilename != NULL) {
        in = open(infilename, O_RDONLY);
      }
      else {
        in = -1;
      }
      out = -1;
    }

    printf("Before execution of %s: in: %d, out: %d, last_out: %d\n", *cmd, in, out, last_out);

    // Execution
    pid_t pid = fork();
    pids[i] = pid;
    if (pid == 0) { // child
      if(out != -1) {
        close(out);
      }
      redirect_stdinandout(in, last_out);
      int status = execvp(*cmd, cmd); // Execute current command
      if (status == -1) {
        printf("Command not found.\n");
        exit(1);
      }
    }

    if(in != -1) {
      close(in);
    }
    if(last_out != -1) {
      close(last_out);
    }

    if(cmdlist != NULL) {
      // Update last_out for next command
      last_out = out;
    }
    else {
      // No next command: Close file if not null, just in case
      if(out != -1) {
        close(out);
      }
    }
  }

  if(!background){ // Wait for all processes
    for(i = 0; i < cmdAmount; i++){
      waitpid(pids[i], NULL, 0);
    }
  }

  return 0;
}

/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* initialize the shell */
  char *cmdline;
  char hostname[HOSTNAMEMAX];
  int terminate = 0;
  Shellcmd shellcmd;

  signal(SIGINT, handler); // Listen for Ctrl + C
  
  if (!gethostname(hostname)) {

    /* parse commands until exit or ctrl-c */
    while (!terminate) {
      printf("%s", hostname);
      if (cmdline = readline(":# ")) {
      	if(*cmdline) {
          if (checkIfExit(cmdline)) {
            return EXIT_SUCCESS;
          }
      	  add_history(cmdline);
      	  if (parsecommand(cmdline, &shellcmd)) {
      	    executeshellcmd(&shellcmd);
      	  }
      	}
      	free(cmdline);
      } else terminate = 1;
    }
    printf("Exiting bosh.\n");
  }
  return EXIT_SUCCESS;
}

int checkIfExit(char *firstCmd)
{
  if(strcmp(firstCmd, "exit") == 0){ // they are equal
    return 1;
  }
  return 0;
}

