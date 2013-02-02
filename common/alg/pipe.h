/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_PIPE_H
#define CXX_ALG_PIPE_H
#include "common/config.h"
#include "common/sys/atomic.h"

namespace cxx {
    namespace alg {

        namespace detail {
            /** queue is an efficient queue. it focus on following goal:
             *  + minimise number of allocation/deallocation.
             *  + allow one thread push/back, another thread do pop/front. but user
             * must ensure that there's no pop on empty queue and both threads don't
             * access same element with synchronize.
             */
            template<typename T, int N >
            class queue {
            public:
                queue();
                ~queue();
                T&      front();
                T&      back();
                void    push();
                void    unpush();
                void    pop();
            private:
                struct chunk_t {
                    T           data[N];
                    chunk_t*    prev;
                    chunk_t*    next;
                };
                chunk_t*    begin_;
                int         begin_pos_;
                chunk_t*    back_;
                int         back_pos_;
                chunk_t*    end_;
                int         end_pos_;

                cxx::sys::atomic_ptr_t<chunk_t >    spare_;
            };
        }

        template<typename T, int N >
        class pipe {
        public:
            inline pipe() {
                // insert a terminator element
                queue_.push();
                // let all pointer point to terminator
                r_ = w_ = f_ = &queue_.back();
                c_.set(&queue_.back());
            }

            inline ~pipe() { }

            inline void write(const T& value, bool incomplete) {
                queue_.back() = value;
                queue_.push();

                if(!incomplete) {
                    f_ = &queue_.back();
                }
            }

            inline bool unwrite(T* value) {
                if(f_ == &queue_.back())
                    return false;
                queue_.unpush();
                *value = queue_.back();
                return true;
            }

            inline bool flush() {
                if(w_ == f_) {
                    return true;
                }

                if(c_.cas(w_, f_) != w_) {
                    c_.set(f_);
                    w_ = f_;
                    return false;
                }

                w_ = f_;
                return true;
            }

            inline bool check_read() {
                if(&queue_.front() != r_ && r_)
                    return true;

                r_ = c_.cas(&queue_.front(), NULL);
                if(&queue_.front() == r_ || !r_)
                    return false;

                return true;
            }

            inline bool read(T* value) {
                if(!check_read())
                    return false;

                *value = queue_.front();
                queue_.pop();
                return true;
            }

            inline bool probe(bool (*fn)(T& )) {
                bool rc = check_read();
                assert(rc);

                return (*fn)(queue_.front());
           }
        private:
            /** queue to store pipe items
             * front is the first prefetched item, and used by reader only
             * back is the last un-flushed item, and used by writer only
             */
            detail::queue<T, N >    queue_;
            /** first un-flushed item. used by writer thread only */
            T*                      w_;
            /** first un-prefetched item. used by reader thread only */
            T*                      r_;
            /** first item to be flushed in the future */
            T*                      f_;
            /** the single point of contention between writer and reader thread.
             * point past the last flushed item. if it's NULL, reader is asleep,
             * it should be accessed using atomic operation
             */
            cxx::sys::atomic_ptr_t<T >  c_;

            pipe(const pipe& rhs);
            pipe& operator= (const pipe& rhs);
        };

        namespace detail {
            template<typename T, int N >
            inline queue<T, N >::queue()
                : begin_(NULL), begin_pos_(0), back_(NULL), back_pos_(0),
                  end_(NULL), end_pos_(0)
            {
                begin_ = (chunk_t* )malloc(sizeof(chunk_t));
                assert(begin_);
                end_ = begin_;
            }

            template<typename T, int N >
            inline queue<T, N >::~queue()
            {
                while(true) {
                    if(begin_ == end_) {
                        free(begin_);
                        break;
                    }
                    chunk_t* o = begin_;
                    begin_ = begin_->next;
                    free(o);
                }
                chunk_t* sc = spare_.xchg(NULL);
                if(sc) {
                    free(sc);
                }
            }

            template<typename T, int N >
            inline T& queue<T, N >::front()
            {
                return begin_->data[begin_pos_];
            }

            template<typename T, int N >
            inline T& queue<T, N >::back()
            {
                return back_->data[back_pos_];
            }

            template<typename T, int N >
            inline void queue<T, N >::push()
            {
                back_       = end_;
                back_pos_   = end_pos_;

                if(++end_pos_ != N)
                    return;

                chunk_t* sc = spare_.xchg(NULL);
                if(sc) {
                    end_->next  = sc;
                    sc->prev    = end_;
                }
                else {
                    end_->next  = (chunk_t* )malloc(sizeof(chunk_t));
                    assert(end_->next);
                    end_->next->prev = end_;
                }

                end_ = end_->next;
                end_pos_ = 0;
            }

            template<typename T, int N >
            inline void queue<T, N >::unpush()
            {
                // move 'back' backwards
                if(back_pos_)
                    --back_pos_;
                else {
                    back_pos_   = N - 1;
                    back_       = back_->prev;
                }

                // move 'end' backwards
                if(end_pos_)
                    --end_pos_;
                else {
                    end_pos_    = N - 1;
                    end_        = end_->prev;
                    free(end_->next);
                    end_->next  = NULL;
                }
            }

            template<typename T, int N >
            inline void queue<T, N >::pop()
            {
                if(++begin_pos_ == N) {
                    chunk_t* o  = begin_;
                    begin_      = begin_->next;
                    begin_->prev= NULL;
                    begin_pos_  = 0;

                    chunk_t* cs = spare_.xchg(o);
                    if(cs) {
                        free(cs);
                    }
                }
            }
        }
    }
}

#endif
