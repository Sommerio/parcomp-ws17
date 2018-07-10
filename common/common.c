#include "common.h"

usecs mytime()
{
	struct timeval now;
	gettimeofday(&now,NULL);
	return (usecs)now.tv_usec+(usecs)now.tv_sec*1000000L;
}

int choose_pivot(int *a, int len) {
    return a[len/2];
}


void printArray(basetype a[], int n){
	for(int i=0; i<n; i++){
		fprintf(stdout, FS",", a[i]);
	}
	fprintf(stdout, "\n");
}


void exsum_seq(int x[], int start, int n){
	int sum = 0, prev = 0;

	for(int i=start; i<n; i++){
		sum += prev;
		prev = x[i];
		x[i] = sum;
	}
}


void partition_seq(basetype a[], int n, basetype pivot){
	int i, j;
	for (i = 0, j = n-1; ; i++) {
		//additional check if i and j are in [0..n] 
		//(it's possible that a doesn't contain the pivot)
		while (a[i] <= pivot && i < n) i++;
		while (a[j] > pivot && j > 0) j--;

		if (i >= j) break;

		//swap element
		basetype temp = a[i];
		a[i]     = a[j];
		a[j]     = temp;
	}
}

void fill_data(basetype* a, int n, int s) {
    #ifdef INT
    if (s==0) {
        gen_random_int_array(a,n);
    } else if (s==1) {
        gen_asc_int_array(a,n);
    } else if (s==2) {
        gen_desc_int_array(a,n);
    } else {
        gen_eq_int_array(a,n);
    }
    #endif
    
    #ifdef DOUBLE
    if (s==0) {
        gen_random_double_array(a,n);
    } else if (s==1) {
        gen_asc_double_array(a,n);
    } else if (s==2) {
        gen_desc_double_array(a,n);
    } else {
        gen_eq_double_array(a,n);
    }
    #endif
}
