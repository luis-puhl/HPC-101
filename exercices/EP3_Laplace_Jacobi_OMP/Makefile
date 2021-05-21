CC=gcc -Wall -O3 -g

all: clean bin/laplace_seq bin/laplace_omp out/validate.txt report.txt

bin/laplace_seq: laplace_seq.c
	$(CC) $^ -o $@
bin/laplace_omp: laplace_omp.c
	$(CC) $^ -o $@ -fopenmp

GRID = $(subst out/validate-seq-omp-,,$(subst .txt,,$@))
validation = $(foreach grid,1 50 100,out/validate-seq-omp-$(grid).txt)
$(validation) out/validate-seq-omp-500.txt: bin/laplace_seq bin/laplace_omp
	./bin/laplace_seq $(GRID) 	2>$@.seq.err >$@.seq.txt
	./bin/laplace_omp $(GRID) 4 2>$@.omp.err >$@.omp.txt
	cat $@.seq.err $@.omp.err >$@
	-@echo diff >>$@
	-diff --suppress-common-lines -y $@.seq.txt $@.omp.txt >>$@
out/validate.txt: $(validation)
	cat $^ > $@

out/report-seq.txt: bin/laplace_seq
	./bin/laplace_seq 300 2>$@ >$@.out
THREADS = $(subst out/report-,,$(subst .txt,,$@))
thread-reports = $(foreach thr,1 2 3 4,out/report-$(thr).txt)
$(thread-reports): bin/laplace_omp
	./bin/laplace_omp 300 $(THREADS) 2>$@ >$@.out
report.txt: out/report-seq.txt $(thread-reports) $(validation)
	grep executed $^ > $@

clean:
	rm -rf bin out report.txt
	mkdir bin out
