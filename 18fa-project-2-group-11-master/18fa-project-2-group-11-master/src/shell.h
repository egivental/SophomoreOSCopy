#include "job_list.h"

void shell(void);
	//read in stuff and parse it
	//depending on what it is either call the right function as a pspawn, or do the shell built in
	//the first list below is individual functions that will call file system things
	//the second list is built in stuff

// RUN AS PENN OS PROCESSES
void cat(char*fname);
void s_sleep(char *length);
void busy();

void ls();
void touch(char *fname);
void rm(char* file); 
void ps();

// RUN AS BUILT-IN SHELL FUNCTIONS
void nice_pid(int pid, int priority);
void man();
void bg(int pid, list_element* job_list);
void fg(int pid, list_element* job_list);
void logout();
void move();
void orphanify();
void zombify();
