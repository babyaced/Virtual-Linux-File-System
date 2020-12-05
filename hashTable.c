#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <ctype.h>
#include "dirMgr.h"

#include "hashTable.h"
#include "fsLow.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program
extern int currentBlock;
extern int currentBlockSize;
extern char currentBlockName[255]; //size of 255 for now

unsigned int hash(char* dirEntName) {   
	unsigned int hash_value = 0;
	for (int i = 0; i < strlen(dirEntName); i++) { //Hash Function
		hash_value += dirEntName[i];
		hash_value = (hash_value * dirEntName[i]) % TABLE_SIZE;
	}
	return hash_value;
}

//find dirEnt in hash_table
int hash_table_lookup(char* dirEntName, dir* d) { //pass by value or pass by reference?
	int retVal;
	dirEnt* htlDE = malloc(toBlockSize(sizeof(dirEnt)));
	int index = hash(dirEntName);
	for(int i = 0; i < TABLE_SIZE; i++){
		int try = (index + i) % TABLE_SIZE;
		if(d->dirEnts[try] != -1){
			retVal = LBAread(htlDE, vcb->deBlkCnt, d->dirEnts[try]);
			//we can assume . and .. will be at directly hashed index because they were first dirEnts added + "." and ".." return different hashes
			if(strncmp(dirEntName,htlDE->name,256) == 0 || strncmp(dirEntName,".",2) == 0 || strncmp(dirEntName,"..",3) == 0 )
			{
				free(htlDE);
				htlDE = NULL;
				return d->dirEnts[try];
			}	
				
		}
	}
	free(htlDE);
	htlDE = NULL;
	return -1;
}

bool hash_table_insert(dir* d, dirEnt* dE) { //pass by value or pass by reference?
	if (dE == NULL) return false;
    int index = hash(dE->name);
	for(int i = 0; i < TABLE_SIZE; i++){
		int try = (i+index) % TABLE_SIZE;
		if(d->dirEnts[try] == -1){
			d->dirEnts[try] = dE->loc;
			return true;
		}
	}
	return false;
}

bool hash_table_delete(dir* d, dirEnt* dE){
	int retVal;
	dirEnt* htdDE = malloc(toBlockSize(sizeof(dirEnt)));
	int index = hash(dE->name);
	for(int i = 0; i < TABLE_SIZE; i++){
		int try = (index + i) % TABLE_SIZE;
		if(d->dirEnts[try] != -1){
			retVal = LBAread(htdDE, vcb->deBlkCnt, d->dirEnts[try]);
			if(strncmp(dE->name,htdDE->name,256) == 0){
				d->dirEnts[try] =  -1;
				free(htdDE);
				htdDE = NULL;
				return true;
			}
		}
	}
	free(htdDE);
	htdDE = NULL;
	
	return false;
}



