#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "fsInit.h"
#include "fsLow.h"


//write vcb to logical block 0
//init root directory
//initialize free space


void formatVolume(char* volumeName){
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    int retVal2;
    vcb* vcb1;
    vcb* vcb2;

    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    //init VCB
    vcb1 = initVCB(&volumeSize, &blockSize);
    printf("VCB Block Count: %d\n",vcb1->blockCount);
    printf("VCB Size of Blocks: %d\n",vcb1->sizeOfBlocks);
    printf("VCB Free Block Count: %d\n",vcb1->freeBlockCount);
    //malloc blockSize bytes for VCB
    retVal = LBAwrite(&vcb1,1,0);
    printf("Retval: %d\n", retVal);
    LBAread(vcb2,1,0);
    vcb2 = (vcb*)malloc(blockSize);
    retVal2 = LBAread(&vcb2,1,0);
    printf("Retval2: %d\n", retVal);
    printf("VCB Block Count: %d\n", vcb2->blockCount);
    printf("VCB Size of Blocks: %d\n",vcb2->sizeOfBlocks);
    printf("VCB Free Block Count: %d\n",vcb2->freeBlockCount);
    if(sizeof(vcb)>0)
        printf("Size of VCB: %d\n",sizeof(vcb));
    //write VCB pointer to block 1
}

vcb* initVCB(uint64_t * volSize, uint64_t * blockSize){
    vcb* vcb;
    vcb = malloc(*blockSize);
    if(vcb){
        vcb->blockCount = *volSize;
        vcb->sizeOfBlocks = *blockSize;
        vcb->freeBlockCount = *volSize;
    }else{
        printf("Malloc Failed\n");
    }

    return vcb;
}