
#include "buddysys.h"
#include <iostream>
#include <cmath>
#include <vector>

int minK, maxK;         // gives k value range for the free table, can also use 'minK' to find a free table index
vector<Node*> freelist; // used vector as was easier to 
Node *startaddr;  

/////////////////////////////////////////////////////////////////////////////////

// Helper function. Will find the smallest block size the argument fits in to, and returns the associated k value (exponent) 
int findKValue(long long int memsize) {
    return static_cast<int>(ceil(log2(memsize)));
}


// Function to debug the free list with some helpful data. Loop through each index printing details for any Nodes.
void debugFreeList() {
    cout << "\n\n****************** Debugging Free List ******************" << endl;
    cout << "This free list has: " << freelist.size() << " rows." <<endl;
    
    for(int i = 0; i < freelist.size(); ++i) {
        cout << "\nFreeTable index:  " << i << ",  Associated K value:  " << i + minK << ",  Block Size:  " << (pow(2, i + minK)) << endl;
        Node *node = freelist[i];

        // see if this freelist index has any node/block associated with it.
        if(!node) {
            cout << "\t - This index has no free blocks" << endl;
        } else {
            int count = 0;
            while(node) {
                cout << "- Node: " << count << endl;
                cout << "\tAddress is " << node << " with size (excluding header size) of: " << node->size << endl;
                cout << "\tnode -> next: " << node->next << "   and node -> prev is " << node->previous << endl; 
                
                node = node->next;      // traverse nodes
                count++;
            }
        }
    }

    cout << "\n===================== Finished debugging Free List =====================" << endl;
}


// Initialise the freelist once. Find the max and min block size k values to get the range of the freelist 'k' values.
void initFreeList(){
    minK = findKValue((long long int)sizeof(Node)) + 1; // Set the min k value to be one order higher than nodes header size.
    maxK = findKValue(wholememory->size);               // Set max k value to cover the USABLE data size

    int listsize = maxK - minK + 1;       // freelist only needs to account for values in the min -> max block size range, +1 due to 0 indexing
    freelist.resize(listsize, nullptr);   // update the size of freelist and set values to nullptrs to begin with.

    // Set the largest block size, and store pointer to the starting address.
    // freelist indexes = 0, 1, 2, 3,  4, 5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    // freelist k value = 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
    freelist[maxK - minK] = wholememory;
    startaddr = wholememory;
    
    // cout << "\n[DEBUGGING]     - Startaddr is set to:    " << startaddr << endl;
}



// Malloc function to allocate a space in memory for a given data size. Returns pointer to address of the DATA SECTION.
void *buddyMalloc(int req_mem){
    
    // 'n' is the total space needed and includes the header AND data size.
    long long int n = req_mem + sizeof(Node);     

    // check if memory required is bigger than the total memory. If it is then not enough soace to allocate, so return NULL
    // using the '=' as MEMSIZE = x, but the address space is x - 1, due to 0 indexing
    if(n >= MEMORYSIZE) {
        return NULL;
    }

    // find the smallest k value that can accomodate the total size ('n'). Find the index assoiated with this K value.
    int reqK = findKValue(n);
    int kIndex = reqK - minK;     // eg. reqK is 10 =>   10 - 6   = 4. Thus freetable[4] has k value of 10


// ----------------------------------------------------------   SPLITTING OF BLOCKS  ---------------------------------------------------------- //

    // if there is NOT a block already available then need split up a larger block
    if(!freelist[kIndex]) {
    
        // if no blocks at minimum size, then loop to try find an available larger block to split up (work up the list)
        int nextKIndex = kIndex + 1;        
        while(nextKIndex < freelist.size() && freelist[nextKIndex] == nullptr){
            nextKIndex++;
        }

        // if no blocks to split are available within the freelist then return NULL, CANNOT complete this allocation
        if(nextKIndex >= freelist.size()) {
            return NULL;
        }

        // split larger block size into 2 equal smaller block sizes
        while(nextKIndex > kIndex) {
            
            // set the node are interested in, and then update any connections to maintain link list connections.
            Node *currBlock = freelist[nextKIndex];     
            freelist[nextKIndex] = currBlock->next;   // either NULL, or another node

            if(currBlock->next) {
                currBlock->next->previous = nullptr;   // the next block will become the head of the list
            }

            // Currently on block size 'k', but are wanting block sizes of 'k - 1'.
            long long int newBlockSize = (long long int)(pow(2, nextKIndex - 1 + minK));
            
            // Create two buddy blocks that will be linked together, and side-by-side in memory.
            Node *buddy1 = currBlock;
            buddy1->size = (long long int)((long long int)newBlockSize - (long long int)sizeof(Node));
            buddy1->alloc = 0;              // not allocated yet
            buddy1->previous = nullptr;     // have detached from link list so set to null for now

            // create a new buddy node with an address '2^k-1' places after 'buddy1'
            Node *buddy2 = ((Node *)((uintptr_t)buddy1 + (uintptr_t)newBlockSize));
            buddy2->size = buddy1->size;
            buddy2->alloc = 0;              // not allocated yet
            buddy2->next = nullptr;         // currently doesnt need to point to anything 
            buddy2->previous = buddy1;      // buddy2 address comes after buddy1

            buddy1->next = buddy2;          // buddy1 address is before buddy2
            
            
            // Now need to work way back down the list, splitting blocks util have the minimum size required 
            nextKIndex--;

            // update pointers to add the buddy1 and buddy2 to the free list, putting at the HEAD of the list
            buddy2->next = freelist[nextKIndex];        // buddy2 points to whatever was at the head of the list
            if(freelist[nextKIndex]) {
                freelist[nextKIndex]->previous = buddy2; // if there was a node it now points to buddy2 (buddy is now in front of it)
            }
            freelist[nextKIndex] = buddy1;      // finally update freelist to point to buddy1 (now head of the list)
        }
    }

    // if exits here then something went wrong with splitting, so return NULL
    if(!freelist[kIndex]) { 
        return NULL;
    }


// ----------------------------------------------------------   ALLOCATING FREE BLOCK  ---------------------------------------------------------- //

    // At this point freelist should now have the minimum block size available. Create a pointer to this block
    Node *allocatedBlock = freelist[kIndex];
    long long int currBlockSize = (long long int)(pow(2, reqK));

    // are removing node from front of list. If there is a node after then it becomes the new head.
    if(allocatedBlock->next){
        allocatedBlock->next->previous = nullptr;
    }
    freelist[kIndex] = allocatedBlock->next;
    
    // Mark this block as allocated. Size is the size of the data section only. Ensure no lingering pointers to free list.
    allocatedBlock->alloc = 1;
    allocatedBlock->size = (long long int)(currBlockSize - (long long int)sizeof(Node));   // size of the data section
    allocatedBlock->next = nullptr;                
    allocatedBlock->previous = nullptr;            
    
    // return pointer to DATA SECTION of the node
    return (void *)((Node *)((uintptr_t)allocatedBlock + (uintptr_t)sizeof(Node)));  
} 



