/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_OPTIONS_H
#define CXX_NET_OPTIONS_H
#include "common/config.h"
#include "common/net/tcp_address.h"
#include <vector>

namespace cxx {
    namespace net {

        struct options {
            typedef std::vector<tcp_address_mask >  mask_t;

            /** send buffer size */
            int     sndbuf;
            /** recv buffer size */
            int     rcvbuf;
            /** interval between reconnecting, in milliseconds */
            int     conivl;
            /** backlog for listen function */
            int     backlog;
            /** decoder buffer size */
            int     decbuf;
            /** for accept() filter, if filter is setted, we'll accept the
             * connection matched the masks
             */
            mask_t  filter;

            options() : sndbuf(0), rcvbuf(0), conivl(1000), backlog(10) {}
        };
    }
}

#endif
