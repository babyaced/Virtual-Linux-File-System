#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "fsInit.h"
#include "fsLow.h"
#include "bitMap.h"

#define DIR_ENTS_INIT_SIZE 10
//write vcb to logical block 0
//init root directory
//initialize free space

fSL* fsl;
vCB* vcb;
dir* rd;


void formatVolume(char* volumeName){
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;

    retVal = startPartitionSystem(volumeName,&volumeSize, &blockSize);


    initFSL(volumeSize, blockSize);
    initVCB(volumeSize, blockSize);

    rd = initDir(0);

    vcb->rootDirLocation = rd->parentLocation;

    retVal = LBAwrite(vcb,1,0);  // write VCB to disk
    retVal = LBAwrite(fsl,fsl->fslBlocksUsed,1);
    retVal = LBAwrite(rd,rd->sizeInBlocks, fsl->fslBlocksUsed +1);
    dir* rd2 = malloc(sizeof(rd->sizeInBytes));
    retVal = LBAread(rd2,rd->sizeInBlocks,fsl->fslBlocksUsed +1);

    printf("%lu\n",rd2->parentLocation);
    retVal = closePartitionSystem();
}

void initVCB(uint64_t volSize, uint64_t blockSize){
    printf("Block Size = %lu\n", blockSize);
    vcb = malloc(blockSize);
    if(vcb){
        vcb->blockCount = volSize;
        vcb->sizeOfBlocks = blockSize;
        vcb->freeBlockCount = volSize/ blockSize;
        vcb->blockCount = volSize/ blockSize;
        setFreeBlocks(0,1);
    }else{
        printf("Malloc Failed\n");
    }
}

void initFSL(uint64_t volSize, uint64_t blockSize){
    fsl = malloc(sizeof(fSL));
    int blockCount = volSize/ blockSize;
    int bmSize = blockCount/8;  //number of blocks divided by bits per byte
    printf("FSL Size: %d\n", bmSize);
    fsl->freeSpaceBitmap = malloc(bmSize);
    fsl->freeSpaceBits = bmSize;
    fsl->fslBlocksUsed = (fsl->freeSpaceBits/blockSize) + 1;
    setFreeBlocks(1,fsl->fslBlocksUsed);
    initBM(fsl->freeSpaceBitmap, bmSize);  //Need to correctly indicate blocks taken by VCB and freespaceList
}

/*void initRD(){
    int retVal;
    //create array/structure of directory entries
    //pass pointer to vcb
    //read free space list from disk
    
    //LBAWrite(rd,1,rdStartBlock)

}*/

int findFreeBlocks(uint64_t blocksNeeded){
    int freeBlockCounter = 0;
    for(int i = 0; i < fsl->freeSpaceBits;i++){
        if(freeBlockCounter == blocksNeeded)
            return i - blocksNeeded;
        if(!checkBit(fsl->freeSpaceBitmap,i))
            freeBlockCounter++;
    }
}

void setFreeBlocks(int startingIndex,int count){
    for(int i = startingIndex; i < count + startingIndex; i++){
        setBit(fsl->freeSpaceBitmap,i);
    }
}

dir* initDir(uint64_t block){
    int bytesNeeded = sizeof(dir) + DIR_ENTS_INIT_SIZE *sizeof(dirEnt);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(blocksNeeded);  //find next available blocks
    
    dir* dir = malloc(bytesNeeded);
    dir->sizeInBlocks = blocksNeeded;
    dir->sizeInBytes = bytesNeeded;
    if(block == 0){
        dir->location = dirStartBlock;
        dir->parentLocation = dirStartBlock;
    }
    else{
        //dir->parentLocation = ?
    }

}

