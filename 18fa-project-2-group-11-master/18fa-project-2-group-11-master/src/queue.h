#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "pcb.h"

typedef struct queue {
	pcb_t *pcb;
	struct queue *next;
} queue;

void enqueue_q(queue **head, queue *temp);
void enqueue(queue **head, pcb_t *pcb);
queue *dequeue(queue **head);
int size(queue *head);
void remove_by_pid(queue **head, int pid);

#endif