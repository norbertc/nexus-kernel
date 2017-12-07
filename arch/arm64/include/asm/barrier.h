/*
 * Based on arch/arm/include/asm/barrier.h
 *
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_BARRIER_H
#define __ASM_BARRIER_H

#ifndef __ASSEMBLY__

#define sev()		asm volatile("sev" : : : "memory")
#define wfe()		asm volatile("wfe" : : : "memory")
#define wfi()		asm volatile("wfi" : : : "memory")

#define isb()		asm volatile("isb" : : : "memory")
#define dmb(opt)	asm volatile("dmb " #opt : : : "memory")
#define dsb(opt)	asm volatile("dsb " #opt : : : "memory")

#define mb()		dsb(sy)
#define rmb()		dsb(ld)
#define wmb()		dsb(st)

#define __load_no_speculate_n(ptr, lo, hi, failval, cmpptr, w, sz)	\
({									\
	typeof(*ptr)	__nln_val;					\
	typeof(*ptr)	__failval = 					\
		(typeof(*ptr))(unsigned long)(failval);			\
	asm volatile (							\
	"	cmp	%[c], %[l]\n"					\
	"	ccmp	%[c], %[h], 2, cs\n"				\
	"	b.cs	1f\n"						\
	"	ldr" #sz " %" #w "[v], %[p]\n"				\
	"1:	csel	%" #w "[v], %" #w "[v], %" #w "[f], cc\n"	\
	"	hint	#0x14 // CSDB\n"				\
	: [v] "=&r" (__nln_val)						\
	: [p] "m" (*(ptr)), [l] "r" (lo), [h] "r" (hi),			\
	  [f] "rZ" (__failval), [c] "r" (cmpptr)			\
	: "cc");							\
									\
	__nln_val;							\
})

#define __load_no_speculate(ptr, lo, hi, failval, cmpptr)		\
({									\
	typeof(*(ptr)) __nl_val;					\
									\
	switch (sizeof(__nl_val)) {					\
	case 1:								\
		__nl_val = __load_no_speculate_n(ptr, lo, hi, failval,	\
						 cmpptr, w, b);		\
		break;							\
	case 2:								\
		__nl_val = __load_no_speculate_n(ptr, lo, hi, failval,	\
						 cmpptr, w, h);		\
		break;							\
	case 4:								\
		__nl_val = __load_no_speculate_n(ptr, lo, hi, failval,	\
						 cmpptr, w, );		\
		break;							\
	case 8:								\
		__nl_val = __load_no_speculate_n(ptr, lo, hi, failval,	\
						 cmpptr, x, );		\
		break;							\
	default:							\
		BUILD_BUG();						\
	}								\
									\
	__nl_val;							\
})

#define nospec_load(ptr, lo, hi)					\
({									\
	typeof(ptr) __nl_ptr = (ptr);					\
	__load_no_speculate(__nl_ptr, lo, hi, 0, __nl_ptr);		\
})

#define nospec_ptr(ptr, lo, hi)						\
({									\
	typeof(ptr) __np_ptr = (ptr);					\
	__load_no_speculate(&__np_ptr, lo, hi, 0, __np_ptr);		\
})
	

#ifndef CONFIG_SMP
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	ACCESS_ONCE(*p) = (v);						\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = ACCESS_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	___p1;								\
})

#else

#define smp_mb()	dmb(ish)
#define smp_rmb()	dmb(ishld)
#define smp_wmb()	dmb(ishst)

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	switch (sizeof(*p)) {						\
	case 4:								\
		asm volatile ("stlr %w1, %0"				\
				: "=Q" (*p) : "r" (v) : "memory");	\
		break;							\
	case 8:								\
		asm volatile ("stlr %1, %0"				\
				: "=Q" (*p) : "r" (v) : "memory");	\
		break;							\
	}								\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1;						\
	compiletime_assert_atomic_type(*p);				\
	switch (sizeof(*p)) {						\
	case 4:								\
		asm volatile ("ldar %w0, %1"				\
			: "=r" (___p1) : "Q" (*p) : "memory");		\
		break;							\
	case 8:								\
		asm volatile ("ldar %0, %1"				\
			: "=r" (___p1) : "Q" (*p) : "memory");		\
		break;							\
	}								\
	___p1;								\
})

#endif

#define read_barrier_depends()		do { } while(0)
#define smp_read_barrier_depends()	do { } while(0)

#define set_mb(var, value)	do { var = value; smp_mb(); } while (0)
#define nop()		asm volatile("nop");

#define smp_mb__before_atomic()	smp_mb()
#define smp_mb__after_atomic()	smp_mb()

#endif	/* __ASSEMBLY__ */

#endif	/* __ASM_BARRIER_H */
