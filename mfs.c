#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"

extern fSL* fsl;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock;
extern int currentBlockSize;

int fs_mkdir(const char *pathname, mode_t mode){ //ignore mode for now
    int retVal;
    printf("Path Name: %s\n", pathname);
    // printf("Base Name: %d\n", basename(pathname));


    //relative case
    int dirIndex = initDir(currentBlock,pathname);  //parent block is wherever this function is called from //just use pathname for testing
    dir* parentDir = malloc(sizeof(dir));
    retVal = LBAread(parentDir,currentBlockSize,currentBlock);

    //int parentIndex = findDirEnt(dirname(pathname));//use parent in pathname to find parent block
    
    dirEnt* de = malloc(sizeof(dirEnt));
    printf("Mallocing: %ld bytes", sizeof(dirEnt));

    //initialize directory entry
    de->parentLoc = currentBlock;
    de->loc = dirIndex;
    de->sizeInBlocks = vcb->rdBlkCnt;//need way to get sizeInBlocks //for now just assume same size as rd
    de->type = 1; //type is directory
    strcpy(de->name,pathname);

    addDirEnt(parentDir,de);//write this directory to parent dir's dirEnts
    free(parentDir);
    parentDir = NULL;
    free(de); 
    de = NULL;
    return 0; //not sure what return value is supposed to represent yet
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
    int index = findDirEnt(path,"");
    int retVal = LBAread(de, index,sizeof(dirEnt)/512);

    if(de->type == 1)
        return 0;
    return 1;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    dirEnt* de = malloc(sizeof(dirEnt));
    int index = findDirEnt(path,"");
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
