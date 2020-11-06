#include "freeMgr.h"
#include "fsInit.h"

extern fSL* fsl;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program

int findFreeBlocks(int blocksNeeded){
    //access vcb to get fsl->location
    int retVal;
    //fSL* fsl;// = malloc(vcb->fslBytes);
    
    //retVal = LBAread(fsl,vcb->fslBlkCnt,1);

    int freeBlockCounter = 0;
    for(int i = 0; i < fsl->freeSpaceBytes;i++){ 
        if(freeBlockCounter == blocksNeeded)
            return i - blocksNeeded;
        if(!checkBit(fsl->freeSpaceBitmap,i))  //if bit is free(0)
            freeBlockCounter++;  //increment contigious free block counter
    }
}

void setFreeBlocks(int startingIndex,int count){
    /*vCB* vcb = malloc(sizeof(vCB));
    fSL* fsl = malloc(sizeof(fSL));
    fsl->freeSpaceBitmap = malloc(vcb->fslBytes);*/
    int retVal;

    /*retVal = LBAread(vcb,1,0);
    retVal = LBAread(fsl,vcb->fslBlkCnt,1);*/
    
    for(int i = startingIndex; i < count + startingIndex; i++){  //for blocks written to LBA
        setBit(fsl->freeSpaceBitmap,i);                          //set freeSpaceBitmap bits to occupied(1)
    }
    retVal = LBAwrite(fsl,vcb->fslBlkCnt,1);                     //write new free space list to LBA
//    free(vcb);
//    free(fsl);
}