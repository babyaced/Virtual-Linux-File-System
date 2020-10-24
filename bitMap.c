#include<stdio.h>

void setBit(unsigned int* array, int bitToSet){
    int index = bitToSet/31;
    int bitPos = bitToSet%31;
    
    unsigned int inUse = 1;
    inUse = inUse << bitPos;
    array[index] = array[index] | inUse;
}

void clearBit(unsigned int* array, int bitToClear){
    int index = bitToClear/32;
    int bitPos = bitToClear%32;
    
    unsigned int inUse = 0;
    inUse = inUse << bitPos;
    array[index] = array[index] | inUse;
}

int checkBit(unsigned int* array, int bitToCheck){
    int index = bitToCheck/32;
    int bitPos = bitToCheck%32;

    unsigned int inUse = 1;
    inUse = inUse << bitPos;
    if(array[index] & inUse){
        return 1;
    }else{
        return 0;
    }
}

void initBM(unsigned int* array, int bmSize){
    for(int i = 0; i < bmSize; i++)
    {
        clearBit(array,i);
    }
}

void printBits(int n){
    if (n > 1) 
      printBits(n>>1);
      
    printf("%d ", n & 1); 
} 

void printBM(unsigned int* array, int bmSize){
    for(int i = 0; i < bmSize; i++)
    {
        if(i%31 == 0 && i != 0)
            printf("Array Element: %d\n",array[i%31]);
    }
}

