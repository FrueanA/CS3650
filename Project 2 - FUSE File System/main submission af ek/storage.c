// High-level storage operations for the filesystem.

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>

#include "storage.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "slist.h"

// walks path from root directory, returns inode number
// returns -ENOENT if path not found
static int resolve_path_inum(const char *path) {
  if (strcmp(path, "/") == 0) return 0;  // root
  if (path[0] != '/') return -ENOENT;    // must start with /

  int cur = 0;  // start at root
  const char *p = path + 1;  // skip leading /
  char comp[DIR_NAME_LENGTH];

  // walk each path component
  while (*p) {
    const char *s = strchr(p, '/');
    size_t len = s ? (size_t)(s - p) : strlen(p);
    if (len == 0 || len >= sizeof(comp)) return -ENOENT;
    
    // copy component name
    memcpy(comp, p, len);
    comp[len] = '\0';

    // look up in current directory
    inode_t *node = get_inode(cur);
    int next = directory_lookup(node, comp);
    if (next < 0) return -ENOENT;
    cur = next;

    if (!s) break;  // no more components
    p = s + 1;
  }

  return cur;
}

// splits path into parent directory inode and entry name
// returns 0 on success, negative error on failure
static int resolve_parent(const char *path, int *parent_inum, const char **nameptr) {
  if (strcmp(path, "/") == 0) return -EINVAL;
  if (path[0] != '/') return -EINVAL;

  // find last slash to split path
  const char *last = strrchr(path, '/');
  assert(last != NULL);

  if (last == path) {
    // file in root directory
    *parent_inum = 0;
    *nameptr = path + 1;
    if (**nameptr == '\0') return -EINVAL;
    return 0;
  } else {
    // nested path - resolve parent directory
    size_t plen = (size_t)(last - path);
    char parent[256];
    if (plen >= sizeof(parent)) return -ENAMETOOLONG;
    memcpy(parent, path, plen);
    parent[plen] = '\0';
    
    int inum = resolve_path_inum(parent);
    if (inum < 0) return inum;
    
    *parent_inum = inum;
    *nameptr = last + 1;
    if (**nameptr == '\0') return -EINVAL;
    return 0;
  }
}

// wrapper for resolve_path_inum
static int get_inum(const char *path) {
  return resolve_path_inum(path);
}

// initializes storage system with disk image at path
void storage_init(const char *path) {
  blocks_init(path);
  directory_init();
  printf("+ storage_init(%s)\n", path);
}

// fills stat struct with file/directory attributes
// returns 0 on success, negative error on failure
int storage_stat(const char *path, struct stat *st) {
  int inum = get_inum(path);
  if (inum < 0) return inum;

  inode_t *node = get_inode(inum);

  memset(st, 0, sizeof(struct stat));
  st->st_mode = node->mode;
  st->st_size = node->size;
  st->st_uid = getuid();
  st->st_nlink = node->refs;

  return 0;
}

// reads data from file into buffer
// returns bytes read or negative error
int storage_read(const char *path, char *buf, size_t size, off_t offset) {
  int inum = get_inum(path);
  if (inum < 0) return inum;

  inode_t *node = get_inode(inum);

  // check bounds
  if (offset >= node->size) return 0;
  if (offset + size > (size_t)node->size) {
    size = node->size - offset;
  }

  // no data if no block
  if (node->block < 0) return 0;

  // read from data block
  void *data = blocks_get_block(node->block);
  memcpy(buf, data + offset, size);

  return size;
}

// writes data from buffer to file
// returns bytes written or negative error
int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
  int inum = get_inum(path);
  if (inum < 0) return inum;

  inode_t *node = get_inode(inum);
  int new_size = offset + size;

  // grow file if needed
  if (new_size > node->size) {
    if (grow_inode(node, new_size) < 0) {
      return -ENOSPC;
    }
  }

  // write to data block
  void *data = blocks_get_block(node->block);
  memcpy(data + offset, buf, size);

  blocks_sync();  // flush to disk
  return size;
}

