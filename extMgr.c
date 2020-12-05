#include "freeMgr.h"
#include "fsInit.h"
#include "dirMgr.h"

extern unsigned int* freeSpaceBitmap;  //global
extern vCB* vcb;  //global

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
    int count = 1;

    if (file->ext1.count == 0){
        // add ext to ext1
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;

        file->ext1.lba = lbaPos;
        file->ext1.count = count;

    }
    else if (file->ext2.count == 0){
        // add ext to ext2
        count = count * 2;
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;

        file->ext2.lba = lbaPos;
        file->ext2.count = count;

    }
    else if (file->ext3.count == 0){
        // add ext to ext3
        count = count * 4;
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;
        file->ext3.lba = lbaPos;
        file->ext3.count = count;

    }
    else if (file->ext4.count == 0){
        // add ext to ext4
        count = count * 8;
        int lbaPos = findFreeBlocks(count);
        setFreeBlocks(lbaPos, count);

        nextExt.lba = lbaPos;
        nextExt.count = count;
        file->ext4.lba = lbaPos;
        file->ext4.count = count;
    }
    else { 
        int index = 0;
        count = 16;
        while (index<MAX_SEC_EXTENTS){
            if (file->dExt[index].count == 0) break;
            count = count * 2;
            index++;
        }
        if (index==MAX_SEC_EXTENTS) { // if invalid extent
            printf("Ran out of extents\n");
            ext nullExt;
            return nullExt;
        } else if(index > 0 && file->dExt[index-1].lba + file->dExt[index-1].count >= vcb->blockCount){
            printf("Ran out of space for file\n");
            ext nullExt;
            return nullExt;
        }
        else { // add the ext
            int lbaPos = findFreeBlocks(count);
            if (lbaPos+count > vcb->blockCount) { // if extent size is larger than free space, allocate the remaining blocks only
                printf("Ran out of space for file\n");
                ext nullExt;
                return nullExt;
            }
            setFreeBlocks(lbaPos, count);
            file->dExt[index].lba = lbaPos;
            file->dExt[index].count = count;
        }
    }

    return nextExt;
}

int getLba (dirEnt* file, int logicalAddress){ // gets the lba block within a file from its logical address, adds extents if needed
    int currentAddress = logicalAddress; // currentAddress is the blocks left based on current extent
    int lba = -1; // the return value, desired lba
    int extNum = 1; // the extent we are currently traversing
    while (currentAddress>=0){ // finding which extent contains the logical address
        if (extNum==1) {
            if (file->ext1.count==0) getNextExt(file); // get a new extent if not allocated
            if (currentAddress < file->ext1.count) lba = file->ext1.lba + currentAddress; // if block if in this extent, calculate its lba
            currentAddress -= file->ext1.count; // decrement the blocks left, since we checked this extent
        }
        else if (extNum==2) {
            if (file->ext2.count==0) getNextExt(file); // get a new extent if not allocated
            if (currentAddress < file->ext2.count) lba = file->ext2.lba + currentAddress;
            currentAddress -= file->ext2.count;
        }
        else if (extNum==3) {
            if (file->ext3.count==0) getNextExt(file); // get a new extent if not allocated
            if (currentAddress < file->ext3.count) lba = file->ext3.lba + currentAddress;
            currentAddress -= file->ext3.count;
        }
        else if (extNum==4) {
            if (file->ext4.count==0) getNextExt(file); // get a new extent if not allocated
            if (currentAddress < file->ext4.count) lba = file->ext4.lba + currentAddress;
            currentAddress -= file->ext4.count;
        }
        else {
            int i = extNum - 5; // index in dExt array
            if (file->dExt[i].count==0) getNextExt(file); // get a new extent if not allocated
            if (currentAddress < file->dExt[i].count) lba = file->dExt[i].lba + currentAddress;
            currentAddress -= file->dExt[i].count;
        }
        if (currentAddress>=0) extNum++; // go on to next extent if blocks are still left
    }
    return lba;
}

void deleteExts (dirEnt* file){ // call when a file is deleted to set the extents lba blocks to be free
    if (file->ext1.count > 0){
        clearFreeBlocks(file->ext1.lba, file->ext1.count);
        file->ext1.lba = 0;
        file->ext1.count = 0;
    }
    if (file->ext2.count > 0){
        clearFreeBlocks(file->ext2.lba, file->ext2.count);
        file->ext2.lba = 0;
        file->ext2.count = 0;
    }
    if (file->ext3.count > 0){
        clearFreeBlocks(file->ext3.lba, file->ext3.count);
        file->ext3.lba = 0;
        file->ext3.count = 0;
    }
    if (file->ext4.count > 0){
        clearFreeBlocks(file->ext4.lba, file->ext4.count);
        file->ext4.lba = 0;
        file->ext4.count = 0;
    }

    int index = 0;
    while (index<MAX_SEC_EXTENTS){
        if (file->dExt[index].count == 0) break;
        clearFreeBlocks(file->dExt[index].lba, file->dExt[index].count);
        file->dExt[index].lba = 0;
        file->dExt[index].count = 0;
        index++;
    }
}