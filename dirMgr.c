#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "dirMgr.h"
#include "freeMgr.h"
#include "fsInit.h"
#include "hashTable.h"
#include "fsLow.h"


#define F_CREATE 0x01 //0b00000001
#define D_CREATE 0x02 //0b00000010

// #define TABLE_SIZE 54

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock; //holds LBA block of current directory for use with relative pathnames
extern int currentBlockSize;  //holds # of blocks taken by current directory
extern char currentBlockName[255]; //size of 255 for now

int toBlockSize(int size) { // round up to full block sizes for lbaRead()
    int nBlocks = size / vcb->sizeOfBlocks;
    if (size % vcb->sizeOfBlocks > 0) ++nBlocks;

    return nBlocks * vcb->sizeOfBlocks;
}

int initDir(int parentDEBlock, char* name){  //pass in block of whatever directory this is called from //OR in case of root directory init, pass in sentinel value of 0
    int retVal;
    // printf("Initializing directory named %s\n", name);


    dirEnt* dot = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* dotDot = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* parentDE = malloc(toBlockSize(sizeof(dirEnt)));

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks);
    int dirStartBlock = findFreeBlocks(blocksNeeded); //find next available blocks
    
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dir)));
    dir* initDirD = malloc(toBlockSize(sizeof(dir)));
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* initDirDE = malloc(toBlockSize(sizeof(dirEnt)));
    //initDirD->sizeInBlocks = blocksNeeded;
    //initDirD->sizeInBytes = bytesNeeded;
    initDirD->loc = dirStartBlock;
    if(parentDEBlock == 0){  //initialize root directory
        initDirD->parentLoc = dirStartBlock;  //parent is itself
        //initialize root directory variables of vcb
         
        vcb->dBlkCnt = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
        vcb->deBlkCnt = toBlockSize(sizeof(dirEnt))/vcb->sizeOfBlocks;
        currentBlockSize = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
        strncpy(currentBlockName,name, strlen(name));
        
    }
    else{  //use parent block passed in to initialize dotDot and our parent locs
        retVal = LBAread(parentDE,vcb->deBlkCnt, parentDEBlock);
        strncpy(dotDot->name,"..", 3);
        dotDot->loc = parentDE->loc;
        dotDot->type = 1;
        initDirD->parentLoc = parentDE->loc; // parent is at block index passed in
        initDirDE->parentLoc = parentDE->loc;
    }
    

    strncpy(initDirD->name,name,strlen(name));
    initDirD->name[strlen(name)] = '\0';

    initDirEntries(initDirD);  //initialize dirEnts
    
    setFreeBlocks(dirStartBlock,blocksNeeded); //modify free space bitmap to indicate blocks taken up by this directory
    
    //initialize directory entry
    strncpy(initDirDE->name,name,strlen(name));
    initDirDE->name[strlen(name)] = '\0';
    
    initDirDE->type = 1;
    int deStartBlock = findFreeBlocks(blocksNeeded);
    if(parentDEBlock == 0){
        currentBlock = deStartBlock;  //when root directory is initialized, set currentBlock to it, so future child directories can easily use it
        initDirDE->parentLoc = deStartBlock;
    }
    initDirDE->loc = deStartBlock;
    initDirDE->dataBlkCnt = vcb->dBlkCnt;
    initDirDE->dataIndex = dirStartBlock;

    //initialize "dot" directory entry
    strncpy(dot->name,".",2);
    dot->type = 1;

    dot->loc = initDirDE->loc;
    //write directory entry of directory to disk
    retVal = LBAwrite(initDirDE,vcb->deBlkCnt,deStartBlock);
    setFreeBlocks(deStartBlock,vcb->deBlkCnt);

    if(parentDEBlock == 0){  //if directory is root
        strncpy(dotDot->name,"..", 3);
        dotDot->type = 1;
        dotDot->loc = dot->loc;
        vcb->rdLoc = initDirDE->loc;


        addDirEnt(initDirD,initDirDE);                                //add root directory entry to root directory's directory entries
        addDirEnt(initDirD,dot);                                      //add dot directory entry to root directory's directory entries
        addDirEnt(initDirD,dotDot);                                   //add dot dot directory entry to root directory's directory entries
        // initDotDot(0);
        // retVal = LBAwrite(initDirD,vcb->dBlkCnt, dirStartBlock);
    } 
    else{
        addDirEnt(initDirD,dot);                                      //add dot directory entry
        addDirEnt(initDirD,dotDot);
        // retVal = LBAwrite(initDirD,vcb->dBlkCnt, dirStartBlock);  //write directory we are initializing now to disk
        //read directory of parent
        retVal = LBAread(initDirD,vcb->dBlkCnt,parentDE->dataIndex);      
        addDirEnt(initDirD,initDirDE);                                //add directory entry of directory initialized in this function to parent directory
        // retVal = LBAwrite(initDirD,vcb->dBlkCnt, parentDE->dataIndex);  //save directory of parent to disk

    }
    
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));

    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dir)));;
    free(dot);
    dot = NULL;
    free(dotDot);
    dotDot = NULL;
    free(initDirDE);
    initDirDE = NULL;
    free(initDirD);
    initDirD = NULL;
    free(parentDE);
    parentDE = NULL;

    return dirStartBlock;  //returns block of data of directory, not directory entry of directory 
}

