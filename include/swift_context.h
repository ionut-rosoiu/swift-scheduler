/*
 * swift_context.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_CONTEXT_H_
#define SWIFT_CONTEXT_H_

#include "swift_common.h"
#include "swift_assert.h"
#include "swift_declarations.h"
#include "swift_allocator.h"
#include "swift_deque.h"
#include "swift_declarations.h"
#include "swift_thread.h"

struct swift_context {
	volatile int parallel_finished; /*< true if the parallel region finished */
	swift_size_t thread_num;		/*< the number of threads */
	swift_thread_t *threads;		/*< the threads */
};

void swift_context_init(swift_context_t *context, swift_size_t nprocs);

void swift_context_destroy(swift_context_t *context);


#endif /* SWIFT_CONTEXT_H_ */
