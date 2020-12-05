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

extern unsigned int* freeSpaceBitmap;  //global for whole program
extern vCB* vcb;  //global for whole program
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

    dirEnt* dot = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* dotDot = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* parentDE = malloc(toBlockSize(sizeof(dirEnt)));

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks);
    int dirStartBlock = findFreeBlocks(blocksNeeded); //find next available blocks
    setFreeBlocks(dirStartBlock,blocksNeeded); //modify free space bitmap to indicate blocks taken up by this directory
    
    dir* initDirD = malloc(toBlockSize(sizeof(dir)));
    dirEnt* initDirDE = malloc(toBlockSize(sizeof(dirEnt)));
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
        dotDot->type = 1;
        dotDot->dataBlkCnt = vcb->dBlkCnt;
        dotDot->dataIndex = parentDE->loc;

        int dotDotStartBlock = findFreeBlocks(vcb->deBlkCnt);
        setFreeBlocks(dotDotStartBlock,vcb->deBlkCnt);
        dotDot->loc = dotDotStartBlock;

        //write dot directory entry of directory to disk
        retVal = LBAwrite(dotDot,vcb->deBlkCnt, dotDotStartBlock);
        
        initDirD->parentLoc = parentDE->loc; // parent is at block index passed in
        initDirDE->parentLoc = parentDE->loc;

    }
    
    initDirEntries(initDirD);  //initialize dirEnts
    
    
    
    //=========================================
    //Initialize directory entry
    //=========================================
    strncpy(initDirDE->name,name,strlen(name));
    initDirDE->name[strlen(name)] = '\0';
    
    initDirDE->type = 1;
    initDirDE->dataBlkCnt = vcb->dBlkCnt;
    initDirDE->dataIndex = dirStartBlock;
    initDirDE->dataByteCnt = toBlockSize(sizeof(dirEnt));
    // initialize extents of directory entry
    initDirDE->ext1.count = 0;
    initDirDE->ext2.count = 0;
    initDirDE->ext3.count = 0;
    initDirDE->ext4.count = 0;
    initExts(initDirDE, MAX_SEC_EXTENTS);
    int deStartBlock = findFreeBlocks(blocksNeeded);
    setFreeBlocks(deStartBlock,vcb->deBlkCnt);
    initDirDE->loc = deStartBlock;

    if(parentDEBlock == 0){
        currentBlock = deStartBlock;  //when root directory is initialized, set currentBlock to it, so future child directories can easily use it
        initDirDE->parentLoc = deStartBlock;
    }

    //write directory entry of directory to disk
    retVal = LBAwrite(initDirDE,vcb->deBlkCnt,deStartBlock);
    

    //=======================================
    //initialize "dot" directory entry
    //=======================================
    strncpy(dot->name,".",2);
    dot->type = initDirDE->type;
    dot->dataBlkCnt = initDirDE->dataBlkCnt;
    dot->dataIndex  = initDirDE->loc;

    int dotStartBlock = findFreeBlocks(vcb->deBlkCnt);
    setFreeBlocks(dotStartBlock,vcb->deBlkCnt);
    dot->loc = dotStartBlock;

    //write dot directory entry of directory to disk
    retVal = LBAwrite(dot,vcb->deBlkCnt, dotStartBlock);
    

    if(parentDEBlock == 0){  //if directory is root
        strncpy(dotDot->name,"..", 3);
        dotDot->type = 1;
        dotDot->dataBlkCnt = vcb->dBlkCnt;
        dotDot->dataIndex  = initDirDE->loc;
        dotDot->dataByteCnt = toBlockSize(sizeof(dirEnt));
        int dotdotStartBlock = findFreeBlocks(vcb->deBlkCnt);
        setFreeBlocks(dotdotStartBlock,vcb->deBlkCnt);
        dotDot->loc = dotdotStartBlock;

        //write dot directory entry of directory to disk
        retVal = LBAwrite(dot,vcb->deBlkCnt, dotStartBlock);
        retVal = LBAwrite(dotDot, vcb->deBlkCnt, dotdotStartBlock);
        vcb->rdLoc = initDirDE->loc;

        //addDirEnt(initDirD,initDirDE);                                //add root directory entry to root directory's directory entries
        addDirEnt(initDirD,dot);                                      //add dot directory entry to root directory's directory entries
        addDirEnt(initDirD,dotDot);                                   //add dot dot directory entry to root directory's directory entries
    } 
    else{
        addDirEnt(initDirD,dot);                                      //add dot directory entry
        addDirEnt(initDirD,dotDot);
        //read directory of parent
        retVal = LBAread(initDirD,vcb->dBlkCnt,parentDE->dataIndex);      
        addDirEnt(initDirD,initDirDE);                                //add directory entry of directory initialized in this function to parent directory
    }

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
    setFreeBlocks(deStartBlock,vcb->deBlkCnt);

    dirEnt* initFileDE = malloc(toBlockSize(sizeof(dirEnt)));   //malloc memory for directory entry we want to initialize
    strncpy(initFileDE->name,name, strlen(name));
    initFileDE->name[strlen(name)] ='\0';
    initFileDE->parentLoc = parentDEBlock;
    initFileDE->type = 0;
    initFileDE->loc = deStartBlock;
    initFileDE->dataIndex = -1;      //right now its just an empty file with no data
    initFileDE->dataBlkCnt = 0;      //right now its just an empty file with no data
    initFileDE->dataByteCnt = 0;     //right now its just an empty file with no data

    // extents, init count to 0
    initFileDE->ext1.count = 0;
    initFileDE->ext2.count = 0;
    initFileDE->ext3.count = 0;
    initFileDE->ext4.count = 0;
    initExts(initFileDE, MAX_SEC_EXTENTS);

    retVal = LBAwrite(initFileDE,vcb->deBlkCnt,deStartBlock);
    
    //add this directory entry to directory at parent block
    dirEnt* parentDE = malloc(toBlockSize(sizeof(dirEnt)));          // malloc memory for parent directory entry
    retVal = LBAread(parentDE,vcb->deBlkCnt,parentDEBlock);   // read the parent directory entry into variable

    dir* parentD = malloc(toBlockSize(sizeof(dir)));
    retVal = LBAread(parentD,vcb->dBlkCnt,parentDE->dataIndex);   // read the parent directory entry into variable

    addDirEnt(parentD,initFileDE);   //add the new file's directory entry to directory passed in

    int loc = initFileDE->loc;
    free(initFileDE);
    initFileDE = NULL;
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

    dir* findDirEntD = malloc(toBlockSize(sizeof(dir))); //allocate memory for dir // 720 is temporary
    dirEnt* findDirEntDE = malloc(toBlockSize(sizeof(dirEnt)));
   
    //=========================================================================================
    //ABSOLUTE PATH //we need to start from root
    //=========================================================================================
    if(pathname[0] == '/') {//if pathname is absolute
        deIndex = vcb->rdLoc; //set deIndex to rd deIndex in case user wants to change directory to root
        dirEnt* currentBlockDE = malloc(toBlockSize(sizeof(dirEnt)));
        retVal = LBAread(currentBlockDE,vcb->deBlkCnt,vcb->rdLoc);
        retVal = LBAread(findDirEntD,vcb->dBlkCnt,currentBlockDE->dataIndex);  //read root directory into our starting directory
        deIndex =  vcb->rdLoc;
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
        deIndex =  currentBlock;
        free(currentBlockDE);
        currentBlockDE = NULL;
    }
    while((token = strtok_r(remainder, "/",&remainder))){ //continues while subdirectory exists
        if(abortFlag == true){ //we are trying to iterate into a file, not a directory
            free(original);
            original = NULL;
            free(findDirEntDE);
            findDirEntDE = NULL;
            free(findDirEntD);
            findDirEntD = NULL;
            return -1;  //return error
        }
        // printf("Token: %s\n", token);  //prints next directory
        deIndex = hash_table_lookup(token,findDirEntD);  //look up the name in directory entries of findDirEntD
        if(strncmp(token, ".", 2) == 0 || strncmp(token, "..", 3) == 0)  //if the token is dot or dot dot
        {
            retVal = LBAread(findDirEntDE,vcb->deBlkCnt, deIndex);  //read directory entry (NOT FILE ITSELF) into dirEnt
            deIndex = findDirEntDE->dataIndex;  //dataIndex holds the deIndex of the de that dot or dot dot points to
        }
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
              else{
                  retVal = -1;
              }
                free(original);
                original = NULL;
                free(findDirEntDE); //if pathname is e
                findDirEntDE = NULL;
                free(findDirEntD);
                findDirEntD = NULL;
                return retVal;
            }
            else{  //we are trying to iterate into directory that does not exist
                free(original);
                original = NULL;
                free(findDirEntDE);
                findDirEntDE = NULL;
                free(findDirEntD);
                findDirEntD = NULL;
                return -1; //return error because directory doesn't exist
            }
        }
        retVal = LBAread(findDirEntDE,vcb->deBlkCnt, deIndex);  //read directory entry (NOT FILE ITSELF) into dirEnt
        if(findDirEntDE->type == 1) //if directory entry is a directory
        {
            counter++;
            retVal = LBAread(findDirEntD, vcb->dBlkCnt,findDirEntDE->dataIndex);//read new starting directory into d

        }
        else if(findDirEntDE->type ==0){  //we are at a file
            abortFlag = true;  //if the loop continues after this, then loop will abort with error
                               //else, it will just continue after loop and return dataIndex //required to work with b_open
        }
    }
    free(original);
    original = NULL;
    free(findDirEntDE);
    findDirEntDE = NULL;
    free(findDirEntD);
    findDirEntD = NULL;

    return deIndex; //returns logical block index of directory entry pointed to by path  //if we keep it like this, we could reuse this code for cd and b_open potentially
                    //Explanation: if loop reaches end and directory is valid and occupies basename, then just return directory entry associated with directory, (CD)
                    //OR if file is present at that directory entry then it will have returned a directory entry associated with a file (b_open and potentially other functions)
}

