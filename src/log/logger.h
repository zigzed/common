/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_LOG_LOGGER_H
#define CXX_LOG_LOGGER_H
#include <map>
#include <set>
#include <string>
#include "common/sys/mutex.h"
#include "loginfo.h"

namespace cxx {
    namespace log {

        class LogChannel {
        public:
            virtual ~LogChannel() {}
            virtual void on_message(const LogInfo& info) = 0;
        };

        class LogManager {
        public:
            static LogManager* get();
            void attach(const char* module, LogChannel* channel);
            void detach(const char* module, LogChannel* channel);
            void logger(const char* module, const LogInfo& info);
        private:
            typedef std::set<LogChannel* >              ChannelList;
            typedef std::map<std::string, ChannelList > ChannelInfo;
            ChannelInfo             device_;
            cxx::sys::plainmutex    locker_;
        };

    }
}

#endif
