#include "binaryFileSystem.h"
#include "kernelFileSystem.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if (argc < 2){
		printf("needs a Fat File System to list");
		return 1;
	}
	k_setFileSystem(argv[1]);
	k_loadDirectory();
	char* listDirectory = k_ls();
	printf("%s", listDirectory);
	free(listDirectory);
	k_storeDirectory();
	k_filesLogout();
	return 0;
}