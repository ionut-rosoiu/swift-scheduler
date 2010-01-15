/*
 * swift_thread.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include "swift_thread.h"

void swift_thread_start(swift_thread_t *thread,
						swift_entrypoint_t func) {

	//TODO: check
	pthread_create(&thread->thread, NULL, func, thread);
}

void swift_thread_wait(swift_thread_t *thread) {
	//TODO: check
	pthread_join(thread->thread, NULL);
}
