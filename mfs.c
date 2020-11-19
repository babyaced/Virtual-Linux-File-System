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

struct fs_diriteminfo* dirpItemInfo;

int fs_mkdir(const char *pathname, mode_t mode){ //ignore mode for now
    int retVal;
    char* pathnameCpy = strndup(pathname, strlen(pathname));
    // char* dirToCreate;
    // char* lastDir;
    int dirIndex;
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dir)));
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    // dirEnt* fs_mkdirDE = malloc(toBlockSize(sizeof(dirEnt)));
    int parentDirEntIndex;
    
    /*if (strstr(pathnameCpy, "/") != NULL) {  //
        lastDir = strrchr(pathnameCpy, '/') + 1;
        dirToCreate = strndup(lastDir,strlen(lastDir));
        pathnameCpy[strlen(pathnameCpy)-strlen(dirToCreate)] = '\0';//this will modify lastDir
    }
    else{  //if pathname is relative without ./
        dirToCreate = strndup(pathnameCpy, strlen(pathnameCpy));
    }*/

   

    //=====================================================================================================================================
    //ABSOLUTE PATH AND RELATIVE PATH //just directly use current block data to make directory
    //=====================================================================================================================================
    
    parentDirEntIndex = findDirEnt(pathnameCpy,2);//returns directory entry index of direct parent of the directory we want to create;
    free(pathnameCpy);
    pathnameCpy = NULL;
    // retVal = LBAread(fs_mkdirDE, currentBlockSize, parentDirEntIndex);
    // dirIndex = initDir(fs_mkdirDE->dataIndex,dirToCreate);//parent block is wherever this function is called from //just use pathname for testing
    // free(dirToCreate);
    // dirToCreate = NULL;
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    // free(fs_mkdirDE);
    // fs_mkdirDE = NULL;
    return 0;
}

int fs_rmdir(const char *pathname){
    int retVal;
    printf("Path Name: %s\n", pathname);

    //findDirEnt()  //find location of directory at pathname
    //need to iterate through dirEnts[] of dir and setFreeBlocks for each
    //clear directory entry //reset to zero values
    //setFreeBlocks(vcb,fsl, dir->loc, dir->sizeInBlocks)
    return 0; //not sure what return value is supposed to represent yet
}

fdDir * fs_opendir(const char *name){
    int retVal;
    int dirEntIndex = findDirEnt(name,0);  //find location of directory at pathname

    dirEnt* fs_opendirDE = malloc(toBlockSize(sizeof(dirEnt)));
    printf("Malloced: %d bytes\n", toBlockSize(sizeof(dirEnt)));

    retVal = LBAread(fs_opendirDE, vcb->deBlkCnt, dirEntIndex);

    fdDir* fs_open_fddir = malloc(toBlockSize(sizeof(fdDir)));
    if(fs_open_fddir->isInitialized != 1)
    {
        printf("Malloced: %d bytes\n", toBlockSize(sizeof(fdDir)));
        if(fs_opendirDE->type == 1)
        {
            fs_open_fddir->dirEntryPosition = 0;
            fs_open_fddir->directoryStartLocation = fs_opendirDE->dataIndex;
            fs_open_fddir->d_reclen = sizeof(fs_open_fddir);                        //what does this mean?
            fs_open_fddir->isInitialized = 1;                                       //set initialized to true
        }
    }

    free(fs_opendirDE);
    printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));

    return fs_open_fddir;
}
 
struct fs_diriteminfo* fs_readdir(fdDir *dirp){ //every time I call read it will return the next diritem info //returns name
    int retVal;
    dirpItemInfo = malloc(sizeof(struct fs_diriteminfo));

    dirp->dirEntryPosition = findNextDirEnt(dirp->directoryStartLocation,dirp->dirEntryPosition);

    dirEnt* fs_readdirDE = malloc(toBlockSize(sizeof(dirEnt)));
    printf("Malloced: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dir* fs_readdirD = malloc(toBlockSize(sizeof(dir)));
    printf("Malloced: %d bytes\n", toBlockSize(sizeof(dir)));

    retVal = LBAread(fs_readdirD, vcb->dBlkCnt, dirp->directoryStartLocation);
    retVal = LBAread(fs_readdirDE, vcb->deBlkCnt, fs_readdirD->dirEnts[dirp->dirEntryPosition]);

    free(fs_readdirDE);
    printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_readdirD);
    printf("Freed: %d bytes\n", toBlockSize(sizeof(dir)));
    
    dirpItemInfo->d_reclen = sizeof(fdDir);
    dirpItemInfo->fileType = fs_readdirDE->type;
    strcpy(dirpItemInfo->d_name,fs_readdirDE->name);

    return dirpItemInfo;
}

int fs_closedir(fdDir *dirp){
    free(dirp);
    dirp = NULL;
    free(dirpItemInfo);
    dirpItemInfo = NULL;
    return 0;
}

char * fs_getcwd(char *buf, size_t size){
    int retVal;
    // char* ptr;
    //get and append all directory names to buf
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_getcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    retVal = LBAread(fs_getcwdDE,currentBlockSize,currentBlock);
    while(fs_getcwdDE->parentLoc != fs_getcwdDE->loc){
        strcatF(buf, fs_getcwdDE->name);
        strcatF(buf,"/");
        // printf("Path accumulator: %s\n", buf);
        retVal = LBAread(fs_getcwdDE,currentBlockSize,fs_getcwdDE->parentLoc);
    }
    strcatF(buf,fs_getcwdDE->name);
    strcatF(buf,"/");
    buf[strlen(buf)] = '\0';
    // printf("Path accumulator: %s\n", buf);
    
    // printf("Freeing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
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
    // printf("Mallocing: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_setcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    //buf is path the user wants to change to

    retVal = LBAread(fs_setcwdDE, currentBlockSize, dirEntIndex);
    currentBlock = fs_setcwdDE->dataIndex;                          //set currentBlock to desired directory(NOT DIRECTORY ENTRY)
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

    int lbaLoc = de->loc;
    int blockLength = de->dataBlkCnt;
    printf("\nfs_delete()\nFile Loc = %d\n", de->loc);
    printf("Length = %d\n", de->dataBlkCnt);

    clearFreeBlocks(lbaLoc, blockLength); // freeing file's blocks in the freespace bitmap

    /// Need to remove the dirEnt

    printf("findFreeBlocks(were file was) = %d\n\n", findFreeBlocks(de->dataBlkCnt)); // prints blocks that were just freed (de->loc)

    free(de);
    //printf("Freed: %d bytes\n", toBlockSize(sizeof(dirEnt)));
    deIndex = findDirEnt(filename,0);

    printf("\nINDEX = %d\n", deIndex);
    printf("deStartBlock = %d\n\n", deStartBlock);

    return 0;
}
