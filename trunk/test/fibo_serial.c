/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "swift_compiler.h"

int NO_INLINE sum(int r1, int r2)
{
	return r1 + r2;
}

int NO_INLINE fibo(int n)
{
	if (n < 2) {
		return n;
	} else {
		int r1, r2;

		r1 = fibo(n-1);
		r2 = fibo(n-2);
		return sum(r1, r2);
	}
}

int main(int argc, char **argv) {
	int n, r;

	if (argc != 2) {
		fprintf(stderr, "usage: %s N\n", argv[0]);
		abort();
	}

	n = strtol(argv[1], NULL, 10);
	r = fibo(n);

	fprintf(stderr, "computed result for fibo(%d)=%d\n", n, r);

	return 0;
}
