/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/sys/cputimes.h"
#include <cassert>
#if     defined(OS_LINUX)
    #include <time.h>
    #include <unistd.h>
    #include <sys/times.h>
#elif   defined(OS_WINDOWS)
    #include <windows.h>
#endif

namespace cxx {
    namespace sys {

#if     defined(OS_LINUX)
        static int64_t tick_factor()
        {
            static int64_t factor = 0;
            if(factor == 0) {
                factor = ::sysconf(_SC_CLK_TCK);
                if(factor <= 0 || factor > 1000000000LL) {
                    assert(false);
                    factor = -1;
                }
                else {
                    factor = 1000000000LL / factor;
                }
            }
            return factor;
        }
#endif

        static void get_cpu_times(cpu_times::times& current)
        {
#if     defined(OS_LINUX)
            timespec ts;
            if(::clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
                current.wall = int64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
            }

            tms tm;
            clock_t c = ::times(&tm);
            if(c == static_cast<clock_t >(-1)) {
                current.sys = current.user = -1;
            }
            else {
                current.sys = tm.tms_stime + tm.tms_cstime;
                current.user= tm.tms_utime + tm.tms_cutime;
                int64_t factor = tick_factor();
                if(factor != -1) {
                    current.sys *= factor;
                    current.user*= factor;
                }
                else {
                    current.sys = current.user = -1;
                }
            }
#elif   defined(OS_WINDOWS)
            double nanosec_per_tic = 0.0L;
            {
                LARGE_INTEGER freq;
                if(QueryPerformanceFrequency(&freq)) {
                    nanosec_per_tic = double(1000000000.0L / freq.QuadPart);
                }
            }
            {
                LARGE_INTEGER cont;
                if(nanosec_per_tic <= 0.0L || !QueryPerformanceCounter(&cont)) {
                    assert(false);
                    current.wall = 0;
                }
                else {
                    current.wall = nanosec_per_tic * cont.QuadPart;
                }
            }

            {
                FILETIME creation, exit;
                if(::GetProcessTimes(::GetCurrentProcess(), &creation, &exit,
                                     (LPFILETIME)&current.sys, (LPFILETIME)&current.user)) {
                    current.user *= 100;
                    current.sys  *= 100;
                }
                else {
                    current.sys = current.user = -1;
                }
            }
#endif

        }

        cpu_times::cpu_times()
        {
            start();
        }

        void cpu_times::start()
        {
            in_use_ = true;
            get_cpu_times(usages_);
        }

        void cpu_times::stop()
        {
            if(!in_use_) {
                return;
            }
            in_use_ = false;

            times current;
            get_cpu_times(current);
            usages_.wall = (current.wall - usages_.wall);
            usages_.user = (current.user - usages_.user);
            usages_.sys  = (current.sys - usages_.sys);
        }

        cpu_times::times cpu_times::elapsed()
        {
            if(!in_use_) {
                return usages_;
            }
            times current;
            get_cpu_times(current);
            current.wall -= usages_.wall;
            current.user -= usages_.user;
            current.sys  -= usages_.sys;
            return current;
        }

        void cpu_times::resume()
        {
            if(!in_use_) {
                times current(usages_);
                start();
                usages_.wall -= current.wall;
                usages_.user -= current.user;
                usages_.sys  -= current.sys;
            }
        }

    }
}
