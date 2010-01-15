/*
 * swift_log.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_LOG_H_
#define SWIFT_LOG_H_

#include <stdio.h>
#include "swift_dfg.h"

typedef enum swift_log_level {
	NONE,
	FATAL,
	WARN,
	INFO,
	DEBUG
} swift_log_level_t;

#if LOGGING_ON
#define SWIFT_LOG(level, ...) 			 \
	do {								 \
		fprintf(stderr, __VA_ARGS__);	 \
		fflush(stderr);					 \
	} while(0)
#else
#define SWIFT_LOG(level, ...)
#endif

#define SWIFT_LOG_FRAME_INFO(t, f)				\
	SWIFT_LOG(INFO, "[%d] c=%d f=%c.%d (%d)\n",	\
			t->id,								\
			SWIFT_PROC(f->info),				\
			SWIFT_FRAME_NAME(f->info),			\
			f->dbg,								\
			SWIFT_FRAME_ID(f->info))

#define SWIFT_LOG_FRAME_INFO_STR(s, t, f)				\
	SWIFT_LOG(INFO, "[%d] %s - c=%d f=%c.%d (%d)\n", 	\
				t->id,									\
				s,										\
				SWIFT_PROC(f->info),					\
				SWIFT_FRAME_NAME(f->info),				\
				f->dbg,									\
				SWIFT_FRAME_ID(f->info))


#endif /* SWIFT_LOG_H_ */
