#ifndef _EXT_MGR_H
#define _EXT_MGR_H
#include "fsInit.h"

#define TABLE_SIZE 54  //maximum size to keep dir Size under 512 bytes

typedef struct ext {
    unsigned int lba;
    unsigned int count;
}ext;

ext getNextExt (dirEnt* file);

#endif