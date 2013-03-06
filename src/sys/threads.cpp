/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <string>
#include <stdio.h>  // for fprintf and stderr
#include "common/config.h"
#include "common/sys/threads.h"
#if !defined(OS_WINDOWS)
    #include <sched.h>  // for sched_yield
    #include <unistd.h>
    #include <sys/syscall.h>
#endif

namespace cxx {
    namespace sys {

#if defined(OS_WINDOWS)
        thread_t thread_self()
        {
            thread_t handle;
            handle.fd = GetCurrentThread();
            handle.id = GetCurrentThreadId();
            return handle;
        }

        thread_t thread_create(unsigned (__stdcall *start_address)(void* ), void* param)
        {
            thread_t handle;
            handle.fd = (HANDLE)_beginthreadex(0, 0, start_address, param, 0, &(handle.id));
            return handle;
        }

        void thread_destroy(thread_t& handle)
        {
            CloseHandle(handle.fd);
        }

        void thread_cancel(thread_t& handle)
        {
            TerminateThread(handle.fd, 0);
        }

        bool thread_equal(const thread_t& lhs, const thread_t& rhs)
        {
            return lhs.id == rhs.id;
        }

        void thread_join(thread_t& handle)
        {
            ::WaitForSingleObject(handle.fd, INFINITE);
            ::CloseHandle(handle.fd);
        }

        void thread_sleep(unsigned long msec)
        {
            Sleep(msec);
        }

        void thread_yield()
        {
            Sleep(0);
        }

#else

        thread_t thread_self()
        {
            return pthread_self();
        }

        thread_t thread_create(void* (*start_address)(void* ), void* param)
        {
            thread_t handle = 0;
            pthread_create(&handle, 0, start_address, param);
            return handle;
        }

        void thread_destroy(thread_t& handle)
        {
            pthread_detach(handle);
        }

        void thread_cancel(thread_t& handle)
        {
            pthread_cancel(handle);
        }

        bool thread_equal(const thread_t& lhs, const thread_t& rhs)
        {
            return pthread_equal(lhs, rhs);
        }

        void thread_join(thread_t& handle)
        {
            pthread_join(handle, 0);
        }

        void thread_sleep(unsigned long msec)
        {
            timespec ts;
            ts.tv_sec = msec / 1000;
            ts.tv_nsec= (msec % 1000) * 1000;
            nanosleep(&ts, 0);
        }

        void thread_yield()
        {
            // pthread_yield() is nonstandard, but present on several other systems.
            // sched_yield() is the standardized.
            sched_yield();
        }

#endif

        thread::thread() : refcnt(new long(1)), isjoin(false)
        {
            handle = thread_self();
        }

        thread::~thread()
        {
            --*refcnt;
            if(*refcnt == 0) {
                if(isjoin)
                    thread_destroy(handle);
                delete refcnt;
                refcnt = 0;
            }
        }

        thread::thread(const thread& rhs) : refcnt(rhs.refcnt)
        {
            handle = rhs.handle;
            isjoin = rhs.isjoin;
            ++*refcnt;
        }

        thread& thread::operator =(const thread& rhs)
        {
            if(this == &rhs)
                return *this;

            delete refcnt;
            handle = rhs.handle;
            refcnt = rhs.refcnt;
            isjoin = rhs.isjoin;
            ++*refcnt;
            return *this;
        }

        bool thread::operator ==(const thread& rhs) const
        {
            return thread_equal(handle, rhs.handle);
        }

        bool thread::operator !=(const thread& rhs) const
        {
            return !thread_equal(handle, rhs.handle);
        }

        void thread::join()
        {
            if(isjoin) {
                thread_join(handle);
                isjoin = false;
            }
        }

        void thread::stop()
        {
            thread_cancel(handle);
        }

