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

struct openFileTable{
    int linuxFD;
    char* buffer;
    int ourBufferOffset;
    int bytesInBuffer;
};

struct openFileTable openFileTables[MAX_OPEN_FILES]; 

int areWeInitialized = 0;	//Indicates that this has not been initialized

void b_init()
{
	areWeInitialized = 1;
	for(int i = 0; i < MAX_OPEN_FILES; i++)
	{
		//do something with oftables
		openFileTables[i].linuxFD = -1;
		openFileTables[i].bytesInBuffer = 0;
		openFileTables[i].ourBufferOffset = 0;
	}
}


int b_open (char* filename, int flags){  //cannot open directory
    //startPartitionSystem?
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
		
	if (openFileTables[fd].linuxFD == -1)		//File not open for this descriptor
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
			bytesRead = LBAread (buffer+part1,blocks,openFileTables[fd].linuxFD);
			part3 = bytesRead;
			part2 = part2 - part3;  //part 2 is now < B_CHUNK_SIZE, or file is exusted
			}				
		
		//try to read B_CHUNK_SIZE bytes into our buffer
		bytesRead = LBAread (openFileTables[fd].buffer,1, openFileTables[fd].linuxFD);  //keep as 1 block for now
		
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
    return 0;
}
int b_seek (int fd, off_t offset, int whence){
    return 0;
}
void b_close (int fd){
    //closePartitionSystem?
    //free everything associated with fd?
}