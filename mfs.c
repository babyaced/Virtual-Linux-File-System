#include <stdlib.h>
#include <string.h>

#include "mfs.h"
#include "fsInit.h"
#include "dirMgr.h"
#include "fsLow.h"
#include "freeMgr.h"
#include "hashTable.h"

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
    int dirEntIndex = findDirEnt(pathname,0);    //findDirEnt()  //find location of directory at pathname
    if(dirEntIndex == -1){
        return -1;
    }

    if(dirEntIndex == vcb->rdLoc){
        return -1;
    }

    dirEnt* fs_rmdirPDE = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* fs_rmdirDE = malloc(toBlockSize(sizeof(dirEnt)));
    dir* fs_rmdirD = malloc(toBlockSize(sizeof(dir)));
    dir* fs_rmdirPD = malloc(toBlockSize(sizeof(dir)));
    dirEnt* dotDE = malloc(toBlockSize(sizeof(dir)));
    dirEnt* dotdotDE = malloc(toBlockSize(sizeof(dir)));
    
    retVal = LBAread(fs_rmdirDE, vcb->deBlkCnt, dirEntIndex);

    int directoryEntryCount = 0;
    if(fs_rmdirDE->type == 1){
        
        retVal = LBAread(fs_rmdirD, vcb->dBlkCnt, fs_rmdirDE->dataIndex);

        for(int i = 0; i < (sizeof(fs_rmdirD->dirEnts)/sizeof(fs_rmdirD->dirEnts[0])); i++){
            if(fs_rmdirD->dirEnts[i] != -1){
                directoryEntryCount++;
                if(directoryEntryCount > 2){
                    free(fs_rmdirDE);
                    fs_rmdirDE = NULL;
                    free(fs_rmdirD);
                    fs_rmdirD = NULL;
                    free(fs_rmdirPD);
                    fs_rmdirPD = NULL;
                    return -1;
                }
            }
        }
        int dotIndex = hash_table_lookup(".",fs_rmdirD);
        int dotdotIndex = hash_table_lookup("..", fs_rmdirD);
        retVal = LBAread(dotDE, vcb->deBlkCnt, dotIndex);
        retVal = LBAread(dotdotDE, vcb->deBlkCnt, dotdotIndex);
        retVal = LBAread(fs_rmdirPDE, vcb->deBlkCnt, fs_rmdirDE->parentLoc);
        retVal = LBAread(fs_rmdirPD, vcb->dBlkCnt, fs_rmdirPDE->dataIndex);
        bool success = removeDirEnt(fs_rmdirD, dotDE);
        success = removeDirEnt(fs_rmdirD, dotdotDE);
        success = removeDirEnt(fs_rmdirPD, fs_rmdirDE);
        clearFreeBlocks(fs_rmdirD->loc, vcb->dBlkCnt);
        retVal= 0;
    }else{
        retVal = -1;
    }
    free(fs_rmdirDE);
    fs_rmdirDE = NULL;
    free(fs_rmdirD);
    fs_rmdirD = NULL;
    free(fs_rmdirPD);
    fs_rmdirPD = NULL;
    return retVal;    
    //need to iterate through dirEnts[] of dir and setFreeBlocks for each
    //clear directory entry //reset to zero values
}

