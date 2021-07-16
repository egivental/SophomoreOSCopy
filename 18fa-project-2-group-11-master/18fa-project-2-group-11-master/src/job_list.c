#include "job_list.h"
#include "userFileSystem.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define ID_SIZE 64
#define BUFFER_SIZE 1024

#define STDOUT 1

/**
 * @brief Inits head
 * 
 * @return New head node (initialize fields to 0 or NULL)
 */

struct list_element* init_list_head()
{
	struct list_element *head = malloc(sizeof(struct list_element));
	head->val = 0;
	head->command = "";
	head->stat = "";
	head->next = NULL;
    return head;
}

/**
 * @brief Inserts a node at the head ([new_node] -> [head])
 * 
 * @param head Address of head node
 * @param val Value of new node
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_OKAY) or LIST_HEAD_NULL if the head is NULL
 */

LIST_STATUS insert_head(struct list_element** head, int val, char* command, char* stat)
{
	if (*head == NULL) {
		return LIST_HEAD_NULL;
	}
	struct list_element *new_head = malloc(sizeof(struct list_element));
	new_head->val = val;
	new_head->command = malloc(sizeof(command));
	strcpy(new_head->command, command);
	new_head->stat = stat;
	new_head->next = *head;
	*head = new_head;
    return LIST_OKAY;
}

/**
 * @brief Inserts a node at the tail ([head] -> ... -> [new_node])
 * 
 * @param head Head node
 * @param val Value of new node
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_OKAY) or LIST_HEAD_NULL if the head is NULL

 */

LIST_STATUS insert_tail(struct list_element* head, int val, char* command, char* stat)
{
	char cmd[BUFFER_SIZE];
	strcpy(cmd, command);
    if (head == NULL) {
    	return LIST_HEAD_NULL;
    }
    struct list_element *tail = malloc(sizeof(struct list_element));
    tail->val = val;
    tail->command = strdup(cmd);
	tail->stat = stat;
    tail->next = NULL;
    while (head->next != NULL) {
    	head = head->next;
    }
    head->next = tail;
    return LIST_OKAY;
}

/**
 * @brief Gets the last element
 * 
 * @param head Head node
 * @param tail Tail node to be populated
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_OKAY, LIST_EMPTY, LIST_HEAD_NULL)
 */

LIST_STATUS peek_tail(struct list_element* head, struct list_element** tail)
{
	if (head == NULL) {
		return LIST_HEAD_NULL;
	}
	while (head->next != NULL) {
    	head = head->next;
    }
    tail = &head;
    return LIST_OKAY;
}

/**
 * @brief Pops a node at the head and sets the next node to be head ([xxx-head-xxx] -x> [new_head])
 * 
 * @param head Address of head node
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_OKAY, LIST_EMPTY, LIST_HEAD_NULL)

 */

LIST_STATUS pop_head(struct list_element** head)
{
	if (*head == NULL) {
		return LIST_HEAD_NULL;
	}
	struct list_element* old_head = *head;
	if (old_head->next == NULL) {
		free(old_head);
		return LIST_EMPTY;
	}
    else {
    	*head = old_head->next;
    	free(old_head);
    	return LIST_OKAY;
    }
}

/**
 * @brief Pops a node at the tail and updates the previous node before the tail ([head] -> ... -x> [xxx-node-xxx])
 * 
 * @param head Head node
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_OKAY, LIST_EMPTY, LIST_HEAD_NULL)
 */

LIST_STATUS pop_tail(struct list_element* head)
{
	struct list_element* tail;
	if (head == NULL) {
		return LIST_HEAD_NULL;
	}
	if (head->next == NULL) {
		free(head);
		return LIST_EMPTY;
	}
	while (head->next->next != NULL) {
		head = head->next;
	}
	tail = head->next;
	free(tail->command);
	free(tail);
	head->next = NULL;
    return LIST_OKAY;
}

/**
 * @brief Deletes the first element containing val and fixes the linked list ([head] -> ... -x> [xxx-node-xxx] -x> ...)
 * 
 * @param head Head node
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_OKAY, LIST_ELEMENT_NOT_FOUND, LIST_HEAD_NULL)
 */

LIST_STATUS delete_element(struct list_element** head, int val)
{
	struct list_element* curr = *head;
	if (curr == NULL) {
		return LIST_HEAD_NULL;
	}
	struct list_element* prev = NULL;
	struct list_element* next = curr->next;
	while (next != NULL && curr->val != val) {
		prev = curr;
		curr = curr->next;
		next = curr->next;
	}
	if (curr->val != val && next == NULL) {
		return LIST_ELEMENT_NOT_FOUND;
	}
	else if (curr == *head) {
		return pop_head(head);
	}
	else if (next == NULL) {
		return pop_tail(*head);
	}
    else {
    	prev->next = next;
    	free(curr->command);
    	free(curr);    
    	return LIST_OKAY;	
    }
}

/**
 * @brief Clears all the elements ihe linked list including the head
 * 
 * @param head Head node
 *
 * @return LIST_STATUS to indicate if the function successfully executed (LIST_EMPTY, LIST_HEAD_NULL)
 */


LIST_STATUS clear_list(struct list_element* head)
{
	if (head == NULL) {
		return LIST_HEAD_NULL;
	}
	struct list_element* temp = head->next;
	while (temp != NULL) {
		struct list_element* curr = temp;
		temp = temp->next;
		free(curr->command);
		free(curr);
	}
	free(head);
    return LIST_EMPTY;
}

LIST_STATUS update_status(struct list_element* head, int val, char* stat)
{
	if (head == NULL) {
		return LIST_HEAD_NULL;
	}
	while (head != NULL && head->val != val) {
		head = head->next;
	}
	if (head == NULL) {
		return LIST_ELEMENT_NOT_FOUND;
	}
	head->stat = stat;
    return LIST_OKAY;
}

/**
 * @brief Converts a list status to a string
 * 
 * @param status List status
 *
 * @return string representation of the list status
 */

const char* convert_list_status_to_enum(LIST_STATUS status)
{
	static char str[50];
	switch(status) {
		case LIST_OKAY:
			strcpy(str, "LIST_OKAY");
			break;
    	case LIST_EMPTY:
    		strcpy(str, "LIST_EMPTY");
			break;
    	case LIST_HEAD_NULL:
    		strcpy(str, "LIST_HEAD_NULL");
			break;
   		case LIST_ELEMENT_NOT_FOUND:
   			strcpy(str, "LIST_ELEMENT_NOT_FOUND");
			break;
	}
	return str;
}

/**
 * @brief Prints all the strings to screen and line starts with "List: " and ends with a newline
 * So, for example, if the list contains 2, 3, 6, print on the screen:
 * "List: 2 3 6"
 * 
 * If the head is null just print out "List: "
 * 
 * @param head
 *
 * @return void 
 */

void print_list(struct list_element* head)
{
	if (head != NULL) {
		head = head->next;
	}
	char id[ID_SIZE];
	while (head != NULL) {
		sprintf(id, "%u", head->val);
		f_write(STDOUT, "[", 1);
		f_write(STDOUT, id, strlen(id));
		f_write(STDOUT, "] ", 2);
		f_write(STDOUT, head->command, strlen(head->command));
		f_write(STDOUT, " (", strlen(" ("));
		f_write(STDOUT, head->stat, strlen(head->stat));
		f_write(STDOUT, ")\n", strlen(")\n"));
		head = head->next;
	}
}
