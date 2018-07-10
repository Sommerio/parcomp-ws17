#include "ompquick.h"

int n_total;

void local_partition(basetype a[], basetype aa[], int n, int threads, basetype pivot,
		int num_smaller_elems[], int num_eq_elems[], int num_larger_elems[]) {
	
	#pragma omp parallel default(none) \
	shared(a,aa,n,pivot,num_smaller_elems,num_eq_elems,num_larger_elems,threads) \
    num_threads(threads)
	{
		assert(omp_get_num_threads() == threads);

		int tid = omp_get_thread_num();
		int block = n/threads;
		int start = omp_get_thread_num()*block;
		int end = start + block;

		//last processor should also sort the remaining part of the array
		//and not only to 'end'
		if(tid == threads-1){
			block = n-start;
			end = n;
		}

		int smaller_counter = start;
		int larger_counter = end - 1;
		
		//determine number of smaller, eq and larger elements
		for(int i=start; i<end; i++){
			basetype elem = a[i];

			if(elem == pivot){
				num_eq_elems[tid]++;
			}

			if(elem < pivot){
				num_smaller_elems[tid]++;
				aa[smaller_counter++] = elem;
			}

			if(elem > pivot){
				num_larger_elems[tid]++;
				aa[larger_counter--] = elem;
			}
		}

		for(int i=num_smaller_elems[tid]; i < num_smaller_elems[tid] + num_eq_elems[tid]; i++){
			aa[i+start] = pivot;
		}

		assert(num_smaller_elems[tid] + num_eq_elems[tid] + num_larger_elems[tid] == block);
	}

}

void global_partition(basetype a[], basetype aa[], int n, int threads, basetype pivot,
		int num_smaller_elems[], int num_eq_elems[], int num_larger_elems[],
		int prefix_smaller[], int prefix_eq[], int prefix_larger[]){
	
	//sort globally into array
	#pragma omp parallel default(none) \
	shared(a,aa,n,pivot,num_smaller_elems,num_eq_elems,num_larger_elems,\
			prefix_smaller,prefix_eq,prefix_larger,threads) \
    num_threads(threads)
	{
        assert(omp_get_num_threads() == threads);

		int tid = omp_get_thread_num();
		int threads = omp_get_num_threads();
		int block = n/threads;
		int start = omp_get_thread_num()*block;
		//int end = start + block;

		if(tid == threads-1){
			block = n-start;
			//end = n;
		}

		int aa_counter = start;
		
		//copy smaller elems
		for(int i=prefix_smaller[tid]; i < prefix_smaller[tid] + num_smaller_elems[tid]; i++){
			a[i] = aa[aa_counter++];
		}

		//copy eq elems
		for(int i=prefix_eq[tid]; i < prefix_eq[tid] + num_eq_elems[tid]; i++){
			a[i] = aa[aa_counter++];
		}

		//copy larger elems
		for(int i=prefix_larger[tid]; i < prefix_larger[tid] + num_larger_elems[tid]; i++){
			a[i] = aa[aa_counter++];
		}
	}
}
/*
basetype choosePivot(basetype a[], int n, int threads){
	basetype *pivot_elems = (basetype*)malloc(threads*sizeof(basetype));
	basetype pivot = 0;

	
	#pragma omp parallel default(none) shared(a,n,pivot_elems,threads) num_threads(threads)
	{
        assert(omp_get_num_threads() == threads);

		//double frac = 0.01;
		int limit = 250;

		int tid = omp_get_thread_num();
		int threads = omp_get_num_threads();
		int block = n/threads;
		int start = omp_get_thread_num()*block;
		int end = start + block;

		if(tid == threads-1){
			block = n-start;
			end = n;
		}

		int pivot = 0;
		//for(int i = start; i<start+(block*frac); i++){
		for(int i = start; i<end && i < start + limit; i++){
			pivot += a[i];
			//printf("a[%i]: %i\n", i, a[i]);
			//assert(i >= 0);
		}
		if(end < start+limit){
			pivot = pivot / (end - start);
		} else {
			pivot = pivot / limit;
		}

		pivot_elems[tid] = pivot;
	}

	for(int i=0; i<threads; i++){
		pivot += pivot_elems[i];
	}
	pivot = pivot / threads;

	return pivot;
}
*/

