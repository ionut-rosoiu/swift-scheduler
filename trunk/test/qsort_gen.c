/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>

extern void write_array(char *filename, int N);

int main(int argc, char **argv) {
	int n;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <input_file> <N>\n", argv[0]);
		abort();
	}

	n = strtol(argv[2], NULL, 10);
	write_array(argv[1], n);

	printf("wrote file %s with %d numbers\n", argv[1], n);

	return 0;
}
