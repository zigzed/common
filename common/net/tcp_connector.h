/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_TCP_CONNECTOR_H
#define CXX_NET_TCP_CONNECTOR_H
#include "common/config.h"
#include "common/net/poller.h"
#include "common/net/tcp_address.h"
#include "common/net/options.h"
#include <string>

namespace cxx {
    namespace net {

        class reactor;
        class tcp_address;
        class connection_event;

        /** tcp_connector used makeing connection with remote peer only. when
         * connected, tcp_connector will notify the user the event and the
         * socket file descriptor (for windows, socket handle)
         */
        class tcp_connector : public event_sink {
        public:
            tcp_connector(reactor* r, const tcp_address& addr, const options& opt);
            ~tcp_connector();

            void attach(connection_event* conn_cb);
            void detach(connection_event* conn_cb);
            void close();
        private:
            void on_readable();
            void on_writable();
            void on_expire(int id);

            void doconnect();
            void doclosing();
            fd_t connected();
            void reconnect();

            enum { reconnect_timer_id = 1 };

            reactor*            reactor_;
            poller*             poller_;
            std::string         endpoint_;
            tcp_address         address_;
            fd_t                socket_;
            poller::handle_t    events_;
            bool                bevent_;
            options             option_;
            connection_event*   conncb_;
        };

    }
}

#endif
