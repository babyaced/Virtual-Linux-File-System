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
    uint64_t rootDirLocation; //root directory pointer/index
}vCB;

typedef struct{
    char name[255];
    uint64_t parentLocation;
    uint64_t location;
    uint64_t sizeInBytes;
    uint64_t sizeInBlocks;
    //hash_table dirEnts[HASH_TABLE_SIZE]
}dir;

typedef struct{
    
}dirEnt;

typedef struct{
    unsigned int* freeSpaceBitmap;
    uint64_t freeSpaceBits;
    uint64_t location;
    uint64_t fslBlocksUsed;
}fSL;

void formatVolume(char* volumeName);
void initVCB(uint64_t volSize, uint64_t blockSize);
void initFSL(uint64_t volSize, uint64_t blockSize);
void initRD();
dir* initDir(uint64_t block);

int findFreeBlocks(uint64_t blocksNeeded);
void setFreeBlocks(int startingIndex,int count);

#endif