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
//#include "fsLow.h"
//#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"

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
	formatVolume(filename);

	//Testing LBARead
	int retval = 0;
	vCB* vcb = malloc(512);
	retVal = LBAread(vcb,1,0);

	printf("VCB Block Count: %lu\n", vcb->blockCount);
	printf("VCB Free Block Count:  %lu\n", vcb->freeBlockCount);
	printf("VCB Block Count: %lu\n", vcb->sizeOfBlocks);
	printf("VCB fslBlkCnt: %lu\n", vcb->fslBlkCnt);
	printf("VCB fslBytes: %lu\n", vcb->fslBytes);

	fSL* fsl = malloc(sizeof(fSL));
	//fsl->freeSpaceBitmap = malloc(vcb->fslBytes);
	retVal = LBAread(fsl,vcb->fslBlkCnt,1);
	

	printf("FSL Block Count: %lu\n", fsl->freeSpaceBits);
	printf("FSL Blocks Used:  %lu\n", fsl->fslBlocksUsed);
	printf("FSL Location: %lu\n", fsl->location);

	dir* rd = malloc(sizeof(dir));

	retVal = LBAread(rd,vcb->rdBlkCnt,vcb->rdLoc);
	printf("Rd loc: %lu\n", rd->loc);
	printf("Rd parent loc: %lu\n", rd->parentLoc);
	printf("Rd  size in blocks: %lu\n", rd->sizeInBlocks);
	printf("Rd  size in bytes: %lu\n", rd->sizeInBytes);

	free(vcb);
	free(fsl);
	//free(rd);

	retVal = closePartitionSystem();
	
	return 0;	
	}
	
