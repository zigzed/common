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
//        void coroutine::tmain(unsigned int y, unsigned int x)
//        {
//            intptr_t z;

//            z = x << 16;
//            z <<= 16;
//            z |= y;

//            coroutine* c = (coroutine* )z;

//            try {
//                c->func_(c, c->args_);
//            }
//            catch(const coroutine_error& e) {
//                fprintf(stderr, "coroutine error: %d - %s\n", e.code(), e.what());
//            }
//            c->dead_.store(1);
//            scheduler::shift(c->ctxt(), c->sched()->ctxt());
//        }
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
            ctxt_ = make_fcontext(stack_ + size_, size_, tmain);

            /** 避免多次内存分配，将 context 直接从 stack 上分配 */
//            ctxt_ = new (mem) scheduler::context;
//            memset(&ctxt_->uc, 0, sizeof(ctxt_->uc));
//            stack_ = reinterpret_cast<unsigned char* >(mem + sizeof(scheduler::context));
//            size_ -= sizeof(scheduler::context);

//            sigset_t zero;
//            sigemptyset(&zero);
//            sigprocmask(SIG_BLOCK, &zero, &ctxt_->uc.uc_sigmask);

//            /** must initialize with current context */
//            if(getcontext(&ctxt_->uc) < 0) {
//                int err = cxx::sys::err::get();
//                std::string msg = "get context failed: ";
//                msg += cxx::sys::err::str(err);
//                throw coroutine_error(err, msg);
//            }

//            /** call makecontext to do the real work
//             * leave a few words open on both ends
//             */
//            ctxt_->uc.uc_stack.ss_sp     = stack_ + 8;
//            ctxt_->uc.uc_stack.ss_size   = size_ - 64;

//#if defined(__sun__) && !defined(__MAKECONTEXT_V2_SOURCE)
//    #warning "doing sun thing"
//            /* can avoid this with __MAKECONTEXT_V2_SOURCE but only on SunOS 5.9 */
//            ctxt_->uc.uc_stack.ss_sp     =
//                    (char* )ctxt_->uc.uc_stack.ss_sp
//                    + ctxt_->uc.uc_stack.ss_size;
//#endif
//            /** all this magic is because you have to pass makecontext a
//             * function that takes some number of word-sized variables,
//             * and on 64-bit machines pointers are bigger than words
//             */
//            intptr_t        z = (intptr_t)this;
//            unsigned int    x;
//            unsigned int    y;
//            y = z;
//            z >>= 16;   /* hide undefined 32-bit shift from 32-bit compilers */
//            x = z>> 16;

//            makecontext(&ctxt_->uc, (void(*)())tmain, 2, y, x);
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
