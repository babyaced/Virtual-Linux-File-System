#include <sys/types.h>
#include <stdlib.h>
#include "fsInit.h"
#include "fsLow.h"


//write vcb to logical block 0
//init root directory
//initialize free space


void formatVolume(char* volumeName){
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    vcb* vcb;

    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    //init VCB
    vcb = initVCB(&volumeSize, &blockSize);
    printf("VCB Block Count: %d\n",vcb->blockCount);
    printf("VCB Size of Blocks: %d\n",vcb->sizeOfBlocks);
    printf("VCB Free Block Count: %d\n",vcb->freeBlockCount);
    //malloc blockSize bytes for VCB
    //LBAwrite(vcb,2,0);
    //write VCB pointer to block 1
}

vcb* initVCB(uint64_t * volSize, uint64_t * blockSize){
    vcb *vcb = malloc(*blockSize);
    vcb->blockCount = *volSize;
    vcb->sizeOfBlocks = *blockSize;
    vcb->freeBlockCount = *volSize;
    return vcb;
}