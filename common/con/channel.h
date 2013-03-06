/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_CHANNEL_H
#define CXX_CON_CHANNEL_H
#include "common/con/coroutine.h"
#include "common/sys/mutex.h"
#include "common/sys/error.h"
#include "common/datetime.h"

namespace cxx {
    namespace con {

        /** golang chan风格的通讯机制。
         * channel除了可以通讯外，还可以模拟 wait/post 同步语义。channel 相对 wait/post
         * 原语的优点有：
         *  不需要在scheduler中特殊实现
         *  send/recv是多线程安全的，wait/post 不是。send/recv可以跨越线程、scheduler
         * 发送。wait/post不能
         * wait/post 相对 channel 的优点有：
         *  post 可以广播通知所有的等待对象，send/recv 不能
         *
         * channel默认使用的同步原语是 plainmutex。如果只需要在一个 scheduler 内部使用，
         * 则可以使用 null_mutex。另外, spin_mutex 的性能和 plainmutex 没有太明显的差别。
         * 同时 spin_mutex 在 windows 下存在多线程重入的问题（不同的线程同时调用同一个
         * spin_mutex 的 acquire/release）。因此默认采用 plainmutex
         */
        template<typename T, typename L = cxx::sys::plainmutex >
        class channel {
        public:
            explicit channel(size_t size);
            ~channel();

            bool send(coroutine* c, const T& v, int ms = -1);
            bool recv(coroutine* c, T& v, int ms = -1);
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
            delete[] data_;
        }

        template<typename T, typename L >
        inline bool channel<T, L >::send(coroutine* c, const T& v, int ms)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }

            cxx::datetime       cur(cxx::datetime::now());
            while(size_ >= capacity_ && (ms < 0 || cxx::datetime::now() < cur + cxx::datetimespan(ms))) {
                lock_.release();
                c->delay(0);

                lock_.acquire();
                if(closed_) {
                    lock_.release();
                    return false;
                }
            }

            if(size_ >= capacity_) {
                ENFORCE(ms < 0 || cxx::datetime::now() >= cur + cxx::datetimespan(ms))(ms)(cxx::datetime::now())(cur);
                lock_.release();
                return false;
            }

            data_[ws_++] = v;
            if(ws_ == capacity_) ws_ = 0;
            ++size_;
            lock_.release();

            return true;
        }

        template<typename T, typename L >
        inline bool channel<T, L >::recv(coroutine* c, T& v, int ms)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }

            cxx::datetime       cur(cxx::datetime::now());
            while(size_ <= 0 && (ms < 0 || cxx::datetime::now() < cur + cxx::datetimespan(ms))) {
                lock_.release();
                c->delay(0);
                lock_.acquire();
                if(closed_) {
                    lock_.release();
                    return false;
                }
            }

            if(size_ <= 0) {
                lock_.release();
                ENFORCE(ms < 0 || cxx::datetime::now() >= cur + cxx::datetimespan(ms))(ms)(cxx::datetime::now())(cur);
                return false;
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
