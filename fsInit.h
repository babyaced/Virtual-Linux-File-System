#ifndef _FSINIT_H
#define _FSINIT_H
#include "mfs.h"

typedef struct{
    int blockCount; //number of blocks in volume
    int sizeOfBlocks;//size of blocks
    int freeBlockCount;//free block count
    //free block pointers
    //int freeFCBCount;//free FCB count
    //FCB pointers
    //free space list pointer

    int rdLoc; //root directory pointer/index

    //uint64_t fslLoc;  //free space list pointer/index
    int fslBlkCnt;    //blocks taken up by free space bitmap
    int fslBytes;     //bytes taken up by free space bitmap

    int dBlkCnt;        //size of directories in blocks
    int deBlkCnt;       //size of directory entries in blocks

    uint64_t magicNum;

}vCB;




void formatVolume(char* volumeName,uint64_t volSize, uint64_t blockSize);
void initVCB(int volSize, int blockSize);
void initFSL(int volSize, int blockSize);
void initRD();

void initGlobals(uint64_t volSize, uint64_t blockSize); //initializes global variables when volume is already formatted

void freeGlobals();

#endif