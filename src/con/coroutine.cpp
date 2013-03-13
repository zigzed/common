/** Copyright (C) 2013 wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#include "common/con/coroutine.h"
//#include "context.h"
#include "fcontext.h"
#include "common/sys/threads.h"
#include "common/sys/error.h"
#include <sys/resource.h>
#include <set>
#include <unistd.h>

namespace cxx {
    namespace con {

        ////////////////////////////////////////////////////////////////////////
        size_t stack::page_size()
        {
            return sysconf(_SC_PAGESIZE);
        }

        size_t stack::default_size()
        {
            size_t s = 16 * page_size();
            return s > maximum_size() ? maximum_size() : s;
        }

        size_t stack::minimum_size()
        {
            return 2 * page_size();
        }

        size_t stack::maximum_size()
        {
            rlimit  lim;
            getrlimit(RLIMIT_STACK, &lim);
            return lim.rlim_cur;
        }

        ////////////////////////////////////////////////////////////////////////
        void coroutine::tmain(intptr_t arg)
        {
            coroutine* c = (coroutine* )arg;
            try {
                c->func_(c, c->args_);
            }
            catch(const coroutine_error& e) {
                fprintf(stderr, "coroutine error: %d - %s\n", e.code(), e.what());
            }
            c->dead_.store(1);
            scheduler::shift(c->ctxt(), c->sched()->ctxt());
        }

        coroutine::coroutine(scheduler *s, taskptr f, void *a, char* mem, size_t size)
            : stack_(NULL), size_(size), sche_(s), func_(f), args_(a),
              ctxt_(NULL), stop_(0), dead_(0)
        {
            stack_ = (unsigned char* )mem;
            /** make_fcontext 需要根据栈的方向进行调整 */
            ctxt_ = make_fcontext(stack_ + size_, size_, tmain);
        }

        coroutine::~coroutine()
        {

        }

        void coroutine::yield()
        {
            sche_->resume(this);
            shift();
        }

        void coroutine::resume()
        {
            if(isstop() || isdead()) {
                return;
            }
            sche_->resume(this);
        }

        void coroutine::sleep(int ms)
        {
            if(ms >= 0)
                sche_->sleep(this, ms);
            shift();
        }

        void coroutine::shift()
        {
            if(isstop()) {
                throw coroutine_error(0, "yield a stopped coroutine");
            }
            scheduler::shift(ctxt_, sche_->ctxt());
            if(isstop()) {
                throw coroutine_error(0, "coroutine stopped out of scope");
            }
        }

        bool coroutine::isstop()
        {
            return stop_;
        }

        bool coroutine::isdead()
        {
            return dead_;
        }

        scheduler* coroutine::sched()
        {
            return sche_;
        }

        scheduler::context* coroutine::ctxt()
        {
            return ctxt_;
        }

        void coroutine::stop()
        {
            stop_.store(1);
        }

        int coroutine::getid() const
        {
            return id_;
        }

    }
}
