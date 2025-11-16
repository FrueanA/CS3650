#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include <malloc.h> 
#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <assert.h>

// note: you should not include <stdlib.h> in your final implementation

#include <debug.h> // definition of debug_printf

// each memory block on the heap includes this metadata structure
// it tracks the size of the block, its next neighbor, and whether it's free
typedef struct block {
    size_t size;          // size of the user data region following this block
    struct block *next;   // pointer to the next block in the linked list
    int free;             // 1 = free, 0 = allocated
} block_t;

#define BLOCK_SIZE sizeof(block_t)

// global variable, head of the linked list tracking all allocated blocks
// there should be exactly one global pointer as per the assignment spec
static block_t *head = NULL;

// helper function, find_free_block
// purpose, traverse the linked list to locate the first free block large enough
// to hold size bytes (first-fit strategy)
// arguments, size is the number of bytes requested
// returns, pointer to the first suitable free block or NULL if none found
block_t *find_free_block(size_t size) {
    assert(size > 0); // sanity check for valid allocation size
    block_t *current = head;

    // traverse the block list until a usable free block is found
    while (current != NULL) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }

    // no suitable free block found
    return NULL;
}

// helper function, add_more_space
// purpose, request new memory from the os using sbrk when no existing
// free block can satisfy the request
// arguments, size is the number of bytes to allocate for the user
// returns, pointer to the new block_t struct created in heap space
block_t *add_more_space(size_t size) {
    // move the program break by the total required space
    void *request = sbrk(BLOCK_SIZE + size);
    if (request == (void *) -1) {
        return NULL; // sbrk failed
    }

    // initialize new block metadata in the newly allocated space
    block_t *block = (block_t *) request;
    block->size = size;
    block->next = NULL;
    block->free = 0;

    return block;
}

// mymalloc
// purpose, allocate a block of memory of size s bytes using a first-fit strategy
// arguments, s is the number of bytes requested
// returns, pointer to usable memory on success or NULL on failure
// notes, prints "malloc %zu bytes\n" for debugging
void *mymalloc(size_t s) {
    debug_printf("Malloc %zu bytes\n", s);

    // reject zero-byte allocations
    if (s == 0) {
        return NULL;
    }

    block_t *block;

    // if this is the first allocation, create the initial block
    if (head == NULL) {
        block = add_more_space(s);
        if (block == NULL) {
            return NULL; // out of memory
        }
        head = block;
    } else {
        // attempt to reuse a free block (first-fit)
        block = find_free_block(s);

        // if no free block fits, request additional space from the os
        if (block == NULL) {
            block_t *current = head;
            while (current->next != NULL) {
                current = current->next;
            }

            block = add_more_space(s);
            if (block == NULL) {
                return NULL;
            }

            // append the new block to the linked list
            current->next = block;

            // invariants for debugging and correctness
            assert(block != NULL);
            assert(block->size >= s);
        } else {
            // mark an existing block as allocated
            block->free = 0;
        }
    }

    // return a pointer to the memory region after the block metadata
    return (block + 1);
}

// mycalloc
// purpose, allocate and zero-initialize an array of nmemb elements of size s
// arguments, nmemb is the number of elements, s is the size of each element in bytes
// returns, pointer to allocated zeroed memory or NULL on failure
// notes, prints "calloc %zu bytes\n" for debugging
void *mycalloc(size_t nmemb, size_t s) {
    size_t total_size = nmemb * s;

    // detect potential overflow in multiplication
    if (nmemb != 0 && total_size / nmemb != s) {
        return NULL;
    }

    debug_printf("Calloc %zu bytes\n", total_size);

    // allocate memory using mymalloc
    void *ptr = mymalloc(total_size);

    // zero out the allocated memory (as calloc should)
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

// myfree
// purpose, mark a previously allocated block as free and available for reuse
// arguments, ptr is a pointer to memory previously returned by mymalloc or mycalloc
// returns, void
// notes, does not coalesce adjacent free blocks (future optimization)
void myfree(void *ptr) {
    if (ptr == NULL) {
        return; // no-op on null pointer
    }

    // get block metadata (stored immediately before the user data)
    block_t *block = (block_t*)ptr - 1;
    debug_printf("Freed %zu bytes\n", block->size);

    // prevent double free, block should not already be marked free
    assert(block->free == 0);

    // mark the block as free for future allocations
    block->free = 1;

    // note, this implementation does not coalesce adjacent free blocks
    // future improvement, merge contiguous free regions to reduce fragmentation
}
