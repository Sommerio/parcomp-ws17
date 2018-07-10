TESTS_BIN=./tests.bin
BENCHMARK_BIN=./benchmark.bin
BENCHMARKS_DIR=../benchmarks

run_benchmark_seq() {
	echo "Testing sequential with n = ${1}, ${2} iterations..."
	$BENCHMARK_BIN -n $1 -i $2 -o > "${3}_seq_n${1}"
}

run_benchmark_omp() {
	echo "Testing OpenMP with n = ${1}, ${2} iterations, ${3} threads..."
	$BENCHMARK_BIN -n $1 -i $2 -t $3 -p -a > "${4}_omp_n${1}_t${3}"
}

run_benchmark_cilk() {
	echo "Testing Cilk with n = ${1}, ${2} iterations, ${3} threads, cutoff: {4} ..."
	$BENCHMARK_BIN -n $1 -i $2 -t $3 -c $4 -p -b > "${5}_cilk_n${1}_t${3}_c${4}"
}

run() {
	echo "----- STARTING BENCHMARK FOR DATATYPE ${1} -----"
	PREFIX="${BENCHMARKS_DIR}/${1}"
	
	run_benchmark_seq 2000000 30 "${PREFIX}"
	run_benchmark_seq 5000000 20 "${PREFIX}"
	run_benchmark_seq 10000000 15 "${PREFIX}"
	run_benchmark_seq 20000000 15 "${PREFIX}"
	run_benchmark_seq 50000000 10 "${PREFIX}"
	run_benchmark_seq 100000000 5 "${PREFIX}"
	run_benchmark_seq 200000000 5 "${PREFIX}"

	run_benchmark_omp 1000000 50 1 "${PREFIX}"
	run_benchmark_cilk 1000000 50 1 4096 "${PREFIX}"
	run_benchmark_omp 1000000 50 2 "${PREFIX}"
	run_benchmark_cilk 1000000 50 2 4096 "${PREFIX}"
	run_benchmark_omp 1000000 50 4 "${PREFIX}"
	run_benchmark_cilk 1000000 50 4 4096 "${PREFIX}"
	
	run_benchmark_omp 1000000 50 20 "${PREFIX}"
	run_benchmark_cilk 1000000 50 20 512 "${PREFIX}"
	run_benchmark_omp 2000000 50 20 "${PREFIX}"
	run_benchmark_cilk 2000000 50 20 1024 "${PREFIX}"
	run_benchmark_omp 5000000 30 20 "${PREFIX}"
	run_benchmark_cilk 5000000 30 20 1024 "${PREFIX}"
	run_benchmark_omp 10000000 30 20 "${PREFIX}"
	run_benchmark_cilk 10000000 30 20 2048 "${PREFIX}"
	run_benchmark_omp 20000000 25 20 "${PREFIX}"
	run_benchmark_cilk 20000000 25 20 2048 "${PREFIX}"
	run_benchmark_omp 50000000 20 20 "${PREFIX}"
	run_benchmark_cilk 50000000 20 20 4096 "${PREFIX}"
	run_benchmark_omp 100000000 15 20 "${PREFIX}"
	run_benchmark_cilk 100000000 15 20 8192 "${PREFIX}"
	run_benchmark_omp 20000000 15 20 "${PREFIX}"
	run_benchmark_cilk 20000000 15 20 8192 "${PREFIX}"
	
	echo "Testing different amounts of threads in OpenMP..."
	$BENCHMARK_BIN -n 10000000 -i 20 -T 2 35 -p -a > "${PREFIX}_omp_n10000000_i20_t2-35"
	
	echo "Testing different amounts of threads in Cilk..."
	$BENCHMARK_BIN -n 10000000 -i 20 -T 2 35 -p -b > "${PREFIX}_omp_n10000000_i20_t2-35"
}

echo "Sleeping for ${1} seconds..."
sleep $1

echo "testing integer-based sorting..."

make all TYPE=INT
if [ "$?" != "0" ]; then
	echo "----------------------------------------------"
	echo "----------- ERROR: MAKE FAILED ---------------"
	echo "----------------------------------------------"
	exit 1
fi
$TESTS_BIN
if [ "$?" != "0" ]; then
	echo "----------------------------------------------"
	echo "----------- ERROR: TEST FAILED ---------------"
	echo "----------------------------------------------"
	exit 1
fi

run INT

echo "testing double-based sorting..."

make all TYPE=DOUBLE
if [ "$?" != "0" ]; then
	echo "----------------------------------------------"
	echo "----------- ERROR: MAKE FAILED ---------------"
	echo "----------------------------------------------"
	exit 1
fi
$TESTS_BIN
if [ "$?" != "0" ]; then
	echo "----------------------------------------------"
	echo "----------- ERROR: TEST FAILED ---------------"
	echo "----------------------------------------------"
	exit 1
fi

run DOUBLE

echo "----------------------------------------------"
echo "----------- BENCHMARK FINISHED ---------------"
echo "----------------------------------------------"
exit 0