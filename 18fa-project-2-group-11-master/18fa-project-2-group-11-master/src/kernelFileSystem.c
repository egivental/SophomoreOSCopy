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
#include <sys/time.h>
#include <fcntl.h>


FAT_T* fat;
struct file* root;
int Memory;
struct filedescriptor* KERNELfdtable;

#define TRUE 1;
#define FALSE 0;



/// INITIALIZERS FOR OS LATER///
FILESTATUS k_initDirectory(){

	root = malloc(sizeof(file));
	if (root == NULL){
		perror("Malloc Error");
		exit(1);

		return FILESYSERROR;
	}
	root->next = NULL;
	memcpy(root->fname, "root", 5);
	root->blockStart = 0;
	root->size = -1;

	return FILEOK;
}

FILESTATUS k_mkFlatFat(char* flatfs){
	Memory = open(flatfs, O_RDWR|O_CREAT, S_IRWXU);
	if (Memory < 0){
		perror("open invalid");
		return FILESYSERROR;
	} else {
		ftruncate(Memory, 0);
		int i = 0;
		char* blank = "\0";
		ftruncate(Memory, FAT_WIDTH * (NUM_BLOCKS+1));
	}
	fat = mmap(NULL, FAT_WIDTH, PROT_WRITE | PROT_READ, MAP_SHARED, Memory, 0);
	if (fat == MAP_FAILED){
		perror("mmap invalid");
	} else {
		for (int i = 0; i-(FAT_WIDTH/2); i++)
		{
			fat[i]= (short) -1;
		}
	}
	close(Memory);
	return FILEOK;
}



filedescriptor* k_initFdTable(){

	struct filedescriptor* fdtable = malloc(sizeof(filedescriptor));
	if (fdtable == NULL){
		perror("Malloc Error");
		exit(1);

		return NULL;
	}
	fdtable->position = 0;
	fdtable->mode = F_READ;
	fdtable->timesOpen = 1;
	fdtable->number = STDIN;
	fdtable->fileP = NULL;  //because stdin is not in the flatfat
	fdtable->next = malloc(sizeof(filedescriptor));
	if (fdtable->next == NULL){
		perror("Malloc Error");
		exit(1);

		return NULL;
	}
	struct filedescriptor* stdout = fdtable->next;
	stdout->position = 0;
	stdout->mode = F_WRITE;
	stdout->next = NULL;
	stdout->timesOpen = 1;
	stdout->number = STDOUT;
	stdout->fileP = NULL;  //because stdout is not in the flatFat
	fdtable->next = stdout;

	return fdtable;
}

FILESTATUS k_setFileSystem(char* flatFat){
    Memory = open(flatFat, O_RDWR, S_IRWXU);
	if (Memory < 0){
		perror("making a new flatfat");
		if (k_mkFlatFat(flatFat) == FILESYSERROR){
			return FILESYSERROR;
		}
		Memory = open(flatFat, O_RDWR, S_IRWXU);
	}
	fat = mmap(NULL, FAT_WIDTH, PROT_WRITE | PROT_READ, MAP_SHARED, Memory, 0);

	return FILEOK;
}


/// BASIC FILE STRUCTURE FUNCTIONS ////
file* k_addFile(char* fname){

	file* cur = root;
	while(cur->next){
		cur = cur->next;
	}
	struct file* newFile = malloc(sizeof(file));
	int length = strlen(fname) + 1;
	if (length > FILE_NAME_LENGTH){
		length = FILE_NAME_LENGTH;
	}
	memcpy(newFile->fname, fname, length);
	fname[FILE_NAME_LENGTH-1] = 0;
	newFile->next = NULL;
	newFile->size = 0;
	newFile->blockStart = k_determineBlock();
	cur->next = newFile;

	return newFile;
}

int k_determineBlock(){

	for(int i = 0; i-NUM_BLOCKS; i++){
		if(k_blockFree(i)){
			return i;
		}
	}
	return MEMFULL;
}

int k_blockFree(int n){

	file* cur = root;
	if (n >= NUM_BLOCKS){

		return FALSE;
	}
	while(cur){
		int m = cur->blockStart;
		while(m != -1){
			if (m == n){

				return FALSE;
			}
			m = fat[m];
		}
		cur = cur->next;
	}
	return TRUE;
}


/// FILE DESCRIPTOR OPERATIONS ///
file* k_open(char* name, int mode){

	struct file* cur = root;
	while(cur){
		if (!strncmp(cur->fname, name, FILE_NAME_LENGTH)){

			return cur;
		}
		cur = cur->next;
	}
	if (mode){

		return k_addFile(name);
	}

	return NULL;
}

