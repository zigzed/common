/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_POLLER_H
#define CXX_NET_POLLER_H
#include <map>
#include "common/config.h"
#include "common/sys/atomic.h"
#include "common/sys/threads.h"
#include "common/datetime.h"
#include "common/sys/mutex.h"

namespace cxx {
    namespace net {

#if     defined(OS_LINUX)
            typedef int     fd_t;
#elif   defined(OS_WINDOWS)
            typedef SOCKET  fd_t;
#else
#error unknown platform
#endif

        class poller_event {
        public:
            virtual ~poller_event() {}

            virtual void on_readable(fd_t fd) {}
            virtual void on_writable(fd_t fd) {}
            virtual void on_expire(int id) {}
        };

        class poller {
        public:
            typedef void*   handle_t;
            struct readable {};
            struct writable {};

            static poller* create();

            /** add a timeout to expire in 'timeout' milliseconds. after the
             * expiration, poller_event::on_expire() will be called with timer
             * identifier 'id'
             */
            void add_timer(intptr_t id, int timeout, poller_event* sink);
            /** cancel a timer */
            void del_timer(intptr_t id, poller_event* sink);
            /** default timeout to expire when no 'timer' registered. */
            void def_timer(int timeout);
            /** return current load of the poller */
            int  get_loads() const;

            /** start polling in loop */
            void poll();
            /** poll and wait at most 'ms' milliseconds once
             * @return return number of events in poll */
            int  poll_once(int ms);
            /** stop the polling */
            void stop();

            /** add 'fd' to the poller */
            virtual handle_t    add_fd(fd_t fd, poller_event* sink) = 0;
            /** remove 'fd' from the poller */
            virtual void        del_fd(handle_t handle) = 0;
            /** register 'handle' to readable event */
            virtual void        add_fd(handle_t handle, readable r) = 0;
            /** register 'handle' to writable event */
            virtual void        add_fd(handle_t handle, writable w) = 0;
            /** unregister 'handle' to readable event */
            virtual void        del_fd(handle_t handle, readable r) = 0;
            /** unregister 'handle' to writable event */
            virtual void        del_fd(handle_t handle, writable w) = 0;
            virtual void        destroy() = 0;
        protected:
            poller();
            virtual ~poller();

            /** called be poller implementations to manage the load */
            void adj_loads(int amount);
            /** execute any timer are due. return number of milliseconds to wait
             * to match the next timer or 0 meaning 'no timers'
             */
            int  exe_timer();

            /** poller implementations must implement this method to do the
             * real multiplexing. 'timeout' is wait time for multiplex */
            virtual int do_task(int timeout) = 0;
        private:
            void polling();
            poller(const poller& rhs);
            poller& operator= (const poller& rhs);

            struct timer_info {
                intptr_t      id;
                poller_event* sink;
            };

            typedef std::multimap<cxx::datetime, timer_info >   timers_t;

            cxx::sys::plainmutex mutex_;
            cxx::sys::atomic_t  payload_;
            timers_t            expires_;
            bool                working_;
            int                 timeout_;
        };

    }
}

#endif
