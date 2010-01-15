/*
 * swift_deque.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_DEQUE_H_
#define SWIFT_DEQUE_H_

#include "swift_atomic.h"
#include "swift_common.h"
#include "swift_dfg.h"
#include "swift_declarations.h"
#include "swift_memory.h"

#define SWIFT_DEQUE_EMPTY	(SWIFT_LAST_STATUS_NO - 1)
#define SWIFT_DEQUE_FULL	(SWIFT_DEQUE_EMPTY - 1)
#define SWIFT_DEQUE_ABORT	(SWIFT_DEQUE_EMPTY - 2)

#define SWIFT_DEQUE_SIZE 30000
//9128

struct swift_deque {
	swift_size_t size;				/*< the real size */
	char _pad1[SWIFT_CACHE_LINE_SIZE - sizeof(swift_size_t)];

	swift_size_t size_mask; 		/*< size-1 used for calculating faster the modulo size */
	char _pad2[SWIFT_CACHE_LINE_SIZE - sizeof(swift_size_t)];

	volatile swift_dword_t top;		/*< top index (only incremented; written only by the owner) */
	char _pad3[SWIFT_CACHE_LINE_SIZE - sizeof(swift_dword_t)];

	volatile swift_dword_t top_cached; /*< cached value for the top (top bound) */
	char _pad4[SWIFT_CACHE_LINE_SIZE - sizeof(swift_dword_t)];

	volatile swift_dword_t bottom;	/*< bottom index */
	char _pad5[SWIFT_CACHE_LINE_SIZE - sizeof(swift_dword_t)];

	swift_frame_t **elements;		/*< the circular array of frames */
	char _pad6[SWIFT_CACHE_LINE_SIZE - sizeof(swift_frame_t **)];
};

#include "swift_allocator.h"


/**
 * Initializes the deque
 */
void swift_deque_init(swift_thread_t *thread,
					  swift_deque_t *q,
					  swift_size_t size);

/**
 * Push a frame at the bottom of the deque
 * @returns status == FULL when queue is full
 */
void swift_deque_push(swift_deque_t *q,
					  swift_frame_t *frame,
					  swift_status_t *status);

/**
 * Pop a frame from the bottom of the deque
 * @returns The pushed frame or NULL in case of failure (status == EMPTY)
 */
swift_frame_t* swift_deque_pop(swift_deque_t *q,
							   swift_status_t *status);

/**
 * Steal a frame from the top of the deque
 * @returns The frame stolen or NULL in case of failure (status == (EMPTY || ABORT))
 */
swift_frame_t* swift_deque_steal(swift_deque_t *q,
								 swift_status_t *status);

void swift_deque_destroy(swift_thread_t *thread,
						 swift_deque_t *q);

#endif /* SWIFT_DEQUE_H_ */
