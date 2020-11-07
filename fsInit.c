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

unsigned int* freeSpaceBitmap;  //global for whole damn program
vCB* vcb;  //global for whole damn program
int currentBlock;  //holds current LBA block for use with relative pathnames //initialized to root directory in this file
int currentBlockSize;


void formatVolume(char* volumeName, uint64_t volumeSize, uint64_t blockSize){
    int retVal;

    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);

    // if(retVal == 2){ //if volume doesn't exist //we need to format our volume
        initVCB(volumeSize, blockSize);
        initFSL(volumeSize, blockSize);
        retVal = LBAwrite(freeSpaceBitmap,vcb->fslBlkCnt,1);
        setFreeBlocks(0,1);
        setFreeBlocks(1,vcb->fslBlkCnt);
        initDir(0,"root");
        retVal = LBAwrite(vcb,1,0);
    // }
    /*else if(retVal == 0){ // else read vcb and fsl into our globals, and set currentBlock + currentBlockSize
        vcb = malloc(blockSize);
        retVal = LBAread(vcb,1,0);
        fsl = malloc(sizeof(fSL));
        fsl->freeSpaceBitmap = malloc(vcb->fslBytes);
        retVal = LBAread(fsl,vcb->fslBlkCnt,1);
        currentBlock = vcb->rdLoc;
        currentBlockSize = vcb->rdBlkCnt;
    }*/


}

void initVCB(int volSize, int blockSize){
    printf("Mallocing: %d bytes\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
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
    printf("Callocing: %d bytes\n", bmElements*sizeof(freeSpaceBitmap[0]));
    freeSpaceBitmap = calloc(bmElements,sizeof(freeSpaceBitmap[0]));
    printf("Size of Free Space Bitmap: %d\n", sizeof(freeSpaceBitmap));
    
    vcb->fslBytes = bmSize;
    vcb->fslBlkCnt= (bmSize/blockSize) + 1;
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



