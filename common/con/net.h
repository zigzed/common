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
            acceptor(coroutine* c, bool istcp, const char* server, unsigned short port);
            ~acceptor();
            cxx::net::fd_t accept();
        private:
            bool            istcp_;
            cxx::net::fd_t  socks_;
            coroutine*      task_;
        };

        class connector {
        public:
            explicit connector(coroutine* c, bool istcp);
            cxx::net::fd_t connect(const char* server, unsigned short port);
        private:

            bool        istcp_;
            coroutine*  task_;
        };

        class socketor {
        public:
            explicit socketor(coroutine* c, cxx::net::fd_t fd);
            int     recv(char* data, size_t size);
            int     send(const char* data, size_t size);
            void    close();
        private:
            cxx::net::fd_t  fd_;
            coroutine*      cr_;
        };

    }
}

#endif
