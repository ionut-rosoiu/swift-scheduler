/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "swift.h"

/* datastructures for the two closures */
typedef struct sum_data {
	int r1;
	int r2;
	int *r;
	char _pad[SWIFT_CACHE_LINE_SIZE - sizeof(int)];
} sum_data_t;

typedef struct fibo_data {
	int n;
	int *r;
	// sync related
	swift_size_t sync_frames_remaining;
	char _pad[SWIFT_CACHE_LINE_SIZE - sizeof(swift_size_t)];
} fibo_data_t;

/* forward declaration for the fibo() */
void fibo(swift_thread_t *thread, swift_frame_t *frame);

/* sum(r1, r2, *r) */
void sum(swift_thread_t *thread, swift_frame_t *frame) {
	swift_status_t status;
	sum_data_t *data = frame->private_data;

	SWIFT_LOG_FRAME_INFO_STR("\nsum() ", thread, frame);
	*data->r = data->r1 + data->r2;

	SWIFT_LOG(INFO, "[%d] sum(%d, %d)=%d\n", thread->id, data->r1, data->r2, *data->r);
	swift_signal_frame_done(thread, frame, &status);
}

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

		// sum() data
		swift_frame_t *frame3;
		sum_data_t *data3;

		// sum() frame & private data
		frame3 = (swift_frame_t*) swift_malloc(thread, sizeof(swift_frame_t) + sizeof(sum_data_t), &status);
		data3 = (sum_data_t*) ((char*)frame3 + sizeof(swift_frame_t));
		data3->r = data->r;

		/* spawn fibo(n-1, r1); */
		frame1 = (swift_frame_t*) swift_malloc(thread, sizeof(swift_frame_t) + sizeof(fibo_data_t), &status);
		frame1->closure = fibo;
#ifdef LOGGING_ON
		frame1->creator_id = thread->id;
		frame1->dbg = data->n - 1;
#endif

		data1 = (fibo_data_t*) ((char*)frame1 + sizeof(swift_frame_t));
		data1->n = data->n - 1;
		data1->r = &data3->r1;
		frame1->private_data = data1;

		SWIFT_WRITE_FRAME_INFO(frame1, thread->id, 'f', thread->frame_no++);
		// sync related
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
		data2->r = &data3->r2;
		frame2->private_data = data2;

		SWIFT_WRITE_FRAME_INFO(frame2, thread->id, 'f', thread->frame_no++);
		frame2->sync_frames_remaining = &data->sync_frames_remaining;

		/* spawn sum(r1, r2, r); */
		frame3->closure = sum;
#ifdef LOGGING_ON
		frame3->creator_id = thread->id;
		frame3->dbg = -data->n;
#endif
		frame3->private_data = data3;

		frame3->dependencies_no = 2;
		SWIFT_WRITE_FRAME_INFO(frame3, thread->id, 's', thread->frame_no++);
		frame3->sync_frames_remaining = &data->sync_frames_remaining;

		// 3 depends on 1 and 2
		frame1->dependencies_frame = frame3;
		frame2->dependencies_frame = frame3;
		frame3->dependencies_frame = NULL;

		// this frame awaits for 1,2,3 to finish
		data->sync_frames_remaining = 3;

		// add 1 & 2 in the workque (3 will be added when 1 & 2 finish)
		swift_deque_push(&thread->workque, frame1, &status);
		swift_deque_push(&thread->workque, frame2, &status);
	}

	/* ksync; */
	while ((n = SWIFT_ATOMIC_READ(data->sync_frames_remaining))) {
		SWIFT_LOG(INFO, "[%d] %s rem=%d c=%d f=%c.%d (%d)\n", thread->id, "@sync", n,
					SWIFT_PROC(frame->info), SWIFT_FRAME_NAME(frame->info),
					frame->dbg, SWIFT_FRAME_ID(frame->info));
		swift_scheduler_execute(thread, &status);
	}

	SWIFT_LOG(INFO, "[%d] __fibo(%d)=%d\n", thread->id, data->n, *data->r);

	swift_signal_frame_done(thread, frame, &status);
	swift_frame_done(thread->context, thread, frame, &status);
}

void* thread_start(void *arg) {
	swift_thread_t *thread = arg;
	swift_status_t status;

	SWIFT_LOG(INFO, "_____started thread %d\n", thread->id);

	while (! thread->context->parallel_finished) {
		swift_scheduler_execute(thread, &status);
	}

	//SWIFT_LOG(INFO, "_____finished thread %d: %d frames processed\n", thread->id, thread->stats);
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

	SWIFT_LOG(INFO, "Started nthreads=%d n=%d\n", nthreads, n);

	/* initialize the context */
	swift_context_init(&context, nthreads, &status);

	/* push the first frame */
	frame = (swift_frame_t*) swift_malloc(&context.threads[0], sizeof(swift_frame_t) + sizeof(fibo_data_t), &status);
	SWIFT_FRAME_SET_END_PARALLEL(frame);
	frame->closure = fibo;
#ifdef LOGGING_ON
	frame->creator_id = 0;
	frame->dbg = n;
#endif

	data = (fibo_data_t*) ((char*)frame + sizeof(swift_frame_t));
	data->n = n;
	data->r = &r;
	frame->private_data = data;

	SWIFT_WRITE_FRAME_INFO(frame, 0, 'f', context.threads[0].frame_no++);
	SWIFT_LOG_FRAME_INFO((&context.threads[0]), frame);

	// no wait frame
	frame->dependencies_no = 0;
	frame->dependencies_frame = NULL;
	frame->sync_frames_remaining = &data->sync_frames_remaining;

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
