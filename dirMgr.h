#ifndef _DIR_MGR_H
#define _DIR_MGR_H
#include "fsInit.h"

#define TABLE_SIZE 54  //maximum size to keep dir Size under 512 bytes

typedef struct dirEnt dirEnt;


struct dirEnt{
    char name[255];
    int parentLoc;
    int loc;
    int sizeInBytes;
    int sizeInBlocks;
    short int type;  //0 for file, 1 for directory
    int fileIndex; //can be file or directory
    int fileBlkCnt; //can be file or directory //contiguous
    dirEnt* next; //in case of collisions
};


typedef struct{
    char name[255];
    int parentLoc;
    int loc;
    int sizeInBytes;
    int sizeInBlocks;
    //dirEnt* dirEnts[TABLE_SIZE];
}dir;


void initDir(vCB* vcb, fSL* fsl,int block);
void initDirEntries(dir* d);
int findFreeDirEnt(dir* d);
int findDirEnt(char* dirName);

#endif