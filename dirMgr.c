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

int toBlockSize(int size) { // round up to full block sizes for lbaRead()
    int nBlocks = size / vcb->sizeOfBlocks;
    if (size % vcb->sizeOfBlocks > 0) ++nBlocks;

    return nBlocks * vcb->sizeOfBlocks;
}

int initDir(int parentBlock, char* name){  //pass in block of whatever directory this is called from //this is mostly likely called from user accessible "mkdir" function)
    int retVal;
    // printf("Initializing directory named %s\n", name);

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(blocksNeeded); //find next available blocks
    
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dir)));
    dir* d = malloc(toBlockSize(sizeof(dir)));
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
    //d->sizeInBlocks = blocksNeeded;
    //d->sizeInBytes = bytesNeeded;
    d->loc = dirStartBlock;
    if(parentBlock == 0){  //initialize root directory
        d->parentLoc = dirStartBlock;  //parent is itself
        //initialize root directory variables of vcb
        vcb->rdLoc = d->parentLoc; 
        vcb->rdBlkCnt = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
        vcb->rdLoc = d->loc;
        currentBlock = dirStartBlock;  //when root directory is initialized, set currentBlock to it, so future child directories can easily use it
        currentBlockSize = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
        de->parentLoc = dirStartBlock;
    }
    else{  //use parent block passed in
        d->parentLoc = parentBlock; // parent is at block index passed in
        de->parentLoc = parentBlock;
    }
    strcpy(d->name,name);
    initDirEntries(d);  //initialize dirEnts
    setFreeBlocks(dirStartBlock,blocksNeeded); //modify free space bitmap to indicate blocks taken up by this directory
    
    //initialize directory entry
    strcpy(de->name,name);
    de->type = 1;
    int deStartBlock = findFreeBlocks(blocksNeeded);
    de->loc = deStartBlock;
    de->dataBlkCnt = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
    de->dataIndex = dirStartBlock;

    //add dirent of root directory to root directory or to parent directory
    retVal = LBAwrite(de,toBlockSize(sizeof(dirEnt))/vcb->sizeOfBlocks,deStartBlock);
    setFreeBlocks(deStartBlock,(sizeof(dirEnt)/vcb->sizeOfBlocks)+1);
    if(parentBlock != 0){  //if directory is not root
        // d = NULL;
        retVal = LBAread(d,(sizeof(dir)/vcb->sizeOfBlocks)+1,parentBlock);  //read parent into directory
    }  //else root directory entry will be written to root directory

    //add directory entry to root directory or to parent directory
    addDirEnt(d,de);


    //write directory with directory entry added to disk    
    retVal = LBAwrite(d,toBlockSize(sizeof(dir))/vcb->sizeOfBlocks, dirStartBlock);
    //printf("Current free block: %d\n", toBlockSize(sizeof(dir))/vcb->sizeOfBlocks + dirStartBlock);
    
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    free(de);
    de = NULL;
    printf("Freeing: %d bytes\n", toBlockSize(sizeof(dir)));
    initDirEntries(d);
    free(d);
    d = NULL;

    return dirStartBlock;  //returns block of data of directory, not directory entry of directory
}

void initDirEntries(dir* d){ 
    int length = sizeof(d->dirEnts)/sizeof(d->dirEnts[0]);
    // printf("Length: %d\n",length);
    for(int i = 0; i < length; i++){
        d->dirEnts[i] = -1;
    }
}


int findFreeDirEnt(dir* d){
    //int length = sizeof(d->dirEnts) / sizeof(dirEnt*);
    //search through d->dirEnts for next free dirEnt
    //searching with hashtable should allow unique, and therefore, free entry to be found
    //if not, hashtable will iterate through links until it finds free bit or entry with no size
    return 0;
}

