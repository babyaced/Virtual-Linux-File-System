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
#include "b_io.h"
#include "dirMgr.h"

#define B_CHUNK_SIZE 512
#define MAX_OPEN_FILES 20

//FCB?
typedef struct FD {
    int lbaPosition;
    char* buffer[B_CHUNK_SIZE+2];
    int ourBufferOffset;
    int bytesInBuffer;

    // need to implement still
    int blockInd; // the index of last block in the file
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
        openFileTables[i].bytesInBuffer = -1; // used to tell buffer is empty but not used yet
        openFileTables[i].ourBufferOffset = 0;
        openFileTables[i].blockInd = 0; // how many blocks we wrote into a file
    }
}


int b_open (char* filename, int flags){  //cannot open directory
    //startPartitionSystem?

    // store the lbaPosition in the fd struct
    char dirName[255];
    char baseName[255];
    strcpy(dirName,filename); //makes copy of filename to find dirName //dirname() is destructive!
    strcpy(baseName,filename); //makes copy of filename to find baseName //basename() is destructive!

    strcpy(dirName,basename(dirName));
    strcpy(baseName, basename(baseName));    

    printf("Dir Name: %s\n", dirName);  //will return whole path minus file name
    printf("Base Name: %s\n", baseName); // will return file name

    //pass dirname into findDir(function)
    int dirIndex = findDirEnt(filename);

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

    if (areWeInitialized == 0)
        b_init();  //Initialize system for write

    // check that fd is between 0 and (MAX_OPEN_FILES-1)
    if ((fd < 0) || (fd >= MAX_OPEN_FILES)) {
        return (-1); 					//invalid file descriptor
    }
    // commented out below while testing
    if (openFileTables[fd].lbaPosition == -1) {	//File not open for this descriptor
        //return -1;
    }

    int lbaPosition = 7;
    int lbaIndex = 0; // should store this in fd struct
    openFileTables[fd].blockInd;

    int bytesWritten = 0;
    int bufferSpace = openFileTables[fd].bytesInBuffer - openFileTables[fd].ourBufferOffset;
    if (bufferSpace < 0) {
        bufferSpace = 512;// negative buffLen when buffer not used yet
        openFileTables[fd].bytesInBuffer = 0;
    }

    if (count<=bufferSpace){
        memcpy(openFileTables[fd].buffer, buffer, count);
        printf("\n%s\n", openFileTables[fd].buffer);

        openFileTables[fd].bytesInBuffer += count;
        bytesWritten =+ count;
    }

    else if (bufferSpace>0){
        // memcpy until end of buffer
        memcpy(openFileTables[fd].buffer, buffer, bufferSpace);
//        printf("%s", openFileTables[fd].buffer);
        openFileTables[fd].bytesInBuffer += bufferSpace;
        bytesWritten =+ bufferSpace;

        int retVal = LBAwrite(openFileTables[fd].buffer, 1, lbaPosition + lbaIndex);
        ++ lbaIndex;
        openFileTables[fd].bytesInBuffer = 0;

        while (count - bytesWritten >= B_CHUNK_SIZE) {
//            memcpy(openFileTables[fd].buffer, buffer+bytesWritten, bufferSpace);
//            printf("%s", openFileTables[fd].buffer);
//
            int retVal = LBAwrite(buffer+bytesWritten, 1, lbaPosition + lbaIndex);
            bytesWritten += B_CHUNK_SIZE;

//            printf("\nBytes %d  \n", bytesWritten);

            ++ lbaIndex;
        }
    }

    if (openFileTables[fd].bytesInBuffer == B_CHUNK_SIZE) {
        int retVal = LBAwrite(openFileTables[fd].buffer, 1, lbaPosition + lbaIndex);
        openFileTables[fd].bytesInBuffer = 0;
        ++ lbaIndex;
    }

//    int retVal = LBAwrite("~~~Empty Block for Testing                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      ", 1, lbaPosition + lbaIndex);

//    printf("\n");
//    printf("returned %d  ", bytesWritten);
    return bytesWritten;
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
    /*if(oft->ourBufferOffset > 0 && oft->bytesInBuffer > 0){
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
    openFileTables[fd].linuxFd = -1;*/



    //FD* oft = &openFileTables[fd];
    //printf("Currently closing this file: %d\n", fd);

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
