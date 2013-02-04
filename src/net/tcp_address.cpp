#include "common/net/tcp_address.h"
#include <string.h>
#include <stdlib.h>
#include <cassert>
#include <sstream>
#if !defined(OS_WINDOWS)
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
    #include <netdb.h>
#endif

namespace cxx {
    namespace net {

#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG   0
#endif

        tcp_address::tcp_address()
        {
            memset(&addr_, 0, sizeof(addr_));
        }

        tcp_address::~tcp_address()
        {
        }

        tcp_address::tcp_address(const sockaddr *sa, socklen_t len)
        {
            memset(&addr_, 0, sizeof(addr_));
            if(sa->sa_family == AF_INET && len >= sizeof(addr_.ipv4)) {
                memcpy(&addr_.ipv4, sa, sizeof(addr_.ipv4));
            }
            else if(sa->sa_family == AF_INET6 && len >= sizeof(addr_.ipv6)) {
                memcpy(&addr_.ipv6, sa, sizeof(addr_.ipv6));
            }
        }

#if     defined(OS_LINUX)
#include <ifaddrs.h>

        bool tcp_address::resolve_nic_name(const char *nic, bool ipv4)
        {
            ifaddrs* ifa = NULL;
            int rc = getifaddrs(&ifa);
            assert(rc == 0);
            assert(ifa);
            if(rc != 0) {
                return false;
            }

            bool found = false;
            for(ifaddrs* ifp = ifa; ifp != NULL; ifp = ifp->ifa_next) {
                if(ifp->ifa_addr == NULL)
                    continue;

                int family = ifp->ifa_addr->sa_family;
                if((family == AF_INET || (!ipv4 && family == AF_INET6)) &&
                        !strcmp(nic, ifp->ifa_name))
                {
                    memcpy(&addr_, ifp->ifa_addr,
                           (family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
                    found = true;
                    break;
                }
            }

            freeifaddrs(ifa);

            return found;
        }
#elif   defined(OS_WINDOWS)
        bool tcp_address::resolve_nic_name(const char *nic, bool ipv4)
        {
            return false;
        }
#endif

        bool tcp_address::resolve_interface(const char *itf, bool ipv4)
        {
            sockaddr_storage    ss;
            sockaddr* out_addr = (sockaddr* )&ss;
            size_t    out_size;

            if(ipv4) {
                sockaddr_in ipv4_addr;
                memset(&ipv4_addr, 0, sizeof(ipv4_addr));
                ipv4_addr.sin_family        = AF_INET;
                ipv4_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
                out_size = sizeof(ipv4_addr);
                memcpy(out_addr, &ipv4_addr, out_size);
            }
            else {
                sockaddr_in6 ipv6_addr;
                memset(&ipv6_addr, 0, sizeof(ipv6_addr));
                ipv6_addr.sin6_family       = AF_INET6;
                memcpy(&ipv6_addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
                out_size = sizeof(ipv6_addr);
                memcpy(out_addr, &ipv6_addr, out_size);
            }

            if(strcmp(itf, "*") == 0) {
                assert(out_size <= sizeof(addr_));
                memcpy(&addr_, out_addr, out_size);
                return true;
            }

            // assume the string is NIC name
            bool rc = resolve_nic_name(itf, ipv4);
            if(rc)
                return true;

            // not the NIC name, assume it's the literal address
            addrinfo* res = NULL;
            addrinfo  req;
            memset(&req, 0, sizeof(req));
            req.ai_family   = ipv4 ? AF_INET : AF_INET6;
            req.ai_socktype = SOCK_STREAM;
            req.ai_flags    = AI_PASSIVE | AI_NUMERICHOST;
            if(req.ai_family == AF_INET6)
                req.ai_flags |= AI_V4MAPPED;

            int ret = getaddrinfo(itf, NULL, &req, &res);
            if(ret) {
                return false;
            }

            assert(res != NULL);
            assert(res->ai_addrlen <= sizeof(addr_));
            memcpy(&addr_, res->ai_addr, res->ai_addrlen);
            freeaddrinfo(res);

            return true;
        }

        bool tcp_address::resolve_hostname(const char *hst, bool ipv4)
        {
            addrinfo    req;
            memset(&req, 0, sizeof(req));
            req.ai_family   = ipv4 ? AF_INET : AF_INET6;
            req.ai_socktype = SOCK_STREAM;
            if(req.ai_family == AF_INET6)
                req.ai_flags |= AI_V4MAPPED;

            addrinfo*   res = NULL;
            int rc = getaddrinfo(hst, NULL, &req, &res);
            if(rc) {
                return false;
            }

            assert(res->ai_addrlen <= sizeof(addr_));
            memcpy(&addr_, res->ai_addr, res->ai_addrlen);
            freeaddrinfo(res);
            return true;
        }

        bool tcp_address::resolve(const char *name, bool local, bool ipv4)
        {
            const char* delimiter = strrchr(name, ':');
            if(!delimiter) {
                return false;
            }
            std::string addr_str(name, delimiter - name);
            std::string port_str(delimiter + 1);
            if(addr_str.size() >= 2 && addr_str[0] == '[' &&
                    addr_str[addr_str.size() - 1] == ']')
                addr_str = addr_str.substr(1, addr_str.size() - 2);

            unsigned short port;
            if(port_str == "*" || port_str == "0")
                port = 0;
            else {
                port = atoi(port_str.c_str());
                if(port == 0)
                    return false;
            }

            bool rc;
            if(local) {
                rc = resolve_interface(addr_str.c_str(), ipv4);
            }
            else {
                rc = resolve_hostname(addr_str.c_str(), ipv4);
            }
            if(!rc) {
                return false;
            }

            if(addr_.generic.sa_family == AF_INET6)
                addr_.ipv6.sin6_port= htons(port);
            else
                addr_.ipv4.sin_port = htons(port);

            return true;
        }

        std::string tcp_address::string() const
        {
            if(addr_.generic.sa_family != AF_INET && addr_.generic.sa_family != AF_INET6)
                return std::string();

            char hbuf[NI_MAXHOST];
            int rc = getnameinfo(address(), addrlen(), hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);
            if(rc != 0)
                return std::string();

            if(addr_.generic.sa_family == AF_INET6) {
                std::stringstream s;
                s << "tcp://[" << hbuf << "]:" << ntohs(addr_.ipv6.sin6_port);
                return s.str();
            }
            else {
                std::stringstream s;
                s << "tcp://" << hbuf << ":" << ntohs(addr_.ipv4.sin_port);
                return s.str();
            }

            return std::string();
        }

        const sockaddr* tcp_address::address() const
        {
            return &addr_.generic;
        }

        socklen_t tcp_address::addrlen() const
        {
            if(addr_.generic.sa_family == AF_INET6)
                return (socklen_t)sizeof(addr_.ipv6);
            else
                return (socklen_t)sizeof(addr_.ipv4);
        }

        sa_family_t tcp_address::family() const
        {
            return addr_.generic.sa_family;
        }

        //----------------------------------------------------------------------
        tcp_address_mask::tcp_address_mask()
            : mask_(-1)
        {
        }

        int tcp_address_mask::getmask() const
        {
            return mask_;
        }

        bool tcp_address_mask::resolve(const char *name, bool ipv4)
        {
            std::string addr_str, mask_str;
            const char* delimiter = strrchr(name, '/');
            if(delimiter != NULL) {
                addr_str.assign(name, delimiter - name);
                mask_str.assign(delimiter + 1);
                if(mask_str.empty()) {
                    return false;
                }
            }
            else {
                addr_str.assign(name);
            }

            if(!tcp_address::resolve_hostname(addr_str.c_str(), ipv4)) {
                return false;
            }
            if(mask_str.empty()) {
                if(addr_.generic.sa_family == AF_INET6)
                    mask_ = 128;
                else
                    mask_ = 32;
            }
            else if (mask_str == "0") {
                    mask_ = 0;
            }
            else {
                int mask = atoi(mask_str.c_str());
                if(mask < 1 || (addr_.generic.sa_family == AF_INET6 && mask > 128)
                        || (addr_.generic.sa_family == AF_INET && mask > 32)) {
                    return false;
                }
                mask_ = mask;
            }
            return true;
        }

        std::string tcp_address_mask::string() const
        {
            if(addr_.generic.sa_family != AF_INET && addr_.generic.sa_family != AF_INET6)
                return std::string();

            if(mask_ == -1)
                return std::string();

            char hbuf[NI_MAXHOST];
            int rc = getnameinfo(address(), addrlen(), hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);
            if(rc != 0) {
                return std::string();
            }

            if(addr_.generic.sa_family == AF_INET6) {
                std::stringstream s;
                s << "[" << hbuf << "]/" << mask_;
                return s.str();
            }
            else {
                std::stringstream s;
                s << hbuf << "/" << mask_;
                return s.str();
            }
            return std::string();
        }

        bool tcp_address_mask::matched(const sockaddr *ss, socklen_t len) const
        {
            assert(mask_ != -1 && ss != NULL && len >= sizeof(sockaddr));
            if(ss->sa_family != addr_.generic.sa_family)
                return false;

            if(mask_ > 0) {
                int mask;
                unsigned char* lhs, *rhs;
                if(ss->sa_family == AF_INET6) {
                    assert(len == sizeof(sockaddr_in6));
                    rhs = (unsigned char* )&(((sockaddr_in6* )ss)->sin6_addr);
                    lhs = (unsigned char* )&addr_.ipv6.sin6_addr;
                    mask = sizeof(in6_addr) * 8;
                }
                else {
                    assert(len == sizeof(sockaddr_in));
                    rhs = (unsigned char* )&(((sockaddr_in* )ss)->sin_addr);
                    lhs = (unsigned char* )&addr_.ipv4.sin_addr;
                    mask = sizeof(in_addr) * 8;
                }

                if(mask_ < mask)
                    mask = mask_;

                int bytes = mask / 8;
                if(memcmp(rhs, lhs, bytes) != 0)
                    return false;

                unsigned char last_byte_bits = (0xFFU << (8 - (mask % 8))) & 0xFFU;
                if(last_byte_bits) {
                    if((rhs[bytes] & last_byte_bits) != (lhs[bytes] & last_byte_bits))
                        return false;
                }
            }
            return true;
        }

    }
}
