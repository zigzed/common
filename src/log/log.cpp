/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/log/logmacro.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "loginfo.h"
#include "logger.h"

namespace cxx {
    namespace log {

        inline static const char* get_basename(const char* file)
        {
            const char* basefile = strrchr(file, '/');
            if(basefile) {
                return basefile + 1;
            }
            return file;
        }

        void xMessage(const char* module, LogLevel faci, const char* file, int line, const char* func, const char* format, va_list ap)
        {
            char    buffer[256];
            char*   buf = buffer;
            int     len = sizeof(buffer);

            for(int tries = 10; tries; --tries) {
                va_list args;
#if     defined(va_copy)
                va_copy(args, ap);
#elif   defined(__va_copy)
                __va_copy(args, ap);
#else
                memcpy(&args, &ap, sizeof(va_list));
#endif
                int ncopy = vsnprintf(buf, len, format, args);
                va_end(args);

                if(ncopy > -1 && ncopy < len) {
                    LogInfo info(faci, get_basename(file), func, line, buf);

                    LogManager* mgr = LogManager::get();
                    mgr->logger(module, info);
                    //printf("%s, [%s:%d:%s]: %s\n", module, info.file, line, func, buf);
                    break;
                }
                else {
                    if(ncopy > 0) {
                        len = ncopy + 1;
                    }
                    else {
                        len *= 2;
                    }
                    if(buf != buffer) {
                        delete[] buf;
                    }
                    buf = new char[len];
                }
            }

            if(buf != buffer) {
                delete[] buf;
            }
        }

        void xMessage(const char *module, LogLevel faci, const char *file, int line, const char *func, const char *format, ...)
        {
            va_list args;
            va_start(args, format);
            xMessage(module, faci, file, line, func, format, args);
            va_end(args);
        }

    }
}
