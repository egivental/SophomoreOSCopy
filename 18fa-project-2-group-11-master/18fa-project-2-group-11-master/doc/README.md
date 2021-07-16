# Project 2

## Information
Group members: Emile Givental, Dan Truong, Seyoung Kim, Greg Kofman

Usernames: givental, truongd, seyoungk, gkofman

## Source Files
catFlatFat.c, fileSystem.c, fileSystem.h, mkFlatFat.c, kernel.c, kernel.h, pcb.h, queue.c, queue.h, scheduler.c, scheduler.h, signals.h, log.c, log.h, shell.c, shell.h, pennos.c, tokenizer.c, tokenizer.h, create.c, create.h.

## Extra Credit
Init process.

## Compilation Instructions
Run `make` in the terminal to compile all files. Run `./bin/mkFlatFat flatfs` to create the file system. Run `./bin/pennOS flatfs log` to run Penn OS.

## Work Accomplished

We have implemented rudimentary file system that can robustly handle file creation and removal.
We have implemented various kernel functions and user functions that indirectly call kernel function to interact with the kernel. In terms of the shell, we have implemented a shell that can take in various Linux commands, as well as a scheduler that handles the processing and selection of processes to make active/inactive. Finally, we have implemented a means to log events onto a file and keep track of what functions we have called.

More on the functions that we have implemented can be discussed in the Companion Document.

## Data Structures and justifications
* PCB Structure

  The PCB structure can be found in pcb.h with type pcb_t. It consists of the follwoing components:
  * ucontext_t uc - context for the process
  * pid_t pid
  * struct pcb_t \*ppcb - The parent
  * struct pcb_t \*cpcb - The child
  * struct pcb_t \*spcb - A sibling
  * int nice - The nice priority level
  * unsigned int stackid
  * char \*cmd - The process name
  * int runs_left - runs left in scheduler
  * filedescriptor \*fdtable - FD table
  * status_e old_status
  * status_e curr_status

  We include these elements in our PCB structure, as these were variables that we found appropriate and needed by our scheduler. In addition, many of these elements parallel what is typically found in the context of an actual process on an OS.

## Code Layout

In /src folder:

* Files for the file system: catFlatFat.c, fileSystem.c, fileSystem.h, mkFlatFat.c
* Files for the kernel: kernel.c, kernel.h
* Files for the scheduler: kernel.c, kernel.h, pcb.h, queue.c, queue.h, scheduler.c, scheduler.h, signals.h, signalQueue.c, signalQueue.h
* Files for the shell: log.c, log.h, shell.c, shell.h, pennos.c, tokenizer.c, tokenizer.h

In /bin folder:

* Compiled binaries pennOS, catFlatFat, lsFlatFat, mkFlatFat

