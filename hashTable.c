#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <ctype.h>
#include "dirMgr.h"

#include "hashTable.h"

#define TABLE_SIZE 54 //maximum size to keep dirEnt under 512 bytes

unsigned int hash(char* dirEntName) {   
	unsigned int hash_value = -1;
	for (int i = 0; i < strlen(dirEntName); i++) { //Hash Function
		hash_value += dirEntName[i];
		hash_value = (hash_value * dirEntName[i]) % TABLE_SIZE;
	}
	return hash_value;
}

//find word in table
int hash_table_lookup(char* dirEntName, dir* d) { //pass by value or pass by reference?
	int index = hash(dirEntName);
	int value = d->dirEnts[index];
	//dirEnt* tmp = d->dirEnts[index];
	//while (tmp != NULL && strcasecmp(tmp->name, dirEntName) != 0) { //while there is a word at tmp and word at tmp is not equal to word
	//	tmp = tmp->next; //go to next in linked list
	//}
	return value;
}

bool hash_table_insert(dirEnt* dE, dir* d) { //pass by value or pass by reference?
	if (dE == NULL) return false;
    int value;
    value = hash_table_lookup(dE->name, d);
    if( value != -1){  //If entry exists at that pointer 
		//add to linked chain at that pointer
        return false;
    }
    else{  // add new entry to hashtable
        int index = hash(dE->name);
	    //dE->next = d->dirEnts[index]; //
	    d->dirEnts[index] = dE->loc;  //add LBAindex of directory entry to directory's directory entries
	    return true;
    }
}

int hash_table_find_free(dir* d){
	return 0;
}



