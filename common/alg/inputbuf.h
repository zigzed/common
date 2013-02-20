/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_INPUTBUF_H
#define CXX_ALG_INPUTBUF_H
#include "common/config.h"
#include "common/sys/mutex.h"

namespace cxx {
    namespace alg {

        struct buf_header;
        struct writer_inf;
        struct reader_inf;
        struct dat_header;

        class input_buffer {
        public:
            class writer {
            public:
                explicit writer(input_buffer* buf);
                ~writer();
                /** reserve some buffer an element to write
                 * @param size  the needed size in bytes
                 * @return return the allocated buffer or NULL if input_buffer
                 * is full
                 */
                void* reserve(size_t size);
                /** write the reserved buffer to the input_buffer
                 */
                void flush();
                /** check if the writer is full */
                bool full(size_t size);
            private:
                input_buffer* buf_;
                buf_header*   hdr_;
                reader_inf*   rdx_;
                writer_inf*   wtx_;
                void*         ptr_;
                size_t        pos_;
                size_t        len_;
            };

            class reader {
            public:
                explicit reader(input_buffer* buf);
                ~reader();
                /** read an element from the input_buffer
                 * @param buf   buffer pointer to the next unreaded element. if
                 * input_buffer is empty, buf is NULL.
                 * @return return the size of the next unreaded element. if
                 * input_buffer is empty, return 0 instead
                 */
                size_t  read(void** buf);
                /** the element get by read() is processed, and the buffer can
                 * be reused by other writers
                 */
                void    done();
                /** check if the this reader is read all elements */
                bool    empty();
            private:
                input_buffer* buf_;
                buf_header*   hdr_;
                reader_inf*   rdx_;
                writer_inf*   wtx_;
                void*         ptr_;
                size_t        pos_;
                size_t        len_;
            };

            /** create a input_buffer object
             * an input_buffer is a buffer which read/write with reader and writer
             * interface. which can have multiple readers for one input_buffer,
             * every reader process every elements of the input_buffer
             *
             * @param buffer    if buffer is not null, the buffer is managed by
             * caller (create and destroy). if buffer is null, the buffer is
             * allocated by input_buffer
             * @param size      the buffer size in bytes
             * @param reader    the reader counts
             */
            input_buffer(void* buffer, size_t size, size_t readers);
            virtual ~input_buffer();

            /** create a writer object to write to the input_buffer */
            writer  create_writer();
            /** create a reader object to read from the input_buffer */
            reader  create_reader();
        protected:
            virtual void reader_lock_acquire() = 0;
            virtual void reader_lock_release() = 0;
            virtual void writer_lock_acquire() = 0;
            virtual void writer_lock_release() = 0;
        private:
            friend class reader;
            friend class writer;

            void*       buffer_;
            buf_header* header_;
            reader_inf* reader_;
            writer_inf* writer_;
            bool        ownbuf_;
        };

        // a thread-neutral input buffer, not thread-safe
        class ns_input_buffer : public input_buffer {
        public:
            ns_input_buffer(void* buffer, size_t size, size_t readers);
        private:
            void reader_lock_acquire() {}
            void reader_lock_release() {}
            void writer_lock_acquire() {}
            void writer_lock_release() {}
        };

        // a thread-safe input buffer
        class ts_input_buffer : public input_buffer {
        public:
            ts_input_buffer(void* buffer, size_t size, size_t readers);
        private:
            void reader_lock_acquire();
            void reader_lock_release();
            void writer_lock_acquire();
            void writer_lock_release();

            cxx::sys::plainmutex    mutex_;
        };

        // a process-safe input buffer, used for inter-process communication
        class ps_input_buffer : public input_buffer {
        public:
            ps_input_buffer(void* buffer, size_t size, size_t readers, const char* name);
        private:
            void reader_lock_acquire();
            void reader_lock_release();
            void writer_lock_acquire();
            void writer_lock_release();

            cxx::sys::namedmutex    mutex_;
        };

    }
}

#endif
