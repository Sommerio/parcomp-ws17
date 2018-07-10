CC = gcc
CFLAGS = -Wall -fcilkplus -O3
CFLAGS_SEQ = -Wall -DSEQUENTIAL -O3
OBJECTFILES = *.bin

.PHONY: all clean

all: cilkqsort

cilkqsort: cilkqsort_main.c cilkqsort.c ../common/common.c ../common/datagenerator.c ../common/quicksort-seq.c
	$(CC) $(CFLAGS) -o $@.bin $^

cilkqsort-seq: cilkqsort_main.c cilkqsort.c ../common/common.c ../common/datagenerator.c ../common/quicksort-seq.c
	$(CC) $(CFLAGS_SEQ) -o $@.bin $^

clean:
	rm -f $(OBJECTFILES)
