#include <string.h>

#include "shell.h"
#include "tokenizer.h"
#include "signals.h"
#include "kernelFileSystem.h"
#include "userFileSystem.h"
#include "kernel.h"
#include "log.h"
#include "scheduler.h"
#include "job_list.h"

#define BUFFER_SIZE 1024
#define NUM_ARGS 4
#define DEFAULT_PRIORITY 1

#define OUT 1
#define IN 2
#define BOTH 3

// helper function to determine if redirect instruction
// returns 0 if no redirect, OUT if >, IN if < and BOTH if > < or < >. -1 if invalid
int is_redirect_instruction(char* command) {
	char* token;
	int in = 0;
	int out = 0;
	TOKENIZER *tokenizer = init_tokenizer(command);
	while((token = get_next_token(tokenizer)) != NULL) {
		if (strcmp(">", token) == 0) {
			free(token);
			if (out > 0) {
				return -1;
			}
			out++;
		}
		else if (strcmp("<", token) == 0) {
			free(token);
			if (in > 0) {
				return -1;
			}
			in++;
		}
		else {
			free(token);
		}
	}
	free_tokenizer(tokenizer);
	if (in && out) {
		return BOTH;
	}
	else if (out) {
		return OUT;
	}
	else if (in) {
		return IN;
	}
	else {
		return 0;
	}
}

// RUN AS PENN OS PROCESSES
void cat(char* fname) {
	char* output = malloc(BUFFER_SIZE);
	for (int i = 0; i < BUFFER_SIZE; i++) {
		output[i] = '\0';
	}
	int fd = f_open(fname, F_READ);
	if (fd < 0) {
		perror("Error opening file");
	}
	while (f_read(fd, output, BUFFER_SIZE) > 0) {
		f_write(STDOUT, output, strlen(output));
	}
	f_close(fd);
	free(output);
	p_exit();
}

void s_sleep(char *length) {
	int n = strtol(length, NULL, 10);
	p_sleep(n);
	p_exit();
}

void zombie_child() {
	p_exit();
}

void zombify() {
	char *args[1];
	args[0] = NULL;
	p_spawn(zombie_child, "zombie_child", 0, 0, args, FOREGROUND);
	while(1);
	p_exit();
}

void orphan_child() {
	while(1);
}

void orphanify() {
	char *args[1];
	args[0] = NULL;
	p_spawn(orphan_child, "orphan_child", 0, 0, args, FOREGROUND);
	p_exit();
}

void busy() {
	while (1);
}

void ls() {
	char* top = f_ls();
	f_write(1, top, strlen(top));
	free(top);
	p_exit();
}

void touch(char *fname) {
	int fd = f_open(fname, F_WRITE);
	if (fd < 0) {
		perror("Error opening file");
	}
	f_close(fd);
	p_exit();
}

void rm(char* file) {
	int status = f_unlink(file);
	if (status < 0) {
		perror("Error removing file");
	}
	p_exit();
} 

void cp(char* filename){
	f_cp(filename);
}

void ps() {
	p_ps();
	p_exit();
}

// RUN AS BUILT-IN SHELL FUNCTIONS
void nice_pid(int pid, int priority) {
	p_nice(pid, priority);
}

void man() {
	int num_commands = 16;
	char *commands[num_commands];
	commands[0] = "cat [fname]\n";
	commands[1] = "nice priority command [arg]\n";
	commands[2] = "sleep n\n";
	commands[3] = "busy\n";
	commands[4] = "ls\n";
	commands[5] = "touch file\n";
	commands[6] = "rm file\n";
	commands[7] = "ps\n";
	commands[8] = "nice_pid priority pid\n";
	commands[9] = "man\n";
	commands[10] = "bg [pid]\n";
	commands[11] = "fg [pid]\n";
	commands[12] = "jobs\n";
	commands[13] = "logout\n";
	commands[14] = "mv\n";
	commands[15] = "cp\n";
	commands[16] = "df\n";
	for (int i = 0; i <= num_commands; i++) {
		f_write(STDOUT, commands[i], strlen(commands[i]));
	}

}

void bg(int pid, list_element* job_list) {
	// no pid specified, find last stopped process in job_list
	if (pid < 0) {
		while (job_list != NULL) {
			if (strcmp(job_list->stat, "stopped") == 0) {
				pid = job_list->val;
			}
			job_list = job_list->next;
		}
	}
	p_kill(pid, S_SIGCONT);
}

void fg(int pid, list_element* job_list) {
	// no pid specified, find last stopped process in job_list
	if (pid < 0) {
		while (job_list != NULL) {
			if (strcmp(job_list->stat, "stopped") == 0) {
				pid = job_list->val;
			}
			job_list = job_list->next;
		}
	}
	p_kill(pid, S_SIGCONT);
}

