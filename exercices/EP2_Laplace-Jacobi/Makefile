CC=gcc -Wall -g

all: clean laplace_seq laplace_pth validate.txt

laplace_seq: laplace.c laplace_seq.c
	$(CC) $^ -o $@
laplace_org: laplace.c laplace_org.c
	$(CC) $^ -o $@

laplace_pth: laplace.c laplace_pth.c
	$(CC) $^ -o $@ -lpthread
laplace_que: laplace.c laplace_que.c
	$(CC) $^ -o $@ -lpthread

laplace_omp: laplace_omp.c
	$(CC) $^ -o $@ -fopenmp

GRID = $(subst validate-seq-pth-,,$(subst .txt,,$@))
validate-seq-pth-5.txt validate-seq-pth-50.txt validate-seq-pth-500.txt: laplace_seq laplace_pth
	./laplace_seq $(GRID) 	2>seq-$(GRID).err >seq-$(GRID).txt
	./laplace_que $(GRID) 4 2>que-$(GRID).err >que-$(GRID).txt
	./laplace_pth $(GRID) 4 2>pth-$(GRID).err >pth-$(GRID).txt
	-diff --suppress-common-lines -y seq-$(GRID).txt pth-$(GRID).txt >$@
validate.txt: validate-seq-pth-5.txt validate-seq-pth-50.txt validate-seq-pth-50.txt
	cat $^ > $@

THREADS = $(subst report-,,$(subst .txt,,$@))
report-1.txt report-2.txt report-3.txt report-4.txt: laplace_que
	./laplace_que 100 $(THREADS) 2>$@.err >$@
report-org.txt: laplace_org
	./laplace_org 100 2>$@.err >$@
report.txt: report-org.txt report-1.txt report-2.txt report-3.txt report-4.txt
	grep executed $^ > $@

clean:
	rm -f laplace_seq laplace_pth laplace_omp validate* *.txt *.err
