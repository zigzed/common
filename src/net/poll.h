/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_NET_POLL_H
#define CXX_NET_POLL_H
#include "common/config.h"
#include "common/net/poller.h"

#if defined(OS_QNXNTO) || defined(OS_AIX) || defined(OS_LINUX)
#include <sys/poll.h>
#include <vector>

namespace cxx {
    namespace net {

        class poll : public poller {
        public:
            poll();
            ~poll();

            handle_t add_fd(fd_t fd, poller_event *sink);
            void     del_fd(handle_t handle);
            void     add_fd(handle_t handle, readable r);
            void     add_fd(handle_t handle, writable w);
            void     del_fd(handle_t handle, readable r);
            void     del_fd(handle_t handle, writable w);
        private:
            void     do_task(int timeout);
            void     destroy();

            struct poll_entry {
                fd_t            id;
                poller_event*   cb;
            };

            typedef std::vector<poll_entry >    fdtable_t;
            typedef std::vector<pollfd >        pollset_t;

            fdtable_t   tables_;
            pollset_t   pollfd_;
            bool        retire_;
        };

    }
}

#endif

#endif
