/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/net/tcp_connector.h"
#include "common/net/reactor.h"
#include "common/net/iothread.h"
#include "common/net/helper.h"
#include "common/net/tcp_connection.h"
#include "common/sys/error.h"
#include <cassert>

namespace cxx {
    namespace net {

        tcp_connector::tcp_connector(reactor *r, const tcp_address& addr, const options& opt)
            : reactor_(r), poller_(NULL), address_(addr), socket_(-1), bevent_(false),
              option_(opt), conncb_(NULL)
        {
            endpoint_ = addr.string();
            reconnect();
        }

        tcp_connector::~tcp_connector()
        {
            close();
        }

        void tcp_connector::attach(connection_event *conn_cb)
        {
            conncb_ = conn_cb;
        }

        void tcp_connector::detach(connection_event *conn_cb)
        {
            assert(conncb_ == conn_cb);
            conncb_ = NULL;
        }

        void tcp_connector::close()
        {
            poller_->del_timer(reconnect_timer_id, this);
            if(bevent_) {
                poller_->del_fd(events_);
                bevent_ = false;
            }
            if(socket_ != -1) {
                if(conncb_)
                    conncb_->on_closed(endpoint_.c_str(), socket_);
                ip::closesocket(socket_);
            }
        }

        void tcp_connector::doconnect()
        {
            socket_ = ip::opensocket(address_.family(), SOCK_STREAM, IPPROTO_TCP);
            ip::unblocking(socket_);

            io_thread* thread = reactor_->choose();
            poller_ = thread->get_poller();

            int rc = connect(socket_, address_.address(), address_.addrlen());
            if(rc == 0) {
                // connecting is successed immediately
                events_ = poller_->add_fd(socket_, this);
                bevent_ = true;

                on_writable();
                return;
            }
            else if(sys::err::get() == EINPROGRESS) {
                // connecting is in progress
                events_ = poller_->add_fd(socket_, this);
                bevent_ = true;
                poller_->add_fd(events_, poller::writable());
                return;
            }
            else {
                // other errors such as refuse, resource exhaust, timeout ...
                ip::closesocket(socket_);
                reconnect();
            }
        }

        void tcp_connector::reconnect()
        {
            if(option_.conivl > 0) {
                poller* p = reactor_->choose()->get_poller();
                p->add_timer(reconnect_timer_id, option_.conivl, this);
            }
        }

        fd_t tcp_connector::connected()
        {
            int         err = 0;
            socklen_t   len = sizeof(err);
            int rc = getsockopt(socket_, SOL_SOCKET, SO_ERROR, &err, &len);
            assert(rc == 0);

            if(err != 0) {
                sys::err::set(err);
                assert(err == ECONNREFUSED || err == ECONNRESET ||
                       err == ETIMEDOUT || err == EHOSTUNREACH ||
                       err == ENETUNREACH || err == ENETDOWN);
                return -1;
            }
            fd_t result = socket_;
            socket_ = -1;
            return result;
        }

        void tcp_connector::on_readable()
        {
            on_writable();
        }

        void tcp_connector::on_writable()
        {
            fd_t fd = connected();
            if(bevent_) {
                poller_->del_fd(events_);
                bevent_ = false;
            }

            if(fd == -1) {
                ip::closesocket(socket_);
                if(conncb_)
                    conncb_->on_connect_fail(endpoint_.c_str(), sys::err::get());
                reconnect();
                return;
            }

            if(conncb_)
                conncb_->on_connected(endpoint_.c_str(), fd);
        }

        void tcp_connector::on_expire(int id)
        {
            assert(id == reconnect_timer_id);
            doconnect();
        }


    }
}
