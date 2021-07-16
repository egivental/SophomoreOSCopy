# PennOS API
## File System - User Functions
### f_open

**Synopsis**

Open a File Descriptor (User)

int f_open(char* fname, int mode)

**Description**

Returns a file descriptor (a non-negative integer), which can be used in f_write and f_read calls to write to or read from memory. The file descriptor itself is placed in the pcb file descriptor linked list for the process which is called f_open.

mode can be F_READ, F_WRITE, or F_APPEND

For F_READ the only call that can be made on this File Descriptor is

**Return Value**

On Success, the file descriptor is returned. On Failure, -1 is returned.

### f_close
**Synopsis**

Close a File Descriptor (User)

int f_close(int fd);

**Description**

Closes a file corresponding to inputted file descriptor (that is non-standard input or output).

**Return Value**

On Success, 0 is returned. On Failure, -1 is returned.

### f_read
**Synopsis**

Read from a File (User)

int f_read(int fd, int n);

**Description**

Takes in a file descriptor and an integer value of *n* bytes. *n* bytes will be read from the file referenced by fd.

**Return Value**

On success, the number of bytes read is returned (positive if any were read and 0 if the end of the inputted file was read). On failure, -1 is returned.

### f_write
**Synopsis**

Write to a file (User)

int f_write(int fd, const char * str, int numb);

**Description**

Takes in a file descriptor, a string reference, and an integer value of *numb* bytes. *numb* bytes from the inputted string will be written to the file, and the file pointer of the file corresponding to inputted fd will be offset accordingly by *numb*.

**Return Value**

On success, the number of bytes written to the file is returned. Otherwise, -1 is returned.

### f_lseek
**Synopsis**

Move the file position

int f_lseek(int fd, int offset, int whence);

**Description**

Takes in a file descriptor, an integer offset value of *n*, and a constant *whence*. The file pointer of the inputted file is reposition to the offset based on the inputted *whence*.

whence can be F_SEEK_CUR, F_SEEK_SET, or F_SEEK_END:

* F_SEEK_CUR: Moves the position of the file pointer by *offset* from its current position.
* F_SEEK_SET: Sets the position of the file pointer to *offset* (equivalent to moving the pointer by *offset* from the beginning of the file)
* F_SEEK_END: Offsets the position of the file pointer from the end of the file.


**Return Value**

On success, the position of the file descriptor (as an unsigned int) is returned. On failure, -1 is returned.

### f_unlink
**Synopsis**

Removes the File fname

int f_unlink(const char * fname)

**Description**

Removes the file with name fname from the directory, and closes all file descriptors associated with that file. If the file can not be located in the directory, no file descriptors are closed, no file is removed, and -1 is returned.

**Return Value**

Returns 0 if succesful.
Returns -1 if file not found.

### f_dup
**Synopsis**

Make a file be STDOUT or STDIN

int f_dup(int oldfd, char* fname, int pid)

**Description**

Takes and oldfd, which must be 0 (STDIN) or 1 (STDOUT) and makes it now point to a file. Now, writes to STDIN or STDOUT will write or read from files, and not the terminal.

**Return Value**

Returns 0 for a succesful file descriptor change.
Returns -1 for an unsuccesful file descriptor change.

### f_ls
**Synopsis**

List all files in the directory.

char* f_ls()

**Description**

Traverses through all files in FlatFAT system and displays details on each file in the system (name, size, and block number).

**Return Value**

This function is a void function, so nothing is returned.

### f_logout
**Synopsis**

Log out of the File System (User land)

void f_logout()

**Description**

Free all heap-allocated memory used by the file system prior to logout.

**Return Value**

This function is a void function, so nothing is returned.

### f_defragmentSystem
**Synopsis**

Defragment the File (User land)

void f_defragmentSystem()

**Description**

Reorder the blocks in filesystem such that files are stored in the fewest contiguous regions (files are not located in separate, non-consecutive blocks). The function first determines an accurate ordering of the blocks, with regions ordered by the ordering of the first occurrence of each file's original first block. Then it moves copy of the file information to its new position determined by the defragmentation ordering.

**Return Value**

This function is a void function, so nothing is returned.


## File System - Kernel Functions

### FILESTATUS
Enumerations for Returns in Kernel File System:
* FILENOTFOUND
* MEMFULL
* FILESYSERROR
* INVALID_WRITE
* INVALID_READ
* FILEOK
* FILEDELETED
* FILEMADE


### k_initDirectory
**Synopsis**

Initializes the Directory Structure

void k_initDirectory()

**Description**

Initalizes the root linked list which holds the file pointers.

**Return Value**

Returns FILEOK on Successful Initialization.
Return FILESYSERROR on Unsuccessful Initialization.

### k_initFdTable
**Synopsis**

Creates a new file descriptor table

filedescriptor* k_initFdTable()

**Description**

Initializes a file descriptor table with default values:
* The file pointer will be at position 0.
* It will indicate that is was opened once.

**Return Value**

