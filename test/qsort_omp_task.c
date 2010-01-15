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

extern void read_array(char *filename, int **arr, int *n);
extern void free_array(int *arr);
extern void check_array(int *arr, int n);

int partition (int *data, int p, int r)
{
    int x = data[p];
    int k = p;
    int l = r+1;
    int t;

    while (1) {
        do k++; while ((data[k] <= x) && (k < r));
        do l--; while (data[l] > x);
        while (k < l) {
            t = data[k];  data[k] = data[l];  data[l] = t;
            do k++; while (data[k] <= x);
            do l--; while (data[l] > x);
        }
        t = data[p];  data[p] = data[l];  data[l] = t;
        return l;
    }
}

void quicksort_omp_par (int *data, int p, int r)
{
    if (p < r) {
            int q = partition (data, p, r);

            #pragma omp task
            quicksort_omp_par(data, p, q-1);

            #pragma omp task
            quicksort_omp_par(data, q+1, r);

			#pragma omp taskwait
    }
}

void quicksort(int *arr, int n)
{
	quicksort_omp_par(arr, 0, n);
}

int main(int argc, char **argv)
{
	int n, *a;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <input_file>\n", argv[0]);
		abort();
	}

	read_array(argv[1], &a, &n);

	quicksort(a, n);

	check_array(a, n);

	free_array(a);

	printf("done.\n");
	return 0;
}
