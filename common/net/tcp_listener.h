/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_TCP_LISTENER_H
#define CXX_NET_TCP_LISTENER_H
#include "common/config.h"
#include "common/net/poller.h"
#include "common/net/tcp_address.h"
#include "common/net/options.h"

namespace cxx {
    namespace net {

        class reactor;
        class tcp_address;
        class connection_event;

        class tcp_listener : public event_sink {
        public:
            tcp_listener(reactor* r, const tcp_address &addr, const options& opt);
            ~tcp_listener();

            bool listen(int backlog);
            void close();
            void attach(connection_event* conn_cb);
            void detach(connection_event* conn_cb);

        private:
            void on_readable();
            void on_writable();
            void on_expire(int id);

            fd_t accept();

            reactor*            reactor_;
            tcp_address         address_;
            poller*             poller_;
            fd_t                socket_;
            poller::handle_t    events_;
            bool                bevent_;
            connection_event*   conn_cb_;
            std::string         endpoint_;
            options             option_;
        };

    }
}

#endif
