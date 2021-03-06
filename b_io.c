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
#include "mfs.h"

#define B_CHUNK_SIZE vcb->sizeOfBlocks
#define MAX_OPEN_FILES 20

extern vCB* vcb;
extern unsigned int* freeSpaceBitmap;

//FCB?
typedef struct FD {
    char* buffer;           //buffer to hold buffered writes
    int extOffset;          //tracks the index within the extents
    int pLoc;               //physical start location of file on disk
    int bytesInBuffer;      //how many bytes are in the buffer assigned to the file at the moment
    int ourBufferOffset;    //how many bytes are written into the file's buffer at the moment
    int dirEntIndex;        //open directory entry's location in lba
    int bytesWR;           //bytes written or read to/from file // to compare with file size
    int lastRead;
    int flags;
}FD;

FD openFileTables[MAX_OPEN_FILES]; // fd is index in fd openFileTables[]

int areWeInitialized = 0;	//Indicates that this has not been initialized

void b_init()
{


    for(int i = 0; i < MAX_OPEN_FILES; i++)
    {
        openFileTables[i].extOffset = 0;         //tracks the index within the extents
        openFileTables[i].pLoc = -1;              //physical start location of file data on lba
        openFileTables[i].bytesInBuffer = 0;      //how many bytes are in the buffer assigned to the file at the moment
        openFileTables[i].ourBufferOffset = 0;    //how many bytes are written into the file's buffer at the moment
        openFileTables[i].dirEntIndex = -1;        //open directory entry's location in lba
        openFileTables[i].bytesWR = 0;           //bytes written or read // to compare with file size
        openFileTables[i].flags = 0;              //maybe initialize this to -1 instead because 0 is O_RDONLY
        openFileTables[i].lastRead = 0;  
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
    int dirEntIndex;

    if(flags & O_CREAT)  //if O_CREAT flag is passed in
        dirEntIndex = findDirEnt(filename,1);  //get directory entry of that file //create it if it doesn't exist
    else
        dirEntIndex = findDirEnt(filename,0); //get directory entry of that file

    if(dirEntIndex == -1)  //error opening filename //file doesn't exist
        return (-1);

    int fcbFD = b_getFCB();

    if((flags & O_ACCMODE) == O_RDONLY){
        openFileTables[fcbFD].flags |= O_RDONLY;
    }
    if((flags & O_ACCMODE) == O_WRONLY){
        openFileTables[fcbFD].flags |= O_WRONLY;
    }

    dirEnt* b_openDE = malloc(toBlockSize(sizeof(dirEnt)));

    int retVal = LBAread(b_openDE,vcb->deBlkCnt, dirEntIndex);

    if(flags & O_TRUNC){
        deleteExts(b_openDE);              //free extents allocated to the file
        b_openDE->dataIndex = -1;           //reset data index to null
        b_openDE->dataBlkCnt = 0;          //reset data blocks written to zero
        b_openDE->dataByteCnt = 0;            //reset size in bytes to zero
        retVal = LBAwrite(b_openDE,vcb->deBlkCnt, dirEntIndex);  //save the changes
    }
    

    if(b_openDE->dataIndex == -1) //in case of unitialized file
    {
        openFileTables[fcbFD].pLoc = -2;  //indicate that no blocks are allocated for file data //not real value
    }else{  //in case of initialized file
        openFileTables[fcbFD].pLoc = b_openDE->dataIndex;  //save the LBA position
    }

    openFileTables[fcbFD].dirEntIndex = b_openDE->loc;
    free(b_openDE);
    b_openDE = NULL;

    //allocate our buffer
    openFileTables[fcbFD].buffer = malloc(B_CHUNK_SIZE);
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
    if((openFileTables[fd].flags & O_ACCMODE) & O_WRONLY){
        printf("You do not have permission to read from this file\n");
        return -1;
    }
    dirEnt* b_readDE = malloc(toBlockSize(sizeof(dirEnt)));  //for saving modified data to opened directory entry
    int retVal = LBAread(b_readDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex);

    int blockToReadFrom;

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
            int bufferOffset = part1;

            for(int i = 0; i < blocks; i++){
                blockToReadFrom = getLba(b_readDE,openFileTables[fd].extOffset);
                blocksRead = LBAread(buffer+bufferOffset,1,blockToReadFrom);
                openFileTables[fd].extOffset++;
                bufferOffset += B_CHUNK_SIZE;                                      //offset the buffer by 1 block
            }
            
            if(openFileTables[fd].bytesWR > b_readDE->dataByteCnt){
                bytesRead = b_readDE->dataByteCnt%vcb->sizeOfBlocks;
            }
            else{
                bytesRead = blocks* vcb->sizeOfBlocks;
            }
      
            part3 = bytesRead;
            part2 = part2 - part3;  //part 2 is now < B_CHUNK_SIZE, or file is exusted
        }

        //try to read B_CHUNK_SIZE bytes into our buffer
        if(openFileTables[fd].lastRead != 1){
            blockToReadFrom = getLba(b_readDE,openFileTables[fd].extOffset);
            blocksRead = LBAread(openFileTables[fd].buffer,1,blockToReadFrom);  //keep as 1 block for now
            openFileTables[fd].bytesWR += blocksRead* vcb->sizeOfBlocks;

            if(openFileTables[fd].bytesWR > b_readDE->dataByteCnt){
                bytesRead = b_readDE->dataByteCnt%vcb->sizeOfBlocks;
                openFileTables[fd].lastRead = 1;  //indicates last read
            }
            else{
                bytesRead = blocksRead* vcb->sizeOfBlocks;
            }
            openFileTables[fd].ourBufferOffset = 0;
            openFileTables[fd].extOffset++;
            openFileTables[fd].bytesInBuffer = bytesRead; //how many bytes are actually in bufferfer
        }
        else if(openFileTables[fd].lastRead == 1){  // error handling here...  if read fails
            openFileTables[fd].bytesInBuffer = 0;
            bytesRead = 0;
        }


       

        

        if (bytesRead < part2) // not even enough left to satisfy read
            part2 = bytesRead;

        if (part2 > 0)	// memcpy bytesRead
        {
            memcpy (buffer+part1+part3, openFileTables[fd].buffer +openFileTables[fd].ourBufferOffset, part2);
            openFileTables[fd].ourBufferOffset = openFileTables[fd].ourBufferOffset + part2;
        }

    }
    free(b_readDE);
    b_readDE = NULL;
    bytesReturned = part1 + part2 + part3;
    return (bytesReturned);
}


