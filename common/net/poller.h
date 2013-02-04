/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_POLLER_H
#define CXX_NET_POLLER_H
#include <map>
#include "common/config.h"
#include "common/sys/atomic.h"
#include "common/sys/threads.h"
#include "common/datetime.h"

namespace cxx {
    namespace net {

#if     defined(OS_LINUX)
            typedef int     fd_t;
#elif   defined(OS_WINDOWS)
            typedef SOCKET  fd_t;
#else
#error unknown platform
#endif

        class event_sink {
        public:
            virtual ~event_sink() {}

            virtual void on_readable() = 0;
            virtual void on_writable() = 0;
            virtual void on_expire(int id) = 0;
        };

        class poller {
        public:
            typedef void*   handle_t;
            struct readable {};
            struct writable {};

            static poller* create();



            /** add a timeout to expire in 'timeout' milliseconds. after the
             * expiration, event_sink::on_expire() will be called with timer
             * identifier 'id'
             */
            void add_timer(int id, int timeout, event_sink* sink);
            /** cancel a timer */
            void del_timer(int id, event_sink* sink);
            /** default timeout to expire when no 'timer' registered. */
            void def_timer(int timeout);
            /** return current load of the poller */
            int  get_loads() const;

            void start();
            void stop ();

            /** add 'fd' to the poller */
            virtual handle_t    add_fd(fd_t fd, event_sink* sink) = 0;
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
            virtual void do_task(int timeout) = 0;
        private:
            void polling();
            poller(const poller& rhs);
            poller& operator= (const poller& rhs);

            struct timer_info {
                int         id;
                event_sink* sink;
            };

            typedef std::multimap<cxx::datetime, timer_info >   timers_t;

            cxx::sys::atomic_t  payload_;
            timers_t            expires_;
            cxx::sys::thread    threads_;
            bool                working_;
            int                 timeout_;
        };

    }
}

#endif
