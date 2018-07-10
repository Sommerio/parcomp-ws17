#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "./mpi/mpiquick.h"
#include "mpi.h"

void verify_equal(basetype *a, basetype *a2, int n){
    for (int i=0; i<n; i++){
		if(a[i] != a2[i]){
			printf(FS" vs "FS"\n", a[i], a2[i]);
            printf("should: ");
            printArray(a,n);
            printf("is: ");
            printArray(a2,n);

		}
		assert(a[i]==a2[i]);
	}
    printf("CHECK - array is equal to sequentially sorted\n");
}

void benchmark(int n){
	int threads, tid;

	//srandom(time(NULL));
	//initialize MPI

	MPI_Comm_size(MPI_COMM_WORLD,&threads);
	MPI_Comm_rank(MPI_COMM_WORLD,&tid);

	if((threads & (threads - 1)) != 0  || n < threads){
		//threads is not a power of two
		printf("This algorithm works only, if threads is a power of two and larger than n (but threads is %i) -> exiting\n", threads);
		MPI_Abort(MPI_COMM_WORLD, -1);
	}

	basetype* toSort = malloc(n * sizeof(basetype));
	basetype* toSortSeq = (basetype *) malloc(n * sizeof(basetype));
	fill_data(toSort, n, 0);
	for(int i=0; i<n; i++) toSortSeq[i] = toSort[i];

	
	struct ArrayInfo x = distribute_data(toSort, n);


	MPI_Comm start_comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &start_comm);
	int new_n;

	MPI_Barrier(MPI_COMM_WORLD);

	double stime = MPI_Wtime();
	basetype *a = mpi_quick(x.a, x.n, start_comm, &new_n);
	MPI_Barrier(MPI_COMM_WORLD);
	double etime = MPI_Wtime();

	basetype* collected = collect_array(a, new_n);
	double etime_coll = MPI_Wtime();


	if (tid == 0) {
		double stime_seq = MPI_Wtime();
		quicksort_seq(toSortSeq, n);
		double etime_seq = MPI_Wtime();

		verify_equal(toSortSeq,collected,n);
		
		printf("par\t\tseq\t\tspeedup\t\tspeedup_coll\n");
		printf("%f\t", etime - stime);
		printf("%f\t", etime_seq - stime_seq);
		if ((etime - stime) != 0 && (etime_coll - stime) != 0) {
			printf("%f\t", (etime_seq - stime_seq) / (etime - stime));
			printf("%f", (etime_seq - stime_seq) / (etime_coll - stime));
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	MPI_Init(&argc,&argv);
	benchmark(10000000);
	MPI_Finalize();
}
