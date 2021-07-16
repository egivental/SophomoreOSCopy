# Kernel and Shell Documentation

## List of Shell Functions
Required:
* cat \[fname] (S\*) cat a file to stdout
* nice priority command [arg]\ (S\*) set the priority level of the command to priority
and execute the command
* sleep n (S\*) sleep for n seconds
* busy (S\*) busy wait indefinitely
* ls (S\*) list all files in the file system and relevant information
* touch file (S\*) create a zero size file file if it does not exist
* rm file (S\*) remove a file file
* ps (S\*) list all running processes on PennOS
* nice pid priority pid (S) adjust the nice level of process pid to priority priority
* man (S) list all available commands
* bg \[pid] (S) continue the last stopped thread or the thread pid
* fg \[pid] (S) bring the last stopped or backgrounded thread to the foreground or the thread specified by pid
* jobs (S\*) list current running processes in the shell
* logout (S) exit the shell, and shutdown PennOS

Extra:
* Extra 1 - mv - move/rename a file
* Extra 2 - cp - make a copy of file
* Extra 3 - df - display space left on file system (in blocks)

## Kernel - User Functions

### p_spawn
**Synopsis**

Fork a new child thread.

pid\_t p\_spawn(void \*func, char \*cmd, int argc, char \*argv[], int state)

**Description**

p\_spawn() forks a new thread that retains most of the attributes of the parent thread. Once the thread is spawned, it will execute the function referenced by _func_. The function will have name _cmd_ and _argc_ number of arguments which are stored in the array _argv_. The new thread will be specified to run in the foreground or background by _state_.

**Return Value**

On success, returns a new PID. Exits on error.


### p_kill
**Synopsis**

Kills a thread.

p\_kill(int pid, int signal)

**Description**

p\_kill() kills the thread referenced by _pid_ with the signal _signal_.

**Return Value**


### p_wait
**Synopsis**

Wait until a child of calling thread changes state.

wait\_t\* p\_wait(int mode);

**Description**

p\_wait() sets the calling thread as blocked (and does not return) until a child of the calling thread changes state. The mode argument should be used to indicate a NOHANG condition. In this case, p\_wait() should not block and should return NULL immediately if there are no child threads to wait on. If the calling thread has no children, p\_wait() should return NULL immediately.

**Return Value**

p\_wait() returns a structure, _wait\_t_, with two fields: pid_t _pid_, the process id of the child thread that changed state, and status_e (an enum) _status_, indicating the state of the child.


### p_exit
**Synopsis**

Exits the current thread.

p\_exit();

**Description**

Exits the current thread unconditionally.

**Return Value**

Returns 1 on success, -1 on error.


### p_nice
**Synopsis**

Set the priority level of a thread.

p\_nice(int pid, int priority);

**Description**

p\_nice() sets the level of the thread _pid_ to _priority_.

**Return Value**

Returns 1 on success, -1 on error.


### p_info
**Synopsis**

Returns information about a thread.

p\_info(int pid);

**Description**

p\_info() returns information about the thread referenced by _pid_.

**Return Value**

Returns a structure, info\_t, which contains the following fields: status_e (an enum) _status_, indicating the state of the child, char* _command_, the name of the command executed by the thread (e.g., cat), and int _priority_, indicating the priority level of the thread _pid_.


### p_sleep
**Synopsis**

Blocks the thread for a certain number of ticks.

**Description**

p\_sleep(int ticks);

p\_sleep() sets the thread pid to blocked until _ticks_ of the clock occur, and then sets the thread to running.

**Return Value**

Void.

### Status Functions
**Synopsis**

Checks the status to see if a thread is exited, stopped, continued, or terminated.

**Description**

int W\_WIFEXITED(status_e status);

int W\_WIFSTOPPED(status_e status);

int W\_WIFCONTINUED(status_e status);

int W\_WIFSIGNALED(status_e status);

W\_WIFEXITED() returns true if the child terminated normally, that is, by a call to p exit or by returning. W\_WIFSTOPPED() returns true if the child was stopped by a signal. W\_WIFCONTINUED() returns true if the child was continued by a signal. W\_WIFSIGNALED() returns true if the child was terminated by a signal, that is, by a call to p kill with the S SIGTERM signal.

**Return Value**

Returns 1 when _status_ corresponds to the function's status, 0 otherwise.


## Kernel - Kernel Functions

### k\_process\_create
**Synopsis**

Creates a new child thread and PCB.

pid_t k\_process\_create(void \*func, char \*cmd, int num_args, char \*args[], int fg_bg)

**Description**

Create a new child thread and associated PCB. The new thread retains most properties of the parents, but will execute the function referenced by _func_. The function will have name _cmd_ and _argc_ number of arguments which are stored in the array _argv_. The new thread will be specified to run in the foreground or background by _state_.

**Return Value**

Returns a pid_t that is the pid of the new child thread, with which you can access the PCB through the PCB table.

### k\_process\_kill
**Synopsis**

Kill a process

k\_process\_kill(pcb t * process, int signal)

**Description**
Kill the process referenced by *process* with the signal *signal*.


**Return Value**

### k\_process\_terminate
**Synopsis**

Handle a terminated/returned process

k\_process\_terminate(pcb t \* process)

**Description**

 (K) called when a thread returns or is terminated.
This will perform any necessary clean up, such as, but not limited to: freeing memory, setting the status of the child, etc.

**Return Value**


### k\_process\_wait
**Synopsis**


**Description**
wait_t \*k\_process\_wait(int mode);

**Return Value**


### Helper Functions - Kernel Level
void sig\_alrm\_handler();

void start\_timer();

void pause\_timer();

void go\_to\_scheduler();

void init\_kern();

