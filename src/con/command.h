/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_COMMAND_H
#define CXX_CON_COMMAND_H
#include "common/con/coroutine.h"
#include "common/sys/event.h"
#include "common/sys/mutex.h"
#include "common/alg/pipe.h"
#include "common/net/poller.h"


namespace cxx {
    namespace con {

        struct scheduler::command {
            enum {
                spawn, resume, sleep, quit
            } type;
            union {
                struct {
                    coroutine*      task;
                } spawn;
                struct {
                    coroutine*      task;
                } resume;
                struct {
                    coroutine*      task;
                    int             when;
                } sleep;
                struct {
                } quit;
            } args;
        };

        struct scheduler::reactor {
            struct ch {
                coroutine*                  c;
                cxx::net::poller::handle_t  h;
                ch(coroutine* a, cxx::net::poller::handle_t b) : c(a), h(b) {}
            };

            typedef cxx::alg::pipe<scheduler::command, 256 >    pipe_t;
            typedef cxx::sys::plainmutex                        lock_t;
            typedef cxx::net::poller                            poll_t;
            typedef cxx::net::poller::handle_t                  handle_t;
            typedef std::map<cxx::net::fd_t, handle_t >         hmap_t;
            typedef std::map<cxx::net::fd_t, ch >               wait_t;

            struct receiver;

            lock_t      mutex_;
            sys::event  event_;
            pipe_t      queue_;
            size_t      wsize_;
            size_t      rsize_;
            bool        active_;
            poll_t*     poller_;
            receiver*   future_;
            wait_t      waiter_;
            hmap_t      handle_;

            explicit reactor(scheduler* s);
            ~reactor();
            void    send(const scheduler::command& c);
            bool    recv(scheduler::command* c, int ms);
            size_t  size() const;
            void    wait(coroutine* c, cxx::net::fd_t f, cxx::net::poller::readable r);
            void    wait(coroutine* c, cxx::net::fd_t f, cxx::net::poller::writable w);
            void    drop(cxx::net::fd_t f);
            void    drop(coroutine* c);
        };

    }
}

#endif
