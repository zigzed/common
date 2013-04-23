/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/con/fd.h"
#include "common/sys/error.h"
#include "common/datetime.h"
#include <unistd.h>
#include <sstream>

namespace cxx {
    namespace con {

        static int nonblock(int fd)
        {
            return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
        }

        filer::filer(coroutine *c, const char *name)
            : fd_(-1), cr_(c)
        {
            fd_ = open(name, O_LARGEFILE | O_NOATIME);
            if(fd_ == -1) {
                int r = cxx::sys::err::get();
                std::stringstream msg;
                msg << "open file \'" << name << "\' failed: ";
                throw file_error(r, msg.str());
            }
            nonblock(fd_);
        }

        void filer::close()
        {
            cr_->sched()->erase(fd_);
            ::close(fd_);
            fd_ = -1;
        }

        off_t filer::seek(off_t offset, int from)
        {
            return lseek(fd_, offset, from);
        }

        int filer::load(char *data, int size, int ms)
        {
            int m = 0;
            cxx::datetime begin(cxx::datetime::now());
            while((m = ::read(fd_, data, size)) < 0 && cxx::sys::err::get() == EAGAIN) {
                if(ms >= 0 && (cxx::datetime::now() - begin).getTotalMilliSeconds() >= ms) {
                    m = -2;
                    break;
                }
                cr_->sched()->await(cr_, fd_, cxx::net::poller::readable(), ms);
            }
            return m;
        }

        int filer::save(const char *data, int size)
        {
            int m, tot;

            for(tot = 0; tot < size; tot += m) {
                while((m = ::write(fd_, data + tot, size - tot)) < 0 &&
                      cxx::sys::err::get() == EAGAIN) {
                    cr_->sched()->await(cr_, fd_, cxx::net::poller::writable());
                }
                if(m < 0)
                    return m;
                if(m == 0)
                    return m;
            }
            return tot;
        }

    }
}
