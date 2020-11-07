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
#include <stdbool.h>

#ifndef HASHTABLE_H
#define HASHTABLE_H

#define TABLE_SIZE 54 //maximum size to keep dirEnt under 512 bytes

unsigned int hash(char* dirEntName);

//find word in table
int hash_table_lookup(char* dirEntName, dir* d);

bool hash_table_insert(dirEnt* dE, dir* d);

int hash_table_find_free(dir* d);

/*int dir_ent_lookup(char* token, dir* d);*/


/*void process_top_ten();*/
#endif