        namespace detail {
#if defined(OS_WINDOWS)
        template<typename Func >
        struct threadarg {
            threadarg(const Func& func, const char* name) : mutex_(NULL), threadfunc(func), name_(name) {
                mutex_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            }
            void waitfin() {
                ::WaitForSingleObject(mutex_, INFINITE);
            }
            void started() {
                ::SetEvent(mutex_);
            }
            const char* name() const {
                return name_.c_str();
            }
            ~threadarg() {
                ::CloseHandle(mutex_);
            }

            HANDLE			mutex_;
            const Func&     threadfunc;
            std::string     name_;
        };
#else
            template<typename Func >
            struct threadarg {
                threadarg(const Func& func, const char* name) : name_(name), isstart(false), threadfunc(func) {
                    pthread_mutex_init(&mutex_, 0);
                    pthread_cond_init(&conds_, 0);
                }
                ~threadarg() {
                    pthread_mutex_destroy(&mutex_);
                    pthread_cond_destroy(&conds_);
                }
                void waitfin() {
                    pthread_mutex_lock(&mutex_);
                    while(!isstart) {
                        pthread_cond_wait(&conds_, &mutex_);
                    }
                    pthread_mutex_unlock(&mutex_);
                }
                void started() {
                    pthread_mutex_lock(&mutex_);
                    isstart = true;
                    pthread_cond_signal(&conds_);
                    pthread_mutex_unlock(&mutex_);
                }
                const char* name() const {
                    return name_.c_str();
                }

                pthread_mutex_t	mutex_;
                pthread_cond_t	conds_;
                std::string		name_;
                bool			isstart;
                const Func&	    threadfunc;
            };
#endif

#if defined(OS_WINDOWS)
            unsigned __stdcall threadproxy(void* param)
#else
            void* threadproxy(void* param)
#endif
            {
                typedef cxx::FastDelegate0<cxx::detail::DefaultVoid >    Delegate;
                typedef threadarg<Delegate >                        ThreadArg;
                std::string name;
                try {
                    ThreadArg* p = static_cast<ThreadArg* >(param);
                    name = p->name();
                    // 拷贝构造，防止 threadarg<Delegate >::threadfunc 在 create 函数
                    // 中离开作用域。
                    Delegate threadfunc = p->threadfunc;
                    p->started();
                    threadfunc();
                }
                catch(...) {
                    fprintf(stderr, "thread '%s' exception catched and aborted\n", name.c_str());
                }

                return 0;
            }
        }


        thread threadcontrol::create(const Delegate& func, const char* name)
        {
            typedef detail::threadarg<Delegate >    ThreadArg;
            ThreadArg	param(func, name);
            thread		thrds;
            thrds.handle = thread_create(detail::threadproxy, &param);
            thrds.isjoin = true;
            param.waitfin();
            return thrds;
        }

        void threadcontrol::sleep(long msec)
        {
            thread_sleep(msec);
        }

        void threadcontrol::yield()
        {
            thread_yield();
        }

        void threadcontrol::cancel(thread& thrd, long waittime)
        {
            if(waittime < 0)
                thrd.join();
            else {
#if defined(OS_WINDOWS)
                DWORD ret = ::WaitForSingleObject(thrd.handle.fd, waittime);
                if(ret == WAIT_TIMEOUT)
                    thread_cancel(thrd.handle);
#else
                pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
                pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
                sleep(waittime);
                thread_cancel(thrd.handle);
#endif
            }
        }

        size_t threadcontrol::thrdid()
        {
#if defined(OS_WINDOWS)
            return GetCurrentThreadId();
#else
            return syscall(__NR_gettid);
#endif

        }

        int threadcontrol::policy(int len, int *cpu)
        {
#if defined(OS_WINDOWS)
            DWORD_PTR   set = 0;
            for(int i = 0; i < len; ++i) {
                set |= (1 << cpu[i]);
            }
            return SetThreadAffinityMask(GetCurrentThread(), set);
#else
            pid_t       tid = syscall(__NR_gettid);
            cpu_set_t   set;
            CPU_ZERO(&set);
            for(int i = 0; i < len; ++i) {
                CPU_SET(cpu[i], &set);
            }
            return sched_setaffinity(tid, sizeof(set), &set);
#endif
        }

    }
}
