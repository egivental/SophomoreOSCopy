#include <stdlib.h>
#include <ucontext.h>
#include <valgrind/valgrind.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "pcb.h"
#include "queue.h"
#include "scheduler.h"
#include "log.h"
#include "kernelFileSystem.h"
#include "userFileSystem.h"

pid_t fg_pid = 1;
pid_t largest_pid = 1;
pcb_t *pcb_table[TABLESIZE];

pcb_t *init;
pcb_t *curr_process_pcb;
unsigned long tick = 0;
void *signal_stack;
unsigned int signal_stack_id = 0;
ucontext_t signal_context; // interrupt
sigset_t set;
ucontext_t *cur_context;


void pause_timer() {
	struct itimerval it;

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	it.it_value = it.it_interval;

	setitimer(ITIMER_REAL, &it, NULL);
}

wait_t *p_wait(int mode) {
	return k_process_wait(mode);
}

void kern_logout(){
	for (int i = 3; i < TABLESIZE; i++){
		if (pcb_table[i]){
			free(pcb_table[i]->cmd);
			struct filedescriptor* check = pcb_table[i]->fdtable;
			struct filedescriptor* temp;
			while(check->next){
				temp = check->next;
				check->next = temp->next;
				free(temp);
			}
			free(check);
			free(pcb_table[i]->uc.uc_stack.ss_sp);
			free(pcb_table[i]);
		}
	}
}

void k_process_terminate(pid_t pid) {
	if (pcb_table[pid] == NULL) {
		return;
	}
	else {
		pcb_t *to_remove = pcb_table[pid];
		remove_from_queues(pid);
		pcb_t *c = to_remove->cpcb;
		while (c != NULL) {
			log_event(tick, "ORPHAN", c->pid, c->nice, c->cmd);
			pcb_t *temp = c->spcb;
			k_process_terminate(c->pid);
			c = temp;
		}
		pcb_table[pid] = NULL;
	}

}

wait_t *k_process_wait(int mode) {
	pause_timer();

	pcb_t *cpcb = curr_process_pcb->cpcb;
	if (curr_process_pcb->cpcb != NULL) {
		curr_process_pcb->old_status = curr_process_pcb->curr_status;
		curr_process_pcb->curr_status = WAITING;
	}
	else {
		curr_process_pcb->old_status = WAITING;
		curr_process_pcb->curr_status = READY;
		start_timer();
		return NULL;
	}

	wait_t *ret = (wait_t *)malloc(sizeof(wait_t));
	if (ret == NULL) {
		perror("malloc failed");
		exit(1);
	}

	if (mode == HANG) {
		while (1) {
			while (cpcb != NULL) {
				if (cpcb->old_status != cpcb->curr_status) {
					if (cpcb->curr_status == ZOMBIED || cpcb->curr_status == SIGNALZOMBIED){
						free(cpcb->cmd);
						struct filedescriptor* check = cpcb->fdtable;
						struct filedescriptor* temp;
						while(check->next){
							temp = check->next;
							check->next = temp->next;
							free(temp);
						}
						free(check);
						struct pcb_t* parent = cpcb->ppcb;
						free(cpcb->uc.uc_stack.ss_sp);
						struct pcb_t* sibling = parent->cpcb;
						if (sibling == cpcb){
							parent->cpcb = sibling->spcb;
						}
						while(sibling->spcb)
						{
							if (sibling->spcb == cpcb){
								sibling->spcb = sibling->spcb->spcb;
								break;
							}
						}
						ret->pid = cpcb->pid;
						ret->status = cpcb->curr_status;
						int storepid = cpcb->pid;
						k_process_terminate(cpcb->pid);	
						free(cpcb);
						pcb_table[storepid] = NULL;
						curr_process_pcb->curr_status = READY;
						start_timer();
						return ret;
					} else {
						curr_process_pcb->old_status = curr_process_pcb->curr_status;
						curr_process_pcb->curr_status = READY;
						ret->pid = cpcb->pid;
						ret->status = cpcb->curr_status;
						start_timer();
						return ret;
					}					
				}
				cpcb = cpcb->spcb;
			}
			cpcb = curr_process_pcb->cpcb;
			go_to_scheduler();
		}
	}
	else { // NOHANG
		while (cpcb != NULL) {
			if (cpcb->curr_status != cpcb->old_status) {
				if (cpcb->curr_status == ZOMBIED || cpcb->curr_status == SIGNALZOMBIED){
					free(cpcb->cmd);
					struct filedescriptor* check = cpcb->fdtable;
					struct filedescriptor* temp;
					while(check->next){
						temp = check->next;
						check->next = temp->next;
						free(temp);
					}
					free(check);
					struct pcb_t* parent = cpcb->ppcb;
					struct pcb_t* sibling = parent->cpcb;
					if (sibling == cpcb){
						parent->cpcb = sibling->spcb;
					}
					while(sibling->spcb) {
						if (sibling->spcb == cpcb){
							sibling->spcb = sibling->spcb->spcb;
							break;
						}
					}
					ret->pid = cpcb->pid;
					ret->status = cpcb->curr_status;	
					int storepid = cpcb->pid;
					k_process_terminate(cpcb->pid);
					free(cpcb);
					pcb_table[storepid] = NULL;
					curr_process_pcb->curr_status = READY;
					start_timer();
					return ret;
				} else {
					ret->pid = cpcb->pid;
					ret->status = cpcb->curr_status;
					cpcb->old_status = cpcb->curr_status;
					start_timer();
					return ret;
				}
			}
			cpcb = cpcb->spcb;
		}
	}
	curr_process_pcb->old_status = curr_process_pcb->curr_status;
	curr_process_pcb->curr_status = READY;
	start_timer();
	return NULL;
}

