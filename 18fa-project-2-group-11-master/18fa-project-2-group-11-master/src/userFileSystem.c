#include "userFileSystem.h"
#include "kernelFileSystem.h"
#include "kernel.h"
#include "signals.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0


int f_open(char* name, int mode)
{
	pause_timer();
	file* np;
	if ((mode == F_WRITE) || (mode == F_APPEND)){
		np = k_open(name, TRUE);
	} else {
		np = k_open(name, FALSE);
		if (!np){
			return -1;
		}
	}
	struct filedescriptor* newFD = malloc(sizeof(filedescriptor));

	newFD->fileP = np;

	if (mode == F_APPEND){
		newFD->position = (newFD->fileP)->size;
	} else {
		newFD->position = 0;
		if (mode == F_WRITE){
			k_cleanFat(newFD->fileP->blockStart);
			newFD->fileP->size = 0;
		}
	}

	newFD->mode = mode;
	struct filedescriptor* cur = curr_process_pcb->fdtable;
	while(cur->next && (cur->number==cur->next->number-1)){
		cur = cur->next;
	}
	if (cur->next){
		struct filedescriptor* temp = cur->next;
		cur->next = newFD;
		newFD->next = temp;
	} else {
		cur->next = newFD;
		newFD->next = NULL;
	}
	newFD->number = cur->number +1;
	newFD->timesOpen = 1;
	k_storeDirectory();
	return newFD->number;
}

int f_write(int fd, char* str, int numb){
		



	pause_timer();
	filedescriptor* filed = curr_process_pcb->fdtable;
	while(filed){
		if (filed->number == fd){
			break;
		}
		filed=filed->next;
	}
	if (!filed){
		printf("File descriptor is closed or doesn't exist\n");
		return FILENOTFOUND;
	}
	if(filed->mode == F_READ){
		printf("Tried to Write to a Read File Descriptor\n");
		return INVALID_WRITE;
	}
	if (filed->mode == F_APPEND){
		filed->position = filed->fileP->size;
	}
	k_storeDirectory();

	if ((((filed->number == 0) || (filed->number == 1)) && filed->fileP == NULL) && ((fg_pid != curr_process_pcb->pid) && (curr_process_pcb->pid >3)))
	{
		k_process_kill(curr_process_pcb->pid, S_SIGSTOP);
		return 0;
	}
	return k_write(filed, str, numb);

}

int f_close(int fd){
	pause_timer();
	filedescriptor* filed = curr_process_pcb->fdtable->next->next;
	filedescriptor* temp = curr_process_pcb->fdtable->next;
	if (fd == STDIN || fd == STDOUT){
		printf("can't close stdin or stdout!\n");
		return -1;
	}

	while(filed){
		if (filed->number == fd){
			temp->next = filed->next;
			free(filed);
			return 0;
		}
		temp = filed;
		filed = filed->next;
	}
	k_storeDirectory();
	return -1;
}

int f_unlink(char* fname){
	pause_timer();
	file* checkFiles = root->next;
	file* temp = root;
	while(checkFiles){
		if (!strncmp(checkFiles->fname, fname, 256))
		{
			temp->next = checkFiles->next;
			filedescriptor* filed = curr_process_pcb->fdtable->next;
			filedescriptor* prev = curr_process_pcb->fdtable;
			while(filed){
				if (filed->fileP == checkFiles){
					prev->next = filed->next;
					free(filed);
					filed = prev->next->next;
				} else {
					prev = filed;
					filed = filed->next;
				}
			}
			k_cleanFat(checkFiles->blockStart);
			break;
		}
		temp = checkFiles;
		checkFiles = checkFiles->next;
	}
	if (!checkFiles){
		k_storeDirectory();
		return 1;
	}
	pcb_t* check;
	for (int i = 2; i<TABLESIZE; i++){
		check = pcb_table[i];
		if (check){
			struct filedescriptor* fdcheck = check->fdtable;
			while (fdcheck && fdcheck->fileP == checkFiles){
				struct filedescriptor* temp = fdcheck;
				fdcheck = fdcheck->next;
				free(temp);
			}
			if (!fdcheck){
				free(checkFiles);
				k_storeDirectory();
			}
			struct filedescriptor* fdcheckNext = check->fdtable->next;
			while(fdcheckNext){
				if (fdcheckNext->fileP == checkFiles){
					fdcheck->next = fdcheckNext->next;
					free(fdcheckNext);
					fdcheckNext = fdcheck->next;
				} else {
					fdcheck = fdcheckNext; 
					fdcheckNext = fdcheckNext->next;
				}
			}
		}
	}
	free(checkFiles);
	k_storeDirectory();
	return 1;

}

