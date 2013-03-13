/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/con/coroutine.h"
//#include "context.h"
#include "fcontext.h"
#include "command.h"

namespace cxx {
    namespace con {

        cxx::sys::atomic_t scheduler::idgen(1);

        ////////////////////////////////////////////////////////////////////////
        coroutine_error::coroutine_error(int code, const std::string &msg)
            : std::runtime_error(msg), code_(code)
        {
        }

        int coroutine_error::code() const
        {
            return code_;
        }

        ////////////////////////////////////////////////////////////////////////
        scheduler::scheduler()
        {
            ctxt_   = new context();
            queue_  = new scheduler::reactor(this);
        }

        scheduler::~scheduler()
        {
            quit();
            delete ctxt_;
            delete queue_;
        }

        void scheduler::spawn(taskptr func, void *arg, int stack_size)
        {
            size_t length = (stack_size + stack::page_size() - 1 + sizeof(coroutine) + sizeof(context)) /
                            stack::page_size() * stack::page_size();
            char*  buffer = new char[length];
            coroutine* c  = new (buffer) coroutine(this, func, arg, buffer + sizeof(coroutine), length - sizeof(coroutine));

            c->id_ = (int)++idgen;
            command   m;
            m.type = command::spawn;
            m.args.spawn.task = c;
            queue_->send(m);
        }

        void scheduler::start()
        {
            schedule();
        }

        void scheduler::schedule()
        {
            bool    running = true;
            int     timeout = 0;
            cxx::datetime now;
            while(running) {
                command   m;
                timeout = calc_expire();
                if(queue_->recv(&m, timeout)) {
                    switch(m.type) {
                    case command::spawn:
                        ready_.insert(m.args.spawn.task);
                        do_schedule(m.args.spawn.task);
                        break;
                    case command::resume:
                        do_schedule(m.args.resume.task);
                        break;
                    case command::sleep:
                        now = cxx::datetime::now();
                        block_.insert(std::make_pair
                                      (now + cxx::datetimespan(m.args.sleep.when),
                                       m.args.sleep.task));
                        ready_.erase(m.args.sleep.task);
                        break;
                    case command::quit:
                        running = false;
                        break;
                    }
                }

                if(ready_.empty() && block_.empty() && queue_->size() == 0) {
                    running = false;
                    break;
                }
            }
            for(ready_t::iterator it = ready_.begin(); it != ready_.end(); ++it) {
                (*it)->stop();
            }
        }

        void scheduler::sleep(coroutine *c, int ms)
        {
            command   m;
            m.type = command::sleep;
            m.args.sleep.task = c;
            m.args.sleep.when = ms;
            queue_->send(m);
        }

        void scheduler::resume(coroutine *c)
        {
            command   m;
            m.type = command::resume;
            m.args.resume.task = c;
            queue_->send(m);
        }

        void scheduler::await(coroutine *c, net::fd_t f, net::poller::readable r)
        {
            queue_->wait(c, f, r);
            c->shift();
        }

        void scheduler::await(coroutine *c, net::fd_t f, net::poller::writable w)
        {
            queue_->wait(c, f, w);
            c->shift();
        }

        void scheduler::close(net::fd_t f)
        {
            queue_->drop(f);
        }

        scheduler::context* scheduler::ctxt()
        {
            return ctxt_;
        }

        void scheduler::quit()
        {
            command   m;
            m.type = command::quit;
            queue_->send(m);
        }

        void scheduler::shift(scheduler::context *f, scheduler::context *t)
        {
            // switch to the scheduler
//            if(swapcontext(&f->uc, &t->uc) < 0) {
//                int err = cxx::sys::err::get();
//                fprintf(stderr, "swapcontext failed: %d - %s\n", err, cxx::sys::err::str(err).c_str());
//                assert(0);
//            }
            jump_fcontext(f, t, 0);
        }

        void scheduler::do_schedule(coroutine *c)
        {
            jump_fcontext(ctxt_, c->ctxt(), (intptr_t)c);
            if(c->isdead()) {
                ready_.erase(c);
                queue_->drop(c);
                block_t::iterator it = block_.begin();
                while(it != block_.end()) {
                    if(it->second == c) {
                        block_.erase(it++);
                    }
                    else {
                        it++;
                    }
                }
                char* p = (char* )c;
                c->~coroutine();
                delete[] p;
            }
        }

        int scheduler::calc_expire()
        {
            if(block_.empty()) {
                return 500;
            }

            cxx::datetime current = cxx::datetime::now();
            block_t::iterator it = block_.begin();
            while(it != block_.end()) {
                // multimap is sorted, so we can stop checking the subsequent
                // and return the time to wait for the next on
                if(it->first > current) {
                    return (it->first - current).getTotalMilliSeconds();
                }
                else {
                    resume(it->second);
                    block_.erase(it++);
                }
            }

            return 500;
        }

        ////////////////////////////////////////////////////////////////////////
        scheduler_group::scheduler_group(size_t count)
            : size_(count)
        {
            for(size_t i = 0; i < size_; ++i) {
                scheduler* s = new scheduler();
                sche_.push_back(s);
            }
        }

        scheduler_group::~scheduler_group()
        {
            stop();
            for(size_t i = 0; i < size_; ++i) {
                delete sche_[i];
            }
        }

        void scheduler_group::start()
        {
            for(size_t i = 0; i < size_; ++i) {
                scheduler* s = sche_[i];
                cxx::sys::thread t = cxx::sys::threadcontrol::create(cxx::MakeDelegate(s, &scheduler::start), "scheduler");
                thrd_.push_back(t);
            }
            for(size_t i = 0; i < size_; ++i) {
                thrd_[i].join();
            }
        }

        void scheduler_group::stop()
        {
            for(size_t i = 0; i < size_; ++i) {
                sche_[i]->quit();
            }
        }

        size_t scheduler_group::size() const
        {
            return size_;
        }

        scheduler_group::sched_t& scheduler_group::operator [](size_t i)
        {
            return sche_[i];
        }

        const scheduler_group::sched_t& scheduler_group::operator [](size_t i) const
        {
            return sche_[i];
        }

    }
}
