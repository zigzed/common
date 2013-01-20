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

        atomic_t::operator long() const
        {
            return atomic_exchange_and_add(&value_, 0);
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

        atomic_t::operator long() const
        {
            return value_;
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
