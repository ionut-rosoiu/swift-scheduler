/*
 * swift_allocator.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "swift_assert.h"
#include "swift_allocator.h"
#include "swift_log.h"

inline
void* swift_malloc_system(size_t size) {
	void *ret = malloc(size);

	swift_assert(ret);

	return ret;
}

inline
void swift_free_system(void *addr) {
	free(addr);
}

inline
void* swift_malloc(swift_thread_t *thread, size_t size) {
	//TODO: check
	void *ret;

#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
	swift_allocator_list_t **b, *elem;
	size_t sz;

	/* allocate space for the allocator's list */
	size += sizeof(swift_allocator_list_t);
	size = SWIFT_MEM_ALIGN(size);

	for (b = thread->buckets, sz = SWIFT_MALLOC_ALIGN;
		 sz < size;
		 sz <<= 1, b++) {
	}

	elem = *b;
	if (elem) {
		/* reuse from the bucket */
		*b = elem->next;
		SWIFT_LOG(INFO, "get %d from bucket %p  --> elem=%p\n", sz, (void*)b, (void*) elem);

	} else {
		/* get from the heap */
		elem = thread->heap_top;
		thread->heap_top = ((char*)thread->heap_top + SWIFT_MEM_ALIGN(sz));

		SWIFT_LOG(INFO, "get %d from heap for bucket %p --> elem=%p\n", sz, (void*)b,  (void*)elem);
	}

	/* store from what bucket this came from */
	elem->next = b;

	swift_assert((void*)elem < thread->heap_limit);

	ret = elem;
	ret = (void*) ((char*)ret + SWIFT_MEM_ALIGN(sizeof(swift_allocator_list_t)));

	SWIFT_LOG(INFO, "malloc return %p\n", ret);
#else
	ret = malloc(size);
#endif

	return ret;
}

inline
void swift_free(swift_thread_t *thread, void *addr) {
#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
	swift_allocator_list_t *elem;
	swift_allocator_list_t **b;

	elem = (swift_allocator_list_t*) ((char*)addr - SWIFT_MEM_ALIGN(sizeof(swift_allocator_list_t)));

	/* get the bucket this element came from */
	b = elem->next;

	SWIFT_LOG(INFO, "return addr=%p to bucket %p (*b=%p) --> elem=%p\n", addr, (void*)b, (void*)*b, (void*)elem);

	/* return to the bucket */
	elem->next = *b;
	*b = elem;
#else
	//free(addr);
#endif
}

inline
void swift_retire_frame(swift_thread_t *thread, swift_frame_t *frame) {
	/* drop into the pool */
	swift_free(thread, frame);
}
