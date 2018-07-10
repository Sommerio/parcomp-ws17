#ifndef INTRODUCTION_PARALLEL_COMPUTING_COMMON_H
#define INTRODUCTION_PARALLEL_COMPUTING_COMMON_H

#include <stdio.h>
#include <sys/time.h>
#include "datagenerator.h"
#include "quicksort-seq.h"
#include "types.h"


#define MICRO 1000000.0
#define MILLI 1000.0

// a quick hack
typedef unsigned long long usecs;

usecs mytime();

int choose_pivot(int *a, int len);

void printArray(basetype a[], int n);

void exsum_seq(int x[], int start, int n);

void partition_seq(basetype a[], int n, basetype pivot);

void printPartArray(basetype a[], int start, int end);

void fill_data(basetype* a, int n, int s);

#endif //INTRODUCTION_PARALLEL_COMPUTING_COMMON_H
