/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/net/tcp_listener.h"
#include "common/net/tcp_address.h"
#include "common/net/tcp_connection.h"
#include "common/net/reactor.h"
#include "common/net/iothread.h"
#include "common/net/helper.h"
#include "common/sys/error.h"

namespace cxx {
    namespace net {

        tcp_listener::tcp_listener(reactor *r, const tcp_address& addr, const options& opt)
            : reactor_(r), address_(addr), poller_(NULL), socket_(-1), bevent_(false),
              conn_cb_(NULL), option_(opt)
        {
            endpoint_ = addr.string();
        }

        tcp_listener::~tcp_listener()
        {
            close();
        }

        void tcp_listener::close()
        {
            if(bevent_) {
                poller_->del_fd(events_);
                bevent_ = false;
            }
            if(socket_ != -1) {
                if(conn_cb_)
                    conn_cb_->on_closed(endpoint_.c_str(), socket_);
                ip::closesocket(socket_);
            }
        }

        void tcp_listener::attach(connection_event *conn_cb)
        {
            conn_cb_ = conn_cb;
        }

        void tcp_listener::detach(connection_event *conn_cb)
        {
            ENFORCE(conn_cb_ == conn_cb)(conn_cb_)(conn_cb);
            conn_cb_ = NULL;
        }

        bool tcp_listener::listen()
        {
            socket_ = ip::opensocket(address_.family(), SOCK_STREAM, IPPROTO_TCP);
            if(socket_ == -1) {
                return false;
            }

            ip::unblocking(socket_);
            tcp::reuseaddr(socket_);

            io_thread* thread = reactor_->choose();
            poller_ = thread->get_poller();

            int rc = ::bind(socket_, address_.address(), address_.addrlen());
            if(rc != 0) {
                ip::closesocket(socket_);
                return false;
            }

            rc = ::listen(socket_, option_.backlog);
            if(rc != 0) {
                ip::closesocket(socket_);
                return false;
            }
            events_ = poller_->add_fd(socket_, this);
            bevent_ = true;
            poller_->add_fd(events_, poller::readable());
            return true;
        }

        fd_t tcp_listener::accept()
        {
            ENFORCE(socket_ != -1)(socket_);

            sockaddr_storage ss;
            socklen_t        ss_len = sizeof(ss);
            fd_t sock = ::accept(socket_, (sockaddr* )&ss, &ss_len);
            if(sock == -1) {
                int e = cxx::sys::err::get();
                ENFORCE(e == EAGAIN || e == EWOULDBLOCK || e == EINTR
                        || e == ECONNABORTED || e == EPROTO || e == ENOBUFS
                        || e == ENOMEM || e == EMFILE || e == ENFILE);
                return -1;
            }

            if(!option_.filter.empty()) {
                bool matched = false;
                for(options::mask_t::iterator it = option_.filter.begin(); it != option_.filter.end(); ++it) {
                    tcp_address_mask& mask = *it;
                    if(mask.matched((sockaddr* )&ss, ss_len)) {
                        matched = true;
                        break;
                    }
                }
                if(!matched) {
                    ip::closesocket(sock);
                    return -1;
                }
            }

            return sock;
        }

        void tcp_listener::on_readable(fd_t)
        {
            fd_t fd = accept();
            if(fd == -1) {
                return;
            }

            if(conn_cb_)
                conn_cb_->on_accepted(endpoint_.c_str(), fd);
        }

        void tcp_listener::on_writable(fd_t)
        {
            ENFORCE(false);
        }

        void tcp_listener::on_expire(int id)
        {
            ENFORCE(false);
        }


    }
}
