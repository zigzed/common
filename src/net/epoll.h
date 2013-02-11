/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_NET_EPOLL_H
#define CXX_NET_EPOLL_H
#include "common/config.h"

#ifdef  OS_LINUX
#include "common/net/poller.h"
#include "common/sys/mutex.h"
#include <vector>
#include <sys/epoll.h>

namespace cxx {
    namespace net {

        class epoll : public poller {
        public:
            epoll();
            ~epoll();

            handle_t add_fd(fd_t fd, poller_event *sink);
            void     del_fd(handle_t handle);
            void     add_fd(handle_t handle, readable r);
            void     add_fd(handle_t handle, writable w);
            void     del_fd(handle_t handle, readable r);
            void     del_fd(handle_t handle, writable w);
        private:
            void     do_task(int timeout);
            void     destroy();

            struct epoll_entry {
                fd_t            fd;
                epoll_event     ev;
                poller_event*   cb;
            };

            typedef std::vector<epoll_entry* >  retired_t;
            /** handle for the epoll function */
            fd_t        epollfd_;
            retired_t   retired_;
            cxx::sys::plainmutex    mutex_;
        };

    }
}

#endif

#endif
