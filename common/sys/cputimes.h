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
                int64_t wall;   //< �����ʱ�䡣������ǲ���ʱ�ӵ���Ӱ���ʱ��
                int64_t user;   //< ���ĵ��û�̬ʱ�䣬����ͳ�ƵĽ��̺����ӽ���
                int64_t sys;    //< ���ĵ�ϵͳ̬ʱ�䣬����ͳ�ƵĽ��̺����ӽ���
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
