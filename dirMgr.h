#ifndef _DIR_MGR_H
#define _DIR_MGR_H
#include "fsInit.h"

#define TABLE_SIZE 54  //maximum size to keep dir Size under 512 bytes

typedef struct dirEnt dirEnt;


struct dirEnt{
    //metadata //as long as this is less than 512 bytes, than we don't need to know sizeInBlocks
    char name[255];
    int parentLoc;
    int loc;
    int sizeInBytes;
    int sizeInBlocks;
    short int type;  //0 for file, 1 for directory

    //data
    int fileIndex; //can be file or directory
    int fileBlkCnt; //can be file or directory //contiguous
    //dirEnt* next; //in case of collisions //IGNORE FOR NOW
};


typedef struct{
    char name[255];
    int parentLoc;
    int loc;
    int sizeInBytes;
    int sizeInBlocks;
    unsigned int dirEnts[TABLE_SIZE];
}dir;


int initDir(int block, char* name);

void initDirEntries(dir* d);
void uninitDirEntries(dir* d);

int findFreeDirEnt(dir* d);
int findDirEnt(char* dirName, char* baseName);
void addDirEnt(dir* parentDir, dirEnt* de);

#endif