/* 
   Opgave 2

   redirect.h

 */

#ifndef _REDIRECT_H
#define _REDIRECT_H

int redirect_stdinandout(char *, char *);
int redirect_stdincmd(char *, char *[], char *);
int redirect_stdoutcmd(char *, char *[], char *);
int redirect_stdin(char *);
int redirect_stdout(char *);

#endif
