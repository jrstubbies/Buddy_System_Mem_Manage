
#ifndef __BUDDYSYS_H__
#define __BUDDYSYS_H__

#include "auxiliary.h"


extern long long int MEMORYSIZE;
typedef unsigned char byte;         // shorter, replace cast to (char *) with cast to (byte *)



struct llist { long long int size;   //size of the block (ONLY for data, this size does not consider the Node size! (so it is same as s[k])
               int alloc;               //0 is free, 1 means allocated
               struct llist * next;     //next component
               struct llist * previous; //previous component
};


typedef struct llist Node;
extern Node *wholememory;

void initFreeList();                    // function to initialise the free list in 'main.cpp'
void *buddyMalloc(int request_memory); 
void buddyFree(void *p);
void debugFreeList();               // function used to see blocks currently in free table

#endif