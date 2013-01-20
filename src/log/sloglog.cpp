/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "sloglog.h"
#include <string.h>
#include <stdio.h>

#if defined(OS_WINDOWS)
	#define	snprintf	_snprintf
#endif

namespace cxx {
    namespace log {

        SlogChannel::SlogChannel(LogLevel level, const char* module, Facility faci,
                                 const char *host, unsigned port)
            : level_(level), module_(module), host_(host), port_(port), faci_(faci)
        {
#if defined(OS_WINDOWS)
            WSADATA         wsaData;
            ::WSAStartup(0x101, &wsaData);
            unsigned long   ip;
#else
            in_addr_t       ip;
#endif
            struct hostent* pent = ::gethostbyname(host_.c_str());
            if(pent == NULL) {
                ip = ::inet_addr(host_.c_str());
                pent = ::gethostbyaddr((const char* )&ip, sizeof(ip), AF_INET);
                if(pent == NULL)
                    return;
            }
            addr_.sin_family        = AF_INET;
            addr_.sin_port          = htons(port_);
            ::memcpy(&addr_.sin_addr, pent->h_addr_list[0], pent->h_length);
            sock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        }

        SlogChannel::~SlogChannel()
        {
            if(sock_) {
#if defined(OS_WINDOWS)
                closesocket(sock_);
                WSACleanup();
#else
                close(sock_);
#endif
                sock_ = -1;
            }
        }

        void SlogChannel::on_message(const LogInfo &info)
        {
            if(info.faci > level_) {
                return;
            }

            char			buff[1024];
            char*           p = buff;
            int             size = 1024;
            while(true) {
                int count = snprintf(buff, size, "<%d><%s> [%s:%d:%s] %s\n",
                                     (int)faci_ + (int)level_, module_.c_str(),
                                     info.file, info.line, info.func, info.buff);
                if(count > -1 && count < size) {
                    int sent = 0;
                    cxx::sys::plainmutex::scopelock mutex(lock_);
                    while(count > sent) {
                        int bytes = sendto(sock_, buff + sent, count - sent, 0, (sockaddr* )&addr_, sizeof(addr_));
                        if(bytes > 0) {
                            sent += bytes;
                        }
                        else {
                            break;
                        }
                    }
                    break;
                }
                else {
                    if(count > 0) {
                        size = count + 1;
                    }
                    else {
                        size *= 2;
                    }
                    if(p != buff) {
                        delete[] p;
                    }
                    p = new char[size];
                }
            }
            if(p != buff) {
                delete[] p;
            }
        }

    }
}