basetype choosePivot2(basetype a[], int n, int threads){
	int pivot = 0;
	int limit = 100*threads;
	for(int i = 0; i < n && i<limit; i++){
		pivot += a[i];
	}
	if(n < limit){
		pivot = pivot / n;
	} else {
		pivot = pivot / limit;
	}
	return pivot;
}

void partition(basetype a[], basetype aa[], int n, int *n1, int *n2, int threads){
    //int pivot_index = rand() % n;
	//basetype pivot = a[pivot_index];
	//basetype pivot = (a[0] + a[n-1] + a[n/2] + a[n/4] + a[n/6] + a[n/8]) / 6;
	basetype pivot = choosePivot2(a,n,threads);

    #ifdef VERBOSE
	printf("pivot: "FS", index: %i\n", pivot, pivot_index);
    #endif
	assert(n >= threads);

	int *num_smaller_elems = (int*)malloc(threads*sizeof(int));
	int *num_eq_elems = (int*)malloc(threads*sizeof(int));
	int *num_larger_elems = (int*)malloc(threads*sizeof(int));

	(void) memset(num_smaller_elems,0,threads*sizeof(int));
	(void) memset(num_eq_elems,0,threads*sizeof(int));
	(void) memset(num_larger_elems,0,threads*sizeof(int));

	int *prefix_smaller = (int*)malloc(threads*sizeof(int));
	int *prefix_eq = (int*)malloc(threads*sizeof(int));
	int *prefix_larger = (int*)malloc(threads*sizeof(int));
	
	//sort local blocks
	(void) local_partition(a,aa,n,threads,pivot,num_smaller_elems,num_eq_elems,num_larger_elems);

	//calculate ex-prefix-sums for indices
	(void) memcpy(prefix_smaller,num_smaller_elems, sizeof(int)*threads);
	(void) memcpy(prefix_eq,num_eq_elems, sizeof(int)*threads);
	(void) memcpy(prefix_larger,num_larger_elems, sizeof(int)*threads);

	(void) exsum_seq(prefix_smaller, 0, threads);
	(void) exsum_seq(prefix_eq, 0, threads);
	(void) exsum_seq(prefix_larger, 0, threads);

	//cumulate prefixes
	for(int i=0; i<threads; i++){
		prefix_eq[i] += prefix_smaller[threads-1] + num_smaller_elems[threads-1];
	}
	for(int i=0; i<threads; i++){
		prefix_larger[i] += prefix_eq[threads-1] + num_eq_elems[threads-1];
	}

	/*
	printf("##########\n");
	printf("sm: "); printArray(num_smaller_elems,threads);
	printf("eq: "); printArray(num_eq_elems,threads);
	printf("la: "); printArray(num_larger_elems,threads);
	printf("sm pr: "); printArray(prefix_smaller,threads);
	printf("eq pr: "); printArray(prefix_eq,threads);
	printf("la pr: "); printArray(prefix_larger,threads);
	printf("##########\n");
	*/

	//sort globally into array
	(void) global_partition(a,aa,n,threads,pivot,num_smaller_elems,num_eq_elems,num_larger_elems,prefix_smaller,prefix_eq,prefix_larger);

	//printArray(aa,n);
	
	//verify partitioning
	/*
	for (int i=0; i<prefix_eq[0]; i++){
		assert(a[i]<pivot);
	}

	for (int i=prefix_eq[0]; i<prefix_larger[0]; i++){
		assert(a[i] == pivot);
	}

	for (int i=prefix_larger[0]; i<n; i++){
		assert(a[i]>pivot);
	}
	*/

	//index of last smaller element
	*n1 = prefix_eq[0] - 1;
	//index of first larger element
	*n2 = prefix_larger[0];

	
	free(num_smaller_elems);
	free(num_eq_elems);
	free(num_larger_elems);
	free(prefix_smaller);
	free(prefix_eq);
	free(prefix_larger);
}