void initDirEntries(dir* initDED){ 
    int length = sizeof(initDED->dirEnts)/sizeof(initDED->dirEnts[0]);
    // printf("Length: %d\n",length);
    for(int i = 0; i < length; i++){
        initDED->dirEnts[i] = -1;
    }
}

int initFile(int parentDEBlock, char* name){ //takes in parent directory data block and name of new file
    int retVal;
    //this just creates directory entry in directory at parentBlock
    int deStartBlock = findFreeBlocks(vcb->deBlkCnt); //find next available blocks for our new directory entry

    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* initFileDE = malloc(toBlockSize(sizeof(dirEnt)));   //malloc memory for directory entry we want to initialize
    strncpy(initFileDE->name,name, strlen(name));
    initFileDE->name[strlen(name)] ='\0';
    initFileDE->parentLoc = parentDEBlock;
    initFileDE->type = 0;
    initFileDE->loc = deStartBlock;
    initFileDE->dataIndex = 0;      //right now its just an empty file with no data
    initFileDE->dataBlkCnt = 0;     //right now its just an empty file with no data

    // extents, init count to 0
    initFileDE->ext1.count = 0;
    initFileDE->ext2.count = 0;
    initFileDE->ext3.count = 0;
    initFileDE->ext4.count = 0;
    initExts(initFileDE, MAX_SEC_EXTENTS);

    retVal = LBAwrite(initFileDE,vcb->deBlkCnt,deStartBlock);
    setFreeBlocks(deStartBlock,vcb->deBlkCnt);

    //add this directory entry to directory at parent block
    dirEnt* parentDE = malloc(toBlockSize(sizeof(dirEnt)));          // malloc memory for parent directory entry
    retVal = LBAread(parentDE,vcb->deBlkCnt,parentDEBlock);   // read the parent directory entry into variable

    dir* parentD = malloc(toBlockSize(sizeof(dir)));
    retVal = LBAread(parentD,vcb->dBlkCnt,parentDE->dataIndex);   // read the parent directory entry into variable

    addDirEnt(parentD,initFileDE);   //add the new file's directory entry to directory passed in

    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dir)));
    // retVal = LBAwrite(parentD,vcb->dBlkCnt,parentDE->dataIndex);

    int loc = initFileDE->loc;
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    free(initFileDE);
    initFileDE = NULL;
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dir)));
    free(parentDE);
    parentDE = NULL;
    free(parentD);
    parentD = NULL;
    return loc;  //return directory entry location, b_write will decide dataIndex and dataBlkCnt later
}

