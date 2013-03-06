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
