/*
 * swift_thread.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_THREAD_H_
#define SWIFT_THREAD_H_

#include "swift_common.h"
#include "swift_allocator.h"
#include "swift_declarations.h"
#include "swift_context.h"
#include "swift_deque.h"
#include <pthread.h>

#define SWIFT_THREAD_RUNNING	0
#define SWIFT_THREAD_WAITING	1

struct swift_thread {
	struct swift_context *context;		/*< global info (related to the other threads) */
	volatile int state;					/*< the state of the thread */
	volatile int stop;					/*< true if the thread must stop */

	int stats;							/*< thread statistics */

	swift_id_t 	id;						/*< the id of the thread */
	swift_deque_t workque;				/*< the workqueue */

#ifdef SWIFT_USE_CUSTOM_ALLOCATOR
	void *heap;							/*< base for the thread's heap */
	void *heap_top;						/*< the top of the thread's heap */
	void *heap_limit;					/*< the limit for the thread's heap */
	swift_allocator_list_t **buckets;	/*< the allocator's buckets */
#endif

	swift_size_t last_sleep_time;		/*< used by the backoff algorithm (in ns) */
	swift_size_t round_robin_id;		/*< the last target in a round robin steal search */
	swift_size_t frame_no;				/*< the number of frames this thread has spawned */

	pthread_t thread;					/*< system-level thread */
};

typedef void* (*swift_entrypoint_t) (void*);

void swift_thread_start(swift_thread_t *thread,
						swift_entrypoint_t func);

void swift_thread_wait(swift_thread_t *thread);



#endif /* SWIFT_THREAD_H_ */
