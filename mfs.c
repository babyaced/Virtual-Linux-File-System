#include "mfs.h"

int fs_mkdir(const char *pathname, mode_t mode){ //ignore mode for now
    printf("Path Name: %s\n", pathname);
    //find free blocks
    //initDir(pathname,parent block)  //where does parent block come from
    //write this directory to parent dir's dirEnts
    return 0;
}

int fs_rmdir(const char *pathname){
    return 0;
}

fdDir * fs_opendir(const char *name){
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

int fs_setcwd(char *buf){ //linux chdir
    return 0;
}

int fs_isFile(char * path){ //return 1 if file, 0 otherwise
    return 0;
}

int fs_isDir(char * path){ //return 1 if directory, 0 otherwise
    return 0;
}		

int fs_stat(const char *path, struct fs_stat *buf){
    return 0;
}

int fs_delete(char* filename){ //removes a file
    return 0;
}
