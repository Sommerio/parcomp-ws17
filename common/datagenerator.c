#include "datagenerator.h"

//source: https://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
long true_rand(long max) {
	unsigned long
	// max <= RAND_MAX < ULONG_MAX, so this is okay.
			num_bins = (unsigned long) max + 1,
			num_rand = (unsigned long) RAND_MAX + 1,
			bin_size = num_rand / num_bins,
			defect   = num_rand % num_bins;

	long x;
	do {
		x = random();
	}
		// This is carefully written not to overflow
	while (num_rand - defect <= (unsigned long)x);

	// Truncated division is intentional
	return x/bin_size;
}

void gen_random_int_array(int a[], int size){
	for(int i=0; i<size; i++) a[i] = ((int)true_rand(size*2))-size;
}

void gen_random_uint_array(int a[], int size){
	for(int i=0; i<size; i++) a[i] = (int)true_rand(size);
}

void gen_asc_int_array(int a[], int size){
	int offset = (int)true_rand(size);
	for(int i=0; i<size; i++) a[i] = i - offset;
}

void gen_desc_int_array(int a[], int size){
	int offset = (int)true_rand(size);
	for(int i=0; i<size; i++) a[i] = (size-i) - offset;
}

void gen_eq_int_array(int a[], int size){
	for(int i=0; i<size; i++) a[i] = -1;
}

void gen_random_double_array(double a[], int size){
	for(int i=0; i<size; i++) a[i] = (double)random()/RAND_MAX*2.0-1.0;
}

void gen_asc_double_array(double a[], int size){
	int offset = (int)true_rand(size);
	int offset2 = (double)random()/RAND_MAX*2.0-1.0;
	for(int i=0; i<size; i++) a[i] = i - offset + offset2;
}

void gen_desc_double_array(double a[], int size){
	int offset = (int)true_rand(size);
	int offset2 = (double)random()/RAND_MAX*2.0-1.0;
	for(int i=0; i<size; i++) a[i] = (size-i) - offset + offset2;
}

void gen_eq_double_array(double a[], int size){
	for(int i=0; i<size; i++) a[i] = -1.0;
}
