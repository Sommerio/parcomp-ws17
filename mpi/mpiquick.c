#include "mpiquick.h"
#include <time.h>

int n_total;

struct ArrayInfo distribute_data(basetype *input, int n){
	int threads, tid;
	MPI_Comm_size(MPI_COMM_WORLD,&threads);
	MPI_Comm_rank(MPI_COMM_WORLD,&tid);

	int sendcounts[threads];
	int displs[threads];
	n_total = n;
	int blocksize = n/threads;		

	//distribute data
	if(tid == 0){
		
		/*
		int temp[] = {8,5,0,4,3,6,9,10,1,2,7,55,99,11};
		for(int i=0; i<n; i++){
			input[i] = temp[i];
		}
		*/
		

		for(int i=0; i<threads; i++){
			sendcounts[i] = blocksize;
		}
		//last thread may have a bigger array
		sendcounts[threads-1] = n - (threads-1) * blocksize;
		for(int i=0; i<threads; i++) displs[i] = sendcounts[i];
		exsum_seq(displs,0,threads);
		
		/*
		printf("x sendcounts: "); printArray(sendcounts, threads);
		printf("x displs: "); printArray(displs, threads);
		*/
	}
	if(tid == threads-1){
		n = n - tid * blocksize;
	} else {
		n = blocksize;
	}
	

	basetype *a = (basetype*)malloc(n*sizeof(basetype));
	MPI_Scatterv(input,sendcounts,displs,TYPE_MPI,a,n,TYPE_MPI,0,MPI_COMM_WORLD);
	
	/*
	printf("tid: %i, n: %i == ", tid, n);
	printArray(a,n);
	*/

	struct ArrayInfo x;
	x.a = a;
	x.n = n;
	
	return x;
}


basetype choosePivot(basetype a[], int n, MPI_Comm comm){
	int threads, tid;
	MPI_Comm_size(comm,&threads);
	MPI_Comm_rank(comm,&tid);
	
	int limit = 200;

	basetype pivot = 0;

	for(int i=0; i<limit && i<n; i++){
		pivot += a[i];
	}



	if(n != 0){
		if(n < limit){
			pivot = pivot/ (basetype) n;
		} else {
			pivot = pivot/ (basetype) limit;
		}
	}
	
	basetype *pivot_elems = (basetype*)malloc(threads*sizeof(basetype));
	
	//printf("my pivot: " FS "\n", pivot);
	
	MPI_Allgather(&pivot,1,TYPE_MPI,pivot_elems,1,TYPE_MPI,comm);
	
	pivot = 0;
	for(int i=0; i<threads; i++){
		pivot += pivot_elems[i];
	}
	pivot = pivot / (basetype) threads;

	/*
	printf("pivots: ");
	printArray(pivot_elems, threads);
	printf("final pivot: " FS "\n", pivot);
	*/

	free(pivot_elems);
	return pivot;
}


basetype *partition_local(basetype a[], int n, basetype pivot, MPI_Comm comm, int *first_larger_index){
	int tid;
	MPI_Comm_rank(comm,&tid);
	
    basetype *helper = (basetype*)malloc(n*sizeof(basetype));

	//printf("partitioning around pivot: %i, while n = %i \n", pivot, n);
	//printArray(a,n);

	int smaller_counter = 0;
	int larger_counter = n-1;
	int equal_counter = 0;

	for(int i=0; i<n; i++){
		basetype elem = a[i];

		if(elem < pivot){
			helper[smaller_counter++] = elem;
		}

		if(elem > pivot){
			helper[larger_counter--] = elem;
		}

		if(elem == pivot){
			equal_counter++;
		}
	}
	//set equal elements
	for(int i = smaller_counter; i<larger_counter + 1; i++){
		helper[i] = pivot;
	}
	assert(smaller_counter - 1 + equal_counter == larger_counter);

	/*
	//verify
	for(int i=0; i<smaller_counter; i++){
		assert(helper[i] <= pivot);
	}
	for(int i=larger_counter+1; i<n; i++){
		assert(helper[i] > pivot);
	}

    printf("partitioned array on tid %i (pivot: "FS"): ", tid, pivot);
	printArray(helper,n);
	*/

	*first_larger_index = smaller_counter;
    return helper;
}


basetype *exchange(basetype a[], int n, int first_larger_index, MPI_Comm comm, int *n_out){
    int threads, tid;
	MPI_Comm_size(comm,&threads);
	MPI_Comm_rank(comm,&tid);

    //TAG 0 = smaller elems
    //TAG 1 = larger elems

	int send_count;
	int partner_tid;
	int own_start_index;
	int send_start_index;

    if(tid % 2 == 0){
        //even processes send larger elements to tid + 1
        partner_tid = tid+1;
		own_start_index = 0;
		send_start_index = first_larger_index;
		send_count = n-first_larger_index;
    } else {
        //uneven processes send smaller elements to tid - 1
        partner_tid = tid-1;
		own_start_index = first_larger_index;
		send_start_index = 0;
		send_count = first_larger_index;
	}

	/*
	printf("(threads = %i) old array on tid %i before exchange: ", threads, tid);
	printArray(a,n);
	*/

	//exchange number of to be sent elements first
	int recv_count = 0;

	MPI_Status status1;
	MPI_Sendrecv(&send_count, 1, MPI_INT, partner_tid, 0,
	&recv_count, 1, MPI_INT, partner_tid, MPI_ANY_TAG, comm, &status1);
	
	//printf("tid %i sent %i elements and received %i elements\n", tid, send_count, recv_count);

	//now we know how many elements we are going to recv -> allocate mem
	basetype *recv_buffer = (basetype*)malloc(recv_count*sizeof(basetype));

	//exchange elements
	MPI_Status status2;
	MPI_Sendrecv(&a[send_start_index], send_count, TYPE_MPI, partner_tid, 0,
	recv_buffer, recv_count, TYPE_MPI, partner_tid, MPI_ANY_TAG, comm, &status2);

	//printf("x tid %i received: ", tid);
	//printArray(recv_buffer, recv_count);

	int own_elems = (n-send_count);
	int new_n = own_elems + recv_count;
	//printf("new n: %i\n", new_n);
	basetype *aa = (basetype*)malloc(new_n*sizeof(basetype));

	//copy own elements to new array
	for(int i=0; i<n-send_count; i++){
		aa[i] = a[i + own_start_index];
	}
	
	for(int i=0; i<recv_count; i++){
		aa[i+own_elems] = recv_buffer[i];
	}

	/*
	printf("(threads = %i) new array on tid %i after exchange: ", threads, tid);
	printArray(aa,new_n);
	*/

	*n_out = new_n;
	return aa;
}



