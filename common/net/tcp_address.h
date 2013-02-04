/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_TCPADDRESS_H
#define CXX_NET_TCPADDRESS_H
#include <string>
#include "common/config.h"

#if     defined(OS_LINUX)
    #include <netinet/in.h>
    #include <sys/socket.h>
#elif   defined(OS_WINDOWS)
    #include <winsock2.h>
    typedef unsinged short sa_family_t;
#endif

namespace cxx {
    namespace net {

        class tcp_address {
        public:
            tcp_address();
            tcp_address(const sockaddr* sa, socklen_t len);
            virtual ~tcp_address();

            bool                resolve(const char* name, bool local, bool ipv4);
            virtual std::string string() const;
            sa_family_t         family() const;
            const sockaddr*     address() const;
            socklen_t           addrlen() const;
        private:
            bool    resolve_nic_name (const char* nic, bool ipv4);
            bool    resolve_interface(const char* itf, bool ipv4);
            bool    resolve_hostname (const char* hst, bool ipv4);

            union {
                sockaddr     generic;
                sockaddr_in  ipv4;
                sockaddr_in6 ipv6;
            } addr_;
        };

        class tcp_address_mask : public tcp_address {
        public:
            tcp_address_mask();

            bool        resolve(const char *name, bool ipv4);
            std::string string() const;
            int         getmask() const;
            bool        matched(const sockaddr* ss, socklen_t len) const;
        private:
            int     mask_;
        };

    }
}

#endif
