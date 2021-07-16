#ifndef SIGNALS
#define SIGNALS

#include <sys/time.h>

typedef enum signals {
	S_SIGSTOP,
	S_SIGCONT,
	S_SIGTERM
} signals_e;

typedef struct signalNode{
	signals_e sig;
	pid_t pid;
	struct signalNode* next;
} signalNode;
#endif