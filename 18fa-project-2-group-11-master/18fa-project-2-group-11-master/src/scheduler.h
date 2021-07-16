#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "queue.h"
#include "pcb.h"



#define NUM_QUEUES 3
#define MULT 1.5


extern queue *active[NUM_QUEUES];
extern queue *inactive[NUM_QUEUES];

void set_quanta(queue *head, int quanta);
void add_to_active();
void add_to_inactive(pcb_t *pcb);
void set_time();
pcb_t *next_to_schedule();
void scheduler_logout();
void remove_from_queues(int pid);

#endif