int k_exit() {
	pause_timer(); // pause the timer
	if(pcb_table[curr_process_pcb->pid] == NULL) { // if the current process does not have an entry in the table, return
		start_timer();
		return -1;
	}
	else {
		// update status
		curr_process_pcb->old_status = pcb_table[curr_process_pcb->pid]->curr_status;
		curr_process_pcb->curr_status = ZOMBIED;

		// if the current process was in the foreground, give terminal control to parent
		if (curr_process_pcb->pid == fg_pid) {
			fg_pid = curr_process_pcb->ppcb->pid;
		}
		// log
		log_event(tick, "EXITED", curr_process_pcb->pid, curr_process_pcb->nice, curr_process_pcb->cmd);
		log_event(tick, "ZOMBIE", curr_process_pcb->pid, curr_process_pcb->nice, curr_process_pcb->cmd);

		// schedule
		go_to_scheduler();
	
		return 1;
	}
}

int p_exit() {
	return k_exit();
}

void k_ps() {
	pause_timer();
	int i = 0;
	while(i < TABLESIZE) {
		char *buf = malloc(100);
		if (pcb_table[i] != NULL) {
			if (pcb_table[i]->curr_status == READY || pcb_table[i]->curr_status == SIGNALREADY) {
				sprintf(buf, "PID: %d, CMD: %s - ready\n", i, pcb_table[i]->cmd);
				f_write(STDOUT, buf, strlen(buf));
			}
			else if (pcb_table[i]->curr_status == ZOMBIED || pcb_table[i]->curr_status == SIGNALZOMBIED) {
				sprintf(buf, "PID: %d, CMD: %s - zombied\n", i, pcb_table[i]->cmd);
				f_write(STDOUT, buf, strlen(buf));
			}
			else if (pcb_table[i]->curr_status == STOPPED || pcb_table[i]->curr_status == SIGNALSTOPPED) {
				sprintf(buf, "PID: %d, CMD: %s - stopped\n", i, pcb_table[i]->cmd);
				f_write(STDOUT, buf, strlen(buf));
			}
			else if (pcb_table[i]->curr_status == WAITING) {
				sprintf(buf, "PID: %d, CMD: %s - waiting\n", i, pcb_table[i]->cmd);
				f_write(STDOUT, buf, strlen(buf));
			}
		}
		free(buf);
		i++;
	}
	start_timer();
}

void p_ps() {
	k_ps();
}

