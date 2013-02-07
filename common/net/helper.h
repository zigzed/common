/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_HELPER_H
#define CXX_NET_HELPER_H
#include "common/config.h"
#include "common/net/poller.h"

namespace cxx {
    namespace net {

        namespace ip {
            void    unblocking(fd_t s);
            fd_t    opensocket(int domain, int type, int protocol);
            void    closesocket(fd_t& s);
        }

        namespace tcp {
            void    no_delays(fd_t s);
            void    no_delack(fd_t s);
            void    setsndbuf(fd_t s, int size);
            void    setrcvbuf(fd_t s, int size);
            void    reuseaddr(fd_t s);
        }

    }
}

#endif
