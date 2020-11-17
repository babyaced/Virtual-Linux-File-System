#include "freeMgr.h"
#include "fsInit.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program

int findFreeBlocks(int blocksNeeded){
    int retVal;

    int freeBlockCounter = 0;
    for(int i = 0; i < vcb->fslBytes;i++){
        if(freeBlockCounter == blocksNeeded)
            return i - blocksNeeded;
        if(!checkBit(freeSpaceBitmap,i))  //if bit is free(0)
            freeBlockCounter++;  //increment contigious free block counter
    }
}

void setFreeBlocks(int startingIndex,int count){
    int retVal;
    for(int i = startingIndex; i < count + startingIndex; i++){  //for blocks written to LBA
        setBit(freeSpaceBitmap,i);                          //set freeSpaceBitmap bits to occupied(1)
    }
    retVal = LBAwrite(freeSpaceBitmap,vcb->fslBlkCnt,1);                     //write new free space list to LBA
}

void clearFreeBlocks(int startingIndex,int count){
    int retVal;
    for(int i = startingIndex; i < count + startingIndex; i++){  //for blocks written to LBA
        clearBit(freeSpaceBitmap,i);                          //set freeSpaceBitmap bits to free(0)
    }
    retVal = LBAwrite(freeSpaceBitmap,vcb->fslBlkCnt,1);                     //write new free space list to LBA
}