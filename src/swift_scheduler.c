/*
 * swift_scheduler.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include "swift_scheduler.h"
#include <stdlib.h>
#include <sched.h>

inline
swift_thread_t* swift_get_steal_target(swift_context_t *context, swift_thread_t* thread)
{
	/* round robin */
	++thread->round_robin_id;
	thread->round_robin_id %= context->thread_num;

	if (thread->round_robin_id == thread->id) {
		++thread->round_robin_id;
		thread->round_robin_id %= context->thread_num;
	}

	return &context->threads[thread->round_robin_id];
}

inline
swift_frame_t* swift_get_frame(swift_context_t *context, swift_thread_t *thread)
{
	swift_frame_t *frame = NULL;
	swift_thread_t *target = NULL;
	swift_status_t status;

	/* 1. get a frame from the bottom of the workqueue */
	SWIFT_LOG(INFO, "[%d] get_frame_try_deq\n", thread->id);
	frame = swift_deque_pop(&thread->workque, &status);
	if (frame) {
		SWIFT_LOG_FRAME_INFO_STR("deq_bottom", thread, frame);
		goto exit;
	}

	/* 2. try to steal a frame */
	target = swift_get_steal_target(context, thread);
	SWIFT_LOG(INFO, "[%d] get_frame_try_steal %d\n", thread->id, target->id);
	do {
		frame = swift_deque_steal(&target->workque, &status);
	} while (status == SWIFT_DEQUE_ABORT);

	if (frame) {
		SWIFT_LOG_FRAME_INFO_STR("-STOLEN-", thread, frame);
		goto exit;
	}

	//FIXME: experimental!
	sched_yield();

exit:
	return frame;
}

inline
void swift_scheduler_execute(swift_thread_t *thread)
{
	swift_frame_t *frame;

	// execute some work
	SWIFT_LOG(INFO, "[%d] sched_exec\n", thread->id);
	frame = swift_get_frame(thread->context, thread);

	if (frame) {
		SWIFT_LOG_FRAME_INFO_STR("scheduler_execute", thread, frame);
		frame->closure(thread, frame);
	}
	//else {
	//	swift_backoff(thread, status);
	//	SWIFT_LOG(INFO, "[%d] slept %u\n", thread->id, thread->last_sleep_time);
	//}
}

inline
void swift_signal_frame_done(swift_thread_t *thread, swift_frame_t *frame)
{
	swift_status_t status;

	SWIFT_LOG_FRAME_INFO_STR("frame_done_signal_sync", thread, frame);

	/* signal sync */
	// decrement the number of frames needed to be sync'd
	SWIFT_ATOMIC_DECR(*frame->sync_frames_remaining);

	// this is the last frame for the sync
#ifdef LOGGING_ON
	// if this is the last frame before the sync, signal sync done
	if (SWIFT_CAS(*frame->sync_frames_remaining, 0, 0)) {
		SWIFT_LOG_FRAME_INFO_STR("frame_done_signal_sync_FINAL", thread, frame);
	}
#endif

	/* signal dependencies */
	if (frame->dependencies_frame) {
		swift_frame_t *f = frame->dependencies_frame;
		SWIFT_LOG_FRAME_INFO_STR("frame_done_signal_deps", thread, frame);

		SWIFT_ATOMIC_DECR(f->dependencies_no);

		// this is the last frame for the dependent frame
		if (SWIFT_CAS(f->dependencies_no, 0, 0)) {
			SWIFT_LOG_FRAME_INFO_STR("frame_done_signal_deps_FINAL", thread, frame);
			swift_deque_push(&thread->workque, f, &status);
		}
	}
}

inline
void swift_frame_done(swift_context_t *context, swift_thread_t *thread, swift_frame_t *frame)
{

	/* the last frame for the parallel region */
	if (SWIFT_FRAME_IS_END_PARALLEL(frame)) {
		SWIFT_LOG_FRAME_INFO_STR("--LAST FRAME DONE!--", thread, frame);
		thread->context->parallel_finished = 1;
	}

	thread->stats += 1;

	swift_retire_frame(thread, frame);
}
