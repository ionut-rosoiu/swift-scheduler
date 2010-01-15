/*
 * swift_dfg.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_DFG_H_
#define SWIFT_DFG_H_

#include "swift_declarations.h"
#include "swift_common.h"

#define SWIFT_FRAME_END_PARALLEL 0x01  /*< by ending this frame the parallel region has finished */

#define SWIFT_FRAME_SET_END_PARALLEL(frame) \
	((frame)->flags |= SWIFT_FRAME_END_PARALLEL)

#define SWIFT_FRAME_IS_END_PARALLEL(frame) \
	((frame)->flags & SWIFT_FRAME_END_PARALLEL)

#define SWIFT_FRAME_ID_MASK		0x0000FFFF
#define SWIFT_FRAME_ID_OFFSET	0
#define SWIFT_FRAME_NAME_MASK	0x00FF0000
#define SWIFT_FRAME_NAME_OFFSET	16
#define SWIFT_PROC_MASK			0xFF000000
#define SWIFT_PROC_OFFSET		24

#define SWIFT_FRAME_ID(v)		   ((v) & SWIFT_FRAME_ID_MASK) >> SWIFT_FRAME_ID_OFFSET
#define SWIFT_FRAME_NAME(v)	(char)(((v) & SWIFT_FRAME_NAME_MASK) >> SWIFT_FRAME_NAME_OFFSET)
#define SWIFT_PROC(v)			   ((v) & SWIFT_PROC_MASK) >> SWIFT_PROC_OFFSET

#if LOGGING_ON
#define SWIFT_WRITE_FRAME_INFO(frame, proc, name, id)							\
	frame->info = ((proc << SWIFT_PROC_OFFSET) & SWIFT_PROC_MASK) |				\
				  ((name << SWIFT_FRAME_NAME_OFFSET) & SWIFT_FRAME_NAME_MASK) |	\
				  ((id << SWIFT_FRAME_ID_OFFSET) & SWIFT_FRAME_ID_MASK)
#else
#define SWIFT_WRITE_FRAME_INFO(frame, proc, name, id)
#endif

/**
 * The closure for this frame
 */
typedef void (*swift_closure_handler)(swift_thread_t *thread, struct swift_frame* frame);

/**
 * The handler to be executed when the frame is finished
 */
typedef void (*swift_frame_finished_handler)(swift_thread_t *thread, struct swift_frame* frame);

struct swift_frame {
	volatile int flags;						/*< frame flags */

#ifdef LOGGING_ON
	volatile int info;						/*< TODO: frame info (temporary, for debug) */
	int dbg;
	swift_id_t creator_id;					/*< the id of the initial creator for the frame */
#endif

	swift_closure_handler closure;			/*< the closure for this frame */

	volatile int dependencies_no; 			/*< the number of unavailable variables */
	struct swift_frame *dependencies_frame;	/*< the frame that awaits the unavailable data */

	// closure-dependent data
	void *private_data;						/*< private data related to each specific closure
												this gets deallocated when the frame is retired
											    */

	swift_size_t *sync_frames_remaining;	/*< the number to decrement when finishing execution of this frame */

	// doubly-linked list
	struct swift_frame *prev;
	struct swift_frame *next;
};


#endif /* SWIFT_DFG_H_ */
