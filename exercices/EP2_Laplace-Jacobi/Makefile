CC=gcc -Wall -g

all: clean laplace_seq laplace_pth validate.txt

laplace_seq: laplace_seq.c
	$(CC) $^ -o $@

laplace_pth: laplace_pth.c
	$(CC) $^ -o $@ -lpthread

laplace_omp: laplace_omp.c
	$(CC) $^ -o $@ -fopenmp

GRID = $(subst validate-seq-pth-,,$(subst .txt,,$@))
validate-seq-pth-5.txt validate-seq-pth-50.txt validate-seq-pth-500.txt: laplace_seq laplace_pth
	./laplace_seq $(GRID) >seq-$(GRID).txt
	./laplace_pth $(GRID) 4 2>pth-$(GRID).err >pth-$(GRID).txt
	cat pth-$(GRID).txt pth-$(GRID).err
	-diff --suppress-common-lines -y seq-$(GRID).txt pth-$(GRID).txt >$@
validate.txt: validate-seq-pth-5.txt validate-seq-pth-50.txt validate-seq-pth-50.txt
	cat $^ > $@

clean:
	rm -f laplace_seq laplace_pth laplace_omp validate* *.txt *.err
