/** Copyright (C) 2013 wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#include "common/con/coroutine.h"
#include "context.h"
#include "common/sys/threads.h"
#include <sys/resource.h>
#include <set>

namespace cxx {
    namespace con {

        ////////////////////////////////////////////////////////////////////////
        struct scheduler::running {
            int                 count;
            int                 shift;
            int                 status;
            scheduler::task*    curr;
            scheduler::context* ctxt;
            scheduler::tasklist ready;
            scheduler::task**   tasks;
            int                 ntask;
            static size_t       idgen;

            running() :
                count(0), shift(0), status(0), curr(NULL), ctxt(NULL),
                tasks(NULL), ntask(0)
            {
                ctxt = new scheduler::context();
                ready.head = NULL;
                ready.tail = NULL;
            }
            ~running() {
                delete ctxt;
                free(tasks);
            }
        };

        struct scheduler::waiting {
            typedef std::pair<int, scheduler::task* >      pair_t;
            typedef std::map<int, scheduler::tasklist >    wait_t;
            typedef std::set<pair_t >                      post_t;
            wait_t              block;
            post_t              posts;
        };

        struct scheduler::sleeping {
            scheduler::tasklist sleep;
            int                 count;
            sleeping() : count(0) {
                sleep.head = NULL;
                sleep.tail = NULL;
            }
        };

        size_t scheduler::running::idgen   = 0;

        ////////////////////////////////////////////////////////////////////////
        static size_t page_size()
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
        static uvlong nsec()
        {
            struct timeval tv;

            if(gettimeofday(&tv, 0) < 0)
                return -1;
            return (uvlong)tv.tv_sec*1000*1000*1000 + tv.tv_usec*1000;
        }

        scheduler::scheduler()
            : running_(NULL), waiting_(NULL), sleeping_(NULL)
        {
            running_ = new scheduler::running();
        }

        scheduler::~scheduler()
        {
            delete sleeping_;
            delete waiting_;
            delete running_;
        }

        void scheduler::taskmain(uint y, uint x)
        {
            scheduler::task* t;
            ulong z;

            z = x << 16;
            z <<= 16;
            z |= y;
            t = (scheduler::task* )z;

            coroutine c(t->engine, t);
            t->startfn(&c, t->startarg);
            c.stop(0);
        }

        scheduler::task* scheduler::taskalloc(taskptr fn, void* arg, unsigned int stack)
        {
            scheduler::task *t;
            sigset_t zero;
            uint x, y;
            ulong z;

            /* allocate the task and stack together */
            t = (scheduler::task* )malloc(sizeof *t+stack);
            if(t == nil){
                fprint(2, "taskalloc malloc: %r\n");
                abort();
            }
            memset(t, 0, sizeof *t);
            t->stk = (uchar*)(t+1);
            t->stksize = stack;
            t->id = ++scheduler::running::idgen;
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

            makecontext(&t->context.uc, (void(*)())taskmain, 2, y, x);

            return t;
        }

        scheduler::task* scheduler::spawn(taskptr fn, void* arg, int stack)
        {
            int id;
            task *t;

            t = taskalloc(fn, arg, stack);
            running_->count++;

            if(running_->ntask % 64 == 0){
                running_->tasks =
                        (task** )realloc(running_->tasks,
                                         (running_->ntask + 64) * sizeof(running_->tasks[0]));
                if(running_->tasks == nil){
                    fprint(2, "out of memory\n");
                    abort();
                }
            }
            t->alltaskslot = running_->ntask;
            running_->tasks[running_->ntask++] = t;
            taskready(t);
            return t;
        }

        void scheduler::ctxtshift(context *f, context *t)
        {
            if(swapcontext(&f->uc, &t->uc) < 0){
                fprint(2, "swapcontext failed: %r\n");
                assert(0);
            }
        }

        void scheduler::setstate(task *t, const char *fmt, ...)
        {
            va_list arg;
            va_start(arg, fmt);
            vsnprint(t->state, sizeof(t->state), fmt, arg);
            va_end(arg);
        }

        int scheduler::yield()
        {
            int n;

            n = running_->shift;
            taskready(running_->curr);
            setstate(running_->curr, "yield");
            taskshift();
            return running_->shift - n - 1;
        }

        void scheduler::sleeptsk(coroutine* c, void* arg)
        {
            int i, ms;
            uvlong now;
            task* t;
            scheduler* s = c->sched();

            c->system();
            c->name("sleep");

            for(;;) {
                // let everyone else run
                while(s->yield() > 0)
                    ;
                if((t=s->sleeping_->sleep.head) == nil)
                    ms = 1000;
                else{
                    /* sleep at most 5s */
                    now = nsec();
                    if(now >= t->alarmtime)
                        ms = 0;
                    else if(now+5*1000*1000*1000LL >= t->alarmtime)
                        ms = (t->alarmtime - now)/1000000;
                    else
                        ms = 5000;
                }
                cxx::sys::threadcontrol::sleep(ms);

                // we're the only one runnable
                now = nsec();
                while((t = s->sleeping_->sleep.head) && now >= t->alarmtime) {
                    del_task(s->sleeping_->sleep, t);
                    if(!t->system && --s->sleeping_->count == 0)
                        s->running_->count--;
                    s->taskready(t);
                }
            }
        }

        int scheduler::delay(int ms)
        {
            uvlong when, now;
            task* running = running_->curr;
            task* t;

            if(!sleeping_) {
                sleeping_ = new scheduler::sleeping();
                spawn(sleeptsk, NULL, stack::minimum_size());
            }

            tasklist& s = sleeping_->sleep;

            now = nsec();
            when = now + (uvlong)ms * 1000000;
            for(t = s.head; t != nil && t->alarmtime < when; t = t->next)
                ;
            if(t) {
                running->prev = t->prev;
                running->next = t;
            }
            else {
                running->prev = s.tail;
                running->next = (scheduler::task* )nil;
            }

            t = running;
            t->alarmtime = when;
            if(t->prev)
                t->prev->next = t;
            else
                s.head = t;
            if(t->next)
                t->next->prev = t;
            else
                s.tail = t;

            if(!t->system && sleeping_->count++ == 0)
                running_->count++;
            taskshift();

            return (nsec() - now) / 1000000;
        }

        void scheduler::taskshift()
        {
            needstack(ctask(), 0);
            ctxtshift(&ctask()->context, running_->ctxt);
        }

        void scheduler::quit(int status)
        {
            running_->status = status;
            task* p = running_->ready.head;
            while(p) {
                p->exiting = 1;
                p = p->next;
            }
        }

        void scheduler::blocktsk(coroutine* c, void* arg)
        {
            scheduler* s = c->sched();
            s->delay(50);
            for(scheduler::waiting::wait_t::iterator it = s->waiting_->block.begin();
                it != s->waiting_->block.end(); ++it) {
                s->post(it->first, 2);
            }
        }

        bool scheduler::wait(int object, int ms)
        {
            if(!waiting_) {
                waiting_ = new scheduler::waiting();
                spawn(blocktsk, NULL, stack::minimum_size());
            }

            uvlong when, now;
            now = nsec();
            when = now + (uvlong)ms * 1000000;

            waiting::wait_t::iterator it;
            while((it = waiting_->block.find(object)) == waiting_->block.end()) {
                tasklist t;
                t.head = NULL;
                t.tail = NULL;
                waiting_->block.insert(std::make_pair(object, t));
            }
            tasklist& w = it->second;

            add_task(w, running_->curr);
            setstate(running_->curr, "wait");
            taskshift();

            scheduler::waiting::post_t::iterator st = waiting_->posts.find(std::make_pair(object, running_->curr));
            if(st != waiting_->posts.end()) {
                waiting_->posts.erase(st);
                return true;
            }
            else {
                while(nsec() < when) {
                    yield();
                }
                return false;
            }
        }

        int scheduler::post(int object, int all)
        {
            int i = 0;

            if(!waiting_)
                return 0;

            waiting::wait_t::iterator it = waiting_->block.find(object);
            if(it != waiting_->block.end()) {
                tasklist& w = it->second;

                for(i = 0; ; i++) {
                    if(i == 1 && !all) {
                        break;
                    }
                    task* t = NULL;
                    if((t = w.head) == nil)
                        break;
                    del_task(w, t);
                    taskready(t);
                    if(all != 2) {
                        waiting_->posts.insert(std::make_pair(object, t));
                    }
                }
            }
            return i;
        }

        scheduler::task* scheduler::ctask()
        {
            return running_->curr;
        }

        void scheduler::needstack(scheduler::task *t, int n)
        {
            if((char*)&t <= (char*)t->stk
                    || (char*)&t - (char*)t->stk < 256+n){
                fprint(2, "task stack overflow: &t=%p tstk=%p n=%d\n", &t, t->stk, 256+n);
                abort();
            }
        }

        void scheduler::taskready(task *t)
        {
            t->ready = 1;
            add_task(running_->ready, t);
        }

        void scheduler::add_task(tasklist& list, task *t)
        {
            tasklist* l = &list;
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

        void scheduler::del_task(tasklist& list, task *t)
        {
            tasklist* l = &list;
            if(t->prev)
                t->prev->next = t->next;
            else
                l->head = t->next;
            if(t->next)
                t->next->prev = t->prev;
            else
                l->tail = t->prev;
        }

        static void taskinfo(int s)
        {
            // dothing
        }

        void scheduler::start()
        {
            struct sigaction sa, osa;

            memset(&sa, 0, sizeof sa);
            sa.sa_handler = taskinfo;
            sa.sa_flags = SA_RESTART;
            sigaction(SIGQUIT, &sa, &osa);

#ifdef SIGINFO
            sigaction(SIGINFO, &sa, &osa);
#endif

            schedule();
        }

        int scheduler::schedule()
        {
            int i;
            task *t;

            for(;;){
                if(running_->count == 0)
                    return (running_->status);
                t = running_->ready.head;
                if(t == nil){
                    fprint(2, "no runnable tasks! %d tasks stalled\n", running_->count);
                    return (1);
                }
                del_task(running_->ready, t);

                if(!t->exiting) {
                    t->ready = 0;
                    running_->curr = t;
                    running_->shift++;

                    ctxtshift(running_->ctxt, &t->context);
                }

                running_->curr = (task* )nil;
                if(t->exiting){
                    if(!t->system)
                        running_->count--;
                    i = t->alltaskslot;
                    running_->tasks[i] = running_->tasks[--running_->ntask];
                    running_->tasks[i]->alltaskslot = i;
                    free(t);
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
        coroutine::coroutine(scheduler *s, scheduler::task *t)
            : sche_(s), task_(t)
        {
        }

        scheduler* coroutine::sched()
        {
            return sche_;
        }

        int coroutine::yield()
        {
            scheduler* sche = sched();
            return sche->yield();
        }

        int coroutine::delay(int ms)
        {
            scheduler* sche = sched();
            return sche->delay(ms);
        }

        void coroutine::ready()
        {
            sched()->taskready(task_);
        }

        void coroutine::shift()
        {
            sched()->taskshift();
        }

        void coroutine::system()
        {
            scheduler::task* t = this->task_;
            scheduler* sche = sched();
            if(!t->system) {
                t->system = 1;
                --sche->running_->count;
            }
        }

        scheduler::task* coroutine::ctask() const
        {
            return task_;
        }

        void** coroutine::data()
        {
            return &task_->udata;
        }

        size_t coroutine::getid() const
        {
            return task_->id;
        }

        void coroutine::stop(int status)
        {
            scheduler::task* t = this->task_;
            scheduler* sche = sched();
            if(t && sche) {
                t->exiting = 1;
                sche->running_->status = status;
                sche->taskshift();
            }
        }

        void coroutine::name(const char *fmt, ...)
        {
            va_list arg;
            scheduler::task* t = task_;

            va_start(arg, fmt);
            vsnprint(t->name, sizeof(t->name), fmt, arg);
            va_end(arg);
        }

        const char* coroutine::name() const
        {
            scheduler::task* t = task_;
            return t->name;
        }

        void coroutine::state(const char *fmt, ...)
        {
            va_list arg;
            scheduler::task* t = task_;

            va_start(arg, fmt);
            vsnprint(t->state, sizeof(t->state), fmt, arg);
            va_end(arg);
        }

        const char* coroutine::state() const
        {
            scheduler::task* t = task_;
            return t->state;
        }

        ////////////////////////////////////////////////////////////////////////
        scheduler_group::scheduler_group(size_t count)
            : size_(count)
        {
            for(size_t i = 0; i < size_; ++i) {
                scheduler* s = new scheduler();
                sche_.push_back(s);
            }
        }

        scheduler_group::~scheduler_group()
        {
            stop(-1);
            for(size_t i = 0; i < size_; ++i) {
                delete sche_[i];
            }
        }

        void scheduler_group::start()
        {
            for(size_t i = 0; i < size_; ++i) {
                scheduler* s = sche_[i];
                cxx::sys::thread t = cxx::sys::threadcontrol::create(cxx::MakeDelegate(s, &scheduler::start), "scheduler");
                thrd_.push_back(t);
            }
            for(size_t i = 0; i < size_; ++i) {
                thrd_[i].join();
            }
        }

        void scheduler_group::stop(int status)
        {
            for(size_t i = 0; i < size_; ++i) {
                sche_[i]->quit(status);
            }
        }

        size_t scheduler_group::size() const
        {
            return size_;
        }

        scheduler_group::sched_t& scheduler_group::operator [](size_t i)
        {
            return sche_[i];
        }

        const scheduler_group::sched_t& scheduler_group::operator [](size_t i) const
        {
            return sche_[i];
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