fdDir * fs_opendir(const char *name){
    int retVal;
    int dirEntIndex = findDirEnt(name,0);  //find location of directory at pathname

    dirEnt* fs_opendirDE = malloc(toBlockSize(sizeof(dirEnt)));

    retVal = LBAread(fs_opendirDE, vcb->deBlkCnt, dirEntIndex);

    fdDir* fs_open_fddir = malloc(toBlockSize(sizeof(fdDir)));
    if(fs_open_fddir->isInitialized != 1)
    {
        if(fs_opendirDE->type == 1)
        {
            fs_open_fddir->dirpItemInfo = malloc(sizeof(struct fs_diriteminfo));
            fs_open_fddir->dirEntryPosition = 0;
            fs_open_fddir->directoryStartLocation = fs_opendirDE->dataIndex;
            fs_open_fddir->d_reclen = sizeof(fs_open_fddir);                        //what does this mean?
            fs_open_fddir->isInitialized = 1;                                       //set initialized to true
        }
        else{
            return NULL;
        }        
    }

    free(fs_opendirDE);
    fs_opendirDE = NULL;

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
    dir* fs_readdirD = malloc(toBlockSize(sizeof(dir)));

    retVal = LBAread(fs_readdirD, vcb->dBlkCnt, dirp->directoryStartLocation);
    retVal = LBAread(fs_readdirDE, vcb->deBlkCnt, fs_readdirD->dirEnts[dirEntPos]);
    
    dirp->dirpItemInfo->d_reclen = sizeof(fdDir);
    dirp->dirpItemInfo->fileType = fs_readdirDE->type;
    strncpy(dirp->dirpItemInfo->d_name,fs_readdirDE->name, strlen(fs_readdirDE->name));
    dirp->dirpItemInfo->d_name[strlen(fs_readdirDE->name)] = '\0';

    free(fs_readdirDE);
    fs_readdirDE = NULL;
    free(fs_readdirD);
    fs_readdirD = NULL;

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
    dirEnt* fs_setcwdDE = malloc(toBlockSize(sizeof(dirEnt)));
    //buf is path the user wants to change to

    retVal = LBAread(fs_setcwdDE, currentBlockSize, dirEntIndex);
    if(fs_setcwdDE->type == 0)
        return -1;
    currentBlock = fs_setcwdDE->loc;                          //set currentBlock to desired directory(NOT DIRECTORY ENTRY)
    free(fs_setcwdDE);
    fs_setcwdDE = NULL;
    return 0; //returns 0 with success
}

int fs_isFile(char * path){ //return 1 if file, 0 otherwise
    int deIndex = findDirEnt(path,0); //get index of directory entry pointed to by path
    dirEnt* fs_isFileDE = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(fs_isFileDE,vcb->deBlkCnt, deIndex); //read directory entry into fs_isFileDE

    if(fs_isFileDE->type == 0)
    {
        free(fs_isFileDE);
        fs_isFileDE=NULL;
        return 1;
    }
    free(fs_isFileDE);
    fs_isFileDE=NULL;
        
    return 0;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    int deIndex = findDirEnt(path,0); //get index of directory entry pointed to by path
    dirEnt* fs_isDirDE = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(fs_isDirDE,vcb->deBlkCnt, deIndex); //read directory entry into fs_isDirDE

    if(fs_isDirDE->type == 1)
    {
        free(fs_isDirDE);
        fs_isDirDE=NULL;
        return 1;
    }
    free(fs_isDirDE);
    fs_isDirDE=NULL;
    return 0;
}		

int fs_stat(const char *path, struct fs_stat *buf){
    int dirEntIndex = findDirEnt(path,0);

    dirEnt* fs_statDE = malloc(toBlockSize(sizeof(dirEnt)));

    int retVal = LBAread(fs_statDE,vcb->deBlkCnt, dirEntIndex);
    buf->st_blksize = vcb->sizeOfBlocks;
    buf->st_size =  fs_statDE->dataByteCnt;
    buf->st_blocks = fs_statDE->dataBlkCnt;

    free(fs_statDE);
    fs_statDE = NULL;
    return 0;
}

int fs_delete(char* filename){ //removes a file
    int ret;

    printf("\nfs_delete\n");

    int deIndex = findDirEnt(filename, 0);
    if (deIndex==-1) return -1; // if no file found, fs_delete returns error
    if (fs_isDir(filename)) return -1; // return error is path is a directory

    dirEnt* de = malloc(toBlockSize(sizeof(dirEnt)));
    int retVal = LBAread(de, vcb->deBlkCnt, deIndex); //read directory entry into de

    dirEnt* parentDE = malloc(toBlockSize(sizeof(dirEnt)));

    retVal =  LBAread(parentDE, vcb->deBlkCnt, de->parentLoc);

    dir* parentD = malloc(toBlockSize(sizeof(dir)));
    retVal = LBAread(parentD, vcb->dBlkCnt, parentDE->dataIndex); //read directory entry into de

    deleteExts(de); // deleting the data / blobs of a file

    bool success = removeDirEnt(parentD, de);
    if (success){
        ret = 0;
        printf("de delete SUCCESS\n");
    }
    else{
        ret = -1;
        printf("de delete FAILED\n");
    } 

    deIndex = findDirEnt(filename,0);

    free(de);
    de = NULL;
    free(parentDE);
    parentDE = NULL;
    free(parentD);
    parentD = NULL;
    return ret;
}

