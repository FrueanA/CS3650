// Directory manipulation functions.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include "slist.h"

// initializes root directory as inode 0
void directory_init() {
  void *ibm = get_inode_bitmap();
  
  // only init if not already done
  if (!bitmap_get(ibm, 0)) {
    int root_inum = alloc_inode();
    assert(root_inum == 0);
    
    inode_t *root = get_inode(0);
    root->mode = 040755;  // directory with rwxr-xr-x
    root->size = 0;
  }
}

// looks up entry by name in directory
// returns inode number or -ENOENT if not found
int directory_lookup(inode_t *di, const char *name) {
  assert(di != NULL);
  
  // no entries if no block allocated
  if (di->block < 0) {
    return -ENOENT;
  }
  
  dirent_t *entries = (dirent_t *)blocks_get_block(di->block);
  int num_entries = di->size / sizeof(dirent_t);
  
  // search through all entries
  for (int i = 0; i < num_entries; i++) {
    if (strcmp(entries[i].name, name) == 0) {
      return entries[i].inum;
    }
  }
  
  return -ENOENT;
}

// adds new entry to directory
// returns 0 on success, negative error code on failure
int directory_put(inode_t *di, const char *name, int inum) {
  assert(di != NULL);
  
  // check if name already exists
  if (directory_lookup(di, name) >= 0) {
    return -EEXIST;
  }
  
  // check name length
  if (strlen(name) >= DIR_NAME_LENGTH) {
    return -ENAMETOOLONG;
  }
  
  // allocate block for entries if needed
  if (di->block < 0) {
    di->block = alloc_block();
    if (di->block < 0) {
      return -ENOSPC;
    }
    di->size = 0;
  }
  
  // check if directory is full
  int num_entries = di->size / sizeof(dirent_t);
  int max_entries = BLOCK_SIZE / sizeof(dirent_t);
  
  if (num_entries >= max_entries) {
    return -ENOSPC;
  }
  
  // add new entry at end
  dirent_t *entries = (dirent_t *)blocks_get_block(di->block);
  strcpy(entries[num_entries].name, name);
  entries[num_entries].inum = inum;
  
  di->size += sizeof(dirent_t);
  
  return 0;
}

// removes entry from directory by name
// returns 0 on success, -ENOENT if not found
int directory_delete(inode_t *di, const char *name) {
  assert(di != NULL);
  
  if (di->block < 0) {
    return -ENOENT;
  }
  
  dirent_t *entries = (dirent_t *)blocks_get_block(di->block);
  int num_entries = di->size / sizeof(dirent_t);
  
  // find and remove entry
  for (int i = 0; i < num_entries; i++) {
    if (strcmp(entries[i].name, name) == 0) {
      // swap with last entry to fill gap
      if (i < num_entries - 1) {
        entries[i] = entries[num_entries - 1];
      }
      di->size -= sizeof(dirent_t);
      return 0;
    }
  }
  
  return -ENOENT;
}

// returns linked list of all entry names in directory
slist_t *directory_list(inode_t *di) {
  if (!di || di->block < 0) {
    return NULL;
  }

  dirent_t *entries = (dirent_t *)blocks_get_block(di->block);
  int num_entries = di->size / sizeof(dirent_t);

  // build list in reverse order so it comes out right
  slist_t *list = NULL;
  for (int i = num_entries - 1; i >= 0; i--) {
    list = slist_cons(entries[i].name, list);
  }

  return list;
}

// prints directory contents for debugging
void print_directory(inode_t *dd) {
  assert(dd != NULL);
  
  printf("Directory contents:\n");
  
  if (dd->block < 0) {
    printf("  (empty)\n");
    return;
  }
  
  dirent_t *entries = (dirent_t *)blocks_get_block(dd->block);
  int num_entries = dd->size / sizeof(dirent_t);
  
  for (int i = 0; i < num_entries; i++) {
    printf("  %s -> inode %d\n", entries[i].name, entries[i].inum);
  }
}
