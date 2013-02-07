/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/ipc/mmap.h"
#include "common/sys/error.h"
#if     defined(OS_LINUX)
    #include <unistd.h>
    #include <sys/mman.h>
#elif   defined(OS_WINDOWS)
#endif

namespace cxx {
    namespace ipc {

#if     defined(OS_LINUX)

        const memory_mappable::mapping_handle_t memory_mappable::invalid_handle    = -1;

        memory_mappable::memory_mappable()
        {
        }

        memory_mappable::~memory_mappable()
        {
        }

        mapped_region::mapped_region()
            : handle_(memory_mappable::invalid_handle), buffer_(NULL),
              offset_(0), length_(0), extoff_(0)
        {
        }

        ////////////////////////////////////////////////////////////////////////
        static std::size_t get_page_size()
        {
            static std::size_t pagesize = 0;
            if(pagesize == 0) {
                pagesize = sysconf(_SC_PAGE_SIZE);
            }
            return pagesize;
        }

        mapped_region::mapped_region(const memory_mappable &mappable, accessmode_t mode,
                                     offset_t offset, std::size_t length, void *buffer)
            : handle_(mappable.handle()), buffer_(NULL), offset_(0),
              length_(0), extoff_(0), acmode_(mode)
        {
            if(length == 0) {
                offset_t total_size = mappable.size();
                if(total_size <= offset) {
                    throw ipc_error("mapped region out of range", 0);
                }
                length = total_size - offset;
            }
            move(offset, length, buffer);
        }

        void mapped_region::move(offset_t offset, std::size_t length, void *buffer)
        {
            if(buffer_ != 0) {
                close();
            }

            int prots = 0;
            int flags = 0;
            switch(acmode_) {
            case ReadOnly:
                prots |= PROT_READ;
                flags |= MAP_SHARED;
                break;
            case ReadWrite:
                prots |= (PROT_WRITE | PROT_READ);
                flags |= MAP_SHARED;
                break;
            case CopyOnWrite:
                prots |= (PROT_WRITE | PROT_READ);
                flags |= MAP_PRIVATE;
                break;
            default:
                throw ipc_error("unknown mapping mode", 0);
                break;
            }

            // mapped region must be page aligned
            size_t      page_size   = get_page_size();
            offset_t    extra_offset= offset - (offset / page_size) * page_size;
            if(buffer) {
                buffer = static_cast<char* >(buffer) - extra_offset;
            }

            void* base = mmap(buffer, extra_offset + length, prots, flags,
                              handle_, offset - extra_offset);
            if(base == MAP_FAILED) {
                int errcode = sys::err::get();
                close();
                throw ipc_error("mmap failed", errcode);
            }

            buffer_ = static_cast<char* >(base) + extra_offset;
            extoff_ = extra_offset;
            offset_ = offset;
            length_ = length;

            if(buffer && (base != buffer)) {
                close();
                throw ipc_error("can't mapping to specified address", 0);
            }
        }

        size_t mapped_region::size() const
        {
            return length_;
        }

        void* mapped_region::data() const
        {
            return buffer_;
        }

        mapped_region::offset_t mapped_region::offset() const
        {
            return offset_;
        }

        bool mapped_region::attach(const memory_mappable &mappable, accessmode_t mode)
        {
            if(handle_ != memory_mappable::invalid_handle) {
                return false;
            }
            acmode_ = mode;
            handle_ = mappable.handle();
        }

        bool mapped_region::commit(std::size_t region_offset, std::size_t bytes, bool async)
        {
            if(buffer_ == 0) {
                return false;
            }
            if(region_offset >= length_ || (region_offset + bytes) > length_) {
                return false;
            }
            if(bytes == 0) {
                bytes = length_ - region_offset;
            }

            return msync(static_cast<char* >(buffer_) + region_offset, bytes, (async ? MS_ASYNC : MS_SYNC)) == 0;
        }

        bool mapped_region::advise(advisemode_t mode)
        {
            unsigned int mode_padv = 0;
            unsigned int mode_madv = 0;
            switch(mode) {
            case Normal:
                #if     defined(POSIX_MADV_NORMAL)
                mode_padv = POSIX_MADV_NORMAL;
                #elif   defined(MADV_NORMAL)
                mode_madv = MADV_NORMAL;
                #endif
                break;
            case Random:
                #if     defined(POSIX_MADV_RANDOM)
                mode_padv = POSIX_MADV_RANDOM;
                #elif   defined(MADV_RANDOM)
                mode_madv = MADV_RANDOM;
                #endif
                break;
            case Sequential:
                #if     defined(POSIX_MADV_SEQUENTIAL)
                mode_padv = POSIX_MADV_SEQUENTIAL;
                #elif   defined(MADV_SEQUENTIAL)
                mode_madv = MADV_SEQUENTIAL;
                #endif
                break;
            case WillNeed:
                #if     defined(POSIX_MADV_WILLNEED)
                mode_padv = POSIX_MADV_WILLNEED;
                #elif   defined(MADV_WILLNEED)
                mode_madv = MADV_WILLNEED;
                #endif
                break;
            case DontNeed:
                #if     defined(POSIX_MADV_DONTNEED)
                mode_padv = POSIX_MADV_DONTNEED;
                #elif   defined(MADV_DONTNEED)
                mode_madv = MADV_DONTNEED;
                #endif
                break;
            default:
                return false;
                break;
            }

            char* address = static_cast<char * >(buffer_) - extoff_;
            size_t length = length_ + extoff_;
            if(mode_padv != 0) {
                return posix_madvise(address, length, mode_padv) == 0;
            }
            else if(mode_madv != 0) {
                return madvise(address, length, mode_madv) == 0;
            }
            return false;
        }

        mapped_region::~mapped_region()
        {
            close();
        }

        void mapped_region::close()
        {
            munmap(static_cast<char* >(buffer_) - extoff_, length_ + extoff_);
            buffer_ = 0;
            length_ = 0;
            extoff_ = 0;
        }

#elif   defined(OS_WINDOWS)

#endif

    }
}
