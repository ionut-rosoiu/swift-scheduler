/*
 * swift_context.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include "swift_context.h"
#include <stdlib.h>
#include <string.h>

void swift_context_init(swift_context_t *context, swift_size_t nprocs) {
	//TODO: check
	swift_thread_t *thread;
	int i;
#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
	int j;
#endif

	context->thread_num = nprocs;
	context->parallel_finished = 0;

	context->threads = (swift_thread_t*) swift_malloc_system(nprocs * sizeof(swift_thread_t));

	//TODO: check
	for (i=0; i<nprocs; i++) {
		thread = &context->threads[i];

#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
		thread->heap = swift_malloc_system(SWIFT_HEAP_SIZE);
		thread->buckets = (swift_allocator_list_t**) swift_malloc_system(SWIFT_BUCKET_COUNT * sizeof(void*));
		swift_assert(thread->heap && thread->buckets);

		thread->heap_top = thread->heap;
		thread->heap_limit = ((char*)thread->heap + SWIFT_HEAP_SIZE);

		for (j=0; j<SWIFT_BUCKET_COUNT; j++) {
			thread->buckets[j] = NULL;
		}
#endif

		swift_deque_init(thread,
						 &thread->workque,
						 SWIFT_DEQUE_SIZE);

		//TODO: check
		thread->context = context;
		thread->id = i;
		thread->stats = 0;
		thread->stop = 0;
		thread->round_robin_id = i;
		thread->last_sleep_time = i * 10 + 1;
		thread->frame_no = 0;
	}
}

void swift_context_destroy(swift_context_t *context) {
	int i;
#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
	int j;
#endif

	for (i=0; i<context->thread_num; i++) {
		swift_deque_destroy(&context->threads[i], &context->threads[i].workque);

#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
		for (j=0; j<SWIFT_BUCKET_COUNT; j++) {
			context->threads[i].buckets[j] = NULL;
		}
		swift_free_system(context->threads[i].buckets);
#endif

	}

	swift_free_system(context->threads);
}

