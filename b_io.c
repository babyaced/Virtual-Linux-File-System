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
#include "fsLow.h"
#include "freeMgr.h"

#define B_CHUNK_SIZE 512
#define MAX_OPEN_FILES 20

extern vCB* vcb;
extern unsigned int* freeSpaceBitmap; 

//FCB?
typedef struct FD {
    int lLoc;   //"fake" logical location
    int pLoc;   //"real" disk location
    char* buffer;
    int ourBufferOffset;
    int bytesInBuffer;

    int dirEntIndex;

    // need to implement still
    int lOffset; //offset of blocks for a file of block length greater than 1
    int blocksAlloced;  //how many blocks are currently allocated to a file

}FD;

FD openFileTables[MAX_OPEN_FILES]; // fd is index in fd openFileTables[]

int areWeInitialized = 0;	//Indicates that this has not been initialized

void b_init()
{

   
    for(int i = 0; i < MAX_OPEN_FILES; i++)
    {
        //do something with oftables
        openFileTables[i].lLoc = -1;         //extent offset tracker
        openFileTables[i].pLoc = -1;
        openFileTables[i].bytesInBuffer = 0; // used to tell buffer is empty but not used yet
        openFileTables[i].ourBufferOffset = 0;
        openFileTables[i].dirEntIndex = 0;     //for keeping track of open directory entry
        openFileTables[i].lOffset = 0;      // how many blocks we are into a file
        openFileTables[i].blocksAlloced = 0;  //how many blocks are allocated to the file
    }
    areWeInitialized = 1;
}

int b_getFCB()
{
	for (int i = 0; i < MAX_OPEN_FILES; i++)
		{
		if (openFileTables[i].pLoc == -1)
			{
			openFileTables[i].pLoc = -2; // used but not assigned
			return i;		//Not thread safe
			}
		}
	return (-1);  //all in use
}


int b_open (const char* filename, int flags){  //cannot open directory

    if(areWeInitialized == 0) b_init(); //initialize our system

    //pass dirname into findDir(function)
    int dirEntIndex = findDirEnt(filename,1);  //get directory entry of that file //create it if it doesn't exist

    if(dirEntIndex == -1)  //error opening filename //file doesn't exist
        return (-1);
    
    int fcbFD = b_getFCB();
    
    dirEnt* b_openDE = malloc(toBlockSize(sizeof(dirEnt)));
    // printf("Malloced %d bytes for b_openDE\n",toBlockSize(sizeof(dirEnt)));

    int retVal = LBAread(b_openDE,vcb->deBlkCnt, dirEntIndex);

    

    if(b_openDE->dataIndex == -1) //in case of unitialized file
    {
        openFileTables[fcbFD].pLoc = -2;  //indicate that no blocks are allocated for file data //not real value
    }else{  //in case of initialized file
        openFileTables[fcbFD].pLoc = b_openDE->dataIndex;  //save the LBA position
    }
    
    openFileTables[fcbFD].dirEntIndex = b_openDE->loc;
    free(b_openDE);
    b_openDE = NULL;
    // printf("Freed   %d bytes for b_openDE and set b_openDE to NULL\n",toBlockSize(sizeof(dirEnt)));
    //int fileDataBlocks = b_openDE->dataBlkCnt;

    //allocate our buffer
    openFileTables[fcbFD].buffer = malloc(B_CHUNK_SIZE);
    // printf("Malloced %d bytes for openFileTables[%d].buffer\n",B_CHUNK_SIZE,fcbFD);
    if(openFileTables[fcbFD].buffer == NULL)
    {
        b_close(fcbFD);  //close the file
        openFileTables[fcbFD].pLoc = -1; //free FCB
        return -1;  //return error code
    }
    return (fcbFD);  //return our file descriptor
}

int b_read (int fd, char * buffer, int count)  //this is copy of bierman's version 
{
    int blocksRead;             //how many blocks were read
    int bytesRead;				// for our reads
    int bytesReturned;			// what we will return
    int part1, part2, part3;	// holds the three potential copy lengths

    if (areWeInitialized == 0) b_init();  //Initialize our system

    // check that fd is between 0 and (MAX_OPEN_FILES-1)
    if ((fd < 0) || (fd >= MAX_OPEN_FILES))
    {
        return (-1); 					//invalid file descriptor
    }

    if (openFileTables[fd].pLoc == -1)		//File not open for this descriptor
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
            LBAread(buffer+part1,blocks,openFileTables[fd].lLoc);                    //need to make sure that this is not reading from where it is not supposed to, because it assumes the blocks are physically contigious
            bytesRead = blocks* vcb->sizeOfBlocks;
            part3 = bytesRead;
            part2 = part2 - part3;  //part 2 is now < B_CHUNK_SIZE, or file is exusted
        }

        //try to read B_CHUNK_SIZE bytes into our buffer
        LBAread(openFileTables[fd].buffer,1, openFileTables[fd].lLoc);  //keep as 1 block for now
        bytesRead = vcb->sizeOfBlocks;

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


