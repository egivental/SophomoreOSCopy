#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "pcb.h"
#include "signals.h"

#define NOHANG 1
#define HANG 0

#define BACKGROUND 1
#define FOREGROUND 0

#define STACKSIZE 4096
#define TABLESIZE 1024
#define SHELLPID 2
#define INTERVAL 100

extern pcb_t *curr_process_pcb;
extern int fg_pid;
extern pcb_t *pcb_table[TABLESIZE];

typedef struct wait_t {
	pid_t pid;
	status_e status;
} wait_t;

typedef struct info_t {
	char *command;
	status_e status;
	int priority;
} info_t;

wait_t *p_wait(int mode);
void kern_logout();
void go_to_scheduler();
pid_t k_process_create(void *func, char *cmd, int nice, int num_args, char *args[], int fg_bg);
pid_t p_spawn(void *func, char *cmd, int nice, int argc, char *argv[], int state);
int p_nice(pid_t pid, int nice);
void p_sleep(int ticks);
int p_exit();
int p_kill(int kill_pid, signals_e signal);
void sig_alrm_handler();
void start_timer();
void pause_timer();
void go_to_scheduler();
void init_kern();
wait_t *k_process_wait(int mode);
void k_schedule();
void p_sleep(int ticks);
int W_WIFEXITED(wait_t t);
int W_WIFSTOPPED(wait_t t);
int W_WIFCONTINUED(wait_t t);
int W_WIFSIGNALED(wait_t t);
int p_exit();
int k_process_kill(int kill_pid, signals_e signal);
void p_ps();
void init_logout();


#endif
