#pragma once

#ifndef CPUCLOCKTIMER
#include <sys/time.h>
typedef struct {
  struct timeval begin;
  struct timeval end;
} stopwatch_t;

/**
 * Start the given timer.
 */
static inline void start_timer(stopwatch_t *timer) {
  gettimeofday(&timer->begin, 0);
}

/** 
 * Stop the given timer.
 */
static inline void stop_timer(stopwatch_t *timer) {
  gettimeofday(&timer->end, 0);
}

/**
 * Compute the delta between the given timevals in seconds.
 */
static inline double time_in_secs(stopwatch_t *timer) {
  long s = timer->end.tv_sec - timer->begin.tv_sec;
  long ms = timer->end.tv_usec - timer->begin.tv_usec;
  return s + ms * 1e-6;
}
#else
#include <time.h>
typedef struct {
  clock_t begin;
  clock_t end;
} stopwatch_t;

/**
 * Start the given timer.
 */
static inline void start_timer(stopwatch_t *timer) {
  timer->begin = clock();
}

/**
 * Stop the given timer.
 */
static inline void stop_timer(stopwatch_t *timer) {
  timer->end = clock();
}
/**
 * Compute the delta between the given timevals in seconds.
 */
static inline double time_in_secs(stopwatch_t *timer) {
  return (double) (timer->end - timer->begin) / CLOCKS_PER_SEC;
}
#endif


