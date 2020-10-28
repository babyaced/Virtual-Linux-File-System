#ifndef _DIR_MGR_H
#define _DIR_MGR_H
#include "fsInit.h"

#define TABLE_SIZE 54  //maximum size to keep dirEnt Size under 512 bytes


typedef struct{
    char name[255];
    uint64_t parentLoc;
    uint64_t loc;
    uint64_t sizeInBytes;
    uint64_t sizeInBlocks;
    short int type;
    //dir* dirEnts[TABLE_SIZE];
    //file*?
}dirEnt;


typedef struct{
    char name[255];
    uint64_t parentLoc;
    uint64_t loc;
    uint64_t sizeInBytes;
    uint64_t sizeInBlocks;
    dirEnt* dirEnts[TABLE_SIZE];
}dir;


void initDir(vCB* vcb, fSL* fsl,uint64_t block);
void initDirEntries(dir* d);

#endif