# Threaded Merge Sort Experiments

## Host 1: GitHub Codespaces

- **CPU:** AMD EPYC 7763 64-Core Processor
- **Cores:** 4 (2 physical cores, 2 threads per core)
- **Cache size:** 32 MB L3
- **RAM:** 16 GB (15 GiB available)
- **Storage:** 32 GB (11 GB used, 20 GB available)
- **OS:** Ubuntu 22.04 LTS (Codespaces container)
- **Approx. running processes:** ~183

### Input data

The dataset used is 100,000,000 elements. It was created using:
```bash
./numbers 1 100000000 > big.txt
```

The single-threaded implementation `msort` takes approximately 6.667611 seconds to sort these elements.

### Experiments

#### 1 Thread

**Command used to run experiment:**
```bash
MSORT_THREADS=1 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 7.053692 seconds
2. 6.979775 seconds
3. 6.961451 seconds
4. 6.952874 seconds

#### 2 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=2 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 5.607078 seconds
2. 5.676697 seconds
3. 5.577499 seconds
4. 5.568447 seconds

#### 4 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=4 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 7.544432 seconds
2. 7.521487 seconds
3. 7.230970 seconds
4. 7.271674 seconds

#### 8 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=8 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 7.991239 seconds
2. 7.647452 seconds
3. 7.741702 seconds
4. 7.765664 seconds

---

## Host 2: MacBook Pro (Apple M2)

- **CPU:** Apple M2
- **Cores:** 8 (4 performance + 4 efficiency)
- **Cache size:** Unknown (Apple doesn't publicly document M2 cache details)
- **RAM:** 16 GB
- **Storage:** 460 GB total (410 GB used, 28 GB available)
- **OS:** macOS 15.6.1 (Sequoia)
- **Approx. running processes:** ~544

### Input data

The dataset used is 100,000,000 elements. It was created using:
```bash
./numbers 1 100000000 > big.txt
```

The single-threaded implementation `msort` takes approximately 17.348622 seconds to sort these elements.

### Experiments

#### 1 Thread

**Command used to run experiment:**
```bash
MSORT_THREADS=1 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 17.884125 seconds  
2. 18.087439 seconds  
3. 17.928342 seconds  
4. 18.148534 seconds  

#### 2 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=2 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 11.719555 seconds  
2. 11.857190 seconds  
3. 11.663391 seconds  
4. 11.516185 seconds  

#### 4 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=4 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 11.427106 seconds
2. 13.011996 seconds
3. 11.508683 seconds
4. 11.370373 seconds

#### 8 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=8 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 14.592912 seconds
2. 14.573603 seconds
3. 13.961152 seconds
4. 14.351247 seconds

#### 16 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=16 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 17.737602 seconds
2. 17.666951 seconds
3. 17.283942 seconds
4. 17.392493 seconds

#### 24 Threads

**Command used to run experiment:**
```bash
MSORT_THREADS=24 ./tmsort big.txt > /dev/null
```

**Sorting portion timings:**

1. 19.819064 seconds
2. 18.051125 seconds
3. 19.498606 seconds
4. 21.060645 seconds

---

## Observations and Conclusions

On Codespaces with 2 cores, performance improved from 7.0 seconds with 1 thread to 5.6 seconds with 2 threads. However, using more than 2 threads made things slower: 4 threads took 7.4 seconds and 8 threads took 7.8 seconds. The best performance happened at 2 threads, which matches the 2 cores available in the virtual environment.

On the MacBook Pro M2 with 8 cores, 1 thread took 18.0 seconds while 2 threads took 11.7 seconds. Performance stayed about the same with 4 threads at 11.8 seconds, but got worse with 8 threads at 14.3 seconds. Using even more threads (16 and 24) made it slower still, reaching 17.4 and 19.6 seconds. Even though the M2 has 8 cores, the best performance was with only 2-4 threads.

Codespaces couldn't use more than 2 threads effectively because it only has 2 cores available. When there are more threads than cores, the system wastes time switching between threads instead of doing useful work. On the M2, merge sort needs a lot of memory access, and all the threads end up waiting for memory rather than doing calculations. The M2's mix of fast and slow cores also affects how well threads can work together.

Both systems show that the best number of threads is close to the number of cores, but not always exactly the same. Using too many threads creates extra work from switching between threads and sharing resources, which slows everything down. For tasks like merge sort that use a lot of memory, matching threads to cores isn't enough because memory access becomes the limiting factor.