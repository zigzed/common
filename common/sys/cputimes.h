/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_SYS_CPUTIMES_H
#define CXX_SYS_CPUTIMES_H
#include "common/config.h"
#include <stdint.h>
#include <string>

namespace cxx {
    namespace sys {

        class cpu_times {
        public:
            struct times {
                int64_t wall;   //< 计算机时间。计算的是不受时钟调整影响的时间
                int64_t user;   //< 消耗的用户态时间，包括统计的进程和其子进程
                int64_t sys;    //< 消耗的系统态时间，包括统计的进程和其子进程
                times() : wall(0), user(0), sys(0) {}
                void clear() {
                    wall = user = sys = 0LL;
                }
            };

            cpu_times();
            times   elapsed() const;
            void    start();
            void    stop();
            void    resume();

            std::string report() const;
        private:
            times   usages_;
            bool    in_use_;
        };

    }
}

#endif
