/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <cassert>
#include "common/config.h"
#include "common/sys/mutex.h"

namespace cxx {
    namespace sys {

#if defined(OS_WINDOWS)

        plainmutex::plainmutex()
        {
            ::InitializeCriticalSection(&locker);
        }

        plainmutex::~plainmutex()
        {
            ::DeleteCriticalSection(&locker);
        }

        void plainmutex::acquire()
        {
            ::EnterCriticalSection(&locker);
        }

        void plainmutex::release()
        {
            ::LeaveCriticalSection(&locker);
        }

        triedmutex::triedmutex()
        {
            locker = ::CreateMutex(NULL, NULL, NULL);
            assert(locker != NULL);
        }

        triedmutex::~triedmutex()
        {
            ::CloseHandle(locker);
        }

        void triedmutex::acquire()
        {
            DWORD res = ::WaitForSingleObject(locker, INFINITE);
            assert(res == WAIT_OBJECT_0);
        }

        void triedmutex::release()
        {
            BOOL res = ::ReleaseMutex(locker);
            assert(res == TRUE);
        }

        bool triedmutex::trylock()
        {
            DWORD res = ::WaitForSingleObject(locker, 0);
            assert(res != WAIT_FAILED && res != WAIT_ABANDONED);
            return res == WAIT_OBJECT_0;
        }

        timedmutex::timedmutex()
        {
            locker = ::CreateMutex(NULL, NULL, NULL);
            assert(locker != NULL);
        }

        timedmutex::~timedmutex()
        {
            ::CloseHandle(locker);
        }

        void timedmutex::acquire()
        {
            DWORD res = ::WaitForSingleObject(locker, INFINITE);
            assert(res == WAIT_OBJECT_0);
        }

        void timedmutex::release()
        {
            BOOL res = ::ReleaseMutex(locker);
            assert(res == TRUE);
        }

        bool timedmutex::trylock()
        {
            DWORD res = ::WaitForSingleObject(locker, 0);
            assert(res != WAIT_FAILED && res != WAIT_ABANDONED);
            return res == WAIT_OBJECT_0;
        }

        bool timedmutex::waitfor(unsigned long msec)
        {
            DWORD res = ::WaitForSingleObject(locker, msec);
            assert(res != WAIT_FAILED && res != WAIT_ABANDONED);
            return res == WAIT_OBJECT_0;
        }

        rwlock::rwlock()
        {
            locker.counter = -1;
            locker.rdevent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            assert(locker.rdevent != NULL);
            locker.gateway = ::CreateEvent(NULL, FALSE, TRUE, NULL);
            assert(locker.gateway != NULL);
            locker.wtmutex = ::CreateMutex(NULL, FALSE, NULL);
            assert(locker.wtmutex != NULL);
        }

        rwlock::~rwlock()
        {
            ::CloseHandle(locker.wtmutex);
            ::CloseHandle(locker.gateway);
            ::CloseHandle(locker.rdevent);
        }

        void rwlock::acquire(bool read)
        {
            if(!read) {
                ::WaitForSingleObject(locker.wtmutex, INFINITE);
                ::WaitForSingleObject(locker.gateway, INFINITE);
            }
            else {
                if(::InterlockedIncrement(&(locker.counter)) == 0) {
                    ::WaitForSingleObject(locker.gateway, INFINITE);
                    ::SetEvent(locker.rdevent);
                }
                ::WaitForSingleObject(locker.rdevent, INFINITE);
            }
        }

        void rwlock::release(bool read)
        {
            if(!read) {
                ::SetEvent(locker.gateway);
                ::ReleaseMutex(locker.wtmutex);
            }
            else {
                if(::InterlockedDecrement(&(locker.counter)) < 0) {
                    ::ResetEvent(locker.rdevent);
                    ::SetEvent(locker.gateway);
                }
            }
        }

#else

#ifdef __CYGWIN__
#define        PTHREAD_MUTEX_FAST_NP                   PTHREAD_MUTEX_NORMAL
#define        PTHREAD_MUTEX_RECURSIVE_NP              PTHREAD_MUTEX_RECURSIVE
#define        PTHREAD_MUTEX_ERRORCHECK_NP             PTHREAD_MUTEX_ERRORCHECK
#define        PTHREAD_MUTEX_TIMED_NP                  PTHREAD_MUTEX_NORMAL
#define        PTHREAD_MUTEX_ADAPTIVE_NP               PTHREAD_MUTEX_NORMAL
#endif /* __CYGWIN__ */

        plainmutex::plainmutex()
        {
            pthread_mutexattr_init(&locker.mtxattr);
            int	type = 0;
            type = PTHREAD_MUTEX_RECURSIVE_NP;
            pthread_mutexattr_settype(&locker.mtxattr, type);
            int res = pthread_mutex_init(&locker.handler, &locker.mtxattr);
            assert(res == 0);
        }

        plainmutex::~plainmutex()
        {
            int res = pthread_mutex_destroy(&locker.handler);
            assert(res == 0);
            pthread_mutexattr_destroy(&locker.mtxattr);
        }

        void plainmutex::acquire()
        {
            int res = pthread_mutex_lock(&locker.handler);
            assert(res == 0);
        }

        void plainmutex::release()
        {
            int res = pthread_mutex_unlock(&locker.handler);
            assert(res == 0);
        }

