//
// Created by Simeon MACKE on 04.01.2018.
//
#include "cilkqsort.h"

void compare(int argc, char *argv[]) {
    int i, n;
    basetype *a, *aa, *a2;
    int s; // sequence type 0, 1, ...
    char *t = NULL; //number of workers/threads
    int chunk_size = 0;

    unsigned seed;

    usecs start, stop;

    s = 0;
    n = 1;
    seed = 0;
    for (i=1; i<argc&&argv[i][0]=='-'; i++) {
        if (argv[i][1]=='n') i++,sscanf(argv[i],"%d",&n);
        if (argv[i][1]=='s') i++,sscanf(argv[i],"%d",&s);
        if (argv[i][1]=='t') i++,t=argv[i];
        if (argv[i][1]=='S') i++,sscanf(argv[i],"%d",&seed);
        if (argv[i][1]=='c') i++,sscanf(argv[i],"%d",&chunk_size);
    }

    a = (basetype *)malloc(n*sizeof(basetype));
    aa = (basetype*)malloc(n*sizeof(basetype));
    a2 = (basetype*)malloc(n*sizeof(basetype));

    srandom(seed);
    fill_data(a,n,s);
    memcpy(a2, a, n * sizeof(basetype));

    set_chunk_size(chunk_size);
    set_cilk_workers(t);

    start = mytime();
    cilk_qsort(a, aa, n);
    stop = mytime();
    // verify
    for (i=0; i<n-1; i++) assert(a[i]<=a[i+1]);
    printf("Parallel sorting time %lld\n",stop-start);

    usecs par = stop-start;

    start = mytime();
    quicksort_seq(a2,n);
    stop = mytime();
    printf("Sequential sorting time %lld\n",stop-start);

    usecs seq = stop-start;

    printf("Speedup: %f\n", seq/(double)par);

    for (i=0; i<n-1; i++) assert(a[i]==a2[i]);

    free(a);
    free(aa);
    free(a2);
}

int main(int argc, char *argv[]) {
    compare(argc, argv);
    return 0;
}