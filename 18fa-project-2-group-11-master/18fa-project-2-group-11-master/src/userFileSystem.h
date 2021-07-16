#ifndef userFileSystem
#define userFileSystem 

///////////        USER FUNCTIONS     /////////////
int f_open(char * fname, int mode); /*(U) open a file name fname with the mode
      mode and return a file descriptor. The allowed modes are as follows: F WRITE - writing and reading,
      truncates if the file exists, or creates it if it does not exist; F READ - open the file for reading only,
      return an error if the file does not exist; F APPEND - open the file for reading and writing but does not
      truncate the file if exists; additionally, the file pointer references the end of the file. f open returns a
      file descriptor on success and a negative value on error.
*/


int f_read(int fd, char* str, int n); /*(U) read n bytes from the file referenced by fd. On return, f read
      returns the number of bytes read, 0 if EOF is reached, or a negative number on error.
   */                                 


int f_write(int fd, char * str, int numb); /*(U) write n bytes of the string referenced
        by str to the file fd and increment the file pointer by n. On return, f write returns the number of
        bytes written, or a negative value on error.
*/

int f_close(int fd); /* (U) close the file fd and return 0 on success, or a negative value on failure.
*/

int f_unlink(char * fname);

	/*
	(U) remove the file
*/
int f_lseek(int fd, int offset, int whence);

/* (U) reposition the file pointer for fd to the
        offset relative to whence. You must also implement the constants F SEEK SET, F SEE CUR,
        and F SEEK END, which reference similar file whences as their similarly named counterparts in
        lseek(2).*/

char* f_ls();
void f_logout();
int f_dup(int oldfd, char* fname, int pid);
void f_defragmentSystem();
void f_move(char* fileName, char* newFileName);
void f_cp(char* filename);
#endif