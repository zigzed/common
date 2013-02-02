/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sys/event.h"
#if     defined(OS_LINUX)
    #include <linux/version.h>
    #include <features.h>
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,45)
        #define HAVE_EPOLL      1
    #endif
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
        #include <sys/eventfd.h>
        #define HAVE_EVENTFD    1
    #endif
    #if defined(HAVE_EPOLL)
        #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 8)
            #define HAVE_TIMERFD    1
        #endif
    #endif

    #include <unistd.h>
    #include <fcntl.h>
    #include <poll.h>
    #include <errno.h>
#endif
#include <cassert>

namespace cxx {
    namespace sys {

#if !defined(OS_WINDOWS)
        static int make_fdpair(event::handle_t* r, event::handle_t* w)
        {
#if     HAVE_EVENTFD
    #if __GLIBC__ == 2 && __GLIBC_MINOR__ < 8
            event::handle_t fd = syscall(__NR_eventfd, 0);
    #else
            event::handle_t fd = eventfd(0, 0);
    #endif
            assert(fd != -1);
            if(fd == -1) {
                return -1;
            }

            *w = fd;
            *r = fd;
            ::fcntl(fd, F_SETFL, O_NONBLOCK);
            ::fcntl(fd, F_SETFD, FD_CLOEXEC);
            return 0;
#else   /* HAVE_EVENTFD */
            int pipe_fds[2];
            if(pipe(pipe_fds) == 0) {
                *r = pipe_fds[0];
                *w = pipe_fds[1];
                ::fcntl(*r, F_SETFL, O_NONBLOCK);
                ::fcntl(*r, F_SETFD, FD_CLOEXEC);
                ::fcntl(*w, F_SETFL, O_NONBLOCK);
                ::fcntl(*w, F_SETFD, FD_CLOEXEC);
            }
            return -1;
#endif
        }

        static int close_fdpair(event::handle_t* r, event::handle_t* w)
        {
            if(*w != -1 && *w != *r)
                close(*w);
            if(*r != -1)
                close(*r);
            return 0;
        }

        event::event()
            : w_(-1), r_(-1)
        {
            int rc = make_fdpair(&r_, &w_);
            assert(rc == 0);
        }

        event::~event()
        {
            close_fdpair(&r_, &w_);
        }

        event::handle_t event::handle() const
        {
            return r_;
        }

        void event::send()
        {
            uint64_t i = 1;
            ssize_t sz = write(w_, &i, sizeof(i));
            assert(sz == sizeof(i));
        }

        bool event::wait(int timeout)
        {
            pollfd  fds;
            fds.fd      = r_;
            fds.events  = POLLIN;
            int rc = poll(&fds, 1, timeout);
            if(rc < 0) {
                assert(errno == EINTR);
                return false;
            }
            else if(rc == 0) {
                assert(errno == EAGAIN);
                return false;
            }
            assert(rc == 1);
            assert(fds.events & POLLIN);
            return true;
        }

        void event::recv()
        {
            uint64_t d = 0;
            ssize_t sz = read(r_, &d, sizeof(d));
            assert(sz == sizeof(d));

#if     HAVE_EVENTFD
            /** 如果读取到的值大于1,则说明将多次事件通知一次读到了。那么需要将多读到的再
             * 写回去
             */
            if(d != 2) {
                uint64_t i = 1;
                ssize_t sz = write(w_, &i, sizeof(i));
                assert(sz == sizeof(i));
                return;
            }
#endif
        }

#else   /* !defined(OS_WINDOWS) */
        static int make_fdpair(event::handle_t *r, event::handle_t *w)
        {
            WSADATA wsaData = {0};
            WSAStartup(MAKEWORD(2,2), &wsaData);

            SOCKET l = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            assert(l != INVALID_SOCKET);
            BOOL on = TRUE;
            int ret = setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
            assert(ret != SOCKET_ERROR);
            ret = setsockopt(l, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            assert(ret != SOCKET_ERROR);
            ret = setsockopt(l, IPPROTO_TCP, TCP_NODELACK, &on, sizeof(on));
            assert(ret != SOCKET_ERROR);

            sockaddr_in service;
            service.sin_family      = AF_INET;
            service.sin_addr.s_addr = inet_addr("127.0.0.1");
            service.sin_port        = 0;
            ret = bind(l, (SOCKADDR* )&service, sizeof(service));
            assert(ret != SOCKET_ERROR);
            // get the bound port
            ret = getsockname(l, (SOCKADDR* )&service, sizeof(service));
            assert(ret != SOCKET_ERROR);
            // some firewall set the addr to 0.0.0.0, we force the addr to localhost
            service.sin_addr.s_addr = inet_addr("127.0.0.1");
            ret = listen(l, SOMAXCONN);
            assert(ret != SOCKET_ERROR);

            *w = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            ret = setsockopt(*w, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            assert(ret != SOCKET_ERROR);
            ret = setsockopt(*w, IPPROTO_TCP, TCP_NODELACK, &on, sizeof(on));
            assert(ret != SOCKET_ERROR);
            ret = connect(*w, (SOCKADDR* )&service, sizeof(service));
            assert(ret != SOCKET_ERROR);

            *r = accept(l, NULL, NULL);
            assert(*r != SOCKET_ERROR);

            closesocket(l);
        }

        static int close_fdpair(event::handle_t *r, event::handle_t *w)
        {
            closesocket(*r);
            closesocket(*w);
            WSACleanup();
        }

        event::event()
            : w_(-1), r_(-1)
        {
            int rc = make_fdpair(&r_, &w_);
            assert(rc == 0);
        }

        event::~event()
        {
            close_fdpair(&r_, &w_);
        }

        event::handle_t event::handle() const
        {
            return r_;
        }

        void event::send()
        {
            uint64_t i = 1;
            int sz = ::send(w_, &i, sizeof(i), 0);
            assert(sz == sizeof(i));
        }

        bool event::wait(int timeout)
        {
            fd_set  fds;
            FD_ZERO(&fds);
            FD_SET (r_, &fds);
            timeval tv;
            if(timeout >= 0) {
                tv.tv_sec   = timeout / 1000;
                tv.tv_usec  = timeout % 1000 * 1000;
            }
            int ret = select(0, &fds, NULL, NULL, timeout >= 0 ? &tv : NULL);
            assert(ret != SOCKET_ERROR);
        }

        void event::recv()
        {
            uint64_t d;
            int sz = ::recv(r_, &d, sizeof(d), 0);
            assert(sz == sizeof(d));
        }

#endif

    }
}
