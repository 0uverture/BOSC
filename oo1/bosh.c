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
  printshellcmd(shellcmd);
  Cmd *cmdlist = shellcmd->the_cmds;
  while (cmdlist != NULL) {
    char **cmd = cmdlist->cmd; // Store command in new pointer
    cmdlist = cmdlist->next; // Next command

    char **printcmd = cmd;

    if(strcmp("ls", *printcmd) == 0) // This comparison works. Looking at command without its arguments.
    {
      printf("Command is ls?: %s\n", *printcmd);
      pid_t pid = fork();
      if (pid == 0) { // Child process
        char *args[] = {
        "/bin/bash",
        "-c",
        *printcmd,
        NULL
      };
      //args[0] = strcat("./", *printcmd);
      //args[1] = NULL;
      execvp(args[0], args);
      }
      else { // Parent process
        int status;
        int pidres = waitpid(pid, &status, 0);
        printf("Command finished: %s\n", *printcmd);
      }
    }

    /* while (*printcmd != NULL) { // Iterate command & arguments
      printf(" %s ", *printcmd++); // print the cmd and arguments
    } */ // Leftover code from print.c
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

