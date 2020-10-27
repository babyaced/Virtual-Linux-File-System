#ifndef _FSINIT_H
#define _FSINIT_H
#include "mfs.h"

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

    uint64_t rdLoc; //root directory pointer/index
    uint64_t rdBlkCnt;        //blocks taken up by root directory

    //uint64_t fslLoc;  //free space list pointer/index
    uint64_t fslBlkCnt;    //blocks taken up by free space list
    uint64_t fslBytes;
}vCB;

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

#endif