/*
 * swift_compiler.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_COMPILER_H_
#define SWIFT_COMPILER_H_

#ifdef GNU_COMPILER
	#define NO_INLINE __attribute__((noinline))
	#define HOT __attribute__((hot))
#else
	#define NO_INLINE
	#define HOT
#endif

#endif /* SWIFT_COMPILER_H_ */
