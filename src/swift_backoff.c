/*
 * swift_backoff.c
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#include "swift_backoff.h"
#include "swift_log.h"
#define _POSIX_C_SOURCE 199309L
#include <time.h>

void swift_backoff(swift_thread_t *thread) {
	struct timespec req, rem;

	//TODO: limit backoff (or cycle)
	req.tv_sec = 0;
	req.tv_nsec = thread->last_sleep_time;

	while (nanosleep(&req, &rem) == -1) {
		req = rem;
	}

	thread->last_sleep_time = (thread->last_sleep_time << 1) % 100L;
}
