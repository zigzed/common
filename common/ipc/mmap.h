/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_IPC_MMAP_H
#define CXX_IPC_MMAP_H
#include <cstddef>  // for std::size_t
#include <stdint.h>
#include <stdexcept>
#include "common/config.h"

namespace cxx {
    namespace ipc {

        class ipc_error : public std::runtime_error {
        public:
            ipc_error(const char* msg, int code)
                : std::runtime_error(msg), code_(code)
            {
            }

            int code() const { return code_; }
        private:
            int code_;
        };

        class memory_mappable {
        public:
            enum accessmode_t { InvalidMode, ReadOnly, ReadWrite };
#if     defined(OS_LINUX)
            typedef int     mapping_handle_t;
#elif   defined(OS_WINDOWS)
            typedef HANDLE  mapping_handle_t;
#endif
            typedef int64_t offset_t;

            static const    mapping_handle_t    invalid_handle;

            memory_mappable();
            virtual ~memory_mappable();
            virtual mapping_handle_t    handle() const = 0;
            virtual accessmode_t        mode() const = 0;
            virtual offset_t            size() const = 0;
            virtual offset_t            size(offset_t len) = 0;
        };

        class mapped_region {
        public:
            enum accessmode_t { InvalidMode, ReadOnly, ReadWrite, CopyOnWrite };
            enum advisemode_t { Normal, Random, Sequential, WillNeed, DontNeed };

            typedef int64_t offset_t;

            mapped_region();
            mapped_region(const memory_mappable&    mappable,
                          accessmode_t              mode,
                          offset_t                  offset = 0,
                          std::size_t               length = 0,
                          void*                     buffer = 0);
            ~mapped_region();

            std::size_t     size() const;
            void*           data() const;
            void            move(offset_t offset, std::size_t length, void* buffer = 0);
            offset_t        offset() const;
            bool            commit(std::size_t region_offset = 0, std::size_t bytes = 0, bool async = true);
            bool            advise(advisemode_t mode);
            bool            attach(const memory_mappable& mappable, accessmode_t mode = ReadWrite);
        private:
            void close();
            memory_mappable::mapping_handle_t   handle_;
            void*                               buffer_;
            offset_t                            offset_;
            std::size_t                         length_;
            std::size_t                         extoff_;
            accessmode_t                        acmode_;

            mapped_region(const mapped_region& rhs);
            mapped_region& operator= (const mapped_region& rhs);
        };

        class file_mapping : public memory_mappable {
        public:
            static bool remove(const char* filename);

            file_mapping();
            file_mapping(const char* filename, memory_mappable::accessmode_t mode);
            file_mapping(const file_mapping& rhs);
            file_mapping& operator= (const file_mapping& rhs);
            ~file_mapping();

            offset_t            size(offset_t len);
            offset_t            size() const;
            mapping_handle_t    handle() const;
            accessmode_t        mode() const;
        private:
            void close();
            int*                refcnt_;
            mapping_handle_t    handle_;
            accessmode_t        mode_;
        };

        class shared_memory : public memory_mappable {
        public:
            static bool remove(const char* filename);

            shared_memory();
            shared_memory(const char* name, memory_mappable::accessmode_t mode);
            shared_memory(const shared_memory& rhs);
            shared_memory& operator= (const shared_memory& rhs);
            ~shared_memory();

            offset_t            size(offset_t len);
            offset_t            size() const;
            mapping_handle_t    handle() const;
            accessmode_t        mode() const;
        private:
            void close();

            int*                refcnt_;
            mapping_handle_t    handle_;
            accessmode_t        mode_;
        };

        /***********************************************************************
         * @ file mapping example:
         * cxx::ipc::file_mapping mapping("/home/usr/file", cxx::ipc::memory_mappable::ReadWrite);
         * mapping.size(100 * 1024 * 1024);
         * for(int i = 0; i < 100 * 1024 * 1024 / 4096; ++i) {
         *     cxx::ipc::mapped_region region(mapping, cxx::ipc::mapped_region::ReadWrite, 0, 4096);
         *     void* address = region.data();
         *     memset(address, 0xFF, 4096);
         *     region.flush();
         * }
         *
         * cxx::ipc::file_mapping::remove("/home/usr/file");
         **********************************************************************/

    }
}

#endif
