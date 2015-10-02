/******************************************************************************
   main.c

   Implementation of a simple FIFO buffer as a linked list defined in list.h.

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"

// FIFO list;
List *fifo;

int main(int argc, char* argv[])
{
  fifo = list_new();

  Node *n1;
  Node *n2;
  Node *n3;

  printf("Let us begin...\n");
  int pid = fork();
  if (pid == 0) // Child process: Add 2, read one
  {
    list_add(fifo, node_new_str("s14"));

    n2 = list_remove(fifo);
    if (n2 == NULL) { printf("Error no elements in list\n"); }
    
    list_add(fifo, node_new_str("s34"));
    
    // Print results
    if (n2 != NULL) {
      printf("%s\n", (char*)n2->elm);
    }
    else{
      printf("n3 ded.\n");
    }
  }
  else // Parent process: Read two, add one
  {
    n1 = list_remove(fifo);
    if (n1 == NULL) { printf("Error no elements in list\n"); }
    
    list_add(fifo, node_new_str("s24"));
    
    n3 = list_remove(fifo);
    if (n3 == NULL) { printf("Error no elements in list\n"); }
    
    // Print results
    if (n1 != NULL) {
      printf("%s\n", (char*)n1->elm);
    }
    else{
      printf("n1 ded.\n");
    }
    if (n3 != NULL){
      printf("%s\n", (char*)n3->elm);
    }
    else{
      printf("n3 ded.\n");
    }
    
    
    // Wait for child
    waitpid(pid, NULL, 0);
  }
  return 0;
}

