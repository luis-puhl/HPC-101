# Flag for implicit rules
CC=mpicc
# Flag for implicit rules. Turn on debug info
CFLAGS = -Wall -g

.PHONY: all clean
all: clean bin/hello hello.out bin/send-rcv send-rcv.out
clean:
	-rm -rf bin *.out

bin/%: %.c
	-@mkdir bin 2> /dev/null
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@

%.out: bin/%
	-./$^ | tee $@
	-mpirun $^ | tee -a $@
