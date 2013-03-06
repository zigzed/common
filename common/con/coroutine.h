/** Copyright (C) wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#ifndef CXX_CON_COROUTINE_H
#define CXX_CON_COROUTINE_H
#include <stddef.h>
#include <map>
#include <vector>
#include "common/sys/threads.h"

namespace cxx {
    namespace con {

        class coroutine;
        typedef void (*taskptr)(coroutine*, void* );

        class stack {
        public:
            /** coroutine's default stack size (16 pages) */
            static size_t default_size();
            /** coroutine's minimum stack size (2 pages) */
            static size_t minimum_size();
            /** coroutine's maximum stack size (same as process's limit) */
            static size_t maximum_size();
        };

        /** scheduler is coroutine's run environment */
        class scheduler {
        public:
            struct task;
            struct context;

            explicit scheduler();
            ~scheduler();

            /** create a coroutine task from the given function */
            task*   spawn(taskptr func, void* arg, int stack = stack::default_size());
            /** get the current task */
            task*   ctask();
            /** give up the CPU for the current coroutine task */
            int     yield();
            /** give up the CPU for at least 'ms' milliseconds */
            int     delay(int ms);

            /** starting the coroutine with entry function fn */
            void    start();
            /** quit the scheduler */
            void    quit (int status);
        private:
            scheduler(const scheduler& rhs);
            scheduler& operator= (const scheduler& rhs);

            friend class coroutine;

            struct tasklist {
                task* head;
                task* tail;
            };

            static void taskmain(unsigned int y, unsigned int x);
            static void add_task(tasklist& list, task* t);
            static void del_task(tasklist& list, task* t);
            static void sleeptsk(coroutine* c, void* arg);
            static void blocktsk(coroutine* c, void* arg);
            static void setstate(task* t, const char* fmt, ...);

            int     schedule();
            void    taskready(task* t);
            void    taskshift();
            void    ctxtshift(context* f, context* t);
            void    needstack(task* t, int n);
            task*   taskalloc(taskptr p, void* arg, unsigned int stack);

            struct running;
            struct sleeping;

            running*    running_;
            sleeping*   sleeping_;
        };

        /** coroutine is a task running under scheduler */
        class coroutine {
        public:
            /** give up the CPU to other coroutine tasks */
            int         yield();
            /** give up the CPU to other coroutine at least for 'ms' milliseconds */
            int         delay(int ms);
            /** coroutine is ready for schedule */
            void        ready();
            void        shift();

            void**      data();
            void        state(const char* fmt, ...);
            const char* state() const;
            size_t      getid() const;
            scheduler*  sched();
            void        name (const char* fmt, ...);
            const char* name () const;
            void        stop (int status);
            void        system();
            scheduler::task* ctask() const;
        private:
            coroutine(scheduler* s, scheduler::task* t);
            coroutine(const coroutine& rhs);
            coroutine& operator= (const coroutine& rhs);

            friend class scheduler;
            scheduler*          sche_;
            scheduler::task*    task_;
        };

        class scheduler_group {
        public:
            typedef scheduler*  sched_t;

            explicit scheduler_group(size_t count);
            ~scheduler_group();

            void            start();
            void            stop(int status);
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
