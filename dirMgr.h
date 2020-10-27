#ifndef _DIR_MGR_H
#define _DIR_MGR_H
#include "fsInit.h"

#define TABLE_SIZE 2441;

typedef struct{
    char name[255];
    uint64_t parentLocation;
    uint64_t location;
    uint64_t sizeInBytes;
    uint64_t sizeInBlocks;
    //dirEnt* hash_table[TABLE_SIZE];
}dir;

typedef struct{
    char name[255];
    uint64_t parentLocation;
    uint64_t location;
    uint64_t sizeInBytes;
    uint64_t sizeInBlocks;
    short int type;
    //hash_table dirEnts[HASH_TABLE_SIZE]
}dirEnt;

typedef struct{
    
}dirent;

void initDir(vCB* vcb, fSL* fsl,uint64_t block);

#endif