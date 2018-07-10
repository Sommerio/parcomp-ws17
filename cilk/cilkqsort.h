//
// Created by Simeon MACKE on 04.01.2018.
//
#ifndef CILKQSORT
#define CILKQSORT

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <string.h>

#include "../common/common.h"


#ifdef SEQUENTIAL
#define cilk_spawn
#define cilk_sync
#define cilk_for for
#else
// cilkplus libraries
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif


/**
 * Sets the cutoff value for the parallel quicksort. If chunk_size is 0, the default value will be used.
 * @param chunk_size The cutoff-size for the quicksort-algorithm. The array will be split in chunks of chunk_size for partitioning.
 */
void set_chunk_size(int chunk_size);

/**
 * Sets the number of workers Cilk should use for the parallel quicksort.
 * @param threads The number of cilk-workers as an integer string. If threads is NULL, the Cilk default will be used.
 */
void set_cilk_workers(char * workers);

/**
 * Does the same as void set_cilk_workers(char * workers), but accepts an integer
 * @param threads The number of cilk-workers as an integer string.
 */
void set_cilk_workers_int(int workers);


/**
 * The Cilk Quicksort algorithm. The array a will contain the sorted array.
 * @param a Array of basetype elements to sort.
 * @param aa Array of the size n. Will be used for temporary storing some values.
 * @param n The number of elements in the array a. Positive integer.
 */
void cilk_qsort(basetype *a, basetype *aa, int n);

#endif //CILKQSORT
