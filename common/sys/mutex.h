/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_SYS_MUTEX_H
#define	CXX_SYS_MUTEX_H
#include "common/config.h"
#include "common/sys/locker.h"
#include "common/sys/error.h"
#ifndef OS_WINDOWS
    #include <sys/time.h>
    #include <fcntl.h>
    #include <pthread.h>
    #include <semaphore.h>
#endif

namespace cxx {
    namespace sys {

        namespace detail {
#if defined(OS_WINDOWS)
            typedef CRITICAL_SECTION    plainmutex_t;
            typedef HANDLE              triedmutex_t;
            typedef HANDLE              timedmutex_t;
            struct rwlock_t {
                HANDLE  rdevent;
                HANDLE  wtmutex;
                HANDLE  gateway;
                long    counter;
            };
#else
            struct plainmutex_t {
                pthread_mutex_t		handler;
                pthread_mutexattr_t	mtxattr;
            };
            typedef	pthread_mutex_t		triedmutex_t;

            struct timedmutex_t {
                pthread_mutex_t	mutex;
                pthread_cond_t	conds;
                bool			lockd;
            };

            struct namedmutex_t {
                sem_t*          mutex;
            };

            struct rwlock_t {
                pthread_rwlock_t        handler;
                pthread_rwlockattr_t    rwlattr;
            };
#endif

        }

        class plainmutex {
            friend class detail::syncobj<plainmutex>;
        public:
            typedef detail::plainmutex_t            mutex_t;
            typedef detail::scopelock<plainmutex>	scopelock;
            plainmutex();
            ~plainmutex();
            void acquire();
            void release();
        private:
            plainmutex(const plainmutex& );
            plainmutex& operator= (const plainmutex& );

            detail::plainmutex_t	locker;
        };

        class triedmutex {
            friend class detail::syncobj<triedmutex>;
        public:
            typedef detail::scopelock<triedmutex>	scopelock;
            typedef detail::triedlock<triedmutex>	triedlock;

            triedmutex();
            ~triedmutex();
        private:
            triedmutex(const triedmutex& );
            triedmutex& operator= (const triedmutex& );

            void acquire();
            void release();
            bool trylock();

            detail::triedmutex_t	locker;
        };

        class timedmutex {
            friend class detail::syncobj<timedmutex>;
        public:
            typedef detail::scopelock<timedmutex>	scopelock;
            typedef detail::triedlock<timedmutex>	triedlock;
            typedef detail::timedlock<timedmutex>	timedlock;

            timedmutex();
            ~timedmutex();
        private:
            timedmutex(const timedmutex& );
            timedmutex& operator= (const timedmutex& );

            void acquire();
            void release();
            bool trylock();
            bool waitfor(unsigned long msec);

            detail::timedmutex_t	locker;
        };

        // named mutex, used for inter-process synchronization
        class namedmutex {
            friend class detail::syncobj<namedmutex>;
        public:
            typedef detail::scopelock<namedmutex>   scopelock;
            typedef detail::triedlock<namedmutex>   triedlock;
            typedef detail::timedlock<namedmutex>   timedlock;

            explicit namedmutex(const char* name);
            ~namedmutex();
        private:
            namedmutex(const namedmutex& );
            namedmutex& operator= (const namedmutex& );

            void acquire();
            void release();
            bool trylock();
            bool waitfor(unsigned long msec);

            detail::namedmutex_t    locker;
        };

        class rwlock {
            friend class detail::syncobj<rwlock>;
        public:
            typedef detail::readerlock<rwlock>		readerlock;
            typedef detail::writerlock<rwlock>		writerlock;

            rwlock();
            ~rwlock();
        private:
            rwlock(const rwlock& );
            rwlock& operator= (const rwlock& );

            void acquire(bool read);
            void release(bool read);

            detail::rwlock_t		locker;
        };

    }

}

#endif
