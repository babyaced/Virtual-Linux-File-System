#ifndef _FREE_MGR_H
#define _FREE_MGR_H
#include "fsInit.h"


int findFreeBlocks(vCB* vcb,fSL* fsl,uint64_t blocksNeeded);
void setFreeBlocks(vCB* vcb,fSL* fsl,int startingIndex,int count);

#endif 