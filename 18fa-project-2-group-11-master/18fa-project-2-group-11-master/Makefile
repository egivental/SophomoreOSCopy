CFLAGS=-g
CC=clang
SRCS= src/pennos.c src/kernelFileSystem.c src/userFileSystem.c src/shell.c src/kernel.c src/log.c src/queue.c src/tokenizer.c src/scheduler.c src/signalQueue.c src/job_list.c
OBJS= src/pennos.o src/kernelFileSystem.o src/userFileSystem.o src/shell.o src/kernel.o src/log.o src/queue.o src/tokenizer.o src/scheduler.o src/signalQueue.o src/job_list.o
LDFLAGS=
LIBS=

all: clean pennOS mkFlatFat lsFlatFat catFlatFat

pennOS: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o bin/pennOS $(OBJS)

mkFlatFat: src/mkFlatFat.o src/binaryFileSystem.o src/kernelFileSystem.o 
	clang -g -o bin/mkFlatFat src/mkFlatFat.o src/kernelFileSystem.o src/binaryFileSystem.o  

lsFlatFat: src/lsFlatFat.o src/binaryFileSystem.o src/kernelFileSystem.o 
	clang -g -o bin/lsFlatFat src/lsFlatFat.o src/kernelFileSystem.o src/binaryFileSystem.o    

catFlatFat: src/catFlatFat.o src/binaryFileSystem.o src/kernelFileSystem.o 
	clang -g -o bin/catFlatFat src/catFlatFat.o src/kernelFileSystem.o src/binaryFileSystem.o 

clean:
	rm -rf src/*.o bin/pennOS bin/lsFlatFat bin/mkFlatFat bin/catFlatFat log