void move(char* fileName, char* newFileName){
	f_move(fileName, newFileName);
}

void defragment(){
	f_defragmentSystem();
}

void logout() {
	f_logout();
	kern_logout();
	scheduler_logout();
	logging_logout();
}

// returns arg_count
int select_function(TOKENIZER * tokenizer, void **func, char *args[]) {
	char* token;
	int arg_count = 0;
	if ((token = get_next_token(tokenizer)) != NULL) {
		if (strcmp(token, "cat") == 0) {
			free(token);
			if ((token = get_next_token(tokenizer)) != NULL) {
				args[0] = token;
				arg_count = 1;
				*func = cat;
			}
		}
		else if (strcmp(token, "sleep") == 0) {
			free(token);
			if ((token = get_next_token(tokenizer)) != NULL) {
				args[0] = token;
				arg_count = 1;
				*func = s_sleep;
			}
		}
		else if (strcmp(token, "busy") == 0) {
			free(token);
			*func = busy;
		}
		else if (strcmp(token, "ls") == 0) {
			free(token);
			*func = ls;
		}
		else if (strcmp(token, "touch") == 0) {
			free(token);
			if ((token = get_next_token(tokenizer)) != NULL) {
				args[0] = token;
				arg_count = 1;
				*func = touch;
			}
		}
		else if (strcmp(token, "rm") == 0) {
			free(token);
			if ((token = get_next_token(tokenizer)) != NULL) {
				args[0] = token;
				arg_count = 1;
				*func = rm;
			}
		}
		else if (strcmp(token, "ps") == 0) {
			free(token);
			*func = ps;
		}
	}
	return arg_count;
}


