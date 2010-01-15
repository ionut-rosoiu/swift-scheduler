/*
 * swift_scheduler.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_SCHEDULER_H_
#define SWIFT_SCHEDULER_H_

#include "swift_common.h"
#include "swift_backoff.h"
#include "swift_context.h"
#include "swift_thread.h"
#include "swift_deque.h"
#include "swift_log.h"
#include "swift_compiler.h"

/**
 * Try to get a frame either from own workque or from another thread
 */
swift_frame_t* swift_get_frame(swift_context_t *context, swift_thread_t *thread);

/**
 * Get the steal target according to the current steal strategy
 */
swift_thread_t* swift_get_steal_target(swift_context_t *context, swift_thread_t* thread);

/**
 * Try to execute some work either by removing frames from own workqueue, or by stealing
 */
void swift_scheduler_execute(swift_thread_t *thread);

/**
 * Signal the finish of this frame (i.e. decrement the sync number and the data dependencies)
 */
void swift_signal_frame_done(swift_thread_t *thread, swift_frame_t *frame);

/**
 * Retire the frame to the pool
 * If this is the frame that started the parallel computation, end it
 */
void swift_frame_done(swift_context_t *context,
					  swift_thread_t *thread,
					  swift_frame_t *frame);

#endif /* SWIFT_SCHEDULER_H_ */
