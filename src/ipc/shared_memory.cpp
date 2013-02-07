/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/ipc/mmap.h"
#include "common/sys/error.h"
#if     defined(OS_LINUX)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#elif   defined(OS_WINDOWS)
#endif

namespace cxx {
    namespace ipc {

#if     defined(OS_LINUX)

        bool shared_memory::remove(const char *filename)
        {
            return (::shm_unlink(filename) == 0);
        }

        shared_memory::shared_memory()
            : refcnt_(new int(1)), handle_(invalid_handle), mode_(InvalidMode)
        {
        }

        shared_memory::shared_memory(const char *name, memory_mappable::accessmode_t mode)
            : refcnt_(new int(1)), handle_(invalid_handle), mode_(mode)
        {
            int oflag = 0;
            if(mode == ReadOnly) {
                oflag = O_RDONLY;
            }
            else if(mode == ReadWrite) {
                oflag = O_RDWR;
            }
            oflag |= O_CREAT;
            while(true) {
                handle_ = shm_open(name, oflag, 0644);
                if(handle_ >= 0) {
                    fchmod(handle_, 0644);
                    break;
                }
                else if(sys::err::get() == EEXIST) {
                    if((handle_ = shm_open(name, oflag, 0644)) >= 0 || sys::err::get() != ENOENT) {
                        break;
                    }
                }
            }
            if(handle_ == invalid_handle) {
                int errcode = sys::err::get();
                close();
                throw ipc_error("can't open shm for mapping", errcode);
            }
        }

        memory_mappable::offset_t shared_memory::size() const
        {
            struct ::stat   buf;
            if(0 != fstat(handle_, &buf)) {
                throw ipc_error("can't stat shm size", sys::err::get());
            }
            return buf.st_size;
        }

        memory_mappable::offset_t shared_memory::size(offset_t len)
        {
            offset_t cur = size();
            if(0 != ftruncate(handle_, len)) {
                throw ipc_error("can't truncate shm size", sys::err::get());
            }
            return cur;
        }

        memory_mappable::mapping_handle_t shared_memory::handle() const
        {
            return handle_;
        }

        memory_mappable::accessmode_t shared_memory::mode() const
        {
            return mode_;
        }

        shared_memory::shared_memory(const shared_memory &rhs)
        {
            if(--*refcnt_ <= 0) {
                close();
            }
            refcnt_ = rhs.refcnt_;
            handle_ = rhs.handle_;
            mode_   = rhs.mode_;
            ++*refcnt_;
        }

        shared_memory& shared_memory::operator = (const shared_memory& rhs)
        {
            if(this == &rhs) {
                return *this;
            }
            if(--*refcnt_ <= 0) {
                close();
            }
            refcnt_ = rhs.refcnt_;
            handle_ = rhs.handle_;
            mode_   = rhs.mode_;
            ++*refcnt_;
        }

        shared_memory::~shared_memory()
        {
            if(--*refcnt_ <= 0) {
                close();
            }
        }

        void shared_memory::close()
        {
            delete refcnt_;
            refcnt_ = NULL;
            if(handle_ != invalid_handle) {
                ::close(handle_);
            }
        }

#elif   defined(OS_WINDOWS)
#endif

    }
}
