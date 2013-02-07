/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/net/tcp_connection.h"
#include "common/net/helper.h"
#include <cassert>
#if     defined(OS_LINUX)
    #include <sys/socket.h>
#elif   defined(OS_WINDOWS)
    #include <winsock2.h>
#endif

namespace cxx {
    namespace net {

        tcp_connection::tcp_connection(fd_t fd, const char *endpoint, const options& opt)
            : socket_(fd), endpoint_(endpoint), option_(opt)
        {
            assert(socket_ != -1);
            ip::unblocking(socket_);
            if(option_.sndbuf) {
                tcp::setsndbuf(socket_, option_.sndbuf);
            }
            if(option_.rcvbuf) {
                tcp::setrcvbuf(socket_, option_.rcvbuf);
            }

#ifdef  SO_NOSIGPIPE
            int set = 1;
            int rc = setsockopt(socket_, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
            ENFORCE(rc == 0)(rc)(cxx::sys::err::get());
#endif
        }

        tcp_connection::~tcp_connection()
        {
            close();
        }

        void tcp_connection::close()
        {
            ip::closesocket(socket_);
        }

        int tcp_connection::send(const char *data, int size)
        {
            int sent_bytes = 0;
            int sent_flags = 0;
#if defined(MSG_NOSIGNAL)
            sent_flags = MSG_NOSIGNAL;
#endif
            sent_bytes = ::send(socket_, data, size, sent_flags);
            if(sent_bytes == -1) {
                int code = cxx::sys::err::get();
                if(code == EAGAIN || code == EWOULDBLOCK || code == EINTR) {
                    return 0;
                }
                else {
                    ENFORCE(code != EACCES && code != EBADF && code != EDESTADDRREQ
                         && code != EFAULT && code != EINVAL && code != EISCONN
                         && code != EMSGSIZE && code != ENOMEM && code != ENOTSOCK
                         && code != EOPNOTSUPP)(code);
                    return -1;
                }
            }
            return sent_bytes;
        }

        int tcp_connection::recv(char *data, int size)
        {
            int rcvd_bytes = 0;
            int rcvd_flags = 0;
            rcvd_bytes = ::recv(socket_, data, size, rcvd_flags);
            if(rcvd_bytes == -1) {
                int code = cxx::sys::err::get();
                if(code == EAGAIN || code == EWOULDBLOCK || code == EINTR) {
                    return 0;
                }
                else {
                    ENFORCE(code != EBADF && code != EFAULT && code != EINVAL
                         && code != ENOMEM && code != ENOTSOCK)(code);
                    return -1;
                }
            }
            return rcvd_bytes;
        }

        fd_t tcp_connection::get_fd() const
        {
            return socket_;
        }

    }
}
