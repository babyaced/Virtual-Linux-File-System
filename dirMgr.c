#include <stdlib.h>
#include <string.h>
#include "dirMgr.h"
#include "freeMgr.h"
#include "fsInit.h"
#include "hashTable.h"

#define TABLE_SIZE 54

extern fSL* fsl;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock; //holds current LBA block for use with relative pathnames
extern int currentBlockSize;

int initDir(int parentBlock, char* name){  //pass in block of whatever directory entry this is called from //this is mostly likely called from user accessible "mkdir" function)
    int retVal;

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(blocksNeeded); //find next available blocks
    
    printf("Mallocing: %d bytes\n", bytesNeeded);
    dir* d = malloc(bytesNeeded);
    d->sizeInBlocks = blocksNeeded;
    d->sizeInBytes = bytesNeeded;
    d->loc = dirStartBlock;
    if(parentBlock == 0){
        d->parentLoc = dirStartBlock;  //parent is itself
        d->loc = dirStartBlock;
        vcb->rdLoc = d->parentLoc;
        vcb->rdBlkCnt = d->sizeInBlocks;
        vcb->rdLoc = d->loc;
        strcpy(d->name,name);
        currentBlock = dirStartBlock;
        currentBlockSize = blocksNeeded;
        setFreeBlocks(dirStartBlock,blocksNeeded);
    }
    else{
        d->parentLoc = parentBlock; // parent is at block index passed in
        d->loc = dirStartBlock;
        strcpy(d->name, name);
    }

    //initDirEntries(d);
    retVal = LBAwrite(d,d->sizeInBlocks, dirStartBlock);
    printf("Current free block: %d\n", d->sizeInBlocks + dirStartBlock);
    printf("Freeing: %d bytes\n", bytesNeeded);


    uninitDirEntries(d);
    free(d);
    d = NULL;

    return dirStartBlock;
}

void initDirEntries(dir* d){ 
    int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    printf("Length: %d\n",length);
    printf("Mallocing: %ld Bytes\n", sizeof(d->dirEnts));
    for(int i = 0; i < length; i++){
        d->dirEnts[i] = malloc(sizeof(dirEnt*));
    }
    printf("Size of dirEnt[length-1]: %ld\n",sizeof(d->dirEnts[length-1]));
}

void uninitDirEntries(dir* d){ 
    int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    printf("Length: %d\n",length);
    printf("Freeing: %ld Bytes\n", sizeof(d->dirEnts));
    for(int i = 0; i < length; i++){
        free(d->dirEnts[i]);
        d->dirEnts[i] = NULL;
    }
    printf("Size of dirEnt[length-1]: %ld\n",sizeof(d->dirEnts[length-1]));
}


int findFreeDirEnt(dir* d){
    //int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    //search through d->dirEnts for next free dirEnt
    //searching with hashtable should allow unique, and therefore, free entry to be found
    //if not, hashtable will iterate through links until it finds free bit or entry with no size
    return 0;
}


int findDirEnt(char* dirName, char* baseName){  // will eventually be edited to take in LBA from caller that is starting directory
    //for now start at root and iterate through directories
    //split directory name into parts
    char* token;
    char* remainder = dirName;

    //get root directory(temporary for testing) //real version will just read directory passed in to this function
    int retVal;
    // int rootDirLoc = vcb->rdLoc;  //hold root dir index
    // int rootDirBlks = vcb->rdBlkCnt; //# of blocks allocated to root

    //Relative Case
    int sizeOfDir = sizeof(dir);
    int sizeOfDirEnts = 54*sizeof(dirEnt*);
    printf("Size of Dir: %d\n", sizeOfDir);
    printf("Size of Dir Ents: %d\n", sizeOfDirEnts);
    dir* d = malloc(sizeOfDir); //allocate memory for dir // 720 is temporary
    //initDirEntries(d);
    printf("Mallocing: %d Bytes\n", sizeOfDir);
    dirEnt* de = malloc(sizeof(dirEnt*));
    printf("Mallocing: %ld Bytes\n", sizeof(dirEnt*));
    retVal = LBAread(d,currentBlockSize,currentBlock); // read into our dir function
    //hash_table_lookup
    while((token = strtok_r(remainder, "/",&remainder))){ //continues while subdirectory exists
        printf("Token: %s\n", token);  //prints next directory
        de = hash_table_lookup(token,d->dirEnts);  //look up the name in directory entries of d
        if(de == NULL)
            return -1;  //return errorCode
        retVal = LBAread(de, de->sizeInBlocks, de->loc);  //read directory entry (NOT FILE ITSELF) into dirEnt
        if(de->type == 1) //if directory entry is a directory
            retVal = LBAread(d, de->fileBlkCnt,de->fileIndex);//read new starting directory into d*/
    }
    printf("Freeing: %d Bytes\n", sizeOfDir);
    uninitDirEntries(d);
    free(d);
    d = NULL;
    printf("Freeing: %ld Bytes\n", sizeof(dirEnt*));
    //free(de);
    //de = NULL;


    //return de->fileIndex;//returns logical block index of file pointed to by directory entry  //if we keep it like this, we could reuse this code for cd and b_open potentially
                         //Explanation: if loop reaches end and directory is valid and occupies basename, then just return fileIndex of directory, (CD)
                         //             OR if file is present at that fileIndex then it will have returned a file (b_open and potentially other functions)
    return 0; //so it runs
}

void addDirEnt(dir* parentDir, dirEnt* dE){
    bool success;
    if(dE->type ==1)  // if type of directory entry is directory
    {
        hash_table_insert(dE,parentDir);  //need to edit hash_table_insert to take other info (ie. location)
    }
    else if(dE->type == 0)//else if type is file
    {
        success = hash_table_insert(dE,parentDir);  //need to only pass pointer to dirEnt
    }
    
}