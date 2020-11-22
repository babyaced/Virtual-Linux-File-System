#include <stdlib.h>
#include <string.h>

#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"
#include "fsLow.h"
#include "freeMgr.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock;
extern int currentBlockSize;
extern char currentBlockName[255]; //size of 255 for now



int fs_mkdir(const char *pathname, mode_t mode){ //ignore mode for now
    int retVal;
    char* pathnameCpy = strndup(pathname, strlen(pathname));
    int dirIndex;
    int parentDirEntIndex;   

    //=====================================================================================================================================
    //ABSOLUTE PATH AND RELATIVE PATH //just directly use current block data to make directory
    //=====================================================================================================================================
    
    parentDirEntIndex = findDirEnt(pathnameCpy,2);//returns directory entry index of direct parent of the directory we want to create;
    if(parentDirEntIndex == -1)
        return -1;
    free(pathnameCpy);
    pathnameCpy = NULL;
    return 0;
}

int fs_rmdir(const char *pathname){
    int retVal;
    printf("Path Name: %s\n", pathname);
    printf("fs_rmdir\n");
    char* pathnameCpy = strndup(pathname, strlen(pathname));
    //findDirEnt()  //find location of directory at pathname
    int dirIndex = findDirEnt(pathname, 0);                          //good
    int parentDirEntIndex = findDirEnt(pathnameCpy, 0);              //don't need to do another find for parent
    
    dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
    printf("Mallocing: %ld bytes\n", sizeof(dirEnt));
    dir* parentDir = malloc(toBlockSize(sizeof(dir)));
    printf("Mallocing: %ld bytes\n", sizeof(dir));

    retVal = LBAread(de, vcb->deBlkCnt, dirIndex);                      //good
    
    int blockLength = de->dataBlkCnt;                                         
    int index = de->dataIndex;
    
    //de->dataIndex = dirIndex;
    if(!pathname) {                                                
        printf("%s does not exist.\n", pathname);
        return -1; //return -1 if error
    }

    if (parentDirEntIndex == -1) {
        printf("%s does not exist.\n", pathnameCpy);
        return -1;
    }
    //need to iterate through dirEnts[] of dir and setFreeBlocks for each //you can use a for loop and use i < sizeof(d->dirEnts)/(sizeof(d->dirEnts[0])
    if(de->dataBlkCnt > 0) {                                            //check if the int value in the directory entry array is -1
                                                                        //read into a directory entry
        //clear directory entry //reset to zero values
        //parentDir->sizeOfBlocks                                      //check the type of the directory entry you just read
        clearFreeBlocks(index, blockLength); //if dirEnt->type == 0(its a file)
                                             //you can call fs_delete that jay is working on, but you'll need to find a way to pass in the updated path
                                             //OR clear the blocks here, and set the int value in the directory entry array is -1
                                             ////if dirEnt->type == 1(its a directory)
                                             // need to recursively delete the entries in the directory //you probably want to have a helper function, so you don't
                                             //have to keep track of and pass in the path each time
    }     
    free(parentDir);
    free(de);
    parentDir = NULL;
    de = NULL;
    return 0; //not sure what return value is supposed to represent yet
              //retVal
}

fdDir * fs_opendir(const char *name){
    int retVal;
    int dirEntIndex = findDirEnt(name,0);  //find location of directory at pathname

    dirEnt* fs_opendirDE = malloc(toBlockSize(sizeof(dirEnt)));
    // printf("Malloced: %d bytes\n", toBlockSize(sizeof(dirEnt)));

    retVal = LBAread(fs_opendirDE, vcb->deBlkCnt, dirEntIndex);

    fdDir* fs_open_fddir = malloc(toBlockSize(sizeof(fdDir)));
    if(fs_open_fddir->isInitialized != 1)
    {
        // printf("Malloced: %d bytes\n", toBlockSize(sizeof(fdDir)));
        if(fs_opendirDE->type == 1)
        {
            fs_open_fddir->dirpItemInfo = malloc(sizeof(struct fs_diriteminfo));
            fs_open_fddir->dirEntryPosition = 0;
            fs_open_fddir->directoryStartLocation = fs_opendirDE->dataIndex;
            fs_open_fddir->d_reclen = sizeof(fs_open_fddir);                        //what does this mean?
            fs_open_fddir->isInitialized = 1;                                       //set initialized to true
        }
    }

    free(fs_opendirDE);
    // printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));

    return fs_open_fddir;
}
 
struct fs_diriteminfo* fs_readdir(fdDir *dirp){ //every time I call read it will return the next diritem info //returns name
    int retVal;
    int dirEntPos;

    dirEntPos = findNextDirEnt(dirp->directoryStartLocation,dirp->dirEntryPosition);

    if(dirEntPos < dirp->dirEntryPosition){
        return NULL;
    }
    dirp->dirEntryPosition = dirEntPos;

    dirEnt* fs_readdirDE = malloc(toBlockSize(sizeof(dirEnt)));
    // printf("Malloced: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dir* fs_readdirD = malloc(toBlockSize(sizeof(dir)));
    // printf("Malloced: %d bytes\n", toBlockSize(sizeof(dir)));

    retVal = LBAread(fs_readdirD, vcb->dBlkCnt, dirp->directoryStartLocation);
    retVal = LBAread(fs_readdirDE, vcb->deBlkCnt, fs_readdirD->dirEnts[dirEntPos]);
    
