/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: fsLow.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>


#include "b_io.h"

#define FT_REGFILE	DT_REG //type for fs_diriteminfo
#define FT_DIRECTORY DT_DIR //type for fs_dirIteminfo
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


struct fs_diriteminfo  //get names of diritems //for ls
	{
    unsigned short d_reclen;    /* length of this record */  //size of structures
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};


typedef struct  //keeps track of File control block iterations through directory //iterator function //for ls
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	struct fs_diriteminfo* dirpItemInfo;
	bool isInitialized;
	
	unsigned short  d_reclen;		/*length of this record */
	unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
	} fdDir;


int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);
fdDir * fs_opendir(const char *name);  
struct fs_diriteminfo *fs_readdir(fdDir *dirp); //every time I call read it will return the next diritem info //returns name
int fs_closedir(fdDir *dirp);

char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdir
int fs_isFile(char * path);	//return 1 if file, 0 otherwise
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file

int fs_move(char* srcPath, char* destPath);

//helper functions
void strcatF(char* dest, char* src);
// int deleteDirEnt(int deIndex);



struct fs_stat  //file stats//metadata
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */

	//ignore these for now
	// time_t    st_accesstime;   	/* time of last access */           
	// time_t    st_modtime;   	/* time of last modification */
	// time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *buf); 

#endif

