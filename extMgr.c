#include "freeMgr.h"
#include "fsInit.h"
#include "dirMgr.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program

ext getNextExt (dirEnt* file){
    ext nextExt;

    int count = 50; // how many blocks to add to each extent
    int lbaPos = findFreeBlocks(count);
    setFreeBlocks(lbaPos, count);

    nextExt.lba = lbaPos;
    nextExt.count = count;

    if (file->ext1.count == 0){
        // add ext to ext1
        printf("ADD EXT1\n");
        printf("%d\n", lbaPos);

        file->ext1.lba = lbaPos;
        file->ext1.count = count;
    }
    else if (file->ext2.count == 0){
        // add ext to ext2
        printf("ADD EXT2\n");
        printf("%d\n", lbaPos);

        file->ext2.lba = lbaPos;
        file->ext2.count = count;
    }
    else if (file->ext3.count == 0){
        // add ext to ext3
        printf("ADD EXT3\n");

        printf("%d\n", lbaPos);

        file->ext3.lba = lbaPos;
        file->ext3.count = count;
    }
    else if (file->ext4.count == 0){
        // add ext to ext4
        printf("ADD EXT4\n");

        printf("%d\n", lbaPos);

        file->ext4.lba = lbaPos;
        file->ext4.count = count;
    }
    else { // if primary extents ar filled
        // iterate through secondary extents
        int index;
        for (index = 0; index <= 64; index++){
            if (file->dExt[index].count == 0) {
                file->dExt[index].lba = lbaPos;
                file->dExt[index].count = count;
                printf("ADD EXT%d\n%d\n", index, lbaPos);
                if (index==64) printf("no more ext left, add tertiary extents\n");
            }
        }

    }

    return nextExt;
}

void deleteExts (dirEnt* file){ // call when a file is deleted to set the extents lba blocks to be free
    if (file->ext1.count > 0){
        clearFreeBlocks(file->ext1.lba, file->ext1.count);
        printf("deleted ext1\n");
    }
    if (file->ext2.count > 0){
        clearFreeBlocks(file->ext2.lba, file->ext2.count);
        printf("deleted ext2\n");
    }
    if (file->ext3.count > 0){
        clearFreeBlocks(file->ext3.lba, file->ext3.count);
    }
    if (file->ext4.count > 0){
        clearFreeBlocks(file->ext4.lba, file->ext4.count);
    }
}