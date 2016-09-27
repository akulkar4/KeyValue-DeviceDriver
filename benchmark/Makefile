all: benchmark

benchmark: benchmark.c
	$(CC) -g -O3 benchmark.c -o benchmark -lckv
clean:
	rm -f benchmark
