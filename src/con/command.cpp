/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "command.h"

namespace cxx {
    namespace con {

        struct scheduler::reactor::receiver : public cxx::net::poller_event {

            explicit receiver(scheduler* s, sys::event& e,
                              scheduler::reactor::wait_t* w,
                              scheduler::reactor::time_t* t)
                : s_(s), e_(e), r_(false), w_(w), t_(t) {}

            void on_readable(cxx::net::fd_t fd) {
                if(e_.handle() == fd) {
                    r_ = true;
                    return;
                }
                scheduler::reactor::wait_t::iterator it = w_->find(fd);
                assert(it != w_->end());
                if(it == w_->end())
                    return;
                coroutine* c = it->second.c;
                //w_->erase(it);
                s_->resume(c);
            }
            void on_writable(cxx::net::fd_t fd) {
                scheduler::reactor::wait_t::iterator it = w_->find(fd);
                assert(it != w_->end());
                if(it == w_->end())
                    return;
                coroutine* c = it->second.c;
                //w_->erase(it);
                s_->resume(c);
            }
            void on_expire(int id) {
                scheduler::reactor::time_t::iterator it = t_->find(id);
                assert(it != t_->end());
                if(it == t_->end())
                    return;
                coroutine* c = it->second;
                t_->erase(it);
                s_->resume(c);
            }

            scheduler*                  s_;
            sys::event&                 e_;
            bool                        r_;
            cxx::net::poller::handle_t  h_;
            scheduler::reactor::wait_t* w_;
            scheduler::reactor::time_t* t_;
        };

        scheduler::reactor::reactor(scheduler* s)
            : wsize_(0), rsize_(0), active_(false), poller_(NULL),
              future_(NULL)
        {
            bool ok = queue_.read(NULL);
            ENFORCE(!ok);
            poller_ = cxx::net::poller::create();
            future_  = new scheduler::reactor::receiver(s, event_, &waiter_, &expire_);
            future_->h_ = poller_->add_fd(event_.handle(), future_);
        }

        scheduler::reactor::~reactor()
        {
            poller_->stop();
            poller_->del_fd(future_->h_);
            for(hmap_t::iterator it = handle_.begin(); it != handle_.end(); ++it) {
                poller_->del_fd(it->second);
            }
            poller_->destroy();
            delete future_;
        }

        void scheduler::reactor::send(const scheduler::command& m)
        {
            bool ok = false;
            {
                lock_t::scopelock   lock(mutex_);
                queue_.write(m);
                ok = queue_.flush();
                ++wsize_;
            }
            if(!ok)
                event_.send();
        }

        bool scheduler::reactor::recv(scheduler::command* m, int ms)
        {
            if(active_) {
                bool ok = queue_.read(m);
                if(ok) {
                    ++rsize_;
                    return true;
                }
                active_ = false;
                event_.recv();
            }

            // 为了简化接收消息的API（避免使用回调的方式），在这里用异步的API模拟阻塞的API
            // 此处理方法不会丧失异步的好处和性能
            future_->r_ = false;
            poller_->add_fd(future_->h_, cxx::net::poller::readable());
            int n = poller_->poll_once(ms);

            if(future_->r_) {
                assert(n > 0);
                active_ = true;
                bool ok = queue_.read(m);
                assert(ok);
                if(ok) {
                    ++rsize_;
                }
            }

            return future_->r_;
        }

        size_t scheduler::reactor::size() const
        {
            assert(wsize_ >= rsize_);
            return wsize_ - rsize_;
        }

        void scheduler::reactor::wait(coroutine *c, net::fd_t f, net::poller::readable r)
        {
            cxx::net::poller::handle_t h;
            hmap_t::iterator it = handle_.find(f);
            if(it != handle_.end()) {
                h = it->second;
            }
            else {
                h = poller_->add_fd(f, future_);
                waiter_.insert(std::make_pair(f, ch(c, h)));
                handle_.insert(std::make_pair(f, h));
            }
            poller_->add_fd(h, r);
        }

        void scheduler::reactor::wait(coroutine *c, net::fd_t f, net::poller::writable w)
        {
            cxx::net::poller::handle_t h;
            hmap_t::iterator it = handle_.find(f);
            if(it != handle_.end()) {
                h = it->second;
            }
            else {
                h = poller_->add_fd(f, future_);
                waiter_.insert(std::make_pair(f, ch(c, h)));
                handle_.insert(std::make_pair(f, h));
            }
            poller_->add_fd(h, w);
        }

        void scheduler::reactor::time(coroutine *c, int i)
        {
            expire_.insert(std::make_pair(i, c));
        }

    }
}
