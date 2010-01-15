/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "swift_compiler.h"

extern void read_array(char *filename, int **arr, int *n);
extern void free_array(int *arr);
extern void print_array(int *arr, int n);
extern void check_array(int *arr, int n);

void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

int partition(int *a, int l, int r)
{
	int pivot = a[l];
	int i = l;
	int j = r + 1;

	while (1) {
		do {
			++i;
		} while (a[i] <= pivot && i <= r);

		do {
			--j;
		} while (a[j] > pivot);

		if (i >= j) {
			break;
		}

		swap(&a[i], &a[j]);
	}

	swap(&a[l], &a[j]);
	return j;
}

void qs(int *a, int l, int r)
{
	int pivot;

	if (l < r) {
		pivot = partition(a, l, r);
		qs(a, l, pivot-1);
		qs(a, pivot+1, r);
	}
}

int quicksort(int *a, int size)
{
	qs(a, 0, size - 1);
	return 0;
}

int main(int argc, char **argv)
{
	int n, r, *a;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <input_file>\n", argv[0]);
		abort();
	}

	read_array(argv[1], &a, &n);
	print_array(a, n);

	r = quicksort(a, n);

	check_array(a, n);

	free_array(a);

	printf("done.\n");
	return 0;
}
