/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/sys/atomic.h"

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    // gcc x86
    static long atomic_exchange_and_add(long* pw, long dv)
    {
        long r;
        __asm__ __volatile__
                (
                    "lock\n\t"
                    "xadd %1, %0":
                    "+m"( *pw ), "=r"(r):	// outputs (%0, %1)
                    "1"( dv ):				// inputs (%2 == %1)
                    "memory", "cc"			// clobbers
                );
        return r;
    }

namespace cxx {
    namespace sys {

        atomic_t::atomic_t(long v) : value_(v)
        {
        }

        long atomic_t::operator++()
        {
            return atomic_exchange_and_add(&value_, +1) + 1;
        }

        long atomic_t::operator--()
        {
            return atomic_exchange_and_add(&value_, -1) - 1;
        }

        long atomic_t::add(long inc)
        {
            return atomic_exchange_and_add(&value_, inc);
        }

        long atomic_t::sub(long dec)
        {
            dec *= -1;
            return atomic_exchange_and_add(&value_, dec);
        }

        atomic_t::operator long() const
        {
            return atomic_exchange_and_add(&value_, 0);
        }

        namespace detail {
            void* atomic_ptr_base_t::do_xchg(void *val)
            {
                 void *old;
                __asm__ volatile (
                    "lock; xchg %0, %2"
                    : "=r" (old), "=m" (ptr_)
                    : "m" (ptr_), "0" (val));
                return old;
            }

            void* atomic_ptr_base_t::do_cas(void *cmp, void *val)
            {
                void *old;
                __asm__ volatile (
                    "lock; cmpxchg %2, %3"
                    : "=a" (old), "=m" (ptr_)
                    : "r" (val), "m" (ptr_), "0" (cmp)
                    : "cc");
                return old;
            }
        }

    }
}

#elif defined(OS_WINDOWS) || defined(__CYGWIN__)
// win32
#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cxx {
    namespace sys {

        atomic_t::atomic_t(long v) : value_(v)
        {
        }

        long atomic_t::operator++()
        {
            return InterlockedIncrement(&value_);
        }

        long atomic_t::operator--()
        {
            return InterlockedDecrement(&value_);
        }

        long atomic_t::add(int inc)
        {
            return InterlockedExchangeAdd((LONG* )&value_, inc);
        }

        long atomic_t::sub(int dec)
        {
            dec *= -1;
            return InterlockedExchangeAdd((LONG* )&value_, dec);
        }

        atomic_t::operator long() const
        {
            return value_;
        }

        namespace detail {
            void* atomic_ptr_base_t::do_xchg(void *val)
            {
                return InterlockedExchangePointer((PVOID* )&ptr_, val);
            }

            void* atomic_ptr_base_t::do_cas(void *cmp, void *val)
            {
                return InterlockedCompareExchangePointer((volatile PVOID* )&ptr_, val, cmp);
            }
        }

    }
}

#elif defined(__ARM_ARCH_7A__) && defined(__GNUC__)
namespace cxx {
    namespace sys {

        atomic_t::atomic_t(long v) : value_(v)
        {
        }

        long atomic_t::operator++()
        {
            long old, flag, tmp;
            __asm__ volatile (
                "       dmb     sy\n\t"
                "1:     ldrex   %0, [%5]\n\t"
                "       add     %2, %0, %4\n\t"
                "       strex   %1, %2, [%5]\n\t"
                "       teq     %1, #0\n\t"
                "       bne     1b\n\t"
                "       dmb     sy\n\t"
                : "=&r"(old), "=&r"(flag), "=&r"(tmp), "+Qo"(value_)
                : "Ir"(1), "r"(&value_)
                : "cc");
            return old;
        }

        long atomic_t::operator--()
        {
            long old, flag, tmp;
            __asm__ volatile (
                "       dmb     sy\n\t"
                "1:     ldrex   %0, [%5]\n\t"
                "       sub     %2, %0, %4\n\t"
                "       strex   %1, %2, [%5]\n\t"
                "       teq     %1, #0\n\t"
                "       bne     1b\n\t"
                "       dmb     sy\n\t"
                : "=&r"(old), "=&r"(flag), "=&r"(tmp), "+Qo"(value_)
                : "Ir"(1), "r"(&value_)
                : "cc");
            return old;
        }

        atomic_t::operator long() const
        {
            return value_;
        }

        namespace detail {
            void* atomic_ptr_base_t::do_xchg(void* val)
            {
                void* old;
                unsigned int flag;
                __asm__ volatile (
                    "       dmb     sy\n\t"
                    "1:     ldrex   %1, [%3]\n\t"
                    "       strex   %0, %4, [%3]\n\t"
                    "       teq     %0, #0\n\t"
                    "       bne     1b\n\t"
                    "       dmb     sy\n\t"
                    : "=&r"(flag), "=&r"(old), "+Qo"(ptr_)
                    : "r"(&ptr_), "r"(val)
                    : "cc");
                return old;
            }

            void* atomic_ptr_base_t::do_cas(void* cmp, void* val)
            {
                void *old;
                unsigned int flag;
                __asm__ volatile (
                    "       dmb     sy\n\t"
                    "1:     ldrex   %1, [%3]\n\t"
                    "       mov     %0, #0\n\t"
                    "       teq     %1, %4\n\t"
                    "       it      eq\n\t"
                    "       strexeq %0, %5, [%3]\n\t"
                    "       teq     %0, #0\n\t"
                    "       bne     1b\n\t"
                    "       dmb     sy\n\t"
                    : "=&r"(flag), "=&r"(old), "+Qo"(ptr_)
                    : "r"(&ptr_), "r"(cmp_), "r"(val_)
                    : "cc");
                return old;
            }
        }
    }

#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
// gcc
#if __GNUC__ * 100 + __GNUC_MINOR__ >= 402
#include <ext/atomicity.h>
#else
#include <bits/atomicity.h>
#endif

#if defined(__GLIBCXX__)
using __gnu_cxx::__atomic_add;
using __gnu_cxx::__exchange_and_add;
#endif

namespace cxx {
    namespace sys {

        atomic_t::atomic_t(long v) : value_(v)
        {
        }

        long atomic_t::operator++()
        {
            return __exchange_and_add(&value_, +1) + 1;
        }

        long atomic_t::operator--()
        {
            return __exchange_and_add(&value_, -1) - 1;
        }

        atomic_t::operator long() const
        {
            return __exchange_and_add(&value_, 0);
        }

    }
}

#else
#error	Unrecognized threading platform
#endif
