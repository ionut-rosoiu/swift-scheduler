/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "swift_compiler.h"

int  sum(int r1, int r2)
{
    return r1 + r2;
}

int  fibo(int n)
{
    int x, y;
    
    if (n < 2) {
        return n;
   }
   
    #pragma omp task shared(x)
    x = fibo(n - 1);
    
    #pragma omp task shared(y)
    y = fibo(n - 2);
    
    #pragma omp taskwait
   return x + y;
}

int main(int argc, char **argv) {
	int n, r;

	if (argc != 2) {
		fprintf(stderr, "usage: %s N\n", argv[0]);
		abort();
	}

	n = strtol(argv[1], NULL, 10);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
        r = fibo(n);
        }
    }

	fprintf(stderr, "computed result for fibo(%d)=%d\n", n, r);

	return 0;
}
