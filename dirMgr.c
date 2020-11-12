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

int initDir(int parentBlock, char* name){  //pass in block of whatever directory this is called from //OR in case of root directory init, pass in sentinel value of 0
    int retVal;
    // printf("Initializing directory named %s\n", name);

    int bytesNeeded = sizeof(dir);
    int blocksNeeded = (bytesNeeded/vcb->sizeOfBlocks) + 1;
    int dirStartBlock = findFreeBlocks(blocksNeeded); //find next available blocks
    
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dir)));
    dir* initDirD = malloc(toBlockSize(sizeof(dir)));
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* initDirDE = malloc(toBlockSize(sizeof(dirEnt)));
    //initDirD->sizeInBlocks = blocksNeeded;
    //initDirD->sizeInBytes = bytesNeeded;
    initDirD->loc = dirStartBlock;
    if(parentBlock == 0){  //initialize root directory
        initDirD->parentLoc = dirStartBlock;  //parent is itself
        //initialize root directory variables of vcb
        vcb->rdLoc = initDirD->parentLoc; 
        vcb->rdBlkCnt = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
        vcb->rdLoc = initDirD->loc;
        currentBlock = dirStartBlock;  //when root directory is initialized, set currentBlock to it, so future child directories can easily use it
        currentBlockSize = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
        initDirDE->parentLoc = dirStartBlock;
    }
    else{  //use parent block passed in
        initDirD->parentLoc = parentBlock; // parent is at block index passed in
        initDirDE->parentLoc = parentBlock;
    }
    strncpy(initDirD->name,name,strlen(name));
    initDirEntries(initDirD);  //initialize dirEnts
    setFreeBlocks(dirStartBlock,blocksNeeded); //modify free space bitmap to indicate blocks taken up by this directory
    
    //initialize directory entry
    strncpy(initDirDE->name,name,strlen(name));
    initDirDE->type = 1;
    int deStartBlock = findFreeBlocks(blocksNeeded);
    initDirDE->loc = deStartBlock;
    initDirDE->dataBlkCnt = toBlockSize(sizeof(dir))/vcb->sizeOfBlocks;
    initDirDE->dataIndex = dirStartBlock;

    //write directory entry of directory to disk
    retVal = LBAwrite(initDirDE,toBlockSize(sizeof(dirEnt))/vcb->sizeOfBlocks,deStartBlock);
    setFreeBlocks(deStartBlock,(sizeof(dirEnt)/vcb->sizeOfBlocks)+1);

    if(parentBlock != 0){  //if directory is not root
        retVal = LBAwrite(initDirD,currentBlockSize, dirStartBlock);  //write directory we are initializing now to disk
        retVal = LBAread(initDirD,currentBlockSize,parentBlock);      //read directory of parent 
        addDirEnt(initDirD,initDirDE);                                //add directory entry of directory initialized in this function to parent directory
        retVal = LBAwrite(initDirD,toBlockSize(sizeof(dir))/vcb->sizeOfBlocks, parentBlock);  //save directory of parent to disk
    } 
    else{  //else root directory entry will be written to root directory
        addDirEnt(initDirD,initDirDE);  //add root directory entry to root directory's directory entries 
        retVal = LBAwrite(initDirD,currentBlockSize, dirStartBlock);
    }
    
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    free(initDirDE);
    initDirDE = NULL;
    printf("Freeing: %d bytes\n", toBlockSize(sizeof(dir)));
    // initDirEntries(initDirD);
    free(initDirD);
    initDirD = NULL;

    return dirStartBlock;  //returns block of data of directory, not directory entry of directory
}

void initDirEntries(dir* initDED){ 
    int length = sizeof(initDED->dirEnts)/sizeof(initDED->dirEnts[0]);
    // printf("Length: %d\n",length);
    for(int i = 0; i < length; i++){
        initDED->dirEnts[i] = -1;
    }
}

int initFile(int parentBlock, char* name){ //takes in parent directory data block and name of new file
    int retVal;
    //this just creates directory entry in directory at parentBlock
    int deStartBlock = findFreeBlocks(currentBlockSize); //find next available blocks for our new directory entry

    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* initFileDE = malloc(toBlockSize(sizeof(dirEnt)));   //malloc memory for directory entry we want to initialize
    strncpy(initFileDE->name,name, strlen(name));
    initFileDE->parentLoc = parentBlock;
    initFileDE->type = 0;
    initFileDE->loc = deStartBlock;
    initFileDE->dataIndex = -1;      //right now its just an empty file with no data
    initFileDE->dataBlkCnt = -1;     //right now its just an empty file with no data

    retVal = LBAwrite(initFileDE,toBlockSize(sizeof(dirEnt))/vcb->sizeOfBlocks,deStartBlock);
    setFreeBlocks(deStartBlock,(sizeof(dirEnt)/vcb->sizeOfBlocks)+1);

    //add this directory entry to directory at parent block
    dir* initFileD = malloc(toBlockSize(sizeof(dir)));          // malloc memory for parent directory
    retVal = LBAread(initFileD,currentBlockSize,parentBlock);   // read the parent directory into variable

    addDirEnt(initFileD,initFileDE);   //add the new file's directory entry to directory passed in

    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dir)));
    retVal = LBAwrite(initFileD,currentBlockSize, parentBlock);

    int loc = initFileDE->loc;
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    free(initFileDE);
    initFileDE = NULL;
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dir)));
    free(initFileD);
    initFileD = NULL;
    return loc;  //return directory entry location, b_write will decide dataIndex and dataBlkCnt later
}

