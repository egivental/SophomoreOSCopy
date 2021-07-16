#ifndef binaryFileSystem
#define binaryFileSystem
#include "kernelFileSystem.h"

///////////        USER FUNCTIONS     /////////////
int b_open(char * fname, int mode); /*(U) open a file name fname with the mode
      mode and return a file descriptor. The allowed modes are as follows: F WRITE - writing and reading,
      truncates if the file exists, or creates it if it does not exist; F READ - open the file for reading only,
      return an error if the file does not exist; F APPEND - open the file for reading and writing but does not
      truncate the file if exists; additionally, the file pointer references the end of the file. f open returns a
      file descriptor on success and a negative value on error.
*/


int b_read(int fd, char* str, int n); /*(U) read n bytes from the file referenced by fd. On return, f read
      returns the number of bytes read, 0 if EOF is reached, or a negative number on error.
   */                                 


int b_write(int fd, char * str, int numb); /*(U) write n bytes of the string referenced
        by str to the file fd and increment the file pointer by n. On return, f write returns the number of
        bytes written, or a negative value on error.
*/

int b_close(int fd); /* (U) close the file fd and return 0 on success, or a negative value on failure.
*/

int b_lseek(int fd, int offset, int whence);

void k_initKERNELFdTable();

void k_filesLogout();
void k_fileDescriptorLogout();
void b_fileSystemLogout();
#endif







