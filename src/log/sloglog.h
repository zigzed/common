/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_LOG_SLOGLOG_H
#define CXX_LOG_SLOGLOG_H
#include "common/config.h"
#include "loginfo.h"
#if defined(OS_WINDOWS)
    #include <winsock2.h>
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif
#include <string>
#include "logger.h"
#include "common/sys/mutex.h"

namespace cxx {
    namespace log {

        enum Facility {
            kern    = (0 << 3), ///< kernel messages
            user    = (1 << 3), ///< random user-level messages
            mail    = (2 << 3), ///< mail system
            daemon  = (3 << 3), ///< system daemons
            auth    = (4 << 3), ///< security/authorization messages
            syslog  = (5 << 3), ///< messages generated internally
            lpr     = (6 << 3), ///< line printer subsystem
            news    = (7 << 3), ///< network news subsystem
            uucp    = (8 << 3), ///< UUCP subsystem
            cron    = (9 << 3), ///< clock daemon
            authpriv= (10<< 3), ///< security/authorization messages (private)
            ftp     = (11<< 3), ///< ftp daemon
            local0  = (16<< 3), ///< reserved for local use
            local1  = (17<< 3), ///< reserved for local use
            local2  = (18<< 3), ///< reserved for local use
            local3  = (19<< 3), ///< reserved for local use
            local4  = (20<< 3), ///< reserved for local use
            local5  = (21<< 3), ///< reserved for local use
            local6  = (22<< 3), ///< reserved for local use
            local7  = (23<< 3)  ///< reserved for local use
        };

        class SlogChannel : public LogChannel {
        public:
            SlogChannel(LogLevel level, const char* module, Facility faci, const char* host, unsigned port);
            ~SlogChannel();
            void on_message(const LogInfo &info);
        private:
            LogLevel        level_;
            std::string     module_;
            std::string     host_;
            unsigned short  port_;
            sockaddr_in     addr_;
    #if defined(OS_WINDOWS)
            SOCKET          sock_;
    #else
            int             sock_;
    #endif
            Facility        faci_;
            cxx::sys::plainmutex lock_;
        };
    }
}

#endif
