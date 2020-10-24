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
    fSL* fsl2;


    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    //init VCB
    vcb1 = initVCB(volumeSize, blockSize);
    printf("VCB Block Count: %lu\n",vcb1->blockCount);
    printf("VCB Size of Blocks: %lu\n",vcb1->sizeOfBlocks);
    printf("VCB Free Block Count: %lu\n",vcb1->freeBlockCount);
    //malloc blockSize bytes for VCB
    retVal = LBAwrite(&vcb1,1,0);  // write VCB to disk
    printf("Retval: %d\n", retVal);
    fsl = initfSL(volumeSize, blockSize);
    printf("Size of FSL: %ld\n", sizeof(fsl->freeSpaceBitmap));
    int blocksForFSL = (fsl->freeSpaceListSize/blockSize) + 1;
    printf("Blocks for FSL: %d\n", blocksForFSL);
    retVal = LBAwrite(&fsl,blocksForFSL,1);
    setBit(fsl->freeSpaceBitmap, 0);
    printBM(fsl->freeSpaceBitmap, fsl->freeSpaceListSize);
    int inUse = checkBit(fsl->freeSpaceBitmap, 0);
    printf("In Use?: %d\n", inUse);
    
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

fSL* initfSL(uint64_t volSize, uint64_t blockSize){
    fSL* fsl;
    fsl = malloc(sizeof(fSL));
    int blockCount = volSize/ blockSize;
    int bmSize = blockCount/8;
    printf("FSL Size: %d\n", bmSize);
    fsl->freeSpaceBitmap = malloc(bmSize);
    fsl->freeSpaceListSize = bmSize;
    initBM(fsl->freeSpaceBitmap, bmSize);
    return fsl;
}