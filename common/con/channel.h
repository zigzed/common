/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_CHANNEL_H
#define CXX_CON_CHANNEL_H
#include <queue>
#include "common/con/coroutine.h"
#include "common/sys/mutex.h"
#include "common/sys/error.h"
#include "common/datetime.h"

namespace cxx {
    namespace con {

        /** golang chan����ͨѶ���ơ�
         * channel���˿���ͨѶ�⣬������ģ�� wait/post ͬ�����塣channel ��� wait/post
         * ԭ����ŵ��У�
         *  ����Ҫ��scheduler������ʵ��
         *  send/recv�Ƕ��̰߳�ȫ�ģ�wait/post ���ǡ�send/recv���Կ�Խ�̡߳�scheduler
         * ���͡�wait/post����
         * wait/post ��� channel ���ŵ��У�
         *  post ���Թ㲥֪ͨ���еĵȴ�����send/recv ����
         *
         * channelĬ��ʹ�õ�ͬ��ԭ���� plainmutex�����ֻ��Ҫ��һ�� scheduler �ڲ�ʹ�ã�
         * �����ʹ�� null_mutex������, spin_mutex �����ܺ� plainmutex û��̫���ԵĲ��
         * ͬʱ spin_mutex �� windows �´��ڶ��߳���������⣨��ͬ���߳�ͬʱ����ͬһ��
         * spin_mutex �� acquire/release�������Ĭ�ϲ��� plainmutex
         */
        template<typename T, typename L = cxx::sys::plainmutex >
        class channel {
        public:
            explicit channel(size_t size);
            ~channel();

            bool send(coroutine* c, const T& v);
            bool send(coroutine* c, const T& v, int ms);
            bool recv(coroutine* c, T& v);
            bool recv(coroutine* c, T& v, int ms);
            void close();
        private:
            typedef std::queue<coroutine* > queue_t;

            L       lock_;
            bool    closed_;
            size_t  capacity_;
            size_t  size_;
            T*      data_;
            size_t  rs_;
            size_t  ws_;
            queue_t rq_;
            queue_t wq_;
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
        inline bool channel<T, L >::send(coroutine *c, const T &v, int ms)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }

            cxx::datetime when = cxx::datetime::now() + cxx::datetimespan(ms);
            while(size_ >= capacity_) {
                lock_.release();
                c->sleep(ms);

                lock_.acquire();
                if(closed_) {
                    lock_.release();
                    return false;
                }
                if(cxx::datetime::now() >= when) {
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
                r->resume();
            }
            else {
                lock_.release();
            }

            return true;
        }

        template<typename T, typename L >
        inline bool channel<T, L >::recv(coroutine *c, T &v, int ms)
        {
            lock_.acquire();
            if(closed_) {
                lock_.release();
                return false;
            }

            cxx::datetime when = cxx::datetime::now() + cxx::datetimespan(ms);
            while(size_ <= 0) {
                lock_.release();
                c->sleep(ms);

                lock_.acquire();
                if(closed_) {
                    lock_.release();
                    return false;
                }
                if(cxx::datetime::now() >= when) {
                    lock_.release();
                    return false;
                }
            }

            v = data_[rs_++];
            if(rs_ == capacity_) rs_ = 0;
            --size_;
            if(!wq_.empty()) {
                coroutine* w = wq_.front();
                wq_.pop();
                lock_.release();
                w->resume();
            }
            else {
                lock_.release();
            }
            return true;
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
                wq_.push(c);
                lock_.release();
                c->shift();

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
                r->resume();
            }
            else {
                lock_.release();
            }

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
                rq_.push(c);
                lock_.release();
                c->shift();

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
                coroutine* w = wq_.front();
                wq_.pop();
                lock_.release();
                w->resume();
            }
            else {
                lock_.release();
            }
            return true;
        }

        template<typename T, typename L >
        inline void channel<T, L >::close()
        {
            lock_.acquire();
            closed_ = true;

            while(!rq_.empty()) {
                rq_.front()->resume();
                rq_.pop();
            }
            while(!wq_.empty()) {
                wq_.front()->resume();
                wq_.pop();
            }

            lock_.release();
        }

    }
}

#endif