// changes file size, growing or shrinking as needed
// returns 0 on success, negative error on failure
int storage_truncate(const char *path, off_t size) {
  int inum = get_inum(path);
  if (inum < 0) return inum;

  inode_t *node = get_inode(inum);

  if (size < node->size) {
    return shrink_inode(node, size);
  } else if (size > node->size) {
    return grow_inode(node, size);
  }

  return 0;
}

// creates new file or directory with given mode
// returns 0 on success, negative error on failure
int storage_mknod(const char *path, int mode) {
  if (path[0] != '/') return -EINVAL;

  const char *name;
  int parent_inum;
  int rv = resolve_parent(path, &parent_inum, &name);
  if (rv < 0) return rv;

  // check if already exists
  if (get_inum(path) >= 0) return -EEXIST;

  // allocate inode
  int inum = alloc_inode();
  if (inum < 0) return -ENOSPC;

  // initialize inode
  inode_t *node = get_inode(inum);
  node->mode = mode;
  node->size = 0;
  node->block = -1;
  node->refs = 1;

  // add to parent directory
  inode_t *parent = get_inode(parent_inum);
  rv = directory_put(parent, name, inum);
  if (rv < 0) {
    free_inode(inum);
    return rv;
  }

  blocks_sync();
  return 0;
}

// removes file from filesystem
// returns 0 on success, negative error on failure
int storage_unlink(const char *path) {
  int inum = get_inum(path);
  if (inum < 0) return inum;

  const char *name;
  int parent_inum;
  int rv = resolve_parent(path, &parent_inum, &name);
  if (rv < 0) return rv;

  // remove from parent directory
  inode_t *parent = get_inode(parent_inum);
  rv = directory_delete(parent, name);
  if (rv < 0) return rv;

  // free the inode and its block
  free_inode(inum);
  blocks_sync();
  return 0;
}

// creates hard link from 'from' to 'to'
// returns 0 on success, negative error on failure
int storage_link(const char *from, const char *to) {
  int inum = get_inum(from);
  if (inum < 0) return inum;
  if (to[0] != '/') return -EINVAL;

  const char *to_name;
  int parent_inum;
  int rv = resolve_parent(to, &parent_inum, &to_name);
  if (rv < 0) return rv;

  // add new directory entry pointing to same inode
  inode_t *parent = get_inode(parent_inum);
  rv = directory_put(parent, to_name, inum);
  if (rv < 0) return rv;

  // increment reference count
  inode_t *node = get_inode(inum);
  node->refs++;
  return 0;
}

// moves/renames file from 'from' to 'to'
// returns 0 on success, negative error on failure
int storage_rename(const char *from, const char *to) {
  int inum = get_inum(from);
  if (inum < 0) return inum;

  // remove target if it exists
  int to_inum = get_inum(to);
  if (to_inum >= 0) {
    storage_unlink(to);
  }

  const char *from_name, *to_name;
  int from_parent, to_parent;

  // resolve both paths
  int rv = resolve_parent(from, &from_parent, &from_name);
  if (rv < 0) return rv;
  rv = resolve_parent(to, &to_parent, &to_name);
  if (rv < 0) return rv;

  inode_t *from_dir = get_inode(from_parent);
  inode_t *to_dir = get_inode(to_parent);

  // remove from old location
  rv = directory_delete(from_dir, from_name);
  if (rv < 0) return rv;

  // add to new location
  rv = directory_put(to_dir, to_name, inum);
  if (rv < 0) {
    // restore on failure
    directory_put(from_dir, from_name, inum);
    return rv;
  }

  blocks_sync();
  return 0;
}

// updates file timestamps (not implemented in this simple fs)
// returns 0 on success
int storage_set_time(const char *path, const struct timespec ts[2]) {
  int inum = get_inum(path);
  if (inum < 0) return inum;
  return 0;  // timestamps not stored
}

// lists contents of directory at path
// returns linked list of names, or NULL on error
slist_t *storage_list(const char *path) {
  int inum = get_inum(path);
  if (inum < 0) return NULL;

  inode_t *node = get_inode(inum);
  if (!S_ISDIR(node->mode)) return NULL;

  return directory_list(node);
}