        triedmutex::triedmutex()
        {
            int res = pthread_mutex_init(&locker, 0);
            assert(res == 0);
        }

        triedmutex::~triedmutex()
        {
            int res = pthread_mutex_destroy(&locker);
            assert(res == 0);
        }

        void triedmutex::acquire()
        {
            int res = pthread_mutex_lock(&locker);
            assert(res == 0);
        }

        void triedmutex::release()
        {
            int res = pthread_mutex_unlock(&locker);
            assert(res == 0);
        }

        bool triedmutex::trylock()
        {
            int res = pthread_mutex_trylock(&locker);
            assert(res == 0 || res == EBUSY);
            return res == 0;
        }

        namedmutex::namedmutex(const char *name)
        {
            locker.mutex = sem_open(name, O_RDWR | O_CREAT, 0644, 1);
            assert(locker.mutex != 0);
        }

        namedmutex::~namedmutex()
        {
            int res = sem_close(locker.mutex);
            assert(res == 0);
        }

        void namedmutex::acquire()
        {
            int res = sem_wait(locker.mutex);
            assert(res == 0);
        }

        void namedmutex::release()
        {
            int res = sem_post(locker.mutex);
            assert(res == 0);
        }

        bool namedmutex::trylock()
        {
            int res = sem_trywait(locker.mutex);
            assert(res == 0 || res == EAGAIN);
            return res == 0;
        }

        bool namedmutex::waitfor(unsigned long msec)
        {
            timeval     tv;
            gettimeofday(&tv, NULL);
            timespec    ts;
            tv.tv_usec += msec;
            ts.tv_sec = tv.tv_sec + (tv.tv_usec / 1000);
            ts.tv_nsec= (tv.tv_usec % 1000) * 1000000;
            int res = sem_timedwait(locker.mutex, &ts);
            assert(res == 0 || res == ETIMEDOUT);
            return res == 0;
        }

        timedmutex::timedmutex()
        {
            int res = 0;
            res = pthread_mutex_init(&(locker.mutex), 0);
            assert(res == 0);
            res = pthread_cond_init(&(locker.conds), 0);
            assert(res == 0);
            locker.lockd = false;
        }

        timedmutex::~timedmutex()
        {
            assert(!locker.lockd);
            int res = 0;
            res = pthread_mutex_destroy(&(locker.mutex));
            assert(res == 0);
            res = pthread_cond_destroy(&(locker.conds));
            assert(res == 0);
        }

        void timedmutex::acquire()
        {
            int res = 0;
            res = pthread_mutex_lock(&(locker.mutex));
            assert(res == 0);
            while(locker.lockd) {
                res = pthread_cond_wait(&(locker.conds), &(locker.mutex));
                assert(res == 0);
            }
            assert(!locker.lockd);
            locker.lockd = true;

            res = pthread_mutex_unlock(&(locker.mutex));
            assert(res == 0);
        }

        bool timedmutex::trylock()
        {
            int res = 0;
            res = pthread_mutex_lock(&(locker.mutex));
            assert(res == 0);
            bool ret = false;
            if(!locker.lockd) {
                locker.lockd = true;
                ret = true;
            }
            res = pthread_mutex_unlock(&(locker.mutex));
            assert(res == 0);
            return ret;
        }

        bool timedmutex::waitfor(unsigned long msec)
        {
            timespec ts;
            int res = 0;
            res = pthread_mutex_lock(&(locker.mutex));
            assert(res == 0);

            while(locker.lockd) {
                res = pthread_cond_timedwait(&(locker.conds), &(locker.mutex), &ts);
                assert(res == 0 || res == ETIMEDOUT);
                if(res == ETIMEDOUT)
                    break;
            }

            bool ret = false;
            if(!locker.lockd) {
                locker.lockd = true;
                ret = true;
            }

            res = pthread_mutex_unlock(&(locker.mutex));
            assert(res == 0);
            return ret;
        }

        void timedmutex::release()
        {
            int res = 0;
            res = pthread_mutex_lock(&(locker.mutex));
            assert(res == 0);
            assert(locker.lockd);
            locker.lockd = false;

            res = pthread_cond_signal(&(locker.conds));
            assert(res == 0);

            res = pthread_mutex_unlock(&(locker.mutex));
            assert(res == 0);
        }

        rwlock::rwlock()
        {
            pthread_rwlockattr_init(&locker.rwlattr);
            int shd = PTHREAD_PROCESS_PRIVATE;
            pthread_rwlockattr_setpshared(&locker.rwlattr, shd);
            int res = pthread_rwlock_init(&locker.handler, &locker.rwlattr);
            assert(res == 0);
        }

        rwlock::~rwlock()
        {
            int res = pthread_rwlock_destroy(&locker.handler);
            assert(res == 0);
            pthread_rwlockattr_destroy(&locker.rwlattr);
        }

        void rwlock::acquire(bool read)
        {
            int res = 0;
            if(read) {
                res = pthread_rwlock_rdlock(&locker.handler);
            }
            else {
                res = pthread_rwlock_wrlock(&locker.handler);
            }
            assert(res == 0);
        }

        void rwlock::release(bool /*read*/)
        {
            int res = pthread_rwlock_unlock(&locker.handler);
            assert(res == 0);
        }
#endif

    }

}
