#ifndef JOB_LIST_H
#define JOB_LIST_H

#include <stdlib.h>
#include <stdio.h>

typedef struct list_element
{
    int val;
    char* command;
    char* stat;
    struct list_element* next;
} list_element;

typedef enum
{
    LIST_OKAY,
    LIST_EMPTY,
    LIST_HEAD_NULL,
    LIST_ELEMENT_NOT_FOUND
} LIST_STATUS;

struct list_element* init_list_head();

LIST_STATUS insert_head(struct list_element** head, int val, char* command, char* stat);

LIST_STATUS insert_tail(struct list_element* head, int val, char* command, char* stat);

LIST_STATUS peek_tail(struct list_element* head, struct list_element** tail);

LIST_STATUS pop_head(struct list_element** head);

LIST_STATUS pop_tail(struct list_element* head);

LIST_STATUS delete_element(struct list_element** head, int val);

LIST_STATUS clear_list(struct list_element* head);

LIST_STATUS update_status(struct list_element* head, int val, char* stat);

const char* convert_list_status_to_enum(LIST_STATUS status);

void print_list(struct list_element* head);

#endif