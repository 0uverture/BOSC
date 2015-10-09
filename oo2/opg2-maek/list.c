/******************************************************************************
   list.c

   Implementation of simple linked list defined in list.h.

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "list.h"

pthread_mutex_t mutexlength;
pthread_mutex_t mutexelement;

/* list_new: return a new list structure */
List *list_new(void)
{
  List *l;

  l = (List *) malloc(sizeof(List));
  l->len = 0;

  /* insert root element which should never be removed */
  l->first = l->last = (Node *) malloc(sizeof(Node));
  l->first->elm = NULL;
  l->first->next = NULL;
  return l;
}

/* list_add: add node n to list l as the last element */
void list_add(List *l, Node *n)
{
  pthread_mutex_lock(&mutexlength);
  int length = l->len; // Retrieve length

  if (length > 0) // Already contains one or more elements
  {
    pthread_mutex_unlock(&mutexlength);

    pthread_mutex_lock(&mutexelement);
    // Link to next (new) node
    l->last->next = n; // Make previously last element link to new last element ("next")
  }
  else // Is empty
  {
    pthread_mutex_unlock(&mutexlength);

    pthread_mutex_lock(&mutexelement);
    l->first->next = n; // Set first Node to given Node
  }

  l->last = n; // Define as last element
  pthread_mutex_unlock(&mutexelement);

  pthread_mutex_lock(&mutexlength);
  l->len++; // Update length regardless
  pthread_mutex_unlock(&mutexlength);
}

/* list_remove: remove and return the first (non-root) element from list l */
Node *list_remove(List *l)
{
  int length = l->len; // Retrieve length
  if (length > 0) // Contains one or more elements
  {
    Node *removed = l->first->next; // Get currently first Node ('next' of header-Node)
    Node *newFirst = removed->next; // Get new first Node
    if (newFirst == NULL) // List is being emptied
    {
      l->first->next = l->last = NULL;
    }
    else { // Two or more elements
      l->first->next = newFirst; // Overwrite "first->next" (First Node apart from header-Node) to be the next element in FIFO list
    }
    
    // One element removed regardless
    l->len--;

    // List no longer holds any connection to previously first element, return:
    return removed;
  }
  else // List is empty
  {
    return NULL;
  }
}

/* node_new: return a new node structure */
Node *node_new(void)
{
  Node *n;
  n = (Node *) malloc(sizeof(Node));
  n->elm = NULL;
  n->next = NULL;
  return n;
}

/* node_new_str: return a new node structure, where elm points to new copy of s */
Node *node_new_str(char *s)
{
  Node *n;
  n = (Node *) malloc(sizeof(Node));
  n->elm = (void *) malloc((strlen(s)+1) * sizeof(char));
  strcpy((char *) n->elm, s);
  n->next = NULL;
  return n;
}
