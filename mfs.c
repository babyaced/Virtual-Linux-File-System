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
    char* pathnameCpy = strdup(pathname);

    //=====================================================================================
    //ABSOLUTE PATH //we need to go down through directory tree until we can make directory
    //=====================================================================================
    if(pathname[0] == '/') //if pathname is absolute
    {
        
        char* lastDir = strrchr(pathnameCpy, '/');
        printf("StrrChr: %s\n", lastDir);
        pathnameCpy[strlen(pathnameCpy)-strlen(lastDir)] = '\0';
        printf("Truncated Path: %s\n", pathnameCpy);



        int parentDirIndex = findDirEnt(pathnameCpy);  //returns root of the directory  
                                                    //need to modify logic so it doesn't return absolute last directory(which would be the one we want to create)
                                                    //we can cut off the pathname, before the last directory name is reached (EASIER)
                                                    //and initDir() here
                                                    //OR have findDirEnt() use initDir() 
    }

    //=====================================================================================================================================
    //RELATIVE PATH //just directly use current block data to make directory
    //=====================================================================================================================================
    else{ //if pathname is not absolute
    int dirIndex = initDir(currentBlock,pathname);  //parent block is wherever this function is called from //just use pathname for testing
    dir* parentDir = malloc(sizeof(dir));
    printf("Mallocing: %ld bytes\n", sizeof(dir));
    retVal = LBAread(parentDir,currentBlockSize,currentBlock);

    //int parentIndex = findDirEnt(dirname(pathname));//use parent in pathname to find parent block
    
    //Malloc Directory entry
    dirEnt* de = malloc(sizeof(dirEnt));
    printf("Mallocing: %ld bytes\n", sizeof(dirEnt));

    //initialize directory entry
    de->parentLoc = currentBlock;
    
    de->dataIndex = dirIndex; //store pointer to data
    de->dataBlkCnt = vcb->rdBlkCnt;
    de->sizeInBlocks = vcb->rdBlkCnt;//all directories should be same size 
    de->type = 1; //type is directory
    strcpy(de->name,pathname);

    //write directory entry to disk
    int deIndex  = findFreeBlocks((sizeof(dirEnt)/vcb->sizeOfBlocks) +1);
    de->loc = deIndex;  //store location of directory entry
    retVal = LBAwrite(de,(sizeof(dirEnt)/vcb->sizeOfBlocks) +1,deIndex); //Is there any point to this?
    setFreeBlocks(deIndex,(sizeof(dirEnt)/vcb->sizeOfBlocks) +1);
    addDirEnt(parentDir,de);//write this  directory(only index to metadata) to parent dir's dirEnts
    free(parentDir);
    printf("Freeing: %ld bytes\n", sizeof(dir));
    parentDir = NULL;
    free(de);
    printf("Freeing: %ld bytes\n", sizeof(dirEnt));
    de = NULL;
    return 0; //not sure what return value is supposed to represent yet
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
    //call b_open(name,flags?)? // NO b_open only for opening files
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
    return 0;
}

int fs_setcwd(char *buf){ //linux chdir  //cd
    return 0;
}

int fs_isFile(char * path){ //return 1 if file, 0 otherwise
    dirEnt* de = malloc(sizeof(dirEnt));
    int index = findDirEnt(path);
    int retVal = LBAread(de, index,sizeof(dirEnt)/512);

    if(de->type == 1)
        return 0;
    return 1;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    dirEnt* de = malloc(sizeof(dirEnt));
    int index = findDirEnt(path);
    int retVal = LBAread(de, index,sizeof(dirEnt)/512);

    if(de->type == 0)
        return 0;
    return 1;
}		

int fs_stat(const char *path, struct fs_stat *buf){
    return 0;
}

int fs_delete(char* filename){ //removes a file
    //findDirEnt()  //find location of file at pathname
    return 0;
}
