#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsInit.h"
#include "bitMap.h"
#include "dirMgr.h"
#include "freeMgr.h"

unsigned int* freeSpaceBitmap;  //global for whole damn program
vCB* vcb;  //global for whole damn program
int currentBlock;  //holds current LBA block for use with relative pathnames //initialized to root directory in this file
int currentBlockSize;

void initGlobals(uint64_t volumeSize, uint64_t blockSize){
    int retVal;
    printf("Mallocing: %d bytes\n", blockSize);
    vcb = malloc(toBlockSize(sizeof(vCB)));
    retVal = LBAread(vcb,1,0); //read already existing vcb from disk
    int blockCount = volumeSize/ blockSize;
    int bmSize = blockCount/8 +1;
    int bmElements = (blockCount/32) +1;  //number of blocks divided by bits in int
    printf("Callocing: %d bytes\n", bmElements*sizeof(freeSpaceBitmap[0]));
    freeSpaceBitmap = calloc(bmElements,sizeof(freeSpaceBitmap[0]));
    printf("Mallocing: %d bytes\n", vcb->fslBytes);
    retVal = LBAread(freeSpaceBitmap, vcb->fslBlkCnt,1);  //read already existing freeSpaceBitmap from disk
    currentBlock = vcb->rdLoc;   //initialize current directory to root directory
    currentBlockSize = vcb->rdBlkCnt; //initialize current directory block size to root directory block size
}


void formatVolume(char* volumeName, uint64_t volumeSize, uint64_t blockSize){
    int retVal;

    // retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);
    initVCB(volumeSize, blockSize);
    initFSL(volumeSize, blockSize);
    setFreeBlocks(0,1);
    setFreeBlocks(1,vcb->fslBlkCnt);
    retVal = LBAwrite(freeSpaceBitmap,vcb->fslBlkCnt,1);
    retVal = initDir(0,"root");
    retVal = LBAwrite(vcb,1,0);
}

void initVCB(int volSize, int blockSize){
    printf("Mallocing: %d bytes\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
    }else{
        printf("Malloc Failed\n");
    }
}

void initFSL(int volSize, int blockSize){
    
    int blockCount = volSize/ blockSize;
    int bmSize = toBlockSize(blockCount/8); //divide number of blocks by 
    int bmElements = (bmSize/4);  //number of blocks divided by bits in int
    printf("Callocing: %d bytes\n", bmElements*sizeof(int));
    freeSpaceBitmap = calloc(bmElements,sizeof(int));
    
    vcb->fslBytes = bmSize;
    vcb->fslBlkCnt= (bmSize/blockSize);
}




