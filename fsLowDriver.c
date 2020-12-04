/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: fsLowDriver.c
*
* Description: This is a demo to show how to use the fsLow
* 	routines.
*
**************************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include "b_io.h"
#include "fsLow.h"
//#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"
#include "fsshell.c"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock; //holds current LBA block for use with relative pathnames
extern int currentBlockSize;

int main (int argc, char *argv[])
{	
	char * filename;
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    
	if (argc > 3)
		{
		filename = argv[1];
		volumeSize = atoll (argv[2]);
		blockSize = atoll (argv[3]);
		}
	else
		{
		printf ("Usage: fsLowDriver volumeFileName volumeSize blockSize\n");
		return -1;
		}
		
	retVal = startPartitionSystem(filename,&volumeSize, &blockSize);
	vCB* volumeControlBlock = malloc(blockSize);
	retVal = LBAread(volumeControlBlock,1,0);
	if(volumeControlBlock->magicNum == 0x6f8e66c7d3c61738){
		initGlobals(volumeSize, blockSize);
	}
	else{
		formatVolume(filename, volumeSize, blockSize);
	}
	free(volumeControlBlock);
	volumeControlBlock = NULL;
	
	int retval = 0;

	dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));

	retVal = LBAread(de,vcb->deBlkCnt, vcb->rdLoc);

	printf("Root directory entry: \n");
	printf("DE->LOC: %d\n",de->loc);
	printf("DE->TYPE: %d\n",de->type);

	printf("TESTING MEMORY BUG WITH LS -L\n");
	fs_mkdir("test",0);
	fs_mkdir("test2",0);
	printf("MADE TEST DIRECTORIES\n");
	retVal = LBAread(de,vcb->deBlkCnt, vcb->rdLoc);
	printf("DE->LOC: %d\n",de->loc);
	printf("DE->TYPE: %d\n",de->type);
	fdDir* dirp;

	dirp = fs_opendir("");
	printf("DE->LOC: %d\n",de->loc);
	printf("DE->TYPE: %d\n",de->type);
	printf("OPENED ROOT\n");
	retVal = displayFiles(dirp, 0,1);
	printf("LS -L");
	

	free(de);
	de = NULL;
 	retVal = closePartitionSystem();
	freeGlobals();


		
	return 0;	
	}
	
