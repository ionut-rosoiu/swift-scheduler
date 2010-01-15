/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "swift.h"

int limit;

extern void read_array(char *filename, int **arr, int *n);
extern void free_array(int *arr);
extern void check_array(int *arr, int n);

typedef struct qs_data {
	int *a;
	int l;
	int r;

	char _pad1[SWIFT_CACHE_LINE_SIZE - sizeof(swift_size_t)];

	swift_size_t sync_frames_remaining;

	char _pad2[SWIFT_CACHE_LINE_SIZE - sizeof(swift_size_t)];
} qs_data_t;

/* forward declaration for the fibo() */
void qs(swift_thread_t *thread, swift_frame_t *frame);


inline
void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

inline
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

void qs_ws(swift_thread_t *thread, swift_frame_t *frame)
{
	qs_data_t *data = frame->private_data;
	swift_status_t status;
	int pivot, n;

	SWIFT_LOG(INFO, "\n[%d] qs() c=%d f=%c.%d-%d (%d)\n", thread->id,
								SWIFT_PROC(frame->info), SWIFT_FRAME_NAME(frame->info),
								((frame->dbg >> 16) & 0xFF), (frame->dbg & 0xFF),
								SWIFT_FRAME_ID(frame->info));

	if (data->l < data->r) {
		swift_frame_t frame1;
		qs_data_t data1;

		swift_frame_t frame2;
		qs_data_t data2;

#ifdef LOGGING_ON
		int i;
#endif

		pivot = partition(data->a, data->l, data->r);

#ifdef LOGGING_ON
		for (i=data->l; i<=data->r; i++) {
			if (i == pivot) {
				fprintf(stderr, "%d<%d> ", i, data->a[i]);
			} else {
				fprintf(stderr, "%d(%d) ", i, data->a[i]);
			}
		}
		fprintf(stderr, "\n");
#endif

		// ---------- left ---------
		frame1.closure = qs_ws;
		frame1.flags = 0;
#ifdef LOGGING_ON
		frame1.creator_id = thread->id;
		frame1.dbg = ((data->l & 0xFF) << 16) | ((pivot - 1) & 0xFF);
#endif
		data1.a = data->a;
		data1.l = data->l;
		data1.r = pivot - 1;

		frame1.private_data = &data1;

		SWIFT_WRITE_FRAME_INFO((&frame1), thread->id, 'q', thread->frame_no++);

		frame1.dependencies_frame = NULL;
		frame1.sync_frames_remaining = &data->sync_frames_remaining;

		// ---------- right ---------
		frame2.closure = qs_ws;
		frame2.flags = 0;
#ifdef LOGGING_ON
		frame2.creator_id = thread->id;
		frame2.dbg = (((pivot + 1) & 0xFF) << 16) | (data->r & 0xFF);
#endif
		data2.a = data->a;
		data2.l = pivot + 1;
		data2.r = data->r;

		frame2.private_data = &data2;

		SWIFT_WRITE_FRAME_INFO((&frame2), thread->id, 'q', thread->frame_no++);

		frame2.dependencies_frame = NULL;
		frame2.sync_frames_remaining = &data->sync_frames_remaining;

		// ------------------------
		// wait for 1&2 to finish
		data->sync_frames_remaining = 2;

		// put it into the workqueue
		swift_deque_push(&thread->workque, &frame1, &status);
		swift_deque_push(&thread->workque, &frame2, &status);

		// sync
		while ((n = SWIFT_ATOMIC_READ(data->sync_frames_remaining))) {
			SWIFT_LOG(INFO, "[%d] %s rem=%d c=%d f=%c.%d-%d (%d)\n", thread->id, "@sync", n,
							SWIFT_PROC(frame->info), SWIFT_FRAME_NAME(frame->info),
							((frame->dbg >> 16) & 0xFF), (frame->dbg & 0xFF),
							SWIFT_FRAME_ID(frame->info));
			swift_scheduler_execute(thread);
		}

		SWIFT_LOG(INFO, "[%d] __qs(%d, %d)\n", thread->id, data->l, data->r);

		if (SWIFT_FRAME_IS_END_PARALLEL(frame)) {
			SWIFT_LOG(INFO, "[%d] END_PARALLEL!!!\n", thread->id);
			thread->stop = 1;
		}
	}

	swift_signal_frame_done(thread, frame);
	swift_frame_done(thread->context, thread, frame);
}

void* thread_start(void *arg) {
	swift_thread_t *thread = arg;
	int i = 0;

	SWIFT_LOG(INFO, "_____started thread %d\n", thread->id);

	while(!thread->stop) {
		swift_scheduler_execute(thread);
	}

	// inform all other threads that they must stop
	for (i=0; i<thread->context->thread_num; i++) {
		thread->context->threads[i].stop = 1;
	}


	SWIFT_LOG(INFO, "_____finished thread %d: %d frames processed\n", thread->id, thread->stats);
	printf("_____finished thread %d: %d frames processed\n", thread->id, thread->stats);

	return arg;
}

int quicksort(int *a, int size, int nthreads)
{
	swift_context_t context;
	swift_frame_t frame;
	qs_data_t data;
	swift_status_t status;
	int i;

	SWIFT_LOG(INFO, "Started nthreads=%d\n", nthreads);

	/* initialize the context */
	swift_context_init(&context, nthreads);

	/* push the first frame */
	SWIFT_FRAME_SET_END_PARALLEL(&frame);
	frame.closure = qs_ws;
#ifdef LOGGING_ON
	frame.dbg = size & 0xFF;
	frame.creator_id = 0;
#endif

	data.a = a;
	data.l = 0;
	data.r = size - 1;
	frame.private_data = &data;

	SWIFT_WRITE_FRAME_INFO((&frame), 0, 'q', context.threads[0].frame_no++);
	SWIFT_LOG_FRAME_INFO((&context.threads[0]), (&frame));

	frame.dependencies_no = 0;
	frame.dependencies_frame = NULL;
	frame.sync_frames_remaining = &data.sync_frames_remaining;

	swift_deque_push(&context.threads[0].workque, &frame, &status);
	SWIFT_LOG(INFO, "pushed first frame\n");


	/* start the threads */
	for (i=0; i<nthreads; i++) {
		swift_thread_start(&context.threads[i], thread_start);
	}

	/* wait for the threads to finish */
	for (i=0; i<nthreads; i++) {
		SWIFT_LOG(INFO, "_____waiting %d\n", i);
		swift_thread_wait(&context.threads[i]);
		SWIFT_LOG(INFO, "_____waiting %d done\n", i);
	}

	/* check the result */
	printf("done with the first frame.\n");
	swift_context_destroy(&context);
	return 0;
}

int main(int argc, char **argv)
{
	int n, *a, nthreads;

	if (argc != 4) {
		fprintf(stderr, "usage: %s <input_file> <nthreads> <serial_limit>\n", argv[0]);
		abort();
	}

	read_array(argv[1], &a, &n);

	nthreads = strtol(argv[2], NULL, 10);
	limit = strtol(argv[3], NULL, 10);

	quicksort(a, n, nthreads);

	check_array(a, n);

	free_array(a);

	printf("done.\n");
	return 0;
}
