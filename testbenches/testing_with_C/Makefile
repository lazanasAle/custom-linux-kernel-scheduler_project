CC=gcc
CFLAGS=-Wall -static -O2
PROGS=schedTest syscallTest

all: $(PROGS)

schedTest: schedTestbench.c
	$(CC) $(CFLAGS) -o schedTest schedTestbench.c
	
syscallTest: testbench-syscalls.c
	$(CC) $(CFLAGS) -o syscallTest testbench-syscalls.c
	
clean:
	rm -rf schedTest syscallTest

