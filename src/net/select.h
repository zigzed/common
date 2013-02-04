/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_NET_SELECT_H
#define CXX_NET_SELECT_H
#include "common/config.h"
#include "common/net/poller.h"

#if defined(OS_WINDOWS) || defined(OS_LINUX)

#if     defined(OS_WINDOWS)
#include <winsock2.h>
#elif   defined(OS_LINUX)
#include <sys/select.h>
#endif
#include <vector>

namespace cxx {
    namespace net {

        class select : public poller {
        public:
            select();
            ~select();

            handle_t add_fd(fd_t fd, event_sink *sink);
            void     del_fd(handle_t handle);
            void     add_fd(handle_t handle, readable r);
            void     add_fd(handle_t handle, writable w);
            void     del_fd(handle_t handle, readable r);
            void     del_fd(handle_t handle, writable w);
        private:
            struct select_entry {
                fd_t        fd;
                event_sink* cb;
            };
            typedef std::vector<select_entry >  fdset_t;

            void        do_task(int timeout);
            void        destroy();
            static bool is_removed(const select_entry& se);

            fd_t    max_fd_;

            fd_set  src_rd_;
            fd_set  src_wr_;
            fd_set  src_er_;
            fd_set  cur_rd_;
            fd_set  cur_wr_;
            fd_set  cur_er_;

            fdset_t fd_set_;
            bool    retire_;
        };

    }
}

#endif

#endif
