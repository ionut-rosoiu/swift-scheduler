/*
 * swift_atomic.h
 *
 * (c) 2009 Ionut Rosoiu <ionut.rosoiu@gmail.com>
 *
 */

#ifndef SWIFT_ATOMIC_H_
#define SWIFT_ATOMIC_H_

#define SWIFT_ATOMIC_READ(x)	\
	(x)

#define SWIFT_ATOMIC_WRITE(x, val)	\
	(x) = (val)

#define SWIFT_ATOMIC_INCR(x)	\
	__sync_add_and_fetch(&(x), 1)

#define SWIFT_ATOMIC_DECR(x)	\
	__sync_sub_and_fetch(&(x), 1)

#define SWIFT_CAS(x, comp, val)	\
	__sync_bool_compare_and_swap((int*)&(x), (comp), (val))

#endif /* SWIFT_ATOMIC_H_ */
