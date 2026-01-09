// Inode manipulation routines.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

const int INODE_COUNT = 256;

// returns pointer to inode table stored in block 1
inode_t *get_inode_table() {
  return (inode_t *)blocks_get_block(1);
}

// prints inode fields for debugging
void print_inode(inode_t *node) {
  if (node) {
    printf("inode{refs: %d, mode: %04o, size: %d, block: %d}\n",
           node->refs, node->mode, node->size, node->block);
  }
}

// returns pointer to inode at given index
inode_t *get_inode(int inum) {
  assert(inum >= 0 && inum < INODE_COUNT);
  inode_t *table = get_inode_table();
  return &table[inum];
}

// allocates a new inode, initializes it, returns index or -1 on failure
int alloc_inode() {
  void *ibm = get_inode_bitmap();
  
  // find first free inode
  for (int ii = 0; ii < INODE_COUNT; ++ii) {
    if (!bitmap_get(ibm, ii)) {
      bitmap_put(ibm, ii, 1);  // mark as used
      
      // initialize all fields
      inode_t *node = get_inode(ii);
      memset(node, 0, sizeof(inode_t));
      node->refs = 1;
      node->mode = 0;
      node->size = 0;
      node->block = -1;  // no block yet
      
      printf("+ alloc_inode() -> %d\n", ii);
      return ii;
    }
  }
  
  return -1;  // no free inodes
}

// frees inode and its data block
void free_inode(int inum) {
  printf("+ free_inode(%d)\n", inum);
  
  inode_t *node = get_inode(inum);
  
  // free the data block if allocated
  if (node->block >= 0) {
    free_block(node->block);
  }
  
  // clear inode data
  memset(node, 0, sizeof(inode_t));
  
  // mark inode as free in bitmap
  void *ibm = get_inode_bitmap();
  bitmap_put(ibm, inum, 0);
}

// grows file to new size, allocating block if needed
// only supports files up to 4K
// returns 0 on success, -1 on failure
int grow_inode(inode_t *node, int size) {
  // max file size is one block (4K)
  if (size > BLOCK_SIZE) {
    return -1;
  }
  
  // allocate block if needed
  if (node->block < 0) {
    node->block = alloc_block();
    if (node->block < 0) {
      return -1;
    }
  }
  
  node->size = size;
  return 0;
}

// shrinks file to new size, freeing block if size is 0
// returns 0 on success
int shrink_inode(inode_t *node, int size) {
  // free block if shrinking to zero
  if (size == 0 && node->block >= 0) {
    free_block(node->block);
    node->block = -1;
  }
  
  node->size = size;
  return 0;
}

// returns block number for given file block index
// only block 0 is valid (single block files)
int inode_get_bnum(inode_t *node, int file_bnum) {
  if (file_bnum == 0) {
    return node->block;
  }
  return -1;
}
