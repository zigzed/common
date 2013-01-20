/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_LOG_FILELOG_H
#define CXX_LOG_FILELOG_H
#include "logger.h"
#include "common/sys/mutex.h"

namespace cxx {
    namespace log {

        class FileChannel : public LogChannel {
        public:
            FileChannel(LogLevel level, const char* filename, size_t size);
            ~FileChannel();
            void on_message(const LogInfo &info);
        private:
            void rotate();
            void dclose();

            LogLevel    faci_;
            FILE*       file_;
            size_t      size_;
            size_t      vols_;
            std::string name_;
            long        curr_;
            cxx::sys::plainmutex    lock_;
        };

    }
}


#endif