int addDirEnt(dir* parentDir, dirEnt* dE){
    bool success = hash_table_insert(parentDir, dE);  //need to only pass pointer to dirEnt
    if(success == false){
        printf("ERROR: Failed to add file, parent directory is full!\n");
        return -1;
    }
    else{
        int retVal = LBAwrite(parentDir,vcb->dBlkCnt,parentDir->loc);
        if(retVal != vcb->dBlkCnt){
            return -1;
        }
    }
    return 0; 
}

bool removeDirEnt(dir* parentDir, dirEnt* dE){
    bool success = hash_table_delete(parentDir, dE); 
    if(success == false){
        printf("ERROR: Failed to delete Directory Entry\n");
        return -1;
    }
    else{
        //need to free directory entry's space
        clearFreeBlocks(dE->loc,vcb->deBlkCnt);
        int retVal = LBAwrite(parentDir,vcb->dBlkCnt,parentDir->loc);
        if(retVal != vcb->dBlkCnt){
            return false;
        }
    }
    return true;
}

int findNextDirEnt(int directoryIndex, int startingDirectoryEntryIndex){
    int retVal;
    dir* findNextDirEntD = malloc(toBlockSize(sizeof(dir)));
    retVal = LBAread(findNextDirEntD, vcb->dBlkCnt, directoryIndex);

    for(int i = startingDirectoryEntryIndex; i< (sizeof(findNextDirEntD->dirEnts)/(sizeof(findNextDirEntD->dirEnts[0]))); i++)
    {
        
        if(findNextDirEntD->dirEnts[i] != -1)
        {
            free(findNextDirEntD);
            findNextDirEntD = NULL;
            return i;
        }
    }
    free(findNextDirEntD);
    findNextDirEntD = NULL;
    return 0;  //no more occupied dirEnts
}
