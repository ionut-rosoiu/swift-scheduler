/*
 * swift_memory.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_MEMORY_H_
#define SWIFT_MEMORY_H_

/* 64b cache line size */
#define SWIFT_CACHE_LINE_SIZE 64

#define swift_memory_read_barrier() \
	__asm__ __volatile__ ("" : : : "memory");

#define swift_memory_write_barrier() \
	__asm__ __volatile__ ("" : : : "memory");

#define swift_memory_barrier() \
	__asm__ __volatile__ ("" : : : "memory");


#endif /* SWIFT_MEMORY_H_ */
