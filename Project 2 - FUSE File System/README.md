## User-Space File System (FUSE)

A user-space file system implemented using **FUSE**, backed by a fixed-size disk image.  
The project implements core file system functionality including files, directories, and block management.

---

## Overview

This file system mounts a 1MB disk image and provides a functional hierarchical file system. All metadata and file data are managed manually, closely following classic Unix-style file system design.

---

## Features

### File Support
- Create, read, write, rename, and delete files
- Supports small files (≤ 4KB)
- File name support (10+ characters)
- Up to 128 files within a 1MB disk image

### Directory Support
- Nested directories
- `mkdir`, `rmdir`, `rename`
- Directory listing (`readdir`)
- Files can be created and moved between directories

### Storage Design
- Custom on-disk layout
- Block-based allocation
- Bitmap-based free space tracking
- Separate metadata and data management layers

---

## Advanced Functionality

- Dynamic block allocation and deallocation
- Proper cleanup on file deletion
- Optional support for large files (up to hundreds of KB)
- Works without regenerating the disk image
