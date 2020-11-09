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
		
	retVal = startPartitionSystem(filename,&volumeSize, &blockSize);
	formatVolume(filename, volumeSize, blockSize);
	// initGlobals(volumeSize,blockSize);
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

	retVal = LBAread(freeSpaceBitmap,vcb->fslBlkCnt,1);

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

	//testing fs_mkdir()
	mode_t mode = NULL;  //ignore for now
	retVal = fs_mkdir("/root/test", mode);
	
	//testing b_open()
	retVal = b_open("/root/test",0);
	retVal = b_open("test2",0);

	//testing fs_isDir()
	retVal = fs_isDir("test");

	if(retVal == 1)
		printf("'test' was a directory\n");
	
	//testing fs_isDir()
	retVal = fs_isFile("test");

	if(retVal == 0)
		printf("'test' was not a file\n");

	char* path = "test";
	retVal = fs_setcwd(path);
	printf("Current Block: %d\n", currentBlock);

	retVal = fs_mkdir("test2", mode);

	//retVal = b_open("/root/test",0);

	//testing getcwd()
	char* buf = malloc(257);
	char* ret;
	ret = fs_getcwd(buf, 256);
	printf("CWD is %s\n",ret);
	free(buf);

	// test b_write()
	// char buff[2048] = "The unanimous Declaration of the thirteen united States of America, When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume among the powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation.We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.--That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, --That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness. Prudence, indeed, will dictate that Governments long established should not be changed for light and transient causes; and accordingly all experience hath shewn, that mankind are more disposed to suffer, while evils are sufferable, than to right themselves by abolishing the forms to which they are accustomed. But when a long train of abuses and usurpations, pursuing invariably the same Object evinces a design to reduce them under absolute Despotism, it is their right, it is their duty, to throw off such Government, and to provide new Guards for their future security.--Such has been the patient sufferance of these Colonies; and such is now the necessity which constrains them to alter their former Systems of Government. The history of the present King of Great Britain is a history of repeated injuries and usurpations, all having in direct object the establishment of an absolute Tyranny over these States. To";
	// retVal = b_write(0, buff, 2048);


	retVal = closePartitionSystem();

	printf("Freeing: %ld bytes\n", vcb->sizeOfBlocks);
	printf("Freeing: %ld bytes\n", vcb->fslBytes);
	free(vcb);
	
	vcb=NULL;
	
	free(freeSpaceBitmap);
	freeSpaceBitmap = NULL;
	
	return 0;	
	}
	
