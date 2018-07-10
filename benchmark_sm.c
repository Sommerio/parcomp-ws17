//
// Created by Simeon MACKE on 04.01.2018.
//

#include "cilk/cilkqsort.h"
#include "openmp/ompquick.h"

typedef void (*parallel_qsort)(basetype*,basetype*,int);

int t;
int parallel_only = 0;

usecs average_time(usecs *l, int n) {
    usecs sum = 0;
    for (int i = 0; i<n; i++) {
        sum += l[i];
    }
    return sum/n;
}

float average(float l[], int n) {
    double sum = 0;
    for (int i = 0; i<n; i++) {
        sum += l[i];
    }
    return (float) sum/n;
}

void run_omp_quick(basetype *a,basetype *aa, int n) {
    omp_quick(a, aa, n, t);
}

void nothing(basetype *a,basetype *aa, int n) {}

void set_workers(int workers) {
    set_cilk_workers_int(workers);
    t = workers;
}


void run_benchmark(parallel_qsort parallel_qsort, int n, int iterations, int s, usecs *runtime_par, usecs *runtime_seq, float *speedup) {
    fprintf(stderr, "Running benchmark with %d threads, n=%d, %d iterations...\n", t, n, iterations);
    basetype *a = (basetype *)malloc(n*sizeof(basetype));
    basetype *a2 = (basetype *)malloc(n*sizeof(basetype));
    basetype *aa = (basetype*)malloc(n*sizeof(basetype));

    for (int i=0; i<iterations; i++) {
        usecs start, stop;

        fill_data(a,n,s);
        memcpy(a2, a, n * sizeof(basetype));

        start = mytime();
        parallel_qsort(a,aa,n);
        stop = mytime();

        runtime_par[i] = stop - start;

        if (!parallel_only) {
            start = mytime();
            quicksort_seq(a2,n);
            stop = mytime();
            runtime_seq[i] = stop - start;
            speedup[i] = runtime_seq[i] / (float)runtime_par[i];
        } else {
            runtime_seq[i] = 0;
            speedup[i] = 0;
        }
    }

    free(aa);
    free(a2);
    free(a);
}

void run_single(parallel_qsort parallel_qsort, int n, int iterations, int s) {
    usecs *runtime_par = (usecs *)malloc(iterations*sizeof(usecs));
    usecs *runtime_seq = (usecs *)malloc(iterations*sizeof(usecs));
    float *speedup = (float *)malloc(iterations*sizeof(float));

    run_benchmark(parallel_qsort, n, iterations, s, runtime_par, runtime_seq, speedup);

    usecs avg_par_time = average_time(runtime_par, iterations);
    usecs avg_seq_time = average_time(runtime_seq, iterations);
    float avg_speedup = average(speedup, iterations);

    printf("pass\tt_par\tt_seq\tspeedup\n");
    for (int i=0; i<iterations; i++) {
        printf("%d\t%lld\t%lld\t%f\n", i, runtime_par[i], runtime_seq[i], speedup[i]);
    }
    printf("-------------------------------------\n");
    printf("Avg.\t%lld\t%lld\t%f\n", avg_par_time, avg_seq_time, avg_speedup);

    free(speedup);
    free(runtime_seq);
    free(runtime_par);
}

void run_threads(parallel_qsort parallel_qsort, int n, int iterations, int s, int minthreads, int maxthreads) {
    usecs *runtime_par = (usecs *)malloc(iterations*sizeof(usecs));
    usecs *runtime_seq = (usecs *)malloc(iterations*sizeof(usecs));
    float *speedup = (float *)malloc(iterations*sizeof(float));

    printf("threads\tt_par\tt_seq\tspeedup\n");
    for (int i=minthreads; i<=maxthreads; i++) {
        set_workers(i);
        run_benchmark(parallel_qsort, n, iterations, s, runtime_par, runtime_seq, speedup);

        usecs avg_par_time = average_time(runtime_par, iterations);
        usecs avg_seq_time = average_time(runtime_seq, iterations);
        float avg_speedup = average(speedup, iterations);

        printf("%d\t%lld\t%lld\t%f\n", i, avg_par_time, avg_seq_time, avg_speedup);
    }

    free(speedup);
    free(runtime_seq);
    free(runtime_par);
}

void run_size(parallel_qsort parallel_qsort, int iterations, int s, int min_n, int max_n) {
    usecs *runtime_par = (usecs *)malloc(iterations*sizeof(usecs));
    usecs *runtime_seq = (usecs *)malloc(iterations*sizeof(usecs));
    float *speedup = (float *)malloc(iterations*sizeof(float));

    printf("n\tt_par\tt_seq\tspeedup\n");
    for (int n=min_n; n<=max_n; n<<=1) {
        run_benchmark(parallel_qsort, n, iterations, s, runtime_par, runtime_seq, speedup);

        usecs avg_par_time = average_time(runtime_par, iterations);
        usecs avg_seq_time = average_time(runtime_seq, iterations);
        float avg_speedup = average(speedup, iterations);

        printf("%d\t%lld\t%lld\t%f\n", n, avg_par_time, avg_seq_time, avg_speedup);
    }

    free(speedup);
    free(runtime_seq);
    free(runtime_par);
}