void hypercube_calc(int a[], int n){
	int h[n];
	int c1 = 0;
	int c2 = 0;

	for(int i=0; i<n; i++){
		if(i % 2 == 0){
			h[c1++] = a[i];
		} else {
			h[(c2++) + n/2] = a[i];
		}
	}
	
	//copy back
	for(int i=0; i<n; i++) a[i] = h[i];

	if(n > 1){
		hypercube_calc(&a[0], n/2);
		hypercube_calc(&a[n/2], n/2);
	}
}


basetype *collect_array(basetype a[], int n){
	int threads, tid;
	MPI_Comm_size(MPI_COMM_WORLD,&threads);
	MPI_Comm_rank(MPI_COMM_WORLD,&tid);
	
	basetype *collected = NULL;
	
	if(tid == 0){
		collected = (basetype*)malloc(n_total*sizeof(basetype));
	}

	//send number of expected elements to root
	int elem_count[threads];
	int displs[threads];

	MPI_Gather(&n, 1, MPI_INT, elem_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//copy number of elements to displs
	for(int i=0; i<threads; i++) displs[i] = elem_count[i];

	
	if(tid == 0){
		int displs_t[threads];
		
		/*
		printf("displs: ");
		printArray(displs, threads);
		printf("elem count: ");
		printArray(elem_count, threads);
		*/
		
		//transpose displacement for the right order
		int transpose[threads];

		//first fill array with [0..threads-1]
		for(int i=0; i<threads; i++) transpose[i] = i;
		hypercube_calc(transpose, threads);
		
		/*
		printf("transpose vector: ");
		printArray(transpose, threads);
		*/

		//transpose elem_count and displs
		for(int i=0; i<threads; i++){
			displs_t[transpose[i]] = displs[i];
		}

		exsum_seq(displs_t,0,threads);

		/*
		printf("transposed displs: ");
		printArray(displs_t, threads);
		*/

		//transpose back
		for(int i=0; i<threads; i++){
			displs[transpose[i]] = displs_t[i];
		}
	}

	MPI_Gatherv(a,n,TYPE_MPI,collected,elem_count,displs,TYPE_MPI,0,MPI_COMM_WORLD);

	if(tid == 0){
		/*
		printf("xxxx collected: ");
		printArray(collected,n_total);
		*/
	}
	return collected;
}


basetype *mpi_quick(basetype a[], int n, MPI_Comm comm, int *new_n){
	int threads, tid;
	MPI_Comm_size(comm,&threads);
	MPI_Comm_rank(comm,&tid);

	basetype *exchanged = a;
	
	while(threads > 1){
		basetype pivot = choosePivot(a,n,comm);
		int first_larger_index;
		basetype *partitioned = partition_local(a,n,pivot,comm,&first_larger_index);
		exchanged = exchange(partitioned,n,first_larger_index,comm,&n);
		//split communicators
		MPI_Comm new_comm;
		MPI_Comm_split(comm, tid%2, tid, &new_comm);

		MPI_Comm_rank(new_comm,&tid);
		MPI_Comm_size(new_comm,&threads);

		a = exchanged;
		comm = new_comm;
	}

	assert(threads == 1);

	int global_tid;
	MPI_Comm_rank(MPI_COMM_WORLD,&global_tid);
	
	//printf("tid (global) %i sorting seq: %f\n", global_tid, (double) n/n_total);
	quicksort_seq(exchanged, n);
	//printArray(exchanged, n);
	

	*new_n = n;
	return exchanged;
}

/*
int main(int argc, char *argv[]) {
	int threads, tid;
	int n = 7;

	//srandom(time(NULL));
	//initialize MPI
	MPI_Init(&argc,&argv);

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

	
	if(tid == 0){
		printf("input: ");
		printArray(toSort, n);
	}


	struct ArrayInfo x = distribute_data(toSort, n);


	MPI_Comm start_comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &start_comm);
	int new_n;

	MPI_Barrier(MPI_COMM_WORLD);

	double stime = MPI_Wtime();
	basetype *a = mpi_quick(x.a, x.n, start_comm, &new_n);
	MPI_Barrier(MPI_COMM_WORLD);
	double etime = MPI_Wtime();

	printf("tid %i: ", tid);
	printArray(a,new_n);

	basetype* collected = collect_array(a, new_n);


	if (tid == 0) {
		double stime_seq = MPI_Wtime();
		quicksort_seq(toSortSeq, n);
		double etime_seq = MPI_Wtime();

		verify_equal(toSortSeq,collected,n);
		
		printf("par\t\tseq\t\tspeedup\n");
		printf("%f\t", etime - stime);
		printf("%f\t", etime_seq - stime_seq);
		if ((etime - stime) != 0) {
			printf("%f", (etime_seq - stime_seq) / (etime - stime));
		}
		printf("\n");
	}
	
	MPI_Finalize();
}
*/
