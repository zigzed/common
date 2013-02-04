/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/net/helper.h"
#ifndef OS_WINDOWS
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
#endif
#include <cassert>

namespace cxx {
    namespace net {

        namespace ip {

            void unblocking(fd_t s)
            {
#if     defined(OS_WINDOWS)
                unsigned long nonblock = 1;
                int rc = ioctlsocket(s, FIONBIO, &nonblock);
                assert(rc != -1);
#else
                int flags = fcntl(s, F_GETFL, 0);
                if(flags == -1)
                    flags = 0;
                int rc = fcntl(s, F_SETFL, flags | O_NONBLOCK);
                assert(rc != -1);
#endif
            }

            fd_t opensocket(int domain, int type, int protocol)
            {
#if defined(SOCK_CLOEXEC)
                type |= SOCK_CLOEXEC;
#endif
                fd_t s = ::socket(domain, type, protocol);
                if(s == -1)
                    return -1;

#if !defined(SOCK_CLOEXEC) && defined(FD_CLOEXEC)
                int rc = fcntl(s, F_SETFD, FD_CLOEXEC);
                assert(rc != -1);
#endif

#if defined(OS_WINDOWS)
                BOOL ret = SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);
                assert(ret == TRUE);
#endif
                return s;
            }

        }

        namespace tcp {

            void no_delays(fd_t s)
            {
                int nodelay = 1;
                int rc = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char* )&nodelay, sizeof(nodelay));
                assert(rc != -1);
            }

            void no_delack(fd_t s)
            {
#if defined(TCP_NODELACK)
                int nodelack = 1;
                int rc = setsockopt(s, IPPROTO_TCP, TCP_NODELACK, (char* )&nodelack, sizeof(nodelack));
                assert(rc != -1);
#endif
            }

        }

    }
}
