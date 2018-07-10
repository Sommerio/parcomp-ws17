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


void mpi_single_test(int n, void (*array_gen)(basetype *, int)) {
	int threads, tid;
	//initialize MPI

	MPI_Comm_size(MPI_COMM_WORLD,&threads);
	MPI_Comm_rank(MPI_COMM_WORLD,&tid);

	if((threads & (threads - 1)) != 0  || n < threads){
		//threads is not a power of two
		printf("This algorithm works only, if threads is a power of two and larger than n (but threads is %i) -> exiting\n", threads);
		MPI_Abort(MPI_COMM_WORLD, -1);
	}

	basetype *input = (basetype*)malloc(n*sizeof(basetype));
	//init the array
	(void) (*array_gen)(input,n);
	basetype *copied_input = (basetype*)malloc(n*sizeof(basetype));
	for(int i=0; i<n; i++) copied_input[i] = input[i];

	struct ArrayInfo x = distribute_data(input,n);

	MPI_Comm start_comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &start_comm);
	int new_n;

	MPI_Barrier(MPI_COMM_WORLD);
	basetype *a = mpi_quick(x.a,x.n,start_comm, &new_n);
	MPI_Barrier(MPI_COMM_WORLD);
	basetype *collected = collect_array(a,new_n);


	if(tid == 0){
		quicksort_seq(copied_input,n);
		verify_equal(copied_input, collected, n);
		printf("\n");
	}
	
}


void tests_mpi(){
	srandom(233);
	mpi_single_test(9, gen_random_int_array);
    
	//n = 2^22, random_int_array
	mpi_single_test(4194304, gen_random_int_array);

	//n = 2^22, random_int_array
	mpi_single_test(4194304, gen_asc_int_array);

	//n = 2^22, random_int_array
	mpi_single_test(4194304, gen_desc_int_array);

	//n = 2^22, random_int_array
	mpi_single_test(4194304, gen_eq_int_array);

	//n = 2^22+1, random_int_array
	mpi_single_test(4194305, gen_random_int_array);

	//n = 2^22-1, random_int_array
	mpi_single_test(4194303, gen_random_int_array);

	//n = prime, random_int_array
	mpi_single_test(999749, gen_random_int_array);

	//n = prime
	mpi_single_test(999749, gen_asc_int_array);

	//n = prime
	mpi_single_test(999749, gen_desc_int_array);

	//n = prime
	mpi_single_test(999749, gen_eq_int_array);
}


int main(int argc, char *argv[])
{	
	MPI_Init(&argc,&argv);
	tests_mpi();
	MPI_Finalize();
    return 0;
}
