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
    char **cmd = cmdlist->cmd; // Store command in new pointer
    cmdlist = cmdlist->next; // Next command

    // Print Shellcmd
    printshellcmd(shellcmd);

    // Execution background vs foreground
    char *rd_stdin = shellcmd->rd_stdin;
    char *rd_stdout = shellcmd->rd_stdout;
    if (shellcmd->background == 1) {
      backgroundcmd(*cmd, cmd, rd_stdin, rd_stdout);
    }
    else { // Execute command and wait for it
      foregroundcmd(*cmd, cmd, rd_stdin, rd_stdout);
    }
    
  }
  return 0;
}

/*int executecommandandwait(char *commandwithargs) {
  pid_t pid = fork();
  if (pid == 0) { // Child process
    char *args[] = {
      "/bin/bash",
      "-c",
      commandwithargs,
      NULL
    };
    execvp(args[0], args);
  }
  else if (pid > 0) { // Parent process
    int status;
    int pidres = waitpid(pid, &status, 0);
    printf("Command finished: %s, with status: %d\n", commandwithargs, status);
  }
  else {
    printf("pid craaaaaaassh.\n");
  }
}

int executecommandinbg(char *commandwithargs) {
  pid_t pid1 = fork();
  if (pid1 == 0) { // Child process
    printf("Entered child process.\n");
    char *args[] = {
      "/bin/bash",
      "-c",
      commandwithargs,
      NULL
    };
    char[] cmd = commandwithargs[0];

    int status = execvp(args[0], args);
    printf("Background process finished: %s, with status: %d\n", commandwithargs, status);
  }
  else if (pid1 > 0) { // Parent process
    printf("I don't need to care about what goes on in the child process.\n");
  }
}*/

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

