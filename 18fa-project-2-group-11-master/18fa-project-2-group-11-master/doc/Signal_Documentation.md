#  Signals and Signal Queue Documentation

## Signals

### S_SIGSTOP

Stops the thread that receives this signal.

### S_SIGCONT

Continues the thread that receives this signal.

### S_SIGTERM

Terminates the thread that receives this signal.



## Signal Queue Functions

### sigEnqueue
**Synopsis**

Add a signal to the end of the signal Queue

int sigEnqueue(signals_e sig, pid_t pid)

**Description**

Creates a node that takes in the signal *sig* and PID *pid*. sigEnqueue() then adds this node to the end of the the signal Queue.

**Return Value**

Returns 1 on a successful enqueue, and -1 if a calloc error arises

### sigPop
**Synopsis**

signalNode* sigPop()

**Description**

Checks if the signal queue is already empty. If not, the signal node at the front of the queue is popped off and returned.

**Return Value**

Returns the recently popped off signal node. Returns 0 if the signal queue was empty.

