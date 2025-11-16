/**
 * Simple, single-threaded merge sort.
 *
 * Do not modify this file so you can use it to check the tmsort implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <assert.h>

#include "timing.h"

#define tty_printf(...) (isatty(1) && isatty(0) ? printf(__VA_ARGS__) : 0)

#ifndef SHUSH
#define log(...) (fprintf(stderr, __VA_ARGS__))
#else 
#define log(...)
#endif

/** The number of threads to be used for sorting. Default: 1 */
int thread_count = 1;

/**
 * Print the given array of longs, an element per line.
 */
void print_long_array(const long *array, int count) {
  for (int i = 0; i < count; ++i) {
    printf("%ld\n", array[i]);
  }
}

/**
 * Merge two slices of nums into the corresponding portion of target.
 */
void merge(long nums[], int from, int mid, int to, long target[]) {
  int left = from;
  int right = mid;

  int i = from;
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
  if (left < mid) {
    memmove(&target[i], &nums[left], (mid - left) * sizeof(long));
  }
  else if (right < to) {
    memmove(&target[i], &nums[right], (to - right) * sizeof(long));
  }

}


/**
 * Sort the given slice of nums into target.
 *
 * Warning: nums gets overwritten.
 */
void merge_sort_aux(long nums[], int from, int to, long target[]) {
  if (to - from <= 1) {
    return;
  }

  int mid = (from + to) / 2;
  merge_sort_aux(target, from, mid, nums);
  merge_sort_aux(target, mid, to, nums);
  merge(nums, from, mid, to, target);
}


/**
 * Sort the given array and return the sorted version.
 *
 * The result is malloc'd so it is the caller's responsibility to free it.
 *
 * Warning: The source array gets overwritten.
 */
long *merge_sort(long nums[], int count) {
  long *result = calloc(count, sizeof(long));
  assert(result != NULL);

  memmove(result, nums, count * sizeof(long));

  merge_sort_aux(nums, 0, count, result);

  return result;
}

/**
 * Based on command line arguments, allocate and populate an input and a 
 * helper array.
 *
 * Returns the number of elements in the array.
 */
int allocate_load_array(int argc, char **argv, long **array) {
  const char *path = argc > 1 ? argv[1] : "-";

  FILE *input = NULL;
  if (strcmp(path, "-") == 0) 
    input = stdin;
  else
    input = fopen(path, "r");

  if (input == NULL) {
    fprintf(stdin, "Error opening file %s: %s", path, strerror(errno));
    exit(1);
  }

  size_t count;
  assert(1 == fscanf(input, "%zu", &count));

  *array = calloc(count, sizeof(long));
  assert(*array != NULL);

  long element;
  size_t i = 0;
  while (i < count && fscanf(input, "%ld", &element) != EOF)  {
    (*array)[i++] = element;
  }

  log("Read %zu items\n", i);

  return count;
}

int main(int argc, char **argv) {
  stopwatch_t timer;

  if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    fprintf(
        stderr, 
        "Usage: %s <filename>\n\n"
        "The first line of the file should be a count followed by that many lines containing\n"
        "a single decimal integer.\n",
        argv[0]);
    return 1;
  }


  // get the number of threads from the environment variable SORT_THREADS
  if (getenv("MSORT_THREADS") != NULL)
    thread_count = atoi(getenv("MSORT_THREADS"));

  log("Running with %d thread(s). Reading input.\n", thread_count);

  // Read the input
  start_timer(&timer);
  long *array = NULL;
  int count = allocate_load_array(argc, argv, &array);
  stop_timer(&timer);

  log("Array read in %f seconds, beginning sort.\n", 
      time_in_secs(&timer));
 
  // Sort the array
  start_timer(&timer);
  long *result = merge_sort(array, count);
  stop_timer(&timer);
  
  log("Sorting completed in %f seconds.\n", time_in_secs(&timer));

  // Print the result
  start_timer(&timer);
  print_long_array(result, count);
  stop_timer(&timer);
  
  log("Array printed in %f seconds.\n", time_in_secs(&timer));

  free(array);
  free(result);

  return 0;
}