// Takes in a pointer to the address of the DATA SECTION. Will need to calc the BASE address to use for the free list.
void buddyFree(void *p){
    
    // check that p actually contains something. If not don't do anything and return.
    if (!p) {
        return;
    }

    // this points to the BASE address of the block to be freed.
    Node *block = (Node*)((uintptr_t)p - (uintptr_t)sizeof(Node));

    // get the TOTAL size of the block associated with 'p'.   Node -> size only has size of the DATA section.
    long long int currBlockSize = (long long int)block->size + (long long int)sizeof(Node);

    // find the associated 'k' value for this block size, and its index in freelist
    int currK = findKValue(currBlockSize);
    int kIndex = currK - minK;


// ----------------------------------------------------------   COALESCING BLOCKS  ---------------------------------------------------------- //

    // Try and coalesce buddy blocks based on the block to be freed.
    while(true) {
        
        // Calculate the address for the buddy of the block to be freed.
        uintptr_t blockAddr = (uintptr_t)block;
        uintptr_t buddyAddr = (uintptr_t)startaddr + ((blockAddr - (uintptr_t)startaddr) ^ currBlockSize);
        Node *buddy = (Node *)buddyAddr;

        // First check that this address is within the original memory block
        if(buddyAddr < (uintptr_t)startaddr || buddyAddr >= (uintptr_t)startaddr + MEMORYSIZE) {
            break;
        }

        // If the buddy is within the memory block, check if of the same size, and if its been allocated.
        if(buddy->alloc != 0 || buddy->size != block->size) {
            break;
        }
            
        // If buddy block is available to coalesce, then safely remove from the linked list by updating relevant connections.
        if(buddy->next) {
            buddy->next->previous = buddy->previous;     // the node after buddy now links to node before buddy
        }

        if(buddy->previous) {
            buddy->previous->next = buddy->next;    // likewise the node before buddy now links to node after buddy
        }

        if(freelist[kIndex] == buddy) {
            freelist[kIndex] = buddy->next;   // if buddy was head of list, need to point to new head
        }


        // Want to maintain pointer to the block that comes first in memory, update if buddy is before the current block
        if(buddyAddr < blockAddr) {
            block = buddy;
        }

        // update the size of the block to one size higher.  Node->size only accounting for size of the DATA SECTION 
        block->size = ((long long int)(pow(2, currK+1)) - (long long int)(sizeof(Node))); 

         
        // Repeat process as much as can
        currK++;
        kIndex++; 
    }

// ----------------------------------------------------------   ADD BLOCK TO FREE LIST  ---------------------------------------------------------- //

    // Once any coalescing is done, the block is no longer allocated and added to freelist. Update any existing connections
    block->alloc = 0;                   
    block->next = freelist[kIndex];     // adding block to front of the list
    if(freelist[kIndex]) {
        freelist[kIndex]->previous = block;
    }
    block->previous = nullptr;
    freelist[kIndex] = block;
}
