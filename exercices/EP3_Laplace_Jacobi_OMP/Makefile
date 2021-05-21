CC=gcc -Wall -O3 -g

all: clean bin/laplace_seq bin/laplace_omp out/validate.txt out/report.txt

bin/laplace_seq: laplace_seq.c
	$(CC) $^ -o $@
bin/laplace_omp: laplace_omp.c
	$(CC) $^ -o $@ -fopenmp

GRID = $(subst out/validate-seq-omp-,,$(subst .txt,,$@))
out/validate-seq-omp-5.txt out/validate-seq-omp-50.txt out/validate-seq-omp-500.txt: bin/laplace_seq bin/laplace_omp
	./bin/laplace_seq $(GRID) 	2>$@.seq-$(GRID).err >$@.seq-$(GRID).txt
	./bin/laplace_omp $(GRID) 4 2>$@.omp-$(GRID).err >$@.omp-$(GRID).txt
	-diff --suppress-common-lines -y $@.seq-$(GRID).txt $@.omp-$(GRID).txt >$@
out/validate.txt: out/validate-seq-omp-5.txt out/validate-seq-omp-50.txt out/validate-seq-omp-50.txt
	cat $^ > $@

THREADS = $(subst out/report-,,$(subst .txt,,$@))
out/report-1.txt out/report-2.txt out/report-3.txt out/report-4.txt: bin/laplace_omp
	./bin/laplace_omp 100 $(THREADS) 2>$@.err >$@
out/report.txt: out/report-1.txt out/report-2.txt out/report-3.txt out/report-4.txt
	grep executed $^ > $@

clean:
	rm -rf bin out
	mkdir bin out
