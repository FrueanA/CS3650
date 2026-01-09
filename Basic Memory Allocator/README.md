## Custom Memory Allocator Using mmap
A lightweight implementation of malloc, calloc, and free in C using page-based memory allocation with mmap instead of the deprecated sbrk interface.
This project explores how modern Unix/Linux systems manage heap memory and demonstrates core allocator techniques.

## Overview

The allocator requests memory directly from the operating system using mmap, working with page-sized regions rather than a continuously growing heap. It maintains a free list for reusable blocks and handles fragmentation through block splitting and coalescing.

Allocations are divided into:
Small blocks: Served from page-sized regions and reused via the free list
Large blocks: Allocated as multi-page mmap regions and released immediately with munmap

### Key Features

- Uses mmap / munmap for all memory management
- Page-size aware (sysconf(_SC_PAGESIZE))
- Free list with address-ordered insertion
- Block splitting to reduce waste
- Block coalescing to limit fragmentation
- Efficient calloc leveraging zeroed anonymous pages
- Optional debug tracing for allocator behavior
