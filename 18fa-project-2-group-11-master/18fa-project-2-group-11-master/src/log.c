#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include "log.h"

#define LINESIZE 100

char *filename = "log";
FILE *fp;

void init_logging(char *log_name) {
	{
   #ifdef __linux__
       mkdir(filename, 777); 
   #else
       _mkdir(filename);
   #endif
   }
	char *dir = calloc(250, sizeof(char));
	memcpy(dir, "log/" , 4);
	if (log_name != NULL) {
		filename = log_name;
		fp = fopen(strcat(dir, filename), "w+");
	} else {
		fp = fopen(strcat(dir, filename), "w+");
	}
	if (!fp){
			perror("fileopen");
		}
	free(dir);
}

void log_event(unsigned long ticks, char *func, pid_t pid, int nice, char *cmd) {
	fprintf(fp, "[%lu]\t%s\t%d\t%d\t%s\n", ticks, func, pid, nice, cmd);
}

void logging_logout()
{
	fclose(fp);
}
