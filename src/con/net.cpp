/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/con/net.h"
#include "common/net/helper.h"
#include <stdlib.h>
#include <sstream>

#if     defined(OS_LINUX)
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
#elif   defined(OS_WINDOWS)
    #include <winsock2.h>
    typedef unsinged short sa_family_t;
#endif


namespace cxx {
    namespace con {

        static bool netlookup(const char* name, unsigned int* ip);
        static bool parseip(const char* name, unsigned int* ip);

        network_error::network_error(int code, const std::string &msg)
            : std::runtime_error(msg), code_(code)
        {
        }

        int network_error::code() const
        {
            return code_;
        }


        acceptor::acceptor(coroutine* c, bool istcp, const char *server, unsigned short port)
            : istcp_(istcp), socks_(-1), task_(c)
        {
            int         proto = istcp_ ? SOCK_STREAM : SOCK_DGRAM;
            sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin_family   = AF_INET;
            if(server != NULL && strcmp(server, "*") != 0) {
                unsigned int ip;
                if(!netlookup(server, &ip)) {
                    int r = sys::err::get();
                    std::stringstream msg;
                    msg << "network lookup \'" << server << ":" << port << "' failed";
                    throw network_error(r, msg.str());
                }
                memcpy(&sa.sin_addr, &ip, 4);
            }
            sa.sin_port     = htons(port);
            socks_ = cxx::net::ip::opensocket(AF_INET, proto, istcp_ ? IPPROTO_TCP : IPPROTO_UDP);
            if(socks_ == -1) {
                int r = sys::err::get();
                std::stringstream msg;
                msg << "open socket failed: " << r << ": " << sys::err::str(r);
                throw network_error(r, msg.str());
            }
            if(istcp_)
                cxx::net::tcp::reuseaddr(socks_);
            if(bind(socks_, (sockaddr* )&sa, sizeof(sa)) < 0) {
                int r = sys::err::get();
                cxx::net::ip::closesocket(socks_);
                std::stringstream msg;
                msg << "binding failed: " << r << ": " << sys::err::str(r);
                throw network_error(r, msg.str());
            }

            cxx::net::ip::unblocking(socks_);

            if(::listen(socks_, 16) < 0) {
                int r = sys::err::get();
                cxx::net::ip::closesocket(socks_);
                std::stringstream msg;
                msg << "listen failed: " << r << ": " << sys::err::str(r);
                throw network_error(r, msg.str());
            }
        }

        acceptor::~acceptor()
        {
            task_->sched()->close(socks_);
            cxx::net::ip::closesocket(socks_);
        }

        cxx::net::fd_t acceptor::accept()
        {
            task_->sched()->await(task_, socks_, cxx::net::poller::readable());

            sockaddr_in sa;
            socklen_t   len = sizeof(sa);
            cxx::net::fd_t fd = ::accept(socks_, (sockaddr* )&sa, &len);
            if(fd < 0) {
                return -1;
            }

            cxx::net::ip::unblocking(fd);
            //cxx::net::tcp::no_delays(fd);

            return fd;
        }

        ////////////////////////////////////////////////////////////////////////
        connector::connector(coroutine *c, bool istcp)
            : istcp_(istcp), task_(c)
        {
        }

