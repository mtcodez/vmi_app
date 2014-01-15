all: test

test: vmi_test.c
	gcc vmi_test.c -o test -lxenstat -g
clean:
	rm -f test
