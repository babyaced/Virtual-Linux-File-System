#ifndef _EXT_MGR_H
#define _EXT_MGR_H
#include "fsInit.h"

typedef struct dirEnt dirEnt;

typedef struct ext {
    unsigned int lba;
    unsigned int count;
}ext;

ext getNextExt (dirEnt* file);
void deleteExts (dirEnt* file);
void initExts (dirEnt* file, int count);
#endif