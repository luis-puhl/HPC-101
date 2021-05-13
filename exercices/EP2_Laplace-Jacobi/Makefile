CC=gcc

all: clean laplace_seq laplace_pth laplace_omp

laplace_seq: laplace_seq.c
	$(CC) $^ -o $@

laplace_pth: laplace_pth.c
	$(CC) $^ -o $@ -lpthread

laplace_omp: laplace_omp.c
	$(CC) $^ -o $@ -fopenmp

seq.txt: laplace_seq
	./$^ 5 > $@
pth.txt: laplace_pth
	./$^ 5 4 > $@

validate.txt: seq.txt pth.txt
	-diff --suppress-common-lines -y $^ > $@

clean:
	rm -f laplace_seq laplace_pth laplace_omp