void go_to_scheduler() {
	start_timer();
	getcontext(&signal_context);

	free(signal_stack);
	VALGRIND_STACK_DEREGISTER(signal_stack_id);

	signal_stack = malloc(STACKSIZE);

	unsigned int stackid = VALGRIND_STACK_REGISTER(signal_stack, signal_stack + STACKSIZE);
	signal_stack_id = stackid;

	signal_context.uc_stack.ss_sp = signal_stack;
	signal_context.uc_stack.ss_size = STACKSIZE;
	signal_context.uc_stack.ss_flags = 0;
	sigemptyset(&signal_context.uc_sigmask);
	sigaddset(&signal_context.uc_sigmask, SIGALRM);

	makecontext(&signal_context, k_schedule, 1);

	swapcontext(cur_context, &signal_context);
}

pid_t k_process_create(void *func, char *cmd, int nice, int num_args, char *args[], int fg_bg) {
	pause_timer();

	struct pcb_t *create_pcb = (pcb_t *)malloc(sizeof(pcb_t));
	
	int i = 2;
	while(pcb_table[i]){
		i++;
	}
	create_pcb->pid = i;
	create_pcb->nice = nice;
	create_pcb->cmd = cmd;
	create_pcb->cpcb = NULL;
	create_pcb->spcb = NULL;
	create_pcb->ppcb = curr_process_pcb;
	struct pcb_t *curr = curr_process_pcb->cpcb;
	if (curr != NULL) {
		while (curr->spcb != NULL) {
			curr = curr->spcb;
		}
		curr->spcb = create_pcb;
	} else {
		curr_process_pcb->cpcb = create_pcb;
	}

	create_pcb->old_status = READY;
	create_pcb->fdtable = k_initFdTable();
	create_pcb->curr_status = READY;

	if (curr_process_pcb->pid == fg_pid && fg_bg == FOREGROUND) { // foreground
		fg_pid = create_pcb->pid;
	}

	getcontext(&(create_pcb->uc));

	void *stack;
	if ((stack = malloc(STACKSIZE)) == NULL) {
		perror("malloc failed");
		exit(1);
	}

	unsigned int stackid = VALGRIND_STACK_REGISTER(stack, stack + STACKSIZE);
	create_pcb->stackid = stackid;
	create_pcb->uc.uc_stack.ss_sp = stack;
	create_pcb->uc.uc_stack.ss_size = STACKSIZE;
	create_pcb->uc.uc_stack.ss_flags = 0;
	if (sigemptyset(&(create_pcb->uc.uc_sigmask)) < 0) {
		perror("sigemptyset failed");
		exit(1);
	}
	if (num_args == 0) {
		makecontext(&(create_pcb->uc), func, 1);
	}
	else if (num_args == 1) {
		makecontext(&(create_pcb->uc), func, 2, args[0]);
	}
	else if (num_args == 2) {
		makecontext(&(create_pcb->uc), func, 3, args[0], args[1]);
	}
	
	pcb_table[create_pcb->pid] = create_pcb;

	add_to_inactive(create_pcb);

	pid_t create_pid = create_pcb->pid;
	log_event(tick, "CREATE", create_pid, create_pcb->nice, create_pcb->cmd);
	start_timer();
	return create_pid;
}

pid_t p_spawn(void *func, char *cmd, int nice, int num_args, char *args[], int fg_bg) {
	return k_process_create(func, cmd, nice, num_args, args, fg_bg);
}

info_t *k_info(int pid) {
	pause_timer();

	if(pcb_table[pid] == NULL) {
		perror("PID not in pcb table");
		return NULL;
	}
	else {
		info_t *ret = (info_t *)malloc(sizeof(info_t));

		ret->command = pcb_table[pid]->cmd;
		ret->priority = pcb_table[pid]->nice-1;
		ret->status = pcb_table[pid]->curr_status;

		start_timer();

		return ret;
	}
}

info_t *p_info(int pid) {
	return k_info(pid);
}

int k_nice(pid_t pid, int nice) {
	pause_timer();
	if (pcb_table[pid] == NULL) {
		perror("Invalid pid");
		return -1;
	}
	else {
		pcb_table[pid]->nice = nice;

		start_timer();
		return 1;
	}
}

