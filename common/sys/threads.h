/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_SYS_THREAD_H
#define	CXX_SYS_THREAD_H
#include "common/config.h"
#if defined(OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif
#include "common/fastdelegate.h"

namespace cxx {
    namespace sys {

#if defined(OS_WINDOWS)
        struct thread_t {
            HANDLE          fd;
            unsigned int    id;
        };
#else
        typedef pthread_t	thread_t;
#endif

        class thread {
        public:
            thread();
            ~thread();
            thread(const thread& rhs);
            thread& operator= (const thread& rhs);

            bool operator== (const thread& rhs) const;
            bool operator!= (const thread& rhs) const;
            void join();
            void stop();
        private:
            friend class threadcontrol;
            thread_t	handle;
            long*		refcnt;
            bool		isjoin;
        };

        class threadpool {
        public:
            struct worker {
                virtual ~worker();
                virtual void execute() = 0;
            };

            threadpool(long threadnumber = 0, long stacksize = 0);
            ~threadpool();
            void shutdown(long wait = 0);
            void pushback(worker* job);
            void size(long threadnumber);
            long size() const;
        private:
            struct threadpoolimpl;
            threadpoolimpl* impl_;
        };

        class threadcontrol {
        public:
            typedef cxx::FastDelegate0<cxx::detail::DefaultVoid >    Delegate;
            static thread   create(const Delegate& func, const char* name = "");
            static void		yield();
            static void		sleep(long msec);
            static void		cancel(thread& thrd, long waittime);
            static size_t   thrdid();
            // confine thread to processor
            static int      policy(int len, int* cpu);
        };

        ///////////////////////////////////////////////////////////////////////////
        // please note, threadcontrol::create() syntax changed, because old version
        // has bug in virtual inheritance member function
        // here is the example
        /**
         * class Test {
         * public:
         *     void run_in_thread() {}
         * };
         * Test test;
         * cxx::threadcontrol::create(cxx::MakeDelegate(&test, &Test::run_in_thread));
         *
         * void test_thread() {}
         * cxx::threadcontrol::create(cxx::MakeDelegate(&test_thread));
         *
         */

    }
}

#endif

