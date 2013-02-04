#include "poll.h"

#if defined(OS_QNXNTO) || defined(OS_AIX) || defined(OS_LINUX)
#include <errno.h>
#include <cassert>

namespace cxx {
    namespace net {

        poll::poll()
            : retire_(false)
        {
        }

        poller::handle_t poll::add_fd(fd_t fd, event_sink *sink)
        {
            int sz = tables_.size();
            if(sz <= fd) {
                tables_.resize(fd + 1);
                while(sz != fd) {
                    tables_[sz].id = -1;
                    ++sz;
                }
            }
            pollfd pfd = { fd, 0, 0 };
            pollfd_.push_back(pfd);
            assert(tables_[fd].id == -1);

            tables_[fd].id = pollfd_.size() - 1;
            tables_[fd].cb = sink;

            adj_loads(1);

            return (handle_t)(long)fd;
        }

        void poll::del_fd(handle_t handle)
        {
            fd_t fd = (fd_t)(long)handle;
            fd_t id = tables_[fd].id;
            assert(id != -1);

            pollfd_[id].fd = -1;
            tables_[fd].id = -1;
            retire_ = true;

            adj_loads(-1);
        }

        void poll::add_fd(handle_t handle, readable r)
        {
            fd_t fd = (fd_t)(long)handle;
            int  id = tables_[fd].id;
            pollfd_[id].events |= POLLIN;
        }

        void poll::add_fd(handle_t handle, writable w)
        {
            fd_t fd = (fd_t)(long)handle;
            int  id = tables_[fd].id;
            pollfd_[id].events |= POLLOUT;
        }

        void poll::del_fd(handle_t handle, readable r)
        {
            fd_t fd = (fd_t)(long)handle;
            int  id = tables_[fd].id;
            pollfd_[id].events &= ~((short)POLLIN);
        }

        void poll::del_fd(handle_t handle, writable w)
        {
            fd_t fd = (fd_t)(long)handle;
            int  id = tables_[fd].id;
            pollfd_[id].events &= ~((short)POLLOUT);
        }

        void poll::do_task(int timeout)
        {
            int rc = ::poll(&pollfd_[0], pollfd_.size(), timeout);
            if(rc == 0)
                return;
            if(rc < 0)
                assert(errno == EINTR);

            for(int i = 0; i < pollfd_.size(); ++i) {
                if(pollfd_[i].fd == -1)
                    continue;
                if(pollfd_[i].revents & (POLLERR | POLLHUP))
                    tables_[pollfd_[i].fd].cb->on_readable();
                if(pollfd_[i].fd == -1)
                    continue;
                if(pollfd_[i].revents & POLLIN)
                    tables_[pollfd_[i].fd].cb->on_readable();
                if(pollfd_[i].fd == -1)
                    continue;
                if(pollfd_[i].revents & POLLOUT)
                    tables_[pollfd_[i].fd].cb->on_writable();
            }

            if(retire_) {
                int i = 0;
                while(i < pollfd_.size()) {
                    if(pollfd_[i].fd == -1) {
                        pollfd_.erase(pollfd_.begin() + i);
                    }
                    else {
                        tables_[pollfd_[i].fd].id = i;
                        i++;
                    }
                }
                retire_ = false;
            }
        }

    }
}

#endif
