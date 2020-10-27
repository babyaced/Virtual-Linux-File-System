#include "freeMgr.h"

int findFreeBlocks(vCB *vcb,  fSL *fsl, uint64_t blocksNeeded){
    //access vcb to get fsl->location
    int retVal;
    //fSL* fsl;// = malloc(vcb->fslBytes);
    
    //retVal = LBAread(fsl,vcb->fslBlkCnt,1);

    int freeBlockCounter = 0;
    for(int i = 0; i < fsl->freeSpaceBits;i++){
        if(freeBlockCounter == blocksNeeded)
            return i - blocksNeeded;
        if(!checkBit(fsl->freeSpaceBitmap,i))
            freeBlockCounter++;
    }
}

void setFreeBlocks(vCB *vcb, fSL *fsl, int startingIndex,int count){
    /*vCB* vcb = malloc(sizeof(vCB));
    fSL* fsl = malloc(sizeof(fSL));
    fsl->freeSpaceBitmap = malloc(vcb->fslBytes);*/
    int retVal;

    /*retVal = LBAread(vcb,1,0);
    retVal = LBAread(fsl,vcb->fslBlkCnt,1);*/
    
    for(int i = startingIndex; i < count + startingIndex; i++){
        setBit(fsl->freeSpaceBitmap,i);
    }
    retVal = LBAwrite(fsl,vcb->fslBlkCnt,1);
//    free(vcb);
//    free(fsl);
}