int fs_move(char* srcPath, char* destPath){
    int srcDEInd, destDEInd, retVal, validMove;
    bool rename = false;
    dirEnt* srcDE  = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* destDE = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* srcParentDE = malloc(toBlockSize(sizeof(dirEnt)));
    dirEnt* destParentDE = malloc(toBlockSize(sizeof(dirEnt)));
    dir* srcParentD = malloc(toBlockSize(sizeof(dir)));
    dir* destParentD = malloc(toBlockSize(sizeof(dir)));
    


    srcDEInd = findDirEnt(srcPath, 0);
    if(srcDEInd == -1){
        //src path not found, file to be moved not found so we must return error
        validMove = -1;
    }
    destDEInd = findDirEnt(destPath, 0);
    if(destDEInd == -1){
        //dest path not found, but user may be intending to move the file and rename it,
        destDEInd = findDirEnt(destPath, 1);    //pass in create flag and try again
        if(destDEInd == -1){
            validMove = -1;                //error could be invalid pathname
        }
        else{
            retVal = LBAread(destDE, vcb->deBlkCnt, destDEInd);
            rename = true;
        }
        //so don't return just yet
    }else{  //destDEInd exists, fine if it is directory, not fine if it is a file
        retVal = LBAread(destDE, vcb->deBlkCnt, destDEInd);
        if(destDE->type == 0){  //we are going to overwrite file
            validMove = -1;         //return error
        }
    }

    retVal = LBAread(srcDE, vcb->deBlkCnt, srcDEInd);
    if(srcDE->type == 1){
        //cant move a directory
        validMove = -1;
    }
    
    if(validMove != -1){
        //first delete the srcDE from its parent directory 
        retVal = LBAread(srcParentDE, vcb->deBlkCnt, srcDE->parentLoc);
        retVal = LBAread(srcParentD, vcb->dBlkCnt, srcParentDE->dataIndex);
        hash_table_delete(srcParentD, srcDE);
        retVal = LBAwrite(srcParentD, vcb->dBlkCnt, srcParentDE->dataIndex); //write the directory with the deleted src file to disk

        if(rename == true){ //we need to rename the srcDE to and move it to the parent directory of the destDE we just created
            strncpy(srcDE->name, destDE->name, strlen(destDE->name));  //rename                            
            srcDE->name[strlen(destDE->name)] = '\0';                  //explicitly null terminate the name 
            srcDE->parentLoc = destDE->parentLoc;                      //copy parent directory entry location because that may change
            retVal = LBAread(destParentDE,vcb->deBlkCnt, destDE->parentLoc);
            retVal = LBAread(destParentD, vcb->dBlkCnt,destParentDE->dataIndex);  
            removeDirEnt(destParentD,destDE);                          //remove temporary destDE
        }
        else{//or we need to move the srcDE to the directory at destDE
            retVal = LBAread(destParentD, vcb->dBlkCnt, destDE->dataIndex);
            srcDE->parentLoc = destDE->loc;  
            if(retVal == -1){
                return -1;   //directory is likely full
            }
        }
        addDirEnt(destParentD, srcDE);                             //add our srcDE to destDE's parent directory's directory entries
        retVal = LBAwrite(srcDE, vcb->deBlkCnt, srcDE->loc);       //write the updated src directory entry to disk
        validMove = 0;
    }


    //free all the things
    free(srcDE);
    srcDE = NULL;
    free(destDE);
    destDE = NULL;
    free(srcParentDE);
    srcParentDE = NULL;
    free(destParentDE);
    destParentDE = NULL;
    free(srcParentD);
    srcParentD = NULL;
    free(destParentD);
    destParentD = NULL;
    return validMove;
}