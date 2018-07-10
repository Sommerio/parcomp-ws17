#ifndef INTRODUCTION_PARALLEL_COMPUTING_OMPQUICK_H
#define INTRODUCTION_PARALLEL_COMPUTING_OMPQUICK_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "../common/common.h"

#include "mpi.h"

struct ArrayInfo{
	basetype *a;
	int n;
};

struct ArrayInfo distribute_data(basetype *input, int n);
basetype *collect_array(basetype a[], int n);
basetype *mpi_quick(basetype a[], int n, MPI_Comm comm, int *new_n);

#endif //INTRODUCTION_PARALLEL_COMPUTING_OMPQUICK_H