int findDirEnt(const char* pathname, u_int8_t options){  // will eventually be edited to take in LBA from caller that is starting directory
    //for now start at root and iterate through directories
    //split directory name into parts
    char* original;
    char* token;
    char* remainder;
    original = strdup(pathname);
    remainder = original;
    int retVal;
    int deIndex;
    int counter = 0;

    bool abortFlag = false;

    // printf("Mallocing: %d Bytes\n", toBlockSize(sizeof(dir)));
    dir* findDirEntD = malloc(toBlockSize(sizeof(dir))); //allocate memory for dir // 720 is temporary
    // printf("Mallocing: %d Bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* findDirEntDE = malloc(toBlockSize(sizeof(dirEnt)));
   

    //=========================================================================================
    //ABSOLUTE PATH //we need to start from root
    //=========================================================================================
    if(pathname[0] == '/') {//if pathname is absolute
        deIndex = vcb->rdLoc; //set deIndex to rd deIndex in case user wants to change directory to root
        dirEnt* currentBlockDE = malloc(toBlockSize(sizeof(dirEnt)));
        retVal = LBAread(currentBlockDE,vcb->deBlkCnt,vcb->rdLoc);
        retVal = LBAread(findDirEntD,vcb->dBlkCnt,currentBlockDE->dataIndex);  //read root directory into our starting directory
        free(currentBlockDE);
        currentBlockDE = NULL;
    }
    //=========================================================================
    //Relative Path Name  //use currentBlock data to get starting directory
    //=========================================================================
    else{
        dirEnt* currentBlockDE = malloc(toBlockSize(sizeof(dirEnt)));
        retVal = LBAread(currentBlockDE,vcb->deBlkCnt,currentBlock); // read current block into our directory
        retVal = LBAread(findDirEntD, vcb->dBlkCnt, currentBlockDE->dataIndex);
        free(currentBlockDE);
        currentBlockDE = NULL;
    }
    while((token = strtok_r(remainder, "/",&remainder))){ //continues while subdirectory exists
        if(abortFlag == true){ //we are trying to iterate into a file, not a directory
            return -1;  //return error
        }
        // printf("Token: %s\n", token);  //prints next directory
        deIndex = hash_table_lookup(token,findDirEntD);  //look up the name in directory entries of findDirEntD
        if(deIndex == -1)  //if directory entry is not found, either the directory doesn't exist or the caller wants to create a file or directory
        { 
            if(strcmp(remainder,"") == 0){ //if pathname is empty then we are on the last iteration //therefore the intention is to create a file or directory
              if(options & F_CREATE){
                //create file by default for now
                if(counter == 0)   //if the pathname is only one directory long
                    retVal = initFile(currentBlock,token);  
                else               //
                    retVal = initFile(findDirEntDE->loc,token);
              }
              else if(options & D_CREATE){
                if(counter == 0)   //if the pathname is only one directory long
                    retVal = initDir(currentBlock,token);  
                else               //
                    retVal = initDir(findDirEntDE->loc,token);  //need to change to pass in directory entry loc, not data loc
              }
                free(original);
                original = NULL;
                // printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dirEnt)));
                free(findDirEntDE); //if pathname is e
                findDirEntDE = NULL;
                // printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
                free(findDirEntD);
                findDirEntD = NULL;
                return retVal;
            }
            else{  //we are trying to iterate into directory that does not exist
                free(original);
                original = NULL;
                // printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dirEnt)));
                free(findDirEntDE);
                findDirEntDE = NULL;
                // printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
                free(findDirEntD);
                findDirEntD = NULL;
                return -1; //return error
            }
        }
        retVal = LBAread(findDirEntDE, 1, deIndex);  //read directory entry (NOT FILE ITSELF) into dirEnt
        if(findDirEntDE->type == 1) //if directory entry is a directory
        {
            counter++;
            retVal = LBAread(findDirEntD, findDirEntDE->dataBlkCnt,findDirEntDE->dataIndex);//read new starting directory into d

        }
        else if(findDirEntDE->type ==0){  //we are at a file
            abortFlag = true;  //if the loop continues after this, then loop will abort with error
                               //else, it will just continue after loop and return dataIndex //required to work with b_open
        }
    }
    free(original);
    original = NULL;
    // printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dirEnt)));
    free(findDirEntDE);
    findDirEntDE = NULL;
    // printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
    free(findDirEntD);
    findDirEntD = NULL;

    return deIndex; //returns logical block index of directory entry pointed to by path  //if we keep it like this, we could reuse this code for cd and b_open potentially
                    //Explanation: if loop reaches end and directory is valid and occupies basename, then just return directory entry associated with directory, (CD)
                    //OR if file is present at that directory entry then it will have returned a directory entry associated with a file (b_open and potentially other functions)
}

void addDirEnt(dir* parentDir, dirEnt* dE){
    bool success;
    int retVal;
    if(dE->type ==1)  // if type of directory entry is directory
    {
        success = hash_table_insert(dE,parentDir);  //need to edit hash_table_insert to take other info (ie. location)
        if(success == false){
            printf("ERROR: Failed to add Directory, parent directory is full!\n");
        }
        else{
            retVal = LBAwrite(parentDir,vcb->dBlkCnt,parentDir->loc);
        }
        
    }
    else if(dE->type == 0)//else if type is file
    {
        success = hash_table_insert(dE,parentDir);  //need to only pass pointer to dirEnt
        if(success == false){
            printf("ERROR: Failed to add file, parent directory is full!\n");
        }
        else{
            retVal = LBAwrite(parentDir,vcb->deBlkCnt,parentDir->loc);
        }
    }   
}

int findNextDirEnt(int directoryIndex, int startingDirectoryEntryIndex){
    int retVal;
    dir* findNextDirEntD = malloc(toBlockSize(sizeof(dir)));
    retVal = LBAread(findNextDirEntD, vcb->dBlkCnt, directoryIndex);
    // dirEnt* findNextDirEntDE = malloc(toBlockSize(sizeof(dirEnt)));

    for(int i = startingDirectoryEntryIndex; i< (sizeof(findNextDirEntD->dirEnts)/(sizeof(dirEnt))); i++)
    {
        
        if(findNextDirEntD->dirEnts[i] != -1)
        {
            //retVal = LBAread(findNextDirEntDE, vcb->dSize,findNextDirEntD->dirEnts[i]); //read into our dummy dir ent
            //return findNextDirEntD->dirEnts[i];
            return i;
        }
    }
    return -1;
}

void initDotDot(){
}
