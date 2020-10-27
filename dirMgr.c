#include <stdlib.h>
#include "dirMgr.h"
#include "freeMgr.h"
#include "fsInit.h"

void initDir(vCB* vcb, fSL* fsl,uint64_t block){
    int retVal;
    
    /*vCB* vcb = malloc(sizeof(vCB));
    retVal = LBAread(vcb,1,0);
    

    fSL* fsl = malloc(sizeof(fSL));
    int blockCount = vcb->blockCount;
    int bmSize = blockCount/8;
    fsl->freeSpaceBitmap = malloc(bmSize + 1);
    retVal = LBAread(fsl,5,1);*/

    int bytesNeeded = sizeof(dir) + sizeof(dirEnt);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(vcb,fsl,blocksNeeded); //find next available blocks
    
    dir* d = malloc(bytesNeeded);
    d->sizeInBlocks = blocksNeeded;
    d->sizeInBytes = bytesNeeded;
    d->location = dirStartBlock;
    if(block == 0){
        d->parentLocation = dirStartBlock;  //parent is itself
        vcb->rdLoc = d->parentLocation;
    }
    else{
        d->parentLocation = block; // parent is at block index passed in
    }
    retVal = LBAwrite(d,d->sizeInBlocks, dirStartBlock);
    printf("Current free block: %ld\n", d->sizeInBlocks + dirStartBlock);
    //free(vcb);
    //free(d);
    /*dir* d2 = malloc(bytesNeeded);
    retVal = LBAread(d2,d->sizeInBlocks,dirStartBlock);
    printf("D2 size in blocks: %d\n", d2->sizeInBlocks);*/
}