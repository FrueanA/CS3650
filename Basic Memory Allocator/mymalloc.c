#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <debug.h> // definition of debug_printf

// each memory block on the heap uses this struct to
// track the size of the block, its next neighbor, and whether it's free
typedef struct block {
    size_t size;          // size of the user data region following this block
    struct block *next;   // pointer to the next block in the linked list
    int free;             // 1 = free, 0 = allocated
} block_t;

#define BLOCK_SIZE sizeof(block_t)

// global variable, head of the free list (sorted by address)
static block_t *free_list = NULL;

// store system page size
static size_t PAGE_SIZE = 0;

// mutex for thread-safety
static pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;

// helper to get system page size
static inline size_t get_page_size() {
    if (PAGE_SIZE == 0) {
        PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
        assert(PAGE_SIZE > 0);
    }
    return PAGE_SIZE;
}

// helper function to determine if a request size is "small"
int is_small_block(size_t size) {
    return size < get_page_size() - BLOCK_SIZE;
}

// helper: insert block into free list sorted by address and coalesce adjacent blocks
// prints coalesce debug messages as required
void insert_into_free_list(block_t *block) {
    assert(block != NULL);
    block->free = 1;

    // if free list empty, insert as head
    if (free_list == NULL) {
        free_list = block;
        block->next = NULL;
        return;
    }

    // find insertion point (keep list sorted by address)
    if (block < free_list) {
        block->next = free_list;
        
        // Attempt coalescing with next
        if ((char*)block + BLOCK_SIZE + block->size == (char*)free_list) {
            size_t old_block_size = block->size;
            size_t old_next_size = free_list->size;
            size_t new_size = block->size + BLOCK_SIZE + free_list->size;
            debug_printf("free: coalesce blocks of size %zu and %zu to new block of size %zu\n",
                        old_block_size, old_next_size, new_size);
            block->size = new_size;
            block->next = free_list->next;
        }
        free_list = block;
        return;
    }

    // find insertion point (keep list sorted by address)
    block_t *current = free_list;
    while (current->next != NULL && current->next < block) {
        current = current->next;
    }

    // insert between current and current->next
    block->next = current->next;
    current->next = block;

    // Attempt coalescing with next
    if (block->next != NULL && 
        (char*)block + BLOCK_SIZE + block->size == (char*)block->next) {
        size_t old_block_size = block->size;
        size_t old_next_size = block->next->size;
        size_t new_size = block->size + BLOCK_SIZE + block->next->size;
        debug_printf("free: coalesce blocks of size %zu and %zu to new block of size %zu\n",
                    old_block_size, old_next_size, new_size);
        block->size = new_size;
        block->next = block->next->next;
    }

    // Attempt coalescing with previous (current)
    if ((char*)current + BLOCK_SIZE + current->size == (char*)block) {
        size_t old_current_size = current->size;
        size_t old_block_size = block->size;
        size_t new_size = current->size + BLOCK_SIZE + block->size;
        debug_printf("free: coalesce blocks of size %zu and %zu to new block of size %zu\n",
                    old_current_size, old_block_size, new_size);
        current->size = new_size;
        current->next = block->next;
    }
}

// helper function, find_free_block
// purpose, traverse the free list to locate the first free block large enough
// to hold size bytes
block_t *find_and_remove_free_block(size_t size) {
    block_t *prev = NULL;
    block_t *current = free_list;

    // traverse the free list until a usable free block is found
    while (current != NULL) {
        if (current->size >= size) {
            // remove from free list
            if (prev == NULL) {
                free_list = current->next;
            } else {
                prev->next = current->next;
            }
            current->free = 0;
            current->next = NULL;
            debug_printf("malloc: block of size %zu found\n", current->size);
            return current;
        }
        prev = current;
        current = current->next;
    }

    // no suitable free block found
    return NULL;
}

// add_more_space helper
// request new memory from the os using mmap when no existing
// free block can satisfy the request arguments
block_t *allocate_new_page(void) {
    size_t page_size = get_page_size();

    // for small blocks, allocate one page with mmap
    void *ptr = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return NULL;

    block_t *block = (block_t*)ptr;
    block->size = page_size - BLOCK_SIZE;
    block->next = NULL;
    block->free = 0;

    return block;
}

