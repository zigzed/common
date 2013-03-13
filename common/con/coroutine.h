/** Copyright (C) wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#ifndef CXX_CON_COROUTINE_H
#define CXX_CON_COROUTINE_H
#include <stddef.h>
#include <stdexcept>
#include <map>
#include <set>
#include <vector>
#include "common/datetime.h"
#include "common/sys/threads.h"
#include "common/sys/atomic.h"
#include "common/net/poller.h"

namespace cxx {
    namespace con {

        class coroutine_error : public std::runtime_error {
        public:
            coroutine_error(int code, const std::string& msg);
            int code() const;
        private:
            int code_;
        };

        class coroutine;
        typedef void (*taskptr)(coroutine*, void* );

        class stack {
        public:
            /** get the size of page in bytes */
            static size_t page_size();
            /** coroutine's default stack size (16 pages) */
            static size_t default_size();
            /** coroutine's minimum stack size (2 pages) */
            static size_t minimum_size();
            /** coroutine's maximum stack size (same as process's limit) */
            static size_t maximum_size();
        };

        struct fcontext_t;

        /** scheduler is coroutine's run environment */
        class scheduler {
        public:
            typedef fcontext_t context;

            scheduler();
            ~scheduler();

            /** create a coroutine task from the given function
             * complexity: O(1)
             * threadsafe: yes
             */
            void        spawn(taskptr func, void* arg, int stack = stack::default_size());
            /** starting the coroutine with entry function fn */
            void        start();
            /** resume the corotuine
             * complexity: O(1)
             * threadsafe: yes
             */
            void        resume(coroutine* c);
            /** yield coroutine at least 'ms' milliseconds
             * complexity: O(logN)
             * threadsafe: yes
             */
            void        sleep(coroutine* c, int ms);
            /** get the context of the scheduler, used internal
             * complexity: O(1)
             * threadsafe: yes
             */
            context*    ctxt();

            /** quit the scheduler */
            void        quit ();

            /** async wait for fd readable
             * complexity: O(logN)
             * threadsafe: no. can only be used by coroutine in the current scheduler
             * thread
             */
            void        await(coroutine* c, cxx::net::fd_t f, cxx::net::poller::readable r);
            /** async wait for fd writable
             * complexity: O(logN)
             * threadsafe: no. can only be used by coroutine in the current scheduler
             * thread
             */
            void        await(coroutine* c, cxx::net::fd_t f, cxx::net::poller::writable w);
            void        close(cxx::net::fd_t f);

            /** switch the coroutine from 'f' to 't'
             * complexity: O(1)
             * threadsafe: no. use internal
             */
            static void shift(context* f, context* t);
        private:
            scheduler(const scheduler& rhs);
            scheduler& operator= (const scheduler& rhs);         

            typedef std::multimap<cxx::datetime, coroutine* >                   block_t;
            typedef std::set<coroutine* >                                       ready_t;

            struct  command;
            struct  reactor;

            block_t                 block_; // blocked coroutine
            ready_t                 ready_; // ready coroutine
            reactor*                queue_;
            context*                ctxt_;
            static cxx::sys::atomic_t   idgen;

            int  calc_expire();
            void do_schedule(coroutine* c);
            void schedule();
        };

        /** coroutine is a task running under scheduler */
        class coroutine {
        public:
            /** give up the CPU to the scheduler, which can schedule other
             * coroutine tasks
             */
            void                yield();
            /** give up the CPU to the scheduler and other coroutines at least
             * 'ms' milliseconds
             */
            void                sleep(int ms);
            /** restore the context and ready to run */
            void                resume();
            /** the coroutine is stopped or not */
            bool                isstop();
            /** the coroutine is dead or not */
            bool                isdead();
            /** get the coroutine's scheduler */
            scheduler*          sched();
            /** get the context of the coroutine */
            scheduler::context* ctxt();
            /** stop the current coroutine */
            void                stop();
            int                 getid() const;
            /** give up the CPU till resume() called */
            void                shift();
        private:
            coroutine(const coroutine& rhs);
            coroutine& operator= (const coroutine& rhs);

            friend class scheduler;
            coroutine(scheduler* s, taskptr f, void* a, char* mem, size_t size);
            ~coroutine();

            //static void tmain(unsigned int y, unsigned int x);
            static void tmain(intptr_t arg);

            unsigned char*      stack_;
            size_t              size_;
            scheduler*          sche_;
            taskptr             func_;
            void*               args_;
            scheduler::context* ctxt_;
            cxx::sys::atomic_t  stop_;
            cxx::sys::atomic_t  dead_;
            int                 id_;
        };

        class scheduler_group {
        public:
            typedef scheduler*  sched_t;

            explicit scheduler_group(size_t count);
            ~scheduler_group();

            void            start();
            void            stop();
            size_t          size() const;
            sched_t&        operator[](size_t i);
            const sched_t&  operator[](size_t i) const;
        private:
            typedef std::vector<sched_t >           groups_t;
            typedef std::vector<cxx::sys::thread >  thread_t;
            size_t      size_;
            groups_t    sche_;
            thread_t    thrd_;
        };

    }
}

#endif
