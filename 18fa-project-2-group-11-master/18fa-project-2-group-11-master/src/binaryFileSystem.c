#include "kernelFileSystem.h"
#include "binaryFileSystem.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static struct filedescriptor* fdtable;

const char* READ = "r";
const char* WRITE = "w";
const char* APPEND = "a";

const int TRUE = 1;
const int FALSE = 0;


void k_initKERNELFdTable(){
	fdtable = malloc(sizeof(filedescriptor));
	fdtable->position = 0;
	fdtable->mode = F_READ;
	fdtable->number = STDIN;
	fdtable->fileP = NULL;  //because stdin is not in the flatfat
	fdtable->next = malloc(sizeof(filedescriptor));
	struct filedescriptor* stdout = fdtable->next;
	stdout->position = 0;
	stdout->mode = F_WRITE;
	stdout->next = NULL;
	stdout->number = STDOUT;
	stdout->fileP = NULL;  //because stdout is not in the flatFat
	fdtable->next = stdout;
}

void k_filesLogout(){
	struct file* next = root->next;
	while(next){
		root->next = next->next;
		free(next);
		next = root->next;
	}
	free(root);
}

void k_fileDescriptorLogout(){
	struct filedescriptor* next = fdtable->next;
	while(next){
		fdtable->next = next->next;
		free(next);
		next = fdtable->next;
	}
	free(fdtable);
}

void b_fileSystemLogout(){
	k_filesLogout();
	k_fileDescriptorLogout();
}

int b_open(char* name, int mode)
{
	file* np;
	if ((mode == F_WRITE) || (mode == F_APPEND)){
		np = k_open(name,TRUE);
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
	struct filedescriptor* cur = fdtable;
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
	k_storeDirectory();

	return newFD->number;

}

int b_write(int fd, char* str, int numb){
	filedescriptor* filed = fdtable;
	while(filed){
		if (filed->number == fd){
			break;
		}
		filed=filed->next;
	}
	if (!filed){
		printf("File descriptor is closed\n");
		return FILENOTFOUND;
	}
	if(filed->mode == F_READ){
		printf("Tried to Write to a Read File Descriptor");
		return INVALID_WRITE;
	}
	if (filed->mode == F_APPEND){
		filed->position = filed->fileP->size;
	}
	return k_write(filed, str, numb);
	k_storeDirectory();
}

int b_close(int fd){
	filedescriptor* filed = fdtable->next->next;
	filedescriptor* temp = fdtable->next;
	if (fd == 0 || fd == 1){
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


int b_read(int fd,  char* str, int n){
	filedescriptor* filed = fdtable; //Make this point at those PIDS
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
	k_storeDirectory();
	return k_read(filed, str, n);
}

int b_lseek(int fd, int offset, int whence){
	filedescriptor* filed = fdtable;
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



