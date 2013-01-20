/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_LOG_COUTLOG_H
#define CXX_LOG_COUTLOG_H
#include "common/config.h"
#include "common/log/log.h"
#include "common/sys/mutex.h"
#include "loginfo.h"
#include "logger.h"

namespace cxx {
    namespace log {

        class CoutChannel : public LogChannel {
        public:
            CoutChannel(LogLevel level);
            void color(bool enable);
            void on_message(const LogInfo &info);
        private:
            void setupcolor();
            void resetcolor();

            bool        color_;
            LogLevel    level_;
            cxx::sys::plainmutex    lock_;
#ifdef OS_WINDOWS
            CONSOLE_SCREEN_BUFFER_INFO csbi_;
            HANDLE                     hout_;
#endif
        };

    }
}

#endif
