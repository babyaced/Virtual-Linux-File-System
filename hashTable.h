/**************************************************************
* Class:  CSC-415-03 Fall 2020
* Name: Daniel Simpson
* Student ID: 920576134
* GitHub ID: babyaced
* Project: Assignment 4 – Word Blast
*
* File: Simpson_Daniel_HW4_LinkedList.h
*
* Description: This file contains an implementation of a hash
*              table used to store unique words longer than 6
*              characters and their counts. It also contains
*              the implementation of a function that finds 
*			   the 10 most frequent of those words.
**************************************************************/

#ifndef SIMPSON_DANIEL_HW4_HASHTABLE_H
#define SIMPSON_DANIEL_HW4_HASHTABLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <ctype.h>
#include "dirMgr.h"

#define TABLE_SIZE 54 //maximum size to keep dirEnt under 512 bytes

/*unsigned int hash(char* dirEntName) {   
	char* lower = (char*)malloc(sizeof(dirEntName));
	for(int i = 0; dirEntName[i]; i++){
  		lower[i] = tolower(dirEntName[i]);  // Ensures that case is ignored
	}
	unsigned int hash_value = 0;
	for (int i = 0; i < strlen(lower); i++) { //Hash Function
		hash_value += lower[i];
		hash_value = (hash_value * lower[i]) % TABLE_SIZE;
	}
	free(lower);
	lower = NULL;
	return hash_value;
}

//find word in table
dirEnt* hash_table_lookup(char* dirEntName, dir* d) { //pass by value or pass by reference?
	int index = hash(dirEntName);
	dirEnt* tmp = d->dirEnts[index];
	while (tmp != NULL && strcasecmp(tmp->name, dirEntName) != 0) { //while there is a word at tmp and word at tmp is not equal to word
		tmp = tmp->next; //go to next in linked list
	}
	return tmp;
}

bool hash_table_insert(dirEnt* dE, dir* d) { //pass by value or pass by reference?
	if (dE == NULL) return false;
    dirEnt *tmp;
    tmp = hash_table_lookup(dE->name, d);
    if(tmp != NULL){  //If word exists at that pointer 
		//pthread_mutex_lock(&lock);
		//pthread_mutex_unlock(&lock);
        return false;
    }
    else{  // add new entry to hashtable
        int index = hash(dE->name);
	    dE->next = d->dirEnts[index]; 
	    d->dirEnts[index] = dE;
	    return true;
    }
}

int hash_table_find_free(dir* d){
}

/*int dir_ent_lookup(char* token, dir* d){
	int index = hash(token);
	dirEnt* tmp = d->dirEnts[index];
	while (tmp != NULL && strcasecmp(tmp->name, token) != 0) { //while there is a word at tmp and word at tmp is not equal to word
		tmp = tmp->next; //go to next in linked list
	}
	return tmp->fileIndex;
}*/

/*void process_top_ten()
{
	int max = 0;
	int indexOfMax = 0;
	int linksAfterIndex = 0;
	dirEnt* arrayOfTop10[10];
	printf("Printing top 10 words 6 characters or more.\n");
	for(int i = 0; i < 10; i++){ //Until 10 maxes recorded
		max = 0;
		for(int j = 0; j < TABLE_SIZE; j++){  //Traverse through whole hashtable
			if(hash_table[j] != NULL)  //If word is at index j
			{
				dirEnt* tmp = hash_table[j];
				int linksForward = 0;  //To record links traversed
				while(tmp!= NULL){
					if(tmp->count > max){ // if new max found
						max = tmp->count; // record new max
						indexOfMax = j;  //record position
						linksAfterIndex = linksForward; // record position
					}
					linksForward++;
					tmp = tmp->next; //go to next link
				}
			}
		}
		dirEnt* temp = hash_table[indexOfMax];
		for(int k = 0;k< linksAfterIndex; k++){  //find word at index
			temp = temp->next;
		}
		if(temp != NULL && temp->count != 0){ // Print table
			printf("Number %d is %s with a count of %d\n", i+1,temp->word, temp->count);
			temp->count = 0;
		}
	}
}*/
#endif