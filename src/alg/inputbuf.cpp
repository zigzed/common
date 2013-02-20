/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/inputbuf.h"
#include "common/sys/error.h"
#include <stdint.h>

namespace cxx {
    namespace alg {


        struct buf_header {
            // buffer length excluding the header and tailer
            size_t  length;
            // how many readers
            size_t  reader;
        };

        struct reader_inf {
            // the slowest reader's position
            size_t  r_tail;
        };

        struct writer_inf {
            // the flushed writer's position
            size_t  w_tail;
            // the fastest writer's position (un-flushed)
            size_t  w_head;
        };

        struct dat_header {
            // size of the item (the item size, excluding dat_header and padding)
            int     size;
            // refn of the item;
            int     refn;
        };

        // up round to sizeof(dat_header) aligned
        static size_t get_up_aligned(size_t size)
        {
            ENFORCE(sizeof(dat_header) == 8);
            if(size & 0x07)
                size = (size & (~0x07)) + 8;
            return size;
            //return (size + sizeof(dat_header) - 1) / sizeof(dat_header) * sizeof(dat_header);
        }

        // round down to sizeof(dat_header) aligned
        static size_t get_fl_aligned(size_t size)
        {
            return size / sizeof(dat_header) * sizeof(dat_header);
        }

        input_buffer::input_buffer(void *buffer, size_t size, size_t readers)
            : ownbuf_(buffer == NULL)
        {
            ENFORCE(size > sizeof(buf_header) + sizeof(writer_inf) + sizeof(reader_inf) + sizeof(dat_header));
            if(buffer == NULL) {
                size   = get_up_aligned(size);
                buffer = new char[size];
            }
            buffer_ = buffer;
            header_ = (buf_header* )buffer_;
            header_->length = get_fl_aligned(size) - sizeof(buf_header) - sizeof(writer_inf) - sizeof(reader_inf);
            header_->reader = readers;
            reader_ = (reader_inf* )((char* )buffer_ + sizeof(buf_header));
            reader_->r_tail = 0;
            writer_ = (writer_inf* )((char* )buffer_ + size - sizeof(writer_inf));
            writer_->w_head = 0;
            writer_->w_tail = 0;
        }

        input_buffer::~input_buffer()
        {
            if(ownbuf_) {
                delete[] (char* )buffer_;
            }
        }

        input_buffer::reader input_buffer::create_reader()
        {
            return reader(this);
        }

        input_buffer::writer input_buffer::create_writer()
        {
            return writer(this);
        }

        //--------------------------------------------------------------------//
        input_buffer::reader::reader(input_buffer *buf)
            : buf_(buf), hdr_(buf->header_), rdx_(buf->reader_), wtx_(buf->writer_),
              ptr_((char* )hdr_ + sizeof(buf_header) + sizeof(reader_inf)),
              pos_(0), len_(0)
        {
        }

        input_buffer::reader::~reader()
        {
        }

        bool input_buffer::reader::empty()
        {
            // 允许 reader == w_tail，不允许 w_head 等于 reader
            size_t writen = 0;
            buf_->reader_lock_acquire();
            writen = wtx_->w_tail;
            buf_->reader_lock_release();

            ENFORCE(pos_ <= writen)(pos_)(writen);
            return pos_ == writen;
        }

        size_t input_buffer::reader::read(void **buf)
        {
            if(pos_ != 0 && len_ != 0) {
                throw std::logic_error("read() must be followed by done()");
            }
            if(empty()) {
                return 0;
            }

            // if it's a padding element, rewind to the header of the buffer
            dat_header* dh = (dat_header* )((char* )ptr_ + pos_ % hdr_->length);
            if(dh->size == 0) {
                //printf("R0: %d %d %d\n", pos_, pos_ % hdr_->length, hdr_->length);
                pos_ += (hdr_->length - pos_ % hdr_->length);
                return read(buf);
            }

            len_ = get_up_aligned(dh->size) + sizeof(dat_header);
            *buf = (char* )ptr_ + pos_ % hdr_->length + sizeof(dat_header);

            //printf("r: %7d %7d %7d\n", pos_, len_, rdx_->r_tail);
            return dh->size;
        }

        void input_buffer::reader::done()
        {
            if(pos_ == 0 && len_ == 0) {
                throw std::logic_error("done() must be followed by read()");
            }          

            dat_header* dh = (dat_header* )((char* )ptr_ + pos_ % hdr_->length);
            ENFORCE(get_up_aligned(dh->size) == len_ - sizeof(dat_header));
            pos_ += len_;
            len_ = 0;

            buf_->writer_lock_acquire();
            --dh->refn;
            if(dh->refn == 0) {
                rdx_->r_tail = pos_;
            }
            buf_->writer_lock_release();

            //printf("d: %7d %7d %7d\n", pos_, len_, rdx_->r_tail);
        }

