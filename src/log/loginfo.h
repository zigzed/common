/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_LOG_LOGINFO_H
#define CXX_LOG_LOGINFO_H
#include "common/datetime.h"
#include "common/log/log.h"

namespace cxx {
    namespace log {

        struct LogInfo {
            LogLevel        faci;
            const char*     file;
            const char*     func;
            int             line;
            cxx::datetime   time;
            char*           buff;
            LogInfo(LogLevel l, const char* f, const char* x, int n, char* b)
                : faci(l), file(f), func(x), line(n), time(cxx::datetime::now()), buff(b)
            {
            }
        };

    }
}

#endif
