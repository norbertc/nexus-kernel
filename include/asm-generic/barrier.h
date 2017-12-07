/* Generic barrier definitions, based on MN10300 definitions.
 *
 * It should be possible to use these on really simple architectures,
 * but it serves more as a starting point for new ports.
 *
 * Copyright (C) 2007 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version
 * 2 of the Licence, or (at your option) any later version.
 */
#ifndef __ASM_GENERIC_BARRIER_H
#define __ASM_GENERIC_BARRIER_H

#ifndef __ASSEMBLY__

#define nop() asm volatile ("nop")

/*
 * Force strict CPU ordering.
 * And yes, this is required on UP too when we're talking
 * to devices.
 *
 * This implementation only contains a compiler barrier.
 */

#define mb()	asm volatile ("": : :"memory")
#define rmb()	mb()
#define wmb()	asm volatile ("": : :"memory")

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	rmb()
#define smp_wmb()	wmb()
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#endif

#define set_mb(var, value)  do { var = value;  mb(); } while (0)
#define set_wmb(var, value) do { var = value; wmb(); } while (0)

#define read_barrier_depends()		do {} while (0)
#define smp_read_barrier_depends()	do {} while (0)

/**
 * nospec_ptr() - Ensure a  pointer is bounded, even under speculation.
 *
 * @ptr: the pointer to test
 * @lo: the lower valid bound for @ptr, inclusive
 * @hi: the upper valid bound for @ptr, exclusive
 *
 * If @ptr falls in the interval [@lo, @i), returns @ptr, otherwise returns
 * NULL.
 *
 * Architectures should override this to ensure that ptr falls in the [lo, hi)
 * interval both under architectural execution and under speculation,
 * preventing propagation of an out-of-bounds pointer to code which is
 * speculatively executed.
 */
#ifndef nospec_ptr
#define nospec_ptr(ptr, lo, hi)						\
({									\
	typeof (ptr) __ptr = (ptr);					\
	typeof (ptr) __lo = (lo);					\
	typeof (ptr) __hi = (hi);					\
									\
	(__lo <= __ptr && __ptr < __hi) ? __ptr : NULL;			\
})
#endif

/**
 * nospec_load() - Load a pointer, respecting bounds under speculation
 *
 * @ptr: the pointer to load
 * @lo: the lower valid bound for @ptr, inclusive
 * @hi: the upper valid bound for @ptr, exclusive
 *
 * If @ptr falls in the interval [@lo, @hi), returns the value at @ptr,
 * otherwise returns (typeof(*ptr))0.
 *
 * Architectures should override this to ensure that ptr falls in the [lo, hi)
 * interval both under architectural execution and under speculation,
 * preventing speculative out-of-bounds reads.
 */
#ifndef nospec_load
#define nospec_load(ptr, lo, hi)					\
({									\
	typeof (ptr) __ptr = (ptr);					\
	typeof (ptr) __lo = (lo);					\
	typeof (ptr) __hi = (hi);					\
									\
	(__lo <= __ptr && __ptr <= __hi) ?				\
		*__ptr : 						\
		(typeof(*__ptr))(unsigned long)0;			\
})
#endif

/**
 * nospec_array_load - Load an array entry, respecting bounds under speculation
 *
 * @arr: the base of the array
 * @idx: the index of the element to load
 * @sz: the number of elements in the array
 *
 * If @idx falls in the interval [0, @sz), returns the value at @arr[@idx],
 * otherwise returns (typeof(*ptr))0.
 *
 * This is a wrapper around nospec_load(), provided for convenience.
 * Architectures should implement nospec_load() to ensure this is the case
 * under speculation.
 */
#define nospec_array_load(arr, idx, sz)					\
({									\
	typeof(*(arr)) *__arr = arr;					\
	typeof(idx) __idx = idx;					\
	typeof(sz) __sz = __sz;						\
									\
	nospec_load(__arr + __idx, __arr, __arr + __sz);		\
})

#ifndef smp_mb__before_atomic
#define smp_mb__before_atomic()	smp_mb()
#endif

#ifndef smp_mb__after_atomic
#define smp_mb__after_atomic()	smp_mb()
#endif

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	ACCESS_ONCE(*p) = (v);						\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = ACCESS_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	___p1;								\
})

#endif /* !__ASSEMBLY__ */
#endif /* __ASM_GENERIC_BARRIER_H */
