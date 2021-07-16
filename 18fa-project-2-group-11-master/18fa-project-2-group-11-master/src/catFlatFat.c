#include "binaryFileSystem.h"
#include "kernelFileSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define BUFFERSIZE 1025

char buf[1025];
int fd;

void sigHandler(int signo)
{
	if (signo == SIGINT)
	{
		b_close(fd);
		b_fileSystemLogout();
		exit(0);
	}
}

int main(int argc, char** argv)
{
	if (signal(SIGINT, sigHandler) == SIG_ERR) {
		printf("SIGINT not caught");
	}

	if (argc != 4){
		printf("Requires 3 parameters, only given %d\n", argc - 1);
		exit(0);
	}
	k_setFileSystem(argv[1]);
	k_loadDirectory();
	int numRead = 1;
	k_initKERNELFdTable();
	
	buf[1024] = '\0';
	int i = 0;

	if (!strcmp(argv[3], "-r")){
		fd = b_open(argv[2], F_READ);
		if (fd <0){
			printf("This file does not exist\n");
			b_fileSystemLogout();
			return -1;
		}
		numRead= 1;
		while(numRead != 0){
			numRead = b_read(fd, buf, 1024);
			b_write(1, buf, 1024);
		}
		
		b_close(fd);
		b_fileSystemLogout();
		return 0;
	}


	if (!strcmp(argv[3], "-w")){
		fd = b_open(argv[2], F_WRITE);
		while(1){
			numRead = b_read(0, buf, 1);
			if (numRead == 0){
				break;	
			}
			b_write(fd, buf, 1);
			numRead = 1;
		}	
		b_close(fd);
		b_fileSystemLogout();
		return 0;	
	}	

	if (!strcmp(argv[3], "-a")){
		fd = b_open(argv[2], F_APPEND);
		while(1){
			numRead = b_read(0, buf, 1);
			if (numRead == 0){
				break;	
			}
			b_write(fd, buf, 1);
			numRead = 1;
		}	
		b_close(fd);
		b_fileSystemLogout();
		return 0;	
	}
	b_close(fd);
	b_fileSystemLogout();
	return 0;

}