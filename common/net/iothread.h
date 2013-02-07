/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_IOTHREAD_H
#define CXX_NET_IOTHREAD_H
#include "common/config.h"
#include "common/sys/event.h"
#include "common/net/poller.h"

namespace cxx {
    namespace net {

        class io_thread : public event_sink {
        public:
            io_thread();
            ~io_thread();

            void    start();
            void    stop();
            poller* get_poller();
            int     get_load();
        private:
            void    on_readable();
            void    on_writable();
            void    on_expire(int id);

            sys::event          events_;
            poller::handle_t    hevent_;
            poller*             poller_;

            io_thread(const io_thread& rhs);
            io_thread& operator= (const io_thread& rhs);
        };

    }
}

#endif
