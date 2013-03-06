/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_CHANNEL_H
#define CXX_CON_CHANNEL_H
#include "common/con/coroutine.h"
#include "common/sys/mutex.h"

namespace cxx {
    namespace con {

        template<typename T, typename L = cxx::sys::plainmutex >
        class channel {
        public:
            explicit channel(size_t size);
            ~channel();

            bool send(coroutine* c, const T& v);
            bool recv(coroutine* c, T& v);
        private:
            void close();

            L       lock_;
            bool    closed_;
            size_t  capacity_;
            size_t  size_;
            T*      data_;
            size_t  rs_;
            size_t  ws_;
        };

        template<typename T, typename L >
        inline channel<T, L >::channel(size_t size)
            : closed_(false), capacity_(size), size_(0), data_(new T[size]),
              rs_(0), ws_(0)
        {
        }

        template<typename T, typename L >
        inline channel<T, L >::~channel()
        {
            close();
        }

        template<typename T, typename L >
        inline bool channel<T, L >::send(coroutine* c, const T& v)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }

            while(size_ >= capacity_) {
                lock_.release();
                c->delay(0);
                lock_.acquire();

                if(closed_) {
                    lock_.release();
                    return false;
                }
            }

            data_[ws_++] = v;
            if(ws_ == capacity_) ws_ = 0;
            ++size_;
            lock_.release();

            return true;
        }

        template<typename T, typename L >
        inline bool channel<T, L >::recv(coroutine* c, T& v)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }
            while(size_ <= 0) {
                lock_.release();
                c->delay(0);
                lock_.acquire();
                if(closed_) {
                    lock_.release();
                    return false;
                }
            }

            v = data_[rs_++];
            if(rs_ == capacity_) rs_ = 0;
            --size_;
            lock_.release();
            return true;
        }

        template<typename T, typename L >
        inline void channel<T, L >::close()
        {
            lock_.acquire();
            closed_ = true;
            lock_.release();
        }


    }
}
#endif
