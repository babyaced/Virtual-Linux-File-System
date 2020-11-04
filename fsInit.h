#ifndef _FSINIT_H
#define _FSINIT_H
#include "mfs.h"

typedef struct{
    //volume name
    //magic number
    int blockCount; //number of blocks in volume
    int sizeOfBlocks;//size of blocks
    int freeBlockCount;//free block count
    //free block pointers
    int freeFCBCount;//free FCB count
    //FCB pointers
    //free space list pointer

    int rdLoc; //root directory pointer/index
    int rdBlkCnt;        //blocks taken up by root directory

    //uint64_t fslLoc;  //free space list pointer/index
    int fslBlkCnt;    //blocks taken up by free space list
    int fslBytes;
}vCB;

typedef struct{
    unsigned int* freeSpaceBitmap;
    int freeSpaceBytes;
    int freeSpaceBits;
    int location;
    int fslBlocksUsed;
}fSL;

void formatVolume(char* volumeName);
void initVCB(int volSize, int blockSize);
void initFSL(int volSize, int blockSize);
void initRD();

#endif