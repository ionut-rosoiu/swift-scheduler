/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define MAX 100

void read_array(char *filename, int **a, int *n)
{
	FILE *f = fopen(filename, "rb");
	int *arr, fsize, N, i;

	if (f == NULL) {
		fprintf(stderr, "Cannot open file: %s", filename);
		abort();
	}

	*a = NULL;
	arr = NULL;

	fseek(f, 0, SEEK_END);
	fsize = (int) ftell(f);
	N = fsize / sizeof(int);
	fseek(f, 0, SEEK_SET);

	arr = (int *) malloc(N * sizeof(int));

	if (!arr) {
		fprintf(stderr, "Out of memory");
		abort();
	}

	*a = arr;
	*n = N;

	for (i=0; i<N; i++) {
		if (!fread(&arr[i], sizeof(int), 1, f)) {
			fprintf(stderr, "Error reading item %d\n", i);
		}
	}
}

void write_array(char *filename, int N)
{
	FILE *f = fopen(filename, "wb");
	int i, val;

	if (f == NULL) {
		fprintf(stderr, "Cannot open file: %s", filename);
		abort();
	}

	srand(time(NULL));

	for (i=0; i<N; i++) {
		val = rand() % MAX;
		fwrite(&val, sizeof(int), 1, f);
	}

	fclose(f);
}

void free_array(int *a)
{
	free(a);
}

void print_array(int *a, int n)
{
	int i = 0;
	for (i=0; i<n-1; i++) {
		printf ("%d.%d\n", i, a[i]);
	}
	printf("\n");
}

void check_array(int *a, int n)
{
	int i = 0;
	for (i=0; i<n-1; i++) {
		if (a[i] > a[i+1]) {
			fprintf(stderr, "Invalid array: at pos %d, elements %d %d", i, a[i], a[i+1]);
			abort();
		}
	}
}
