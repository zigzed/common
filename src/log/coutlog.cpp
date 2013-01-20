/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "coutlog.h"
#include <stdio.h>

namespace cxx {
    namespace log {

        static const char* level(LogLevel level)
        {
            static const char* message[] = {
                "emerg", "alert", "crit", "error",
                "warn", "notice", "info", "debug"
            };
            if(level >= 0 && level < sizeof(message)/sizeof(message[0])) {
                return message[level];
            }
            return "unknown";
        }

        CoutChannel::CoutChannel(LogLevel level)
            : color_(true), level_(level)
        {
#ifdef  OS_WINDOWS
            hout_ = GetStdHandle(STD_OUTPUT_HANDLE); //获取标准输出句柄
            GetConsoleScreenBufferInfo(hout_, &csbi_);
#endif
        }

        void CoutChannel::color(bool enable)
        {
            color_ = enable;
        }

        void CoutChannel::on_message(const LogInfo &info)
        {
            if(info.faci > level_) {
                return;
            }
            cxx::sys::plainmutex::scopelock mutex(lock_);
            if(info.faci <= cxx::log::error) {
                setupcolor();
            }

            cxx::datetime::calendar cur(info.time);
            printf("%04d%02d%02d %02d%02d%02d.%03d <%s> [%s:%d:%s] %s\n",
                   cur.year, cur.mon, cur.day, cur.hour, cur.min, cur.sec, cur.msec,
                   level(info.faci), info.file, info.line, info.func,
                   info.buff);

            resetcolor();
            if(info.faci <= cxx::log::error) {
                fflush(stdout);
            }
        }

        void CoutChannel::setupcolor()
        {
            if(color_) {
#ifdef OS_WINDOWS
                SetConsoleTextAttribute(hout_,FOREGROUND_RED|FOREGROUND_INTENSITY);//设置文本颜色
#else
                printf("%s", "\033[0;31m");
#endif
            }
        }

        void CoutChannel::resetcolor()
        {
            if(color_) {
#ifdef OS_WINDOWS
                SetConsoleTextAttribute(hout_, csbi_.wAttributes);
#else
                printf("%s", "\033[0m");
#endif
            }

        }

    }
}
