/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/net/poller.h"
#include <cassert>
#include "epoll.h"
#include "devpoll.h"
#include "kqueue.h"
#include "poll.h"
#include "select.h"

namespace cxx {
    namespace net {

#if     defined(OS_LINUX)
    #define NET_USE_EPOLL
#elif   defined(OS_WINDOWS)
    #define NET_USE_SELECT
#elif   defined(OS_FREEBSD) || defined(OS_NETBSD) || defined(OS_OPENBSD) || defined(OS_MACOS)
    #define NET_USE_KQUEUE
#elif   defined(OS_SOLARIS) || defined(OS_HPUX)
    #define NET_USE_DEVPOLL
#elif   defined(OS_QNXNTO) || defined(OS_AIX)
    #define NET_USE_POLL
#else
    #error unknown platform
#endif

        poller* poller::create()
        {
#if     defined(NET_USE_EPOLL)
            return new epoll();
#elif   defined(NET_USE_SELECT)
            return new select();
#elif   defined(NET_USE_KQUQUE)
            return new kqueue();
#elif   defined(NET_USE_DEVPOLL)
            return new devpoll();
#elif   defined(NET_USE_POLL)
            return new poll();
#else
            return new select();
#endif
        }

#undef  NET_USE_EPOLL
#undef  NET_USE_SELECT
#undef  NET_USE_KQUEUE
#undef  NET_USE_DEVPOLL
#undef  NET_USE_POLL

        poller::poller() : payload_(0), working_(true), timeout_(1000)
        {
        }

        poller::~poller()
        {
            stop();
            assert(get_loads() == 0);
        }

        int poller::get_loads() const
        {
            return (long)payload_;
        }

        void poller::adj_loads(int amount)
        {
            if(amount > 0) {
                payload_.add(amount);
            }
            else {
                amount *= -1;
                payload_.sub(amount);
            }
        }

        void poller::def_timer(int timeout)
        {
            timeout_ = timeout;
        }

        void poller::add_timer(intptr_t id, int timeout, poller_event *sink)
        {
            cxx::datetime time = cxx::datetime::now() + cxx::datetimespan(0, 0, 0, 0, timeout);
            timer_info    info = { id, sink };

            cxx::sys::plainmutex::scopelock lock(mutex_);
            expires_.insert(std::make_pair(time, info));
        }

        void poller::del_timer(intptr_t id, poller_event *sink)
        {
            cxx::sys::plainmutex::scopelock lock(mutex_);
            // time complexity of 'del_timer' is O(n). because it used rarely.
            for(timers_t::iterator it = expires_.begin(); it != expires_.end(); ++it) {
                if(it->second.id == id && it->second.sink == sink) {
                    expires_.erase(it);
                }
            }
        }

        int poller::exe_timer()
        {
            if(expires_.empty()) {
                return 0;
            }

            timers_t backup;
            {
                cxx::sys::plainmutex::scopelock lock(mutex_);
                backup.swap(expires_);
            }

            cxx::datetime current = cxx::datetime::now();
            timers_t::iterator it = backup.begin();
            while(it != backup.end()) {
                // std::multimap is sorted, so we can stop checking the subsequent
                // and return the time to wait for the next one.
                if(it->first > current) {
                    {
                        cxx::sys::plainmutex::scopelock lock(mutex_);
                        expires_.insert(backup.begin(), backup.end());
                    }
                    return (it->first - current).getTotalMilliSeconds();
                }

                it->second.sink->on_expire(it->second.id);
                backup.erase(it++);
            }

            {
                cxx::sys::plainmutex::scopelock lock(mutex_);
                expires_.insert(backup.begin(), backup.end());
            }

            return 0;
        }

        void poller::stop()
        {
            working_ = false;
        }

        void poller::poll()
        {
            while(working_) {
                int timer = exe_timer();
                if(timer == 0) {
                    timer = timeout_;
                }
                do_task(timer);
            }
        }

        int poller::poll_once(int ms)
        {
            int timer = exe_timer();
            if(timer == 0)
                timer = timeout_;
            if(timer > ms)
                timer = ms;
            return do_task(timer);
        }

    }
}
