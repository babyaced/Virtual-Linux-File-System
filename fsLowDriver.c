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

extern fSL* fsl;  //global for whole damn program
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

	//Testing LBARead
	
	/*vCB* vcb2 = malloc(512);
	printf("Mallocing: %d bytes\n", 512);
	
	retVal = LBAread(vcb2,1,0);

	printf("VCB Block Count: %d\n", vcb2->blockCount);
	printf("VCB Free Block Count:  %d\n", vcb2->freeBlockCount);
	printf("VCB Block Count: %d\n", vcb2->sizeOfBlocks);
	printf("VCB fslBlkCnt: %d\n", vcb2->fslBlkCnt);
	printf("VCB fslBytes: %d\n", vcb2->fslBytes);
	printf("VCB Root Directory Block Count: %d\n", vcb2->rdBlkCnt);
	printf("VCB Root Directory Location: %d\n", vcb2->rdLoc);

	printf("Mallocing: %d bytes\n", sizeof(fSL));
	fSL* fsl2 = malloc(sizeof(fSL));
	printf("Mallocing: %d bytes\n", vcb2->fslBytes);
	fsl2->freeSpaceBitmap = malloc(vcb2->fslBytes);

	retVal = LBAread(fsl2,vcb2->fslBlkCnt,1);
	


	printf("FSL Block Count: %d\n", fsl2->freeSpaceBits);
	printf("FSL Blocks Used:  %d\n", fsl2->fslBlocksUsed);
	printf("FSL Location: %d\n", fsl2->location);

	
	printf("Freeing: %d bytes\n", vcb2->fslBytes);
	free(fsl2->freeSpaceBitmap);
	printf("Freeing: %d bytes\n", sizeof(fSL));
	free(fsl2);
	fsl2= NULL;

	dir* d2 = malloc(sizeof(dir));
	printf("Mallocing: %d bytes\n", sizeof(dir));

	retVal = LBAread(d2,vcb2->rdBlkCnt,vcb2->rdLoc);
	printf("Rd loc: %d\n", d2->loc);
	printf("Rd parent loc: %d\n", d2->parentLoc);
	printf("Rd  size in blocks: %d\n", d2->sizeInBlocks);
	printf("Rd  size in bytes: %d\n", d2->sizeInBytes);

	printf("Freeing: %d bytes\n", sizeof(dir));
	free(d2);
	d2 = NULL;


	fsl = NULL;
	free(vcb2);
	printf("Freeing: %d bytes\n", 512);
	vcb2= NULL;*/


	//free(rd->dirEnts);

	mode_t mode = NULL;
	retVal = fs_mkdir("test", mode);
	
	//testing b_open()
	retVal = b_open("test",0);

	// test b_write()
	//char * buff = "CSC415 Group Term Assignment - File System: This is a GROUP assignment written in C. Only one person on the team needs to submit the project.Over the past moth you and your team have been designing components of a file system. You have defined the goals and designed the directory entry structure, the volume structure and the free space. You have written buffered read and write routines. Now it is time to implement your file system.To help I have written the low level LBA based read and write. The routines are in fsLow.c, the necessary header for you to include file is fsLow.h. You do NOT need to understand the code in fsLow, but you do need to understand the header file and the functions. There are 4 key functions:int startPartitionSystem (char * filename, uint64_t * volSize, uint64_t * blockSize);startPartitionSystem – you specify a file name that is the “volume” of your hard drive. The volume size is rounded to even block size and is only used when creating the volume file. The block size must be a power of 2 (nominally 512 bytes).On Return, the function will return 0 if success or a negative number if an error occurs. The values pointed to by volSize and blockSize are filled in with the values of the existing volume file.int closePartitionSystem ();closePartitionSystem – closes down the volume file and must be called prior to terminating the process, but no read or writes can happen after this call.uint64_t LBAwrite (void * buffer, uint64_t lbaCount, uint64_t lbaPosition);uint64_t LBAread (void * buffer, uint64_t lbaCount, uint64_t lbaPosition);LBAread and LBAwrite take a buffer, a count of LBA blocks and the starting LBA block number (0 based). The buffer must be large enough for the number of blocks * the block size.On return, these function returns the number of blocks read or written.The source file fsLowDriver.c is a simple driver for the driver system and should not be part of your program.In addition, I have written a hexdump utility that will allow you to analyze your volume fi";
	//retVal = b_write(0, buff, 1);

	//free(rd);


	retVal = closePartitionSystem();

	free(vcb);
	vcb=NULL;
	free(fsl->freeSpaceBitmap);
	free(fsl);
	
	return 0;	
	}
	
