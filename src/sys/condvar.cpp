/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sys/condvar.h"
#include <cassert>
#if     defined(OS_LINUX)
    #include <errno.h>
#endif

namespace cxx {
    namespace sys {

#if     defined(OS_LINUX)

        namespace {
            struct lock_on_exit {
                lock_on_exit() : m_(NULL) {}

                void activate(cxx::sys::plainmutex& m) {
                    m.release();
                    m_ = &m;
                }
                ~lock_on_exit() {
                    if(m_) {
                        m_->acquire();
                    }
                }

                cxx::sys::plainmutex*   m_;
            };
        }

        cond_var::cond_var()
        {
            int res = pthread_cond_init(&cond_, NULL);
            assert(res == 0);
        }

        cond_var::~cond_var()
        {
            int res = pthread_cond_destroy(&cond_);
            assert(res == 0);
        }

        void cond_var::notify_all()
        {
            int res = pthread_cond_broadcast(&cond_);
            assert(res == 0);
        }

        void cond_var::notify_one()
        {
            int res = pthread_cond_signal(&cond_);
            assert(res == 0);
        }

        void cond_var::wait(plainmutex &m)
        {
            int res = 0;
            {
                lock_on_exit    guard;
                guard.activate(m);
                do {
                    res = pthread_cond_wait(&cond_, &m.locker.handler);
                } while(res == EINTR);
            }
            assert(res == 0);
        }

        bool cond_var::wait(plainmutex &m, int ms)
        {
            lock_on_exit    guard;
            int         res = 0;
            timespec    ts;
            timeval     tv;
            ::gettimeofday(&tv, NULL);
            TIMEVAL_TO_TIMESPEC(&tv, &ts);
            ts.tv_nsec += (ms * 1000000);
            ts.tv_sec  += (ts.tv_nsec / 1000000000LL);
            ts.tv_nsec  = (ts.tv_nsec % 1000000000LL);
            {
                guard.activate(m);
                res = pthread_cond_timedwait(&cond_, &m.locker.handler, &ts);
            }

            if(res == ETIMEDOUT)
                return false;
            assert(res == 0);
            return true;
        }

#elif   defined(OS_WINDOWS)
#endif

    }
}