int b_write (int fd, char * buffer, int count)
{
	if (areWeInitialized == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAX_OPEN_FILES-1)
	if ((fd < 0) || (fd >= MAX_OPEN_FILES))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (openFileTables[fd].pLoc == -1)		//File not open for this descriptor
		{
		return -1;
		}	
		
	int bytesWritten = 0;
	int blocksDirectlyWritten = 0;
	int callerBufferOffset = 0;
    int blockToWriteTo = 0;

    int retVal;
    dirEnt* b_writeDE = malloc(toBlockSize(sizeof(dirEnt)));  //for saving modified data to opened directory entry
    // printf("Malloced %d bytes for b_openDE\n",toBlockSize(sizeof(dirEnt)));
    retVal = LBAread(b_writeDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex);


	if(count >= B_CHUNK_SIZE){
		if(openFileTables[fd].bytesInBuffer == 0)  //if there are no bytes in our buffer
		{
			while(count >= bytesWritten + B_CHUNK_SIZE){
                if(openFileTables[fd].pLoc == -2){  //if this is a clean file
                    openFileTables[fd].pLoc = findFreeBlocks(count/vcb->sizeOfBlocks);  //initialize location of data for file and find needed blocks
                    openFileTables[fd].blocksAlloced = count/vcb->sizeOfBlocks;
                    b_writeDE->dataIndex = openFileTables[fd].pLoc;
                    b_writeDE->dataBlkCnt = openFileTables[fd].blocksAlloced;
                    setFreeBlocks(openFileTables[fd].pLoc,openFileTables[fd].blocksAlloced);
                    blockToWriteTo = openFileTables[fd].pLoc;
                }
                else if(openFileTables[fd].lOffset >= openFileTables[fd].blocksAlloced){ //if the blocks originally allocated already are completely full
                    if(openFileTables[fd].lLoc == -1)
                        openFileTables[fd].lLoc = 0;
                    openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].lLoc);
                    blockToWriteTo = openFileTables[fd].pLoc;
                }else{
                    blockToWriteTo = openFileTables[fd].pLoc + openFileTables[fd].lOffset;
                }

                blocksDirectlyWritten = LBAwrite(buffer + callerBufferOffset,1,blockToWriteTo);
                if(openFileTables[fd].lLoc != -1)
                    openFileTables[fd].lLoc++;
                openFileTables[fd].lOffset ++;  //increment our logical offset

				callerBufferOffset += blocksDirectlyWritten * vcb->sizeOfBlocks;   //increment caller buffer offset
				bytesWritten += blocksDirectlyWritten * vcb->sizeOfBlocks;  //increment byte count for this loop

			}
			int bytesNeeded = count - bytesWritten;  
			memcpy(openFileTables[fd].buffer, buffer + callerBufferOffset,bytesNeeded); //copy amount of bytes needed to buffer
			openFileTables[fd].bytesInBuffer += bytesNeeded; // increment our byte count
			openFileTables[fd].ourBufferOffset += bytesNeeded; //increment our buffer offset

		}
		else{//if there are bytes in our buffer  //no need to allocate blocks to the open fd
			memcpy(openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, buffer, B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer);  //copy what we can to our buffer
            bytesWritten = B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer;
            openFileTables[fd].ourBufferOffset += B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer; //increment our buffer offset
            openFileTables[fd].bytesInBuffer += B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer; // increment our byte count
            callerBufferOffset = bytesWritten;  //Offset caller buffer by amount of bytes already copied into our buffer
            if(openFileTables[fd].pLoc == -2){
                openFileTables[fd].pLoc = findFreeBlocks((openFileTables[fd].bytesInBuffer+ count)/vcb->sizeOfBlocks);  //initialize location of data for file and find needed blocks
                openFileTables[fd].blocksAlloced = (openFileTables[fd].bytesInBuffer+ count)/vcb->sizeOfBlocks;
                b_writeDE->dataIndex = openFileTables[fd].pLoc;
                b_writeDE->dataBlkCnt = openFileTables[fd].blocksAlloced;
                setFreeBlocks(openFileTables[fd].pLoc,openFileTables[fd].blocksAlloced);
                blockToWriteTo = openFileTables[fd].pLoc;
            }else if(openFileTables[fd].lOffset >= openFileTables[fd].blocksAlloced){ //if the blocks originally allocated already are completely full
                if(openFileTables[fd].lLoc == -1)
                    openFileTables[fd].lLoc = 0;
                openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].lLoc);
                blockToWriteTo = openFileTables[fd].pLoc;
            }else{   //if blocks originally allocated are not not full
                blockToWriteTo = openFileTables[fd].pLoc + openFileTables[fd].lOffset;
            }
            printf("Strlen(buffer): %ld\n", strlen(openFileTables[fd].buffer));
			blocksDirectlyWritten = LBAwrite(openFileTables[fd].buffer,1,blockToWriteTo);  //Write 1 block to disk
            // bytesWritten += blocksDirectlyWritten * vcb->sizeOfBlocks;  //increment byte count for this loop
            openFileTables[fd].lOffset++;
            if(openFileTables[fd].lLoc != -1)
                openFileTables[fd].lLoc++;

			int bytesNeeded = count - callerBufferOffset;  //decrement count by bytes copied into our buffer and store in bytesNeeded
			openFileTables[fd].bytesInBuffer = 0;  // reset our buffer
			openFileTables[fd].ourBufferOffset = 0;
			while(bytesNeeded >= bytesWritten + B_CHUNK_SIZE){  //if there is more than B_CHUNK_SIZE bytes needed to be written
                if(openFileTables[fd].lOffset >= openFileTables[fd].blocksAlloced){ //if the blocks originally allocated already are completely full
                    if(openFileTables[fd].lLoc == -1)
                        openFileTables[fd].lLoc = 0;
                    openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].lLoc);
                    blockToWriteTo = openFileTables[fd].pLoc;
                }else{
                    blockToWriteTo = openFileTables[fd].pLoc + openFileTables[fd].lOffset;
                }

				blocksDirectlyWritten = LBAwrite(buffer + callerBufferOffset,1,blockToWriteTo);  //write B_CHUNK_SIZE bytes directly
                openFileTables[fd].lOffset++;
                if(openFileTables[fd].lLoc != -1)
                    openFileTables[fd].lLoc++;
				callerBufferOffset += blocksDirectlyWritten * vcb->sizeOfBlocks;  //offset the caller buffer
				bytesWritten += blocksDirectlyWritten * vcb->sizeOfBlocks;  //increment our bytesWritten

			}
			bytesNeeded = count - bytesWritten;  //decrement bytes needed by bytes directly written, if any
			memcpy(openFileTables[fd].buffer, buffer + callerBufferOffset, bytesNeeded);  //copy bytesNeeded into our buffer
			openFileTables[fd].bytesInBuffer += bytesNeeded;  // increment our byte count
			openFileTables[fd].ourBufferOffset += bytesNeeded;  //increment our offset
		}

	}else{
		if(openFileTables[fd].bytesInBuffer + count <= B_CHUNK_SIZE){  // if bytes requested wont overflow buffer
			memcpy(openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, buffer, count); // add to current buffer
			openFileTables[fd].bytesInBuffer += count;
			openFileTables[fd].ourBufferOffset += count;
		}else{  //if bytes requested will overflow buffer
            if(openFileTables[fd].pLoc == -2){
                    openFileTables[fd].pLoc = findFreeBlocks((openFileTables[fd].bytesInBuffer+ count)/vcb->sizeOfBlocks);  //initialize location of data for file
                    openFileTables[fd].blocksAlloced = (openFileTables[fd].bytesInBuffer+ count)/vcb->sizeOfBlocks;
                    b_writeDE->dataIndex = openFileTables[fd].pLoc;
                    b_writeDE->dataBlkCnt = openFileTables[fd].blocksAlloced;
                    setFreeBlocks(openFileTables[fd].pLoc,openFileTables[fd].blocksAlloced);
                    blockToWriteTo = openFileTables[fd].pLoc + openFileTables[fd].lOffset;
            }else if(openFileTables[fd].lOffset >= openFileTables[fd].blocksAlloced){ //if the blocks originally allocated already are completely full
                if(openFileTables[fd].lLoc == -1)
                    openFileTables[fd].lLoc = 0;
                openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].lLoc);
                blockToWriteTo = openFileTables[fd].pLoc;
            }else{
                blockToWriteTo = openFileTables[fd].pLoc + openFileTables[fd].lOffset;
            }
			memcpy(openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, buffer, B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer); //Copy bytes from caller that we can
			callerBufferOffset = B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer;  //Offset caller buffer by amount of bytes already copied into our buffer
			openFileTables[fd].bytesInBuffer = B_CHUNK_SIZE;  //buffer is full
            openFileTables[fd].ourBufferOffset = B_CHUNK_SIZE;

			blocksDirectlyWritten = LBAwrite(openFileTables[fd].buffer, 1, blockToWriteTo);  //Write full buffer to file
            openFileTables[fd].lOffset++;         //increment logical offset since we just wrote a block
            if(openFileTables[fd].lLoc != -1)
                openFileTables[fd].lLoc++;         //increment logical offset since we just wrote a block

			openFileTables[fd].bytesInBuffer = 0;  //Buffer is now empty
			openFileTables[fd].ourBufferOffset = 0; //Reset our offset
			memcpy(openFileTables[fd].buffer, buffer + callerBufferOffset, count - callerBufferOffset);  // copy remaining bytes into buffer
			openFileTables[fd].bytesInBuffer += count - callerBufferOffset; // increment our byte count
			openFileTables[fd].ourBufferOffset += count - callerBufferOffset; //increment our offset
		}
	}
    retVal = LBAwrite(b_writeDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex); //save modified directory entry to disk
    free(b_writeDE);
    b_writeDE = NULL;
    // printf("Freed   %d bytes for b_writeDE and set b_writeDE to NULL\n",toBlockSize(sizeof(dirEnt)));
	return -1;
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
    dirEnt* b_closeDE = malloc(toBlockSize(sizeof(dirEnt)));  //for saving modified data to opened directory entry
    // printf("Malloced %d bytes for b_closeDE\n",toBlockSize(sizeof(dirEnt)));
    int blockToWriteTo;

    int retVal = LBAread(b_closeDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex);

    if(openFileTables[fd].bytesInBuffer > 0){ // if there are bytes remaining in the buffer
        if(openFileTables[fd].lOffset < openFileTables[fd].blocksAlloced){
            blockToWriteTo = openFileTables[fd].pLoc + openFileTables[fd].lOffset;
        }
        else if(openFileTables[fd].pLoc == -2){
            openFileTables[fd].pLoc = findFreeBlocks(B_CHUNK_SIZE/vcb->sizeOfBlocks);  //find free blocks for our file  
            b_closeDE->dataIndex = openFileTables[fd].pLoc;  //save data index just found to our directory entry
            b_closeDE->dataBlkCnt = B_CHUNK_SIZE/vcb->sizeOfBlocks;
            blockToWriteTo = openFileTables[fd].pLoc;
        }
        else{
            if(openFileTables[fd].lLoc == -1)
                openFileTables[fd].lLoc = 0;
            openFileTables[fd].pLoc = getLba(b_closeDE,openFileTables[fd].lLoc);
            blockToWriteTo = openFileTables[fd].pLoc;
        }
        retVal = LBAwrite(openFileTables[fd].buffer, B_CHUNK_SIZE/vcb->sizeOfBlocks, blockToWriteTo);  //write rest of buffer to disk
        retVal = LBAwrite(b_closeDE, vcb->deBlkCnt, b_closeDE->loc);  //save modified directory entry to disk
        if(openFileTables[fd].lLoc != 0)
            openFileTables[fd].lLoc++;
        openFileTables[fd].lOffset++;
    }

    free(b_closeDE);
    b_closeDE = NULL;
    // printf("Freed %d bytes for b_closeDE and set b_closeDE to NULL\n",toBlockSize(sizeof(dirEnt)));
    free(openFileTables[fd].buffer);
    // printf("Freed %d bytes for openFileTables[%d].buffer and set openFileTables[%d].buffer to NULL\n",B_CHUNK_SIZE,fd,fd);
    openFileTables[fd].buffer = NULL;
    openFileTables[fd].lLoc = -1;
    openFileTables[fd].pLoc = -1;
    openFileTables[fd].bytesInBuffer = 0;
    openFileTables[fd].ourBufferOffset = 0;
}
