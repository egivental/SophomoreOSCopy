#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scheduler.h"
#include "queue.h"



queue *active[NUM_QUEUES];
queue *inactive[NUM_QUEUES];

int curr_q = -1;

void set_quanta(queue *head, int quanta) {
	queue *ptr;
	ptr = head;
	while (ptr != NULL) {
		if (ptr->pcb != NULL) {
			ptr->pcb->runs_left = quanta;
			ptr = ptr->next;
		}
	}
}

void add_to_active() {
	int q = 0;
	for (q = 0; q < NUM_QUEUES; q++) {
		queue *curr = inactive[q];
		if (curr == NULL) {
			continue;
		}
		else {
			enqueue_q(&active[q], inactive[q]);
			inactive[q] = NULL;
		}
	}
}

void add_to_inactive(pcb_t *pcb) {
	enqueue(&inactive[pcb->nice + 1], pcb);
}

void remove_from_queues(int pid) {
	for (int i = 0; i < NUM_QUEUES; i++) {
		remove_by_pid(&active[i], pid);
		remove_by_pid(&inactive[i], pid);
	}
}

void set_time() {
	int q;
	queue *nonempty = NULL;

	for (q = 0; q < NUM_QUEUES; q++) {
		if (active[q] == NULL) {
			continue;
		}
		else {
			nonempty = active[q];
		}
	}

	int num_lows = size(nonempty);

	set_quanta(nonempty, 1);

	if (q == 2) {
		int num_mids = num_lows * MULT;
		int num_highs = num_lows * MULT * MULT;

		if (active[1] != NULL) {
			set_quanta(active[1], num_mids / size(active[1]));
		}

		if (active[0] != NULL) {
			set_quanta(active[0], num_highs / size(active[0]));
		}
	}
	else if (q == 1) {
		int num_highs = num_lows * MULT;

		if (active[0] != NULL) {
			set_quanta(active[0], num_highs / size(active[0]));
		}
 	}
}

pcb_t *next_to_schedule() {
	pcb_t *p = NULL;
	while (p == NULL) {
		if (active[0] == NULL && active[1] == NULL && active[2] == NULL) {
			// nothing to schedule
			if (inactive[0] == NULL && inactive[1] == NULL && inactive[2] == NULL) {
				break;
			}
			add_to_active();
			set_time();
		}

		int q;
		queue *dq = NULL;
		for (q = 1; q <= NUM_QUEUES; q++) {
			if (active[(curr_q + q) % NUM_QUEUES] == NULL) {
				continue;
			}
			else {
				curr_q = (curr_q + q) % NUM_QUEUES;
				active[curr_q]->pcb->runs_left--;
				dq = dequeue(&active[curr_q]);
				if (dq->pcb->runs_left > 0 && dq->pcb->curr_status != ZOMBIED) {
					enqueue_q(&active[curr_q], &(*dq));
				}
				else if (dq->pcb->curr_status != ZOMBIED) {
					enqueue_q(&inactive[curr_q], &(*dq));
				}
				break;
			}
		}

		p = dq->pcb;

		if (p->curr_status == SIGNALZOMBIED || p->curr_status == ZOMBIED || p->curr_status == SIGNALSTOPPED || p->curr_status == STOPPED) {
			p = NULL;
		}
		else {
			continue;
		}
	}

	return p;
}

void scheduler_logout(){
	queue* head;
	queue* temp;
	for (int i = 0; i < 3; i++){
		head = active[i];
		if (head != NULL){
			while(head->next){
				temp = head->next;; 
				head->next = temp->next;
				free(temp);
			}
			free(head);
		}
		head = inactive[i];
		if (head != NULL){
			while(head->next){
				temp = head->next;; 
				head->next = temp->next;
				free(temp);
			}
			free(head);
		}
		
	} 
}