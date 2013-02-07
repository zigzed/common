/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/net/reactor.h"
#include "common/net/iothread.h"

namespace cxx {
    namespace net {

        reactor::reactor(int threads)
        {
            for(int i = 0; i < threads; ++i) {
                io_thread* thread = new io_thread();
                threads_.push_back(thread);
            }
        }

        reactor::~reactor()
        {
            for(int i = 0; i < threads_.size(); ++i) {
                io_thread* thread = threads_[i];
                thread->stop();
                delete thread;
            }
        }

        io_thread* reactor::choose()
        {
            io_thread* choosed = NULL;
            int        minload = -1;
            for(int i = 0; i < threads_.size(); ++i) {
                int load = threads_[i]->get_load();
                if(choosed == NULL || load < minload) {
                    minload = load;
                    choosed = threads_[i];
                }
            }
            return choosed;
        }

        void reactor::start()
        {
            for(int i = 0; i < threads_.size(); ++i) {
                threads_[i]->start();
            }
        }

        void reactor::stop()
        {
            for(int i = 0; i < threads_.size(); ++i) {
                threads_[i]->stop();
            }
        }

    }
}