void assign_threads(int n1, int n2, int n, int threads, int *left_threads, int *right_threads){
    //all elements, except the elements between n1 and n2 (equal elements)
    int size = n - ((n2-n1)-1);
    
    double left_frac = (double) (n1 + 1) / size;
    double right_frac = (double) (n - n2) / size;

    int left_t = round(left_frac * (double) threads);
    int right_t = round(right_frac * (double) threads);

    //every non-emtpy block must be handled by at least one thread
    if(left_t == 0 && left_frac != 0){
        left_t = 1;
        right_t = floor(right_frac * (double) threads);
    }
    if(right_t == 0 && right_frac != 0){
        right_t = 1;
        left_t = floor(left_frac * (double) threads);
    }

    //correct rounding errors
    if(left_t + right_t == threads+1){
        if(left_t == 1){
            right_t--;
        } else {
            left_t--;
        }
    }

    #ifdef VERBOSE
    printf("left_frac: %f, right_frac: %f\n", left_frac, right_frac);
    #endif
    assert(left_t != 0 && right_t != 0);
    
    *left_threads = left_t;
    *right_threads = right_t;
}

void quicksort_par(basetype a[], basetype aa[], int n, int threads)
{
    if(n <= threads){
        quicksort_seq(a,n);
        return;
    }
	//n1: index of last smaller element, n2: index of first larger element
    int n1 = 0;
    int n2 = 0;

    if(threads == 1){
        #ifdef VERBOSE
		printf("sorting seq: %f\n", (double) n/n_total);
        #endif
        (void) quicksort_seq(a,n);
        return;
    }

    (void) partition(a, aa, n, &n1, &n2, threads);

    #ifdef VERBOSE
    printf("n1: %i, n2: %i, n: %i\n", n1, n2, n);
    #endif
    //if n1 or n2 is out of range by one, it indicates, that there are no smaller or larger elements
    assert(n1 >= -1);
    assert(n1 < n);
    assert(n2 >= 0);
    assert(n2 <= n);
    assert(n1 < n2);

    int left_t;
    int right_t;
	

    if(n1 == -1 && n2 == n){
        //if there are only elems, which are equal to the pivot
        return;
    } else if(n1 == -1){
        //if there are no smaller elems
        left_t = 0;
        right_t = threads;
    } else if(n2 == n){
        //if there are no larger elems
        left_t = threads;
        right_t = 0;
    } else {
        //if there are smaller and larger elems
        (void) assign_threads(n1, n2, n, threads, &left_t, &right_t);    
    }

    #ifdef VERBOSE
	printf("left_t: %i, right_t: %i, sum: %i, threads: %i\n", left_t, right_t, left_t+right_t, threads);
    #endif

    assert(left_t + right_t == threads);


    #pragma omp parallel default(none) shared(a,aa,n1,n2,n,left_t,right_t)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                if(n1 >= 0){
                    quicksort_par(a, aa, n1 + 1, left_t);
                }
            }
            #pragma omp section
            {
                if(n2 < n){
                    quicksort_par(&a[n2], &aa[n2], n - n2, right_t);
                }
            }
        }
    }

}


void omp_quick(basetype a[], basetype aa[], int n, int threads){
    if(threads < 1){
        printf("setting thread count from %i to %i\n", threads, omp_get_max_threads());
        threads = omp_get_max_threads();
    }
    omp_set_num_threads(threads);
    omp_set_nested(1);
	
	n_total = n;

    (void) quicksort_par(a,aa,n,threads);
}