int k_read(filedescriptor* filed, char* str, int numb){
	int justRead;
	if(filed->number == STDOUT || filed->fileP == NULL){
		justRead = read(0, str, numb);
		if (justRead < 0){
			perror("write invalid");

			return FILESYSERROR;
		}

		return justRead;
	}
	int readBlockStart = (filed->fileP)->blockStart;
	int blocknum = (filed->position)/FAT_WIDTH;
	int haveRead = 0;
	int canFit;
	int newBlock;
	int EOFbool = FALSE;
	if (filed->fileP->size == filed->position){

		return 0;
	}

	if (filed->fileP->size == filed->position){

		return 0;
	}
	if (numb > (filed->fileP->size - filed->position)){
		numb = filed->fileP->size - filed->position;
	}
	while(blocknum){
		readBlockStart = fat[readBlockStart];
		blocknum--;
	}
	lseek(Memory, (FAT_WIDTH * (readBlockStart + 1)) + (filed->position % FAT_WIDTH), SEEK_SET);
	while(numb)
	{
		if (((filed->position % FAT_WIDTH) + numb) < FAT_WIDTH) {
			justRead = read(Memory, str + haveRead, numb);
			if (justRead < 0){
				perror("read invalid");
			}
			haveRead += justRead;
			filed->position += justRead;
			return haveRead;

		}
		canFit = FAT_WIDTH - (filed->position % FAT_WIDTH);
		justRead = read(Memory, str + haveRead, canFit);
		if (justRead < 0){
			perror("read invalid");
		}
		haveRead += justRead;
		filed->position += justRead;
		if (justRead < canFit){
			if (filed->fileP->size < filed->position){
				filed->fileP->size = filed->position;
			}

			return haveRead;
		}
		numb -= justRead;
		readBlockStart = fat[readBlockStart];
		lseek(Memory, FAT_WIDTH * (readBlockStart + 1), SEEK_SET);
	}

	return haveRead;
}

int k_write(filedescriptor* filed, char* str, int numb){
	int justWritten = 0;
	if(filed->number == STDOUT && filed->fileP == NULL){
		justWritten = write(1, str, numb);
		if (justWritten < 0){
			perror("write invalid");

			return FILESYSERROR;
		}

		return justWritten;
	}
	int writeBlockStart = (filed->fileP)->blockStart;
	int blocknum = (filed->position)/FAT_WIDTH;
	int written = 0;
	int canFit;
	int newBlock;


	while(blocknum){
		writeBlockStart = fat[writeBlockStart];
		blocknum--;
	}
	lseek(Memory, (FAT_WIDTH * (writeBlockStart + 1)) + (filed->position % FAT_WIDTH), SEEK_SET);

	while(numb){
		if (((filed->position % FAT_WIDTH) + numb) < FAT_WIDTH ) {
			justWritten = write(Memory, str + written, numb);
			if (justWritten < 0){
				perror("write invalid");
			}
			written += justWritten;
			filed->position += justWritten;
			if (filed->position > filed->fileP->size){
				filed->fileP->size = filed->position;
			}
			return written;
		}
		canFit = FAT_WIDTH - (filed->position % FAT_WIDTH);
		justWritten = write(Memory, str + written, canFit);
		if (justWritten < 0){
			perror("write invalid");
		}
		written += justWritten;
		filed->position += justWritten;
		if (justWritten < canFit){
			if (filed->fileP->size < filed->position){
				filed->fileP->size = filed->position;
			}

			return written;
		}
		numb -= justWritten;
		newBlock = k_determineBlock();
		if (newBlock == MEMFULL){
			if (filed->fileP->size < filed->position){
				filed->fileP->size = filed->position;
			}

			return written;
		}
		fat[writeBlockStart] = newBlock;
		writeBlockStart = newBlock;
		lseek(Memory, FAT_WIDTH * (writeBlockStart + 1), SEEK_SET);
	}

	return written;
}


char* k_ls(){
	char* lsout = malloc(3000);
	if (lsout == NULL){
		perror("malloc error");
	}
	file* cur = root->next;
	if (cur == NULL){
		sprintf(lsout ,"directory is empty\n");
	}
	int i = 0;
	while(cur){
		i+= sprintf(lsout + i, "%s    size: %d   block: %d\n", cur->fname, cur->size, cur->blockStart);
		cur = cur->next;
	}
	return lsout;
}

/// DIRECTORY STORE AND LOAD OPERATIONS ///

