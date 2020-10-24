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
    //root directory pointer
}vcb;

typedef struct{
    unsigned int* freeSpaceBitmap;
    uint64_t freeSpaceListSize;
}fSL;

void formatVolume(char* volumeName);
vcb* initVCB(uint64_t volSize, uint64_t blockSize);
fSL* initfSL(uint64_t volSize, uint64_t blockSize);

#endif