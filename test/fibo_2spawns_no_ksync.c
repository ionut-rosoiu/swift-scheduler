/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "swift.h"

int N;

typedef struct fibo_data {
	int n;
	int *r;
	// sync related
	swift_size_t sync_frames_remaining;
	char _pad[SWIFT_CACHE_LINE_SIZE - sizeof(swift_size_t)];
} fibo_data_t;

/* forward declaration for the fibo() */
void fibo(swift_thread_t *thread, swift_frame_t *frame);


/* void fibo(int r, shared_w int* r) */
void fibo(swift_thread_t *thread, swift_frame_t *frame) {
	fibo_data_t *data = frame->private_data;
	swift_status_t status;
	swift_size_t n;

	SWIFT_LOG_FRAME_INFO_STR("\nfibo() ", thread, frame);

	if (data->n < 2) {
		*data->r = data->n;
		data->sync_frames_remaining = 0;

	} else {
		// fibo(n-1) data
		swift_frame_t *frame1;
		fibo_data_t *data1;

		// fibo(n-2) data
		swift_frame_t *frame2;
		fibo_data_t *data2;

		int r1, r2;

		/* spawn fibo(n-1, r1); */
		frame1 = (swift_frame_t*) swift_malloc(thread, sizeof(swift_frame_t) + sizeof(fibo_data_t), &status);
		frame1->closure = fibo;
#ifdef LOGGING_ON
		frame1->creator_id = thread->id;
		frame1->dbg = data->n - 1;
#endif

		data1 = (fibo_data_t*) ((char*)frame1 + sizeof(swift_frame_t));
		data1->n = data->n - 1;
		data1->r = &r1;
		frame1->private_data = data1;

		SWIFT_WRITE_FRAME_INFO(frame1, thread->id, 'f', thread->frame_no++);
		// sync related
		frame1->dependencies_frame = NULL;
		frame1->sync_frames_remaining = &data->sync_frames_remaining;

		/* spawn fibo(n-2, r2); */
		frame2 = (swift_frame_t*) swift_malloc(thread, sizeof(swift_frame_t) + sizeof(fibo_data_t), &status);
		frame2->closure = fibo;
#ifdef LOGGING_ON
		frame2->creator_id = thread->id;
		frame2->dbg = data->n - 2;
#endif

		data2 = (fibo_data_t*) ((char*)frame2 + sizeof(swift_frame_t));
		data2->n = data->n - 2;
		data2->r = &r2;
		frame2->private_data = data2;

		SWIFT_WRITE_FRAME_INFO(frame2, thread->id, 'f', thread->frame_no++);
		frame2->dependencies_frame = NULL;
		frame2->sync_frames_remaining = &data->sync_frames_remaining;

		// wait for 1,2 to finish
		data->sync_frames_remaining = 2;

		// add 1 & 2 in the workque
		swift_deque_push(&thread->workque, frame1, &status);
		swift_deque_push(&thread->workque, frame2, &status);

		*data->r = r1 + r2;
	}

	SWIFT_LOG(INFO, "[%d] __fibo(%d)=%d\n", thread->id, data->n, *data->r);

	swift_signal_frame_done(thread, frame, &status);
	swift_frame_done(thread->context, thread, frame, &status);
}

void* thread_start(void *arg) {
	swift_thread_t *thread = arg;
	swift_status_t status;
	int i = 0;

	SWIFT_LOG(INFO, "_____started thread %d\n", thread->id);

	while (i++ <= N) {
		swift_scheduler_execute(thread, &status);
	}

	SWIFT_LOG(INFO, "_____finished thread %d: %d frames processed\n", thread->id, thread->stats);
	printf("_____finished thread %d: %d frames processed\n", thread->id, thread->stats);

	return arg;
}

int main(int argc, char **argv) {
	swift_context_t context;
	swift_status_t status;
	swift_frame_t *frame;
	fibo_data_t *data;
	int i, r;
	int nthreads, n;

	if (argc != 3) {
		fprintf(stderr, "usage: %s Nthreads N\n", argv[0]);
		abort();
	}

	nthreads = strtol(argv[1], NULL, 10);
	n = strtol(argv[2], NULL, 10);
	N = n;

	SWIFT_LOG(INFO, "Started nthreads=%d n=%d\n", nthreads, n);

	/* initialize the context */
	swift_context_init(&context, nthreads, &status);

	/* push the first frame */
	frame = (swift_frame_t*) swift_malloc(&context.threads[0], sizeof(swift_frame_t) + sizeof(fibo_data_t), &status);
	SWIFT_FRAME_SET_END_PARALLEL(frame);
	frame->creator_id = 0;
	frame->closure = fibo;
	frame->dbg = n;

	data = (fibo_data_t*) ((char*)frame + sizeof(swift_frame_t));
	data->n = n;
	data->r = &r;
	frame->private_data = data;

	SWIFT_WRITE_FRAME_INFO(frame, 0, 'f', context.threads[0].frame_no++);
	SWIFT_LOG_FRAME_INFO((&context.threads[0]), frame);

	// no wait frame
	frame->dependencies_no = 0;
	frame->dependencies_frame = NULL;
	frame->sync_frames_remaining = &data->sync_frames_remaining; // doesn't matter

	swift_deque_push(&context.threads[0].workque, frame, &status);
	SWIFT_LOG(INFO, "pushed first frame\n");


	/* start the threads */
	for (i=0; i<nthreads; i++) {
		swift_thread_start(&context.threads[i], thread_start, &status);
	}

	/* wait for the threads to finish */
	for (i=0; i<nthreads; i++) {
		SWIFT_LOG(INFO, "_____waiting %d\n", i);
		swift_thread_wait(&context.threads[i], &status);
		SWIFT_LOG(INFO, "_____waiting %d done\n", i);
	}

	/* check the result */
	printf("computed result for fibo(%d)=%d\n", n, r);
	swift_context_destroy(&context, &status);

	return 0;
}