FILESTATUS k_loadDirectory()
{

	int seekReturn = lseek(Memory, FAT_WIDTH, SEEK_SET);
	if (seekReturn < 0){
		perror("invalid lseek");

		return FILESYSERROR;
	}
	char letter;
	if(read(Memory, &letter, sizeof(char)) < 0){
		perror("read error: ");

		return FILESYSERROR;
	}
	int readb = 1;
	int state = 0;
	int nEOF = TRUE;
	int newBlock = 0;
	root = malloc(sizeof(file));
	root->size = -1;
	file* cur = root;
	int i = 0;
	char* buffer = malloc(FILE_NAME_LENGTH + 1 * sizeof(char));
	buffer[256] = 0;
	int numRead;
	while(nEOF){

		while(i==0 || ((buffer[i-1] != '#') && (buffer[i-1] != 0)))
		{
			numRead = read(Memory, buffer + i, sizeof(char));
			if (numRead == 0){
				break;
			} else if (numRead < 0){
				perror("invalid read");

				return FILESYSERROR;
			}
			i++;
			if (i == FILE_NAME_LENGTH){

				return FILESYSERROR;
			}
			readb++;
			if ((readb % FAT_WIDTH) == 0){
				newBlock = fat[newBlock];
				seekReturn = lseek(Memory, 1024*(newBlock+1), SEEK_SET);
			}
		}
		if (buffer[i-1] == 0){
			nEOF = FALSE;
		} else {
			buffer[i-1] = 0;
		}


		switch(state){
			case 0:
				memcpy(cur->fname, buffer, 256);
				break;
			case 1:
				cur->size = atoi(buffer);
				break;
			case 2:
				cur->blockStart = atoi(buffer);
				if (nEOF){
					cur->next = malloc(sizeof(file));
					cur = cur->next;
				} else {
					cur->next = NULL;
				}

				break;
		}

		state = (state + 1)%3;
		i = 0;
	}
	free(buffer);

	return FILEOK;
}

void k_cleanFat(int blockStart){

	int temp;
	while(fat[blockStart] != -1){
		temp = fat[blockStart];
		fat[blockStart] = -1;
		blockStart = temp;
	}

}


FILESTATUS k_storeDirectory(){

	file* cur = root;
	int seekReturn = lseek(Memory, FAT_WIDTH, SEEK_SET);
	if (seekReturn < 0){
		perror("fseek invalid");
	}
	int bytesWritten = 0;
	int canFit;
	char* Store = malloc(16* sizeof(int) +2 + 256 * sizeof(char));
	int currentBlock = 0;
	int newBlock;
	int writtenFromStore = 0;
	int written;
	file* temp;
	while(cur){
		sprintf(Store, "#%s#%d#%d", cur->fname, cur->size, cur->blockStart);

		while ((bytesWritten % FAT_WIDTH) + strlen(Store + writtenFromStore) + 1> FAT_WIDTH){
			canFit = (FAT_WIDTH - (bytesWritten % FAT_WIDTH));
			written = write(Memory, Store + writtenFromStore, canFit);
			if (written < 0){
				perror("write failed: ");

				return FILESYSERROR;
			}
			bytesWritten += written;
			writtenFromStore += written;
			newBlock = k_determineBlock();
			if (newBlock == MEMFULL){

				return MEMFULL;
			}
			if (newBlock < 0) {
				perror("no more memory in file system: ");
			}
			fat[currentBlock] = newBlock;
			currentBlock = newBlock;
			seekReturn = lseek(Memory, FAT_WIDTH, SEEK_SET);
			if (seekReturn < 0 ){
				perror("fseek invalide: ");
			}
			lseek(Memory, 1024*(newBlock+1), SEEK_SET);
		}
		written = write(Memory, Store + writtenFromStore, strlen(Store+writtenFromStore));
		if (written < 0){
			perror("Write Failed: ");

			return FILESYSERROR;
		}
		bytesWritten += written;
		temp = cur->next;
		cur = temp;
	}
	write(Memory, "\0", 1);
	free(Store);

	return FILEOK;
}

void k_fileSysLogout(){

	struct file* next = root->next;
	while(next){
		root->next = next->next;
		free(next);
		next = root->next;
	}
	free(root);
	close(Memory);

}

void k_defragmentSystem(){
	int correctOrder[NUM_BLOCKS];
	int newFat[NUM_BLOCKS];
	for (int i = 0; i< NUM_BLOCKS; i++){
		correctOrder[i] = -1;
		newFat[i] = -1;
	}
	int i = 0;
	int newplace = 0;
	struct file* check = root;
	while(check){
		i = check->blockStart;
		do {
			correctOrder[i] = newplace;
			i = fat[i];
			newplace++;
			
		} while(i != -1);
		newFat[newplace] = -1;
		check = check->next;
	}
	for (i = 0; i< NUM_BLOCKS; i++){
		if (correctOrder[i] == -1){
			correctOrder[i] = i;
		}
	}
	i = 0;
	char buf[FAT_WIDTH];
	char store[FAT_WIDTH];
	newplace = 0;
	lseek(Memory, (i + 1)*FAT_WIDTH, SEEK_SET);
	if (read(Memory,  buf, 1024) < 0){
		perror("system read failed");
		exit(1);
	}

	while(check){
		i = check->blockStart;
		do{
			lseek(Memory, FAT_WIDTH * (correctOrder[i] + 1), SEEK_SET);
			if (read(Memory,  store, 1024) < 0){
				perror("system read failed");
				exit(1);
			}
			lseek(Memory, FAT_WIDTH * (correctOrder[i] + 1), SEEK_SET);
			if(write(Memory, buf, 1024) < 0){
				perror("system write failed");
				exit(1);
			}
			memcpy(buf, store, 1024);
			correctOrder[i] = -1;
			i = fat[i];
		} while(i + 1);
		check = check->next;
	}
	memcpy(fat, newFat, 1024);

}
