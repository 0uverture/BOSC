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
  
  while (cmdlist != NULL ) {
    char **cmd = cmdlist->cmd; // Current command
    cmdlist = cmdlist->next; // Iteration

    // Retrieve next command if any
    char **scndCmd = NULL;
    if (cmdlist != NULL) // if true: pipe was used
      scndCmd = cmdlist->cmd; // Next command

    // Control-prints:
    printf("First command: '%s'\n", cmd[0]);
    if (scndCmd != NULL)
      printf("Second command: '%s'\n", scndCmd[0]);
    
    printshellcmd(shellcmd);

    // Execution

    // Store infile and outfile in variables
    char *rd_stdin = shellcmd->rd_stdin;
    char *rd_stdout = shellcmd->rd_stdout;
    // Background vs. Foreground
    if (shellcmd->background == 1) { // Execute command without waiting for finished execution
      if (scndCmd != NULL) { // Perform piping
        pipecmd(*cmd, cmd, *scndCmd, scndCmd, 1); // Performing pipe with current and next command (with their args) in bg
        if (cmdlist != NULL) {
          cmdlist = cmdlist->next; // Avoid executing 2nd part of pipe (?)
        }
      }
      else {
        backgroundcmd(*cmd, cmd, rd_stdin, rd_stdout);
      }
    }
    else { // Execute command and wait for it
      if (scndCmd != NULL) { // Perform piping
        pipecmd(*cmd, cmd, *scndCmd, scndCmd, 0); // Performing pipe with current and next command (with their args) in fg
        if (cmdlist != NULL)
          cmdlist = cmdlist->next; // Avoid executing 2nd part of pipe (?)
      }
      else {
        foregroundcmd(*cmd, cmd, rd_stdin, rd_stdout);
      }
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

