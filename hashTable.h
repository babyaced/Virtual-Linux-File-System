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