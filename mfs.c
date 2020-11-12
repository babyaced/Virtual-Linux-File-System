#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock;
extern int currentBlockSize;
extern currentBlockName[255]; //size of 255 for now

int fs_mkdir(const char *pathname, mode_t mode){ //ignore mode for now
    int retVal;
    char* pathnameCpy = strndup(pathname, strlen(pathname));
    int dirIndex;
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dir)));
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_mkdirDE = malloc(toBlockSize(sizeof(dirEnt)));
    int parentDirEntIndex;

    char* lastDir = strrchr(pathnameCpy, '/') + 1; 
    char* dirToCreate = strndup(lastDir,strlen(lastDir));  //need to save the last directory name and remove leading '/'
    // dirToCreate = dirToCreate + 1;
    // printf("StrrChr: %s\n", dirToCreate);

    //=====================================================================================
    //ABSOLUTE PATH //we need to go down through directory tree until we can make directory
    //=====================================================================================
    if(pathname[0] == '/') //if pathname is absolute
    {
        pathnameCpy[strlen(pathnameCpy)-strlen(dirToCreate)] = '\0'; //this will modify lastDir
        parentDirEntIndex = findDirEnt(pathnameCpy);   //returns directory entry index of direct parent of the directory we want to create;
        free(pathnameCpy);
        pathnameCpy = NULL;
        retVal = LBAread(fs_mkdirDE, currentBlockSize, parentDirEntIndex);
        dirIndex = initDir(fs_mkdirDE->dataIndex, dirToCreate);
        free(dirToCreate);
        dirToCreate = NULL;
        printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
        free(fs_mkdirDE);
        fs_mkdirDE = NULL;
        return 0;
    }

    //=====================================================================================================================================
    //RELATIVE PATH //just directly use current block data to make directory
    //=====================================================================================================================================
    else{ //if pathname is not absolute
        pathnameCpy[strlen(pathnameCpy)-strlen(dirToCreate)] = '\0';
        parentDirEntIndex = findDirEnt(pathnameCpy);
        free(pathnameCpy);
        pathnameCpy = NULL;
        retVal = LBAread(fs_mkdirDE, currentBlockSize, parentDirEntIndex);
        dirIndex = initDir(fs_mkdirDE->dataIndex,dirToCreate);  //parent block is wherever this function is called from //just use pathname for testing
        free(dirToCreate);
        dirToCreate = NULL;
        printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
        free(fs_mkdirDE);
        fs_mkdirDE = NULL;
        return 0;
    }
    
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
     //findDirEnt()  //find location of directory at pathname
    //need to iterate through dirEnts[] of dir and return name for each
    return 0;
}
 
struct fs_diriteminfo *fs_readdir(fdDir *dirp){ //every time I call read it will return the next diritem info //returns name
    return 0;
}

int fs_closedir(fdDir *dirp){
    return 0;
}

char * fs_getcwd(char *buf, size_t size){
    int retVal;
    // char* ptr;
    //get and append all directory names to buf
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_getcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    retVal = LBAread(fs_getcwdDE,currentBlockSize,currentBlock);
    while(fs_getcwdDE->parentLoc != fs_getcwdDE->loc){
        strcatF(buf, fs_getcwdDE->name);
        strcatF(buf,"/");
        printf("Path accumulator: %s\n", buf);
        retVal = LBAread(fs_getcwdDE,currentBlockSize,fs_getcwdDE->parentLoc);
    }
    strcatF(buf,fs_getcwdDE->name);
    strcatF(buf,"/");
    // ptr = strchr(buf,'\377');  // remove octal escape sequence
    // buf[strlen(buf)-strlen(ptr)] = '\0';
    buf[strlen(buf)] = '\0';
    printf("Path accumulator: %s\n", buf);
    
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
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
    int dirEntIndex = findDirEnt(buf);
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_setcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    //buf is path the user wants to change to

    retVal = LBAread(fs_setcwdDE, currentBlockSize, dirEntIndex);
    currentBlock = fs_setcwdDE->dataIndex;                          //set currentBlock to desired directory(NOT DIRECTORY ENTRY)
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_setcwdDE);
    fs_setcwdDE = NULL;
    return 0; //returns 0 with success
}

int fs_isFile(char * path){ //return 1 if file, 0 otherwise
    int deIndex = findDirEnt(path); //get index of directory entry pointed to by path
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_isFileDE = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(fs_isFileDE, (sizeof(dirEnt)/vcb->sizeOfBlocks)+1, deIndex); //read directory entry into fs_isFileDE

    if(fs_isFileDE->type == 0)
    {
        printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
        free(fs_isFileDE);
        fs_isFileDE=NULL;
        return 1;
    }
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_isFileDE);
    fs_isFileDE=NULL;
        
    return 0;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    int deIndex = findDirEnt(path); //get index of directory entry pointed to by path
    printf("Mallocing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_isDirDE = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(fs_isDirDE,(sizeof(dirEnt)/vcb->sizeOfBlocks)+1, deIndex); //read directory entry into fs_isDirDE

    if(fs_isDirDE->type == 1)
    {
        printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
        free(fs_isDirDE);
        fs_isDirDE=NULL;
        return 1;
    }
    printf("Freeing: %ld bytes\n", toBlockSize(sizeof(dirEnt)));
    free(fs_isDirDE);
    fs_isDirDE=NULL;
    return 0;
}		

int fs_stat(const char *path, struct fs_stat *buf){
    return 0;
}

int fs_delete(char* filename){ //removes a file
    //findDirEnt()  //find location of file at pathname

    int deStartBlock = findFreeBlocks(currentBlockSize);

    int deIndex = findDirEnt(filename);
    if (deIndex >= deStartBlock) { // if file was just created from findDirEnt
        /// first delete that dirEnt
        printf("\nError: File \"%s\" does not exist.\n\n", filename);
        return (-1);
    }

    dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(de, (sizeof(dirEnt)/vcb->sizeOfBlocks)+1, deIndex); //read directory entry into fs_isFileDE

    int lbaLoc = de->loc;
    int blockLength = de->dataBlkCnt;
    printf("\nfs_delete()\nFile Loc = %d\n", de->loc);
    printf("Length = %d\n", de->dataBlkCnt);

    clearFreeBlocks(lbaLoc, blockLength); // freeing file's blocks in the freespace bitmap

    /// Need to remove the dirEnt

    printf("findFreeBlocks(were file was) = %d\n\n", findFreeBlocks(de->dataBlkCnt)); // prints blocks that were just freed (de->loc)
    //deIndex = findDirEnt(filename);

    //printf("\nINDEX = %d\n", deIndex);
    //printf("deStartBlock = %d\n\n", deStartBlock);

    return 0;
}