Returns the pointer to the head of the initialized file descriptor table.
Returns NULL if there was a malloc error.

### k_mkFlatFat
**Synopsis**

Makes a Flat FAT Host OS File for Memory

FILESTATUS k_mkFlatFat(char* flatfs);

**Description**

Creates a new file descriptor table. The only open file descriptors on it are STDIN and STDOUT. This file descriptor's head is returned.

**Return Value**

Returns the pointer to the head of the new file descriptor table.
Returns NULL on error.

### k_addFile
**Synopsis**

Creates a new file

struct file* k_addFile(char* fname)

**Description**

Creates a new file with name *fname* (truncated to the maximum name length of 256) and size 0. It will be assigned the first available block number as its block start.

**Return Value**

Returns the pointer to the newly created file.

### k_setFileSystem
**Synopsis**

Mount a FlatFAT file system

FILESTATUS k_setFileSystem(char* flatFat)

**Description**

Map a PennOS file system directly into memory (a file on the host with the name *flatFat*).

**Return Value**

Returns FILEOK on Successful Initialization.
Return FILESYSERROR on Unsuccessful Initialization.

### k_determineBlock
**Synopsis**

Determine the first available block

int k_determineBlock()

**Description**

Traverses through possible block numbers up to the maximum possible number of block entries, and returns the first number corresponding to a free block.

**Return Value**

Return the block number of the first free block in the file system, and MEMFULL if no blocks are free.


### k_blockFree
**Synopsis**

Checks if a block is free

int k_blockFree(int n);

**Description**

Checks if block number *n* is within range of the maximum number of entries. If so, iterates through all files in system and go through all the blocks of each files to search if the block number is in use.

If the block not in use in any of the files, then it is free.

**Return Value**

Return TRUE if block *n* is free, and FALSE otherwise.

### k_open
**Synopsis**

Open a File Descriptor (Kernel)

file* k_open(char* name, int mode)

**Description**

First searches for a file with a name matching the inputted *name*.
If the name doesn't exist and *mode* is TRUE, a new file titled *name* is created.

mode can be TRUE or FALSE.
TRUE corresponds to an f_open call with permissions to write to or append a file.
TRUE corresponds to an f_open call with permissions to only read from a file.

**Return Value**

Returns NULL if no file was created.
Otherwise, returns the pointer of the open (or newly created file).

### k_read
**Synopsis**

Read from a File (Kernel)

int k_read(filedescriptor* filed, char* str, int numb)

**Description**

Read *numb* bytes from file descriptor *filed* starting at *str*.

**Return Value**

Returns the number of bytes that have been read.
Return FILESYSERROR on an unsuccessful write.

**Notes**

Called from the user file system function f_read().

### k_write
**Synopsis**

Write to a File (Kernel)

int k_write(filedescriptor* filed, char* str, int numb)

**Description**

If writing to STDOUT, then write *numb* bytes of the string *str* to STDOUT.
Otherwise, if writing to a file, write *numb* bytes of the *str* across the blocks of a file.

**Return Value**

Returns FILEOK on Successful Initialization.

Returns the number of bytes written on a successful write to STDOUT or a file.
Return FILESYSERROR on an unsuccessful write.

**Notes**

Called from the user file system function f_write().

### k_ls
**Synopsis**

List all available Files

void k_ls()

**Description**

Traverses through all files in directory (top level of FlatFAT system) and displays details on each file in the system (name, size, and block number).

**Return Value**

This function is a void function, so nothing is returned.

### k_loadDirectory
**Synopsis**

Load Directory from Memory

FILESTATUS k_loadDirectory()

**Description**

Reads from the flat file on the host and loads the bytes as files on our PennOS file system. Returns a file status based on whether files were able to be loaded from memory.

**Return Value**

Returns FILEOK on Successful Load of Directory from Memory.
Returns FILESYSERROR on Unsuccessful Load of Directory from Memory.

### k_cleanFat
**Synopsis**

Clear Up a File from A FAT file system

void k_cleanFat(int blockStart)

**Description**

Starting at *blockStart*, iterate through subsequent blocks of a file and clearing them of any links to other blocks (assign them all a link value of -1).

**Return Value**

This function is a void function, so nothing is returned.

### k_storeDirectory
**Synopsis**

Stores into the Memory

FILESTATUS k_storeDirectory()

**Description**

Writes bytes into memory (the flat file on the host).
Returns a file status based on whether the file was able to be stored in the directory.

**Return Value**

Returns FILEOK on Successful Initialization.
Returns FILESYSERROR on Unsuccessful Initialization.
Returns MEMFULL if all blocks are being used in file system.

### k_fileSysLogout
**Synopsis**

Log out of the File System

void k_fileSysLogout()

**Description**

Free all heap-allocated memory used by the file system prior to logout.

**Return Value**

This function is a void function, so nothing is returned.

### k_defragmentSystem
**Synopsis**

Defragment the File system (Kernel Land)

void k_defragmentSystem()

**Description**