        //--------------------------------------------------------------------//
        input_buffer::writer::writer(input_buffer *buf)
            : buf_(buf), hdr_(buf->header_), rdx_(buf_->reader_), wtx_(buf_->writer_),
              ptr_((char* )hdr_ + sizeof(buf_header) + sizeof(reader_inf)),
              pos_(0), len_(0)
        {
        }

        input_buffer::writer::~writer()
        {
            // TODO: flush or rollback?
            if(len_ != 0) {
                flush();
            }
        }

        bool input_buffer::writer::full(size_t size)
        {
            // 需要在判断 full() 的同时写入包头，避免其他线程修改了 wtx_->w_head
            size_t rdtail = 0;
            size_t needed = get_up_aligned(size) + sizeof(dat_header);

            buf_->writer_lock_acquire();
            rdtail = rdx_->r_tail;
            pos_ = wtx_->w_head;

            size_t left = hdr_->length - (wtx_->w_head - rdx_->r_tail);
            if(left <= needed) {
                buf_->writer_lock_release();
                return true;
            }

            if(hdr_->length - pos_ % hdr_->length >= needed) {
                wtx_->w_head += needed;
                dat_header* dh = (dat_header* )((char* )ptr_ + pos_ % hdr_->length);
                dh->refn = hdr_->reader;
                dh->size = size;
                buf_->writer_lock_release();
                return false;
            }
            else {
                {
                    dat_header* dh = (dat_header* )((char* )ptr_ + pos_ % hdr_->length);
                    dh->refn = 0;
                    dh->size = 0;
                }

                pos_ += (hdr_->length - pos_ % hdr_->length);
                wtx_->w_head = pos_;
                if(rdtail % hdr_->length - pos_ % hdr_->length > needed) {
                    wtx_->w_head += needed;
                    dat_header* dh = (dat_header* )((char* )ptr_ + pos_ % hdr_->length);
                    dh->refn = hdr_->reader;
                    dh->size = size;
                    buf_->writer_lock_release();
                    return false;
                }
                else {
                    buf_->writer_lock_release();
                    return true;
                }
            }
        }

        void* input_buffer::writer::reserve(size_t size)
        {
            if(get_up_aligned(size) + sizeof(dat_header)  >= hdr_->length / 2) {
                throw std::logic_error("memory is too large to fit into inputbuf");
            }
            if(pos_ != 0 && len_ != 0) {
                throw std::logic_error("reserve() must be followed by flush()");
            }
            if(full(size)) {
                return NULL;
            }

            len_ = get_up_aligned(size) + sizeof(dat_header);
            //printf("x: %7d %7d %7d %7d\n", pos_, len_, wtx_->w_tail, wtx_->w_head);

            return ((char* )ptr_ + pos_ % hdr_->length + sizeof(dat_header));
        }

        void input_buffer::writer::flush()
        {
            if(pos_ == 0 && len_ == 0) {
                throw std::logic_error("reserve() must be followed by flush()");
            }

            buf_->writer_lock_acquire();
            if(pos_ == wtx_->w_tail || pos_ % hdr_->length < wtx_->w_tail) {
                wtx_->w_tail = pos_ + len_;
            }
            buf_->writer_lock_release();

            pos_ += len_;
            len_ = 0;
            //printf("f: %7d %7d %7d %7d\n", pos_, len_, wtx_->w_tail, wtx_->w_head);
        }

        //--------------------------------------------------------------------//
        ns_input_buffer::ns_input_buffer(void *buffer, size_t size, size_t readers)
            : input_buffer(buffer, size, readers)
        {
        }

        ts_input_buffer::ts_input_buffer(void *buffer, size_t size, size_t readers)
            : input_buffer(buffer, size, readers)
        {
        }

        void ts_input_buffer::reader_lock_acquire()
        {
            mutex_.acquire();
        }

        void ts_input_buffer::reader_lock_release()
        {
            mutex_.release();
        }

        void ts_input_buffer::writer_lock_acquire()
        {
            mutex_.acquire();
        }

        void ts_input_buffer::writer_lock_release()
        {
            mutex_.release();
        }

        ps_input_buffer::ps_input_buffer(void *buffer, size_t size, size_t readers, const char *name)
            : input_buffer(buffer, size, readers), mutex_(name)
        {
        }

        void ps_input_buffer::reader_lock_acquire()
        {
            mutex_.acquire();
        }

        void ps_input_buffer::reader_lock_release()
        {
            mutex_.release();
        }

        void ps_input_buffer::writer_lock_acquire()
        {
            mutex_.acquire();
        }

        void ps_input_buffer::writer_lock_release()
        {
            mutex_.release();
        }

    }
}
