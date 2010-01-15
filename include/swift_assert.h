/*
 * swift_assert.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_ASSERT_H_
#define SWIFT_ASSERT_H_

#include <stdio.h>

#define swift_assert(cond)	\
	do {																\
		if (!(cond)) {													\
			printf("Assertion failed: %s:%d\n", __FILE__, __LINE__);		\
			exit(-1);													\
		}																\
	} while(0)


#endif /* SWIFT_ASSERT_H_ */
