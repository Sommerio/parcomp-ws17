#include "quicksort-seq.h"

// recursive sequential quicksort
void quicksort_seq(basetype *A, int len) {
	if (len < 2) return;

	basetype pivot = A[len / 2];

	int i, j;
	for (i = 0, j = len - 1; ; i++, j--) {
		while (A[i] < pivot) i++;
		while (A[j] > pivot) j--;

		if (i >= j) break;

		//swap elements
		basetype temp = A[i];
		A[i]     = A[j];
		A[j]     = temp;
	}

	quicksort_seq(A, i);
	quicksort_seq(A + i, len - i);
}
