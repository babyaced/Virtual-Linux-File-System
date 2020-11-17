#ifndef _DIR_MGR_H
#define _DIR_MGR_H
#include "fsInit.h"

#define TABLE_SIZE 54  //maximum size to keep dir Size under 512 bytes

typedef struct dirEnt dirEnt;


struct dirEnt{
    char name[255];
    //metadata //as long as this is less than 512 bytes, than we don't need to know sizeInBlocks
    int parentLoc;
    int loc;
    //int sizeInBytes;
    //int sizeInBlocks;
    short int type;  //0 for file, 1 for directory

    //data
    int dataIndex; //can be file or directory
    int dataBlkCnt; //can be file or directory //contiguous
    int dataSize; //holds size of data in bytes
    //dirEnt* next; //in case of collisions //IGNORE FOR NOW
    
};


typedef struct{
    char name[255];
    int parentLoc;
    int loc;
    //int sizeInBytes;
    //int sizeInBlocks;
    int dirEnts[TABLE_SIZE];
    
}dir;


int initDir(int block, char* name);

int initFile(int block, char* name);

void initDirEntries(dir* d);


int mkFile(char *pathname, dir* d);
int findDirEnt(const char* pathName);
void addDirEnt(dir* parentDir, dirEnt* de);


int findNextDirEnt(int directoryIndex, int startingDirectoryEntryIndex);


//helper Functions
int toBlockSize(int size);
#endif
