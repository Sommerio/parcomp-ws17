#include "cilkqsort.h"

//chunk size is tweaking value
int CHUNK_SIZE = 8192;

void exsum(int x[], int init_sum, int n){
    int sum = init_sum, prev = 0;

    for(int i=0; i<n; i++){
        sum += prev;
        prev = x[i];
        x[i] = sum;
    }
}

static void parallel_partition1(basetype a[], basetype aa[], int n, int chunk_number, int num_chunks, basetype pivot, int prefix_smaller[], int prefix_eq[], int prefix_larger[], int total_chunks)
{
    if (num_chunks > 1) {
        cilk_spawn parallel_partition1(a, aa, n, chunk_number, num_chunks/2, pivot, prefix_smaller, prefix_eq, prefix_larger, total_chunks);
        cilk_spawn parallel_partition1(a, aa, n, chunk_number + num_chunks/2, num_chunks-num_chunks/2, pivot, prefix_smaller, prefix_eq, prefix_larger, total_chunks);

        cilk_sync;
    } else {

        int start = chunk_number*CHUNK_SIZE;
        int chunk_size = CHUNK_SIZE;

        //last chunk stops at the end of the data range
        if (chunk_number == total_chunks-1) {
            chunk_size = n-start;
            if (chunk_size == 0) return;
        }
        int end = start + chunk_size;

        basetype *last_smaller = &aa[start];
        basetype *first_greater = &aa[end];

        for (int i = start; i < end; i++){
            if (a[i] == pivot) {
                prefix_eq[chunk_number]++;
            } else if (a[i] < pivot){
                prefix_smaller[chunk_number]++;

                *(last_smaller++) = a[i];
            } else {
                prefix_larger[chunk_number]++;

                *(--first_greater) = a[i];
            }
        }

        assert(prefix_smaller[chunk_number] + prefix_eq[chunk_number] + prefix_larger[chunk_number] == chunk_size);
    }
}

static void parallel_partition2(basetype a[], basetype aa[], int n, int chunk_number, int num_chunks, basetype pivot, int prefix_smaller[], int prefix_eq[], int prefix_larger[], int total_chunks)
{

    if (num_chunks > 1) {
        //printf("Partition step 2: Splitting. Part 1 Start: %d Length: %d Part 2 Start: %d Length %d\n", chunk_number, num_chunks/2, chunk_number + num_chunks/2, num_chunks-num_chunks/2);
        cilk_spawn parallel_partition2(a, aa, n, chunk_number, num_chunks/2, pivot, prefix_smaller, prefix_eq, prefix_larger, total_chunks);
        cilk_spawn parallel_partition2(a, aa, n, chunk_number + num_chunks/2, num_chunks-num_chunks/2, pivot, prefix_smaller, prefix_eq, prefix_larger, total_chunks);

        //cilk_sync;
    } else {

        int start = chunk_number*CHUNK_SIZE;
        int chunk_size = CHUNK_SIZE;
        //last chunk always goes to the end of the array
        if (chunk_number == total_chunks-1) {
            chunk_size = n - start;
            if (chunk_size == 0) return;
        }
        int end = start + chunk_size;

        //write smaller elements to global
        for (int i = prefix_smaller[chunk_number]; i < prefix_smaller[chunk_number+1]; i++) {
            a[i] = aa[start++];
        }

        //write elements equal to pivot to global
        for (int i = prefix_eq[chunk_number]; i < prefix_eq[chunk_number+1]; i++) {
            a[i] = pivot;
        }

        //write larger elements to global
        for (int i = prefix_larger[chunk_number]; i < prefix_larger[chunk_number+1]; i++) {
            a[i] = aa[--end];
        }

        assert(start <= end);
    }
}

static void partition(basetype pivot, basetype a[], basetype aa[], int n, int num_chunks, int *last_smaller, int *first_larger)
{

    int *prefix_smaller = (int*)malloc((num_chunks+1)*sizeof(int));
    int *prefix_eq = (int*)malloc((num_chunks+1)*sizeof(int));
    int *prefix_larger = (int*)malloc((num_chunks+1)*sizeof(int));

    (void) memset(prefix_smaller,0,(num_chunks+1)*sizeof(int));
    (void) memset(prefix_eq,0,(num_chunks+1)*sizeof(int));
    (void) memset(prefix_larger,0,(num_chunks+1)*sizeof(int));

    parallel_partition1(a, aa, n, 0, num_chunks, pivot, prefix_smaller, prefix_eq, prefix_larger, num_chunks);

    exsum(prefix_smaller, 0, num_chunks+1);
    exsum(prefix_eq, prefix_smaller[num_chunks], num_chunks+1);
    exsum(prefix_larger, prefix_eq[num_chunks], num_chunks+1);

    parallel_partition2(a, aa, n, 0, num_chunks, pivot, prefix_smaller, prefix_eq, prefix_larger, num_chunks);

    *last_smaller =  prefix_eq[0] - 1;
    *first_larger =  prefix_larger[0];

    free(prefix_smaller);
    free(prefix_eq);
    free(prefix_larger);
}

void cilk_qsort(basetype *a, basetype *aa, int n)
{
    int num_chunks = (n + CHUNK_SIZE - 1)/CHUNK_SIZE;

    if (num_chunks > 1) {
        basetype pivot = a[n/2];
        int last_smaller = 0;
        int first_larger = 0;

        partition(pivot, a, aa, n, num_chunks, &last_smaller, &first_larger);

        assert(last_smaller >= -1);
        assert(last_smaller < n);
        assert(first_larger >= 0);
        assert(first_larger <= n);
        assert(last_smaller < first_larger);


        if(last_smaller >= 0){
            cilk_spawn cilk_qsort(a, aa, last_smaller + 1);
        }

        if(first_larger < n){
            cilk_spawn cilk_qsort(&a[first_larger], &aa[first_larger], n - first_larger);
        }
        //cilk_sync;

    } else {
        quicksort_seq(a, n);
    }
}

/*void cilk_qsort(basetype a[], basetype bla[], int n)
{
    int i, j;
    basetype aa;

    if (n<2) return;

    // partition
    basetype pivot = a[0]; // choose an element non-randomly...
    i = 0; j = n;
    for (;;) {
        while (++i<j&&a[i]<pivot);
        while (a[--j]>pivot);
        if (i>=j) break;
        aa = a[i]; a[i] = a[j]; a[j] = aa;
    }
    // swap pivot
    aa = a[0]; a[0] = a[j]; a[j] = aa;

    cilk_spawn cilk_qsort(a, bla, j);
    cilk_spawn cilk_qsort(a+j+1, bla, n-j-1);
    cilk_sync;
}*/

void set_chunk_size(int chunk_size) {
    if (chunk_size) CHUNK_SIZE = chunk_size;
}

void set_cilk_workers(char * workers) {
#ifndef SEQUENTIAL
    if (workers) {
        __cilkrts_end_cilk();
        __cilkrts_set_param("nworkers",workers);
    }
    fprintf(stderr, "Total number of workers: %d\n",__cilkrts_get_nworkers());
#endif
}

void set_cilk_workers_int(int workers){
    char w[12];
    sprintf(w, "%d", workers);
    (void) set_cilk_workers(w);
}