Reorder the blocks in filesystem such that files are stored in the fewest contiguous regions (files are not located in separate, non-consecutive blocks). The function first determines an accurate ordering of the blocks, with regions ordered by the ordering of the first occurrence of each file's original first block. Then it moves copy of the file information to its new position determined by the defragmentation ordering.

**Return Value**

This function is a void function, so nothing is returned.


## Standalone Executable File System Functions ##
These functions interact with the file system without having to be called from the shell.


### k_mkFlatFat
**Synopsis**

Makes a Flat FAT Host OS File for Memory

FILESTATUS k_mkFlatFat(char* flatfs);

**Description**

Creates a file called flatfs which serves as the Memory of the OS. The root directory is initialized to only contain the root file, which starts in block 0 with size 0.

**Return Value**

Returns FILESYSERROR if Unsuccessful.
Returns FILEOK if Succesful.

**Notes**

Used only by mkFlatFat. File must be formatted by begining of OS operations.

### k_initKERNELFdTable
**Synopsis**

Initialize the file descriptor table

void k_initKERNELFdTable()

**Description**
**Return Value**

**Notes**

Used only by catFlatFat. File must be formatted by begining of OS operations.

### k_filesLogout()
**Synopsis**

Free heap memory associated to files

void k_filesLogout()

**Description**

Iterate through all files and free all memory allocated to structure elements of each file.

**Return Value**

This function is a void function, so nothing is returned.

### k_fileDescriptorLogout
**Synopsis**

Free heap memory associated to file descriptors

void k_fileDescriptorLogout()

**Description**

Iterate through all file descriptors on the FD table and free all memory allocated to structure elements of each file descriptor.

**Return Value**

This function is a void function, so nothing is returned.

### b_fileSystemLogout
**Synopsis**

Log out from the file system

void b_fileSystemLogout() (Binary File System)

**Description**

Frees heap memory allocated to the file descriptors on the FD table and the files themselves.

**Return Value**

This function is a void function, so nothing is returned.

**Notes**

Used only by catFlatFat. File must be formatted by begining of OS operations.

### b_open(char* name, int mode)
**Synopsis**

Open a file by name (Binary File System)

int b_open(char* name, int mode)

**Description**

Opens a file *fname* with the mode *mode* and return a file descriptor.
The *mode* can attain the following constants:
* F_WRITE - writes onto the file and truncates if the file exists, or creates it if it does not exist
* F_READ - open the file for reading only, return an error if the file does not exist
* F_APPEND - open the file for reading and writing if it already exist but does not truncate the file; the file pointer also references the end of the file.

**Return Value**

Returns a file descriptor on success and -1 on error.

**Notes**

Used only by catFlatFat. File must be formatted by begining of OS operations.

### b_write(int fd, char* str, int numb)
**Synopsis**

Write bytes to a file (Binary File System)

int b_write(int fd, char* str, int numb)

**Description**

Checks that file *fd* exists and that the F_WRITE/F_APPEND flag is inputted.
Writes *numb* bytes of the string referenced by *str* to the file *fd*; increments the file pointer by *numb*.

**Return Value**

Returns the number of bytes written, or -1 on error.

**Notes**

Used only by catFlatFat. File must be formatted by begining of OS operations.

### b_close(int fd)
**Synopsis**

Close a file

int b_close(int fd) (Binary File System)

**Description**

Free all heap allocated memory for the a file noted by *fd* in our file system before closing it. Return -1 if a file corresponding to the fd does not exist or if an invalid call to close STDIN or STDOUT was made.

**Return Value**

Returns 0 on success, or -1 on failure.

**Notes**

Used only by catFlatFat. File must be formatted by begining of OS operations.

### b_read(int fd, char* str, int n)
**Synopsis**

Read bytes from a file (Binary File System)

int b_read(int fd, char* str, int n)

**Description**

Checks that file *fd* exists and that the F_READ flag is inputted.
Reads *n* bytes from the file referenced by *fd* onto a buffer *str*.

**Return Value**

Returns the number of bytes read, 0 if EOF is reached, or a -1 on error.

**Notes**

Used only by catFlatFat. File must be formatted by begining of OS operations.

### b_lseek(int fd, int offset, int whence)
**Synopsis**

Move the file position (Binary File System)

int b_lseek(int fd, int offset, int whence)

**Description**

Takes in a file descriptor, an integer offset value of *n*, and a constant *whence*. The file pointer of the inputted file is reposition to the offset based on the inputted *whence*.

whence can be F_SEEK_CUR, F_SEEK_SET, or F_SEEK_END:

* F_SEEK_CUR: Moves the position of the file pointer by *offset* from its current position.
* F_SEEK_SET: Sets the position of the file pointer to *offset* (equivalent to moving the pointer by *offset* from the beginning of the file)
* F_SEEK_END: Offsets the position of the file pointer from the end of the file.

**Return Value**

On success, the position of the file descriptor (as an unsigned int) is returned. On failure, -1 is returned.

