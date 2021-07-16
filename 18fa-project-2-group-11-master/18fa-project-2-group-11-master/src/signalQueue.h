#ifndef _SIG_QUEUE_H_
#define _SIG_QUEUE_H_

#include "signals.h"

typedef struct signal_queue {
  signalNode *signal;
  struct queue *next;
} signal_queue;

extern struct signalNode* signalQueueHead;


int sigEnqueue(signals_e sig, pid_t pid);
signalNode* sigPop();

#endif