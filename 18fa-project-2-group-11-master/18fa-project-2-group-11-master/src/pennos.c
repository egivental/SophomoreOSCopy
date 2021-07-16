#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "kernel.h"
#include "shell.h"
#include "signals.h"
#include "kernelFileSystem.h"
#include "userFileSystem.h"


int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 3) {
		perror("PennOS takes 1-2 arguments\n");
		exit(1);
	}

	// user specified a log file name
	if (argc == 3) {
		init_logging(argv[2]);
	} else {
		init_logging(NULL);
	}

	// start the kernel
	init_kern();

	char *args[1];
	args[0] = NULL;
	char *cmd = "pennOS";
	k_setFileSystem(argv[1]);
	k_loadDirectory();
	
	p_spawn(shell, cmd, -1, 0, args, FOREGROUND);
	while(1);
}