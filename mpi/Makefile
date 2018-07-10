CC = mpicc
CFLAGS = -std=gnu99 -Wall -O3
TYPE = INT
LIBS = -lm
OBJECTFILES = *.bin
COMMONFILES = ../common/common.c ../common/datagenerator.c ../common/quicksort-seq.c 

.PHONY: all clean

mpiquick: mpiquick.c $(COMMONFILES)
	$(CC) $(CFLAGS) -D$(TYPE) -o $@.bin $^ $(LIBS)

clean:
	rm -f $(OBJECTFILES)
