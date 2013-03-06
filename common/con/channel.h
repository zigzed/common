/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_CHANNEL_H
#define CXX_CON_CHANNEL_H
#include "common/con/coroutine.h"
#include "common/sys/atomic.h"
#include <queue>

namespace cxx {
    namespace con {

        class spinlock {
        public:
            spinlock();
            void acquire();
            void release();
        private:
            enum { Locked, Unlocked };
            cxx::sys::atomic_t  state_;
        };

        template<typename T >
        class channel {
        public:
            explicit channel(size_t size);
            ~channel();

            bool send(coroutine* c, const T& v);
            bool recv(coroutine* c, T& v);
        private:
            void close();

            spinlock    lock_;
            bool    closed_;
            size_t  capacity_;
            size_t  size_;
            T*      data_;
            size_t  rs_;
            size_t  ws_;
            typedef std::queue<coroutine* > coque_t;
            coque_t rq_;
            coque_t wq_;
        };

        template<typename T >
        inline channel<T >::channel(size_t size)
            : closed_(false), capacity_(size), size_(0), data_(new T[size]),
              rs_(0), ws_(0)
        {
        }

        template<typename T >
        inline channel<T >::~channel()
        {
            //close();
        }

        template<typename T >
        inline bool channel<T >::send(coroutine* c, const T& v)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }

            while(size_ >= capacity_) {
                wq_.push(c);
                lock_.release();
                c->yield();
                lock_.acquire();

                if(closed_) {
                    lock_.release();
                    return false;
                }
            }

            data_[ws_++] = v;
            if(ws_ == capacity_) ws_ = 0;
            ++size_;
            if(!rq_.empty()) {
                coroutine* r = rq_.front();
                rq_.pop();
                lock_.release();
                r->yield();
            }
            else {
                lock_.release();
            }

            return true;
        }

        template<typename T >
        inline bool channel<T >::recv(coroutine* c, T& v)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }
            while(size_ <= 0) {
                rq_.push(c);
                lock_.release();
                c->yield();
                lock_.acquire();
                if(closed_) {
                    lock_.release();
                    return false;
                }
            }

            v = data_[rs_++];
            if(rs_ == capacity_) rs_ = 0;
            --size_;
            if(!wq_.empty()) {
                coroutine* r = wq_.front();
                wq_.pop();
                lock_.release();
                r->yield();
            }
            else {
                lock_.release();
            }
            return true;
        }

        template<typename T >
        inline void channel<T >::close()
        {
            lock_.acquire();
            closed_ = true;
            while(!rq_.empty()) {
                rq_.front()->ready();
                rq_.pop();
            }
            while(!wq_.empty()) {
                wq_.front()->ready();
                wq_.pop();
            }
            lock_.release();
        }


    }
}
#endif
