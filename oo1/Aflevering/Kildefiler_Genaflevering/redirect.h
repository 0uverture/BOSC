/* 
   Opgave 2

   redirect.h

 */

#ifndef _REDIRECT_H
#define _REDIRECT_H

int redirect_stdinandout(int, int);
int redirect_stdincmd(char *, char *[], char *);
int redirect_stdoutcmd(char *, char *[], char *);
int redirect_stdin(int);
int redirect_stdout(int);

#endif
