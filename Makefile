CC = gcc
CFLAGS = -Wall -O3
TYPE = INT
LIBS = -fopenmp -fcilkplus -lm
OBJECTFILES = *.bin
COMMONFILES = common/common.c common/datagenerator.c common/quicksort-seq.c

.PHONY: all clean

all: tests benchmark mpitests mpibenchmark

tests: tests.c $(COMMONFILES) openmp/ompquick.c cilk/cilkqsort.c
	$(CC) $(CFLAGS) -D$(TYPE) -o $@.bin $^ $(LIBS)

mpitests: tests_mpi.c $(COMMONFILES) mpi/mpiquick.c
	mpicc -std=gnu99 $(CFLAGS) -D$(TYPE) -o $@.bin $^ -lm

benchmark: benchmark_sm.c $(COMMONFILES) openmp/ompquick.c cilk/cilkqsort.c
	$(CC) $(CFLAGS) -D$(TYPE) -DNDEBUG -o $@.bin $^ $(LIBS)

mpibenchmark: benchmark_mpi.c $(COMMONFILES) mpi/mpiquick.c
	mpicc -std=gnu99 $(CFLAGS) -D$(TYPE) -o $@.bin $^ -lm

clean:
	rm -f $(OBJECTFILES)
