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

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(vcb,fsl,blocksNeeded); //find next available blocks
    
    dir* d = malloc(bytesNeeded);
    d->sizeInBlocks = blocksNeeded;
    d->sizeInBytes = bytesNeeded;
    d->loc = dirStartBlock;
    if(block == 0){
        d->parentLoc = dirStartBlock;  //parent is itself
        vcb->rdLoc = d->parentLoc;
    }
    else{
        d->parentLoc = block; // parent is at block index passed in
    }

    initDirEntries(d);
    retVal = LBAwrite(d,d->sizeInBlocks, dirStartBlock);
    printf("Current free block: %ld\n", d->sizeInBlocks + dirStartBlock);
    free(d);
    //free(vcb);
    //free(d);
    /*dir* d2 = malloc(bytesNeeded);
    retVal = LBAread(d2,d->sizeInBlocks,dirStartBlock);
    printf("D2 size in blocks: %d\n", d2->sizeInBlocks);*/
}

void initDirEntries(dir* d){
    int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    printf("Length: %d\n",length);
    for(int i = 0; i < length; i++){
        d->dirEnts[i] = malloc(sizeof(dirEnt));
        d->dirEnts[i]->parentLoc = d->parentLoc;
        d->dirEnts[i]->loc = d->loc;
        d->dirEnts[i]->sizeInBlocks = 0;
        d->dirEnts[i]->sizeInBytes = 0;
    }
}