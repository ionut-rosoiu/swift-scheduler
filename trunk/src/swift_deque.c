/*
 * swift_deque.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include "swift_deque.h"
#include <stdio.h>

void swift_deque_init(swift_thread_t *thread, swift_deque_t *q, swift_size_t size) {
	//automatically resized to a power of two
	//SWIFT_CHECK(size & (size-1), SWIFT_RETURN_WITH_STATUS(SWIFT_INVALID_PARAM));

	if (size & (size-1)) {
		/* round up to a power of two */
		swift_size_t newSize = 1;

		while (size) {
			size >>= 1;
			newSize <<= 1;
		}

		size = newSize;
	}

	q->size = size;
	q->size_mask = size - 1;
	q->top = q->bottom = q->top_cached = 0;
	q->elements = (swift_frame_t**) swift_malloc(thread, size * sizeof(swift_frame_t*));
}

void swift_deque_push(swift_deque_t *q, swift_frame_t *frame, swift_status_t *status) {
	swift_dword_t t, b;
	SWIFT_CHECK(NULL != q, SWIFT_RETURN_WITH_STATUS(SWIFT_INVALID_PARAM));

	b = q->bottom;
	/* avoid some cache misses by using this local variable */
	t = q->top_cached;

	if (b - t >= q->size_mask) { /* size_mask = size - 1 */
		/* because we've used a cached value, we might have a false positive */
		t = q->top;
		q->top_cached = t;

		if (b - t >= q->size_mask) {
			//TODO: handle reallocation (...and ALL the problems that come along...)
			// for the moment fail this case
			*status = SWIFT_DEQUE_FULL;
			return;
		}
	}

	/* push the new value at the bottom */
	q->elements[b & q->size_mask] = frame;
	q->bottom = b + 1;

	*status = SWIFT_SUCCESS;
}

swift_frame_t* swift_deque_pop(swift_deque_t *q, swift_status_t *status) {
	swift_frame_t *ret = NULL;
	swift_dword_t b, t;
	long size;

	SWIFT_CHECK(NULL != q, *status = SWIFT_INVALID_PARAM; return NULL);

	b = q->bottom;

	b--;
	q->bottom = b;

	/* top must be read _after_ bottom is written (i.e. incremented) */
	swift_memory_write_barrier();

	t = q->top;
	q->top_cached = t;

	size = (long)(b - t);

	if (size < 0) {
		q->bottom = t;

		*status = SWIFT_DEQUE_EMPTY;
		return NULL;
	}

	/* get a reference to the element to return */
	ret = q->elements[b & q->size_mask];

	/* enough active elements left in the array */
	if (size > 0) {
		return ret;
	}

	/* check to see if the last element was concurrently stolen */
	if (! SWIFT_CAS(q->top, t, t+1)) {
		/* someone has stolen our element */
		*status = SWIFT_DEQUE_EMPTY;
		ret = NULL;
	}

	/* the array is empty and q->top is t+1; adjust bottom */
	q->bottom = t + 1;
	q->top_cached = t + 1;

	return ret;
}

swift_frame_t* swift_deque_steal(swift_deque_t *q, swift_status_t *status) {
	swift_dword_t t, b;
	swift_frame_t *ret = NULL;

	SWIFT_CHECK(NULL != q, *status = SWIFT_INVALID_PARAM; return NULL);

	t = q->top;

	/* a read barrier is needed to guarantee a consistent view of the memory
	 * see also the write_barrier() in the pop()
	 */
	swift_memory_read_barrier();

	b = q->bottom;

	if ((long)(b - t) <= 0) {
		*status = SWIFT_DEQUE_EMPTY;
		return NULL;
	}

	/* get a reference to the element _before_ trying the CAS
	 * (i.e. _before_ checking we've won the race);
	 * if not, in case we win, we might get a reference to a newly
	 * push()-ed element instead of the current one
	 */
	ret = q->elements[t & q->size_mask];

	if (! SWIFT_CAS(q->top, t, t+1)) {
		*status = SWIFT_DEQUE_ABORT;
		return NULL;
	}

	*status = SWIFT_SUCCESS;
	return ret;
}

void swift_deque_destroy(swift_thread_t *thread, swift_deque_t *q) {
	swift_free(thread, q->elements);
}
