#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsInit.h"
#include "bitMap.h"
#include "dirMgr.h"
#include "freeMgr.h"
#include "fsLow.h"

unsigned int* freeSpaceBitmap;  //global for whole damn program
vCB* vcb;  //global for whole damn program
int currentBlock;  //holds current LBA block for use with relative pathnames //initialized to root directory in this file
int currentBlockSize;
char currentBlockName[255];

void initGlobals(uint64_t volumeSize, uint64_t blockSize){
    int retVal;
    // printf("Mallocing: %ld bytes\n", blockSize);
    vcb = malloc(blockSize);
    retVal = LBAread(vcb,1,0); //read already existing vcb from disk
    int blockCount = volumeSize/ blockSize;
    int bmSize = toBlockSize(blockCount/8); //divide number of blocks by 
    int bmElements = (bmSize/4);  //number of blocks divided by bits in int
    // printf("Callocing: %ld bytes\n", bmElements*sizeof(int));
    freeSpaceBitmap = calloc(bmElements,sizeof(int));
    retVal = LBAread(freeSpaceBitmap, vcb->fslBlkCnt,1);  //read already existing freeSpaceBitmap from disk
    currentBlock = vcb->rdLoc;   //initialize current directory to root directory
    currentBlockSize = vcb->dBlkCnt; //initialize current directory block size to root directory block size
}


void formatVolume(char* volumeName, uint64_t volumeSize, uint64_t blockSize){
    int retVal;

    // retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);
    initVCB(volumeSize, blockSize);
    initFSL(volumeSize, blockSize);
    setFreeBlocks(0,1);
    setFreeBlocks(1,vcb->fslBlkCnt);
    retVal = LBAwrite(freeSpaceBitmap,vcb->fslBlkCnt,1);
    retVal = initDir(0,"");
    vcb->magicNum = 0x6f8e66c7d3c61738;
    retVal = LBAwrite(vcb,1,0);
}

void initVCB(int volSize, int blockSize){
    // printf("Mallocing: %d bytes\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
    }else{
        // printf("Malloc Failed\n");
    }
}

void initFSL(int volSize, int blockSize){
    
    int blockCount = volSize/ blockSize;
    int bmSize = toBlockSize(blockCount/8); //divide number of blocks by 
    int bmElements = (bmSize/4);  //number of blocks divided by bits in int
    // printf("Callocing: %ld bytes\n", bmElements*sizeof(int));
    freeSpaceBitmap = calloc(bmElements,sizeof(int));
    
    vcb->fslBytes = bmSize;
    vcb->fslBlkCnt= (bmSize/blockSize);
}

void freeGlobals()
{
    // printf("Freeing: %d bytes\n", vcb->sizeOfBlocks);
	// printf("Freeing: %d bytes\n", vcb->fslBytes);
	free(vcb);
	
	vcb=NULL;
	
	free(freeSpaceBitmap);
	freeSpaceBitmap = NULL;
}