        cxx::net::fd_t connector::connect(const char *server, unsigned short port, int ms)
        {
            unsigned int ip4;
            if(!netlookup(server, &ip4)) {
                int r = sys::err::get();
                std::stringstream msg;
                msg << "network lookup \'" << server << ":" << port << "' failed";
                throw network_error(r, msg.str());
            }
            int proto = istcp_ ? SOCK_STREAM : SOCK_DGRAM;
            cxx::net::fd_t fd = cxx::net::ip::opensocket(AF_INET, proto, istcp_ ? IPPROTO_TCP : IPPROTO_UDP);
            if(fd == -1) {
                int r = sys::err::get();
                std::stringstream msg;
                msg << "open socket failed: " << r << ": " << sys::err::str(r);
                throw network_error(r, msg.str());
            }

            cxx::net::ip::unblocking(fd);

            sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            memcpy(&sa.sin_addr, &ip4, 4);
            sa.sin_family   = AF_INET;
            sa.sin_port     = htons(port);

            if(::connect(fd, (sockaddr* )&sa, sizeof(sa)) < 0) {
                int r = cxx::sys::err::get();
                if(r != EINPROGRESS) {
                    cxx::net::ip::closesocket(fd);
                    std::stringstream msg;
                    msg << "connect to \'" << server << ": " << port << "\' failed: ";
                    throw network_error(r, msg.str());
                }
            }

            task_->sched()->await(task_, fd, cxx::net::poller::writable(), ms);

            socklen_t sn = sizeof(sa);
            if(getpeername(fd, (sockaddr* )&sa, &sn) >= 0) {
                return fd;
            }

            int n = 0;
            sn = sizeof(n);
            getsockopt(fd, SOL_SOCKET, SO_ERROR, &n, &sn);
            if(n == 0) {
                n = ECONNREFUSED;
            }
            task_->sched()->close(fd);
            cxx::net::ip::closesocket(fd);
            sys::err::set(n);

            return -1;
        }

        ////////////////////////////////////////////////////////////////////////
        socketor::socketor(coroutine *c, net::fd_t fd)
            : fd_(fd), cr_(c)
        {
            assert(fd_ != -1);
        }

        void socketor::close()
        {
            cr_->sched()->close(fd_);
            cxx::net::ip::closesocket(fd_);
        }

        int socketor::send(const char *data, size_t size)
        {
            int m, tot;

            for(tot = 0; tot < size; tot += m) {
                while((m = ::write(fd_, data + tot, size - tot)) < 0 &&
                      cxx::sys::err::get() == EAGAIN) {
                    cr_->sched()->await(cr_, fd_, cxx::net::poller::writable());
                }
                if(m < 0)
                    return m;
                if(m == 0)
                    return m;
            }
            return tot;
        }

        int socketor::recv(char *data, size_t size, int ms)
        {
            int m = 0;
            cxx::datetime begin(cxx::datetime::now());
            while((m = ::read(fd_, data, size)) < 0 && cxx::sys::err::get() == EAGAIN) {
                if(ms >= 0 && (cxx::datetime::now() - begin).getTotalMilliSeconds() >= ms) {
                    m = -2;
                    break;
                }
                cr_->sched()->await(cr_, fd_, cxx::net::poller::readable(), ms);
            }
            return m;
        }

        ////////////////////////////////////////////////////////////////////////
        static bool netlookup(const char *name, unsigned int *ip)
        {
            hostent* he;
            if(parseip(name, ip)) {
                return true;
            }

            if((he = gethostbyname(name)) != 0) {
                *ip = *(unsigned int* )he->h_addr;
                return true;
            }
            return false;
        }

#define CLASS(p)    ((*(unsigned char* )(p)) >> 6)
        static bool parseip(const char *name, unsigned int *ip)
        {
            unsigned char addr[4];
            char* p = (char* )name;
            int         i = 0;
            for(i = 0; i < 4 && *p; ++i) {
                int x = strtoul(p, &p, 0);
                if(x < 0 || x >= 256)
                    return false;
                if(*p != '.' && *p != 0)
                    return false;
                if(*p == '.')
                    p++;
                addr[i] = x;
            }
            switch(CLASS(addr)) {
            case 0:
            case 1:
                if(i == 3) {
                    addr[3] = addr[2];
                    addr[2] = addr[1];
                    addr[1] = 0;
                }
                else if(i == 2) {
                    addr[3] = addr[2];
                    addr[2] = 0;
                    addr[1] = 0;
                }
                else if(i != 4) {
                    return false;
                }
                break;
            case 2:
                if(i == 3) {
                    addr[3] = addr[2];
                    addr[2] = 0;
                }
                else if(i != 4) {
                    return false;
                }
                break;
            }
            *ip = *(unsigned int* )addr;
            return 0;
        }
#undef CLASS


    }
}
