/* 

   bosh.c : BOSC shell 

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "redirect.h"
#include "parser.h"
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

    char **printcmd = cmd;

    int runinbg = 0;

    char commandwithargs[COMMANDANDARGSMAX] = "";
    while (*printcmd != NULL) { // Iterate command & arguments
      if(strcmp("&", *printcmd) == 0) // This comparison works. Looking at command without its arguments.
      {
        printf("Command should be run in bg.\n");
        // Change "shellcmd"...
        shellcmd->background = 1;
        runinbg = 1;
      }
      else {
        strcat(commandwithargs, " "); // Add space
        strcat(commandwithargs, *printcmd++); // Add argument
      }
    } // Iterating command (first) + arguments

    // Print Shellcmd
    printshellcmd(shellcmd);

    // Execution
    if (runinbg == 1){
      executecommandinbg(commandwithargs);
    }
    else { // Execute command and wait for it
      executecommandandwait(commandwithargs);
    }
    
  }
  return 0;
}

int executecommandandwait(char *commandwithargs) {
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
  else { // Parent process
    int status;
    int pidres = waitpid(pid, &status, 0);
    printf("Command finished: %s\n", commandwithargs);
  }
}

int executecommandinbg(char *commandwithargs) {
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
  else { // Parent process
    printf("I don't care for my child.\n");
  }
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

