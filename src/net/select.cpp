/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "select.h"
#include <cassert>
#include <algorithm>

namespace cxx {
    namespace net {

        select::select()
            : max_fd_(-1), retire_(false)
        {
            FD_ZERO(&src_rd_);
            FD_ZERO(&src_wr_);
            FD_ZERO(&src_er_);
        }

        select::~select()
        {
        }

        poller::handle_t select::add_fd(fd_t fd, poller_event *sink)
        {
            select_entry se = { fd, sink };
            fd_set_.push_back(se);
            assert(fd_set_.size() <= FD_SETSIZE);

            FD_SET(fd, &src_er_);
            if(fd > max_fd_)
                max_fd_ = fd;

            adj_loads(1);
            return (void* )(long)fd;
        }

        void select::del_fd(handle_t handle)
        {
            fd_t fd = (fd_t)(long)handle;
            for(fdset_t::iterator it = fd_set_.begin(); it != fd_set_.end(); ++it) {
                if(it->fd == fd) {
                    it->fd = -1;
                    FD_CLR(fd, &src_er_);
                    FD_CLR(fd, &src_wr_);
                    FD_CLR(fd, &src_rd_);

                    FD_CLR(fd, &cur_er_);
                    FD_CLR(fd, &cur_wr_);
                    FD_CLR(fd, &cur_rd_);

                    if(max_fd_ == fd) {
                        max_fd_ = -1;
                        for(size_t i = 0; i < fd_set_.size(); ++i) {
                            if(fd_set_[i].fd > max_fd_) {
                                max_fd_ = fd_set_[i].fd;
                            }
                        }
                    }

                    retire_ = true;
                    adj_loads(-1);
                    break;
                }
            }
        }

        void select::add_fd(handle_t handle, readable r)
        {
            fd_t fd = (fd_t)(long)handle;
            FD_SET(fd, &src_rd_);
        }

        void select::add_fd(handle_t handle, writable w)
        {
            fd_t fd = (fd_t)(long)handle;
            FD_SET(fd, &src_wr_);
        }

        void select::del_fd(handle_t handle, readable r)
        {
            fd_t fd = (fd_t)(long)handle;
            FD_CLR(fd, &src_rd_);
        }

        void select::del_fd(handle_t handle, writable w)
        {
            fd_t fd = (fd_t)(long)handle;
            FD_CLR(fd, &src_wr_);
        }

        bool select::is_removed(const select::select_entry& se)
        {
            return se.fd == -1;
        }

        int select::do_task(int timeout)
        {
            memcpy(&cur_er_, &src_er_, sizeof(src_er_));
            memcpy(&cur_wr_, &src_wr_, sizeof(src_wr_));
            memcpy(&cur_rd_, &src_rd_, sizeof(src_rd_));

            timeval tv = { (timeout / 1000), (timeout % 1000 * 1000) };
            int rc = ::select(max_fd_ + 1, &cur_rd_, &cur_wr_, &cur_er_, &tv);
            assert(rc != -1);

            int iter = 0;
            int done = 0;
            while(iter < fd_set_.size() && done < rc) {
                if(fd_set_[iter].fd == -1)
                    continue;
                if(FD_ISSET(fd_set_[iter].fd, &cur_er_)) {
                    ++done;
                    fd_set_[iter].cb->on_failed(fd_set_[iter].fd);
                }
                if(fd_set_[iter].fd == -1)
                    continue;
                if(FD_ISSET(fd_set_[iter].fd, &cur_wr_)) {
                    ++done;
                    fd_set_[iter].cb->on_writable(fd_set_[iter].fd);
                }
                if(fd_set_[iter].fd == -1)
                    continue;
                if(FD_ISSET(fd_set_[iter].fd, &cur_rd_)) {
                    ++done;
                    fd_set_[iter].cb->on_readable(fd_set_[iter].fd);
                }
            }

            if(retire_) {
                fd_set_.erase(std::remove_if(fd_set_.begin(), fd_set_.end(), is_removed),
                              fd_set_.end());
                retire_ = false;
            }
            return rc;
        }

        void select::destroy()
        {
            delete this;
        }

    }
}