int f_read(int fd,  char* str, int n){
	pause_timer();
	filedescriptor* filed = curr_process_pcb->fdtable; //Make this point at those PIDS
	while(filed){
		if (filed->number == fd){
			break;
		}
		filed=filed->next;
	}
	if (!filed){
		printf("File Not Found\n");
		return -1;
	}
	if(filed->mode == F_APPEND){
		printf("Tried to Read from a File Descriptor\n");
		return -1;
	}
	if ((((filed->number == 0) || (filed->number == 1)) && filed->fileP == NULL) && ((fg_pid != curr_process_pcb->pid) && (curr_process_pcb->pid > 3)))
	{
		k_process_kill(curr_process_pcb->pid, S_SIGSTOP);
		return 0;
	}


	return k_read(filed, str, n);
}

int f_lseek(int fd, int offset, int whence){
	pause_timer();
	filedescriptor* filed = curr_process_pcb->fdtable;
	while(filed){
		if (filed->number == fd){
			break;
		}
		filed=filed->next;
	}
	if (!filed){
		printf("File Not Found\n");
		return -1;
	}
	switch(whence){
		case F_SEEK_CUR:
			filed->position += offset;
			break;
		case F_SEEK_SET:
			filed->position = offset;
			break;
		case F_SEEK_END:
			filed->position = filed->fileP->size + offset;
			break;
	}
	k_storeDirectory();
	return filed->position;
}

int f_dup(int oldfd, char* fname, int pid){
	pause_timer();
	if (oldfd > 1){
		return -1;
	}
	struct file* check = root;
	while(check){
		if (!strncmp(check->fname, fname, strlen(fname))){
			break;
		}
		check = check->next;
	}
	if(!check){
		check = k_addFile(fname);
	}
	struct filedescriptor* fdtable = pcb_table[pid]->fdtable;
	if (oldfd == 0){
		if (fdtable->fileP != NULL){
			return -1;
		}
		fdtable->fileP = check;
	} else if(oldfd == 1){
		if (fdtable->next->fileP != NULL){
			return -1;
		}
		fdtable->next->fileP = check;
	}
	return 0;
}

char* f_ls(){
	return k_ls();
}

void f_logout(){
	k_fileSysLogout();
}

void f_defragmentSystem(){
	pause_timer();
	k_defragmentSystem();
	start_timer();
}

void f_move(char* fileName, char* newFileName){
	pause_timer();
	file* check = root;
	file* newCheck = root;
	while(check){
		if (!strncmp(check->fname, fileName, FILE_NAME_LENGTH)){
			break;
		}
		check = check->next;
	}

	while(newCheck){
		if (!strncmp(newCheck->fname, newFileName, FILE_NAME_LENGTH)){
			break;
		}
		newCheck = newCheck->next;
	}

	if (check && newCheck){
		char* buf = malloc(sizeof(char) * 1024);
		int fd = f_open(check->fname, F_READ);
		int newfd = f_open(newCheck->fname, F_WRITE);
		while(f_read(fd, buf, 1024)){
			f_write(newfd, buf, 1024);
		}	
		close(fd);
		close(newfd);
		f_unlink(fileName);
	}
	start_timer();
}


void f_cp(char* filename){
	pause_timer();
	char* newFileName = malloc(strlen(filename) + 2);
	memcpy(newFileName, filename,  strlen(filename) + 1);
	strcat(newFileName, "1");
	int fd = f_open(filename, F_READ);
	int newfd = f_open(newFileName, F_WRITE);
	free(newFileName);
	char* buf = malloc(1024);
	
	while(f_read(fd, buf, 1024)){
		f_write(newfd, buf, 1024);
	}	
	f_close(fd);
	f_close(newfd);
	start_timer();
}

