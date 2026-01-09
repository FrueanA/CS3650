## Concurrent Merge Sort with POSIX Threads
A multithreaded implementation of merge sort in C using POSIX threads (pthread).
This project parallelizes the recursive structure of merge sort to improve performance on multi-core systems and evaluates scalability across different machines and thread counts.

## Project Overview

The project starts from a standard top-down merge sort implementation and extends it into a concurrent algorithm by executing recursive sub-sorts in parallel. The result is a configurable, thread-limited merge sort that adapts to the available hardware while preserving correctness and stability.

The program reads a fixed number of signed long integers from standard input, sorts them, and writes the sorted output to standard output.

### Design & Implementation
Baseline Merge Sort
Uses a classic recursive merge sort

Operates on two arrays:
- an input array
- a helper/target array used during merging
- Modifies the original array while producing a fully sorted result

### Concurrent Extension
- Parallelizes recursive calls using pthread_create
- Each recursive split may spawn a new thread, subject to a global thread limit
- Ensures the total number of active threads never exceeds a user-defined maximum
- Synchronizes threads using pthread_join before merging sub-results

#### Thread Control
The maximum number of threads is configured via the environment variable export MSORT_THREADS=8

On sufficiently large inputs, the implementation uses exactly the specified number of threads (including the main thread)
- Prevents oversubscription and excessive thread creation

### Performance Experiments

To evaluate scalability, the concurrent merge sort was benchmarked on multiple machines with varying hardware configurations.

For each system:
- Large input sizes were used so the single-threaded version ran for 10+ seconds
- Experiments were repeated with different thread counts (1, 2, number of cores, beyond core count)
- Each configuration was run multiple times to account for variance
- Execution time was measured for the sorting phase only
  
Detailed results and observations are documented in experiments.md.
