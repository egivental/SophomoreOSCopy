#ifndef kernelFileSystem
#define kernelFileSystem

typedef short FAT_T;
extern struct file *root;
extern FAT_T* fat;
extern int Memory;

typedef enum
{
	FILENOTFOUND = -5,
	MEMFULL,
	FILESYSERROR,
	INVALID_WRITE,
	INVALID_READ,
	FILEOK,
	FILEDELETED,
	FILEMADE
} FILESTATUS;

typedef struct file {
	short blockStart;
	unsigned int size;
	char fname[256] ;
	struct file* next;
} file;

#define FAT_WIDTH 1024
#define NUM_BLOCKS 512
#define FILE_NAME_LENGTH 256
#define STDIN 0
#define STDOUT 1



typedef struct filedescriptor{
	struct file* fileP;
	unsigned int position;
	int mode;
	struct filedescriptor* next;
	int number;
	int timesOpen;
}filedescriptor;

enum{
	F_READ,
	F_WRITE,
	F_APPEND
};

enum{
	F_SEEK_SET,
	F_SEEK_CUR,
	F_SEEK_END
};


///////          KERNEL FUNCTIONS       ////////////
FILESTATUS k_initDirectory();
filedescriptor* k_initFdTable();
struct file* k_addFile(char* fname);
FILESTATUS k_setFileSystem(char* flatFat);
int k_determineBlock();
int k_blockFree(int n);
file* k_open(char* name, int mode);
int k_read(filedescriptor* filed, char* str, int numb);
int k_write(filedescriptor* filed, char* str, int numb);
char* k_ls();
FILESTATUS k_loadDirectory();
void k_cleanFat(int blockStart);
FILESTATUS k_storeDirectory();
void k_fileSysLogout();
void k_defragmentSystem();
FILESTATUS k_mkFlatFat();
int k_mv();
#endif