int b_write (int fd, char * buffer, int count)
{
    if((openFileTables[fd].flags & O_ACCMODE) == O_RDONLY){
        printf("You do not have permission to write to this file\n");
        return -1;
    }



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
    retVal = LBAread(b_writeDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex);

	if(count >= B_CHUNK_SIZE){
		if(openFileTables[fd].bytesInBuffer == 0)  //if there are no bytes in our buffer
		{
			while(count >= bytesWritten + B_CHUNK_SIZE){


                if(openFileTables[fd].pLoc == -2){ //if this is a clean file
                    openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].extOffset);
                    b_writeDE->dataIndex = openFileTables[fd].pLoc;  //record that initial position of the extent in the data index
                    blockToWriteTo = openFileTables[fd].pLoc;

                } else{
                    blockToWriteTo = getLba(b_writeDE,openFileTables[fd].extOffset);
                }

                blocksDirectlyWritten = LBAwrite(buffer + callerBufferOffset,1,blockToWriteTo);

                openFileTables[fd].extOffset++;         //increment our extent offset

                b_writeDE->dataBlkCnt++;                //increment the data size by 1 block

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

            if(openFileTables[fd].pLoc == -2){ //if this is a clean file
                openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].extOffset);
                b_writeDE->dataIndex = openFileTables[fd].pLoc;  //record that initial position of the extent in the data index
                blockToWriteTo = openFileTables[fd].pLoc;
            } else{
                blockToWriteTo = getLba(b_writeDE,openFileTables[fd].extOffset);
            }

            blocksDirectlyWritten = LBAwrite(openFileTables[fd].buffer,1,blockToWriteTo);  //Write 1 block to disk

            openFileTables[fd].extOffset++;     //increment our extent offset
            b_writeDE->dataBlkCnt++;                //increment the data size by 1 block
            b_writeDE->dataByteCnt += vcb->sizeOfBlocks;

            int bytesNeeded = count - callerBufferOffset;  //decrement count by bytes copied into our buffer and store in bytesNeeded
            openFileTables[fd].bytesInBuffer = 0;  // reset our buffer
            openFileTables[fd].ourBufferOffset = 0;
            while(bytesNeeded >= bytesWritten + B_CHUNK_SIZE){  //if there is more than B_CHUNK_SIZE bytes needed to be written

                if(openFileTables[fd].pLoc == -2){ //if this is a clean file
                    openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].extOffset);
                    b_writeDE->dataIndex = openFileTables[fd].pLoc;  //record that initial position of the extent in the data index
                    blockToWriteTo = openFileTables[fd].pLoc;
                } else{
                    blockToWriteTo = getLba(b_writeDE,openFileTables[fd].extOffset);
                }

                blocksDirectlyWritten = LBAwrite(buffer + callerBufferOffset,1,blockToWriteTo);  //write B_CHUNK_SIZE bytes directly

                openFileTables[fd].extOffset++;     //increment our extent offset
                b_writeDE->dataBlkCnt++;                //increment the data size by 1 block
                b_writeDE->dataByteCnt += vcb->sizeOfBlocks;

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
            bytesWritten += count;
        }else{  //if bytes requested will overflow buffer

            if(openFileTables[fd].pLoc == -2){ //if this is a clean file
                openFileTables[fd].pLoc = getLba(b_writeDE,openFileTables[fd].extOffset);
                b_writeDE->dataIndex = openFileTables[fd].pLoc;  //record that initial position of the extent in the data index
                blockToWriteTo = openFileTables[fd].pLoc;
            } else{
                blockToWriteTo = getLba(b_writeDE,openFileTables[fd].extOffset);
            }

            memcpy(openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, buffer, B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer); //Copy bytes from caller that we can
            callerBufferOffset = B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer;  //Offset caller buffer by amount of bytes already copied into our buffer
            openFileTables[fd].bytesInBuffer = B_CHUNK_SIZE;  //buffer is full
            openFileTables[fd].ourBufferOffset = B_CHUNK_SIZE;
            bytesWritten += B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer;

            blocksDirectlyWritten = LBAwrite(openFileTables[fd].buffer, 1, blockToWriteTo);  //Write full buffer to file

            openFileTables[fd].extOffset++;     //increment our extent offset
            b_writeDE->dataBlkCnt++;                //increment the data size by 1 block
            b_writeDE->dataByteCnt += vcb->sizeOfBlocks;

            openFileTables[fd].bytesInBuffer = 0;  //Buffer is now empty
            openFileTables[fd].ourBufferOffset = 0; //Reset our offset
            memcpy(openFileTables[fd].buffer, buffer + callerBufferOffset, count - callerBufferOffset);  // copy remaining bytes into buffer
            openFileTables[fd].bytesInBuffer += count - callerBufferOffset; // increment our byte count
            openFileTables[fd].ourBufferOffset += count - callerBufferOffset; //increment our offset
            bytesWritten += count - callerBufferOffset;
        }
    }
    retVal = LBAwrite(b_writeDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex); //save modified directory entry to disk
    free(b_writeDE);
    b_writeDE = NULL;
    return bytesWritten;
}

