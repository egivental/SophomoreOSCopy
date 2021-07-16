#include "kernelFileSystem.h"
#include "binaryFileSystem.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if (argc < 2){
		printf("requires 2 inputs, only 1 given");
		return -1;
	} else if (argc > 2){
		printf("excess parameters ignored");
	}
	k_mkFlatFat(argv[1]);
	k_setFileSystem(argv[1]);
	k_initDirectory();
	k_storeDirectory();
	k_filesLogout();
	return 0;
}