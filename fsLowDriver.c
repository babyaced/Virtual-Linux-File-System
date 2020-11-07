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
//#include "fsLow.h"
//#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"

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
		
	//retVal = startPartitionSystem (filename, &volumeSize, &blockSize);
	//closePartitionSystem();	
	/*printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);
	memset (buf, 0, blockSize*2);
	strcpy (buf, "Now is the time for all good people to come to the aid of their countrymen\n");
	strcpy (&buf[blockSize+10], "Four score and seven years ago our fathers brought forth onto this continent a new nation\n");
	LBAwrite (buf, 2, 0);
	LBAwrite (buf, 2, 3);
	LBAread (buf2, 2, 0);
	if (memcmp(buf, buf2, blockSize*2)==0)
		{
		printf("Read/Write worked\n");
		}
	else
		printf("FAILURE on Write/Read\n");
	

	free (buf);
	free(buf2);
	*/
	formatVolume(filename, volumeSize, blockSize);
	int retval = 0;

	// retVal = startPartitionSystem(filename,&volumeSize, &blockSize);
	//Testing LBARead
	/*vcb = malloc(512);
	printf("Mallocing: %d bytes\n", 512);
	
	retVal = LBAread(vcb,1,0);

	printf("VCB Block Count: %d\n", vcb->blockCount);
	printf("VCB Free Block Count:  %d\n", vcb->freeBlockCount);
	printf("VCB Block Count: %d\n", vcb->sizeOfBlocks);
	printf("VCB fslBlkCnt: %d\n", vcb->fslBlkCnt);
	printf("VCB fslBytes: %d\n", vcb->fslBytes);
	printf("VCB Root Directory Block Count: %d\n", vcb->rdBlkCnt);
	printf("VCB Root Directory Location: %d\n", vcb->rdLoc);

	printf("Mallocing: %d bytes\n", vcb->fslBytes);
	freeSpaceBitmap = malloc(vcb->fslBytes);

	retVal = LBAread(freeSpaceBitmap,vcb->fslBlkCnt,1);*/

	/*dir* d2 = malloc(sizeof(dir));
	printf("Mallocing: %d bytes\n", sizeof(dir));

	retVal = LBAread(d2,vcb->rdBlkCnt,vcb->rdLoc);
	printf("Rd loc: %d\n", d2->loc);
	printf("Rd parent loc: %d\n", d2->parentLoc);
	printf("Rd  size in blocks: %d\n", d2->sizeInBlocks);
	printf("Rd  size in bytes: %d\n", d2->sizeInBytes);

	printf("Freeing: %d bytes\n", sizeof(dir));
	free(d2);
	d2 = NULL;*/


	mode_t mode = NULL;
	retVal = fs_mkdir("/test/test2", mode);
	
	//testing b_open()
	retVal = b_open("test",0);


	retVal = closePartitionSystem();

	printf("Freeing: %ld bytes\n", vcb->sizeOfBlocks);
	printf("Freeing: %ld bytes\n", vcb->fslBytes);
	free(vcb);
	
	vcb=NULL;
	
	free(freeSpaceBitmap);
	freeSpaceBitmap = NULL;
	
	return 0;	
	}
	
