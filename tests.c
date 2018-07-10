#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "./openmp/ompquick.h"
#include "cilk/cilkqsort.h"

void verify_sorted(basetype *a, int n){
    for (int i=0; i<n-1; i++) assert(a[i]<=a[i+1]);
    printf("CHECK - array is sorted\n");
}

void verify_equal(basetype *a, basetype *a2, int n){
    for (int i=0; i<n-1; i++) assert(a[i]<=a2[i+1]);
    printf("CHECK - array is equal to sequentially sorted\n");
}

void omp_single_test(int n, int t, void (*array_gen)(basetype *, int) ){
    basetype *a         = (basetype*)malloc(n*sizeof(basetype));
    basetype *control         = (basetype*)malloc(n*sizeof(basetype));
    basetype *helper    = (basetype*)malloc(n*sizeof(basetype));
    (void) (*array_gen)(a,n);
    memcpy(control, a, n*sizeof(basetype));
    (void) omp_quick(a,helper,n,t);
    (void) verify_sorted(a,n);
    quicksort_seq(control, n);
    verify_equal(a, control, n);
    free(a);
    free(helper);
}

void cilk_single_test(int n, int t, void (*array_gen)(basetype *, int) ){
    basetype *a         = (basetype*)malloc(n*sizeof(basetype));
    basetype *control         = (basetype*)malloc(n*sizeof(basetype));
    basetype *helper    = (basetype*)malloc(n*sizeof(basetype));
    (void) (*array_gen)(a,n);
    memcpy(control, a, n*sizeof(basetype));
    (void) set_cilk_workers_int(t);
    (void) cilk_qsort(a,helper,n);
    (void) verify_sorted(a,n);
    quicksort_seq(control, n);
    verify_equal(a, control, n);
    free(a);
    free(helper);
}

void tests_omp_cilk(){
    #ifdef INT
    //random integer array with array seed 420
    srandom(420);
    (void) omp_single_test(1, 1, gen_random_int_array);
    (void) cilk_single_test(1, 1, gen_random_int_array);

    //desc integer array
    //n < t
    (void) omp_single_test(2, 4, gen_desc_int_array);
    (void) cilk_single_test(2, 4, gen_desc_int_array);

    //random integer array
    //p != 2^k
    (void) omp_single_test(1024, 3, gen_random_int_array);
    (void) cilk_single_test(1024, 3, gen_random_int_array);

    //random integer array
    //n = prime
    (void) omp_single_test(99971, 4, gen_random_int_array);
    (void) cilk_single_test(99971, 4, gen_random_int_array);

    //random integer array
    //n = prime, p = prime
    (void) omp_single_test(99971, 3, gen_random_int_array);
    (void) cilk_single_test(99971, 3, gen_random_int_array);

    //test ascending array
    (void) omp_single_test(99971, 3, gen_asc_int_array);
    (void) cilk_single_test(99971, 3, gen_asc_int_array);

    //test descending array
    (void) omp_single_test(99971, 3, gen_desc_int_array);
    (void) cilk_single_test(99971, 3, gen_desc_int_array);

    //test eq array
    (void) omp_single_test(99971, 3, gen_eq_int_array);
    (void) cilk_single_test(99971, 3, gen_eq_int_array);

    //test with a large number of threads
    (void) omp_single_test(99971, 71, gen_random_int_array);
    (void) cilk_single_test(99971, 71, gen_random_int_array);


    //different chunk sizes for cilk
    (void) set_chunk_size(0);
    (void) cilk_single_test(99971, 3, gen_random_int_array);
    (void) set_chunk_size(1);
    (void) cilk_single_test(99971, 3, gen_random_int_array);
    (void) set_chunk_size(1024);
    (void) cilk_single_test(99971, 3, gen_random_int_array);
    (void) set_chunk_size(1783);
    (void) cilk_single_test(99971, 3, gen_random_int_array);
    (void) cilk_single_test(99971, 4, gen_random_int_array);
    (void) cilk_single_test(99971, 3, gen_asc_int_array);
    (void) cilk_single_test(99971, 3, gen_desc_int_array);
    (void) cilk_single_test(99971, 3, gen_eq_int_array);
    #endif

    #ifdef DOUBLE
    //random integer array with array seed 420
    srandom(420);
    (void) omp_single_test(1, 1, gen_random_double_array);
    (void) cilk_single_test(1, 1, gen_random_double_array);
    
    //desc integer array
    //n < t
    (void) omp_single_test(2, 4, gen_desc_double_array);
    (void) cilk_single_test(2, 4, gen_desc_double_array);
    
    //random integer array
    //p != 2^k
    (void) omp_single_test(1024, 3, gen_random_double_array);
    (void) cilk_single_test(1024, 3, gen_random_double_array);
    
    //random integer array
    //n = prime
    (void) omp_single_test(99971, 4, gen_random_double_array);
    (void) cilk_single_test(99971, 4, gen_random_double_array);
    
    //random integer array
    //n = prime, p = prime
    (void) omp_single_test(99971, 3, gen_random_double_array);
    (void) cilk_single_test(99971, 3, gen_random_double_array);

    //test ascending array
    (void) omp_single_test(99971, 3, gen_asc_double_array);
    (void) cilk_single_test(99971, 3, gen_asc_double_array);

    //test descending array
    (void) omp_single_test(99971, 3, gen_desc_double_array);
    (void) cilk_single_test(99971, 3, gen_desc_double_array);

    //test eq array
    (void) omp_single_test(99971, 3, gen_eq_double_array);
    (void) cilk_single_test(99971, 3, gen_eq_double_array);

    //test with a large number of threads
    (void) omp_single_test(99971, 71, gen_random_double_array);
    (void) cilk_single_test(99971, 71, gen_random_double_array);
    
    //different chunk sizes for cilk
    (void) set_chunk_size(0);
    (void) cilk_single_test(99971, 3, gen_random_double_array);
    (void) set_chunk_size(1);
    (void) cilk_single_test(99971, 3, gen_random_double_array);
    (void) set_chunk_size(1024);
    (void) cilk_single_test(99971, 3, gen_random_double_array);
    (void) set_chunk_size(1783);
    (void) cilk_single_test(99971, 3, gen_random_double_array);
    (void) cilk_single_test(99971, 4, gen_random_double_array);
    (void) cilk_single_test(99971, 3, gen_asc_double_array);
    (void) cilk_single_test(99971, 3, gen_desc_double_array);
    (void) cilk_single_test(99971, 3, gen_eq_double_array);
    #endif
}


int main(int argc, char *argv[])
{	
	tests_omp_cilk();
    return 0;
}
