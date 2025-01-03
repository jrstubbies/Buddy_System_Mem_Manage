# Buddy_System_Mem_Manage

A C++ program to implement the memory management buddy system strategy. This program
achieves the same effects as C Standard Library's malloc() and free() by allocating the 
smallest block of memory possible, and combining unused memory blocks using the buddy system.

/////////////////////////////////////////////////////////////////////////////////

NOTE:

    The code provided in 'main.cpp' and 'auxiliary.cpp' was produced by Napoleon Reyes 
    (see relevant files for further references)

    Code in the 'buddysys.cpp' file is produced by myself    

/////////////////////////////////////////////////////////////////////////////////


OVERVIEW:

    The given memory block is treated as a doubly linked list with nodes representing
    different sizes of memory. This allows the use of pointers and pointer arithmetic to
    locate memory addresses of the buddies. A 'free list' is constructed using an array of doubly
    linked lists. This tracks the amount of certain sized memory blocks.

    When an allocation of memory is required the program will attempt to find the smallest 
    block of memory available. If there is no elgible memory of this size then a bigger memory
    block will be split into two buddies (this repeats until either cant accomodate the request, or the perfect
    block size if found).

    Memory is freed based on the buddy system. If two buddies are not being used, then they can combine
    into a larger memory block size (this process cascades up if this new block has its buddy free as well).
    Buddies are found using memory addresses and block sizes. For example:
        - A 512 byte block is split into two. The result is two 256 byte blocks. Buddy 1 would occupy address
        0 - 255, while Buddy 2 would occupy the remaining addresses 256 - 511.
        - If both Buddy 1 and 2 are free then they can be combined into a bigger 512 byte block 