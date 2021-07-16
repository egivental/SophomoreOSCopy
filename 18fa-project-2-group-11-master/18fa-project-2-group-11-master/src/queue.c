#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

void enqueue_q(queue **head, queue *temp) {
	if (*head != NULL) {
		queue *ptr = *head;
		while (ptr->next != NULL) {
			ptr = ptr->next;
		}
		ptr->next = temp;
	}
	else {
		*head = temp;
	}
}

void remove_by_pid(queue **head, int pid) {
	if (head != NULL) {
		queue *curr = *head;
		queue *prev = NULL;
		while (curr != NULL) {
			if (curr->pcb->pid == pid && prev == NULL) {
				*head = curr->next;
				free(curr);
			}
			else if (curr->pcb->pid == pid && prev != NULL) {
				prev->next = curr->next;
				curr->next = NULL;
				free(curr);
			}
			prev = curr;
			curr = curr->next;
		}
	}
}

void enqueue(queue **head, pcb_t *pcb) {
	queue *q;
	q = (queue *)calloc(1, sizeof(queue));
	q->pcb = pcb;
	q->next = NULL;
	queue *ptr = *head;
	if (ptr != NULL) {
		while (ptr->next != NULL) {
			ptr = ptr->next;
		}
		ptr->next = q;
	}
	else {
		*head = q;
	}

}

queue *dequeue(queue **head) {
	if (head == NULL) {
		return NULL;
	}
	else {
		queue *curr = *head;
		*head = curr->next;
		curr->next = NULL;
		return curr;
	}
}

int size(queue *head) {
	if (head == NULL)  {
		return 0;
	}
	queue *curr = head;
	int length = 0;
	while (curr != NULL) {
		length++;
		curr = curr->next;
	}
	return length;
}

