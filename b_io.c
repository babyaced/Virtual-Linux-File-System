/**************************************************************
* Class:  CSC-415-0# 
* Name:
* Student ID:
* Project: File System Project
*
* File: b_io.c
*
* Description: basic I/O functions
*
**************************************************************/
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <dirMgr.h>
#include "b_io.h"

#define B_CHUNK_SIZE 512
#define MAX_OPEN_FILES 20

typedef struct FD {
    int lbaPosition;
    char* buffer;
    int ourBufferOffset;
    int bytesInBuffer;
}FD;

FD openFileTables[MAX_OPEN_FILES]; // fd is index in fd openFileTables[]

int areWeInitialized = 0;	//Indicates that this has not been initialized

void b_init()
{

	areWeInitialized = 1;
	for(int i = 0; i < MAX_OPEN_FILES; i++)
	{
		//do something with oftables
		openFileTables[i].lbaPosition = -1;
		openFileTables[i].bytesInBuffer = 0;
		openFileTables[i].ourBufferOffset = 0;
	}
}


int b_open (char* filename, int flags){  //cannot open directory
    //startPartitionSystem?
	
	// store the lbaPosition in the fd struct  
    char dirName[255];
	char baseName[255];  
    strcpy(dirName,filename); //makes copy of filename to find dirName //dirname() is destructive!
    strcpy(baseName,filename); //makes copy of filename to find baseName //basename() is destructive!

    printf("Dir Name: %s\n",dirname(dirName));  //will return whole path minus file name
    printf("Base Name: %s\n",basename(baseName)); // will return file name

    //pass dirname into findDir(function)
    int dirIndex = findDir(dirName);

    //read 
    //return lba index as "fd"
}

int b_read (int fd, char * buffer, int count)  //this is copy of bierman's version 
	{
	int bytesRead;				// for our reads
	int bytesReturned;			// what we will return
	int part1, part2, part3;	// holds the three potential copy lengths
	
	if (areWeInitialized == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAX_OPEN_FILES-1)
	if ((fd < 0) || (fd >= MAX_OPEN_FILES))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (openFileTables[fd].lbaPosition == -1)		//File not open for this descriptor
		{
		return -1;
		}	
		
	
	// number of bytes available to copy from buffer
	int remain = openFileTables[fd].bytesInBuffer - openFileTables[fd].ourBufferOffset;	
	part3 = 0;				//only used if count > B_CHUNK_SIZE
	if (remain >= count)  	//we have enough in buffer
		{
		part1 = count;		// completely buffered
		part2 = 0;
		}
	else
		{
		part1 = remain;				//spanning buffer (or first read)
		part2 = count - remain;
		}
				
	if (part1 > 0)	// memcpy part 1
		{
		memcpy (buffer, openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, part1);
		openFileTables[fd].ourBufferOffset = openFileTables[fd].ourBufferOffset + part1;
		}
		
	if (part2 > 0)		//We need to read to copy more bytes to user
		{
		// Handle special case where user is asking for more than a buffer worth
		if (part2 > B_CHUNK_SIZE)
			{
			int blocks = part2 / B_CHUNK_SIZE; // calculate number of blocks they want
			bytesRead = LBAread (buffer+part1,blocks,openFileTables[fd].lbaPosition);
			part3 = bytesRead;
			part2 = part2 - part3;  //part 2 is now < B_CHUNK_SIZE, or file is exusted
			}				
		
		//try to read B_CHUNK_SIZE bytes into our buffer
		bytesRead = LBAread (openFileTables[fd].buffer,1, openFileTables[fd].lbaPosition);  //keep as 1 block for now
		
		// error handling here...  if read fails
		
		openFileTables[fd].bytesInBuffer = bytesRead; //how many bytes are actually in bufferfer
		
		if (bytesRead < part2) // not even enough left to satisfy read
			part2 = bytesRead;
			
		if (part2 > 0)	// memcpy bytesRead
			{
			memcpy (buffer+part1+part3, openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, part2);
			openFileTables[fd].ourBufferOffset = openFileTables[fd].ourBufferOffset + part2;
			}
			
		}
	bytesReturned = part1 + part2 + part3;
	return (bytesReturned);	
}

int b_write (int fd, char * buffer, int count){
/*
    if (areWeInitialized == 0) 
	b_init();  //Initialize system for write

	// check that fd is between 0 and (MAX_OPEN_FILES-1)
    if ((fd < 0) || (fd >= MAX_OPEN_FILES)) {
        return (-1); 					//invalid file descriptor
    }
		
    if (openFileTables[fd].linuxFD == -1) {	//File not open for this descriptor
		return -1;
    }
*/
    // get block position from oft struct
    int lbaPosition = 1;
    // int count converts to lbaCount
    int lbaCount = 4;// test
    int retVal = LBAwrite(buffer, lbaCount, lbaPosition);
    return retVal;
}
int b_seek (int fd, off_t offset, int whence){

    if (areWeInitialized == 0) b_init();  //Initialize our system

    if(whence == 0){
        printf("The %d is currently in read mode.\n", fd);
    }else {
        printf("The %d is currently in write mode.\n", fd);
    }

    //empties our buffer
    if(openFileTables[fd].bytesInBuffer == 512){
        openFileTables[fd].bytesInBuffer = 0;
    }

    //still need to adjust file location
}
void b_close (int fd){
    //closePartitionSystem?
    //free everything associated with fd?
    FD* oft = &openFileTables[fd];
    printf("Currently closing this file: %d\n", fd);

    //dummy variable
    int blockIndex;
    int lbaCount;
    int linuxFD;

    //writing the left over bytes
    if(oft->ourBufferOffset > 0 && oft->bytesInBuffer > 0){
        printf("The file was in write mode.\n");
        if(blockIndex = -1){
            //
            printf("We do not have enough free space.\n");
            return 0;
        }else {
            //writes remainder bytes into a new block
            printf("writing the leftover bytes now.\n");
            LBAwrite(buffer, lbaCount, lbaPosition);
        }
    }

    //idk if we are allowed to use clse() and not sure if we have a linuxFD or similar variable
    close(openFileTables[fd].linuxFD); //not sure what our linux file handler variable is
    free(openFileTables[fd].buffer); //or if we are supposed to have one
    openFileTables[fd].buffer = NULL;
    openFileTables[fd].linuxFd = -1;



    // if(oft->ourBufferOffset > 0 /* && possibly check for file mode here */) {
    //     /* check if we have enough free space, if we do, write last chunk
	// 		else, return */
    //     if (/*block index(?) has enough free space*/) {
	// //do the last write
    //     } else {
    //         return;
    //     }
    // }

    //close
    //free buffer
    //set buffer to null;
    openFileTables->lbaPosition = -1;
}