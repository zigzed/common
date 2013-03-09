/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_CHANNEL_H
#define CXX_ALG_CHANNEL_H
#include "common/config.h"
#include "common/sys/event.h"
#include "common/sys/mutex.h"
#include "common/alg/pipe.h"
#include <cassert>

namespace cxx {
    namespace alg {

        /** channel: a go lang like channel for communication
         * a channel is a MPSC (Multi Producer Single Consumer) queue or SPSC
         * (Single Producer Single Consumer).
         * in MPSC mode, 'L' should be cxx::sys::plainmutex
         */
        template<typename M, int N, typename L = cxx::sys::null_mutex >
        class channel {
        public:
            channel();
            ~channel();

            sys::event::handle_t    get_fd() const;
            void                    send(const M& m);
            bool                    recv(M* m, int timeout);
            size_t                  size();
        private:
            typedef pipe<M, N > pipe_t;

            L           mutex_;
            sys::event  event_;
            pipe_t      pipes_;
            bool        active_;
            size_t      wsize_;
            size_t      rsize_;

            channel(const channel& rhs);
            channel& operator= (const channel& rhs);
        };

        template<typename M, int N, typename L >
        inline channel<M, N, L >::channel()
            : active_(false), wsize_(0), rsize_(0)
        {
            bool ok = pipes_.read(NULL);
            assert(!ok);
        }

        template<typename M, int N, typename L  >
        inline channel<M, N, L >::~channel()
        {
        }

        template<typename M, int N, typename L  >
        inline sys::event::handle_t channel<M, N, L>::get_fd() const
        {
            return event_.handle();
        }

        template<typename M, int N, typename L  >
        inline void channel<M, N, L >::send(const M& m)
        {
            bool ok = false;
            {
                typename L::scopelock lock(mutex_);
                pipes_.write(m);
                ok = pipes_.flush();
                ++wsize_;
            }
            if(!ok)
                event_.send();
        }

        template<typename M, int N, typename L  >
        inline bool channel<M, N, L >::recv(M* m, int timeout)
        {
            if(active_) {
                bool ok = pipes_.read(m);
                if(ok) {
                    ++rsize_;
                    return true;
                }
                active_ = false;
                event_.recv();
            }

            bool rc = event_.wait(timeout);
            if(!rc)
                return false;

            active_ = true;
            bool ok = pipes_.read(m);
            assert(ok);
            if(ok) {
                ++rsize_;
            }
            return ok;
        }

        template<typename M, int N, typename L >
        inline size_t channel<M, N, L >::size()
        {
            typename L::scopelock lock(mutex_);
            return wsize_ - rsize_;
        }

    }
}

#endif
