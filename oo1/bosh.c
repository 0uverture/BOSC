/* 

   bosh.c : BOSC shell 

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
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

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname1(char *hostname)
{
  int res = gethostname(hostname, HOSTNAMEMAX); // From unistd.h
  if (res != 0) { // Succeeded
    return hostname;
  }
  hostname = "";
  return hostname;
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd)
{
  Cmd *cmdlist = shellcmd->the_cmds;
  while (cmdlist != NULL) {
    char **cmd = cmdlist->cmd; // Current command
    cmdlist = cmdlist->next; // Iteration
    char **scndCmd = cmdlist->cmd; // Next command
    printf("%s\n", cmd[0]);
    if (scndCmd != NULL)
      printf("%s\n", scndCmd[0]);
    // Print Shellcmd
    printshellcmd(shellcmd);

    // Execution

    // Store infile and outfile in variables
    char *rd_stdin = shellcmd->rd_stdin;
    char *rd_stdout = shellcmd->rd_stdout;
    // Background vs. Foreground
    if (shellcmd->background == 1) { // Execute command without waiting for finished execution
      if (scndCmd != NULL) { // Perform piping
        pipecmd(*cmd, cmd, *scndCmd, scndCmd, 1); // Performing pipe with current and next command (with their args) in bg
      }
      else {
        backgroundcmd(*cmd, cmd, rd_stdin, rd_stdout);
      }
    }
    else { // Execute command and wait for it
      if (scndCmd != NULL) { // Perform piping
        pipecmd(*cmd, cmd, *scndCmd, scndCmd, 0); // Performing pipe with current and next command (with their args) in fg
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
  
  if (gethostname1(hostname)) {

    /* parse commands until exit or ctrl-c */
    while (!terminate) {
      printf("%s", hostname);
      if (cmdline = readline(":# ")) {
      	if(*cmdline) {
      	  add_history(cmdline);
      	  if (parsecommand(cmdline, &shellcmd)) {
      	    terminate = executeshellcmd(&shellcmd);
      	  }
      	}
      	free(cmdline);
      } else terminate = 1;
    }
    printf("Exiting bosh.\n");
  }    
    
  return EXIT_SUCCESS;
}

