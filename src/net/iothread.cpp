#include "common/net/iothread.h"
#include "common/net/poller.h"
#include <cassert>

namespace cxx {
    namespace net {

        io_thread::io_thread()
            : hevent_(NULL), poller_(NULL)
        {
            poller_ = poller::create();
            hevent_ = poller_->add_fd(events_.handle(), this);
            poller_->add_fd(hevent_, poller::readable());
        }

        io_thread::~io_thread()
        {
            poller_->stop();
            poller_->destroy();
        }

        void io_thread::start()
        {
            poller_->start();
        }

        void io_thread::stop()
        {
            events_.send();
        }

        void io_thread::on_readable()
        {
            poller_->del_fd(hevent_, poller::readable());
            poller_->del_fd(hevent_);
            events_.recv();
            poller_->stop();
        }

        void io_thread::on_writable()
        {
            assert(false);
        }

        void io_thread::on_expire(int id)
        {
            assert(false);
        }

        poller* io_thread::get_poller()
        {
            return poller_;
        }

        int io_thread::get_load()
        {
            return poller_->get_loads();
        }

    }
}
