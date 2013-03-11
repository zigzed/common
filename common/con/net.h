/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_NET_H
#define CXX_CON_NET_H
#include "common/con/coroutine.h"
#include "common/net/poller.h"

namespace cxx {
    namespace con {

        class network_error : public std::runtime_error {
        public:
            network_error(int code, const std::string& msg);
            int code() const;
        private:
            int code_;
        };

        class acceptor {
        public:
            acceptor(bool istcp, const char* server, unsigned short port);
            cxx::net::fd_t accept(coroutine* c);
        private:
            bool            istcp_;
            cxx::net::fd_t  socks_;
        };

        class connector {
        public:
            explicit connector(bool istcp);
            cxx::net::fd_t connect(coroutine* c, const char* server, unsigned short port);
        private:

            bool    istcp_;
        };

        class socketor {
        public:
            explicit socketor(cxx::net::fd_t fd);
            int     recv(coroutine* c, char* data, size_t size);
            int     send(coroutine* c, const char* data, size_t size);
            void    close();
        private:
            cxx::net::fd_t  fd_;
        };

    }
}

#endif
