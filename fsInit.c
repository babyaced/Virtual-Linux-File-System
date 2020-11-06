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

fSL* fsl;  //global for whole damn program
vCB* vcb;  //global for whole damn program


void formatVolume(char* volumeName){
	int volumeSize;
	int blockSize;
    int retVal;

    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    initFSL(volumeSize, blockSize);
    retVal = LBAwrite(fsl,fsl->fslBlocksUsed,1);
    initVCB(volumeSize, blockSize);
    setFreeBlocks(0,1);
    setFreeBlocks(1,fsl->fslBlocksUsed);
    initDir(0,"root");
    retVal = LBAwrite(vcb,1,0);
}

void initVCB(int volSize, int blockSize){
    printf("Mallocing: %d bytes\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
        vcb->fslBlkCnt = fsl->fslBlocksUsed;
        vcb->fslBytes = fsl->freeSpaceBytes;
        //printf("Size of VCB: %ld\n",sizeof(vcb));
    }else{
        printf("Malloc Failed\n");
    }
}

void initFSL(int volSize, int blockSize){
    
    int blockCount = volSize/ blockSize;
    int bmSize = blockCount/8 +1;
    int bmElements = (blockCount/32) +1;  //number of blocks divided by bits in int
    //printf("FSL Size: %d\n", bmSize);  //for mallocing bytes when retrieving data
    fsl = malloc(sizeof(fSL));
    printf("Mallocing: %d bytes\n", sizeof(fSL));
    //int bmElements = bmSize
    fsl->freeSpaceBitmap = calloc(bmElements,sizeof(fsl->freeSpaceBitmap[0]));
    printf("Size of Free Space Bitmap: %d\n", sizeof(fsl->freeSpaceBitmap));
    printf("Mallocing: %d bytes\n", bmSize);
    fsl->freeSpaceBytes = bmSize;
    fsl->freeSpaceBits = blockCount;
    fsl->fslBlocksUsed = (fsl->freeSpaceBytes/blockSize) + 1;
    //initBM(fsl->freeSpaceBitmap, blockCount);  //Need to correctly indicate blocks taken by VCB and freespaceList
    //printf("Size of FSL: %ld\n",sizeof(fsl));
}

/*void initRD(){
    int retVal;
    //create array/structure of directory entries
    //pass pointer to vcb
    //read free space list from disk
    
    //LBAWrite(rd,1,rdStartBlock)

}*/



