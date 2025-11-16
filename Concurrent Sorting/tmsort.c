/**
 * Threaded Merge Sort
 *
 * Multi-threaded merge sort using POSIX threads. Reads MSORT_THREADS environment
 * variable to limit concurrent threads and uses mutex to prevent race conditions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#include <unistd.h>

#include <assert.h>
#include <pthread.h>

#define tty_printf(...) (isatty(1) && isatty(0) ? printf(__VA_ARGS__) : 0)

#ifndef SHUSH
#define log(...) (fprintf(stderr, __VA_ARGS__))
#else
#define log(...)
#endif

// Global variables for thread control
int thread_count = 1;  // max threads allowed (from MSORT_THREADS env var)
int num_threads = 1;   // current active threads
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;  // protects num_threads

void merge_sort_aux(long nums[], int from, int to, long target[]);

// Arguments passed to worker threads
// Must be heap-allocated to avoid stack lifetime issues
typedef struct {
    long *arr;
    long *temp;
    int left;
    int right;
} ThreadArgs;

/**
 * Compute time difference in seconds
 */
double time_in_secs(const struct timeval *begin, const struct timeval *end) {
    long s = end->tv_sec - begin->tv_sec;
    long ms = end->tv_usec - begin->tv_usec;
    return s + ms * 1e-6;
}

/**
 * Print array elements, one per line
 */
void print_long_array(const long *array, int count) {
    for (int i = 0; i < count; ++i) {
        printf("%ld\n", array[i]);
    }
}

/**
 * Merge two sorted slices into target array
 */
void merge(long nums[], int from, int mid, int to, long target[]) {
    int left = from;
    int right = mid;

    int i = from;
    // Compare elements from both slices and copy smaller one
    for (; i < to && left < mid && right < to; i++) {
        if (nums[left] <= nums[right]) {
            target[i] = nums[left];
            left++;
        }
        else {
            target[i] = nums[right];
            right++;
        }
    }
    
    // Copy any remaining elements
    if (left < mid) {
        memmove(&target[i], &nums[left], (mid - left) * sizeof(long));
    }
    else if (right < to) {
        memmove(&target[i], &nums[right], (to - right) * sizeof(long));
    }
}

/**
 * Worker thread entry point - sorts its assigned slice then frees arguments
 */
void *merge_sort_thread(void *args) {
    ThreadArgs *arg = (ThreadArgs *)args;
    merge_sort_aux(arg->arr, arg->left, arg->right, arg->temp);
    free(arg);  // free heap allocation
    return NULL;
}

/**
 * Recursively sort slice using threads when available
 * Note: nums and target swap roles at each recursion level
 */
void merge_sort_aux(long nums[], int from, int to, long target[]) {
    if (to - from <= 1) {
        return;  // base case: already sorted
    }

    int mid = (from + to) / 2;
    pthread_t left_thread;
    int left_created = 0;

    // Try to spawn thread for left half if we have threads available
    pthread_mutex_lock(&thread_count_mutex);
    if (num_threads < thread_count) {
        num_threads++;
        pthread_mutex_unlock(&thread_count_mutex);
        
        // Allocate on heap because thread may outlive this function call
        ThreadArgs *left_args = malloc(sizeof(ThreadArgs));
        assert(left_args != NULL);
        left_args->arr = target;  // arrays swap for next level
        left_args->temp = nums;
        left_args->left = from;
        left_args->right = mid;
        
        pthread_create(&left_thread, NULL, merge_sort_thread, left_args);
        left_created = 1;
    }
    else {
        pthread_mutex_unlock(&thread_count_mutex);
    }

    // Current thread does right half
    merge_sort_aux(target, mid, to, nums);
    
    if (!left_created) {
        // No thread spawned, do left half here
        merge_sort_aux(target, from, mid, nums);
    }
    else {
        // Wait for left thread to finish
        pthread_join(left_thread, NULL);
        pthread_mutex_lock(&thread_count_mutex);
        num_threads--;  // free up thread slot
        pthread_mutex_unlock(&thread_count_mutex);
    }

    // Both halves sorted, now merge them
    merge(nums, from, mid, to, target);
}

/**
 * Sort array using parallel merge sort
 * Returns newly allocated sorted array (caller must free)
 */
long *merge_sort(long nums[], int count) {
    long *result = calloc(count, sizeof(long));
    assert(result != NULL);

    memmove(result, nums, count * sizeof(long));
    merge_sort_aux(nums, 0, count, result);

    return result;
}

/**
 * Load array from file - first line contains count, remaining lines contain integers
 */
int allocate_load_array(int argc, char **argv, long **array) {
    const char *path = argc > 1 ? argv[1] : "-";

    FILE *input = NULL;
    if (strcmp(path, "-") == 0) 
        input = stdin;
    else
        input = fopen(path, "r");

    if (input == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n", path, strerror(errno));
        exit(1);
    }

    size_t count;
    assert(1 == fscanf(input, "%zu", &count));

    *array = calloc(count, sizeof(long));
    assert(*array != NULL);

    long element;
    size_t i = 0;
    while (i < count && fscanf(input, "%ld", &element) != EOF) {
        (*array)[i++] = element;
    }

    log("Read %zu items\n", i);

    if (input != stdin) fclose(input);

    return count;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        fprintf(
            stderr, 
            "Usage: %s <filename>\n\n"
            "The first line of the file should be a count followed by that many lines containing\n"
            "a single decimal integer.\n",
            argv[0]);
        return 1;
    }

    struct timeval begin, end;

    // Get thread count from environment
    if (getenv("MSORT_THREADS") != NULL) {
        thread_count = atoi(getenv("MSORT_THREADS"));
    }

    log("Running with %d thread(s). Reading input.\n", thread_count);

    // Read input
    gettimeofday(&begin, 0);
    long *array = NULL;
    int count = allocate_load_array(argc, argv, &array);
    gettimeofday(&end, 0);

    log("Array read in %f seconds, beginning sort.\n", 
        time_in_secs(&begin, &end));

    // Sort
    gettimeofday(&begin, 0);
    long *result = merge_sort(array, count);
    gettimeofday(&end, 0);
    
    log("Sorting completed in %f seconds.\n", time_in_secs(&begin, &end));

    // Print result
    gettimeofday(&begin, 0);
    print_long_array(result, count);
    gettimeofday(&end, 0);
    
    log("Array printed in %f seconds.\n", time_in_secs(&begin, &end));

    // Cleanup
    free(array);
    free(result);

    return 0;
}