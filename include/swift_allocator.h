/*
 * swift_allocator.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_ALLOCATOR_H_
#define SWIFT_ALLOCATOR_H_

#include <stdlib.h>
#include "swift_common.h"
#include "swift_thread.h"
#include "swift_dfg.h"

/* The address of malloc/realloc is 8 byte alligned */
#define SWIFT_MALLOC_ALIGN 	8

/* The bucket count (i.e. the number of bites for the platform) */
#define SWIFT_BUCKET_COUNT	32

#define SWIFT_MEM_ALIGN(size)	\
	( ((swift_size_t)(size) + (SWIFT_MALLOC_ALIGN) - 1) & (~((SWIFT_MALLOC_ALIGN) - 1)) )

struct swift_allocator_list {
	void *aligned_addr;
	void *next;
};

void* swift_malloc_system(size_t size);

void swift_free_system(void *addr);

void* swift_malloc(swift_thread_t *thread, size_t size);

void swift_free(swift_thread_t *thread, void *addr);

void swift_retire_frame(swift_thread_t *thread, swift_frame_t *frame);


#endif /* SWIFT_ALLOCATOR_H_ */
