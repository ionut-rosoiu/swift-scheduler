/*
 * swift_common.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_COMMON_H_
#define SWIFT_COMMON_H_

#define SWIFT_SUCCESS  			 0
#define SWIFT_FAILURE 			-1
#define SWIFT_INVALID_PARAM 	-2
#define SWIFT_LAST_STATUS_NO 	SWIFT_INVALID_PARAM

typedef int 				swift_status_t;
typedef unsigned int 		swift_size_t;
typedef unsigned char 		swift_id_t;

/*
#define SWIFT_USE_CUSTOM_ALLOCATOR 	1
#undef SWIFT_USE_CUSTOM_ALLOCATOR

#define SWIFT_USE_CUSTOM_THREAD_ALLOCATOR 1
#undef SWIFT_USE_CUSTOM_THREAD_ALLOCATOR

#define LOGGING_ON 1
#undef LOGGING_ON
*/

#define SWIFT_KB					1024
#define SWIFT_MB					(SWIFT_KB * SWIFT_KB)
#define SWIFT_HEAP_SIZE				(100 * SWIFT_MB)

typedef swift_size_t		swift_word_t;
typedef unsigned long long 	swift_dword_t;

#define SWIFT_CHECK(cond, exec) \
	do {						\
		if (!(cond)) { exec; }	\
	} while (0)

#define SWIFT_RETURN_WITH_STATUS(val)	\
	do {								\
		*status = val;					\
		return;							\
	} while (0)

#define SWIFT_CHECK_SUCCESS()	\
	SWIFT_CHECK(SWIFT_SUCCESS == *status, return)

#define SWIFT_CHECK_SUCCESS_RETURN(val) \
	SWIFT_CHECK(SWIFT_SUCCESS == *status, SWIFT_RETURN_WITH_STATUS(val))


#endif /* SWIFT_COMMON_H_ */