/*int mkFile(char *pathname, dir* d){ // makes an empty directory entry, maybe add "mode_t mode"
    // type = 0 for a file

    printf("dirName = %s\n", d->name);
    return 0;

    int retVal;
    char* pathnameCpy = strndup(pathname, strlen(pathname));

    // getting fileName
    char *fileName = pathnameCpy;
    if (strstr(pathname, "/") != NULL) fileName = strrchr(pathnameCpy, '/')+1; // if dir is included in path, separate for fileName
    printf("\n%s\n\n", fileName);

    if(pathname[0] == '/') {//if pathname is absolute
        printf("\n***absolute directory in mkFile: Need to implement***\n");
    }
    else{
        dir* parentDir = malloc(toBlockSize(sizeof(dir)));
        printf("Mallocing parentDir: %ld bytes\n", toBlockSize(sizeof(dir)));
        retVal = LBAread(parentDir,currentBlockSize,currentBlock);
//
//        //Malloc Directory entry

        dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
        printf("Mallocing de: %ld bytes\n", toBlockSize(sizeof(dirEnt)));

    //initialize directory entry
        de->parentLoc = currentBlock;

        //// Need to do: dataIndex

        //de->dataIndex = dirIndex; //store pointer to data
        de->dataBlkCnt = vcb->rdBlkCnt;
        de->sizeInBlocks = vcb->rdBlkCnt;//all directories should be same size
        de->type = 0; //type is file
        strcpy(de->name,fileName); // maybe Pathname?

        //write directory entry to disk
        int deIndex  = findFreeBlocks((sizeof(dirEnt)/vcb->sizeOfBlocks) +1);
        de->loc = deIndex;  //store location of directory entry
        retVal = LBAwrite(de,(sizeof(dirEnt)/vcb->sizeOfBlocks) +1,deIndex); //Is there any point to this?
        setFreeBlocks(deIndex,(sizeof(dirEnt)/vcb->sizeOfBlocks) +1);
        addDirEnt(parentDir,de);//write this  directory(only index to metadata) to parent dir's dirEnts

        printf("\naddDirEnt completed\nhash_table_lookup(fileName): %d\n\n", hash_table_lookup("fileName", d)); // checking if dirEnt was added

        free(parentDir);
        printf("Freeing parentDir: %ld bytes\n", toBlockSize(sizeof(dir)));
        parentDir = NULL;
        free(de);
        printf("Freeing de: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
        de = NULL;
    }


    printf("mkFile() Done\n\n");

//    }
//
//
//    //int freeLoc = findFreeDirEnt();
//
//    printf("freeLoc: %d ", freeLoc);
//
//    printf("heyyo\n");
    return 1;
}*/

int findDirEnt(char* pathname){  // will eventually be edited to take in LBA from caller that is starting directory
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

    printf("Mallocing: %ld Bytes\n", toBlockSize(sizeof(dir)));
    dir* findDirEntD = malloc(toBlockSize(sizeof(dir))); //allocate memory for dir // 720 is temporary
    printf("Mallocing: %ld Bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* findDirEntDE = malloc(toBlockSize(sizeof(dirEnt)));
   

    //=========================================================================================
    //ABSOLUTE PATH //we need to start from root
    //=========================================================================================
    if(pathname[0] == '/') {//if pathname is absolute
        retVal = LBAread(findDirEntD,vcb->rdBlkCnt,vcb->rdLoc);  //read root directory into our directory
    }
    //=========================================================================
    //Relative Path Name  //use currentBlock data to get starting directory
    //=========================================================================
    else{
        retVal = LBAread(findDirEntD,currentBlockSize,currentBlock); // read into our dir function
    }
    while((token = strtok_r(remainder, "/",&remainder))){ //continues while subdirectory exists
        if(abortFlag == true){ //we are trying to iterate into a file, not a directory
            return -1;  //return error
        }
        // printf("Token: %s\n", token);  //prints next directory
        deIndex = hash_table_lookup(token,findDirEntD);  //look up the name in directory entries of findDirEntD
        if(deIndex == -1)  //if directory entry is not found, either the directory doesn't exist
        { 
            if(pathname =""){ //if pathname is empty then we are on the last iteration //therefore the intention is to create a file 
                //create file by default for now
                if(counter == 0)
                    retVal = initFile(currentBlock,token);
                else
                    retVal = initFile(findDirEntDE->dataIndex,token);
                free(original);
                original = NULL;
                printf("Freeing: %ld Bytes\n", toBlockSize(sizeof(dirEnt)));
                free(findDirEntDE);
                findDirEntDE = NULL;
                printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
                free(findDirEntD);
                findDirEntD = NULL;
                return retVal;
            }
            else{  //we are trying to iterate into directory that does not exist
                free(original);
                original = NULL;
                printf("Freeing: %ld Bytes\n", toBlockSize(sizeof(dirEnt)));
                free(findDirEntDE);
                findDirEntDE = NULL;
                printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
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
    printf("Freeing: %ld Bytes\n", toBlockSize(sizeof(dirEnt)));
    free(findDirEntDE);
    findDirEntDE = NULL;
    printf("Freeing: %d Bytes\n", toBlockSize(sizeof(dir)));
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
        retVal = LBAwrite(parentDir,toBlockSize(sizeof(dir))/vcb->sizeOfBlocks,parentDir->loc);
    }
    else if(dE->type == 0)//else if type is file
    {
        success = hash_table_insert(dE,parentDir);  //need to only pass pointer to dirEnt
    }   
}
