/** Copyright (C) 2013 wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#include "common/sys/coroutine.h"
#include "context.h"

namespace cxx {
    namespace sys {

        int coroutine::idgen_ = 0;

        coroutine::coroutine(int stack_size)
            : taskcounts_(0), taskswitch_(0), taskexit_(0),
              running_(NULL), pending_(NULL), alltasks_(NULL),
              nalltask_(0), stack_(stack_size)
        {
            pending_ = new coroutine::context();
            tasklist_.head = NULL;
            tasklist_.tail = NULL;
        }

        coroutine::~coroutine()
        {
            task* t = tasklist_.head;
            task* p = t;
            while(t) {
                p = p->next;
                del_task(t);
                t = p;
            }
            delete pending_;
            free(alltasks_);
        }


        void* taskarg::p1(void * t)
        {
            coroutine::task* p = (coroutine::task* )t;
            return p;
        }

        void* taskarg::p2(void * t)
        {
            coroutine::task* p = (coroutine::task* )t;
            return p->startarg;
        }

        static void task_start(uint y, uint x)
        {
            coroutine::task* t;
            ulong z;

            z = x << 16;
            z <<= 16;
            z |= y;
            t = (coroutine::task* )z;

            t->startfn(t);

            coroutine::check(t);
            coroutine::stop(t, 0);
        }

        coroutine::task* coroutine::taskalloc(taskptr fn, void* arg, unsigned int stack)
        {
            coroutine::task *t;
            sigset_t zero;
            uint x, y;
            ulong z;

            /* allocate the task and stack together */
            t = (coroutine::task* )malloc(sizeof *t+stack);
            if(t == nil){
                fprint(2, "taskalloc malloc: %r\n");
                abort();
            }
            memset(t, 0, sizeof *t);
            t->stk = (uchar*)(t+1);
            t->stksize = stack;
            t->id = ++idgen_;
            t->startfn = fn;
            t->startarg = arg;
            t->engine = this;

            /* do a reasonable initialization */
            memset(&t->context.uc, 0, sizeof t->context.uc);
            sigemptyset(&zero);
            sigprocmask(SIG_BLOCK, &zero, &t->context.uc.uc_sigmask);

            /* must initialize with current context */
            if(getcontext(&t->context.uc) < 0){
                fprint(2, "getcontext: %r\n");
                abort();
            }

            /* call makecontext to do the real work. */
            /* leave a few words open on both ends */
            t->context.uc.uc_stack.ss_sp = t->stk+8;
            t->context.uc.uc_stack.ss_size = t->stksize-64;
        #if defined(__sun__) && !defined(__MAKECONTEXT_V2_SOURCE)		/* sigh */
        #warning "doing sun thing"
            /* can avoid this with __MAKECONTEXT_V2_SOURCE but only on SunOS 5.9 */
            t->context.uc.uc_stack.ss_sp =
                (char*)t->context.uc.uc_stack.ss_sp
                +t->context.uc.uc_stack.ss_size;
        #endif
            /*
             * All this magic is because you have to pass makecontext a
             * function that takes some number of word-sized variables,
             * and on 64-bit machines pointers are bigger than words.
             */
            z = (ulong)t;
            y = z;
            z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
            x = z>>16;

            makecontext(&t->context.uc, (void(*)())task_start, 2, y, x);

            return t;
        }

        coroutine::task* coroutine::create(taskptr fn, void* arg, int stack)
        {
            int id;
            task *t;

            t = taskalloc(fn, arg, stack);
            taskcounts_++;

            if(nalltask_%64 == 0){
                alltasks_ = (task** )realloc(alltasks_, (nalltask_+64)*sizeof(alltasks_[0]));
                if(alltasks_ == nil){
                    fprint(2, "out of memory\n");
                    abort();
                }
            }
            t->alltaskslot = nalltask_;
            alltasks_[nalltask_++] = t;
            taskready(t);
            return t;
        }

        void coroutine::ctxtshift(task *v, context *f, context *t)
        {
            if(swapcontext(&f->uc, &t->uc) < 0){
                fprint(2, "swapcontext failed: %r\n");
                assert(0);
            }
        }

        int coroutine::yield(void *v)
        {
            int n;

            task* t = (task* )v;
            n = t->engine->taskswitch_;
            t->engine->taskready(t->engine->running_);
            //state(t, "yield");
            t->engine->taskshift();

            return t->engine->taskswitch_ - n - 1;
        }

        void coroutine::taskshift()
        {
            needstack(running_, 0);
            ctxtshift(running_, &running_->context, pending_);
        }

        void** coroutine::data(void *v)
        {
            task* t = (task* )v;
            return &t->udata;
        }

        unsigned int coroutine::id(void *v)
        {
            task* t = (task* )v;
            return t->id;
        }

        void coroutine::stop(void *v, int status)
        {
            task* t = (task* )v;
            if(t && t->engine) {
                t->exiting = 1;
                t->engine->taskexit_ = status;
                t->engine->taskshift();
            }
        }

        void coroutine::quit(void* v, int status)
        {
            task* p = (task* )v;
            task* n = p;
            p->engine->taskexit_ = status;
            while(p) {
                p->exiting = 1;
                p = p->prev;
            }
            while(n) {
                n->exiting = 1;
                n = n->next;
            }
        }

        int coroutine::check(void* v)
        {
            int local_variable;
            task* t = (task* )v;

            if((uchar* )&local_variable > t->stk + t->stksize - 64) {
                print("out of bound 1: %p, %p, %d\n", &local_variable, t->stk, t->stksize);
                return (uchar* )&local_variable - t->stk - t->stksize - 64;
            }
            if((uchar* )&local_variable < t->stk + 8) {
                print("out of bound 2: %p, %p, %d\n", &local_variable, t->stk, t->stksize);
                return t->stk + 8 - (uchar* )&local_variable;
            }
            return 0;
        }

        void coroutine::needstack(coroutine::task *t, int n)
        {
            if((char*)&t <= (char*)t->stk
                    || (char*)&t - (char*)t->stk < 256+n){
                fprint(2, "task stack overflow: &t=%p tstk=%p n=%d\n", &t, t->stk, 256+n);
                abort();
            }
        }

        void coroutine::taskready(task *t)
        {
            t->ready = 1;
            add_task(t);
        }

        void coroutine::add_task(task *t)
        {
            task_list* l = &tasklist_;
            if(l->tail) {
                l->tail->next = t;
                t->prev = l->tail;
            }
            else {
                l->head = t;
                t->prev = (task* )nil;
            }
            l->tail = t;
            t->next = (task* )nil;
        }

        void coroutine::del_task(task *t)
        {
            task_list* l = &tasklist_;
            if(t->prev)
                t->prev->next = t->next;
            else
                l->head = t->next;
            if(t->next)
                t->next->prev = t->prev;
            else
                l->tail = t->prev;
        }

        typedef void (*taskentry)(int, char** );

        struct args {
            taskentry   func;
            int         argc;
            char**      argv;
        };

        static void taskmainstart(void* arg)
        {
            void* p1 = taskarg::p1(arg);
            void* p2 = taskarg::p2(arg);
            args* a = (args* )p2;
            a->func(a->argc, a->argv);
        }

        static void taskinfo(int s)
        {
            // dothing
        }

        int coroutine::start(void (*fn)(int, char **), int argc, char **argv)
        {
            struct sigaction sa, osa;

            memset(&sa, 0, sizeof sa);
            sa.sa_handler = taskinfo;
            sa.sa_flags = SA_RESTART;
            sigaction(SIGQUIT, &sa, &osa);

#ifdef SIGINFO
            sigaction(SIGINFO, &sa, &osa);
#endif

            argv0_ = argv[0];
            args arg;
            arg.func = fn;
            arg.argc = argc;
            arg.argv = argv;

            create(taskmainstart, &arg, stack_);
            return schedule();
        }

        int coroutine::schedule()
        {
            int i;
            task *t;

            for(;;){
                if(taskcounts_ == 0)
                    return (taskexit_);
                t = tasklist_.head;
                if(t == nil){
                    fprint(2, "no runnable tasks! %d tasks stalled\n", taskcounts_);
                    return (1);
                }
                del_task(t);

                if(!t->exiting) {
                    t->ready = 0;
                    running_ = t;
                    taskswitch_++;

                    ctxtshift(t, pending_, &t->context);
                }

                running_ = (task* )nil;
                if(t->exiting){
                    if(!t->system)
                        taskcounts_--;
                    i = t->alltaskslot;
                    alltasks_[i] = alltasks_[--nalltask_];
                    alltasks_[i]->alltaskslot = i;
                    free(t);
                }
            }
        }

        void coroutine::name(void *v, const char *fmt, ...)
        {
            va_list arg;
            task* t = (task* )v;

            va_start(arg, fmt);
            vsnprint(t->name, sizeof(t->name), fmt, arg);
            va_end(arg);
        }

        const char* coroutine::name(void *v)
        {
            task* t = (task* )v;
            return t->name;
        }

        void coroutine::state(void *v, const char *fmt, ...)
        {
            va_list arg;
            task* t = (task* )v;

            va_start(arg, fmt);
            vsnprint(t->state, sizeof(t->state), fmt, arg);
            va_end(arg);
        }

        const char* coroutine::state(void *v)
        {
            task* t = (task* )v;
            return t->state;
        }


    }
}


