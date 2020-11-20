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


	//===============================================
	//Testing collisions and overflow
	//===============================================
	/*for(int i = 0; i < 40; i++){
		printf("%d : %d\n", i, freeSpaceBitmap[i]);
	}
	//char* dirName = malloc(3);
	for(int i = 0; i < 62;i++){
		if(i%32 == 0){
			printf("switching int in bitmap\n");
		}
		sprintf(dirName, "%d", i);
		retVal = fs_mkdir(dirName, 2);  //WORKING
	}

	free(dirName);

	for(int i = 0; i < 40; i++){
		printf("%d : %ud\n", i, freeSpaceBitmap[i]);
	}*/

	//===============================================
	//Testing fs_mkdir()
	//===============================================
	retVal = fs_mkdir("/test", 0);  //WORKING

	retVal = fs_mkdir("../test/test2", 0);  //WORKING

	//===============================================
	//Testing b_open and initFile
	//===============================================
	int FD = b_open("file",0);
	
	
	//retVal = b_open("../test/file",0);

	//===============================================
	//Testing fs_setcwd
	//===============================================
	retVal = fs_setcwd("/test");
	printf("Current Block: %d\n", currentBlock);

	retVal = fs_setcwd("/");
	printf("Current Block: %d\n", currentBlock);

	// path = "test2";
	// retVal = fs_setcwd(path);
	// printf("Current Block: %d\n", currentBlock);

	retVal = fs_setcwd("..");
	printf("Current Block: %d\n", currentBlock);

    retVal = fs_setcwd("test/test2");
	printf("Current Block: %d\n", currentBlock);

	retVal = fs_setcwd("..");
	printf("Current Block: %d\n", currentBlock);


	//===============================================
	//Testing fs_isDir()
	//===============================================
	retVal = fs_isDir("/test/test2");

	if(retVal == 1)
		printf("'test2' was a directory\n");
	
	//===============================================
	//Testing fs_isFile()
	//===============================================
	retVal = fs_isFile("/file");

	if(retVal == 1)
		printf("'file' was a file\n");



	// retVal = b_open("/test/file",0);

	//===============================================
	//Testing fs_getcwd
	//===============================================
	char* buf = malloc(257);
	char* ret;
	ret = fs_getcwd(buf, 256);
	printf("CWD is %s\n",ret);

	free(buf);
	buf = NULL;

	retVal = fs_setcwd("..");
	printf("Current Block: %d\n", currentBlock);

	char* buf2 = malloc(256);
	char* ret2;
	
	ret2 = fs_getcwd(buf2, 256);
	printf("CWD is %s\n",ret2);
	free(buf2);
	buf2 = NULL;

	char* buf3 = malloc(256);
	char* ret3;

	retVal = fs_setcwd("/test/test2");
	printf("Current Block: %d\n", currentBlock);
	
	ret3 = fs_getcwd(buf3, 256);
	printf("CWD is %s\n",ret3);
	free(buf3);
	buf3 = NULL;






	//===============================================
	// test b_write()
	//===============================================
	/*char buff[13] = "The unanimous";
	char buff2[13]= "Declaration o";
	char buff512[512] = "The unanimous Declaration of the thirteen united States of America, When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume among the powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation.We hold these truths to be self-evide";
	char buff2048[2048] = "The unanimous Declaration of the thirteen united States of America, When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume among the powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation.We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.--That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, --That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness. Prudence, indeed, will dictate that Governments long established should not be changed for light and transient causes; and accordingly all experience hath shewn, that mankind are more disposed to suffer, while evils are sufferable, than to right themselves by abolishing the forms to which they are accustomed. But when a long train of abuses and usurpations, pursuing invariably the same Object evinces a design to reduce them under absolute Despotism, it is their right, it is their duty, to throw off such Government, and to provide new Guards for their future security.--Such has been the patient sufferance of these Colonies; and such is now the necessity which constrains them to alter their former Systems of Government. The history of the present King of Great Britain is a history of repeated injuries and usurpations, all having in direct object the establishment of an absolute Tyranny over these States. To";
	retVal = b_write(FD, buff2048,2048);
	// retVal = b_write(FD, buff512, 512);
	// retVal = b_write(FD,buff2,13);

	char* buffer = malloc(2049);
	int readCount = b_read(FD, buffer, 2048);
	printf("Buffer: %s\n", buffer);
	free(buffer);*/

 	//fs_delete("/file");

	b_close(FD);

 	// testing extents
 	dirEnt* testDe = malloc(toBlockSize(sizeof(dirEnt)));   //malloc memory for directory entry we want to initialize
 	initilizeSecExts(testDe, MAX_SEC_EXTENTS);
 	int index;
 	for (index = 0; index < MAX_SEC_EXTENTS; index++){
        getNextExt(testDe);
 	}
    deleteExts(testDe);
	
	free(testDe);
	testDe = NULL;

	

 	retVal = closePartitionSystem();

	freeGlobals();
		
	return 0;	
	}
	