int p_nice(pid_t pid, int nice) {
	if (pid <= 1 || pid >= TABLESIZE) {
		perror("Invalid pid");
		return -1;
	}
	else {
		if (nice < -1 || nice > 1) {
		nice = 0;
	}

		return k_nice(pid, nice+1);
	}
}

void k_sleep(int ticks) {
	unsigned long curr_tick = tick;
	while (curr_tick + (10 * ticks) > tick) {
		int i = 0;
		while(i < 5000000) {
			i++;
		}
		go_to_scheduler();
	}
	p_exit();
}

void p_sleep(int ticks) {
	if (ticks < 0) {
		perror("Cannot sleep for negative ticks");
		return;
	}
	else {
		k_sleep(ticks);
	}
}

int p_kill(int kill_pid, signals_e signal) {
	return k_process_kill(kill_pid, signal);
}

int k_process_kill(int kill_pid, signals_e signal) {
	pause_timer();
	if(pcb_table[kill_pid] == NULL) {
		start_timer();
		return -1;
	}

	log_event(tick, "SIGNALED", kill_pid, pcb_table[kill_pid]->nice, pcb_table[kill_pid]->cmd);

	if (signal == S_SIGSTOP) {
		if (pcb_table[kill_pid]->curr_status == READY || pcb_table[kill_pid]->curr_status == SIGNALREADY || pcb_table[kill_pid]->curr_status == WAITING) {
			pcb_table[kill_pid]->old_status = pcb_table[kill_pid]->curr_status;
			pcb_table[kill_pid]->curr_status = SIGNALSTOPPED;

			log_event(tick, "STOPPED", kill_pid, pcb_table[kill_pid]->nice, pcb_table[kill_pid]->cmd);
			if (curr_process_pcb->pid == fg_pid) {
				if (curr_process_pcb->ppcb == NULL) {
					fg_pid = SHELLPID;
				}
				else {
					fg_pid = curr_process_pcb->ppcb->pid;
				}
			}
		}
	}
	else if (signal == S_SIGCONT) {
		if (pcb_table[kill_pid]->curr_status == STOPPED || pcb_table[kill_pid]->curr_status == SIGNALSTOPPED) {
			pcb_table[kill_pid]->old_status = pcb_table[kill_pid]->curr_status;
			pcb_table[kill_pid]->curr_status = SIGNALREADY;

			log_event(tick, "CONTINUED", kill_pid, pcb_table[kill_pid]->nice, pcb_table[kill_pid]->cmd);
		}
	}
	else if (signal == S_SIGTERM) {
		if (pcb_table[kill_pid]->curr_status != ZOMBIED && pcb_table[kill_pid]->curr_status != SIGNALZOMBIED) {
			pcb_table[kill_pid]->old_status = pcb_table[kill_pid]->curr_status;
			pcb_table[kill_pid]->curr_status = SIGNALZOMBIED;

			log_event(tick, "ZOMBIE", kill_pid, pcb_table[kill_pid]->nice, pcb_table[kill_pid]->cmd);

			if (curr_process_pcb->pid == fg_pid) {
				if (curr_process_pcb->ppcb == NULL) {
					fg_pid = SHELLPID;
				}
				else {
					fg_pid = curr_process_pcb->ppcb->pid;
				}
			}
		}
	}
	start_timer();
	return kill_pid;
}

void k_schedule() {
	tick++;
	pcb_t *process_to_sched = next_to_schedule();
	if (process_to_sched == NULL) {
		perror("Nothing to schedule\n");
		exit(1);
	}

	log_event(tick, "SCHEDULE", curr_process_pcb->pid, curr_process_pcb->nice, curr_process_pcb->cmd);

	curr_process_pcb = process_to_sched;
	cur_context = &(process_to_sched->uc);

	setcontext(cur_context);
}

