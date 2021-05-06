CC=gcc

all: clean sequential pthread openmp

sequential:
	$(CC) pi_seq.c -o pi_seq

pthread:
	$(CC) pi_pth.c -o pi_pth -lpthread

openmp:
	# For Linux systems
	$(CC) pi_omp.c -o pi_omp -fopenmp
	# For MacOS with clang
	# /usr/local/opt/llvm/bin/clang pi_omp.c -o pi_omp -fopenmp -L/usr/local/opt/llvm/lib

pi-int.sif: container.def
	sudo singularity build pi-int.sif container.def

ifndef $(OMP_NUM_THREADS)
export OMP_NUM_THREADS=8
endif
run.txt: pi-int.sif
	@echo "OMP_NUM_THREADS=$(OMP_NUM_THREADS)" | tee run.txt
	@echo "*** SEQUENTIAL ***" | tee -a run.txt
	sudo singularity run pi-int.sif pi_seq 100000000 | tee -a run.txt
	@echo "*** PTHREAD ***" | tee -a run.txt
	sudo singularity run pi-int.sif pi_pth 100000000 $(OMP_NUM_THREADS) | tee -a run.txt
	@echo "*** OPENMP ***" | tee -a run.txt
	sudo singularity run pi-int.sif pi_omp 100000000 | tee -a run.txt
	

clean:
	rm -f pi_seq pi_pth pi_omp
clean-run:
	rm -f run.txt
