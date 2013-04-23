/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CONDVAR_H
#define CXX_CONDVAR_H
#include "common/config.h"
#include "common/sys/mutex.h"
#if     defined(OS_LINUX)
    #include <pthread.h>
#elif   defined(OS_WINDOWS)
#endif

namespace cxx {
    namespace sys {

        class cond_var {
        public:
            cond_var();
            ~cond_var();
            void notify_one();
            void notify_all();

            void wait(cxx::sys::plainmutex& m);
            bool wait(cxx::sys::plainmutex& m, int ms);

            template<typename Pred >
            void wait(cxx::sys::plainmutex& m, Pred pred) {
                while(!pred()) wait(m);
            }
            template<typename Pred >
            bool wait(cxx::sys::plainmutex& m, Pred pred, int ms) {
                while(!pred()) {
                    if(!wait(m, ms))
                        return pred();
                }
                return true;
            }
        private:
#if     defined(OS_LINUX)
            pthread_cond_t  cond_;
#endif
        };

        template<typename T = int >
        class state {
        public:
            typedef T  value_type;

            state(value_type s) : state_(s) {}
            ~state() {}
            void set(value_type new_state) {
                {
                    cxx::sys::plainmutex::scopelock guard(mutex_);
                    state_ = new_state;
                }
                conds_.notify_all();
            }

            void wait(value_type exp_state) {
                cxx::sys::plainmutex::scopelock guard(mutex_);
                while(state_ != exp_state)
                    conds_.wait(mutex_);
            }

            value_type operator()() {
                cxx::sys::plainmutex::scopelock guard(mutex_);
                return state_;
            }
        private:
            state(const state& rhs);
            state& operator= (const state& rhs);

            cxx::sys::plainmutex    mutex_;
            cond_var                conds_;
            value_type              state_;
        };

    }

}

#endif
