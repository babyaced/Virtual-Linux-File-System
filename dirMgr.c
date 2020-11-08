#include <stdlib.h>
#include <string.h>
#include "dirMgr.h"
#include "freeMgr.h"
#include "fsInit.h"
#include "hashTable.h"

#define TABLE_SIZE 54

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock; //holds LBA block of current directory for use with relative pathnames
extern int currentBlockSize;  //holds # of blocks taken by current directory

int initDir(int parentBlock, char* name){  //pass in block of whatever directory this is called from //this is mostly likely called from user accessible "mkdir" function)
    int retVal;
    printf("Initializing directory named %s\n", name);

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(blocksNeeded); //find next available blocks
    
    printf("Mallocing: %ld bytes\n", sizeof(dir));
    dir* d = malloc(bytesNeeded);
    d->sizeInBlocks = blocksNeeded;
    d->sizeInBytes = bytesNeeded;
    d->loc = dirStartBlock;
    if(parentBlock == 0){
        d->parentLoc = dirStartBlock;  //parent is itself
        d->loc = dirStartBlock;
        //initialize root directory variables of vcb

        vcb->rdLoc = d->parentLoc; 
        vcb->rdBlkCnt = d->sizeInBlocks;
        vcb->rdLoc = d->loc;
        strcpy(d->name,name);
        setFreeBlocks(dirStartBlock,blocksNeeded); //modify free space bitmap to indicate blocks taken up by this directory
        currentBlock = dirStartBlock;  //when root directory is initialized, set currentBlock to it, so future child directories can easily use it
        currentBlockSize = d->sizeInBlocks;
    }
    else{
        d->parentLoc = parentBlock; // parent is at block index passed in
        d->loc = dirStartBlock;
        strcpy(d->name, name);
        setFreeBlocks(dirStartBlock,blocksNeeded); //modify free space bitmap to indicate blocks taken up by this directory
    }

    initDirEntries(d);
    retVal = LBAwrite(d,d->sizeInBlocks, dirStartBlock);
    printf("Current free block: %d\n", d->sizeInBlocks + dirStartBlock);
    


    // uninitDirEntries(d);
    printf("Freeing: %d bytes\n", bytesNeeded);
    free(d);
    d = NULL;

    return dirStartBlock;
}

void initDirEntries(dir* d){ 
    int length = sizeof(d->dirEnts)/sizeof(d->dirEnts[0]);
    printf("Length: %d\n",length);
    for(int i = 0; i < length; i++){
        d->dirEnts[i] = -1;
    }
    printf("Size of dirEnt[length-1]: %ld\n",sizeof(d->dirEnts[length-1]));
}

/*void uninitDirEntries(dir* d){ 
    int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    printf("Length: %d\n",length);
    printf("Freeing: %ld Bytes\n", sizeof(d->dirEnts));
    for(int i = 0; i < length; i++){
        free(d->dirEnts[i]);
        d->dirEnts[i] = NULL;
    }
    printf("Size of dirEnt[length-1]: %ld\n",sizeof(d->dirEnts[length-1]));
}*/


int findFreeDirEnt(dir* d){
    //int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    //search through d->dirEnts for next free dirEnt
    //searching with hashtable should allow unique, and therefore, free entry to be found
    //if not, hashtable will iterate through links until it finds free bit or entry with no size
    return 0;
}


int findDirEnt(char* pathname){  // will eventually be edited to take in LBA from caller that is starting directory
    //for now start at root and iterate through directories
    //split directory name into parts
    char* token;
    char* remainder = pathname;
    int retVal;
    int deIndex;

    bool abortFlag;

    printf("Mallocing: %ld Bytes\n", sizeof(dir));
    dir* d = malloc(sizeof(dir)); //allocate memory for dir // 720 is temporary
    printf("Mallocing: %ld Bytes\n", sizeof(dirEnt));
    dirEnt* de = malloc(sizeof(dirEnt));
   

    //=========================================================================================
    //ABSOLUTE PATH //we need to start from root
    //=========================================================================================
    if(pathname[0] == '/') {//if pathname is absolute
        retVal = LBAread(d,vcb->rdBlkCnt,vcb->rdLoc);  //read root directory into our directory
    }
    //=========================================================================
    //Relative Path Name  //use currentBlock data to get parent directory
    //=========================================================================
    else{
        retVal = LBAread(d,currentBlockSize,currentBlock); // read into our dir function
    }
    

    
    while((token = strtok_r(remainder, "/",&remainder))){ //continues while subdirectory exists
        if(abortFlag == true){ //we are trying to iterate into a file, not a directory
            return -1;  //return error
        }
        printf("Token: %s\n", token);  //prints next directory
        deIndex = hash_table_lookup(token,d);  //look up the name in directory entries of d
        if(de == NULL)
        {
            //create file by default for now

            // return -1;  //return errorCode
        }
        retVal = LBAread(de, 1, deIndex);  //read directory entry (NOT FILE ITSELF) into dirEnt
        if(de->type == 1) //if directory entry is a directory
            retVal = LBAread(d, de->dataBlkCnt,de->dataIndex);//read new starting directory into d
        else if(de->type ==0){  //we are at a file
            abortFlag = true;  //if the loop continues after this, then loop will abort with error
                               //else, it will just continue after loop and return dataIndex
        }
    }
    printf("Freeing: %d Bytes\n", sizeof(dir));
    // uninitDirEntries(d);
    free(d);
    d = NULL;
    printf("Freeing: %ld Bytes\n", sizeof(dirEnt));
    free(de);
    de = NULL;


    return deIndex; //returns logical block index of directory entry pointed to by path  //if we keep it like this, we could reuse this code for cd and b_open potentially
                    //Explanation: if loop reaches end and directory is valid and occupies basename, then just return directory entry associated with directory, (CD)
                    //OR if file is present at that directory entry then it will have returned a directory entry associated with a file (b_open and potentially other functions)
}

void addDirEnt(dir* parentDir, dirEnt* dE){
    bool success;
    int retVal;
    if(dE->type ==1)  // if type of directory entry is directory
    {
        hash_table_insert(dE,parentDir);  //need to edit hash_table_insert to take other info (ie. location)
        retVal = LBAwrite(parentDir,parentDir->sizeInBlocks,parentDir->loc);
    }
    else if(dE->type == 0)//else if type is file
    {
        success = hash_table_insert(dE,parentDir);  //need to only pass pointer to dirEnt
    }
    
}