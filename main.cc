#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "HRTimer.hh"
#include "Sorters.hh"

void print_usage(const char* prog_name)
{
#ifdef PARALLEL
  printf("Usage:  %s -s <array size> -n <num threads> [ -r <seed> ]\n", prog_name);
#else
  printf("Usage:  %s -s <array size> [ -r <seed> ]\n", prog_name);
#endif
}

int main(int argc, char* argv[])
{
  int array_size = 0;
  int nthreads = 0;
  char c;
  unsigned int seed = time(NULL);

#ifdef PARALLEL  
  while ((c = getopt(argc, argv, "n:s:r:h")) != -1) {
    switch( c ) {
    case 'n':
      nthreads = atoi(optarg);
      break;
#else
  while ((c = getopt(argc, argv, "s:r:h")) != -1) {
    switch( c ) {
#endif
    case 'h':
      print_usage(argv[0]);
      return 0;
    case 'r':
      seed = atoi(optarg);
      break;
    case 's':
      array_size = atoi(optarg);
      break;
    case '?':
      printf("Unrecognized option %c\n", optopt);
      print_usage(argv[0]);
      return 1;
    }
  }

  if (array_size == 0) {
    printf("Missing required option -s\n");
    print_usage(argv[0]);
    return 1;
  }
#ifdef PARALLEL
  if (nthreads == 0) {
    printf("Missing required option -n\n");
    print_usage(argv[0]);
    return 1;
  }
#endif

  uint64_t* array = new uint64_t[array_size];

  srand(seed);

  for (int i=0; i<array_size; i++) {
    array[i] = (rand() & 0xffff);
    array[i] |= (rand() & 0xffff) << 16;
    array[i] |= static_cast<uint64_t>(rand() & 0xffff) << 32;
    array[i] |= static_cast<uint64_t>(rand() & 0xffff) << 48;
  }

  HRTimer timer;
  hrtime_t start, end;

#ifdef PARALLEL
#ifdef SHELL
  ParallelShellSorter sorter(nthreads);
#else
  ParallelRadixSorter sorter(nthreads);
#endif
#else
#ifdef SHELL
  ShellSorter sorter;
#else
  RadixSorter sorter;
#endif
#endif

  start = timer.get_time_ns();
  sorter.sort(array, array_size);
  end = timer.get_time_ns();

  // check

  for (int i=1; i<array_size; i++) {
    if (array[i-1] > array[i]) {
      fprintf(stderr, "Sort failed\n");
      return 1;
    }
  }
  printf("Sort time: %llu nanoseconds\n", end-start);
  return 0;
}
