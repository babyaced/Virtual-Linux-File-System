#include "freeMgr.h"
#include "fsInit.h"
#include "dirMgr.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program

ext getNextExt (dirEnt* file){
    ext nextExt;

    int count = 20; // how many blocks to add to each extent
    int lbaPos = findFreeBlocks(count);

    nextExt.lba = lbaPos;
    nextExt.count = count;

    if (file->ext1.lba == -1){
        // add ext to ext1
        printf("ADD the EXT\n");
    }
    if (file->ext2.lba == -1){
        // add ext to ext2
    }
    if (file->ext3.lba == -1){
        // add ext to ext3
    }
    if (file->ext4.lba == -1){
        // add ext to ext4
    }

    return nextExt;
}

void deleteExts (dirEnt* file){ // call when a file is deleted to set the extents lba blocks to be free
    if (file->ext1.lba > -1){
        clearFreeBlocks(file->ext1.lba, file->ext1.count);
    }
    if (file->ext2.lba > -1){
        clearFreeBlocks(file->ext2.lba, file->ext2.count);
    }
    if (file->ext3.lba > -1){
        clearFreeBlocks(file->ext3.lba, file->ext3.count);
    }
    if (file->ext4.lba > -1){
        clearFreeBlocks(file->ext4.lba, file->ext4.count);
    }
}