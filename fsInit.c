#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "fsInit.h"
#include "fsLow.h"
#include "bitMap.h"


//write vcb to logical block 0
//init root directory
//initialize free space


void formatVolume(char* volumeName){
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    vcb* vcb1;
    fSL* fsl;
    rootDir* rd;


    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    
    fsl = initFSL(volumeSize, blockSize);
    vcb1 = initVCB(volumeSize, blockSize);
    rd = initRD(volumeSize,blockSize, &fsl);

    retVal = LBAwrite(&fsl,fsl->blocksUsed,1);
    retVal = LBAwrite(&vcb1,1,0);  // write VCB to disk
    retVal = closePartitionSystem();
}

vcb* initVCB(uint64_t volSize, uint64_t blockSize){
    vcb* vcb;
    printf("Block Size = %lu\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->blockCount = volSize;
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
    }else{
        printf("Malloc Failed\n");
    }

    return vcb;
}

fSL* initFSL(uint64_t volSize, uint64_t blockSize){
    fSL* fsl;
    fsl = malloc(sizeof(fSL));
    int blockCount = volSize/ blockSize;
    int bmSize = blockCount/8;  //number of blocks divided by bits per byte
    printf("FSL Size: %d\n", bmSize);
    fsl->freeSpaceBitmap = malloc(bmSize);
    fsl->freeSpaceListSize = bmSize;
    fsl->blocksUsed = (fsl->freeSpaceListSize/blockSize) + 1;
    initBM(fsl->freeSpaceBitmap, bmSize);  //Need to correctly indicate blocks taken by VCB and freespaceList
    return fsl;
}

rootDir* initRD(uint64_t blockSize, uint64_t blocks, fSL* fsl){
    //create array/structure of directory entries
    //pass pointer to vcb
    //read free space list from disk
    int rdStartBlock = findFreeBlocks(&fsl,10);  //find next available blocks
    printf("rdStartBlock: %d\n", rdStartBlock);  //should be 1 + fsl->blocksUsed
}

int findFreeBlocks(fSL* fsl, uint64_t blocksNeeded){
    for(int i = 0; i < fsl->freeSpaceListSize;i++){
        if(!checkBit(fsl->freeSpaceBitmap,i))
            return i; //returns index for now
    }
}