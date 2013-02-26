/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_PIPE_H
#define CXX_ALG_PIPE_H
#include "common/config.h"
#include "common/sys/atomic.h"
#include "common/sys/error.h"

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
                struct stat {
                    int acq;
                    int rel;
                    int add;
                    int del;
                };

                queue();
                ~queue();
                T&      front();
                T&      back();
                void    push();
                void    unpush();
                void    pop();
                const stat& info() const { return stat_; }
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
                stat        stat_;

                cxx::sys::atomic_ptr_t<chunk_t >    spare_;
            };
        }

        /** lock free queue implement
         * only a thread can read from the pipe and only a thread can write
         * to the pipe.
         *
         * see also: An Optimistic Approach to Lock-Free FIFO Queues,
         * Edya Ladan-Mozes and Nir Shavit
         */
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

            /** write an item to the pipe without flush it yet.
             */
            inline void write(const T& value) {
                // place the value to the queue, add new terminator.
                queue_.back() = value;
                queue_.push();

                f_ = &queue_.back();
            }

            /** flush all complete items to the pipe. return false if the reader
             * thread is sleeping
             */
            inline bool flush() {
                // if there're no un-flushed items, do nothing
                if(w_ == f_) {
                    return true;
                }

                // try to set 'c_' to 'f_'
                if(c_.cas(w_, f_) != w_) {
                    // compare and swap is not succeessful because 'c_' is NULL.
                    // this means that the reader is asleep. so we can ignore
                    // thread-safe and update the 'c_' in non-atomic manner, and
                    // return false to let the caller know that reader is sleeping
                    c_.set(f_);
                    w_ = f_;
                    return false;
                }

                // reader is alive. just move 'first un-flushed item' point to
                // the 'f_'
                w_ = f_;
                return true;
            }

            /** check whether item is available for reading
             */
            inline bool check_read() {
                // was the value is prefetched already? if so, return
                if(&queue_.front() != r_ && r_)
                    return true;

                // there's no prefetched item, so prefetch more values.
                // prefetching is to retrieve the pointer from 'c_' in atomic
                // fashion. if there're no items to prefetch, set 'c_' to NULL
                r_ = c_.cas(&queue_.front(), NULL);

                // if there're no elements prefetched, exit
                if(&queue_.front() == r_ || !r_)
                    return false;

                // there're at least one value prefetched
                return true;
            }

            /** reads an item from the pipe. return false if there're no items
             * available.
             */
            inline bool read(T* value) {
                // try to prefetch
                if(!check_read())
                    return false;

                // there're at least one value prefetched, return it
                *value = queue_.front();
                queue_.pop();
                return true;
            }

            /** apply the function to the first element in the pipe and return
             * the value returned by the function.
             * the pipe must NOT empty or the function will be crashed
             */
            inline bool probe(bool (*fn)(T& )) {
                bool rc = check_read();
                ENFORCE(rc);

                return (*fn)(queue_.front());
            }

            inline const typename detail::queue<T, N >::stat& info() const {
                return queue_.info();
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
                memset(&stat_, 0, sizeof(stat_));
                begin_ = (chunk_t* )malloc(sizeof(chunk_t));
                ENFORCE(begin_);
                end_ = begin_;

                stat_.acq++;
            }

            template<typename T, int N >
            inline queue<T, N >::~queue()
            {
                while(true) {
                    if(begin_ == end_) {
                        free(begin_);

                        stat_.rel++;
                        break;
                    }
                    chunk_t* o = begin_;
                    begin_ = begin_->next;
                    free(o);

                    stat_.rel++;
                }
                chunk_t* sc = spare_.xchg(NULL);
                if(sc) {
                    free(sc);

                    stat_.rel++;
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
                stat_.add++;

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
                    ENFORCE(end_->next);
                    end_->next->prev = end_;

                    stat_.acq++;
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

                    stat_.rel++;
                }

                stat_.add--;
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

                        stat_.rel++;
                    }
                }

                stat_.del++;
            }
        }
    }
}

#endif
