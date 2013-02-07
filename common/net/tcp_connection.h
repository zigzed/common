/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_TCP_CONNECTION_H
#define CXX_NET_TCP_CONNECTION_H
#include "common/config.h"
#include "common/net/poller.h"
#include "common/net/options.h"

namespace cxx {
    namespace net {

        /** tcp_connection is a connected tcp object, which can be established
         * with tcp_connector or tcp_listener
         */
        class connection_event {
        public:
            virtual ~connection_event() {}
            virtual void on_connected   (const char* addr, fd_t fd) = 0;
            virtual void on_connect_fail(const char* addr, int err) = 0;
            virtual void on_accepted    (const char* addr, fd_t fd) = 0;
            virtual void on_disconnected(const char* addr, fd_t fd) = 0;
            virtual void on_closed      (const char* addr, fd_t fd) = 0;

        };

        class tcp_connection {
        public:
            explicit tcp_connection(fd_t fd, const char* endpoint, const options& opt);
            ~tcp_connection();
            fd_t    get_fd() const;
            int     send(const char* data, int size);
            int     recv(char* data, int size);
            void    close();
        private:
            fd_t        socket_;
            std::string endpoint_;
            options     option_;
        };

    }
}

#endif