/*int b_seek (int fd, off_t offset, int whence){

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
    int blocksRead = 0;
    int bytesWritten = 0;
    int blocksDirectlyWritten = 0;
    int callerBufferOffset = 0;
    int blockToWriteTo = 0;

    dirEnt* b_readDE = malloc(toBlockSize(sizeof(dirEnt)));
    //check if dirty bit or not test
    if(0 < openFileTables[fd].bytesInBuffer < B_CHUNK_SIZE){
        memcpy(openFileTables[fd].buffer + openFileTables[fd].ourBufferOffset, offset, B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer);  //copy what we can to our buffer
        bytesWritten = B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer;
        openFileTables[fd].ourBufferOffset += B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer; //increment our buffer offset
        openFileTables[fd].bytesInBuffer += B_CHUNK_SIZE - openFileTables[fd].bytesInBuffer; // increment our byte count
        callerBufferOffset = bytesWritten;  //Offset caller buffer by amount of bytes already copied into our buffer
        blocksDirectlyWritten = LBAwrite(openFileTables[fd].buffer,1,blockToWriteTo);  //Write 1 block to disk

//        openFileTables[fd].extOffset++;     //increment our extent offset
//        b_writeDE->dataBlkCnt++;                //increment the data size by 1 block

        int bytesNeeded = B_CHUNK_SIZE - openFileTables[fd].ourBufferOffset;  //decrement count by bytes copied into our buffer and store in bytesNeeded
        openFileTables[fd].bytesInBuffer = 0;  // reset our buffer
        openFileTables[fd].ourBufferOffset = 0;
    }


}*/

void b_close (int fd){
    //closePartitionSystem?
    //free everything associated with fd?
    dirEnt* b_closeDE = malloc(toBlockSize(sizeof(dirEnt)));  //for saving modified data to opened directory entry
    int blockToWriteTo;

    int retVal = LBAread(b_closeDE, vcb->deBlkCnt, openFileTables[fd].dirEntIndex);

    if(openFileTables[fd].flags & O_WRONLY){
        if(openFileTables[fd].bytesInBuffer > 0){ // if there are bytes remaining in the buffer
    if(openFileTables[fd].pLoc == -2){ //if this is a clean file
        openFileTables[fd].pLoc = getLba(b_closeDE,openFileTables[fd].extOffset);
        b_closeDE->dataIndex = openFileTables[fd].pLoc;  //record that initial position of the extent in the data index
        blockToWriteTo = openFileTables[fd].pLoc;
    } else{
        blockToWriteTo = getLba(b_closeDE,openFileTables[fd].extOffset);
    }
        retVal = LBAwrite(openFileTables[fd].buffer, B_CHUNK_SIZE/vcb->sizeOfBlocks, blockToWriteTo);  //write rest of buffer to disk

        openFileTables[fd].extOffset++;     //increment our extent offset
        b_closeDE->dataBlkCnt++;                //increment the data size by 1 block
        b_closeDE->dataByteCnt += openFileTables[fd].bytesInBuffer;

        retVal = LBAwrite(b_closeDE, vcb->deBlkCnt, b_closeDE->loc);  //save modified directory entry to disk
    }
    }


    free(b_closeDE);
    b_closeDE = NULL;
    free(openFileTables[fd].buffer);
    openFileTables[fd].buffer = NULL;
    openFileTables[fd].extOffset = 0;
    openFileTables[fd].dirEntIndex = -1;
    openFileTables[fd].pLoc = -1;
    openFileTables[fd].bytesInBuffer = 0;
    openFileTables[fd].ourBufferOffset = 0;
    openFileTables[fd].bytesWR = 0;
    openFileTables[fd].flags = 0;
    openFileTables[fd].lastRead = 0;
}