void test_cutoffs(int len, int n, int s, int start, int end) {
    int cutoff = start;
    printf("cutoff\taverage_time\n");
    basetype *a = (basetype *)malloc(len*sizeof(basetype));
    basetype *aa = (basetype*)malloc(len*sizeof(basetype));
    usecs *runtimes = (usecs *)malloc(n*sizeof(usecs));

    for (;cutoff <= end; cutoff<<=1) {
        usecs start, stop;
        set_chunk_size(cutoff);

        for (int i=0; i<n; i++) {

            fill_data(a,n,s);
            start = mytime();
            cilk_qsort(a, aa, len);
            stop = mytime();
            runtimes[i] = stop-start;
        }
        printf("%d\t%lld\n", cutoff, average_time(runtimes, n));
    }
    free(runtimes);
    free(a);
    free(aa);
}

void print_help(char *name) {
    printf("Usage: %s < -n size | -N min max > -i iterations < -a | -b > [-c cutoff | -C min max ] [-s data_type] [-S seed] [-t threads | -T min max ] [-p]\n"
                   "-n size \t: defines the array size to test.\n"
                   "-N min max\t: Test different array sizes starting at minsize and doubling each run until max.\n"
                   "-i iterations\t: The number of test-iterations\n"
                   "-a\t\t: Benchmark OpenMP\n"
                   "-b\t\t: Benchmark Cilk\n"
                   "-c cutoff\t: cutoff-value (chunk size in Cilk implementation)\n\n"
                   "-C min max\t: Try different values for cutoff, beginning at min, doubling until reaching max\n"
                   "-s data  \t: The kind of data to do the benchmarks on\n"
                   "-S seed  \t: The seed to generate random data from\n"
                   "-t threads\t: Number of threads which should be spawned\n"
                   "-T min max\t: Run the benchmark on min to max threads\n"
                   "-p\t\t: Parallel only. Don't compare to sequential implementation\n"
                   "-o\t\t: Sequential only. Don't compare to parallel implementation\n", name);
}

int main(int argc, char ** argv) {
    int i;
    unsigned int s = 0, seed = 0; //type of data and seed
    char *threads = NULL;
    int cutoff_size = 0;
    int iterations = 10, n = 1000000;
    int omp = 0, cilk = 0, min_threads = 1, max_threads = 0, sequential_only = 0;
    int min_n = 0, max_n = 0, min_cutoff = 16, max_cutoff = 262144;

    for (i=1; i<argc&&argv[i][0]=='-'; i++) {
        if (argv[i][1]=='n') i++,sscanf(argv[i],"%d",&n);
        if (argv[i][1]=='N') i++,sscanf(argv[i],"%d",&min_n),i++,sscanf(argv[i],"%d",&max_n);
        if (argv[i][1]=='s') i++,sscanf(argv[i],"%d",&s);
        if (argv[i][1]=='t') i++,threads=argv[i];
        if (argv[i][1]=='T') i++,sscanf(argv[i],"%d",&max_threads),i++,sscanf(argv[i],"%d",&min_threads);
        if (argv[i][1]=='S') i++,sscanf(argv[i],"%d",&seed);
        if (argv[i][1]=='c') i++,sscanf(argv[i],"%d",&cutoff_size);
        if (argv[i][1]=='C') cutoff_size=-1,i++,sscanf(argv[i],"%d",&min_cutoff),i++,sscanf(argv[i],"%d",&max_cutoff);
        if (argv[i][1]=='i') i++,sscanf(argv[i],"%d",&iterations);
        if (argv[i][1]=='a') omp=1;
        if (argv[i][1]=='b') cilk=1;
        if (argv[i][1]=='p') parallel_only=1;
        if (argv[i][1]=='o') sequential_only=1;
        if (argv[i][1]=='h') { print_help(argv[0]);
                                return 1; }
    }

    if (seed) {
        srandom(seed);
    } else {
        srandom((unsigned int)mytime());
    }

    if(sequential_only) {
        run_single(&nothing, n, iterations, s);
        return 0;
    }

    if (cutoff_size < 0) {
        test_cutoffs(n, iterations, s, min_cutoff, max_cutoff);
        return 0;
    }
    set_chunk_size(cutoff_size);

    if (max_threads) {
        if (omp) run_threads(&run_omp_quick, n, iterations, s, min_threads, max_threads);
        if (cilk) run_threads(&cilk_qsort, n, iterations, s, min_threads, max_threads);
        return 0;
    }

    if (threads) {
        set_cilk_workers(threads);
        sscanf(threads,"%d",&t);
    } else {
        t = omp_get_max_threads();
    }

    if (min_n) {
        if (omp) run_size(&run_omp_quick, iterations, s, min_n, max_n);
        if (cilk) run_size(&cilk_qsort, iterations, s, min_n, max_n);
        return 0;
    }

    if(omp) run_single(&run_omp_quick, n, iterations, s);

    if(cilk) run_single(&cilk_qsort, n, iterations, s);

    return 0;
}
