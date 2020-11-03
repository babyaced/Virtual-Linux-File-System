#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsInit.h"
#include "bitMap.h"
#include "dirMgr.h"
#include "freeMgr.h"
//write vcb to logical block 0
//init root directory
//initialize free space

#define DIR_ENTS_INIT_SIZE 10

static fSL* fsl;  //only global for this file
static vCB* vcb;  //only global for this file
//static dir* rd;


void formatVolume(char* volumeName){
	int volumeSize;
	int blockSize;
    int retVal;

    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    initFSL(volumeSize, blockSize);
    retVal = LBAwrite(fsl,fsl->fslBlocksUsed,1);
    initVCB(volumeSize, blockSize);
    setFreeBlocks(vcb,fsl,0,1);
    setFreeBlocks(vcb, fsl, 1,fsl->fslBlocksUsed);
    initDir(vcb,fsl,0);
    retVal = LBAwrite(vcb,1,0);
    free(vcb);
    vcb = NULL;
    free(fsl->freeSpaceBitmap);
    fsl->freeSpaceBitmap = NULL;

    free(fsl);
    fsl = NULL;
    
    //dir* rd2; = malloc(sizeof(rd->sizeInBytes));
    //retVal = LBAread(rd,rd->sizeInBlocks,fsl->fslBlocksUsed +1);

    //printf("%lu\n",rd2->parentLocation)
}

void initVCB(int volSize, int blockSize){
    printf("Block Size = %d\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
        vcb->fslBlkCnt = fsl->fslBlocksUsed;
        vcb->fslBytes = fsl->freeSpaceBits;
        printf("Size of VCB: %ld\n",sizeof(vcb));
    }else{
        printf("Malloc Failed\n");
    }
}

void initFSL(int volSize, int blockSize){
    
    int blockCount = volSize/ blockSize;
    int bmSize = blockCount/8;  //number of blocks divided by bits per byte
    printf("FSL Size: %d\n", bmSize);  //for mallocing bytes when retrieving data
    fsl = malloc(sizeof(fSL));
    fsl->freeSpaceBitmap = malloc(bmSize);
    fsl->freeSpaceBits = bmSize;
    fsl->fslBlocksUsed = (fsl->freeSpaceBits/blockSize) + 1;
    initBM(fsl->freeSpaceBitmap, blockCount);  //Need to correctly indicate blocks taken by VCB and freespaceList
    printf("Size of FSL: %ld\n",sizeof(fsl));
    
}

/*void initRD(){
    int retVal;
    //create array/structure of directory entries
    //pass pointer to vcb
    //read free space list from disk
    
    //LBAWrite(rd,1,rdStartBlock)

}*/