int mkFile(char *pathname){ // makes an empty directory entry, maybe add "mode_t mode"
    // type = 0 for a file
    int retVal;
    char* pathnameCpy = strdup(pathname);

    // getting fileName
    char *fileName = pathnameCpy;
    if (strstr(pathname, "/") != NULL) fileName = strrchr(pathnameCpy, '/')+1; // if dir is included in path, separate for fileName
    printf("\n%s\n\n", fileName);

    if(pathname[0] == '/') {//if pathname is absolute
        printf("\n***absolute directory in mkFile: Need to implement***\n");
    }
    else{
//        dir* parentDir = malloc(sizeof(dir));
//        printf("Mallocing: %ld bytes\n", sizeof(dir));
//        retVal = LBAread(parentDir,currentBlockSize,currentBlock);
//
//        //Malloc Directory entry
//        dirEnt* de = malloc(sizeof(dirEnt));
//        printf("Mallocing: %ld bytes\n", sizeof(dirEnt));
//
//        //initialize directory entry
//        de->parentLoc = currentBlock;
//
//        //// Need to do: dataIndex
//
//        //de->dataIndex = dirIndex; //store pointer to data
//        de->dataBlkCnt = vcb->rdBlkCnt;
//        de->sizeInBlocks = vcb->rdBlkCnt;//all directories should be same size
//        de->type = 0; //type is file
//        strcpy(de->name,fileName); // maybe Pathname?
//
//        //write directory entry to disk
//        int deIndex  = findFreeBlocks((sizeof(dirEnt)/vcb->sizeOfBlocks) +1);
//        de->loc = deIndex;  //store location of directory entry
//        retVal = LBAwrite(de,(sizeof(dirEnt)/vcb->sizeOfBlocks) +1,deIndex); //Is there any point to this?
//        setFreeBlocks(deIndex,(sizeof(dirEnt)/vcb->sizeOfBlocks) +1);
//        addDirEnt(parentDir,de);//write this  directory(only index to metadata) to parent dir's dirEnts
//        free(parentDir);
//        printf("Freeing: %ld bytes\n", sizeof(dir));
//        parentDir = NULL;
//        free(de);
//        printf("Freeing: %ld bytes\n", sizeof(dirEnt));
//        de = NULL;
    }



//    }
//
//
//    //int freeLoc = findFreeDirEnt();
//
//    printf("freeLoc: %d ", freeLoc);
//
//    printf("heyyo\n");
    return 1;
}

int findDirEnt(char* pathname){  // will eventually be edited to take in LBA from caller that is starting directory
    //for now start at root and iterate through directories
    //split directory name into parts
    char* token;
    char* remainder = strdup(pathname);
    int retVal;
    int deIndex;

    bool abortFlag;

    printf("Mallocing: %ld Bytes\n", toBlockSize(sizeof(dir)));
    dir* d = malloc(toBlockSize(sizeof(dir))); //allocate memory for dir // 720 is temporary
    printf("Mallocing: %ld Bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
   

    //=========================================================================================
    //ABSOLUTE PATH //we need to start from root
    //=========================================================================================
    if(pathname[0] == '/') {//if pathname is absolute
        retVal = LBAread(d,vcb->rdBlkCnt,vcb->rdLoc);  //read root directory into our directory
    }
    //=========================================================================
    //Relative Path Name  //use currentBlock data to get starting directory
    //=========================================================================
    else{
        retVal = LBAread(d,currentBlockSize,currentBlock); // read into our dir function
    }
    
    while((token = strtok_r(remainder, "/",&remainder))){ //continues while subdirectory exists
        if(abortFlag == true){ //we are trying to iterate into a file, not a directory
            return -1;  //return error
        }
        // printf("Token: %s\n", token);  //prints next directory
        deIndex = hash_table_lookup(token,d);  //look up the name in directory entries of d
        if(deIndex == -1)
        {
            //create file by default for now
                  // mkFile(pathname);

            // return -1;  // or return errorCode
        }
        retVal = LBAread(de, 1, deIndex);  //read directory entry (NOT FILE ITSELF) into dirEnt
        if(de->type == 1) //if directory entry is a directory
            retVal = LBAread(d, de->dataBlkCnt,de->dataIndex);//read new starting directory into d
        else if(de->type ==0){  //we are at a file
            abortFlag = true;  //if the loop continues after this, then loop will abort with error
                               //else, it will just continue after loop and return dataIndex
        }
    }
    printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
    free(d);
    d = NULL;
    printf("Freeing: %ld Bytes\n", toBlockSize(sizeof(dirEnt)));
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
        retVal = LBAwrite(parentDir,toBlockSize(sizeof(dir))/vcb->sizeOfBlocks,parentDir->loc);
    }
    else if(dE->type == 0)//else if type is file
    {
        success = hash_table_insert(dE,parentDir);  //need to only pass pointer to dirEnt
    }   
}
