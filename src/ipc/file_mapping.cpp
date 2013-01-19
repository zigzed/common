/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/ipc/mmap.h"
#if     defined(OS_LINUX)
    #include <sys/stat.h>   // for fstat
    #include <unistd.h>     // for unlink
    #include <fcntl.h>      // for open
    #include <errno.h>
#elif   defined(OS_WINDOWS)
#endif

namespace cxx {
    namespace ipc {

#if     defined(OS_LINUX)

        bool file_mapping::remove(const char *filename)
        {
            return (::unlink(filename) == 0);
        }

        file_mapping::file_mapping()
            : refcnt_(new int(1)), handle_(invalid_handle), mode_(InvalidMode)
        {
        }

        file_mapping::file_mapping(const char *filename, memory_mappable::accessmode_t mode)
            : refcnt_(new int(1)), handle_(invalid_handle), mode_(mode)
        {
#if     defined(_LARGEFILE64_SOURCE)
            int flags = O_CREAT | O_LARGEFILE;
#else
            int flags = O_CREAT;
#endif
            if(mode == ReadOnly) {
                flags |= O_RDONLY;
            }
            else if(mode == ReadWrite) {
                flags |= O_RDWR;
            }
            handle_ = open(filename, flags, 0644);
            if(handle_ == invalid_handle) {
                throw ipc_error("can't open file for mapping", errno);
            }
        }

        file_mapping::file_mapping(const file_mapping &rhs)
        {
            if(--*refcnt_ <= 0) {
                close();
            }
            refcnt_ = rhs.refcnt_;
            handle_ = rhs.handle_;
            mode_   = rhs.mode_;
            ++*refcnt_;
        }

        file_mapping& file_mapping::operator = (const file_mapping& rhs)
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

        file_mapping::~file_mapping()
        {
            if(--*refcnt_ <= 0) {
                close();
            }
        }

        memory_mappable::offset_t file_mapping::size() const
        {
            struct ::stat   buf;
            if(0 != fstat(handle_, &buf)) {
                throw ipc_error("can't stat file size", errno);
            }
            return buf.st_size;
        }

        memory_mappable::offset_t file_mapping::size(offset_t len)
        {
            offset_t cur = size();
            if(0 != ftruncate(handle_, len)) {
                throw ipc_error("can't truncate file size", errno);
            }
            return cur;
        }

        memory_mappable::mapping_handle_t file_mapping::handle() const
        {
            return handle_;
        }

        memory_mappable::accessmode_t file_mapping::mode() const
        {
            return mode_;
        }

        void file_mapping::close()
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
