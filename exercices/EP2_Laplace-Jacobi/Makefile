CC=gcc

all: clean laplace_seq laplace_pth laplace_omp

laplace_seq: laplace_seq.c
	$(CC) $^ -o $@

laplace_pth: laplace_pth.c
	$(CC) $^ -o $@ -lpthread

laplace_omp: laplace_omp.c
	$(CC) $^ -o $@ -fopenmp

clean:
	rm -f laplace_seq laplace_pth laplace_omp