    dirp->dirpItemInfo->d_reclen = sizeof(fdDir);
    dirp->dirpItemInfo->fileType = fs_readdirDE->type;
    strncpy(dirp->dirpItemInfo->d_name,fs_readdirDE->name, strlen(fs_readdirDE->name));

    free(fs_readdirDE);
    // printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_readdirD);
    // printf("Freed: %d bytes\n", toBlockSize(sizeof(dir)));

    dirp->dirEntryPosition++;
    return dirp->dirpItemInfo;
}

int fs_closedir(fdDir *dirp){
    free(dirp->dirpItemInfo);
    dirp->dirpItemInfo = NULL;
    free(dirp);
    dirp = NULL;
    return 0;
}

char * fs_getcwd(char *buf, size_t size){
    int retVal;
    //get and append all directory names to buf
    dirEnt* fs_getcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    retVal = LBAread(fs_getcwdDE,vcb->deBlkCnt,currentBlock);
    if(strncmp(fs_getcwdDE->name,"",2) == 0){ //if cwd is root
        strcatF(buf,fs_getcwdDE->name);
        strcatF(buf,"/");
    }
    else  //if cwd is not root
    {
        while(fs_getcwdDE->parentLoc != fs_getcwdDE->loc){
            strcatF(buf, fs_getcwdDE->name);
            strcatF(buf,"/");
            // printf("Path accumulator: %s\n", buf);
            retVal = LBAread(fs_getcwdDE,currentBlockSize,fs_getcwdDE->parentLoc);
        }
    }

    buf[strlen(buf)] = '\0';

    free(fs_getcwdDE);
    fs_getcwdDE = NULL;
    return buf;
}

//helper for getcwd
void strcatF(char* dest, char* src){  //concatenates src to front of dest
    size_t dest_len = strlen(dest) + 1, src_len = strlen(src);
    memmove(dest + src_len, dest, dest_len);
    memcpy(dest, src, src_len);
}

int fs_setcwd(char *buf){ //linux chdir  //cd
    //=====================================================================================
    //ABSOLUTE PATH and RELATIVE PATH cases
    //=====================================================================================
    int retVal;
    int dirEntIndex = findDirEnt(buf,0);
    if(dirEntIndex == -1){
        return -1;
    }
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_setcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    //buf is path the user wants to change to

    retVal = LBAread(fs_setcwdDE, currentBlockSize, dirEntIndex);
    currentBlock = fs_setcwdDE->loc;                          //set currentBlock to desired directory(NOT DIRECTORY ENTRY)
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_setcwdDE);
    fs_setcwdDE = NULL;
    return 0; //returns 0 with success
}

int fs_isFile(char * path){ //return 1 if file, 0 otherwise
    int deIndex = findDirEnt(path,0); //get index of directory entry pointed to by path
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_isFileDE = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(fs_isFileDE,vcb->deBlkCnt, deIndex); //read directory entry into fs_isFileDE

    if(fs_isFileDE->type == 0)
    {
        // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
        free(fs_isFileDE);
        fs_isFileDE=NULL;
        return 1;
    }
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_isFileDE);
    fs_isFileDE=NULL;
        
    return 0;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    int deIndex = findDirEnt(path,0); //get index of directory entry pointed to by path
    dirEnt* fs_isDirDE = malloc(toBlockSize(sizeof(dirEnt)));
    // printf("Malloced: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(fs_isDirDE,vcb->deBlkCnt, deIndex); //read directory entry into fs_isDirDE

    if(fs_isDirDE->type == 1)
    {
        free(fs_isDirDE);
        // printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
        fs_isDirDE=NULL;
        return 1;
    }
    free(fs_isDirDE);
    // printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    fs_isDirDE=NULL;
    return 0;
}		

int fs_stat(const char *path, struct fs_stat *buf){
    int dirEntIndex = findDirEnt(path,0);

    dirEnt* fs_statDE = malloc(toBlockSize(sizeof(dirEnt)));
    // printf("Malloced: %d bytes\n", toBlockSize(sizeof(dirEnt)));

    int retVal = LBAread(fs_statDE,vcb->sizeOfBlocks, dirEntIndex);
    buf->st_blksize = vcb->sizeOfBlocks;
    buf->st_size =  fs_statDE->dataSize;
    buf->st_blocks = fs_statDE->dataBlkCnt;

    free(fs_statDE);
    // printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    return 0;
}

int fs_delete(char* filename){ //removes a file
    //findDirEnt()  //find location of file at pathname

    int deStartBlock = findFreeBlocks(currentBlockSize);

    int deIndex = findDirEnt(filename,0);

    dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(de, vcb->deBlkCnt, deIndex); //read directory entry into fs_isFileDE

    int lbaLoc = de->dataIndex;
    int blockLength = de->dataBlkCnt;
    printf("\nfs_delete()\nFile Loc = %d\n", de->loc);
    printf("Length = %d\n", de->dataBlkCnt);

    if(de->dataBlkCnt > 0){
        clearFreeBlocks(lbaLoc, blockLength); // freeing file's blocks in the freespace bitmap
    }
        

    /// Need to remove the dirEnt
    //removeDirEnt(deIndex)  //will check if directory entry points to file or directory and take appropriate action

    printf("findFreeBlocks(were file was) = %d\n\n", findFreeBlocks(de->dataBlkCnt)); // prints blocks that were just freed (de->loc)

    free(de);
    //printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    deIndex = findDirEnt(filename,0);

    printf("\nINDEX = %d\n", deIndex);
    printf("deStartBlock = %d\n\n", deStartBlock);

    return 0;
}
