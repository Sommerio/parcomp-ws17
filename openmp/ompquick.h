#ifndef INTRODUCTION_PARALLEL_COMPUTING_OMPQUICK_H
#define INTRODUCTION_PARALLEL_COMPUTING_OMPQUICK_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "../common/common.h"

#include <omp.h>

void omp_quick(basetype a[], basetype aa[], int n, int threads);

void run_omp_comparison(int argc, char *argv[]);

#endif //INTRODUCTION_PARALLEL_COMPUTING_OMPQUICK_H
