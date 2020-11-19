#include "freeMgr.h"
#include "fsInit.h"
#include "dirMgr.h"

extern unsigned int* freeSpaceBitmap;  //global for whole damn program
extern vCB* vcb;  //global for whole damn program

void initExts (dirEnt* file, int count){
    file->ext1.count = 0;
    file->ext2.count = 0;
    file->ext3.count = 0;
    file->ext4.count = 0;
    int i;
    for (i=0; i<count; i++)
        file->dExt[i].count = 0;
}

ext getNextExt (dirEnt* file){
    ext nextExt;
    int count = 10; // how many blocks for first extent, doubles each time
//    int lbaPos = findFreeBlocks(count);
//    setFreeBlocks(lbaPos, count);

//    nextExt.lba = lbaPos;
//    nextExt.count = count;

    if (file->ext1.count == 0){
        // add ext to ext1
        printf("ADD EXT1\n");
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;

        file->ext1.lba = lbaPos;
        file->ext1.count = count;
        printf("lba = %d\n", lbaPos);

    }
    else if (file->ext2.count == 0){
        // add ext to ext2
        printf("ADD EXT2\n");
        count = count * 2;
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;

        file->ext2.lba = lbaPos;
        file->ext2.count = count;
        printf("lba = %d\n", lbaPos);

    }
    else if (file->ext3.count == 0){
        // add ext to ext3
        printf("ADD EXT3\n");
        count = count * 4;
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;
        file->ext3.lba = lbaPos;
        file->ext3.count = count;
        printf("lba = %d\n", lbaPos);

    }
    else if (file->ext4.count == 0){
        // add ext to ext4
        printf("ADD EXT4\n");
        count = count * 8;
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;
        file->ext4.lba = lbaPos;
        file->ext4.count = count;
        printf("lba = %d\n", lbaPos);

    }
    else { // if primary extents ar filled
        // iterate through secondary extents
//        int index;
//        for (index = 0; index <= 64; index++){
//            if (file->dExt[index].count == 0) {
//                file->dExt[index].lba = lbaPos;
//                file->dExt[index].count = count;
//                printf("ADD EXT%d\n%d\n", index, lbaPos);
//                if (index==64) printf("no more ext left, add tertiary extents\n");
//            }
//        }
        count = count * 16; // starting at 5th ext, size is count*2^4
        int index = 0;
        while (index<64){
            if (file->dExt[index].count == 0) break;
            count = count * 2;
            index++;
        }
        if (index==64) { // if invalid extent
            printf("Ran out of extents\n");
            ext nullExt;
            return nullExt;
        } else if(index > 0 && file->dExt[index-1].lba + count/2 + count > 20000){
            printf("Ran out of space for file\n");
            ext nullExt;
            return nullExt;
        }
        else { // add the ext
            int lbaPos = findFreeBlocks(count);
            setFreeBlocks(lbaPos, count);
            file->dExt[index].lba = lbaPos;
            file->dExt[index].count = count;
            printf("ADD dEXT%d, actually EXT%d\n", index, index+5);
            printf("lba = %d\n", lbaPos);
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
        printf("deleted ext3\n");
    }
    if (file->ext4.count > 0){
        clearFreeBlocks(file->ext4.lba, file->ext4.count);
        printf("deleted ext4\n");
    }

    int index = 0;
    while (index<64){
        if (file->dExt[index].count == 0) break;
        clearFreeBlocks(file->dExt[index].lba, file->dExt[index].count);
        printf("deleted dEXT%d, actually EXT%d\n", index, index+5);
        index++;
    }
}