void sig_alrm_handler() {
	getcontext(&signal_context);
	free(signal_stack);
	VALGRIND_STACK_DEREGISTER(signal_stack_id);

	if ((signal_stack = malloc(STACKSIZE)) == NULL) {
		perror("malloc failed");
		exit(1);
	}

	signal_stack_id = VALGRIND_STACK_REGISTER(signal_stack, signal_stack + STACKSIZE);
	signal_context.uc_link = cur_context;
	signal_context.uc_stack.ss_sp = signal_stack;
	signal_context.uc_stack.ss_size = STACKSIZE;
	signal_context.uc_stack.ss_flags = 0;
	sigemptyset(&signal_context.uc_sigmask);
	sigaddset(&signal_context.uc_sigmask, SIGALRM);

	makecontext(&signal_context, k_schedule, 1);

	swapcontext(cur_context, &signal_context);
}

void sig_int_handler() {
	if (fg_pid == SHELLPID) {
		return;
	}
	else {
		k_process_kill(fg_pid, S_SIGTERM);
	}
}

void sigtstp_handler() {
	if (fg_pid == SHELLPID) {
		//
	}
	else {
		k_process_kill(fg_pid, S_SIGSTOP);	
	}
}

void setup_signals(void) {

	struct sigaction act;
	act.sa_sigaction = sig_alrm_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART | SA_SIGINFO;

	sigemptyset(&set);
	sigaddset(&set, SIGALRM);

	if (sigaction(SIGALRM, &act, NULL) != 0) {
		perror("sigaction failed");
	}

	struct sigaction sigint_act;
	sigint_act.sa_sigaction = sig_int_handler;
	sigemptyset(&sigint_act.sa_mask);
	sigint_act.sa_flags = SA_RESTART | SA_SIGINFO;

	if (sigaction(SIGINT, &sigint_act, NULL) != 0) {
		perror("sigaction failed");
	}

	struct sigaction sigtstp_act;
	sigtstp_act.sa_sigaction = sigtstp_handler;
	sigemptyset(&sigtstp_act.sa_mask);
	sigtstp_act.sa_flags = SA_RESTART | SA_SIGINFO;

	if (sigaction(SIGTSTP, &sigtstp_act, NULL) != 0) {
		perror("sigaction failed");
	}

}

int W_WIFEXITED(wait_t t) {
	if (t.status == ZOMBIED) {
		return 1;
	}
	else {
		return 0;
	}
}

int W_WIFSTOPPED(wait_t t) {
	if (t.status == SIGNALSTOPPED) {
		return 1;
	}
	else {
		return 0;
	}
}

int W_WIFCONTINUED(wait_t t) {
	if (t.status == SIGNALREADY) {
		return 1;
	}
	else {
		return 0;
	}
}

int W_WIFSIGNALED(wait_t t) {
	if (t.status == SIGNALZOMBIED) {
		return 1;
	}
	else {
		return 0;
	}
}

void start_timer() {
	struct itimerval it;
	setup_signals();
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = INTERVAL * 1000;
	it.it_value = it.it_interval;
	setitimer(ITIMER_REAL, &it, NULL);
}

void init_kern() {

	// create first pcb -- init
	init = (pcb_t*)malloc(sizeof(pcb_t));
	init->pid = 1;
	init->cmd = "init";
	if (getcontext(&(init->uc)) < 0) {
		perror("getcontext failed");
		exit(1);
	}
	init->old_status = WAITING;
	init->curr_status = WAITING;

	void *stack;

	if ((stack = malloc(STACKSIZE)) == NULL) {
		perror("malloc failed");
		exit(1);
	}


	unsigned int stackid = VALGRIND_STACK_REGISTER(stack, stack+STACKSIZE);
	init->stackid = stackid;
	init->uc.uc_stack.ss_sp = stack;
	init->uc.uc_stack.ss_size = STACKSIZE;
	init->uc.uc_stack.ss_flags = 0;
	init->ppcb = NULL;
	init->cpcb = NULL;
	init->spcb = NULL;
	if (sigemptyset(&(init->uc.uc_sigmask)) < 0) {
		perror("sigemptyset failed");
		exit(1);
	}

	cur_context = &(init->uc);


	pcb_table[init->pid] = init;
	curr_process_pcb = init;

	start_timer();
}
