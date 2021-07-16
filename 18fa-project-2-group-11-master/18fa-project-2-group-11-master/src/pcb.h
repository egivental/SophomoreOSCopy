#ifndef _PCB_H_
#define _PCB_H_

#include <ucontext.h>
#include <unistd.h>

#include "kernelFileSystem.h"
#include "userFileSystem.h"


typedef enum status {
	READY,
	WAITING,
	SIGNALSTOPPED,
	SIGNALREADY,
	STOPPED,
	ZOMBIED,
	SIGNALZOMBIED
} status_e;

typedef struct pcb_t {
	ucontext_t uc; // context for the process
	pid_t pid;
	struct pcb_t *ppcb; // parent
	struct pcb_t *cpcb; // child
	struct pcb_t *spcb; // sibling
	int nice;
	unsigned int stackid;
	char *cmd;
	int runs_left; // runs left in scheduler
	filedescriptor* fdtable;
	status_e old_status;
	status_e curr_status;
} pcb_t;

#endif