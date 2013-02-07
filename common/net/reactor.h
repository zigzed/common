/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_NET_REACTOR_H
#define CXX_NET_REACTOR_H
#include "common/config.h"
#include <vector>

namespace cxx {
    namespace net {

        class io_thread;
        class reactor {
        public:
            explicit reactor(int threads);
            ~reactor();
            io_thread* choose();

            void start();
            void stop();
        private:
            typedef std::vector<io_thread* >    io_thread_t;
            io_thread_t threads_;
        };

    }
}

#endif
