#  Scheduler and Process Queue Documentation

## Queue Functions

The process queue(s) structure is discussed in depth in the README.

### set_quanta
**Synopsis**

Set time quanta for Processes in a queue

void set_quanta(queue \*head, int quanta)

**Description**

Sets how many runs the processes a queue can undergo within each of their PCB's.

**Return Value**

This function is a void function, so nothing is returned.

### add_to_active
**Synopsis**

Set inactive processes to active

void add_to_active()

**Description**

Iterates through all the queue nodes of inactive processes and set them to active by moving them to the queue of active processes.

**Return Value**

This function is a void function, so nothing is returned.

### add_to_inactive
**Synopsis**

Add a process to the Inactive queue

void add_to_inactive(pcb_t \*pcb)

**Description**

Takes in a PCB *pcb* and adds it onto queue node to be placed at the end of the queue of inactive processes.

**Return Value**

This function is a void function, so nothing is returned.

### set_time
**Synopsis**

Set Time for Queues Based on Priority

void set_time()

**Description**

Depending on how many queues are present, set\_time() determines the time quanta for the process such that each subsequent process

**Return Value**

This function is a void function, so nothing is returned.

## next_to_schedule
**Synopsis**

Get the next PCB to Schedule

pcb_t \*next_to_schedule()

**Description**

Checks for any inactive process to be scheduled and renders them active if so. Searched for the first process in the highest-priority active queue. If its run count has not yet equaled its quanta, then it gets moved to the end of that active queue. Otherwise, the process gets moved to the the inactive queue. The PCB of this process gets returned to be scheduled.

**Return Value**

Returns the PCB of the process that is ready to be scheduled. Returns NULL if there are no inactive process queues, no processes in the queues, or if the process is zombied.

## scheduler_logout
**Synopsis**

Handles the Scheduler Logout

void scheduler_logout()

**Description**

Frees all heap-allocated memory associated to the scheduler's queue of active and inactive queues.

**Return Value**

This function is a void function, so nothing is returned.

## Process Queue Functions

### enqueue_q
**Synopsis**

Add a queue node to the end of a Queue

void enqueue_q(queue \*\*head, queue \*temp)

**Description**

Places the queue node indicated by *temp* onto the end of the queue indicated by *head*.

**Return Value**

This function is a void function, so nothing is returned.

### enqueue
**Synopsis**

Add a process to the end of a Queue

void enqueue(queue \*\*head, pcb_t \*pcb)

**Description**

Creates a queue node that references the PCB of the process to be enqueued and places this node at the end of the queue indicated by *head*.

**Return Value**

This function is a void function, so nothing is returned.

### dequeue
**Synopsis**

Take a process off the Queue

queue \*dequeue(queue \*\*head)

**Description**

Removes the queue node at the front of the queue off the queue.

**Return Value**

Returns the queue node of the recently popped node. Returns NULL if the queue is empty.

### remove_by_pid
**Synopsis**

Remove a process from Queue by PID

void remove_by_pid(queue \*\*head, int pid);

**Description**

Given a process denoted by PID *pid*, remove this process from the queue indicated by *head*.

**Return Value**

This function is a void function, so nothing is returned.

### size
**Synopsis**

Get the size of the queue

int size(queue \*head)

**Description**

Iterates through the nodes of the queue indicated by *head* and counts the number of nodes in the inputted queue.

**Return Value**

Return an non-negative number corresponding to the length of the queue.
