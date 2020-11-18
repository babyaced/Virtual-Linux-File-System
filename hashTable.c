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

//#define TABLE_SIZE 54 //maximum size to keep dirEnt under 512 bytes

unsigned int hash(char* dirEntName) {   
	unsigned int hash_value = 0;
	for (int i = 0; i < strlen(dirEntName); i++) { //Hash Function
		hash_value += dirEntName[i];
		hash_value = (hash_value * dirEntName[i]) % TABLE_SIZE;
	}
	return hash_value;
}

//find word in table
int hash_table_lookup(char* dirEntName, dir* d) { //pass by value or pass by reference?
	int retVal;
	dirEnt* htlDE = malloc(toBlockSize(sizeof(dirEnt)));
	int index = hash(dirEntName);
	for(int i = 0; i < TABLE_SIZE; i++){
		int try = (index + i) % TABLE_SIZE;
		if(d->dirEnts[try] != -1){
			retVal = LBAread(htlDE, vcb->rdBlkCnt, d->dirEnts[try]);
			if(strncmp(dirEntName,htlDE->name,256) == 0)
				return d->dirEnts[try];
		}
	}
	free(htlDE);
	htlDE = NULL;
	return -1;
}

bool hash_table_insert(dirEnt* dE, dir* d) { //pass by value or pass by reference?
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

bool hash_table_delete(dirEnt* dE, dir* d){
	int retVal;
	dirEnt* htdDE = malloc(toBlockSize(sizeof(dirEnt)));
	int index = hash(dE->name);
	for(int i = 0; i < TABLE_SIZE; i++){
		int try = (index + i) % TABLE_SIZE;
		if(d->dirEnts[try] != -1){
			retVal = LBAread(htdDE, vcb->rdBlkCnt, d->dirEnts[try]);
			if(strncmp(dE->name,htdDE->name,256) == 0){
				d->dirEnts[try] =  -1;
				return true;
			}
		}
	}
	return false;
}



