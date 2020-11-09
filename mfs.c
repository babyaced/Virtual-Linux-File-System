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



        int parentDirIndex = findDirEnt(pathnameCpy);   //returns root of the directory  
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
    //get and append all directory names
    dirEnt* de = malloc(sizeof(dirEnt));
    retVal = LBAread(de,currentBlockSize,currentBlock);
    while(de->parentLoc != de->loc){
        strcatF(buf, de->name);
        strcatF(buf,"/");
        printf("Path accumulator: %s\n", buf);
        retVal = LBAread(de,currentBlockSize,de->parentLoc);
    }
    strcatF(buf,de->name);
    strcatF(buf,"/");
    // ptr = strchr(buf,'\377');  // remove octal escape sequence
    // buf[strlen(buf)-strlen(ptr)] = '\0';
    printf("Path accumulator: %s\n", buf);
    
    //get currentBlock and access parent
    //continue until parentblock is 0
    free(de);
    return buf;
}

//helper for getcwd
void strcatF(char* dest, char* src){  //concatenates src to front of dest
    size_t dest_len = strlen(dest) + 1, src_len = strlen(src);
    memmove(dest + src_len, dest, dest_len);
    memcpy(dest, src, src_len);
}

int fs_setcwd(char *buf){ //linux chdir  //cd
    int retVal;
    //buf is path the user wants to change to
    //=====================================================================================
    //ABSOLUTE PATH //we need to go down through directory tree until we can find directory
    //=====================================================================================
    if(buf[0]=='/'){
        int dirEntIndex = findDirEnt(buf);
        printf("DirEntIndex: %s\n",dirEntIndex);

    }
    //=====================================================================================================================================
    //RELATIVE PATH //just directly use current block data to find directory
    //=====================================================================================================================================
    else{
        int dirEntIndex = findDirEnt(buf);
        dirEnt* de = malloc(sizeof(dirEnt));
        retVal = LBAread(de, currentBlockSize, dirEntIndex);
        currentBlock = de->dataIndex;//set currentBlock to desired directory
        free(de);
        return 0; //returns 0 with success
    }
    return 0;  
}

int fs_isFile(char * path){ //return 1 if file, 0 otherwise
    int deIndex = findDirEnt(path); //get index of directory entry pointed to by path
    dirEnt* de = malloc(sizeof(dirEnt));
    int retVal = LBAread(de, (sizeof(dirEnt)/vcb->sizeOfBlocks)+1, deIndex); //read directory entry into de

    if(de->type == 0)
    {
        free(de);
        de=NULL;
        return 1;
    }
    free(de);
    de=NULL;
        
    return 0;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    int deIndex = findDirEnt(path); //get index of directory entry pointed to by path
    dirEnt* de = malloc(sizeof(dirEnt));
    int retVal = LBAread(de,(sizeof(dirEnt)/vcb->sizeOfBlocks)+1, deIndex); //read directory entry into de

    if(de->type == 1)
    {
        free(de);
        de=NULL;
        return 1;
    }
    return 0;
}		

int fs_stat(const char *path, struct fs_stat *buf){
    return 0;
}

int fs_delete(char* filename){ //removes a file
    //findDirEnt()  //find location of file at pathname
    return 0;
}
