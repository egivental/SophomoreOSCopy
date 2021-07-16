#ifndef _LOGH_
#define _LOGH_

#include <unistd.h>

void init_logging(char *log_name);
void log_event(unsigned long ticks, char *func, pid_t pid, int nice, char *cmd);
void logging_logout();

#endif