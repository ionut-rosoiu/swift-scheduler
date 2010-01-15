/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "swift.h"

#define NUM_THREADS 	2
#define MAX_VAL			100

swift_deque_t deque;
int vals[MAX_VAL];
swift_frame_t frames[MAX_VAL];

void* pusher(void *arg) {
	swift_status_t status;
	int i, j = 0;

	printf("pusher started.\n");
	for (i=0; i<MAX_VAL; i++) {
		do {
			swift_deque_push(&deque, &frames[j++], &status);
		} while (status == SWIFT_DEQUE_FULL);
	}
	printf("pusher finished.\n");

	return arg;
}

void* popper(void *arg) {
	swift_status_t status;
	swift_frame_t *frame;
	int i;

	printf("popper started.\n");
	for (i=0; i<MAX_VAL; i++) {
		frame = swift_deque_pop(&deque, &status);
		if (frame) {
			vals[frame->dependencies_no] = 1;
		}
	}
	printf("popper finished.\n");

	return arg;
}

void* stealer(void *arg) {
	swift_status_t status;
	swift_thread_t *thread = arg;
	swift_frame_t *frame;
	int i;

	printf("stealer %d started\n", thread->id);
	for (i=0; i<MAX_VAL + 1; i++) {
		do {
			frame = swift_deque_steal(&deque, &status);
		} while (status == SWIFT_DEQUE_ABORT);

		if (frame) {
			printf("[S]    pop %d (%p)\n", frame->dependencies_no, (void*)frame);
			vals[frame->dependencies_no] = 1;
		}
	}
	printf("stealer %d finished.\n", thread->id);

	return arg;
}

swift_entrypoint_t funcs[4] = {
		pusher, // must be first
		popper,
		stealer,
		stealer
};

int main() {
	swift_thread_t threads[NUM_THREADS];
	int i;

	//swift_assert(NUM_THREADS == 4);

	/* initialize the deque */
	swift_deque_init(&threads[0], &deque, MAX_VAL);

	/* initialize the value vector */
	for (i=0; i<MAX_VAL; i++) {
		frames[i].dependencies_no = i;
		vals[i] = 0;
	}

	/* start the threads */
	for (i=0; i<NUM_THREADS; i++) {
		threads[i].id = i;

		swift_thread_start(&threads[i], funcs[i]);
	}

	printf("cleaning-up...\n");

	/* clean-up */
	for (i=0; i<NUM_THREADS; i++) {
		swift_thread_wait(&threads[i]);
		printf("thread %d done.\n", i);
	}

	for (i=0; i<MAX_VAL; i++) {
		printf("%d ", i);
		swift_assert(vals[i]);
	}

	swift_deque_destroy(&threads[0], &deque);

	return 0;
}
