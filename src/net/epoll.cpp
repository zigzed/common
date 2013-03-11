/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "epoll.h"
#include "common/sys/error.h"
#ifdef  OS_LINUX
#include <unistd.h>

namespace cxx {
    namespace net {

        epoll::epoll()
            : epollfd_(-1)
        {
            epollfd_ = epoll_create(5);
            assert(epollfd_ != -1);
        }

        epoll::~epoll()
        {
            close(epollfd_);
            for(retired_t::iterator it = retired_.begin(); it != retired_.end(); ++it) {
                delete *it;
            }
        }

        poller::handle_t epoll::add_fd(fd_t fd, poller_event *sink)
        {
            epoll_entry* pe = new epoll_entry();
            pe->fd          = fd;
            pe->ev.events   = 0;
            pe->ev.data.ptr = pe;
            pe->cb          = sink;

            int rc = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &pe->ev);
            ENFORCE(rc != -1 /*|| cxx::sys::err::get() == EEXIST*/)(rc)(cxx::sys::err::get());

            adj_loads(1);
            return pe;
        }

        void epoll::del_fd(handle_t handle)
        {
            epoll_entry* pe = (epoll_entry* )handle;
            int rc = epoll_ctl(epollfd_, EPOLL_CTL_DEL, pe->fd, &pe->ev);
            ENFORCE(rc != -1)(rc)(cxx::sys::err::get());
            pe->fd = -1;
            {
                cxx::sys::plainmutex::scopelock lock(mutex_);
                retired_.push_back(pe);
            }

            adj_loads(-1);
        }

        void epoll::add_fd(handle_t handle, readable r)
        {
            epoll_entry* pe = (epoll_entry* )handle;
            pe->ev.events |= EPOLLIN;
            int rc = epoll_ctl(epollfd_, EPOLL_CTL_MOD, pe->fd, &pe->ev);
            ENFORCE(rc != -1)(rc)(cxx::sys::err::get());
        }

        void epoll::add_fd(handle_t handle, writable w)
        {
            epoll_entry* pe = (epoll_entry* )handle;
            pe->ev.events |= EPOLLOUT;
            int rc = epoll_ctl(epollfd_, EPOLL_CTL_MOD, pe->fd, &pe->ev);
            ENFORCE(rc != -1)(rc)(cxx::sys::err::get());
        }

        void epoll::del_fd(handle_t handle, readable r)
        {
            epoll_entry* pe = (epoll_entry* )handle;
            pe->ev.events &= ~((short)EPOLLIN);
            int rc = epoll_ctl(epollfd_, EPOLL_CTL_MOD, pe->fd, &pe->ev);
            ENFORCE(rc != -1)(rc)(cxx::sys::err::get());
        }

        void epoll::del_fd(handle_t handle, writable r)
        {
            epoll_entry* pe = (epoll_entry* )handle;
            pe->ev.events &= ~((short)EPOLLOUT);
            int rc = epoll_ctl(epollfd_, EPOLL_CTL_MOD, pe->fd, &pe->ev);
            ENFORCE(rc != -1)(rc)(cxx::sys::err::get());
        }

        int epoll::do_task(int timeout)
        {
            static const int MAX_IO_EVENTS = 256;
            epoll_event evbuf[MAX_IO_EVENTS];

            int n = epoll_wait(epollfd_, &evbuf[0], MAX_IO_EVENTS, timeout);
            for(int i = 0; i < n; ++i) {
                epoll_entry* pe = ((epoll_entry* )evbuf[i].data.ptr);
                if(pe->fd == -1)
                    continue;
                if(evbuf[i].events & (EPOLLERR | EPOLLHUP))
                    pe->cb->on_readable(pe->fd);
                // in case of user call 'del_fd' in on_readable()
                if(pe->fd == -1)
                    continue;
                if(evbuf[i].events & EPOLLIN)
                    pe->cb->on_readable(pe->fd);
                // in case of user call 'del_fd' in on_readable()
                if(pe->fd == -1)
                    continue;
                if(evbuf[i].events & EPOLLOUT)
                    pe->cb->on_writable(pe->fd);
            }

            {
                cxx::sys::plainmutex::scopelock lock(mutex_);
                for(size_t i = 0; i < retired_.size(); ++i) {
                    delete retired_[i];
                }
                retired_.clear();
            }
            return n;
        }

        void epoll::destroy()
        {
            delete this;
        }

    }
}

#endif
