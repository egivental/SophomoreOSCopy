#include <stdlib.h>
#include <stdio.h>

#include "signalQueue.h"

struct signalNode* signalQueueHead = NULL;

int sigEnqueue(signals_e sig, pid_t pid) {
  struct signalNode *q;
  q = calloc(1, sizeof(signalNode));
  if (q == NULL) {
    perror("calloc failed");
    return -1;
  }
  q->sig = sig;
  q->pid = pid;
  q->next = NULL;
  struct signalNode* ptr = signalQueueHead;
  
  while (ptr->next) {
    ptr = ptr->next;
  }
  ptr->next = q;
  return 1;
}

signalNode* sigPop() {
  if (!signalQueueHead){
    return NULL;
  }

  struct signalNode* temp = signalQueueHead;
  signalQueueHead = signalQueueHead->next;
  return temp;
}
