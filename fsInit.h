#ifndef _FSINIT_H
#define _FSINIT_H
#include <stdio.h>

#include "fsLow.h"


typedef struct{
    //volume name
    //magic number
    uint64_t blockCount; //number of blocks in volume
    uint64_t sizeOfBlocks;//size of blocks
    uint64_t freeBlockCount;//free block count
    //free block pointers
    uint64_t freeFCBCount;//free FCB count
    //FCB pointers
    //free space list pointer
    //root directory pointer
}vcb;

typedef struct{
    unsigned int* freeSpaceBitmap;
    uint64_t freeSpaceListSize;
    uint64_t blocksUsed;
}fSL;

typedef struct{
    
}rootDir;

void formatVolume(char* volumeName);
vcb* initVCB(uint64_t volSize, uint64_t blockSize);
fSL* initFSL(uint64_t volSize, uint64_t blockSize);
rootDir* initRD(uint64_t blockSize, uint64_t blocks, fSL* fsl);

int findFreeBlocks(fSL* fsl, uint64_t blocksNeeded);

#endif