// mymalloc
// allocate a block of memory of size s bytes,
// prints "malloc %zu bytes\n" for debugging
void *mymalloc(size_t s) {
    debug_printf("Malloc %zu bytes\n", s);

    // reject zero-byte allocations
    if (s == 0) {
        return NULL;
    }

    // lock for thread-safety of free_list and allocator state
    pthread_mutex_lock(&malloc_lock);

    block_t *block = NULL;
    size_t page_size = get_page_size();

    // For small requests, try free list first
    if (is_small_block(s)) {
        block = find_and_remove_free_block(s);
        if (block == NULL) {
            debug_printf("malloc: block of size %zu not found - calling mmap\n", s);
            block = allocate_new_page();
            if (block == NULL) {
                pthread_mutex_unlock(&malloc_lock);
                return NULL;
            }
        }

        // if the new page is larger than requested, consider splitting
        if (block->size > s) {
            size_t original_size = block->size;
            size_t leftover = original_size - s;
            if (leftover >= (BLOCK_SIZE + BLOCK_SIZE)) {
                // split: allocated part remains at start, leftover becomes a free block
                block_t *leftover_block = (block_t*)((char*)(block + 1) + s);
                leftover_block->size = leftover - BLOCK_SIZE;
                leftover_block->next = NULL;
                leftover_block->free = 1;

                block->size = s;
                block->free = 0;

                insert_into_free_list(leftover_block);

                debug_printf("malloc: splitting - blocks of size %zu and %zu created\n",
                            block->size, leftover_block->size);
            }
        }

        void *user_ptr = (void*)(block + 1);
        pthread_mutex_unlock(&malloc_lock);
        return user_ptr;
    } else {
        // Large allocation: use mmap for exact number of pages required
        size_t num_pages = (s + BLOCK_SIZE + page_size - 1) / page_size;
        size_t total_size = num_pages * page_size;
        debug_printf("malloc: large block - mmap region of size %zu\n", total_size);

        void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            pthread_mutex_unlock(&malloc_lock);
            return NULL;
        }

        block = (block_t*)ptr;
        block->size = total_size - BLOCK_SIZE;
        block->next = NULL;
        block->free = 0;

        void *user_ptr = (void*)(block + 1);
        pthread_mutex_unlock(&malloc_lock);
        return user_ptr;
    }
}

// mycalloc
// allocate and zero-initialize an array of nmemb elements of size s
// arguments, prints "calloc %zu bytes\n" for debugging
void *mycalloc(size_t nmemb, size_t s) {
    size_t total_size = nmemb * s;

    // detect potential overflow in multiplication
    if (nmemb != 0 && total_size / nmemb != s) {
        return NULL;
    }

    debug_printf("Calloc %zu bytes\n", total_size);

    // allocate memory using mymalloc (which is thread-safe)
    void *ptr = mymalloc(total_size);
    if (ptr == NULL) return NULL;

    // zero only the user region (mmap'd pages are already zeroed, but safe to memset)
    memset(ptr, 0, total_size);

    return ptr;
}

// myfree
// mark a previously allocated block as free and available for reuse
void myfree(void *ptr) {
    if (ptr == NULL) {
        return; // no-op on null pointer
    }

    // get block metadata (stored immediately before the user data)
    block_t *block = (block_t*)ptr - 1;
    
    // prevent double free, block should not already be marked free
    assert(block->free == 0);

    // lock to protect free_list and related operations
    pthread_mutex_lock(&malloc_lock);

    size_t page_size = get_page_size();

    // For large blocks, use munmap
    if (!is_small_block(block->size)) {
        size_t num_pages = (block->size + BLOCK_SIZE + page_size - 1) / page_size;
        size_t total_size = num_pages * page_size;
        debug_printf("free: munmap region of size %zu\n", total_size);
        debug_printf("Freed %zu bytes\n", block->size);
        pthread_mutex_unlock(&malloc_lock);
        munmap(block, total_size);
        return;
    }

    debug_printf("Freed %zu bytes\n", block->size);

    // For small blocks, add the block to free list and coalesce if needed
    insert_into_free_list(block);

    // Keep at most 2 page-sized free blocks in free list, unmap extras
    size_t page_sized_blocks = 0;
    block_t *prev = NULL;
    block_t *current = free_list;
    size_t page_user_size = get_page_size() - BLOCK_SIZE;

    // traverse free list and count/remove excess page-sized blocks
    while (current != NULL) {
        if (current->size == page_user_size) {
            page_sized_blocks++;
            if (page_sized_blocks > 2) {
                // unmap this block
                if (prev == NULL) {
                    free_list = current->next;
                } else {
                    prev->next = current->next;
                }

                block_t *to_unmap = current;
                current = current->next;
                debug_printf("free: munmap region of size %zu\n", page_size);
                munmap(to_unmap, page_size);
                continue;
            }
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&malloc_lock);
}