////////////////////////////////////////////////////////////////////////
/*
         * Stripped down print library.  Plan 9 interface, new code.
         */

enum
{
    FlagLong = 1<<0,
    FlagLongLong = 1<<1,
    FlagUnsigned = 1<<2
};

static char*
printstr(char *dst, const char *edst, const char *s, int size)
{
    int l, n, sign;

    sign = 1;
    if(size < 0){
        size = -size;
        sign = -1;
    }
    if(dst >= edst)
        return dst;
    l = strlen(s);
    n = l;
    if(n < size)
        n = size;
    if(n >= edst-dst)
        n = (edst-dst)-1;
    if(l > n)
        l = n;
    if(sign < 0){
        memmove(dst, s, l);
        if(n-l)
            memset(dst+l, ' ', n-l);
    }else{
        if(n-l)
            memset(dst, ' ', n-l);
        memmove(dst+n-l, s, l);
    }
    return dst+n;
}

char*
vseprint(char *dst, const char *edst, const char *fmt, va_list arg)
{
    int fl, size, sign, base;
    const char *p;
    char *w;
    char cbuf[2];

    w = dst;
    for(p=fmt; *p && w<edst-1; p++){
        switch(*p){
        default:
            *w++ = *p;
            break;
        case '%':
            fl = 0;
            size = 0;
            sign = 1;
            for(p++; *p; p++){
                switch(*p){
                case '-':
                    sign = -1;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    size = size*10 + *p-'0';
                    break;
                case 'l':
                    if(fl&FlagLong)
                        fl |= FlagLongLong;
                    else
                        fl |= FlagLong;
                    break;
                case 'u':
                    fl |= FlagUnsigned;
                    break;
                case 'd':
                    base = 10;
                    goto num;
                case 'o':
                    base = 8;
                    goto num;
                case 'p':
                case 'x':
                    base = 16;
                    goto num;
num:
                {
                    static char digits[] = "0123456789abcdef";
                    char buf[30], *p;
                    int neg, zero;
                    uvlong luv;

                    if(fl&FlagLongLong){
                        if(fl&FlagUnsigned)
                            luv = va_arg(arg, uvlong);
                        else
                            luv = va_arg(arg, vlong);
                    }else{
                        if(fl&FlagLong){
                            if(fl&FlagUnsigned)
                                luv = va_arg(arg, ulong);
                            else
                                luv = va_arg(arg, long);
                        }else{
                            if(fl&FlagUnsigned)
                                luv = va_arg(arg, uint);
                            else
                                luv = va_arg(arg, int);
                        }
                    }

                    p = buf+sizeof buf;
                    neg = 0;
                    zero = 0;
                    if(!(fl&FlagUnsigned) && (vlong)luv < 0){
                        neg = 1;
                        luv = -luv;
                    }
                    if(luv == 0)
                        zero = 1;
                    *--p = 0;
                    while(luv){
                        *--p = digits[luv%base];
                        luv /= base;
                    }
                    if(base == 16){
                        *--p = 'x';
                        *--p = '0';
                    }
                    if(base == 8 || zero)
                        *--p = '0';
                    w = printstr(w, edst, p, size*sign);
                    goto break2;
                }
                case 'c':
                    cbuf[0] = va_arg(arg, int);
                    cbuf[1] = 0;
                    w = printstr(w, edst, cbuf, size*sign);
                    goto break2;
                case 's':
                    w = printstr(w, edst, va_arg(arg, char*), size*sign);
                    goto break2;
                case 'r':
                    w = printstr(w, edst, strerror(errno), size*sign);
                    goto break2;
                default:
                    p = "X*verb*";
                    goto break2;
                }
            }
break2:
            break;
        }
    }

    assert(w < edst);
    *w = 0;
    return dst;
}

char*
vsnprint(char *dst, uint n, const char *fmt, va_list arg)
{
    return vseprint(dst, dst+n, fmt, arg);
}

char*
snprint(char *dst, uint n, const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vsnprint(dst, n, fmt, arg);
    va_end(arg);
    return dst;
}

char*
seprint(char *dst, char *edst, char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vseprint(dst, edst, fmt, arg);
    va_end(arg);
    return dst;
}

int
vfprint(int fd, const char *fmt, va_list arg)
{
    char buf[256];

    vseprint(buf, buf+sizeof buf, fmt, arg);
    return write(fd, buf, strlen(buf));
}

int
vprint(const char *fmt, va_list arg)
{
    return vfprint(1, fmt, arg);
}

int
fprint(int fd, const char *fmt, ...)
{
    int n;
    va_list arg;

    va_start(arg, fmt);
    n = vfprint(fd, fmt, arg);
    va_end(arg);
    return n;
}

int
print(const char *fmt, ...)
{
    int n;
    va_list arg;

    va_start(arg, fmt);
    n = vprint(fmt, arg);
    va_end(arg);
    return n;
}

char*
strecpy(char *dst, const char *edst, const char *src)
{
    *printstr(dst, edst, src, 0) = 0;
    return dst;
}