void shell(void) {
	wait_t * child;
	int pid_to_wait;
	list_element* job_list = init_list_head();
	
	while (1) {
		char *cmd = malloc(BUFFER_SIZE);
		for (int i = 0; i < BUFFER_SIZE; i++) {
			cmd[i] = '\0';
		}
		void *func;
		int mode = FOREGROUND;
		int arg_count = 0;
		int built_in = 0;
		int priority = DEFAULT_PRIORITY;
		char* output_file = NULL;
		char* input_file = NULL;

		// wait for finished processes to return
		while ((child = p_wait(NOHANG)) != NULL) {
			int pid = child->pid;
			if (W_WIFEXITED(*child) || W_WIFSIGNALED(*child)) {
				delete_element(&job_list, pid);
			}
			else if (W_WIFSTOPPED(*child)) {
				update_status(job_list, pid, "stopped");
			}
			else if (W_WIFCONTINUED(*child)) {
				update_status(job_list, pid, "running");
			}
		}

		// prompt for input
		f_write(STDOUT, "penn-os> ", strlen("penn-os> "));
		int read_ret = f_read(STDIN, cmd, BUFFER_SIZE);
		
		if (read_ret == 0) { // EOF
			clear_list(job_list);
			logout();
			free(cmd);
			exit(0);
		}

		int check_redirect = is_redirect_instruction(cmd);
		if (check_redirect < 0) {
			f_write(STDOUT, "Invalid redirect command.\n", strlen("Invalid redirect command.\n"));
			continue;
		}
		
		// check commands
		char* token;
		char *args[NUM_ARGS];
		TOKENIZER *tokenizer = init_tokenizer(cmd);
		if (tokenizer == NULL) {
			printf("Null");
		}
		int pid;
		if ((token = get_next_token(tokenizer)) != NULL) {
			// RUN AS SHELL SUBROUTINE
			if (strcmp(token, "nice_pid") == 0) {
				free(token);
				int pid;
				int priority;
				if ((token = get_next_token(tokenizer)) != NULL) {
					pid = strtol(token, NULL, 10);
					free(token);
				}
				if ((token = get_next_token(tokenizer)) != NULL) {
					priority = strtol(token, NULL, 10);
					free(token);
					nice_pid(pid, priority);
				}
				built_in = 1;
			}
			else if (strcmp(token, "man") == 0) {
				built_in = 1;
				free(token);
				man();
			}

			else if (strcmp(token, "cp") == 0) {
				built_in = 1;
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[0] = token;
					arg_count = 1;
				}
				cp(args[0]);
			}
			else if (strcmp(token, "bg") == 0) {
				built_in = 1;
				free(token);
				int pid;
				if ((token = get_next_token(tokenizer)) != NULL) {
					pid = strtol(token, NULL, 10);
					free(token);
				}
				else {
					pid = -1;
				}
				bg(pid, job_list);
			}
			else if (strcmp(token, "fg") == 0) {
				built_in = 1;
				free(token);
				int pid;
				if ((token = get_next_token(tokenizer)) != NULL) {
					pid = strtol(token, NULL, 10);
					free(token);
				}
				else {
					pid = -1;
				}
				fg(pid, job_list);
			}
			else if (strcmp(token, "jobs") == 0) {
				built_in = 1;
				free(token);
				print_list(job_list);
			}
			else if (strcmp(token, "defragment") == 0) {
				built_in = 1;
				free(token);
				defragment();
			}

			else if (strcmp(token, "mv") == 0) {
				built_in = 1;
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[0] = token;
				}
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[1] = token;
				}
				move(args[0], args[1]);
			}
			else if (strcmp(token, "logout") == 0) {
				free(token);
				free_tokenizer(tokenizer);
				free(cmd);
				logout();
				exit(0);
			}

			// RUN AS PENN OS PROCESSES
			else if (strcmp(token, "cat") == 0) {
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[0] = token;
					arg_count = 1;
					func = cat;
				}
			}
			else if (strcmp(token, "nice") == 0) {
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					priority = strtol(token, NULL, 10);
					free(token);
				}
				arg_count = select_function(tokenizer, &func, args);
			}
			else if (strcmp(token, "sleep") == 0) {
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[0] = token;
					arg_count = 1;
					func = s_sleep;
				}
				else {
					f_write(STDOUT, "Not enough arguments\n", strlen("Not enough arguments\n"));
					built_in = 1;
				}
			}
			else if (strcmp(token, "busy") == 0) {
				free(token);
				arg_count = 0;
				func = busy;
			}
			else if (strcmp(token, "defragment") == 0) {
				free(token);
				arg_count = 0;	
				func = defragment;
			}
			else if (strcmp(token, "ls") == 0) {
				free(token);
				arg_count = 0;
				func = ls;
			}
			else if (strcmp(token, "touch") == 0) {
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[0] = token;
					arg_count = 1;
					func = touch;
				}
				else {
					f_write(STDOUT, "Not enough arguments\n", strlen("Not enough arguments\n"));
					built_in = 1;
				}
			}
			else if (strcmp(token, "rm") == 0) {
				free(token);
				if ((token = get_next_token(tokenizer)) != NULL) {
					args[0] = token;
					arg_count = 1;
					func = rm;
				}
				else {
					f_write(STDOUT, "Not enough arguments\n", strlen("Not enough arguments\n"));
					built_in = 1;
				}
			}
			else if (strcmp(token, "ps") == 0) {
				free(token);
				arg_count = 0;
				func = ps;
			}
			else if (strcmp(token, "zombify") == 0) {
				free(token);
				func = zombify;
				arg_count = 0;
			}
			else if (strcmp(token, "orphanify") == 0) {
				func = orphanify;
				arg_count = 0;
			}
			else {
				f_write(STDOUT, "Not a valid command\n", strlen("Not a valid command\n"));
				built_in = 1;
				free(token);
			}

			// continue since the function was a built-in one
			if (built_in) {
				free(cmd);
				free_tokenizer(tokenizer);
				continue;
			}

			// continue to parse args and call p_spawn
			if (check_redirect > 0) {
				if (check_redirect == OUT) {
					get_next_token(tokenizer);
					output_file = get_next_token(tokenizer);
				}
				else if (check_redirect == IN) {
					get_next_token(tokenizer);
					input_file = get_next_token(tokenizer);
				}
				else if (check_redirect == BOTH) {
					get_next_token(tokenizer);
					input_file = get_next_token(tokenizer);
					get_next_token(tokenizer);
					output_file = get_next_token(tokenizer);
				}
			}
			if ((token = get_next_token(tokenizer)) != NULL && strcmp(token, "&") == 0) {
				free(token);
				mode = BACKGROUND;
			}
			free_tokenizer(tokenizer);
			cmd[strlen(cmd) - 1] = '\0';
			pid_to_wait = p_spawn(func, cmd, priority, arg_count, args, mode);
			if (check_redirect == OUT) {
				if (output_file != NULL) {
					f_dup(STDOUT, output_file, pid_to_wait);
				}
			}
			else if (check_redirect == IN) {
				if (input_file != NULL) {
					f_dup(STDIN, input_file, pid_to_wait);
				}
			}
			else if (check_redirect == BOTH) {
				if (output_file != NULL && input_file != NULL) {
					f_dup(STDIN, input_file, pid_to_wait);
					f_dup(STDOUT, output_file, pid_to_wait);
				}
			}
			insert_tail(job_list, pid_to_wait, cmd, "running");
			if (mode == FOREGROUND) {
				child = p_wait(HANG);
				int pid = child->pid;
				if (W_WIFEXITED(*child) || W_WIFSIGNALED(*child)) {
					delete_element(&job_list, pid);
				}
				else if (W_WIFSTOPPED(*child)) {
					update_status(job_list, pid, "stopped");
				}
				else if (W_WIFCONTINUED(*child)) {
					update_status(job_list, pid, "running");
				}
			} 
		}
		else {

		}
	}
}
