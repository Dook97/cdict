.PHONY: all clean

all: test

test: lib/avl.c lib/avl.h test.c
	cc -O3 -o $@ -I./lib lib/avl.c test.c
clean:
	rm test
